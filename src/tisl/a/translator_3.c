//
// TISL/src/tisl/a/translator.c
// TISL Ver. 4.x
//

#define TISL_TRANSLATOR_3_C
#include "../../../include/tni.h"
#include "../object.h"
#include "../vm.h"
#include "../tisl.h"
#include "../translator.h"
#include "opcode.h"
#include "../command.h"
#include "../operation.h"

///////////////////

extern tTRANSLATOR vm_get_translator(tPVM vm);
extern VM_RET function_write_code(tPVM vm, tPCELL function, const tINT pc, unsigned char c);
extern VM_RET function_write_code_uint(tPVM vm, tPCELL function, const tINT pc, unsigned int i);

///////////////////
// dummy unknown operation?
static VM_RET t3_get_size_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_write_code_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiDISACRD
static VM_RET t3_s_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCODE
static VM_RET t3_s_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiPUSH_OBJECT obj
static VM_RET t3_s_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCODE operand
static VM_RET t3_s_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
static VM_RET t3_w_type_2_(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCODE operand1 operand2
static VM_RET t3_s_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCALL_TAIL_REC
static VM_RET t3_s_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCALL_TAIL
static VM_RET t3_s_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCALL_LOCAL offset flist
static VM_RET t3_s_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCALL_LOCAL_TAIL offset flist
static VM_RET t3_s_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiRET
static VM_RET t3_s_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiPUSH_LOCAL_FUNCTON offset flist
static VM_RET t3_s_push_lfunction(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_push_lfunction(tPVM vm, tPCELL Clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiPUSH_LAMBDA flist
static VM_RET t3_s_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiLABELS_IN nlist function-list-1 ... function-list-n
// iiFLET_IN nlist function-list-1 ... function-list-n
static VM_RET t3_s_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiAND
static VM_RET t3_s_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiDYNAMIC_LET
static VM_RET t3_s_dynamic_let(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_dynamic_let(tPVM vm, tPCELL clist, const tINT ocde, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiIF
static VM_RET t3_s_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCASE
static VM_RET t3_s_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCASE_USING
static VM_RET t3_s_case_using(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_case_using(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiWHILE clist clist
static VM_RET t3_s_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiFOR plist enttest result iteration
static VM_RET t3_s_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiBLOCK clist tag
static VM_RET t3_s_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiRETURN_FROM tag
static VM_RET t3_s_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCATCH clist
static VM_RET t3_s_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiTHROW
static VM_RET t3_s_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiTAGBODY tag-list code-list ... code-list
static VM_RET t3_s_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiGO (tag . tag-list)
static VM_RET t3_s_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiUNWIND_PROTECT clist clist
static VM_RET t3_s_unwind_protect(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_unwind_protect(tPVM vm, tPCELL clist, const tINT Code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiWITH_OPEN_INPUT_FILE plist clist
// iiWITH_OPEN_OUTPUT_FILE plist clist
// iiWITH_OPEN_IO_FILE plist clist
static VM_RET t3_s_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiCONTINUE_CONDITION
static VM_RET t3_s_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);
// iiWITH_HANDLER
static VM_RET t3_s_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret);

static void t3_s_vm_copy(tPVM vm, tINT* size);
static VM_RET t3_w_vm_copy(tPVM vm, tPCELL function, tINT* pc);

///////////////////

typedef VM_RET (*T3_GET_SIZE)(tPVM, tPCELL, const tINT, tPCELL*, tINT*);
typedef VM_RET (*T3_WRITE_CODE)(tPVM, tPCELL, const tINT, tPCELL*, tPCELL, tINT*, tINT);
typedef struct T3_FUNCTION_ T3_FUNCTION;

struct T3_FUNCTION_ {
	T3_GET_SIZE		get_size;
	T3_WRITE_CODE	write_code;
	void*			data1;
};

///////////////////

const T3_FUNCTION t3_table[]={
// 0
	{ t3_s_discard,			t3_w_discard,			op_discard				 },// iiDISCARD
	{ t3_s_type_1,			t3_w_type_1,			op_push_nil				 },// iiPUSH_NIL
	{ t3_s_type_1,			t3_w_type_1,			op_push_t				 },// iiPUSH_T
	{ t3_s_push_object,		t3_w_push_object,		0						 },// iiPUSH_OBJECT obj
	{ t3_s_type_2,			t3_w_type_2,			op_push_stack			 },// iiPUSH_STACK offset
	{ t3_s_type_2,			t3_w_type_2,			op_push_heap			 },// iiPUSH_HEAP offset
	{ t3_s_type_2,			t3_w_type_2,			op_push_variable		 },// iiPUSH_VARAIABLE bind-list
	{ t3_s_type_1,			t3_w_type_1,			op_call_rec				 },// iiCALL_REC
	{ t3_s_call_tail_rec,	t3_w_call_tail_rec,		0						 },// iiCALL_TAIL_REC
	{ t3_s_type_3,			t3_w_type_3,			op_call					 },// iiCALL bind-list anum
	{ t3_s_call_tail,		t3_w_call_tail,			op_call_tail			 },// iiCALL_TAIL bind-list anum
	{ t3_s_type_3,			t3_w_type_3,			op_call_bind			 },// iiCALL_BIND bind anum
	{ t3_s_call_tail,		t3_w_call_tail,			op_call_bind_tail		 },// iiCALL_BIND_TAIL bind anum
	{ t3_s_call_local,		t3_w_call_local,		0						 },// iiCALL_LOCAL offset flist
	{ t3_s_call_local_tail,	t3_w_call_local_tail,	0						 },// iiCALL_LOCAL_TAIL offset flist
	//
	{ t3_s_ret,				t3_w_ret,				0						 },// iiRET 
	{ t3_s_type_2,			t3_w_type_2_,			op_lambda_in			 },// iiLAMBDA_IN plist
	{ t3_s_type_2,			t3_w_type_2,			op_lambda_out			 },// iiLAMBDA_OUT plist
	{ t3_s_type_2,			t3_w_type_2_,			op_lambda_heap_in		 },// iiLAMBDA_HEAP_IN plist
	{ t3_s_type_2,			t3_w_type_2,			op_lambda_heap_out		 },// iiLAMBDA_HAEP_OUT plist
	{ t3_s_type_2,			t3_w_type_2,			op_push_function		 },// iiPUSH_FUNCTION bind-list
	{ t3_s_push_lfunction,	t3_w_push_lfunction,	op_push_local_function	 },// iiPUSH_LOCAL_FUNCTION offset flist
	{ t3_s_push_lambda,		t3_w_push_lambda,		op_push_lambda			 },// iiPUSH_LAMBDA flist
	{ t3_s_labels,			t3_w_labels,			op_labels_in			 },// iiLABELS_IN nlist function-list-1 ... function-list-n
	{ t3_s_type_1,			t3_w_type_1,			op_labels_out			 },// iiLABELS_OUT
	{ t3_s_labels,			t3_w_labels,			op_flet_in				 },// iiFLET_IN nlist function-list-1 ... function-list-n
	{ t3_s_type_1,			t3_w_type_1,			op_flet_out				 },// iiFLET_OUT
	{ t3_s_and,				t3_w_and,				op_and_check			 },// iiAND clist
	{ t3_s_and,				t3_w_and,				op_or_check				 },// iiOR clist
	{ t3_s_type_2,			t3_w_type_2,			op_set_stack			 },// iiSET_STACK offset
	{ t3_s_type_2,			t3_w_type_2,			op_set_heap				 },// iiSET_HEAP offset
	{ t3_s_type_2,			t3_w_type_2,			op_set_variable			 },// iiSET_VARIABLE bind-list
	{ t3_s_type_2,			t3_w_type_2,			op_set_dynamic			 },// iiSET_DYNAMIC symbol
	{ t3_s_type_2,			t3_w_type_2,			op_set_aref				 },// iiSET_AREF
	{ t3_s_type_2,			t3_w_type_2,			op_set_garef			 },// iiSET_GAREF
	{ t3_s_type_1,			t3_w_type_1,			op_set_elt				 },// iiSET_ELT
	{ t3_s_type_1,			t3_w_type_1,			op_set_property			 },// iiSET_PROPERTY
	{ t3_s_type_1,			t3_w_type_1,			op_set_car				 },// iiSET_CAR
	{ t3_s_type_1,			t3_w_type_1,			op_set_cdr				 },// iiSET_CDR
	{ t3_s_type_2,			t3_w_type_2,			op_set_accessor			 },// iiSET_ACCESSOR name
	{ t3_s_type_2,			t3_w_type_2,			op_push_dynamic			 },// iiPUSH_DYNAMIC name
	{ t3_s_dynamic_let,		t3_w_dynamic_let,		op_dynamic_let			 },// iiDYNAMIC_LET n name-1 ... nmae-n code-list
	{ t3_s_if,				t3_w_if,				0						 },// iiIF code-list code-list
	{ t3_s_case,			t3_w_case,				0						 },// iiCASE n key-list1 clist1 ... key-listn clistn
	{ t3_s_case_using,		t3_w_case_using,		0						 },// iiCASE_USING n key-list1 clist1 ... key-listn clistn
	{ t3_s_while,			t3_w_while,				op_while_check			 },// iiWHITE clist clist
	{ t3_s_for,				t3_w_for,				0						 },// iiFOR_STACK plist endtest result iteration
	{ t3_s_for,				t3_w_for,				0						 },// iiFOR_HEAP plist endtest result iteration
	{ t3_s_block,			t3_w_block,				op_block				 },// iiBLOCK clist tag
	{ t3_s_return_from,		t3_w_return_from,		op_return_from			 },// iiRETURN_FROM tag
	{ t3_s_catch,			t3_w_catch,				op_catch				 },// iiCATCH clist
	{ t3_s_throw,			t3_w_throw,				op_throw				 },// iiTHROW
	{ t3_s_tagbody,			t3_w_tagbody,			op_tagbody				 },// iiTAGBODY tag-list code-list ... code-list
	{ t3_s_go,				t3_w_go,				op_go					 },// iiGO (tag . tag-list)
	{ t3_s_unwind_protect,	t3_w_unwind_protect,	op_unwind_protect		 },// iiUNWIND_PROTECT clist clist
	{ t3_s_type_2,			t3_w_type_2,			op_class				 },// iiPUSH_CLASS bind-list
	{ t3_s_type_2,			t3_w_type_2,			op_the					 },// iiTHE bind-list
	{ t3_s_type_2,			t3_w_type_2,			op_assure				 },// iiASSURE bind-list
	{ t3_s_type_2,			t3_w_type_2,			op_convert				 },// iiCONVERT bind-list
	{ t3_s_type_f,			t3_w_type_f,			op_with_standard_input	 },// iiWITH_STANDARD_INPUT code-list
	{ t3_s_type_f,			t3_w_type_f,			op_with_standard_output	 },// iiWITH_STANDARD_OUTPUT code_list
	{ t3_s_type_f,			t3_w_type_f,			op_with_error_output	 },// iiWITH_ERROR_OUTPUT code_list
	{ t3_s_with_open_file,	t3_w_with_open_file,	op_with_open_input_file	 },// iiWITH_OPEN_INPUT_FILE plist clist
	{ t3_s_with_open_file,	t3_w_with_open_file,	op_with_open_output_file },// iiWITH_OPEN_OUTPUT_FILE plist clist
	{ t3_s_with_open_file,	t3_w_with_open_file,	op_with_open_io_file	 },// iiWITH_OPEN_IO_FILE plist clist
	{ t3_s_type_f,			t3_w_type_f,			op_ignore_errors		 },// iiIGNORE_ERRORS clist
	{ t3_s_continue,		t3_w_continue,			op_continue_condition	 },// iiCONTNUE_CONDITION
	{ t3_s_type_f,			t3_w_type_f,			op_with_handler			 },// iiWITH_HANDLER clist
	{ t3_s_type_f,			t3_w_type_f,			op_time					 },// iiTMIE
	{ t3_s_type_1,			t3_w_type_1,			op_quasiquote			 },// iiQUASIQUOTE
	{ t3_s_type_1,			t3_w_type_1,			op_quasiquote2			 },// iiQUASIQUOTE2
	{ t3_s_type_1,			t3_w_type_1,			op_unquote				 },// iiUNQUOTE
	{ t3_s_type_1,			t3_w_type_1,			op_unquote_splicing		 },// iiUNQUOTE_SPLICING
	{ t3_s_type_1,			t3_w_type_1,			op_unquote_splicing2	 },// iiUNQUOTE_SPLICING2
	{ t3_s_type_1,			t3_w_type_1,			op_functionp			 },
	{ t3_s_type_2,			t3_w_type_2,			op_apply				 },
	{ t3_s_type_2,			t3_w_type_2,			op_funcall				 },
	{ t3_s_type_1,			t3_w_type_1,			op_eq					 },
	{ t3_s_type_1,			t3_w_type_1,			op_eql					 },
	{ t3_s_type_1,			t3_w_type_1,			op_equal				 },
	{ t3_s_type_1,			t3_w_type_1,			op_not					 },
	{ t3_s_type_1,			t3_w_type_1,			op_generic_function_p	 },
	{ t3_s_type_1,			t3_w_type_1,			op_class_of				 },
	{ t3_s_type_1,			t3_w_type_1,			op_instancep			 },
	{ t3_s_type_1,			t3_w_type_1,			op_subclassp			 },
	{ t3_s_type_1,			t3_w_type_1,			op_symbolp				 },
	{ t3_s_type_2,			t3_w_type_2,			op_property				 },
	{ t3_s_type_2,			t3_w_type_2,			op_remove_property		 },
	{ t3_s_type_1,			t3_w_type_1,			op_gensym				 },
	{ t3_s_type_1,			t3_w_type_1,			op_numberp				 },
	{ t3_s_type_1,			t3_w_type_1,			op_parse_number			 },
	{ t3_s_type_1,			t3_w_type_1,			op_number_equal			 },
	{ t3_s_type_1,			t3_w_type_1,			op_number_not_equal		 },
	{ t3_s_type_1,			t3_w_type_1,			op_number_ge			 },
	{ t3_s_type_1,			t3_w_type_1,			op_number_le			 },
	{ t3_s_type_1,			t3_w_type_1,			op_number_greater		 },
	{ t3_s_type_1,			t3_w_type_1,			op_number_less			 },
	{ t3_s_type_2,			t3_w_type_2,			op_addition				 },
	{ t3_s_type_2,			t3_w_type_2,			op_multiplication		 },
	{ t3_s_type_2,			t3_w_type_2,			op_substraction			 },
	{ t3_s_type_2,			t3_w_type_2,			op_quotient				 },
	{ t3_s_type_1,			t3_w_type_1,			op_reciprocal			 },
	{ t3_s_type_2,			t3_w_type_2,			op_max					 },
	{ t3_s_type_2,			t3_w_type_2,			op_min					 },
	{ t3_s_type_1,			t3_w_type_1,			op_abs					 },
	{ t3_s_type_1,			t3_w_type_1,			op_exp					 },
	{ t3_s_type_1,			t3_w_type_1,			op_log					 },
	{ t3_s_type_1,			t3_w_type_1,			op_expt					 },
	{ t3_s_type_1,			t3_w_type_1,			op_sqrt					 },
	{ t3_s_type_1,			t3_w_type_1,			op_sin					 },
	{ t3_s_type_1,			t3_w_type_1,			op_cos					 },
	{ t3_s_type_1,			t3_w_type_1,			op_tan					 },
	{ t3_s_type_1,			t3_w_type_1,			op_atan					 },
	{ t3_s_type_1,			t3_w_type_1,			op_atan2				 },
	{ t3_s_type_1,			t3_w_type_1,			op_sinh					 },
	{ t3_s_type_1,			t3_w_type_1,			op_cosh					 },
	{ t3_s_type_1,			t3_w_type_1,			op_tanh					 },
	{ t3_s_type_1,			t3_w_type_1,			op_atanh				 },
	{ t3_s_type_1,			t3_w_type_1,			op_floatp				 },
	{ t3_s_type_1,			t3_w_type_1,			op_float				 },
	{ t3_s_type_1,			t3_w_type_1,			op_floor				 },
	{ t3_s_type_1,			t3_w_type_1,			op_ceiling				 },
	{ t3_s_type_1,			t3_w_type_1,			op_truncate				 },
	{ t3_s_type_1,			t3_w_type_1,			op_round				 },
	{ t3_s_type_1,			t3_w_type_1,			op_integerp				 },
	{ t3_s_type_1,			t3_w_type_1,			op_div					 },
	{ t3_s_type_1,			t3_w_type_1,			op_mod					 },
	{ t3_s_type_1,			t3_w_type_1,			op_gcd					 },
	{ t3_s_type_1,			t3_w_type_1,			op_lcm					 },
	{ t3_s_type_1,			t3_w_type_1,			op_isqrt				 },
	{ t3_s_type_1,			t3_w_type_1,			op_characterp			 },
	{ t3_s_type_1,			t3_w_type_1,			op_char_equal			 },
	{ t3_s_type_1,			t3_w_type_1,			op_char_not_equal		 },
	{ t3_s_type_1,			t3_w_type_1,			op_char_less			 },
	{ t3_s_type_1,			t3_w_type_1,			op_char_greater			 },
	{ t3_s_type_1,			t3_w_type_1,			op_char_le				 },
	{ t3_s_type_1,			t3_w_type_1,			op_char_ge				 },
	{ t3_s_type_1,			t3_w_type_1,			op_consp				 },
	{ t3_s_type_1,			t3_w_type_1,			op_cons					 },
	{ t3_s_type_1,			t3_w_type_1,			op_car					 },
	{ t3_s_type_1,			t3_w_type_1,			op_cdr					 },
	{ t3_s_type_1,			t3_w_type_1,			op_null					 },
	{ t3_s_type_1,			t3_w_type_1,			op_listp				 },
	{ t3_s_type_2,			t3_w_type_2,			op_create_list			 },
	{ t3_s_type_2,			t3_w_type_2,			op_list					 },
	{ t3_s_type_1,			t3_w_type_1,			op_reverse				 },
	{ t3_s_type_1,			t3_w_type_1,			op_nreverse				 },
	{ t3_s_type_2,			t3_w_type_2,			op_append				 },
	{ t3_s_type_1,			t3_w_type_1,			op_member				 },
	{ t3_s_type_2,			t3_w_type_2,			op_mapcar				 },
	{ t3_s_type_2,			t3_w_type_2,			op_mapc					 },
	{ t3_s_type_2,			t3_w_type_2,			op_mapcan				 },
	{ t3_s_type_2,			t3_w_type_2,			op_maplist				 },
	{ t3_s_type_2,			t3_w_type_2,			op_mapl					 },
	{ t3_s_type_2,			t3_w_type_2,			op_mapcon				 },
	{ t3_s_type_1,			t3_w_type_1,			op_assoc				 },
	{ t3_s_type_1,			t3_w_type_1,			op_basic_array_p		 },
	{ t3_s_type_1,			t3_w_type_1,			op_basic_array_a_p		 },
	{ t3_s_type_1,			t3_w_type_1,			op_general_array_a_p	 },
	{ t3_s_type_2,			t3_w_type_2,			op_create_array			 },
	{ t3_s_type_2,			t3_w_type_2,			op_aref					 },
	{ t3_s_type_2,			t3_w_type_2,			op_garef				 },
	{ t3_s_type_1,			t3_w_type_1,			op_array_dimensions		 },
	{ t3_s_type_1,			t3_w_type_1,			op_basic_vector_p		 },
	{ t3_s_type_1,			t3_w_type_1,			op_general_vector_p		 },
	{ t3_s_type_2,			t3_w_type_2,			op_create_vector		 },
	{ t3_s_type_2,			t3_w_type_2,			op_vector				 },
	{ t3_s_type_1,			t3_w_type_1,			op_stringp				 },
	{ t3_s_type_2,			t3_w_type_2,			op_create_string		 },
	{ t3_s_type_1,			t3_w_type_1,			op_string_equal			 },
	{ t3_s_type_1,			t3_w_type_1,			op_string_not_equal		 },
	{ t3_s_type_1,			t3_w_type_1,			op_string_less			 },
	{ t3_s_type_1,			t3_w_type_1,			op_string_greater		 },
	{ t3_s_type_1,			t3_w_type_1,			op_string_ge			 },
	{ t3_s_type_1,			t3_w_type_1,			op_string_le			 },
	{ t3_s_type_2,			t3_w_type_2,			op_char_index			 },
	{ t3_s_type_2,			t3_w_type_2,			op_string_index			 },
	{ t3_s_type_2,			t3_w_type_2,			op_string_append		 },
	{ t3_s_type_1,			t3_w_type_1,			op_length				 },
	{ t3_s_type_1,			t3_w_type_1,			op_elt					 },
	{ t3_s_type_1,			t3_w_type_1,			op_subseq				 },
	{ t3_s_type_2,			t3_w_type_2,			op_map_into				 },
	{ t3_s_type_1,			t3_w_type_1,			op_streamp				 },
	{ t3_s_type_1,			t3_w_type_1,			op_open_stream_p		 },
	{ t3_s_type_1,			t3_w_type_1,			op_input_stream_p		 },
	{ t3_s_type_1,			t3_w_type_1,			op_output_stream_p		 },
	{ t3_s_type_1,			t3_w_type_1,			op_standard_input		 },
	{ t3_s_type_1,			t3_w_type_1,			op_standard_output		 },
	{ t3_s_type_1,			t3_w_type_1,			op_error_output			 },
	{ t3_s_type_2,			t3_w_type_2,			op_open_input_file		 },
	{ t3_s_type_2,			t3_w_type_2,			op_open_output_file		 },
	{ t3_s_type_2,			t3_w_type_2,			op_open_io_file			 },
	{ t3_s_type_1,			t3_w_type_1,			op_close				 },
	{ t3_s_type_1,			t3_w_type_1,			op_finish_output		 },
	{ t3_s_type_1,			t3_w_type_1,			op_create_string_input_stream },
	{ t3_s_type_1,			t3_w_type_1,			op_create_string_output_stream },
	{ t3_s_type_1,			t3_w_type_1,			op_get_output_stream_string },
	{ t3_s_type_2,			t3_w_type_2,			op_read					 },
	{ t3_s_type_2,			t3_w_type_2,			op_read_char			 },
	{ t3_s_type_2,			t3_w_type_2,			op_preview_char			 },
	{ t3_s_type_2,			t3_w_type_2,			op_read_line			 },
	{ t3_s_type_1,			t3_w_type_1,			op_stream_ready_p		 },
	{ t3_s_type_2,			t3_w_type_2,			op_format				 },
	{ t3_s_type_1,			t3_w_type_1,			op_format_char			 },
	{ t3_s_type_1,			t3_w_type_1,			op_format_float			 },
	{ t3_s_type_1,			t3_w_type_1,			op_format_fresh_line	 },
	{ t3_s_type_1,			t3_w_type_1,			op_format_integer		 },
	{ t3_s_type_1,			t3_w_type_1,			op_format_object		 },
	{ t3_s_type_1,			t3_w_type_1,			op_format_tab			 },
	{ t3_s_type_2,			t3_w_type_2,			op_read_byte			 },
	{ t3_s_type_1,			t3_w_type_1,			op_write_byte			 },
	{ t3_s_type_1,			t3_w_type_1,			op_probe_file			 },
	{ t3_s_type_1,			t3_w_type_1,			op_file_position		 },
	{ t3_s_type_1,			t3_w_type_1,			op_set_file_position	 },
	{ t3_s_type_1,			t3_w_type_1,			op_file_length			 },
	{ t3_s_type_2,			t3_w_type_2,			op_error				 },
	{ t3_s_type_2,			t3_w_type_2,			op_cerror				 },
	{ t3_s_type_2,			t3_w_type_2,			op_signal_condition		 },
	{ t3_s_type_2,			t3_w_type_2,			op_condition_continuable },
	{ t3_s_type_2,			t3_w_type_2,			op_arithmetic_error_operation },
	{ t3_s_type_2,			t3_w_type_2,			op_arithmetic_error_operand },
	{ t3_s_type_2,			t3_w_type_2,			op_domain_error_object	 },
	{ t3_s_type_2,			t3_w_type_2,			op_domain_error_expected_class },
	{ t3_s_type_2,			t3_w_type_2,			op_parse_error_string	 },
	{ t3_s_type_2,			t3_w_type_2,			op_parse_error_expected_class },
	{ t3_s_type_2,			t3_w_type_2,			op_simple_error_format_string },
	{ t3_s_type_2,			t3_w_type_2,			op_simple_error_format_arguments },
	{ t3_s_type_2,			t3_w_type_2,			op_stream_error_stream	 },
	{ t3_s_type_2,			t3_w_type_2,			op_undefined_entity_name },
	{ t3_s_type_2,			t3_w_type_2,			op_undefined_entity_namespace },
	{ t3_s_type_2,			t3_w_type_2,			op_identity				 },
	{ t3_s_type_2,			t3_w_type_2,			op_get_universal_time	 },
	{ t3_s_type_2,			t3_w_type_2,			op_get_internal_run_time },
	{ t3_s_type_2,			t3_w_type_2,			op_get_internal_real_time },
	{ t3_s_type_2,			t3_w_type_2,			op_get_internal_time_units_per_second },
	{ t3_s_type_2,			t3_w_type_2,			op_system },
	{ t3_s_type_2,			t3_w_type_2,			op_exit },
	{ t3_s_type_2,			t3_w_type_2,			op_strftime },
	{ t3_s_type_2,			t3_w_type_2,			op_get_argument },
	{ t3_s_type_2,			t3_w_type_2,			op_get_environment },
	{ t3_s_continue,		t3_w_continue,			op_arity_error },
	{ t3_s_type_2,			t3_w_type_2,			op_eval },
	{ t3_s_type_3,			t3_w_type_3,			op_number_equal_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_number_equal_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_number_not_equal_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_number_not_equal_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_number_less_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_number_less_integer_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_number_less_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_number_le_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_number_le_integer_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_number_le_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_addition_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_addition_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_substraction_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_substraction_integer_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_substraction_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_eq_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_eq_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_eq_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_eq_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			op_eq_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			op_equal_stack_stack },
};

///////////////////

static VM_RET t3_clist_to_function(tPVM vm, tPCELL clist, tPCELL* function);
static VM_RET t3_flist_to_function(tPVM vm, tPCELL flist, tPCELL* function);
static VM_RET t3_code_list_get_size(tPVM vm, tPCELL clist, tINT* size);
static VM_RET t3_code_list_get_size_(tPVM vm, tPCELL clist, tINT* size);
static VM_RET t3_code_list_write_code(tPVM vm, tPCELL clist, tPCELL function, tINT ret);
static VM_RET t3_code_list_write_code_(tPVM vm, tPCELL clist, tPCELL function, tINT* pc, tINT ret);

static void t3_read_argument_1(tPCELL* head, tPOBJECT arg1);
static void t3_read_argument_2(tPCELL* head, tPOBJECT arg1, tPOBJECT arg2);

///////////////////

// コードリストから関数オブジェクトを作成する．
VM_RET translate_pass3(tPVM vm, tPCELL code_list, tPCELL* function)
{
	VM_RET ret;
	t3_clear(vm_get_translator(vm));
	ret=t3_clist_to_function(vm, code_list, function);
	t3_clear(vm_get_translator(vm));
	return ret;
}

VM_RET translate_pass3_defun(tPVM vm, tPCELL flist, tPCELL* function)
{
	VM_RET ret;
	t3_clear(vm_get_translator(vm));
	ret=t3_flist_to_function(vm, flist, function);
	t3_clear(vm_get_translator(vm));
	return ret;
}

VM_RET translate_pass3_method(tPVM vm, tPCELL mlist, tPOBJECT form, tPCELL* method)
{
	tPCELL function;
	tOBJECT obj;
	tBOOL next;
	t3_clear(vm_get_translator(vm));
	t2_set_method_qualifier(vm, mlist_get_qualifier(mlist));
	if (t3_clist_to_function(vm, mlist_get_clist(mlist), &function)) return VM_ERROR;
	cell_to_object(function, &obj);
	if (vm_push(vm, &obj)) goto ERROR;
	next=function_name_list_is_referred(mlist_get_env(mlist)) ? tTRUE : tFALSE;
	if (method_create_(vm, function, mlist_get_pplist(mlist), next, mlist_get_qualifier(mlist), form, method)) { vm_pop(vm); goto ERROR; }
	vm_pop(vm);
	t3_clear(vm_get_translator(vm));
	return VM_OK;
ERROR:
	t3_clear(vm_get_translator(vm));
	return VM_ERROR;
}

///////////////////////////////////////

#define CODE_SIZE_RET	(s_POP_REG+s_RET)

static VM_RET t3_clist_to_function(tPVM vm, tPCELL clist, tPCELL* function)
{
	tOBJECT tmp;
	tINT size=0;
	// コードサイズの計算
	if (t3_code_list_get_size(vm, clist, &size)) return VM_ERROR;
	// 関数オブジェクトの作成
	if (function_create(vm, 0, size, code_list_get_max_sp(clist), vm_get_current_package(vm), function)) return VM_ERROR;
	OBJECT_SET_FUNCTION(&tmp, *function);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// 命令の書き込み
	if (t3_code_list_write_code(vm, clist, *function, size-CODE_SIZE_RET)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

static VM_RET t3_flist_to_function(tPVM vm, tPCELL flist, tPCELL* function)
{
	tOBJECT tmp;
	tINT size=0;
	tPCELL clist, plist;
	clist=function_list_get_code_list(flist);
	plist=function_list_get_parameter_list(flist);
	// コードサイズの計算
	if (t3_code_list_get_size(vm, clist, &size)) return VM_ERROR;
	// 関数オブジェクトの作成
	if (function_create(vm, plist, size, code_list_get_max_sp(clist), vm_get_current_package(vm), function)) return VM_ERROR;
	function_list_set_function(flist, *function);
	OBJECT_SET_FUNCTION(&tmp, *function);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// 命令の書き込み
	if (t3_code_list_write_code(vm, clist, *function, size-CODE_SIZE_RET)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

static VM_RET t3_code_list_get_size(tPVM vm, tPCELL clist, tINT* size)
{
	VM_RET ret;

	*size=s_PUSH_REG;	// push eax;
	ret=t3_code_list_get_size_(vm, clist, size);
	*size+=s_POP_REG+	// pop ebx
		   s_RET;		// ret
	return ret;
}

static VM_RET t3_code_list_get_size_(tPVM vm, tPCELL clist, tINT* size)
{
	tPCELL head;
	tINT code;
	head=code_list_get_head(clist);
	while (code=code_list_get_command(head),
		   !(*t3_table[code].get_size)(vm, clist, code, &head, size));
	return vm_last_condition_is_ok(vm) ? VM_OK : VM_ERROR;
}

static VM_RET t3_code_list_write_code(tPVM vm, tPCELL clist, tPCELL function, tINT ret)
{
	tINT pc;
	pc=0;

	if (opcode_push_reg(vm, function, &pc, reg_EAX)) return VM_ERROR;
	if (t3_code_list_write_code_(vm, clist, function, &pc, ret)) return VM_ERROR;
	if (ret!=pc) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (opcode_pop_reg(vm, function, &pc, reg_EBX)||
		   opcode_ret(vm, function, &pc)) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_code_list_write_code_(tPVM vm, tPCELL clist, tPCELL function, tINT* pc, tINT ret)
{
	tPCELL head;
	tINT command;
	head=code_list_get_head(clist);
	while (command=code_list_get_command(head),
		!(*t3_table[command].write_code)(vm, clist, command, &head, function, pc, ret));
	return vm_last_condition_is_ok(vm) ? VM_OK : VM_ERROR;
}

static void t3_read_argument_1(tPCELL* head, tPOBJECT arg1)
{
	code_list_increment_head(head);
	cons_get_car(*head, arg1);
	code_list_increment_head(head);
}

static void t3_read_argument_2(tPCELL* head, tPOBJECT arg1, tPOBJECT arg2)
{
	code_list_increment_head(head);
	cons_get_car(*head, arg1);
	code_list_increment_head(head);
	cons_get_car(*head, arg2);
	code_list_increment_head(head);
}

///////////////////

// dummy unknown operation?
static VM_RET t3_get_size_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

static VM_RET t3_write_code_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

// iiDISACRD
static VM_RET t3_s_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_SUB_IMMEDIATE_REG+
		   s_MOV_REG_TO_PTR_REG;
	return VM_OK;
}

static VM_RET t3_w_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	code_list_increment_head(head);
	return opcode_pop_reg(vm, function, pc, reg_EAX)||
		   opcode_push_reg(vm, function, pc, reg_EAX)||
	       opcode_mov_ptr_eax_to_ecx(vm, function, pc)||
		   opcode_sub_immediate_reg(vm, function, pc, (void*)0x08, reg_ECX)||
		   opcode_mov_ecx_to_ptr_eax(vm, function, pc);
}

static VM_RET t3_s_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);

	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_PUSH_REG+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;

	return VM_OK;
}

static VM_RET t3_w_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tINT jmp;
	code_list_increment_head(head);
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_call_direct(vm, function, pc, (void*)t3_table[code].data1)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT obj;
	void* procedure;
	tINT jmp;
	// 引数の読込み
	t3_read_argument_1(head, &obj);

	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:			procedure=op_push_integer;			break;
	case OBJECT_FLOAT:				procedure=op_push_float;			break;
	case OBJECT_CHARACTER:			procedure=op_push_character;		break;
	case OBJECT_CONS:				procedure=op_push_cons;				break;
	case OBJECT_STRING:				procedure=op_push_string;			break;
	case OBJECT_SYMBOL:				procedure=op_push_symbol;			break;
	case OBJECT_VECTOR:				procedure=op_push_vector;			break;
	case OBJECT_ARRAY:				procedure=op_push_array;			break;
	default:
		if (OBJECT_IS_CELL(&obj)) {
			procedure=op_push_cell_object;
		} else {
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	}

	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, obj.data.p)||
		opcode_call_direct(vm, function, pc, (void*)procedure)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT obj;
	tINT jmp;
	// 引数の読込み
	t3_read_argument_1(head, &obj);

	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, obj.data.p)||
		opcode_call_direct(vm, function, pc, (void*)(void*)t3_table[code].data1)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_w_type_2_(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT obj;
	tINT jmp;
	// 引数の読込み
	t3_read_argument_1(head, &obj);
	if (function_add_use_object(vm, function, &obj)) return VM_ERROR;

	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, obj.data.p)||
		 opcode_call_direct(vm, function, pc, (void*)(void*)t3_table[code].data1)||
		 opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT op1, op2;
	tINT jmp;
	// 引数の読込み
	t3_read_argument_2(head, &op1, &op2);

	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, op1.data.p)||
		opcode_push_immediate(vm, function, pc, op2.data.p)||
		opcode_call_direct(vm, function, pc, (void*)t3_table[code].data1)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	code_list_increment_head(head);
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_call_direct(vm, function, pc, (void*)op_call_tail_rec)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;

	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT anum, blist;
	t3_read_argument_2(head, &blist, &anum);

	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&blist))||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_INTEGER(&anum))||
		opcode_call_direct(vm, function, pc, (void*)t3_table[code].data1)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT offset, flist;
	tPCELL p;
	void* procedure;
	tINT jmp;

	t3_read_argument_2(head, &offset, &flist);
	p=OBJECT_GET_CELL(&flist);
	if (flist_is_call_next_method(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=op_call_next_method_around; break;
		case METHOD_PRIMARY:	procedure=op_call_next_method_primary; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else if (flist_is_next_method_p(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=op_next_method_p_around; break;
		case METHOD_PRIMARY:	procedure=op_next_method_p_primary; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else {
		p=function_list_get_function(OBJECT_GET_CELL(&flist));
		cell_to_object(p, &flist);
		procedure = function_is_heap(p) ? op_call_local_heap : op_call_local_stack;
	}
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_INTEGER(&offset))||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&flist))||
		opcode_call_direct(vm, function, pc, procedure)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

// iiCALL_LOCAL_TAIL offset flist
static VM_RET t3_s_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT offset, flist;
	tPCELL p;
	void* procedure;

	t3_read_argument_2(head, &offset, &flist);
	p=OBJECT_GET_CELL(&flist);
	if (flist_is_call_next_method(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=op_call_next_method_around; break;
		case METHOD_PRIMARY:	procedure=op_call_next_method_primary; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else if (flist_is_next_method_p(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=op_next_method_p_around; break;
		case METHOD_PRIMARY:	procedure=op_next_method_p_primary; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else {
		p=function_list_get_function(OBJECT_GET_CELL(&flist));
		cell_to_object(p, &flist);
		procedure = function_is_heap(p) ? op_call_local_heap : op_call_local_stack;
	}
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_INTEGER(&offset))||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&flist))||
		opcode_call_direct(vm, function, pc, procedure)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiRET
// コード生成部で最後にretをつけるので なにもすることないはず
// コード終了を表す門番？
static VM_RET t3_s_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	code_list_increment_head(head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_push_lfunction(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_push_lfunction(tPVM vm, tPCELL Clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT offset, flist;
	tPCELL p;
	tINT jmp;
	t3_read_argument_2(head, &offset, &flist);
	p=function_list_get_function(OBJECT_GET_CELL(&flist));
	cell_to_object(p, &flist);
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_INTEGER(&offset))||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&flist))||
		opcode_call_direct(vm, function, pc, op_push_local_function)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT flist;
	tPCELL p;
	tINT jmp;
	t3_read_argument_1(head, &flist);
	if (t3_flist_to_function(vm, OBJECT_GET_CELL(&flist), &p)) return VM_ERROR;
	cell_to_object(p, &flist);
	if (function_add_use_object(vm, function, &flist)) return VM_ERROR;
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&flist))||
		opcode_call_direct(vm, function, pc, (void*)op_push_lambda)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT nlist, obj;
	tINT i, n;
	code_list_increment_head(head);
	cons_get_car(*head, &nlist);
	cons_get_car(OBJECT_GET_CELL(&nlist), &obj);
	n=OBJECT_GET_INTEGER(&obj);
	if (n<0) n=-n;
	for (i=0; i<n; i++) {
		tINT s;
		tPCELL clist, plist, flist, func;
		code_list_increment_head(head);
		cons_get_car(*head, &obj);
		// サイズの計算とコード部分の空の関数の作成
		flist=OBJECT_GET_CELL(&obj);
		clist=function_list_get_code_list(flist);
		plist=function_list_get_parameter_list(flist);
		// コードサイズの計算
		if (t3_code_list_get_size(vm, clist, &s)) return VM_ERROR;
		// 関数オブジェクトの作成
		if (function_create(vm, plist, s, code_list_get_max_sp(clist), vm_get_current_package(vm), &func)) return VM_ERROR;
		function_list_set_function(flist, func);
	}
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT nlist, obj;
	tINT i, n, jmp;
	// 引数の取得と局所関数の変換
	code_list_increment_head(head);
	cons_get_car(*head, &nlist);
	cons_get_car(OBJECT_GET_CELL(&nlist), &obj);
	n=OBJECT_GET_INTEGER(&obj);
	if (n<0) n=-n;
	for (i=0; i<n; i++) {
		tPCELL clist, flist, func;
		code_list_increment_head(head);
		cons_get_car(*head, &obj);
		flist=OBJECT_GET_CELL(&obj);
		clist=function_list_get_code_list(flist);
		func=function_list_get_function(flist);
		if (t3_code_list_write_code(vm, clist, func, function_get_code_size(func)-CODE_SIZE_RET)) return VM_ERROR;
		cell_to_object(func, &obj);
		if (function_add_use_object(vm, function, &obj)) return VM_ERROR;
	}
	code_list_increment_head(head);
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&nlist))||
		opcode_call_direct(vm, function, pc, t3_table[code].data1)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT form;
	t3_read_argument_1(head, &form);
	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_CMP_IMMEDIATE_REG+
		   s_JCC_FULL_DISPLACEMENT+
		   s_SUB_IMMEDIATE_REG+
		   s_MOV_REG_TO_PTR_REG;
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&form), size)) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_w_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT form;
	tINT jmp, dest;
	t3_read_argument_1(head, &form);
	if (opcode_pop_reg(vm, function, pc, reg_EAX)||
		opcode_push_reg(vm, function, pc, reg_EAX)||
		opcode_mov_ptr_eax_to_ecx(vm, function, pc)||
		opcode_mov_ptr_ecx_to_edx(vm, function, pc)||
		opcode_cmp_immediate_reg(vm, function, pc, (void*)0x01, reg_EDX)) return VM_ERROR;
	jmp=*pc;
	*pc+=s_JCC_FULL_DISPLACEMENT;
	if (opcode_sub_immediate_reg(vm, function, pc, (void*)0x08, reg_ECX)||
		opcode_mov_ecx_to_ptr_eax(vm, function, pc)) return VM_ERROR;
	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&form), function, pc, ret)) return VM_ERROR;
	dest=*pc;
	if (code==iiAND) {
		if (opcode_jcc_full_displacement(vm, function, &jmp, tttn_Z, (void*)(dest-jmp-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	} else {
		if (opcode_jcc_full_displacement(vm, function, &jmp, tttn_NZ, (void*)(dest-jmp-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	}

	return VM_OK;
}

static VM_RET t3_s_dynamic_let(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT N;
	tINT i, n;
	code_list_increment_head(head);
	cons_get_car(*head, &N);
	n=OBJECT_GET_INTEGER(&N);
	for (i=0; i<=n; i++) {
		code_list_increment_head(head);
		t3_s_vm_copy(vm, size);
		*size+=s_PUSH_IMMEDIATE+
			   s_PUSH_IMMEDIATE+
			   s_CALL_DIRECT+
			   s_AND_REG_REG+
			   s_JCC_FULL_DISPLACEMENT;
	}
	code_list_increment_head(head);
	return VM_OK;
}

static VM_RET t3_w_dynamic_let(tPVM vm, tPCELL clist, const tINT ocde, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT N, body;
	tINT i, n, jmp;
	tPCELL p;

	code_list_increment_head(head);
	cons_get_car(*head, &N);
	n=OBJECT_GET_INTEGER(&N);
	for (i=0; i<n; i++) {
		tOBJECT name;
		code_list_increment_head(head);
		cons_get_car(*head, &name);
		if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
		if (opcode_push_immediate(vm, function, pc, (void*)n)||
			opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&name))||
			opcode_call_direct(vm, function, pc, op_dynamic_let_init)||
			opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
		jmp=*pc+s_JCC_FULL_DISPLACEMENT;
		if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	}
	code_list_increment_head(head);
	cons_get_car(*head, &body);
	code_list_increment_head(head);

	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &p)) return VM_ERROR;
	cell_to_object(p, &body);
	if (function_add_use_object(vm, function, &body)) return VM_ERROR;
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_push_immediate(vm, function, pc, (void*)n)||
		opcode_push_immediate(vm, function, pc, p)||
		opcode_call_direct(vm, function, pc, op_dynamic_let)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT then_clist, else_clist;
	// 引数の読込み
	t3_read_argument_2(head, &then_clist, &else_clist);
	// サイズの計算
	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_SUB_IMMEDIATE_REG+
		   s_MOV_REG_TO_PTR_REG+
		   s_CMP_IMMEDIATE_REG+
		   s_JCC_FULL_DISPLACEMENT;
		   // else-code
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&else_clist), size)) return VM_ERROR;
	*size+=s_JMP_FULL_DISPLACEMENT;
		   // then-code
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&then_clist), size)) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_w_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT then_clist, else_clist;
	tINT then_jmp, end_jmp, then_dest, end_dest;
	// 引数の読込み
	t3_read_argument_2(head, &then_clist, &else_clist);
	// コードの生成
	if (opcode_pop_reg(vm, function, pc, reg_EAX)||
		opcode_push_reg(vm, function, pc, reg_EAX)||
		opcode_mov_ptr_eax_to_ecx(vm, function, pc)||
		opcode_mov_ptr_ecx_to_edx(vm, function, pc)||
		opcode_sub_immediate_reg(vm, function, pc, (void*)0x08, reg_ECX)||
		opcode_mov_ecx_to_ptr_eax(vm, function, pc)||
		opcode_cmp_immediate_reg(vm, function, pc, (void*)0x01, reg_EDX)) return VM_ERROR;
	then_jmp=*pc;
	*pc+=s_JCC_FULL_DISPLACEMENT;
	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&else_clist), function, pc, ret)) return VM_ERROR;
	end_jmp=*pc;
	*pc+=s_JMP_FULL_DISPLACEMENT;
	// THEN
	then_dest=*pc;
	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&then_clist), function, pc, ret)) return VM_ERROR;
	// END
	end_dest=*pc;
	if (opcode_jcc_full_displacement(vm, function, &then_jmp, tttn_NZ, (void*)(then_dest-then_jmp-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	if (opcode_jmp_full_displacement(vm, function, &end_jmp, (void*)(end_dest-end_jmp-s_JMP_FULL_DISPLACEMENT))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tINT i, n;
	tOBJECT obj;

	code_list_increment_head(head);
	cons_get_car(*head, &obj);
	n=OBJECT_GET_INTEGER(&obj);

	for (i=0; i<n; i++) {
		code_list_increment_head(head);
		code_list_increment_head(head);
		t3_s_vm_copy(vm, size);
		*size+=s_PUSH_IMMEDIATE+
			   s_CALL_DIRECT+
			   s_AND_REG_REG+
			   s_JCC_8BIT_DISPLACEMENT;
		t3_s_vm_copy(vm, size);
		*size+=s_PUSH_IMMEDIATE+
			   s_CALL_DIRECT+
			   s_AND_REG_REG+
			   s_JCC_FULL_DISPLACEMENT+
			   s_POP_REG+
			   s_RET;
	}
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT;
	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT;
	return VM_OK;
}

static VM_RET t3_w_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	void* check, *result, *end;
	tINT i, n, start, end_dest, jz;
	tOBJECT obj, keylist, form;
	tPCELL p;
	// iiCASE と iiCASE_USING の場合わけ
	if (code==iiCASE) {
		check=op_case_check;
		result=op_case_result;
		end=op_case_end;
	} else {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	//
	code_list_increment_head(head);
	cons_get_car(*head, &obj);
	n=OBJECT_GET_INTEGER(&obj);
	start=*pc;
	for (i=0; i<n; i++) {
		code_list_increment_head(head);
		cons_get_car(*head, &keylist);
		code_list_increment_head(head);
		cons_get_car(*head, &form);
		if (t3_clist_to_function(vm, OBJECT_GET_CELL(&form), &p)) return VM_ERROR;
		cell_to_object(p, &form);
		if (function_add_use_object(vm, function, &form)) return VM_ERROR;
		if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
		if (opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&keylist))||
			opcode_call_direct(vm, function, pc, check)||
			opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)||
			opcode_jcc_8bit_displacement(vm, function, pc, tttn_Z,
			s_RET+s_POP_REG+s_JCC_FULL_DISPLACEMENT+s_AND_REG_REG+s_CALL_DIRECT+s_PUSH_IMMEDIATE+s_POP_REG+s_PUSH_REG+s_PUSH_REG
			)) return VM_ERROR;
		if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
		if (opcode_push_immediate(vm, function, pc, p)||
			opcode_call_direct(vm, function, pc, result)||
			opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
		*pc+=s_JCC_FULL_DISPLACEMENT;
		if (opcode_pop_reg(vm, function, pc, reg_EBX)||
			opcode_ret(vm, function, pc)) return VM_ERROR;
	}

	code_list_increment_head(head);

	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_call_direct(vm, function, pc, op_push_nil)) return VM_ERROR;
	end_dest=*pc;
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_call_direct(vm, function, pc, end)) return VM_ERROR;
	jz=start;
	for (i=0; i<n; i++) {
		jz+=s_POP_REG+s_PUSH_REG+s_PUSH_REG+s_PUSH_IMMEDIATE+s_CALL_DIRECT+s_AND_REG_REG+s_JCC_8BIT_DISPLACEMENT+
			s_POP_REG+s_PUSH_REG+s_PUSH_REG+s_PUSH_IMMEDIATE+s_CALL_DIRECT+s_AND_REG_REG;
		if (opcode_jcc_full_displacement(vm, function, &jz, tttn_Z, (void*)(end_dest-jz-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
		jz+=s_POP_REG+s_RET;
	}
	return VM_OK;
}

static VM_RET t3_s_case_using(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT obj;
	tINT i, n;
	code_list_increment_head(head);
	cons_get_car(*head, &obj);
	n=OBJECT_GET_INTEGER(&obj);
	for (i=0; i<n; i++) {
		code_list_increment_head(head);
		code_list_increment_head(head);
		t3_s_vm_copy(vm, size);
		*size+=s_PUSH_IMMEDIATE+
			   s_CALL_DIRECT+
			   s_AND_REG_REG+
			   s_JCC_8BIT_DISPLACEMENT+
			   s_POP_REG+
			   s_RET;
		t3_s_vm_copy(vm, size);
		*size+=s_CALL_DIRECT+
			   s_AND_REG_REG+
			   s_JCC_8BIT_DISPLACEMENT;
		t3_s_vm_copy(vm, size);
		*size+=s_PUSH_IMMEDIATE+
			   s_CALL_DIRECT+
			   s_AND_REG_REG+
			   s_JCC_FULL_DISPLACEMENT+
			   s_POP_REG+
			   s_RET;
	}
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT;
	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT;
	return VM_OK;
}

static VM_RET t3_w_case_using(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT obj, keylist, result;
	tINT i, n, end_dest, start, jz;
	code_list_increment_head(head);
	cons_get_car(*head, &obj);
	n=OBJECT_GET_INTEGER(&obj);
	start=*pc;
	for (i=0; i<n; i++) {
		tPCELL p;
		code_list_increment_head(head);
		cons_get_car(*head, &keylist);
		code_list_increment_head(head);
		cons_get_car(*head, &result);
		if (t3_clist_to_function(vm, OBJECT_GET_CELL(&result), &p)) return VM_ERROR;
		cell_to_object(p, &result);
		if (function_add_use_object(vm, function, &result)) return VM_ERROR;
		if (t3_w_vm_copy(vm, function, pc)||
			opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&keylist))||
			opcode_call_direct(vm, function, pc, op_case_using_predicate)||
			opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)||
			opcode_jcc_8bit_displacement(vm, function, pc, tttn_Z, (s_POP_REG+s_RET))||
			opcode_pop_reg(vm, function, pc, reg_EBX)||
			opcode_ret(vm, function, pc)||
			t3_w_vm_copy(vm, function, pc)||
			opcode_call_direct(vm, function, pc, op_case_using_check)||
			opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)||
			opcode_jcc_8bit_displacement(vm, function, pc, tttn_Z, 
			(s_POP_REG+s_PUSH_REG+s_PUSH_REG+s_PUSH_IMMEDIATE+s_CALL_DIRECT+s_AND_REG_REG+s_JCC_FULL_DISPLACEMENT+s_POP_REG+s_RET))||
			t3_w_vm_copy(vm, function, pc)||
			opcode_push_immediate(vm, function, pc, p)||
			opcode_call_direct(vm, function, pc, op_case_using_result)||
			opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
		*pc+=s_JCC_FULL_DISPLACEMENT;
		if (opcode_pop_reg(vm, function, pc, reg_EBX)||
			opcode_ret(vm, function, pc)) return VM_ERROR;
	}
	code_list_increment_head(head);
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_call_direct(vm, function, pc, op_push_nil)) return VM_ERROR;
	end_dest=*pc;
	if (t3_w_vm_copy(vm, function, pc)) return VM_ERROR;
	if (opcode_call_direct(vm, function, pc, op_case_using_end)) return VM_ERROR;
	jz=start;
	for (i=0; i<n; i++) {
		jz+=s_POP_REG+s_PUSH_REG+s_PUSH_REG+s_PUSH_IMMEDIATE+s_CALL_DIRECT+s_AND_REG_REG+s_JCC_8BIT_DISPLACEMENT+
			s_POP_REG+s_RET+s_POP_REG+s_PUSH_REG+s_PUSH_REG+s_CALL_DIRECT+s_AND_REG_REG+s_JCC_8BIT_DISPLACEMENT+s_POP_REG+s_PUSH_REG+s_PUSH_REG+s_PUSH_IMMEDIATE+
			s_CALL_DIRECT+s_AND_REG_REG;
		if (opcode_jcc_full_displacement(vm, function, &jz, tttn_Z, (void*)(end_dest-jz-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
		jz+=s_POP_REG+s_RET;
	}
	return VM_OK;
}

static VM_RET t3_s_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT test, body;
	t3_read_argument_2(head, &test, &body);
	// LOOP
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&test), size)) return VM_ERROR;
	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_SUB_IMMEDIATE_REG+
		   s_MOV_REG_TO_PTR_REG+
		   s_CMP_IMMEDIATE_REG+
		   s_JCC_FULL_DISPLACEMENT;
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&body), size)) return VM_ERROR;
	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_SUB_IMMEDIATE_REG+
		   s_MOV_REG_TO_PTR_REG;
	*size+=s_JMP_FULL_DISPLACEMENT;
	// next
	return VM_OK;
}

