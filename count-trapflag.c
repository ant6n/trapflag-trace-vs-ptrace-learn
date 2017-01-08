// gnu source is required to use STDOUT_FILENO
#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/** "Debugger" ****************************************************************/
void setTrapFlag() {
  asm volatile("pushfl\n"
               "orl $0x100, (%esp)\n"
               "popfl\n"
               );
}

void clearTrapFlag() {
  asm volatile("pushfl\n"
               "andl $0xfffffeff, (%esp)\n"
               "popfl\n"
               );
}


static long long int num_instructions = 0;
/** trap handler executes for every instruction -- simply counts cycles */
void trapHandler(int signo, siginfo_t *info, void *context) {
  num_instructions++;
}


struct sigaction trapSa;
void startTrace() {
  // set up trap signal handler
  trapSa.sa_flags = SA_SIGINFO;
  trapSa.sa_sigaction = trapHandler;
  sigaction(SIGTRAP, &trapSa, NULL);

  // set trap flag
  setTrapFlag();
}


void stopTrace() {
  clearTrapFlag();

  printf("num instructions: %lld\n", num_instructions);
}



/** "Debuggee" ****************************************************************/
// print on screen using explicit syscall (int 0x80)
void writeSyscall(int fileHandle, char* s) {
  int len = 0;
  while (s[len++]);
  asm(
      "int  $0x80\n"   // sycall
      : // no output regs
      : "a"(4), "b"(fileHandle),"c"(s),"d"(len) // inputs
      );
}


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


int main(int argc, const char* argv[]) {
  startTrace();
  loop(10*1000*1000);
  stopTrace();
}


