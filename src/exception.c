/*
Copyright (c) 2015 Benderx2, 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of R3X nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <r3x_exception.h>
#include <r3x_stacktrace.h>
#include <r3x_format.h>
#include <r3x_disassemble.h>
#include <r3x_dispatcher.h>
#include <r3x_version.h>
#include <nt_malloc.h>

extern bool UseServer;
extern r3x_cpu_t* r3_cpu;

#define MAX_INPUT_LEN 80

char str[MAX_INPUT_LEN+1];
char str2[ MAX_INPUT_LEN+1];

void printstatus(void);
void printregstatus(void);
void debugger(void);
bool startsWith(const char *pre, const char *str);

void REX_EXCEPTION_HANDLER(int SIGNUM) {
	(void)SIGNUM;
	printf("Exception Detected! Debugger ONLINE.\n");
	debugger();
}
void debugger(void) { 
	printstatus();
	if(UseServer == true){
		printf("Virtual Machine is running in client mode. Cannot run debugger\n");
		printf("Quitting\n");
		exit(EXIT_FAILURE);
	}
	printf("Welcome to REX Debugger, based upon FVM Technology. %s.\nType 'help' for help.", R3X_SYSTEM_VERSION);
	// go into debugger 
	while(true) { 
		memset(str, 0, MAX_INPUT_LEN+1);
		unsigned int a1 = 0; unsigned int a2 = 0;
		(void)a1; (void)a2;
		printf("\n$ ");
		char* input = fgets(str, MAX_INPUT_LEN, stdin); // gcc warns for ignoring return values, my ass.
		if(strncmp(input, "help", 4) == 0) { 
			printf("REX Debugger -- help\n");
			printf("Commands: \n");
			printf("status --- Print information about program, CPU and Stack\n");	
			printf("regstatus -- Show register status\n");
			printf("switchdomain x -- Switches domains (processes)\n"); 
			printf("memprobe x -- prints a 32-bit integer @ x address in VM memory\n");
			printf("setip x -- Sets an Instruction Pointer\n");
			printf("setreg 1-20 x - Sets a register to value x\n");
			printf("push x -- Pushes value x to stack\n");
			printf("pop -- Pops from Stack\n");
			printf("continue -- Continues execution\n");
			printf("disasm IP size -- Dissassemble code at instruction pointer 'IP' and size 'size'\n");
			printf("readsym - Read program's symbol table\n");
			printf("mmap -- Prints memory map\n");
			printf("stacktrace n -- Displays n elements, starting from top of stack (Pass -scan-whole before 'n' in order to display stack from the very top.\n");
			printf("callstacktrace n -- Display n elements, starting from top of callstack (Pass -scan-whole before 'n' in order to display stack from the very top)\n");
			printf("quit -- Exits the VM and debugger\n");
		}
		else if(strncmp(input, "quit", 4) == 0) {
			exit(EXIT_SUCCESS);				
		}
		else if(strncmp(input, "memprobe", 8) == 0){ 
			memcpy(&str2, (&str[0] + 9),  MAX_INPUT_LEN-10);
			if(startsWith("0x", &str2[0])==true) {
				a1 = strtol(&str2[0], NULL, 16);
			} else {
				a1 = atoi(&str2[0]);
			}
			if(a1 > r3_cpu->MemorySize) { 
				printf("Cannot probe memory, invalid address\n");
			}	
			else {
				printf("Hex: %x\nDecimal: %u\nAt Address: %d", *(uint32_t*)(&r3_cpu->Memory[a1]), *(uint32_t*)(&r3_cpu->Memory[a1]), a1);	
				a1 = 0;		
			}
		} 
		else if(strncmp(input, "setip", 5) == 0) {
			memcpy(&str2, (&str[0] + 6), 70);
			if(startsWith("0x", &str2[0])==true) {
				a1 = strtol(&str2[0], NULL, 16);
			} else {
				a1 = atoi(&str2[0]);
			}
			if(a1 > r3_cpu->MemorySize) { 
				printf("Invalid Instruction Pointer");			
			}
			r3_cpu->InstructionPointer = a1;
		}
		else if(strncmp(input, "push", 4) == 0) {
			memcpy(&str2, (&str[0] + 5), 70);
			if(startsWith("0x", &str2[0])==true) {
				a1 = strtol(&str2[0], NULL, 16);
			} else {
				a1 = atoi(&str2[0]);
			}
			Stack.Push(r3_cpu->Stack, a1);
			printf("Top of Stack: %u\nStack Size: %u", r3_cpu->Stack->top_of_stack, r3_cpu->Stack->stack_count);	
		}
		else if(strncmp(input, "pop", 3) == 0) { 
			printf("Popped from Stack : %lu", (uint64_t)Stack.Pop(r3_cpu->Stack));		
		}
		else if(strncmp(str, "setreg", 6) == 0) { 
			char* token = strtok(input, " ");
			(void)token;
			a1 = 0; a2 = 0;
			a1 = atoi(strtok(NULL, " "));
			a2 = atoi(strtok(NULL, " "));	
			if(a1 > MAX_NUMBER_OF_REGISTERS) { 
				printf("Invalid Register\n");
			}
			else { 
				r3_cpu->Regs[a1] = a2;			
			}
		}
		else if(strncmp(input, "status", 6) == 0) { 
			printstatus();		
		}
		else if(strncmp(input, "switchdomain", 12) == 0) {
			a1 = 0;
			char* token = strtok(input, " "); 
			(void)token;
			int savedomainID = r3_cpu->RootDomain->CurrentJobID;
			a1 = atoi(strtok(NULL, " "));
			if(r3x_load_job_state(r3_cpu, r3_cpu->RootDomain, a1)==-1){
				printf("Error: Domain does not exist");
				r3_cpu->RootDomain->CurrentJobID = savedomainID;
				r3x_load_job_state(r3_cpu, r3_cpu->RootDomain, savedomainID);
			}
			else {
				r3_cpu->RootDomain->CurrentJobID = a1;
				printf("Switched to domain: %u", (unsigned int)r3_cpu->RootDomain->CurrentJobID);
			}
		}
		else if(strncmp(input, "regstatus", 9) == 0) { 
			printregstatus();
		}
		else if(strncmp(input, "continue", 8) == 0) {
			return;		
		} else if(strncmp(input, "disasm", 6)==0) {
			(void)strtok(input, " ");
			char* token1 = strtok(NULL, " ");
			char* token2 = strtok(NULL , " ");
			if(token1 == NULL || token2 == NULL) {
				printf("Error: expecting arguments");
			} else {
				if(startsWith("0x", token1)==true) {
					a1 = strtol(token1, NULL, 16);
				} else {
					a1 = atoi(token1);
				}
				if(startsWith("0x", token2)==true) {
					a2 = strtol(token2, NULL, 16);
				} else { 
					a2 = atoi(token2);
				}
				if(a1 + a2 >= r3_cpu->MemorySize) {
					printf("Invalid size/instruction pointer");
				} else {
					disassemble(&(r3_cpu->Memory[a1]), a2, stdout, "Disassembler Output: ", a1);
				}
			}
		} else if(strncmp(input, "readsym", 7)==0){
			read_symbol_table((r3x_header_t*)&r3_cpu->Memory[PROG_EXEC_POINT], r3_cpu->Memory, r3_cpu->MemorySize, r3_cpu->InstructionPointer);
		} else if(strncmp(input, "mmap", 4) == 0){
			DumpMemoryMap(r3_cpu->CPUMemoryBlocks);
		} else if(strncmp(input, "stacktrace", 10) == 0) {
			bool scan_whole = false;
			(void)strtok(input, " ");
			char* token1 = strtok(NULL, " ");
			if(!strcmp(token1, "-scan-whole")) {
				token1 = strtok(NULL, " ");
				scan_whole = true;
			}
			if(token1 == NULL) {
				printf("Error: Expected argument: Integer.\n");
			} else {
				unsigned int number_of_elements = atoi(token1);
				unsigned int current_stack_top = 0;
				if(scan_whole==true) {
					current_stack_top = r3_cpu->Stack->stack_count;
				} else {
					current_stack_top = r3_cpu->Stack->top_of_stack-1;
				}
				while(number_of_elements != 0 && current_stack_top != 0) {
						printf("[%u] %" PRIu64 "\n", current_stack_top, (uint64_t)Stack.GetItem(r3_cpu->Stack, current_stack_top));
						current_stack_top--;
						number_of_elements--;
					}
				}
		} else if(strncmp(input, "callstacktrace", 14) == 0) {
			bool scan_whole = false;
			(void)strtok(input, " ");
			char* token1 = strtok(NULL, " ");
			if(!strcmp(token1, "-scan-whole")) {
				token1 = strtok(NULL, " ");
				scan_whole = true;
			}
			if(token1 == NULL) {
				printf("Error: Expected argument: Integer.\n");
			} else {
				unsigned int number_of_elements = atoi(token1);
				unsigned int current_stack_top = 0;
				if(scan_whole==true) {
					current_stack_top = r3_cpu->CallStack->stack_count;
				} else {
					current_stack_top = r3_cpu->CallStack->top_of_stack-1;
				}
				while(number_of_elements != 0 && current_stack_top != 0) {
						printf("[%u] %" PRIu64 "\n", current_stack_top, (uint64_t)Stack.GetItem(r3_cpu->CallStack, current_stack_top));
						current_stack_top--;
						number_of_elements--;
					}
				}
			}
		else if(strncmp(input, "\n", 1)==0){
			
		}
		else { 
			printf("Invalid command. Type 'help' for help");		
		}
	}
}
void printstatus(void) {
	#ifdef R_DEBUG
	printf("Preparing Debugging Info.\nProgram backtrace:\n");
	print_backtrace();
	#endif
	assert(r3_cpu);
	assert(r3_cpu->Stack);
	assert(r3_cpu->CallStack);
	assert(r3_cpu->Memory);
	printf("Generating CPU report..\n");
	printf("|Stack Information| \n");
	printf("Stack (Address) : %u\n", (unsigned int)((intptr_t)r3_cpu->Stack));
	printf("Stack Size : %u\n", (unsigned int)r3_cpu->Stack->stack_count);
	printf("Top of Stack : %u\n", (unsigned int)r3_cpu->Stack->top_of_stack);
	printf("Last pushed value : %u\n", (unsigned int)Stack.GetItem(r3_cpu->Stack, r3_cpu->Stack->top_of_stack-1));
	printf("Last popped value : %u\n", (unsigned int)Stack.GetItem(r3_cpu->Stack, r3_cpu->Stack->top_of_stack));
	printf("|Call Stack Information|\n");
	printf("Call Stack (Address) : %u\n", (unsigned int)((intptr_t)r3_cpu->CallStack));
	printf("Last pushed Value : %u\n", (unsigned int)Stack.GetItem(r3_cpu->CallStack, r3_cpu->CallStack->top_of_stack-1));
	printf("Last popped Value : %u\n", (unsigned int)Stack.GetItem(r3_cpu->CallStack, r3_cpu->CallStack->top_of_stack-1));
	printf("|CPU Information|\n");
	printf("Memory (Address of Buffer): %p\n", (void*)r3_cpu->Memory);
	printf("Memory Size: %u\n", (unsigned int)r3_cpu->MemorySize);
	printf("Instruction Pointer: %u\n", (unsigned int)r3_cpu->InstructionPointer);
	printf("-->|CPU Registers|\n");
	printf("Type 'regstatus' for values for registers\n");
	printf("-->|CPU Flags|\n");
	printf("Equal: %s, Zero: %s, Greater: %s, Lesser: %s\n", r3_cpu->EqualFlag ? "true" : "false", r3_cpu->ZeroFlag ? "true" : "false", r3_cpu->GreaterFlag ? "true" : "false", r3_cpu->LesserFlag ? "true" : "false");
	printf("Current Domain ID: %u\n", r3_cpu->RootDomain->CurrentJobID);
	if(r3_cpu->InstructionPointer > r3_cpu->MemorySize) {
		printf("Exception: Instruction Pointer overflow\n");
	}
	else {
		printf("Last Instruction Executed [OPCODE] : 0x%x\n", r3_cpu->CurrentInstruction);	
	}
	printf("Log complete.\n");
}
void printregstatus(void) { 
	printf("Values for Current Domain:\n"); 
	for(unsigned int i = 0; i <= MAX_NUMBER_OF_REGISTERS; i++) {
		printf("R%u: %u\t", i, (unsigned int)r3_cpu->Regs[i]);	
	}
}
void SIGUSR1_handler(int signum){
	(void)signum;
	printf("Host requesting for status\n");
	printstatus();
	printregstatus();
}
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}