static VM_RET t3_w_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tINT loop_dest, next_dest, next_jcc, loop_jmp;
	tOBJECT test, body;
	t3_read_argument_2(head, &test, &body);
	//
	loop_dest=*pc;
	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&test), function, pc, ret)) return VM_ERROR;
	if (opcode_pop_reg(vm, function, pc, reg_EAX)||
		opcode_push_reg(vm, function, pc, reg_EAX)||
		opcode_mov_ptr_eax_to_ecx(vm, function, pc)||
		opcode_mov_ptr_ecx_to_edx(vm, function, pc)||
		opcode_cmp_immediate_reg(vm, function, pc, (void*)0x01, reg_EDX)) return VM_ERROR;
	next_jcc=*pc;
	*pc+=s_JCC_FULL_DISPLACEMENT;
	if (opcode_sub_immediate_reg(vm, function, pc, (void*)0x08, reg_ECX)||
		opcode_mov_ecx_to_ptr_eax(vm, function, pc)) return VM_ERROR;

	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&body), function, pc, ret)) return VM_ERROR;
	if (opcode_pop_reg(vm, function, pc, reg_EAX)||
		opcode_push_reg(vm, function, pc, reg_EAX)||
		opcode_mov_ptr_eax_to_ecx(vm, function, pc)||
		opcode_sub_immediate_reg(vm, function, pc, (void*)0x08, reg_ECX)||
		opcode_mov_ecx_to_ptr_eax(vm, function, pc)) return VM_ERROR;
	loop_jmp=*pc;
	*pc+=s_JMP_FULL_DISPLACEMENT;
	next_dest=*pc;
	if (opcode_jcc_full_displacement(vm, function, &next_jcc, tttn_Z, (void*)(next_dest-next_jcc-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	if (opcode_jmp_full_displacement(vm, function, &loop_jmp, (void*)(loop_dest-loop_jmp-s_JMP_FULL_DISPLACEMENT))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tOBJECT plist, endtest, result, iteration;
	// 引数の読み取り
	code_list_increment_head(head);
	cons_get_car(*head, &plist);
	code_list_increment_head(head);
	cons_get_car(*head, &endtest);
	code_list_increment_head(head);
	cons_get_car(*head, &result);
	code_list_increment_head(head);
	cons_get_car(*head, &iteration);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_8BIT_DISPLACEMENT;
	// RET
	*size+=s_POP_REG+
		   s_RET;
	// LOOP
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&endtest), size)) return VM_ERROR;

	*size+=s_POP_REG+
		   s_PUSH_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_MOV_PTR_REG_TO_REG+
		   s_SUB_IMMEDIATE_REG+
		   s_MOV_REG_TO_PTR_REG+
		   s_CMP_IMMEDIATE_REG+
		   s_JCC_FULL_DISPLACEMENT;

	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&iteration), size)) return VM_ERROR;
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT+
		   s_JMP_FULL_DISPLACEMENT;
	// RESULT
	if (t3_code_list_get_size_(vm, OBJECT_GET_CELL(&result), size)) return VM_ERROR;
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT plist, endtest, result, iteration;
	void *for_init, *for_test, *for_iteration, *for_result;
	tINT loop_dest, result_dest, ret_dest, result_jcc, loop_jmp, ret_jcc, ret_jcc_2, n;

	// 引数の読み取り
	code_list_increment_head(head);
	cons_get_car(*head, &plist);
	code_list_increment_head(head);
	cons_get_car(*head, &endtest);
	code_list_increment_head(head);
	cons_get_car(*head, &result);
	code_list_increment_head(head);
	cons_get_car(*head, &iteration);
	code_list_increment_head(head);

	if (code==iiFOR_STACK) {
		for_init=op_for_stack_init;
		for_test=op_for_test;
		for_iteration=op_for_stack_iteration;
		for_result=op_for_stack_result;
	} else if (code==iiFOR_HEAP) {
		for_init=op_for_heap_init;
		for_test=op_for_test;
		for_iteration=op_for_heap_iteration;
		for_result=op_for_heap_result;
	} else {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}

	if (function_add_use_object(vm, function, &plist)) return VM_ERROR;

	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&plist))||
		opcode_call_direct(vm, function, pc, for_init)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)||
		opcode_jcc_8bit_displacement(vm, function, pc, tttn_Z, s_POP_REG+s_RET)) return VM_ERROR;
	// RET
	ret_dest=*pc;
	if (opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	// ENDTEST
	loop_dest=*pc;

	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&endtest), function, pc, ret)) return VM_ERROR;
	if (opcode_pop_reg(vm, function, pc, reg_EAX)||
		opcode_push_reg(vm, function, pc, reg_EAX)||
		opcode_mov_ptr_eax_to_ecx(vm, function, pc)||
		opcode_mov_ptr_ecx_to_edx(vm, function, pc)||
		opcode_sub_immediate_reg(vm, function, pc, (void*)0x08, reg_ECX)||
		opcode_mov_ecx_to_ptr_eax(vm, function, pc)||
		opcode_cmp_immediate_reg(vm, function, pc, (void*)0x01, reg_EDX)) return VM_ERROR;
	result_jcc=*pc;
	*pc+=s_JCC_FULL_DISPLACEMENT;

	n=parameter_list_get_number(OBJECT_GET_CELL(&plist));
	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&iteration), function, pc, ret)||
		t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, (void*)n)||
		opcode_call_direct(vm, function, pc, for_iteration)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	ret_jcc=*pc;
	*pc+=s_JCC_FULL_DISPLACEMENT;
	loop_jmp=*pc;
	*pc+=s_JMP_FULL_DISPLACEMENT;
	// RESULT
	result_dest=*pc;
	if (t3_code_list_write_code_(vm, OBJECT_GET_CELL(&result), function, pc, ret)||
		t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&plist))||
		opcode_call_direct(vm, function, pc, for_result)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	ret_jcc_2=*pc;
	*pc+=s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, &result_jcc, tttn_NZ, (void*)(result_dest-result_jcc-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	if (opcode_jcc_full_displacement(vm, function, &ret_jcc, tttn_NZ, (void*)(ret_dest-ret_jcc-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	if (opcode_jcc_full_displacement(vm, function, &ret_jcc_2, tttn_NZ, (void*)(ret_dest-ret_jcc_2-s_JCC_FULL_DISPLACEMENT))) return VM_ERROR;
	if (opcode_jmp_full_displacement(vm, function, &loop_jmp, (void*)(loop_dest-loop_jmp-s_JMP_FULL_DISPLACEMENT))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT block, tag;
	tPCELL f;
	tINT jmp;
	// 引数の読込み
	t3_read_argument_2(head, &block, &tag);
	// 関数の作成
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&block), &f)) return VM_ERROR;
	cell_to_object(f, &block);
	if (function_add_use_object(vm, function, &block)) return VM_ERROR;
	// コードの生成
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&tag))||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&block))||
		opcode_call_direct(vm, function, pc, (void*)op_block)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT tag;
	t3_read_argument_1(head, &tag);
	if (function_add_use_object(vm, function, &tag)) return VM_ERROR;
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&tag))||
		opcode_call_direct(vm, function, pc, (void*)op_return_from)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT body;
	tPCELL f;
	tINT jmp;
	t3_read_argument_1(head, &body);
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &f)) return VM_ERROR;
	cell_to_object(f, &body);
	if (function_add_use_object(vm, function, &body)) return VM_ERROR;
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&body))||
		opcode_call_direct(vm, function, pc, (void*)op_catch)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	code_list_increment_head(head);
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_call_direct(vm, function, pc, (void*)op_throw)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT taglist, flist;
	tPCELL list;
	tINT jmp;

	code_list_increment_head(head);
	cons_get_car(*head, &taglist);
	code_list_increment_head(head);
	cons_get_car(*head, &flist);
	code_list_increment_head(head);

	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&taglist))||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&flist))||
		opcode_call_direct(vm, function, pc, op_tagbody)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;

	if (function_add_use_object(vm, function, &flist)) return VM_ERROR;
	for (list=OBJECT_GET_CELL(&flist); list; list=cons_get_cdr_cons(list)) {
		tOBJECT f;
		tPCELL p;
		cons_get_car(list, &f);
		if (t3_clist_to_function(vm, OBJECT_GET_CELL(&f), &p)) return VM_ERROR;
		cell_to_object(p, &f);
		cons_set_car(list, &f);
	}
	return VM_OK;
}

