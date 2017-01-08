# trapflag-trace-vs-ptrace-learn
exploring the use of ptrace vs the x86 trapflag to step through every instruction of a program.

there are three small programs executing a certain number of instructions:

* count-ptrace.c uses ptrace to step through every instruction, executing ~1e6 instructions
* count-trapflag uses the x86 trap flag and signal handlers to execute ~1e7 instructions
* loop.c executes ~1e9 instructions without any instrumentation

These are the execution times on my netbook (32-bit Intel Atom N450, 1.66GHz):

*  1e6 instructinos in 61.7s in count-ptrace.c (16.2KHz)
*  1e7 instructions in 38.1s in count-trapflag.c (262.5KHz)
*  1e9 instructions in 0.61s in loop.c (1,639MHz)

This implies the following slow-downs of the different instrumentation schemes:

* `101,172x` slower with ptrace
* `  6,244x` slower with trapflag and x86

This implies using the trapflag+signal handlers is 16x faster than using ptrace.
