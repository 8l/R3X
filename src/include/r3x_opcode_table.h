#ifndef __R3X_OPCODE_TABLE_H
#define __R3X_OPCODE_TABLE_H
#ifdef REX_OPTIMIZE
#include <r3x_opcodes.h>
static void* DispatchTable[] = {
	[R3X_SLEEP] = &&INTERP_SLEEP,
	[R3X_PUSH] = &&INTERP_PUSH,
	[R3X_POP] = &&INTERP_POP,
	[R3X_ADD] = &&INTERP_ADD,
	[R3X_SUB] = &&INTERP_SUB,
	[R3X_MUL] = &&INTERP_MUL,
	[R3X_DIV] = &&INTERP_DIV,
	[R3X_FADD] = &&INTERP_FADD,
	[R3X_FSUB] = &&INTERP_FSUB,
	[R3X_FMUL] = &&INTERP_FMUL,
	[R3X_FDIV] = &&INTERP_FDIV,
	[R3X_CMP] = &&INTERP_CMP,
	[R3X_JE] = &&INTERP_JE,
	[R3X_JL] = &&INTERP_JL,
	[R3X_JG] = &&INTERP_JG,
	[R3X_JZ] = &&INTERP_JZ,
	[R3X_JMP] = &&INTERP_JMP,
	[R3X_AND] = &&INTERP_AND,
	[R3X_OR] = &&INTERP_OR,
	[R3X_XOR] = &&INTERP_XOR,
	[R3X_DUP] = &&INTERP_DUP,
	[R3X_LOADS] = &&INTERP_LOADS,
	[R3X_LOAD] = &&INTERP_LOAD,
	[R3X_STORE] = &&INTERP_STORE,
	// 0x1A Reserved
	[R3X_EXIT] = &&INTERP_EXIT,
	[R3X_SYSCALL] = &&INTERP_SYSCALL,
	[R3X_LOADLIB] = &&INTERP_LOADLIB,
	[R3X_LIBEXEC] = &&INTERP_LIBEXEC,
	[R3X_CALL] = &&INTERP_CALL,
	[R3X_RET] = &&INTERP_RET,
	[R3X_PUSHA] = &&INTERP_PUSHA,
	[R3X_POPA] = &&INTERP_POPA,
	[R3X_MEMCPY] = &&INTERP_MEMCPY,
	[R3X_LODSB] = &&INTERP_LODSB,
	[R3X_LODSW] = &&INTERP_LODSW,
	[R3X_LODSD] = &&INTERP_LODSD,
	[R3X_STOSB] = &&INTERP_STOSB,
	[R3X_STOSD] = &&INTERP_STOSD,
	[R3X_STOSW] = &&INTERP_STOSW,
	[R3X_CMPSB] = &&INTERP_CMPSB,
	[R3X_CMPSW] = &&INTERP_CMPSW,
	[R3X_CMPSD] = &&INTERP_CMPSD,
	[R3X_LOADR] = &&INTERP_LOADR,
	[R3X_PUSHR] = &&INTERP_PUSHR,
	[R3X_POPR] = &&INTERP_POPR,
	[R3X_INCR] = &&INTERP_INCR,
	[R3X_DECR] = &&INTERP_DECR,
	[R3X_INT] = &&INTERP_INT,
	[R3X_LOADI] = &&INTERP_LOADI,
	[R3X_NOT] = &&INTERP_NOT,
	[R3X_NEG] = &&INTERP_NEG,
	[R3X_PUSHAR] = &&INTERP_PUSHAR,
	[R3X_POPAR] = &&INTERP_POPAR,
	[R3X_SHR] = &&INTERP_SHR,
	[R3X_SHL] = &&INTERP_SHL,
	[R3X_ROR] = &&INTERP_ROR,
	[R3X_ROL] = &&INTERP_ROL,
	[R3X_CALLDYNAMIC] = &&INTERP_CALLDYNAMIC,
	[R3X_FSIN] = &&INTERP_FSIN,
	[R3X_FCOS] = &&INTERP_FCOS,
	[R3X_FTAN] = &&INTERP_FTAN,
	[R3X_ASIN] = &&INTERP_ASIN,
	[R3X_ACOS] = &&INTERP_ACOS,
	[R3X_ATAN] = &&INTERP_ATAN,
	[R3X_FPOW] = &&INTERP_FPOW,
	[R3X_MOD] = &&INTERP_MOD,
	[R3X_FMOD] = &&INTERP_FMOD,
	[R3X_RCONV] = &&INTERP_RCONV,
	[R3X_ACONV] = &&INTERP_ACONV,
	[R3X_CMPS] = &&INTERP_CMPS,
	[R3X_POPN] = &&INTERP_POPN,
	[R3X_PUSHF] = &&INTERP_PUSHF,
	[R3X_POPF] = &&INTERP_POPF,
	[R3X_TERN] = &&INTERP_TERN,
	[R3X_CATCH] = &&INTERP_CATCH,
	[R3X_THROW] = &&INTERP_THROW,
	[R3X_HANDLE] = &&INTERP_HANDLE,
	[R3X_STORES] = &&INTERP_STORES,
	[R3X_LOADSR] = &&INTERP_LOADSR,
	[R3X_STORESR] = &&INTERP_STORESR,
	[R3X_SETE] = &&INTERP_SETE,
	[R3X_SETNE] = &&INTERP_SETNE,
	[R3X_SETG] = &&INTERP_SETG,
	[R3X_SETL] = &&INTERP_SETL,
	[R3X_FSINH] = &&INTERP_FSINH,
	[R3X_FCOSH] = &&INTERP_FCOSH,
	[R3X_FTANH] = &&INTERP_FTANH,
	[R3X_FABS] = &&INTERP_FABS,
	[R3X_FLOOR] = &&INTERP_FLOOR,
	[R3X_CEIL] = &&INTERP_CEIL,
	[R3X_ASINH] = &&INTERP_ASINH,
	[R3X_ACOSH] = &&INTERP_ACOSH,
	[R3X_ATANH] = &&INTERP_ATANH,
	[R3X_FCONV] = &&INTERP_FCONV,
	[R3X_ICONV] = &&INTERP_ICONV,
	[R3X_FSQRT] = &&INTERP_FSQRT,
	[R3X_JMPL] = &&INTERP_JMPL,
	[R3X_JEL] = &&INTERP_JEL,
	[R3X_JGL] = &&INTERP_JGL,
	[R3X_JLL] = &&INTERP_JLL,
	[R3X_JZL] = &&INTERP_JZL,
	[R3X_PUSHIP] = &&INTERP_PUSHIP,
	/**[ R3X_REX64_INST01 **/
	/**R3X_REX64_INS02**/
	[R3X_ARS] = &&INTERP_ARS,
	[R3X_BREAK] = &&INTERP_BREAK,
	[R3X_CALLL] = &&INTERP_CALLL,
	[RFC_PREFIX] = &&INTERP_RFC,
	[R3X_LOR] = &&INTERP_LOR,
	[R3X_LAND] = &&INTERP_LAND
};
#endif
#endif