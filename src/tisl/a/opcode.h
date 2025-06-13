//
// TISL/src/tisl/opcode.h
// TISL Ver. 4.x
//

#ifndef TISL_OPCODE_H
#define TISL_OPCODE_H

// register identifier
#define reg_EAX						0
#define reg_ECX						1
#define reg_EDX						2
#define reg_EBX						3
#define reg_ESP						4
#define reg_EBP						5
#define reg_ESI						6
#define reg_EDI						7

// 条件テストフィールド
#define tttn_O						 0
#define tttn_NO						 1
#define tttn_B						 2
#define tttn_NAE					 2
#define tttn_NB						 3
#define tttn_AE						 3
#define tttn_E						 4
#define tttn_Z						 4
#define tttn_NE						 5
#define tttn_NZ						 5
#define tttn_BE						 6
#define tttn_NA						 6
#define tttn_NBE					 7
#define tttn_A						 7
#define tttn_S						 8
#define tttn_NS						 9
#define tttn_P						10
#define tttn_PE						10
#define tttn_NP						11
#define tttn_PO						11
#define tttn_L						12
#define tttn_NGE					12
#define tttn_NL						13
#define tttn_GE						13
#define tttn_LE						14
#define tttn_NG						14
#define tttn_NLE					15
#define tttn_G						15

// opecode size[byte]
#define s_CALL_DIRECT				5
#define s_AND_REG_REG				2
#define s_JCC_8BIT_DISPLACEMENT		2
#define s_JCC_FULL_DISPLACEMENT		6
#define s_JMP_8BIT_DISPLACEMENT		2
#define s_JMP_FULL_DISPLACEMENT		5
#define s_NOP						1
#define s_POP_REG					1
#define s_PUSH_REG					1
#define s_PUSH_IMMEDIATE			5
#define s_RET						1

#define s_ADD_IMMEDIATE_EAX			5
#define s_CMP_IMMEDIATE_REG			6
#define s_SUB_IMMEDIATE_REG			6
#define s_MOV_PTR_REG_TO_REG		2
#define s_MOV_REG_TO_PTR_REG		2

// opcode

VM_RET opcode_and_reg_reg(tPVM vm, tPCELL function, tINT* pc, unsigned int reg_id_1, unsigned int reg_id_2);
VM_RET opcode_call_direct(tPVM vm, tPCELL function, tINT* pc, void* destination);
VM_RET opcode_jcc_8bit_displacement(tPVM vm, tPCELL function, tINT* pc, const int tttn, const char displacement);
VM_RET opcode_jcc_full_displacement(tPVM vm, tPCELL function, tINT* pc, const int tttn, void* displacement);
VM_RET opcode_jmp_8bit_displacement(tPVM vm, tPCELL function, tINT* pc, unsigned char displacement);
VM_RET opcode_jmp_full_displacement(tPVM vm, tPCELL function, tINT* pc, void* displacement);
VM_RET opcode_nop(tPVM vm, tPCELL function, tINT* pc);
VM_RET opcode_pop_reg(tPVM vm, tPCELL function, tINT* pc, unsigned int reg_id);
VM_RET opcode_push_reg(tPVM vm, tPCELL function, tINT* pc, unsigned int reg_id);
VM_RET opcode_push_immediate(tPVM vm, tPCELL function, tINT* pc, void* data);
VM_RET opcode_ret(tPVM vm, tPCELL function, tINT* pc);

VM_RET opcode_add_immediate_eax(tPVM vm, tPCELL function, tINT* pc, void* immediate);
VM_RET opcode_cmp_immediate_reg(tPVM vm, tPCELL function, tINT* pc, void* immediate, unsigned int reg_id);
VM_RET opcode_sub_immediate_reg(tPVM vm, tPCELL function, tINT* pc, void* immediate, unsigned int reg_id);

VM_RET opcode_mov_ptr_eax_to_ecx(tPVM vm, tPCELL function, tINT* pc);
VM_RET opcode_mov_ptr_ecx_to_edx(tPVM vm, tPCELL function, tINT* pc);
VM_RET opcode_mov_ecx_to_ptr_eax(tPVM vm, tPCELL function, tINT* pc);

#endif