static VM_RET t3_s_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT tag;

	t3_read_argument_1(head, &tag);
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&tag))||
		opcode_call_direct(vm, function, pc, op_go)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	if (function_add_use_object(vm, function, &tag)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_unwind_protect(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_unwind_protect(tPVM vm, tPCELL clist, const tINT Code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT body, clean_up;
	tPCELL p;
	tINT jmp;
	t3_read_argument_2(head, &body, &clean_up);
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &p)) return VM_ERROR;
	cell_to_object(p, &body);
	if (function_add_use_object(vm, function, &body)) return VM_ERROR;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&clean_up), &p)) return VM_ERROR;
	cell_to_object(p, &clean_up);
	if (function_add_use_object(vm, function, &clean_up)) return VM_ERROR;
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&body))||
		opcode_push_immediate(vm, function, pc, (void*)OBJECT_GET_CELL(&clean_up))||
		opcode_call_direct(vm, function, pc, (void*)op_unwind_protect)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT plist, func;
	tPCELL p;
	void* procedure;
	tINT jmp;
	t3_read_argument_2(head, &plist, &func);
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&func), &p)) return VM_ERROR;
	cell_to_object(p, &func);
	if (function_add_use_object(vm, function, &func)) return VM_ERROR;
	if (function_add_use_object(vm, function, &plist)) return VM_ERROR;
	switch (code) {
	case iiWITH_OPEN_INPUT_FILE:	procedure=op_with_open_input_file;	break;
	case iiWITH_OPEN_OUTPUT_FILE:	procedure=op_with_open_output_file;	break;
	case iiWITH_OPEN_IO_FILE:		procedure=op_with_open_io_file;		break;
	default:						return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, OBJECT_GET_CELL(&plist))||
		opcode_push_immediate(vm, function, pc, p)||
		opcode_call_direct(vm, function, pc, procedure)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

