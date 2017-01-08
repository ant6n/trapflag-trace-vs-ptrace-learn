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


long long int ccycle = 0;
/** trap handler executes for every instruction -- simply counts cycles */
void trapHandler(int signo, siginfo_t *info, void *context) {
  ccycle++;
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

  printf("cycles: %lld\n", ccycle);
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
  // don't trace if the first cmd line arg starts with 'n'
  int traceIt = argc > 1 ? argv[1][0] != 'n' : 1;

  // some computation
  if (traceIt) startTrace();
  loop(1000*1000*1000);
  if (traceIt) stopTrace();
  
  // some syscalls (they have caused problems in the past)
  if (traceIt) startTrace();
  printf("rock\n");
  printf("paper\n");
  printf("scissors\n");
  if (traceIt) stopTrace();
}


/*
//with debugging
cycles: 100001991
real12m51.636s
user1m42.320s
sys11m6.940s

//without debugging
real0m0.071s
user0m0.068s
sys0m0.004s

*/
