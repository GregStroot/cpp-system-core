# Reading List

* Goal: The OS is the enemy of latency. We want to keep it out
    - Understand and remember details regarding CPU and memory virtualisation

- [x] OSTEP Ch 2: The overview
- [ ] OSTEP Ch 4: The abstraction
- [ ] OSTEP Ch 6: Context switching
- [ ] OSTEP Ch 19: TLB

# Chapter 4: The Abstraction: The Process

* **Context Switch** gives the OS the ability to stop running one program and start running another on a given CPU
    * This time-sharing is used by all moderns CPUs

* On top of the time-sharing policies, a **scheduling policy** in the OS will decide what program gets priority

## 4.1: A Process

* The OS abstracts a running program into a **process**
    - To understand what constitutes it, we have to understand its **machine state**
        * E.g. What a program can read or update

* One component of the machine state is its memory
    - The part of memory that the process can address (called its **address space**) is part of the proces

* Further, the part of the process's machine state are *registers*
    - **program counter** (PC) tells us which instruction of the program will execute next
    - The **stack pointer** and associated **frame pointer** are used to manage the stack for:
        - function parameters,
        - local vars and,
        - return addresses

* Finally, programs often assess persistent storage devices
    - Such I/O information mich include list of the files the process currently has open etc.

## 4.2-6:  Overview

* Pretty basic overview of abstractions needed in the OS

# Chapter 6: Mechanism: Limited Direct Execution

## 6.1: Basic Technique: Limited Direct Execution

* The most basic idea is **direct execution** (DE) is to: just run the program directly on the CPU
    - This quickly gives rise to a few problems:
        1. If we just run the program, how can the OS make sure the program doesn't do anything that we don't want it to do, **while still running efficiently**?
        2. When we are running a process, how does the OS stop it from running and switch to another, thus implementing **time sharing** we require for virtualisation of the CPU?

## 6.2: Problem #1: Restricted Operations

* A DE is fast, but what if the process wishes to perform some kind of restricted operation, such as issuing an I/O request or gaining access to more system resources?

* Thus we introduce **user mode**
    - Code run in user mode is restricted in what it can do
        * E.g. cannot issue I/O requests; doing so would result in the processor raising an exception

* In construct to user mode is **kernel mode**
    - In this mode, code can do what it likes

* So then, what should a user process do when it wishes to perform some kind of privileged operation?
    - To enable this, all modern hardware provides the ability for user programs to perform a **systems call**
        * System calls allow the kernel to expose certain key pieces of functionality to user programs

* To execute a system call, a program must execute a special **trap** instruction
    - This instruction simultaneously jumps into the kernel and raises level to kernel mode
    - When finished, the OS calls a special **return-from-trap** instruction

* The hardware needs to be careful when executing a trap, in that it *must make sure to save enough of the caller's registers* to be able to return properly
    - On x86, for example, the processor will push the program counter, flags, and a few other registers onto a per-process **kernel stack**
        * The *return-from-trap* will pop these values off the stack and resume execution of the user-mode program

* We are missing an important detail: how does the trap know which code to run inside the OS?
    - Clearly the calling program cannot specify an address -- this is a security nightmare!
    - The kernel must control what code executes upon a trap

* The kernel does so by setting up a **trap table** at boot time
    - When the machine boots up, it does so in kernel mode and thus can configure machine hardware
    - During this phase, the OS tells the hardware what code to run when certain exceptional events occur
    - The OS informs the hardware of the location of these **trap handlers**, usually with some kind of special instruction
          * Once informed, it remembers these until next reboot

* **Figure 6.2: Limited Direct Execution Protocol** (Very good!)
    - Before running a program, the OS (in kernel mode):
        - Creates entry for process lists
        - Allocates memory for program
        - Load program into memory
        - Setup user stack with argv
        - Fill kernel stack with reg/PC
        - **return-from-trap**
            * Fundamental way to move from kernel-to-user!
    - The kernel then:
        - Restores registers (from kernel stack)
        - Move to user mode
        - Jump to main
    - At the end of the program, the program calls trap (via exit())
    - The kernel then:
        - Frees memory of process
        - Removes from process list

