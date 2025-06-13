//
// TISL/src/tisl/a/opcode.c
// TISL Ver. 4.x
//

#include "../../../include/tni.h"
#include "../object.h"
#include "../vm.h"
#include "../tisl.h"
#include "opcode.h"

extern VM_RET function_write_code(tPVM vm, tPCELL function, const tINT pc, unsigned char c);
extern VM_RET function_write_code_uint(tPVM vm, tPCELL function, const tINT pc, unsigned int i);

#define AND_REG_REG		0x20
#define CALL_DIRECT		0xe8
#define JCC_8			0x70
#define JCC_FULL_1		0x0f
#define JCC_FULL_2		0x80
#define JMP_8			0xeb
#define JMP_FULL		0xe9
#define NOP				0x90
#define POP_REG			0x58
#define PUSH_REG		0x50
#define PUSH_IMMEDIATE	0x68
#define RET				0xc3

#define ADD_IMMEDIATE_EAX	0x05

VM_RET opcode_and_reg_reg(tPVM vm, tPCELL function, tINT* pc, unsigned int reg_id_1, unsigned int reg_id_2)
{
	// AND Logical AND
	// register1 to register2
	// 0010 000w : 11 reg1 reg2
	if (function_write_code(vm, function, *pc, AND_REG_REG|1)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, (unsigned char)(0xc0|(reg_id_1<<8)|(reg_id_2)))) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_call_direct(tPVM vm, tPCELL function, tINT* pc, void* destination)
{
	unsigned int displacement;
	// CALL - Call Procedure (in same segument)
	// direct
	// 1110 1000 : full displacement
	displacement=(unsigned int)destination-(unsigned int)(function_get_code_head(function)+*pc+5);

	if (function_write_code(vm, function, *pc, CALL_DIRECT)) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, displacement)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_jcc_8bit_displacement(tPVM vm, tPCELL function, tINT* pc, const int tttn, const char displacement)
{
	// Jcc - Jump if Condition Is Met
	//  8-bit displacement
	//   0111 tttn : 8-bit displacement
	if (function_write_code(vm, function, *pc, (unsigned char)(JCC_8|tttn))) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, (unsigned char)displacement/**/)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_jcc_full_displacement(tPVM vm, tPCELL function, tINT* pc, const int tttn, void* displacement)
{
	// Jcc - Jump If Condition Is Met
	//  full displacement
	//  0000 1111 1000 tttn full displacement
	if (function_write_code(vm, function, *pc, JCC_FULL_1)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, (unsigned char)(JCC_FULL_2|tttn))) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, (unsigned int)displacement)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_jmp_8bit_displacement(tPVM vm, tPCELL function, tINT* pc, unsigned char displacement)
{
	// JMP Unconditional Jump (to same segment)
	// short 8bit displacement
	// 1110 1011 8bit displacement
	if (function_write_code(vm, function, *pc, JMP_8)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, displacement)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_jmp_full_displacement(tPVM vm, tPCELL function, tINT* pc, void* displacement)
{
	// JMP Unconditional Jump (to same segment)
	// direct
	// 1110 1001 full displacement
	if (function_write_code(vm, function, *pc, JMP_FULL)) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, (unsigned int)displacement)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_nop(tPVM vm, tPCELL function, tINT* pc)
{
	// NOP No Operation
	//  1001 0000
	if (function_write_code(vm, function, *pc, NOP)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_pop_reg(tPVM vm, tPCELL function, tINT* pc, unsigned int reg_id)
{
	// POP Pop a Word from the Stack
	// register
	// 短いほうにしとく
	// 0101 1 reg
	if (function_write_code(vm, function, *pc, (unsigned char)(POP_REG|reg_id))) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_push_reg(tPVM vm, tPCELL function, tINT* pc, unsigned int reg_id)
{
	// PUSH Push Operand onto the Stack
	// resister
	// 短いほうにしておく
	// 0101 0 reg
	if (function_write_code(vm, function, *pc, (unsigned char)(PUSH_REG|reg_id))) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_push_immediate(tPVM vm, tPCELL function, tINT* pc, void* data)
{
	// PUSH Push Operand onto the Stack
	//  immediate
	//  0110 10s0 immediate data
	if (function_write_code(vm, function, *pc, (unsigned int)(PUSH_IMMEDIATE))) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, (unsigned int)data)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_ret(tPVM vm, tPCELL function, tINT* pc)
{
	if (function_write_code(vm, function, *pc, RET)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_add_immediate_eax(tPVM vm, tPCELL function, tINT* pc, void* immediate)
{
	if (function_write_code(vm, function, *pc, ADD_IMMEDIATE_EAX)) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, (unsigned int)immediate)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_cmp_immediate_reg(tPVM vm, tPCELL function, tINT* pc, void* immediate, unsigned int reg_id)
{
	if (function_write_code(vm, function, *pc, 0x81)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, (unsigned char)(0xf8|reg_id))) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, (unsigned int)immediate)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_sub_immediate_reg(tPVM vm, tPCELL function, tINT* pc, void* immediate, unsigned int reg_id)
{
	if (function_write_code(vm, function, *pc, 0x81)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, (unsigned char)(0xe8|reg_id))) return VM_ERROR;
	++*pc;
	if (function_write_code_uint(vm, function, *pc, (unsigned int)immediate)) return VM_ERROR;
	*pc+=4;
	return VM_OK;
}

VM_RET opcode_mov_ptr_eax_to_ecx(tPVM vm, tPCELL function, tINT* pc)
{
	if (function_write_code(vm, function, *pc, 0x8b)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, 0x08)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_mov_ptr_ecx_to_edx(tPVM vm, tPCELL function, tINT* pc)
{
	if (function_write_code(vm, function, *pc, 0x8b)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, 0x11)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

VM_RET opcode_mov_ecx_to_ptr_eax(tPVM vm, tPCELL function, tINT* pc)
{
	if (function_write_code(vm, function, *pc, 0x89)) return VM_ERROR;
	++*pc;
	if (function_write_code(vm, function, *pc, 0x08)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

