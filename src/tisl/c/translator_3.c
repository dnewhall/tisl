//
// TISL/src/tisl/c/translator.c
// TISL Ver. 4.0
//

#include "../../../include/tni.h"
#include "../object.h"
#include "../vm.h"
#include "../tisl.h"
#include "../translator.h"
#include "../command.h"
#include "../operation.h"
#include "opcode.h"

/////////////////////////////

// c/function.c で定義
extern VM_RET function_write_command(tPVM vm, tPCELL function, const tINT pc, void* command);
extern tTRANSLATOR vm_get_translator(tPVM vm);

/////////////////////////////

static VM_RET t3_clist_to_function(tPVM vm, tPCELL clist, tPCELL* function);
static VM_RET t3_flist_to_function(tPVM vm, tPCELL flist, tPCELL* function);

static VM_RET t3_code_list_get_size(tPVM vm, tPCELL clist, tINT* size);
static VM_RET t3_code_list_get_size_(tPVM vm, tPCELL clist, tINT* size);
static VM_RET t3_code_list_write_code(tPVM vm, tPCELL clist, tPCELL function);
static VM_RET t3_code_list_write_code_(tPVM vm, tPCELL clist, tPCELL function, tINT* pc);

static void t3_read_argument_1(tPCELL* head, tPOBJECT arg1);
static void t3_read_argument_2(tPCELL* head, tPOBJECT arg1, tPOBJECT arg2);

