#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

// loop for approximately totalNumInstructions instructions
void loop(unsigned int totalNumInstructions) {
  unsigned int count = totalNumInstructions >> 1; // the loop executes 2 instructions
  asm volatile(
      "1:\n" // define local label
      "decl %%eax\n" // eax -= 1
      "jnz 1b\n" // jump to previous local label 1 if not zero
      : // no output regs
      : "a"(count)
      );
}


void run_target() {
  printf("run target\n");
  
  // allow tracing this process
  if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
    printf("ptrace error\n");
    perror("ptrace");
    return;
  }
  
  // to trace arbitrary program, use execl
  // but we just use trap instruction to start ptrace...
  asm("int3\n" : : );
  
  // ...to execute the loop within the same program
  loop(1*1000*1000);
}

void run_debugger(pid_t child_pid) {
  int wait_status;
  long long int num_instructions = 0;
  printf("debugger started\n");

  // wait for child to stop in its first instruction
  wait(&wait_status);
  
  while (WIFSTOPPED(wait_status)) {
    num_instructions++;
    // make child execute another instruction
    if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
      perror("ptrace");
      return;
    }
    
    if (num_instructions % 100000 == 0) {
      printf("cycle: %lld\n", num_instructions);
    }

    // wait for child to stop in its next instruction
    wait(&wait_status);
  }
  
  printf("the child executed %lld instructions\n", num_instructions);
}

int main(int arc, char* argv[]) {
  pid_t  child_pid;

  child_pid = fork();
  if (child_pid == 0) {
    run_target();
  } else if (child_pid > 0) {
    run_debugger(child_pid);
  } else {
    perror("fork");
    return -1;
  }
  
  return 0;
}