## 6.3: Problem #2: Switching Between Processes

* **Key Question**:
    * How can the OS **regain control** of the CPU so that it can switch between processes?

### A cooperative approach: Wait for system calls

* Early version of Mac or Xerox systems used a **cooperative** approach
    - Processes that run for too long are *assumed* to periodically give up the CPU so that the OS can decide to run some other task

* How does a friendly process give up the CPU in this utopian world?
    - Most process transfer control of the CPU to the OS quite frequently by making **system calls**
    - Systems like this often include an explicit **yield** system call, which just transfers control

* This approach obviously has issues if a process ends up in an infinite loop (whether malicious, or just full of bugs)
    - The only thing you can do is: reboot the machine!

### A non-cooperative approach: The OS Takes Control

* **Key Question**:
    * How can the OS gain control of the CPU even if processes are not being cooperative?
    * What can the OS do to ensure a rogue process does not take over the machine?

* The answer turns out to be simple: a **timer interrupt**
    - A timer device can be programmed to raise an interrupt every so many milliseconds
    - When the interrupt is raised, the current program is halted, and a (pre-configured) **interrupt handler** in the OS runs

* During the boot sequence, the OS must start the timer, which is (of course) a priveleged operation)
    - Once the timer has begun, the OS can feel safe that control will eventually be returned to it
    - The timer can also be turned off (privileged op)

* As with the trap instruction, the timer interrupt must also save enough of the state of hte program that was running when the interrupt occurred such that a subsequent return-from-trap instruction will be able to resume the running program correctly



### Saving and Restoring Context

* Now that the OS has regained control, a decision must be made: whether to continue running the currently-running process, or switch to a different one
    - This decision is made by the **scheduler**
        * Discussed in detail in the coming chapters

* If the decision is made to switch, the OS then executes a low-level piece of code known as a **context switch**

* A context is conceptually simple:
    * the OS has to save a few register values for the currently-executing process (onto its kernel stack)
    * Restore a few registers for the soon-to-be-executing process (from its kernel stack)

* Figure 6.3 shows the flow for a context switch
    * Note that there are two types of register saves/restores that happen during this protocol:
        * The timer interrupt occurs; then The *user registers* of the running process are implicitly saved by the *hardware*, using the kernel stack of that process
        * The second is when the OS decides to switch from A to B:
            * The *kernel registers* are explicitly saved by the software (the OS), but this time into memory in the process structure of the process
            *

## 6.4: Worried About Concurrency?

* **Key question**:
    * What happens when, during a system call, a timer interrupt occurs?
    * What happens when you're handling one interrupt and another one happens?

* This will be covered later

* One idea though is that OSs haave developed a number of sophisticated **locking** schemes to protect concurrent access to internal data structures
    - This enables multiple activities to be on-going within the kernel at the same time

# Chapter 19: Paging: Faster Translations (TLBs)

*

## 19.1 TLB Basic Algorithm

## 19.2 Accessing an array
## 19.3 Who Handles the TLB Miss?
## 19.4 TLB Contents: What's in There?
## 19.5 TLB Issue: Context Switches
## 19.6 Issue: Replacement Policy
## 19.7 A Real TLB Entry
## 19.8 Summary



# Effective Modern C++
## Item 40: Use `std::atomic` for concurrency, `volatile` for special memory

* `volatile` provides no guarentee of operation atomicity and insufficient restrictions on code reordering

* `volatile` is the way we tell compilers that we're dealing with special memory
    - It tells the compiler "Don't perform any optimizations on operations on this memory"

* So if `x` corresponds to special memory, it'd be declared volatile:
    * `volatile int x;`
* Consider the effect on this code sequence:
    ```
    auto y = x; // read x
    y = x;      // read x again (can't be optimised away)

    x = 10;     // write x (can't be optimized away -- started as 10)
    x = 20;     // write x again
    ```
    - This is precisely what we want if `x` is memory-mapped (or has been mapped to a memory location shared across processes)
