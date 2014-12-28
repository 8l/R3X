// REX Virtual Dynamic Linker
// Note: This is different from the native dynamic linker, it is meant to dynamically link 2 REX bytecode.
#include <r3x_dynamic.h>
libimport_struct** lbstructs;
unsigned int total_number_of_structs = 16;
unsigned int number_of_used_structs = 0;
int load_lib_manager(void) { 
	lbstructs = nt_malloc(sizeof(libimport_struct*)*16);
	return 0;
}
int load_dynamic_library(char* name, r3x_cpu_t* CPU) {
	uint32_t loadaddr = CPU->MemorySize;
	FILE* dynamic_lib_file = fopen(name, "r");
	if(dynamic_lib_file == NULL) {	
		printf("Unable to load dynamic object file: %s. ABORTING..\n", name);
		exit(0);
	}
	fseek(dynamic_lib_file, 0L, SEEK_END);
	unsigned int totalsize = ftell(dynamic_lib_file);
	fseek(dynamic_lib_file, 0L, SEEK_SET);
	uint8_t* temp = nt_malloc(totalsize * sizeof(uint8_t));
	unsigned int sizeread = fread(temp, sizeof(uint8_t), totalsize, dynamic_lib_file);
	if(sizeread != totalsize) { 
		printf("Unable to read dynamic object file properly. ABORTING..\n");
		exit(0);
	}
	r3x_dynamic_header_t* dyn_header = (r3x_dynamic_header_t*)&(temp[0]);
	if(dyn_header->header != REX_DYNAMIC_HEADER_VAL) {
		printf("Invalid dynamic object file. ABORTING..\n");
		exit(0);
	}
	// Now modify the library since it assumes no loadpoint...
	uint32_t i = dyn_header->text_section;
	while(i <= dyn_header->text_section + dyn_header->text_size) { 
		switch(temp[i]) {
			case R3X_CALL:
			case R3X_JMP:
			case R3X_JE:
			case R3X_JL:
			case R3X_JG:	
			case R3X_JZ:
				i++;
				*((uint32_t*)&temp[i]) = CPU->MemorySize + *((uint32_t*)&temp[i]);
				i += 4;
				break;
			case R3X_STOSD:
				temp[i] = R3X_STOSD_RELOC;
				i++;
				*((uint32_t*)&temp[i]) = CPU->MemorySize + *((uint32_t*)&temp[i]);
				i += 4;
				break;
			case R3X_LODSD:
				temp[i] = R3X_LODSD_RELOC;
				i++;
				*((uint32_t*)&temp[i]) = CPU->MemorySize + *((uint32_t*)&temp[i]);
				i += 4;
				break;
			case R3X_STOSB:
				temp[i] = R3X_STOSB_RELOC;
				i++;
				*((uint32_t*)&temp[i]) = CPU->MemorySize + *((uint32_t*)&temp[i]);
				i += 4;
				break;
			case R3X_LODSB:
				temp[i] = R3X_LODSB_RELOC;
				i++;
				*((uint32_t*)&temp[i]) = CPU->MemorySize + *((uint32_t*)&temp[i]);
				i += 4;
				break;
			case R3X_PUSH_RELOC:
				temp[i] = R3X_PUSH;
				i++;
				*((uint32_t*)&temp[i]) = CPU->MemorySize + *((uint32_t*)&temp[i]);
				i += 4;
				break;	
			case R3X_SLEEP:
			case R3X_POP:		
			case R3X_POPA:
			case R3X_ADD:
			case R3X_SUB:
			case R3X_DIV:
			case R3X_MUL:
			case R3X_FADD:
			case R3X_FSUB:
			case R3X_FDIV:
			case R3X_FMUL:
			case R3X_CMP:
			case R3X_DUP:
			case R3X_LOADLIB:
			case R3X_LIBEXEC:
			case R3X_MEMCPY:
			case R3X_RET:
			case R3X_NOT:
			case R3X_NEG:
			case R3X_SHR:
			case R3X_SHL:
			case R3X_EXIT:
				i++;
				break;
			case R3X_PUSH:
			case R3X_AND:
			case R3X_XOR:
			case R3X_OR:
			case R3X_LOADS:
			case R3X_PUSHA:
				i += 5;
				break;
			case R3X_LOADI:
			case R3X_LOADR:
				i += 6;
				break;
			case R3X_PUSHR:
			case R3X_POPR:
			case R3X_PUSHAR:
			case R3X_POPAR:
			case R3X_INT:
			case R3X_SYSCALL:
			case R3X_INCR:
			case R3X_DECR:
			      i += 2;
			      break;
			default:
				i++;
				break;
		}
	}
	if(number_of_used_structs >= total_number_of_structs) { 
		lbstructs = nt_realloc(lbstructs, sizeof(libimport_struct*)*(total_number_of_structs+16));
		total_number_of_structs += 16;
	}
	CPU->Memory = nt_realloc(CPU->Memory, CPU->MemorySize+totalsize);
	memcpy(&CPU->Memory[CPU->MemorySize], temp, totalsize);
	nt_free(temp);
	for(unsigned int i = 0; i < total_number_of_structs; i++) { 
		if(lbstructs[i] == NULL)  {
			lbstructs[i] = nt_malloc(sizeof(libimport_struct));
			lbstructs[i]->loadaddr = loadaddr;
			dyn_header = (r3x_dynamic_header_t*)&CPU->Memory[loadaddr];
			lbstructs[i]->libid = i;
			lbstructs[i]->function_count = dyn_header->export_size / sizeof(export_struct);
			lbstructs[i]->functions = (export_struct*)&CPU->Memory[loadaddr + dyn_header->export_section];
			printf("first function: %u\n", lbstructs[i]->functions[1].instruction_pointer);
			CPU->MemorySize += totalsize;
			return i;
		}
	}
	return -1;
}
uint32_t dynamic_call(unsigned int libhandle, unsigned int functionhandle) { 
	if(total_number_of_structs <= libhandle) {
		return 0;
	} else {
		if(lbstructs[libhandle] != NULL) {
			 if(functionhandle <= lbstructs[libhandle]->function_count) { 
				return lbstructs[libhandle]->loadaddr + lbstructs[libhandle]->functions[functionhandle].instruction_pointer;
			 }
		}
	}
	return 0;
}
	
