# "Advanced Topics in Operating Systems" 
 M.Sc. course at the Reichman University <br/>
## piper

This program is a part of our 1st assignment <br/>
Check out <a href="https://github.com/GlaiChen/piper/blob/main/assignment-1.md">piper/assignment-1.md</a> for full details of the assignment <br/>
This program is only the invidual part, and the group part will be commited in dtrugman repo, at the following link - https://github.com/dtrugman/linux-hw1-os </br>

## Invidual Part - Part A
For full answer (which is the actual code) - check out the code at <a href="https://github.com/GlaiChen/piper/blob/main/main.c">piper/main.c </a> <br/>
The main purpose was to write a program that takes a series of program names as arguments, <br/>
and executes them as piped command, e.g. `ps aux | grep "init"` <br/>

## Invidual Part - Part B
In part B, we were asked to consider the command `ps aux | grep pizza-margarita`. "Assuming that at the time of executing this command, there isn't any process with that name in the system - what would be the expected out of this command? Explain why, based on your experience in writing _piper_." <br/>
<br/>
As we learned, pipe commands run in parallel. In that case, we cannot guarantee which command will eventualy run first. <br/>
Actually, it's a matter of details of the workings of the shell combined with scheduler fine-tuning deep in the bowels of the kernel .<br/>
The shell first creates the pipe, the "conduit" for the data that will flow between the processes, and then creates the processes with the ends of the pipe connected to them. <br/>
The first process that is run may block waiting for input from the second process, or block waiting for the second process to start reading data from the pipe. <br/>
In eventualy, the data gets transfered and everything works just fine, as we mentioned before, no matter which command ran first. <br/>

When you run the useage `ps aux | grep "pizza-margarita"` or any other string followed by the `grep`, assuming that the is no process running with that name in the system, we should receive an output with the details of the actual grep process, including the string followed by. <br/>
 <br/>
E.g. with `pipeline` command and the followed outputs: <br/>

![imag <br/>e](https://user-images.githubusercontent.com/70802568/141855567-df6cb7d4-f0a9-4938-9074-7799a42a7d43.png)
