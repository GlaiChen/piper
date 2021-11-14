# Homework 1

**Advanced Operating Systems, Term 1, 2022** <br/>
**Updated**: Monday 25.10.2021 at 14:22 IDT <br/>
**Due**: Tuesday, 9.11.2021 at 11:59pm IDT

## Instructions

Group collaboration (up to 2 people) is permitted on the kernel programming
problems. All other problems in this assignment are to be done individually.
Submission of all homework is recommended to be made via Git.

## Individual Problems:

### (1) Spawn new process - under the hood

A typical shell executes user commands by calling the _fork()_ and then (as the
child) the _execve()_ system calls. In response to piped commands, i.e. several
commands connected by pipes, the shell calls _pipe()_, then _fork()_, then each
child rearranges the stdin/stdout using _dup2()_, and finally _exec()_.

(a) Write a program that takes a series of program names as arguments, and
executes them as piped command. Name the program _piper_, whose usage is:

        usage: piper ARG1 ARG2 [...]

For example, the command:

        piper ls sort head
would operate as if the user has typed at the shell prompt the command:

        ls | sort | head

For this, you will need to use the system calls _fork(2)_ and _execve(2)_ to
execute programs, as well as _pipe(2)_ to create the pipes and _dup2(2)_ to
connect a program's stdin/out to the corresponding pipe.

Note: the _man_ command provides help text for system calls, e.g. _man 2 pipe_.

(b) Consider the command _ps aux | grep pizza-margarita_. Assuming that at the
time of executing this command, there isn't any process with that name in the
system - what would be the expected out of this command? Explain why, based on
your experience in writing _piper_.

## Group kernel programming:

The kernel programming will be done using a Linux VM. Linux platforms can run
on many different architectures, but we will use the X86(64) CPU family. We
will develop on the Linux 5.4 kernel.

In this assignment you will write a system call to dump the process tree and a
user space program to use the system call.

The system call should take three arguments and copy the process tree data
to a buffer in a breadth-first-search (BFS) order. It should only report and
count processes, not threads.

The prototype for the system call will be:

        int ptree(struct prinfo *buf, int *nr, int pid);

You should define, in _include/linux/prinfo.h_, the _struct prinfo_ as:

        struct prinfo {
            pid_t parent_pid;       /* process id of parent */
            pid_t pid;              /* process id */
            long state;             /* current state of process */
            uid_t uid;              /* user id of process owner */
            char comm[16];          /* name of program executed */
            int level;              /* level of this process in the subtree */
        };

To get the _task_struct_ for a given pid, use the following snippet:

        if (root_pid == 0)
            return &init_task;
        else
            return find_task_by_vpid(root_pid);

Parameters description

- _buf_ points to a buffer to store the process tree's data. The data should
  be in BFS order: processes at a higher level should appear earlier (where
  level 0 is considered higher than e.g. level 5).

- _nr_ indicates the size of this buffer (number of entries). The system call
  copies at most that many entries of the process tree data to the buffer and
  stores the number of entries actually copied in _nr_.

- _pid_ indicated the pid of the root of the subtree to traverse. processes
  outside of this subtree should be omitted.

- Return value: return 0 on success, negative error number otherwise.

When traversing the tree in BFS order, fill the buffer until it's full. For
instance, for the tree:

             0
           /   \
          1     2
         / \   / \
        3   4  5  6
                        
Given a buffer with nr==4, the buffer should hold:

        [
          prinfo {pid-0, parent-pid-0, level-0},
          prinfo {pid-1, parent-pid-0, level-1},
          prinfo {pid-2, parent-pid-0, level-1},
          prinfo {pid-3, parent-pid-1, level-2};
        ]
                        
Do not traverse more of the tree than needed: only the first and at most _nr_
processes. There should be no duplicate data of processes inside the buffer.

### (1) Write a skeletal new system call

In this section you will write a simple loadable module, and a new system call
that calls a function of that module. The idea is to simplify development and
testing by using a loadable module that can be reloaded dynamically without
tedious kernel compile, install, and reboot.

