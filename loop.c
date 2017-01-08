// a program that will execute a approximately a certain number of instructions

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
  loop(1000*1000*1000u);
}
