//Chen Gleichger ID 201050135

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_ARGS_TO_CMD (128)

void run_cmd(char* argv) {

    int i = 0;
    int j;
    char* p = strtok(argv, " ");
    char* array[MAX_ARGS_TO_CMD];

    while (NULL != p) {
        array[i++] = p;
        p = strtok(NULL, " ");
    }
    for (j = 1; j < i; j++) {
        array[j - 1] = array[j];
    }
    array[j] = '\0';
    execvp(argv, array);
}
/**
* Responsible to start the program && get the arguments from stdin
*/
int main(int argc, char** argv) {

    if (2 >= argc) {
        if (1 == argc) {
            printf("No commands entered. \n");
        }
        else {
            printf("Cannot pipe less than at least 2 commands \n");
        }
        printf("Usage: %s <command to run> <command to pipe> \n ", argv[0]);
        return ENOENT;
    }

    int pid_array_size = argc * sizeof(pid_t);
    int next_stdin;
    int stdin_fileno = STDIN_FILENO;
    int stdout_fileno;

    pid_t* childs_to_run = malloc(pid_array_size);
    if (!childs_to_run)
        exit(errno);
    memset(childs_to_run, 0, pid_array_size);

    // ./main ls cat [0==ls, 1==cat]
    // ls , i < 1
    for (int i = 0; i < argc; ++i) {
        if (i < argc - 1) {
            int pipefd[2];
            if (pipe2(pipefd, O_CLOEXEC) < 0)
                exit(errno);
            next_stdin = pipefd[0];
            stdout_fileno = pipefd[1];
        }
        else {
            next_stdin = -1;
            stdout_fileno = STDOUT_FILENO;
        }

        pid_t pid = fork(); // get the PID of the fork
        if (pid < 0)  // if fails, returned -1
            exit(errno);

        if (pid == 0) { 
            if (dup2(stdin_fileno, STDIN_FILENO) < 0)  // stdin error
                exit(errno);

            if (dup2(stdout_fileno, STDOUT_FILENO) < 0)  // stdout error
                exit(errno);

            run_cmd(argv[i]); // no errors, run child
        }

        if (STDIN_FILENO != stdin_fileno)
            close(stdin_fileno); // close the fd

        if (STDOUT_FILENO != stdout_fileno)
            close(stdout_fileno); // close the fd

        childs_to_run[i] = pid; // add the PID to the list
        stdin_fileno = next_stdin; // get the next one
    }

    for (int i = 0; i < argc; ++i) {
        int status;
        if (waitpid(childs_to_run[i], &status, 0) != childs_to_run[i])
            exit(errno); // failed to wait for PID
    }

    free(childs_to_run); // free some moemory
    return 0; // exit
}