static VM_RET t3_s_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);

	t3_s_vm_copy(vm, size);
	*size+=s_CALL_DIRECT+
		   s_POP_REG+
		   s_RET;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	code_list_increment_head(head);

	if (t3_w_vm_copy(vm, function, pc)||
		opcode_call_direct(vm,function, pc, t3_table[code].data1)||
		opcode_pop_reg(vm, function, pc, reg_EBX)||
		opcode_ret(vm, function, pc)) return VM_ERROR;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_s_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	//
	t3_s_vm_copy(vm, size);
	*size+=s_PUSH_IMMEDIATE+
		   s_CALL_DIRECT+
		   s_AND_REG_REG+
		   s_JCC_FULL_DISPLACEMENT;
	return VM_OK;
}

static VM_RET t3_w_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc, tINT ret)
{
	tOBJECT body, obj;
	tPCELL f;
	tINT jmp;

	t3_read_argument_1(head, &body);
	// 関数の作成
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &f)) return VM_ERROR;
	cell_to_object(f, &obj);
	if (function_add_use_object(vm, function, &obj)) return VM_ERROR;
	if (t3_w_vm_copy(vm, function, pc)||
		opcode_push_immediate(vm, function, pc, (void*)f)||
		opcode_call_direct(vm, function, pc, (void*)t3_table[code].data1)||
		opcode_and_reg_reg(vm, function, pc, reg_EAX, reg_EAX)) return VM_ERROR;
	jmp=*pc+s_JCC_FULL_DISPLACEMENT;
	if (opcode_jcc_full_displacement(vm, function, pc, tttn_NZ, (void*)(ret-jmp))) return VM_ERROR;
	return VM_OK;
}

//////////////////////////////

static void t3_s_vm_copy(tPVM vm, tINT* size)
{
	*size+=s_POP_REG+s_PUSH_REG+s_PUSH_REG;
}

static VM_RET t3_w_vm_copy(tPVM vm, tPCELL function, tINT* pc)
{
	return opcode_pop_reg(vm, function, pc, reg_EBX)||
		   opcode_push_reg(vm, function, pc, reg_EBX)||
		   opcode_push_reg(vm, function, pc, reg_EBX);
}