///////////////////
// dummy unknown operation?
static VM_RET t3_get_size_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_write_code_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiDISACRD
static VM_RET t3_s_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCODE
static VM_RET t3_s_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiPUSH_OBJECT obj
static VM_RET t3_s_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCODE operand
static VM_RET t3_s_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
static VM_RET t3_w_type_2_(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCODE operand1 operand2
static VM_RET t3_s_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCALL_TAIL_REC
static VM_RET t3_s_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCALL_TAIL
static VM_RET t3_s_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCALL_LOCAL offset flist
static VM_RET t3_s_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCALL_LOCAL_TAIL offset flist
static VM_RET t3_s_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiRET
static VM_RET t3_s_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiPUSH_LOCAL_FUNCTON offset flist
static VM_RET t3_s_push_lfunction(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_push_lfunction(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiPUSH_LAMBDA flist
static VM_RET t3_s_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiLABELS_IN nlist function-list-1 ... function-list-n
// iiFLET_IN nlist function-list-1 ... function-list-n
static VM_RET t3_s_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiAND
static VM_RET t3_s_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiDYNAMIC_LET
static VM_RET t3_s_dynamic_let(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_dynamic_let(tPVM vm, tPCELL clist, const tINT ocde, tPCELL* head, tPCELL function, tINT* pc);
// iiIF
static VM_RET t3_s_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCASE
static VM_RET t3_s_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCASE_USING
static VM_RET t3_s_case_using(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_case_using(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiWHILE clist clist
static VM_RET t3_s_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiFOR plist enttest result iteration
static VM_RET t3_s_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiBLOCK clist tag
static VM_RET t3_s_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiRETURN_FROM tag
static VM_RET t3_s_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCATCH clist
static VM_RET t3_s_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiTHROW
static VM_RET t3_s_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiTAGBODY tag-list code-list ... code-list
static VM_RET t3_s_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiGO (tag . tag-list)
static VM_RET t3_s_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiUNWIND_PROTECT clist clist
static VM_RET t3_s_unwind_protect(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_unwind_protect(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiWITH_OPEN_INPUT_FILE plist clist
// iiWITH_OPEN_OUTPUT_FILE plist clist
// iiWITH_OPEN_IO_FILE plist clist
static VM_RET t3_s_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiCONTINUE_CONDITION
static VM_RET t3_s_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);
// iiWITH_HANDLER
static VM_RET t3_s_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size);
static VM_RET t3_w_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc);

///////////////////

typedef VM_RET (*T3_GET_SIZE)(tPVM, tPCELL, const tINT, tPCELL*, tINT*);
typedef VM_RET (*T3_WRITE_CODE)(tPVM, tPCELL, const tINT, tPCELL*, tPCELL, tINT*);
typedef struct T3_FUNCTION_ T3_FUNCTION;

struct T3_FUNCTION_ {
	T3_GET_SIZE		get_size;
	T3_WRITE_CODE	write_code;
	void*			data1;
};

///////////////////

const T3_FUNCTION t3_table[]={
	{ t3_s_discard,			t3_w_discard,			c_discard				 },// iiDISCARD
	{ t3_s_type_1,			t3_w_type_1,			c_push_nil				 },// iiPUSH_NIL
	{ t3_s_type_1,			t3_w_type_1,			c_push_t				 },// iiPUSH_T
	{ t3_s_push_object,		t3_w_push_object,		0						 },// iiPUSH_OBJECT obj
	{ t3_s_type_2,			t3_w_type_2,			c_push_stack			 },// iiPUSH_STACK offset
	{ t3_s_type_2,			t3_w_type_2,			c_push_heap				 },// iiPUSH_HEAP offset
	{ t3_s_type_2,			t3_w_type_2,			c_push_variable			 },// iiPUSH_VARAIABLE bind-list
	{ t3_s_type_1,			t3_w_type_1,			c_call_rec				 },// iiCALL_REC
	{ t3_s_call_tail_rec,	t3_w_call_tail_rec,		0						 },// iiCALL_TAIL_REC
	{ t3_s_type_3,			t3_w_type_3,			c_call					 },// iiCALL bind-list anum
	{ t3_s_call_tail,		t3_w_call_tail,			c_call_tail				 },// iiCALL_TAIL bind-list anum
	{ t3_s_type_3,			t3_w_type_3,			c_call_bind				 },// iiCALL_BIND bind anum
	{ t3_s_call_tail,		t3_w_call_tail,			c_call_bind_tail		 },// iiCALL_BIND_TAIL bind anum
	{ t3_s_call_local,		t3_w_call_local,		0						 },// iiCALL_LOCAL offset flist
	{ t3_s_call_local_tail,	t3_w_call_local_tail,	0						 },// iiCALL_LOCAL_TAIL offset flist
	//
	{ t3_s_ret,				t3_w_ret,				0						 },// iiRET 
	{ t3_s_type_2,			t3_w_type_2_,			c_lambda_in				 },// iiLAMBDA_IN plist
	{ t3_s_type_2,			t3_w_type_2,			c_lambda_out			 },// iiLAMBDA_OUT plist
	{ t3_s_type_2,			t3_w_type_2_,			c_lambda_heap_in		 },// iiLAMBDA_HEAP_IN plist
	{ t3_s_type_2,			t3_w_type_2,			c_lambda_heap_out		 },// iiLAMBDA_HAEP_OUT plist
	{ t3_s_type_2,			t3_w_type_2,			c_push_function			 },// iiPUSH_FUNCTION bind-list
	{ t3_s_push_lfunction,	t3_w_push_lfunction,	0						 },// iiPUSH_LOCAL_FUNCTION offset flist
	{ t3_s_push_lambda,		t3_w_push_lambda,		0						 },// iiPUSH_LAMBDA flist
	{ t3_s_labels,			t3_w_labels,			c_labels_in				 },// iiLABELS_IN nlist function-list-1 ... function-list-n
	{ t3_s_type_1,			t3_w_type_1,			c_labels_out			 },// iiLABELS_OUT
	{ t3_s_labels,			t3_w_labels,			c_flet_in				 },// iiFLET_IN nlist function-list-1 ... function-list-n
	{ t3_s_type_1,			t3_w_type_1,			c_flet_out				 },// iiFLET_OUT
	{ t3_s_and,				t3_w_and,				c_and					 },// iiAND clist
	{ t3_s_and,				t3_w_and,				c_or					 },// iiOR clist
	{ t3_s_type_2,			t3_w_type_2,			c_set_stack				 },// iiSET_STACK offset
	{ t3_s_type_2,			t3_w_type_2,			c_set_heap				 },// iiSET_HEAP offset
	{ t3_s_type_2,			t3_w_type_2,			c_set_variable			 },// iiSET_VARIABLE bind-list
	{ t3_s_type_2,			t3_w_type_2,			c_set_dynamic			 },// iiSET_DYNAMIC symbol
	{ t3_s_type_2,			t3_w_type_2,			c_set_aref				 },// iiSET_AREF
	{ t3_s_type_2,			t3_w_type_2,			c_set_garef				 },// iiSET_GAREF
	{ t3_s_type_1,			t3_w_type_1,			c_set_elt				 },// iiSET_ELT
	{ t3_s_type_1,			t3_w_type_1,			c_set_property			 },// iiSET_PROPERTY
	{ t3_s_type_1,			t3_w_type_1,			c_set_car				 },// iiSET_CAR
	{ t3_s_type_1,			t3_w_type_1,			c_set_cdr				 },// iiSET_CDR
	{ t3_s_type_2,			t3_w_type_2,			c_set_accessor			 },// iiSET_ACCESSOR name
	{ t3_s_type_2,			t3_w_type_2,			c_push_dynamic			 },// iiPUSH_DYNAMIC name
	{ t3_s_dynamic_let,		t3_w_dynamic_let,		0						 },// iiDYNAMIC_LET n name-1 ... nmae-n code-list
	{ t3_s_if,				t3_w_if,				0						 },// iiIF code-list code-list
	{ t3_s_case,			t3_w_case,				c_case					 },// iiCASE n key-list1 clist1 ... key-listn clistn
	{ t3_s_case,			t3_w_case,				c_case_using			 },// iiCASE_USING n key-list1 clist1 ... key-listn clistn
	{ t3_s_while,			t3_w_while,				0						 },// iiWHITE clist clist
	{ t3_s_for,				t3_w_for,				c_for_stack				 },// iiFOR_STACK plist endtest result iteration
	{ t3_s_for,				t3_w_for,				c_for_heap				 },// iiFOR_HEAP plist endtest result iteration
	{ t3_s_block,			t3_w_block,				0						 },// iiBLOCK clist tag
	{ t3_s_return_from,		t3_w_return_from,		0						 },// iiRETURN_FROM tag
	{ t3_s_catch,			t3_w_catch,				0						 },// iiCATCH clist
	{ t3_s_throw,			t3_w_throw,				0						 },// iiTHROW
	{ t3_s_tagbody,			t3_w_tagbody,			0						 },// iiTAGBODY tag-list code-list ... code-list
	{ t3_s_go,				t3_w_go,				0						 },// iiGO (tag . tag-list)
	{ t3_s_unwind_protect,	t3_w_unwind_protect,	0						 },// iiUNWIND_PROTECT clist clist
	{ t3_s_type_2,			t3_w_type_2,			c_class					 },// iiPUSH_CLASS bind-list
	{ t3_s_type_2,			t3_w_type_2,			c_the					 },// iiTHE bind-list
	{ t3_s_type_2,			t3_w_type_2,			c_assure				 },// iiASSURE bind-list
	{ t3_s_type_2,			t3_w_type_2,			c_convert				 },// iiCONVERT bind-list
	{ t3_s_type_f,			t3_w_type_f,			c_with_standard_input	 },// iiWITH_STANDARD_INPUT code-list
	{ t3_s_type_f,			t3_w_type_f,			c_with_standard_output	 },// iiWITH_STANDARD_OUTPUT code_list
	{ t3_s_type_f,			t3_w_type_f,			c_with_error_output		 },// iiWITH_ERROR_OUTPUT code_list
	{ t3_s_with_open_file,	t3_w_with_open_file,	c_with_open_input_file	 },// iiWITH_OPEN_INPUT_FILE plist clist
	{ t3_s_with_open_file,	t3_w_with_open_file,	c_with_open_output_file	 },// iiWITH_OPEN_OUTPUT_FILE plist clist
	{ t3_s_with_open_file,	t3_w_with_open_file,	c_with_open_io_file		 },// iiWITH_OPEN_IO_FILE plist clist
	{ t3_s_type_f,			t3_w_type_f,			c_ignore_errors			 },// iiIGNORE_ERRORS clist
	{ t3_s_continue,		t3_w_continue,			0						 },// iiCONTNUE_CONDITION
	{ t3_s_type_f,			t3_w_type_f,			c_with_handler			 },// iiWITH_HANDLER clist
	{ t3_s_type_f,			t3_w_type_f,			c_time					 },// iiTMIE
	{ t3_s_type_1,			t3_w_type_1,			c_quasiquote			 },// iiQUASIQUOTE
	{ t3_s_type_1,			t3_w_type_1,			c_quasiquote2			 },// iiQUASIQUOTE2
	{ t3_s_type_1,			t3_w_type_1,			c_unquote				 },// iiUNQUOTE
	{ t3_s_type_1,			t3_w_type_1,			c_unquote_splicing		 },// iiUNQUOTE_SPLICING
	{ t3_s_type_1,			t3_w_type_1,			c_unquote_splicing2		 },// iiUNQUOTE_SPLICING2
	{ t3_s_type_1,			t3_w_type_1,			c_functionp				 },
	{ t3_s_type_2,			t3_w_type_2,			c_apply					 },
	{ t3_s_type_2,			t3_w_type_2,			c_funcall				 },
	{ t3_s_type_1,			t3_w_type_1,			c_eq					 },
	{ t3_s_type_1,			t3_w_type_1,			c_eql					 },
	{ t3_s_type_1,			t3_w_type_1,			c_equal					 },
	{ t3_s_type_1,			t3_w_type_1,			c_not					 },
	{ t3_s_type_1,			t3_w_type_1,			c_generic_function_p	 },
	{ t3_s_type_1,			t3_w_type_1,			c_class_of				 },
	{ t3_s_type_1,			t3_w_type_1,			c_instancep				 },
	{ t3_s_type_1,			t3_w_type_1,			c_subclassp				 },
	{ t3_s_type_1,			t3_w_type_1,			c_symbolp				 },
	{ t3_s_type_2,			t3_w_type_2,			c_property				 },
	{ t3_s_type_2,			t3_w_type_2,			c_remove_property		 },
	{ t3_s_type_1,			t3_w_type_1,			c_gensym				 },
	{ t3_s_type_1,			t3_w_type_1,			c_numberp				 },
	{ t3_s_type_1,			t3_w_type_1,			c_parse_number			 },
	{ t3_s_type_1,			t3_w_type_1,			c_number_equal			 },
	{ t3_s_type_1,			t3_w_type_1,			c_number_not_equal		 },
	{ t3_s_type_1,			t3_w_type_1,			c_number_ge			 },
	{ t3_s_type_1,			t3_w_type_1,			c_number_le			 },
	{ t3_s_type_1,			t3_w_type_1,			c_number_greater		 },
	{ t3_s_type_1,			t3_w_type_1,			c_number_less			 },
	{ t3_s_type_2,			t3_w_type_2,			c_addition				 },
	{ t3_s_type_2,			t3_w_type_2,			c_multiplication		 },
	{ t3_s_type_2,			t3_w_type_2,			c_substraction			 },
	{ t3_s_type_2,			t3_w_type_2,			c_quotient				 },
	{ t3_s_type_1,			t3_w_type_1,			c_reciprocal			 },
	{ t3_s_type_2,			t3_w_type_2,			c_max					 },
	{ t3_s_type_2,			t3_w_type_2,			c_min					 },
	{ t3_s_type_1,			t3_w_type_1,			c_abs					 },
	{ t3_s_type_1,			t3_w_type_1,			c_exp					 },
	{ t3_s_type_1,			t3_w_type_1,			c_log					 },
	{ t3_s_type_1,			t3_w_type_1,			c_expt					 },
	{ t3_s_type_1,			t3_w_type_1,			c_sqrt					 },
	{ t3_s_type_1,			t3_w_type_1,			c_sin					 },
	{ t3_s_type_1,			t3_w_type_1,			c_cos					 },
	{ t3_s_type_1,			t3_w_type_1,			c_tan					 },
	{ t3_s_type_1,			t3_w_type_1,			c_atan					 },
	{ t3_s_type_1,			t3_w_type_1,			c_atan2					 },
	{ t3_s_type_1,			t3_w_type_1,			c_sinh					 },
	{ t3_s_type_1,			t3_w_type_1,			c_cosh					 },
	{ t3_s_type_1,			t3_w_type_1,			c_tanh					 },
	{ t3_s_type_1,			t3_w_type_1,			c_atanh					 },
	{ t3_s_type_1,			t3_w_type_1,			c_floatp				 },
	{ t3_s_type_1,			t3_w_type_1,			c_float					 },
	{ t3_s_type_1,			t3_w_type_1,			c_floor					 },
	{ t3_s_type_1,			t3_w_type_1,			c_ceiling				 },
	{ t3_s_type_1,			t3_w_type_1,			c_truncate				 },
	{ t3_s_type_1,			t3_w_type_1,			c_round					 },
	{ t3_s_type_1,			t3_w_type_1,			c_integerp				 },
	{ t3_s_type_1,			t3_w_type_1,			c_div					 },
	{ t3_s_type_1,			t3_w_type_1,			c_mod					 },
	{ t3_s_type_1,			t3_w_type_1,			c_gcd					 },
	{ t3_s_type_1,			t3_w_type_1,			c_lcm					 },
	{ t3_s_type_1,			t3_w_type_1,			c_isqrt					 },
	{ t3_s_type_1,			t3_w_type_1,			c_characterp			 },
	{ t3_s_type_1,			t3_w_type_1,			c_char_equal			 },
	{ t3_s_type_1,			t3_w_type_1,			c_char_not_equal		 },
	{ t3_s_type_1,			t3_w_type_1,			c_char_less			 },
	{ t3_s_type_1,			t3_w_type_1,			c_char_greater			 },
	{ t3_s_type_1,			t3_w_type_1,			c_char_le				 },
	{ t3_s_type_1,			t3_w_type_1,			c_char_ge				 },
	{ t3_s_type_1,			t3_w_type_1,			c_consp				 },
	{ t3_s_type_1,			t3_w_type_1,			c_cons					 },
	{ t3_s_type_1,			t3_w_type_1,			c_car					 },
	{ t3_s_type_1,			t3_w_type_1,			c_cdr					 },
	{ t3_s_type_1,			t3_w_type_1,			c_null					 },
	{ t3_s_type_1,			t3_w_type_1,			c_listp				 },
	{ t3_s_type_2,			t3_w_type_2,			c_create_list			 },
	{ t3_s_type_2,			t3_w_type_2,			c_list					 },
	{ t3_s_type_1,			t3_w_type_1,			c_reverse				 },
	{ t3_s_type_1,			t3_w_type_1,			c_nreverse				 },
	{ t3_s_type_2,			t3_w_type_2,			c_append				 },
	{ t3_s_type_1,			t3_w_type_1,			c_member				 },
	{ t3_s_type_2,			t3_w_type_2,			c_mapcar				 },
	{ t3_s_type_2,			t3_w_type_2,			c_mapc					 },
	{ t3_s_type_2,			t3_w_type_2,			c_mapcan				 },
	{ t3_s_type_2,			t3_w_type_2,			c_maplist				 },
	{ t3_s_type_2,			t3_w_type_2,			c_mapl					 },
	{ t3_s_type_2,			t3_w_type_2,			c_mapcon				 },
	{ t3_s_type_1,			t3_w_type_1,			c_assoc				 },
	{ t3_s_type_1,			t3_w_type_1,			c_basic_array_p		 },
	{ t3_s_type_1,			t3_w_type_1,			c_basic_array_a_p		 },
	{ t3_s_type_1,			t3_w_type_1,			c_general_array_a_p	 },
	{ t3_s_type_2,			t3_w_type_2,			c_create_array			 },
	{ t3_s_type_2,			t3_w_type_2,			c_aref					 },
	{ t3_s_type_2,			t3_w_type_2,			c_garef				 },
	{ t3_s_type_1,			t3_w_type_1,			c_array_dimensions		 },
	{ t3_s_type_1,			t3_w_type_1,			c_basic_vector_p		 },
	{ t3_s_type_1,			t3_w_type_1,			c_general_vector_p		 },
	{ t3_s_type_2,			t3_w_type_2,			c_create_vector		 },
	{ t3_s_type_2,			t3_w_type_2,			c_vector				 },
	{ t3_s_type_1,			t3_w_type_1,			c_stringp				 },
	{ t3_s_type_2,			t3_w_type_2,			c_create_string		 },
	{ t3_s_type_1,			t3_w_type_1,			c_string_equal			 },
	{ t3_s_type_1,			t3_w_type_1,			c_string_not_equal		 },
	{ t3_s_type_1,			t3_w_type_1,			c_string_less			 },
	{ t3_s_type_1,			t3_w_type_1,			c_string_greater		 },
	{ t3_s_type_1,			t3_w_type_1,			c_string_ge			 },
	{ t3_s_type_1,			t3_w_type_1,			c_string_le			 },
	{ t3_s_type_2,			t3_w_type_2,			c_char_index			 },
	{ t3_s_type_2,			t3_w_type_2,			c_string_index			 },
	{ t3_s_type_2,			t3_w_type_2,			c_string_append		 },
	{ t3_s_type_1,			t3_w_type_1,			c_length				 },
	{ t3_s_type_1,			t3_w_type_1,			c_elt					 },
	{ t3_s_type_1,			t3_w_type_1,			c_subseq				 },
	{ t3_s_type_2,			t3_w_type_2,			c_map_into				 },
	{ t3_s_type_1,			t3_w_type_1,			c_streamp				 },
	{ t3_s_type_1,			t3_w_type_1,			c_open_stream_p		 },
	{ t3_s_type_1,			t3_w_type_1,			c_input_stream_p		 },
	{ t3_s_type_1,			t3_w_type_1,			c_output_stream_p		 },
	{ t3_s_type_1,			t3_w_type_1,			c_standard_input		 },
	{ t3_s_type_1,			t3_w_type_1,			c_standard_output		 },
	{ t3_s_type_1,			t3_w_type_1,			c_error_output			 },
	{ t3_s_type_2,			t3_w_type_2,			c_open_input_file		 },
	{ t3_s_type_2,			t3_w_type_2,			c_open_output_file		 },
	{ t3_s_type_2,			t3_w_type_2,			c_open_io_file			 },
	{ t3_s_type_1,			t3_w_type_1,			c_close				 },
	{ t3_s_type_1,			t3_w_type_1,			c_finish_output		 },
	{ t3_s_type_1,			t3_w_type_1,			c_create_string_input_stream },
	{ t3_s_type_1,			t3_w_type_1,			c_create_string_output_stream },
	{ t3_s_type_1,			t3_w_type_1,			c_get_output_stream_string },
	{ t3_s_type_2,			t3_w_type_2,			c_read					 },
	{ t3_s_type_2,			t3_w_type_2,			c_read_char			 },
	{ t3_s_type_2,			t3_w_type_2,			c_preview_char			 },
	{ t3_s_type_2,			t3_w_type_2,			c_read_line			 },
	{ t3_s_type_1,			t3_w_type_1,			c_stream_ready_p		 },
	{ t3_s_type_2,			t3_w_type_2,			c_format				 },
	{ t3_s_type_1,			t3_w_type_1,			c_format_char			 },
	{ t3_s_type_1,			t3_w_type_1,			c_format_float			 },
	{ t3_s_type_1,			t3_w_type_1,			c_format_fresh_line	 },
	{ t3_s_type_1,			t3_w_type_1,			c_format_integer		 },
	{ t3_s_type_1,			t3_w_type_1,			c_format_object		 },
	{ t3_s_type_1,			t3_w_type_1,			c_format_tab			 },
	{ t3_s_type_2,			t3_w_type_2,			c_read_byte			 },
	{ t3_s_type_1,			t3_w_type_1,			c_write_byte			 },
	{ t3_s_type_1,			t3_w_type_1,			c_probe_file			 },
	{ t3_s_type_1,			t3_w_type_1,			c_file_position		 },
	{ t3_s_type_1,			t3_w_type_1,			c_set_file_position	 },
	{ t3_s_type_1,			t3_w_type_1,			c_file_length			 },
	{ t3_s_type_2,			t3_w_type_2,			c_error				 },
	{ t3_s_type_2,			t3_w_type_2,			c_cerror				 },
	{ t3_s_type_2,			t3_w_type_2,			c_signal_condition		 },
	{ t3_s_type_2,			t3_w_type_2,			c_condition_continuable },
	{ t3_s_type_2,			t3_w_type_2,			c_arithmetic_error_operation },
	{ t3_s_type_2,			t3_w_type_2,			c_arithmetic_error_operand },
	{ t3_s_type_2,			t3_w_type_2,			c_domain_error_object	 },
	{ t3_s_type_2,			t3_w_type_2,			c_domain_error_expected_class },
	{ t3_s_type_2,			t3_w_type_2,			c_parse_error_string	 },
	{ t3_s_type_2,			t3_w_type_2,			c_parse_error_expected_class },
	{ t3_s_type_2,			t3_w_type_2,			c_simple_error_format_string },
	{ t3_s_type_2,			t3_w_type_2,			c_simple_error_format_arguments },
	{ t3_s_type_2,			t3_w_type_2,			c_stream_error_stream	 },
	{ t3_s_type_2,			t3_w_type_2,			c_undefined_entity_name },
	{ t3_s_type_2,			t3_w_type_2,			c_undefined_entity_namespace },
	{ t3_s_type_2,			t3_w_type_2,			c_identity				 },
	{ t3_s_type_2,			t3_w_type_2,			c_get_universal_time	 },
	{ t3_s_type_2,			t3_w_type_2,			c_get_internal_run_time },
	{ t3_s_type_2,			t3_w_type_2,			c_get_internal_real_time },
	{ t3_s_type_2,			t3_w_type_2,			c_get_internal_time_units_per_second },
	{ t3_s_type_2,			t3_w_type_2,			c_system },
	{ t3_s_type_2,			t3_w_type_2,			c_exit },
	{ t3_s_type_2,			t3_w_type_2,			c_strftime },
	{ t3_s_type_2,			t3_w_type_2,			c_get_argument },
	{ t3_s_type_2,			t3_w_type_2,			c_get_environment },
	{ t3_s_continue,		t3_w_continue,			c_arity_error },
	{ t3_s_type_2,			t3_w_type_2,			c_eval },
	{ t3_s_type_3,			t3_w_type_3,			c_number_equal_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_number_equal_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_number_not_equal_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_number_not_equal_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_number_less_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_number_less_integer_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_number_less_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_number_le_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_number_le_integer_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_number_le_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_addition_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_addition_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_substraction_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_substraction_integer_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_substraction_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_eq_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_eq_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_eq_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_eq_stack_stack },
	{ t3_s_type_3,			t3_w_type_3,			c_eq_stack_integer },
	{ t3_s_type_3,			t3_w_type_3,			c_equal_stack_stack },
};

/////////////////////////////

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

/////////////////////////////

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
	if (t3_code_list_write_code(vm, clist, *function)) { vm_pop(vm); return VM_ERROR; }
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
	if (t3_code_list_write_code(vm, clist, *function)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

static VM_RET t3_code_list_get_size(tPVM vm, tPCELL clist, tINT* size)
{
	*size=0;
	return t3_code_list_get_size_(vm, clist, size);
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

static VM_RET t3_code_list_write_code(tPVM vm, tPCELL clist, tPCELL function)
{
	tINT pc=0;
	return t3_code_list_write_code_(vm, clist, function, &pc);
}

static VM_RET t3_code_list_write_code_(tPVM vm, tPCELL clist, tPCELL function, tINT* pc)
{
	tPCELL head;
	tINT command;
	head=code_list_get_head(clist);
	while (command=code_list_get_command(head),
		!(*t3_table[command].write_code)(vm, clist, command, &head, function, pc));
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

/////////////////////////////

// dummy unknown operation?
static VM_RET t3_get_size_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

static VM_RET t3_write_code_dummy(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

// iiDISACRD
static VM_RET t3_s_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=1;
	return VM_OK;
}

static VM_RET t3_w_discard(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	code_list_increment_head(head);
	if (function_write_command(vm, function, *pc, c_discard)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCODE
static VM_RET t3_s_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=1;
	return VM_OK;
}

static VM_RET t3_w_type_1(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	code_list_increment_head(head);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiPUSH_OBJECT obj
static VM_RET t3_s_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);

	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_push_object(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT obj;
	void* procedure;
	// 引数の読込み
	t3_read_argument_1(head, &obj);

	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:			procedure=c_push_integer;			break;
	case OBJECT_FLOAT:				procedure=c_push_float;			break;
	case OBJECT_CHARACTER:			procedure=c_push_character;		break;
	case OBJECT_CONS:				procedure=c_push_cons;				break;
	case OBJECT_STRING:				procedure=c_push_string;			break;
	case OBJECT_SYMBOL:				procedure=c_push_symbol;			break;
	case OBJECT_VECTOR:				procedure=c_push_vector;			break;
	case OBJECT_ARRAY:				procedure=c_push_array;			break;
	default:
		if (OBJECT_IS_CELL(&obj)) {
			procedure=c_push_cell_object;
		} else {
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	}
	if (function_write_command(vm, function, *pc, procedure)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, obj.data.p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCODE operand
static VM_RET t3_s_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);

	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_type_2(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT operand;
	t3_read_argument_1(head, &operand);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, operand.data.p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

static VM_RET t3_w_type_2_(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT operand;
	t3_read_argument_1(head, &operand);
	if (function_add_use_object(vm, function, &operand)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, operand.data.p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCODE operand1 operand2
static VM_RET t3_s_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);

	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_type_3(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT op1, op2;
	t3_read_argument_2(head, &op1, &op2);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, op1.data.p)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, op2.data.p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCALL_TAIL_REC
static VM_RET t3_s_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=1;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_call_tail_rec(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	code_list_increment_head(head);
	if (function_write_command(vm, function, *pc, c_call_rec_tail)) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiCALL_TAIL
static VM_RET t3_s_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_call_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT b, anum;
	t3_read_argument_2(head, &b, &anum);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, b.data.p)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, anum.data.p)) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiCALL_LOCAL offset flist
static VM_RET t3_s_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_call_local(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT offset, flist;
	tPCELL p;
	void* procedure;

	t3_read_argument_2(head, &offset, &flist);
	p=OBJECT_GET_CELL(&flist);
	if (flist_is_call_next_method(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=c_call_next_method_around; break;
		case METHOD_PRIMARY:	procedure=c_call_next_method_primary; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else if (flist_is_next_method_p(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=c_next_method_p_around; break;
		case METHOD_PRIMARY:	procedure=c_next_method_p_primary; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else {
		p=function_list_get_function(OBJECT_GET_CELL(&flist));
		cell_to_object(p, &flist);
		procedure = function_is_heap(p) ? c_call_local_heap : c_call_local_stack;
	}
	if (function_write_command(vm, function, *pc, procedure)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, offset.data.p)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCALL_LOCAL_TAIL offset flist
static VM_RET t3_s_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_call_local_tail(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT offset, flist;
	tPCELL p;
	void* procedure;

	t3_read_argument_2(head, &offset, &flist);
	p=OBJECT_GET_CELL(&flist);
	if (flist_is_call_next_method(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=c_call_next_method_around_tail; break;
		case METHOD_PRIMARY:	procedure=c_call_next_method_primary_tail; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else if (flist_is_next_method_p(vm, p)) {
		switch (t2_get_method_qualifier(vm)) {
		case METHOD_AROUND:		procedure=c_next_method_p_around_tail; break;
		case METHOD_PRIMARY:	procedure=c_next_method_p_primary_tail; break;
		default:				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		OBJECT_SET_INTEGER(&offset, 0);
	} else {
		p=function_list_get_function(OBJECT_GET_CELL(&flist));
		cell_to_object(p, &flist);
		procedure = function_is_heap(p) ? c_call_local_heap_tail : c_call_local_stack_tail;
	}
	if (function_write_command(vm, function, *pc, procedure)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, offset.data.p)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiRET
static VM_RET t3_s_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=1;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_ret(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	code_list_increment_head(head);
	if (function_write_command(vm, function, *pc, c_ret)) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiPUSH_LOCAL_FUNCTON offset flist
static VM_RET t3_s_push_lfunction(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_push_lfunction(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT offset, flist;
	tPCELL p;
	t3_read_argument_2(head, &offset, &flist);
	p=function_list_get_function(OBJECT_GET_CELL(&flist));
	if (function_write_command(vm, function, *pc, c_push_lfunction)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, offset.data.p)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiPUSH_LAMBDA flist
static VM_RET t3_s_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_push_lambda(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT flist;
	tPCELL p;
	t3_read_argument_1(head, &flist);
	if (t3_flist_to_function(vm, OBJECT_GET_CELL(&flist), &p)) return VM_ERROR;
	cell_to_object(p, &flist);
	if (function_add_use_object(vm, function, &flist)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, c_push_lambda)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiLABELS_IN nlist function-list-1 ... function-list-n
// iiFLET_IN nlist function-list-1 ... function-list-n
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
	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_labels(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT nlist, obj;
	tINT i, n;
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
		if (t3_code_list_write_code(vm, clist, func)) return VM_ERROR;
		cell_to_object(func, &obj);
		if (function_add_use_object(vm, function, &obj)) return VM_ERROR;
	}
	code_list_increment_head(head);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&nlist))) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiAND
static VM_RET t3_s_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_and(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT obj;
	tPCELL p;
	t3_read_argument_1(head, &obj);
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&obj), &p)) return VM_ERROR;
	cell_to_object(p, &obj);
	if (function_add_use_object(vm, function, &obj)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiDYNAMIC_LET
static VM_RET t3_s_dynamic_let(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tINT i, n;
	tOBJECT N;
	code_list_increment_head(head);
	cons_get_car(*head, &N);
	n=OBJECT_GET_INTEGER(&N);
	for (i=0; i<=n; i++) {
		code_list_increment_head(head);
	}
	code_list_increment_head(head);
	*size+=n+3;
	return VM_OK;
}

static VM_RET t3_w_dynamic_let(tPVM vm, tPCELL clist, const tINT ocde, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT N, body;
	tINT i, n;
	tPCELL p;

	code_list_increment_head(head);
	cons_get_car(*head, &N);
	n=OBJECT_GET_INTEGER(&N);
	if (function_write_command(vm, function, *pc, c_dynamic_let)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, N.data.p)) return VM_ERROR;
	++*pc;
	for (i=0; i<n; i++) {
		tOBJECT name;
		code_list_increment_head(head);
		cons_get_car(*head, &name);
		if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&name))) return VM_ERROR;
		++*pc;
	}
	code_list_increment_head(head);
	cons_get_car(*head, &body);
	code_list_increment_head(head);
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &p)) return VM_ERROR;
	cell_to_object(p, &body);
	if (function_add_use_object(vm, function, &body)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiIF
static VM_RET t3_s_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_if(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT then_form, else_form;
	tPCELL p;

	t3_read_argument_2(head, &then_form, &else_form);
	if (function_write_command(vm, function, *pc, c_if)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&then_form), &p)) return VM_ERROR;
	cell_to_object(p, &then_form);
	if (function_add_use_object(vm, function, &then_form)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&else_form), &p)) return VM_ERROR;
	cell_to_object(p, &else_form);
	if (function_add_use_object(vm, function, &else_form)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCASE
static VM_RET t3_s_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	tINT i, n;
	tOBJECT obj;
	code_list_increment_head(head);
	cons_get_car(*head, &obj);
	n=OBJECT_GET_INTEGER(&obj);
	*size+=2;
	for (i=0; i<n; i++) {
		code_list_increment_head(head);
		code_list_increment_head(head);
		*size+=2;
	}
	code_list_increment_head(head);
	return VM_OK;
}

static VM_RET t3_w_case(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tINT i, n;
	tOBJECT obj;
	tPCELL p;
	code_list_increment_head(head);
	cons_get_car(*head, &obj);
	n=OBJECT_GET_INTEGER(&obj);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, obj.data.p)) return VM_ERROR;
	++*pc;
	for (i=0; i<n; i++) {
		code_list_increment_head(head);
		cons_get_car(*head, &obj);
		if (function_write_command(vm, function, *pc, obj.data.p)) return VM_ERROR;
		++*pc;
		code_list_increment_head(head);
		cons_get_car(*head, &obj);
		if (t3_clist_to_function(vm, OBJECT_GET_CELL(&obj), &p)) return VM_ERROR;
		cell_to_object(p, &obj);
		if (function_add_use_object(vm, function, &obj)) return VM_ERROR;
		if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
		++*pc;
	}
	code_list_increment_head(head);
	return VM_OK;
}

// iiWHILE clist clist
static VM_RET t3_s_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_while(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT test, body;
	tPCELL p;

	t3_read_argument_2(head, &test, &body);

	if (function_write_command(vm, function, *pc, c_while)) return VM_ERROR;
	++*pc;

	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&test), &p)) return VM_ERROR;
	cell_to_object(p, &test);
	if (function_add_use_object(vm, function, &test)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;

	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &p)) return VM_ERROR;
	cell_to_object(p, &body);
	if (function_add_use_object(vm, function, &body)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;

	return VM_OK;
}

// iiFOR plist enttest result iteration
static VM_RET t3_s_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=5;
	return VM_OK;
}

static VM_RET t3_w_for(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT plist, endtest, result, iteration;
	tPCELL p;

	code_list_increment_head(head);
	cons_get_car(*head, &plist);
	code_list_increment_head(head);
	cons_get_car(*head, &endtest);
	code_list_increment_head(head);
	cons_get_car(*head, &result);
	code_list_increment_head(head);
	cons_get_car(*head, &iteration);
	code_list_increment_head(head);

	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_add_use_object(vm, function, &plist)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&plist))) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&endtest), &p)) return VM_ERROR;
	cell_to_object(p, &endtest);
	if (function_add_use_object(vm, function, &endtest)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&result), &p)) return VM_ERROR;
	cell_to_object(p, &result);
	if (function_add_use_object(vm, function, &result)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&iteration), &p)) return VM_ERROR;
	cell_to_object(p, &iteration);
	if (function_add_use_object(vm, function, &iteration)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiBLOCK clist tag
static VM_RET t3_s_block(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_block(tPVM vm, tPCELL list, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT clist, tag;
	tPCELL p;
	t3_read_argument_2(head, &clist, &tag);

	if (function_write_command(vm, function, *pc, c_block)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&clist), &p)) return VM_ERROR;
	cell_to_object(p, &clist);
	if (function_add_use_object(vm, function, &clist)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, tag.data.p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiRETURN_FROM tag
static VM_RET t3_s_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=2;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_return_from(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT tag;
	t3_read_argument_1(head, &tag);

	if (function_add_use_object(vm, function, &tag)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, c_return_from)) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, tag.data.p)) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiCATCH clist
