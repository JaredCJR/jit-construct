/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "jit-x64_optimize.dasc".
*/

#line 1 "jit-x64_optimize.dasc"
#include <stdint.h>
#include "util.h"

//|.arch x64
#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif
#line 5 "jit-x64_optimize.dasc"
//|.actionlist actions
static const unsigned char actions[80] = {
  81,83,72,137,252,251,255,72,129,195,239,255,72,129,252,235,239,255,128,3,
  235,255,128,43,235,255,15,182,59,72,184,237,237,252,255,208,255,72,184,237,
  237,252,255,208,136,3,255,198,3,0,255,138,11,0,75,1,198,3,0,255,128,59,0,
  15,132,245,249,255,128,59,0,15,133,245,249,255,91,89,195,255
};

#line 6 "jit-x64_optimize.dasc"
//|
//|// Use rbx as our cell pointer.
//|// Since rbx is a callee-save register, it will be preserved
//|// across our calls to getchar and putchar.
//|.define PTR, rbx
//|.define temp, rcx
//|
//|// Macro for calling a function.
//|// In cases where our target is <=2**32 away we can use
//|//   | call &addr
//|// But since we don't know if it will be, we use this safe
//|// sequence instead.
//|.macro callp, addr
//|  mov64  rax, (uintptr_t)addr
//|  call   rax
//|.endmacro

#define Dst &state
#define MAX_NESTING 256

int main(int argc, char *argv[])
{
	if (argc < 2) err("Usage: jit-x64 <inputfile>");
	dasm_State *state;
	initjit(&state, actions);

	unsigned int maxpc = 0;
	int pcstack[MAX_NESTING];
	int *top = pcstack, *limit = pcstack + MAX_NESTING;
	int sequence_count = 0;

	// Function prologue.
	//|  push temp
	//|  push PTR
	//|  mov  PTR, rdi      // rdi store 1st argument
	dasm_put(Dst, 0);
#line 41 "jit-x64_optimize.dasc"

	for (char *p = read_file(argv[1]); *p; p++) {
		switch (*p) {
		case '>':
			sequence_count = 1;
			while( *(++p) == '>' ){
				sequence_count++;
			}
			p--;
			//|  add  PTR,sequence_count
			dasm_put(Dst, 7, sequence_count);
#line 51 "jit-x64_optimize.dasc"
			break;
		case '<':
			sequence_count = 1;
			while( *(++p) == '<' ){
				sequence_count++;
			}
			p--;
			//|  sub  PTR,sequence_count
			dasm_put(Dst, 12, sequence_count);
#line 59 "jit-x64_optimize.dasc"
			break;
		case '+':
			sequence_count = 1;
			while( *(++p) == '+' ){
				sequence_count++;
			}
			p--;
			//|  add  byte [PTR], sequence_count
			dasm_put(Dst, 18, sequence_count);
#line 67 "jit-x64_optimize.dasc"
			break;
		case '-':
			sequence_count = 1;
			while( *(++p) == '-' ){
				sequence_count++;
			}
			p--;
			//|  sub  byte [PTR], sequence_count
			dasm_put(Dst, 22, sequence_count);
#line 75 "jit-x64_optimize.dasc"
			break;
		case '.':
			//|  movzx edi, byte [PTR]
			//|  callp putchar
			dasm_put(Dst, 26, (unsigned int)((uintptr_t)putchar), (unsigned int)(((uintptr_t)putchar)>>32));
#line 79 "jit-x64_optimize.dasc"
			break;
		case ',':
			//|  callp getchar
			//|  mov   byte [PTR], al
			dasm_put(Dst, 37, (unsigned int)((uintptr_t)getchar), (unsigned int)(((uintptr_t)getchar)>>32));
#line 83 "jit-x64_optimize.dasc"
			break;
		case '[':
			if (top == limit) err("Nesting too deep.");
			// Each loop gets two pclabels: at the beginning and end.
			// We store pclabel offsets in a stack to link the loop
			// begin and end together.

			if( *(p+1)=='-' && *(p+2)==']' ){
				p+=2;
				//| mov   byte [PTR], 0
				dasm_put(Dst, 47);
#line 93 "jit-x64_optimize.dasc"
				break;
			}

			if( *(p+1)=='>' && *(p+2)=='+' && *(p+3)=='<' && *(p+4)=='-' && *(p+5)==']' ){
				p+=5;
				//| mov   cl , byte [PTR]
				//| add   byte [PTR+1], cl
				//| mov   byte [PTR], 0
				dasm_put(Dst, 51);
#line 101 "jit-x64_optimize.dasc"
				break;
			}

			maxpc += 2;
			*top++ = maxpc;
			dasm_growpc(&state, maxpc);
			//|  cmp  byte [PTR], 0
			//|  je   =>(maxpc-2)
			//|=>(maxpc-1):
			dasm_put(Dst, 60, (maxpc-2), (maxpc-1));
#line 110 "jit-x64_optimize.dasc"
			break;
		case ']':
			if (top == pcstack) err("Unmatched ']'");
			top--;
			//|  cmp  byte [PTR], 0
			//|  jne  =>(*top-1)
			//|=>(*top-2):
			dasm_put(Dst, 68, (*top-1), (*top-2));
#line 117 "jit-x64_optimize.dasc"
			break;
		}
	}

	// Function epilogue.
	//|  pop  PTR
	//|  pop  temp
	//|  ret
	dasm_put(Dst, 76);
#line 125 "jit-x64_optimize.dasc"

	void (*fptr)(char*) = jitcode(&state);
	char *mem = calloc(30000, 1);
	fptr(mem);
	free(mem);
	free_jitcode(fptr);
	return 0;
}
