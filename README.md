# XV6

## PART 1 - System Calls

### 1.1 - Implementing `syscount` system call

We count syscalls made by process in syscall.c and pass child's syscalls to parent in exit function in proc.c. We also print the number of syscalls if mask has been set in exit function.

**BASIC CHANGE IN PROCESS STRUCTURE -**
**_int p->syscalls[32]_**   array ininitalized to 0s in `allocproc` function in `proc.c` file.
**_int p->mask_**            initialized to 0 in `allocproc` function in `proc.c` file.


1. Added a new system call `syscount` in `syscall.h` file as syscall number 23.

2. Added relevant function in and changes in `syscall.c` file. Also changed the `syscall` function to handle the new system call and add syscalls to process in `p->syscalls[num]++`.

3. Added the function prototype in `user.h` file.

4. Relevant change in `usys.pl` file.

5. `syscount.c` file added in `user` directory to implement the system call.

6. Added the system call in `sysproc.c` file.

7. Modified exit function in `proc.c` file to add child's syscalls to parent's syscalls and print the number of syscalls if mask has been set.

8. Appropriate changes in `Makefile` in `UPROGS` section.

### 1.2 - Implementing `sigalarm` and `sigreturn` system call

Modified `trap.c` so that if alarmticks are set, then it reduces the alarmticks by 1 each tick and if alarmticks becomes 0, then we save current trapframe and execute the alarm handler. `sigreturn` restores the trapframe and sets the alarmflag to 0.

**BASIC CHANGE IN PROCESS STRUCTURE -**
**_int p->alarmticks_**   initialized to 0 in `allocproc` function in `proc.c` file. Changed by `sigalarm` syscall.
**_int p->ticksleft_**            initialized to 0 in `allocproc` function in `proc.c` file.
**_uint64 p->alarmhandler_**            initialized to 0 in `allocproc` function in `proc.c` file. Changed by `sigalarm` syscall.
**_int p->alarmflag_**            initialized to 0 in `allocproc` function in `proc.c` file. Changed by `trap.c` when ticksleft hit 0.
**_struct trapframe* p->alarmtrapframe_**            initialized to 0 in `allocproc` function in `proc.c` file. Changed by `trap.c` when ticksleft hit 0.

1. Added a new system calls `sigalarm` and `sigreturn` in `syscall.h` file as syscall number 24 and 25.

2. Added relevant function in and changes in `sysproc.c` file.

3. Added the function prototypes in `user.h` file.

4. Relevant changes in `usys.pl` file.

5. Changes in `trap.c` file to lower ticksleft which are initialised to alarmticks at the start. When they hit 0, we save the trapframe and execute the alarm handler.

## PART 2 - Scheduling

### 2.1 - Implementing Lottery Based Scheduler (LBS)

Implemented the lottery based scheduler in `proc.c` file. We give 1 ticket to each process in `allocproc` function which can later be changed by `settickets` syscall. We then select a random ticket and select the process with that ticket. If there are other proecsses with the same number of tickets we choose the one with the lesser creation time `p->ctime`. We also implemented `settickets` syscall to change the number of tickets of a process.

**BASIC CHANGE IN PROCESS STRUCTURE -**
**_int p->tickets_**   initialized to 1 in `allocproc` function in `proc.c` file. Changed by `settickets` syscall.

1. Change in `proc.h` file to add `tickets` in `proc` structure.

2. Added a new system call `settickets` in `syscall.h` file as syscall number 26.

3. Added relevant function in and changes in `syscall.c` file.

4. Added the function prototype in `user.h` file.

5. Relevant changes in `usys.pl` file.

6. Makefile changed to accomodate SCHEDULER variable which sets SCHEDULER_LBS macro throughout all kernel files.

7. Scheduler uses a random number generator (xorshift) to select a ticket and then selects the process with that ticket. If there are multiple processes with the same number of tickets, we select the one with the lesser creation time `p->ctime`. Then we context switch to that process.

### 2.2 - Implementing Multi-Level Feedback Queue Scheduler (MLFQ)

Implemented the multi-level feedback queue scheduler in `proc.c` file. We make 4 queues with different time slices (1, 4, 8, 16) and priority levels (0, 1, 2, 3). We keep priority in process structure and change it according to the time spent in the queue. We also change the queue of the process if it uses the full time slice. We also implemented `priority boost` to boost the priority of all processes every 48 ticks in the queue.

**BASIC CHANGE IN PROCESS STRUCTURE -**
**_int p->priority_**   initialized to 0 in `allocproc` function in `proc.c` file. Changed in `trap.c` if priority is reduced or so.
**_int p->timeslice_**   array ininitalized to 0s in `allocproc` function in `proc.c` file. Changed in `trap.c` if time slice is used up.
**_int p->cpuburst_**   initialized to 0 in `allocproc` function in `proc.c` file. Changed in `trap.c` if time slice is used up.
**_int p->lastrun_**   initialized to ticks in `allocproc` function in `proc.c` file. Changed in `trap.c` if time slice is used up.


1. Change in `proc.h` file to add `priority`, `timeslice`, `cpuburst`, `lastrun` in `proc` structure.

2. MLFQ scheduler runs process in the highest queue with the least time slice. If the lowest queue, then we run processes in round robin fashion. We also boost the priority of all processes every 48 ticks.

3. usertrap and kerneltrap functions in `trap.c` file are modified to change the priority, timeslice, cpuburst and lastrun of the process. They also handle preemption of the process if a new process with higher priority is found.


### 2.3 - Testing the Schedulers
- What is the implication of adding the arrival time in the lottery based scheduling policy? Are there any pitfalls to watch out for? What happens if all processes have the same number of tickets?

  - The implication of adding arrival time to the lottery-based scheduling policy is that it introduces a tie-breaking mechanism for processes with equal ticket counts. This modification enhances fairness by giving priority to older processes when ticket counts are the same, preventing potential starvation of long-waiting processes. However, it also adds complexity to the scheduling algorithm and may introduce a slight bias towards older processes.

  - There are several pitfalls to watch out for in this implementation. First, the additional loop to check arrival times increases the computational overhead of the scheduling decision, which could impact performance in systems with many processes. Second, there's a risk of newer processes facing delays in getting selected, especially if the system is consistently at capacity and new processes always have the same ticket count as existing ones.

  - If all processes have the same number of tickets, the lottery aspect of the scheduling becomes insignificant, and the arrival time becomes the primary factor in scheduling decisions. In this scenario, the scheduler would behave similarly to a First-Come-First-Served (FCFS) scheduler, always picking the oldest runnable process among those that pass the initial random selection. This could lead to potential issues such as loss of load balancing properties, unfairness to newer processes, and the possibility of a convoy effect where a long-running CPU-bound process could delay many shorter processes.

- Performance Comparison

    | Scheduler             | Average Wait Time | Average Run Time |
    |:---------------------:|:-----------------:|:----------------:|
    | Default (Round Robin) | 151               | 12               |
    | Lottery Based         | 148               | 12               |
    | MLFQ                  | 131               | 12               |

![alt text](mlfq_timeline_graph.png)

# Networking