static VM_RET t3_s_catch(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_catch(tPVM vm, tPCELL list, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT clist;
	tPCELL p;
	t3_read_argument_1(head, &clist);
	if (function_write_command(vm, function, *pc, c_catch)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&clist), &p)) return VM_ERROR;
	cell_to_object(p, &clist);
	if (function_add_use_object(vm, function, &clist)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiTHROW
static VM_RET t3_s_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=1;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_throw(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	code_list_increment_head(head);

	if (function_write_command(vm, function, *pc, c_throw)) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiTAGBODY tag-list code-list ... code-list
static VM_RET t3_s_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);

	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_tagbody(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT taglist, flist;
	tPCELL list;

	t3_read_argument_2(head, &taglist, &flist);

	if (function_write_command(vm, function, *pc, c_tagbody)) return VM_ERROR;
	++*pc;
	if (function_add_use_object(vm, function, &taglist)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&taglist))) return VM_ERROR;
	++*pc;
	if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&flist))) return VM_ERROR;
	++*pc;

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

// iiGO (tag . tag-list)
static VM_RET t3_s_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=2;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_go(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT tag;
	t3_read_argument_1(head, &tag);
	if (function_write_command(vm, function, *pc, c_go)) return VM_ERROR;
	++*pc;
	if (function_add_use_object(vm, function, &tag)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&tag))) return VM_ERROR;
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiUNWIND_PROTECT clist clist
static VM_RET t3_s_unwind_protect(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_unwind_protect(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT body, cleanup;
	tPCELL p;
	t3_read_argument_2(head, &body, &cleanup);
	if (function_write_command(vm, function, *pc, c_unwind_protect)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&body), &p)) return VM_ERROR;
	cell_to_object(p, &body);
	if (function_add_use_object(vm, function, &body)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&cleanup), &p)) return VM_ERROR;
	cell_to_object(p, &cleanup);
	if (function_add_use_object(vm, function, &cleanup)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiWITH_OPEN_INPUT_FILE plist clist
// iiWITH_OPEN_OUTPUT_FILE plist clist
// iiWITH_OPEN_IO_FILE plist clist
static VM_RET t3_s_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=3;
	return VM_OK;
}