The module should implement the function _get_ptree()_, which takes the same
arguments as the system call above, except that _buf_ is a kernel (not user)
pointer to kernel memory. The function _get_ptree()_ should fill the buffer
with _nr_ dummy entries as follows:

        [
          parent_pid=0, pid=1, state=0, uid=0, comm="dummy", level=0
          parent_pid=1, pid=2, state=0, uid=0, comm="dummy", level=1
          parent_pid=2, pid=3, state=0, uid=0, comm="dummy", level=2
          ...
        ]

The system call should use the number 449, and be implemented in _kernel/ptree.c_.
Note that it does not know the function at compile time, because it belongs to
a module. Instead, it should implement functions for the module to register and
unregister its function at load and unload times, respectively:

        typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
        int register_ptree(ptree_func func);
        void unregister_ptree(ptree_func func);

The function _register_ptree()_ remembers the function pointer and returns 0 on
success, or -EBUSY if another pointer is already registered; and the function
_unregister_ptree()_ removes the function pointer. They should be placed in the
same file as the new system call.

The system call should check if some _get_ptree()_ is registered, and if not,
try to load the module using the kernel's _request_module()_ helper. If still
not registered, the system call should return -ENOSYS, otherwise it should call
the function, then copy the results to userspace, and return appropriate value.

Note: it can be unsafe to call the registered _get_ptree()_ function (why?). To
remedy this, you will need to modify the register/unregister functions, or use
the kernel helper _find_module()_ (see the implementation of _delete_module()_
in _kernel/module.c_ for example). If you need some form locking you may use
the kernel's spinlock as follows:

        DEFINE_SPINLOCK(my_lock);
        spin_lock(&my_lock);
        spin_unlock(&my_lock);

### (2) Implement the full system call

Implement the full semantics of the system call by adding the necessary logic
to the module's _get_ptree()_ instead of the dummy data currently provided.
Remember to not use recursion since the size of the kernel stack is limited.

The code should handle errors that could occur. It should detect and handle at
least the following:

- -EINVAL: if _buf_ or _nr_ are null, or if the _nr_ is less than 1
- -EFAULT: if _buf_ or _nr_ are outside the accessible address space.

When traversing the process tree, hold the _tasklist_lock_ to prevent races.
Grab this lock before you begin, and only release after ending the traversal.
While holding the lock, the code must not do any operations that may result in
a sleep, such as memory allocation, copying in/out of data, etc. To grab and
release thee lock, use the following snippet:

        read_lock(&tasklist_lock);
        do_some_work();
        read_unlock(&tasklist_lock);
 
### (3) Test the new system call

Write a simple C program which calls _ptree()_ with a pid as an argument (if no
argument is provided, return the entire process tree).

Since the actual tree size is unknown in advance, you should start with some
reasonable size for calling _ptree()_, then if insufficient for storing the
the tree, repeatedly double the buffer size and call _ptree()_ until the full
full process tree is received.

Print the contents of the buffer from index 0 to the end. For each process, you
use the following format for program output:

        printf("%d,%s,%d,%d,%ld,%d,%d\n", buf[i].level, buf[i].comm,
        buf[i].pid, buf[i].parent_pid, buf[i].state, buf[i].uid);

Example program output:

        0,swapper/0,0,0,0,0
        1,systemd,1,0,1,0
        1,kthreadd,2,0,1,0
        2,systemd-journal,2924,1,1,0
        ....
        2,kworker/u128:1,1034,2,1026,0
        2,kworker/3:2,1419,2,1026,0
        ...
        5,zsh,1654,1653,1,1000
        ...

Note that the new system call is not supported through the standard libc (hence
not generally accessible); Use the general purpose _syscall(2)_ to invoke the
system call indirectly (see the man page for more details).

Use the Linux _ps_ and _pstree_ commands to verify the accuracy of the data
printed by your test program.