static VM_RET t3_w_with_open_file(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT plist, func;
	tPCELL p;
	t3_read_argument_2(head, &plist, &func);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (function_add_use_object(vm, function, &plist)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, OBJECT_GET_CELL(&plist))) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&func), &p)) return VM_ERROR;
	cell_to_object(p, &func);
	if (function_add_use_object(vm, function, &func)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

// iiCONTINUE_CONDITION
static VM_RET t3_s_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	*size+=1;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t3_w_continue(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	code_list_increment_head(head);
	switch (code) {
	case iiCONTINUE_CONDITION:
		if (function_write_command(vm, function, *pc, c_continue_condition)) return VM_ERROR;
		break;
	case iiARITY_ERROR:
		if (function_write_command(vm, function, *pc, c_arity_error)) return VM_ERROR;
		break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	++*pc;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiWITH_HANDLER
static VM_RET t3_s_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tINT* size)
{
	code_list_increment_head(head);
	code_list_increment_head(head);
	*size+=2;
	return VM_OK;
}

static VM_RET t3_w_type_f(tPVM vm, tPCELL clist, const tINT code, tPCELL* head, tPCELL function, tINT* pc)
{
	tOBJECT f;
	tPCELL p;
	t3_read_argument_1(head, &f);
	if (function_write_command(vm, function, *pc, t3_table[code].data1)) return VM_ERROR;
	++*pc;
	if (t3_clist_to_function(vm, OBJECT_GET_CELL(&f), &p)) return VM_ERROR;
	cell_to_object(p, &f);
	if (function_add_use_object(vm, function, &f)) return VM_ERROR;
	if (function_write_command(vm, function, *pc, p)) return VM_ERROR;
	++*pc;
	return VM_OK;
}

///////////////////

