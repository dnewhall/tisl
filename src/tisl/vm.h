//
// TISL/src/tisl/vm.h
// TISL Ver 4.x
//

#ifndef TISL_VM_H
#define TISL_VM_H

#include <stdio.h>
#include <time.h>

/////////////////////////////////////////////////

#define LF				0x0a
#define CR				0x0d

/////////////////////////////////////////////////
// VM

typedef struct tVM_INIT_ARGS_		tVM_INIT_ARGS;

struct tVM_INIT_ARGS_ {
	int		init_stack_size;
	int		max_stack_size;
};

// TNI用の初期化引数からVM用の初期化引数の作成
void set_vm_init_args(tVM_INIT_ARGS* vm_args, TNI_INIT_ARGS* tni_args);
// VMの生成
tBOOL create_vm(tPTISL tisl, tPVM* vm, tVM_INIT_ARGS* args);
// VMの開放
tBOOL free_vm(tPVM vm);

///////////////////

// evaluator.cで定義
void vm_clear(tPVM vm);
VM_RET vm_evaluate_top_form(tPVM vm, tPOBJECT form, tPOBJECT ret);

///////////////////

TNI* vm_get_tni(tPVM vm);// tni.c で定義

tPTISL vm_get_tisl(tPVM vm);
tBOOL vm_is_main(tPVM vm);

tINT vm_get_state(tPVM vm);
void vm_set_state_ok(tPVM vm);
void vm_set_state_gc_wait(tPVM vm);
void vm_set_state_gc_run(tPVM vm);
void vm_set_state_dead(tPVM vm);
void vm_set_state(tPVM vm, const tINT state);
tPVM vm_get_next(tPVM vm);
void vm_set_next(tPVM vm, tPVM next);

void vm_set_gc_mark(tPVM vm, const tBOOL mark);

tPCELL vm_get_top_package(tPVM vm);
tPCELL vm_get_islisp_package(tPVM vm);
tPCELL vm_get_system_package(tPVM vm);
// current package
tPCELL vm_get_current_package(tPVM vm);
void vm_set_current_package(tPVM vm, tPCELL package);
// function 
tPCELL vm_get_current_function_package(tPVM vm);
tPCELL vm_get_environment(tPVM vm);
void vm_set_environment(tPVM vm, tPCELL environment);
tPCELL vm_get_function(tPVM vm);
void vm_set_function(tPVM vm, tPCELL function);
//tPOBJECT vm_get_invoke_point(tPVM vm);
//void vm_set_invoke_point(tPVM vm, tPOBJECT point);
//void vm_set_invoke_point_sp(tPVM vm);
// 標準ストリーム
tPCELL vm_get_standard_input(tPVM vm);
tPCELL vm_get_standard_output(tPVM vm);
tPCELL vm_get_error_output(tPVM vm);
VM_RET vm_push_standard_input(tPVM vm, tPCELL stream);
void vm_pop_standard_input(tPVM vm);
VM_RET vm_push_standard_output(tPVM vm, tPCELL stream);
void vm_pop_standard_output(tPVM vm);
VM_RET vm_push_error_output(tPVM vm, tPCELL stream);
void vm_pop_error_output(tPVM vm);

///////////////////
// 外部参照

VM_RET vm_new_global_ref(tPVM vm, TISL_OBJECT ref, TISL_OBJECT* new_ref);
void vm_delete_global_ref(tPVM vm, TISL_OBJECT ref);
VM_RET vm_new_local_ref(tPVM vm, tPOBJECT obj, TISL_OBJECT* new_ref);
VM_RET vm_new_local_ref_(tPVM vm, tPOBJECT obj, TISL_OBJECT* new_ref);
void vm_delete_local_ref(tPVM vm, TISL_OBJECT ref);
VM_RET vm_push_local_ref(tPVM vm);
void vm_pop_local_ref(tPVM vm);

///////////////////
// stack

VM_RET vm_push(tPVM vm, tPOBJECT obj);
void vm_pop(tPVM vm);
void vm_top(tPVM vm, tPOBJECT obj);
VM_RET vm_discard(tPVM vm, const tINT pnum);
VM_RET vm_check_stack_overflow(tPVM vm, const tINT n);
VM_RET vm_list(tPVM vm, const tINT n);

VM_RET vm_push_temp(tPVM vm, tPOBJECT obj);
void vm_pop_temp(tPVM vm);
void vm_temp_stack_clear(tPVM vm);

///////////////////
// reader

tBOOL vm_get_reader_eos_error(tPVM vm);
void vm_set_reader_eos_error(tPVM vm);
void vm_reset_reader_eos_error(tPVM vm);

#define TISL_CHAR_SET_SJIS		0
#define TISL_CHAR_SET_EUC		1

tBOOL is_DBCS_lead_byte(tPVM vm, tCHAR c);

// 内部ストリーム

void vm_output_stream_clear(tPVM vm);
VM_RET vm_output_stream_write_char(tPVM vm, tCHAR c);
VM_RET vm_output_stream_to_string(tPVM vm, tPCELL* string);
VM_RET vm_strftime_to_string(tPVM vm, tCSTRING string, struct tm* t, tPCELL* cell);
VM_RET vm_string_to_symbol(tPVM vm, tPCELL string, tPOBJECT symbol);

///////////////////

tBOOL vm_get_writer_flag(tPVM vm);
void vm_set_writer_flag(tPVM vm);
void vm_reset_writer_flag(tPVM vm);

///////////////////
// handle condition

enum {
	TISL_ERROR_OK,
	// <storage-exhausted>
	TISL_ERROR_STORAGE_EXHAUSTED,
	TISL_ERROR_CANNOT_CREATE_CONS,
	TISL_ERROR_CANNOT_CREATE_STRING,
	TISL_ERROR_CANNOT_CREATE_SYMBOL,
	TISL_ERROR_CANNOT_CREATE_VECTOR,
	TISL_ERROR_CANNOT_CREATE_ARRAY,
	TISL_ERROR_STACK_OVERFLOW,
	TISL_ERROR_STACK_UNDERFLOW,
	// <serious-condition>
	TISL_ERROR_SYSTEM_ERROR,

	TISL_ERROR_UNNAMED_ERROR,

	TISL_ERROR_CONVERT_ERROR_ARRAY,
	// <error>
	// <arithmetic-error>
	// <division-by-zero>
	TISL_ERROR_DIVISION_BY_ZERO,
	// <program-error>
	TISL_ERROR_INDEX_OUT_OF_RANGE,
	TISL_ERROR_CANNOT_OPEN_FILE,
	TISL_ERROR_CANNOT_CLOSE_FILE,
	TISL_ERROR_UNKNOWN_FORMAT_CONTROL,
	TISL_ERROR_ARITY_ERROR,
	TISL_ERROR_IMMUTABLE_OBJECT,
	TISL_ERROR_IMMUTABLE_BINDING,
	TISL_ERROR_IMPROPER_ARGUMENT_LIST,
	TISL_ERROR_INVALID_CONVERT_CLASS,
	TISL_ERROR_PACKAGE_QUALIFIER,
	TISL_ERROR_GENERIC_FUNCTION_CLASS,
	TISL_ERROR_NOT_CONGRUENT_LAMBDA_LIST,
	TISL_ERROR_NO_APPLICABLE_METHOD,
	TISL_ERROR_METACLASS,
	TISL_ERROR_INITFORMS,
	TISL_ERROR_SLOT_UNBOUND,
	TISL_ERROR_ABSTRACT_CLASS,
	TISL_ERROR_INVALID_CLASS,
	TISL_ERROR_CANNOT_LINK_FOREIGN_PROCEDURE,
	TISL_ERROR_CANNOT_OPEN_LIBRARY,
	TISL_ERROR_REST_FUNCTION,
	TISL_ERROR_UNKNOWN_SUPERCLASS,
	// <control-error>
	TISL_ERROR_CONTROL_ERROR,
	// <domain-error>
	TISL_ERROR_DOMAIN_ERROR,
	TISL_ERROR_NOT_AN_INPUT_STREAM,
	TISL_ERROR_NOT_AN_OUTPUT_STREAM,
	// <stream-error>
	TISL_ERROR_STREAM_ERROR,
	TISL_ERROR_END_OF_STREAM,
	TISL_ERROR_STREAM_ERROR_BAD_FLAGS,
	TISL_ERROR_CANNOT_CLOSE_STREAM,
	TISL_ERROR_STREAM_IS_CLOSED,
	// <undefined-entity>
	TISL_ERROR_UNDEFINED_ENTITY,
	TISL_ERROR_UNBOUND_VARIABLE,
	TISL_ERROR_UNDEFINED_FUNCTION,
	// parse-error
	TISL_ERROR_PARSE_ERROR,
	TISL_ERROR_PARSE_ERROR_FUNCTION,
	TISL_ERROR_PARSE_ERROR_INTEGER,
	TISL_ERROR_PARSE_ERROR_FLOAT,
	TISL_ERROR_PARSE_ERROR_STRING,
	TISL_ERROR_PARSE_ERROR_ARRAY,
	TISL_ERROR_PARSE_ERROR_CHARACTER,
	TISL_ERROR_PARSE_ERROR_SYMBOL,
	TISL_ERROR_RIGHT_ARC,
	TISL_ERROR_NOT_DELIMITER_AROUND_DOT,
	TISL_ERROR_OBJECTS_BEHIND_DOT,
	TISL_ERROR_UNKNOWN_CHARACTER,
	TISL_ERROR_SHARP_NUMBER,
	// translator-error
	TRANSLATOR_ERROR,
	TRANSLATOR_ERROR_SYNTAX_ERROR,
	TRANSLATOR_ERROR_UNKNOWN_OBJECT,
	TRANSLATOR_ERROR_UNQUOTE,
	TRANSLATOR_ERROR_DOT_LIST,
	TRANSLATOR_ERROR_BAD_OPERATOR,
	TRANSLATOR_ERROR_LAMBDA,
	TRANSLATOR_ERROR_LAMBDA_LIST,
	TRANSLATOR_ERROR_SAME_NAME_PARAMETER,
	TRANSLATOR_ERROR_SAME_NAME_FUNCTION,
	TRANSLATOR_ERROR_BAD_BLOCK,
	TRANSLATOR_ERROR_BAD_TAGBODY_TAG,
	TRANSLATOR_ERROR_ARITY_ERROR,
	TRANSLATOR_ERROR_METHOD_QUALIFIERS,
	TRANSLATOR_ERROR_NOT_TOP_FORM,
};

// タグ
VM_RET vm_push_tag(tPVM vm, tPOBJECT tag);
void vm_pop_tag(tPVM vm);
void vm_set_throw_object(tPVM vm, tPOBJECT obj);
void vm_get_throw_object(tPVM vm, tPOBJECT obj);
void vm_clear_throw_object(tPVM vm);
tBOOL vm_search_tag(tPVM vm, tPOBJECT tag);
void vm_clear_tag(tPVM vm);
tBOOL vm_search_tagbody_tag(tPVM vm, tPCELL tag_pair);
// ハンドラ
VM_RET vm_push_handler(tPVM vm, tPOBJECT handler);
void vm_pop_handler(tPVM vm);
void vm_get_handler(tPVM vm, tPOBJECT handler);
// 特殊例外ハンドラ
VM_RET vm_push_ignore_errors_handler(tPVM vm);
VM_RET vm_push_foreign_function_handler(tPVM vm);
// ハンドラの同定
tBOOL handler_is_system_handler(tPOBJECT handler);
tBOOL handler_is_ignore_errors(tPOBJECT handler);
tBOOL handler_is_foreign_function(tPOBJECT handler);

void vm_set_last_condition(tPVM vm, tPOBJECT condition);
void vm_set_last_condition_ok(tPVM vm);
void vm_set_last_condition_eos_error(tPVM vm);
void vm_set_last_condition_ignore_errors(tPVM vm);
void vm_set_last_condition_user_interrupt(tPVM vm);
void vm_get_last_condition(tPVM vm, tPOBJECT condition);

tBOOL vm_last_condition_is_ok(tPVM vm);
tBOOL vm_last_condition_is_eos_error(tPVM vm);
tBOOL vm_last_condition_is_ignore_errors(tPVM vm);
tBOOL vm_last_condition_is_user_interrupt(tPVM vm);

VM_RET signal_condition(tPVM vm, const int error_id);
VM_RET signal_condition_(tPVM vm, tPCELL condition, tPOBJECT continuable);
VM_RET signal_simple_error_(tPVM vm, tPCELL string, tPCELL list, tPOBJECT continuable);
VM_RET signal_domain_error(tPVM vm, const int error_id, const tINT expected_class_id, tPOBJECT obj);
VM_RET signal_domain_error_(tPVM vm, const int error_id, tPOBJECT c, tPOBJECT obj);
VM_RET signal_stream_error(tPVM vm, const int error_id, tPCELL stream);
VM_RET signal_parse_error(tPVM vm, const int error_id, tPCELL stream);
VM_RET signal_undefined_entity(tPVM vm, const int error_id, tPCELL name, const tINT namespace_id);
VM_RET signal_violation(tPVM vm, const int error_id, tPOBJECT place);

///////////////////////////////////////

VM_RET vm_load(tPVM vm, tPCELL stream, tPOBJECT last);
// nameは記号
VM_RET vm_in_package(tPVM vm, tPCELL name);
//
tPVM tni_get_vm(TNI* tni);

///////////////////////////////////////

#define VM_STATE_OK					0
#define VM_STATE_GC_WAIT			1
#define VM_STATE_GC_RUN				2
#define VM_STATE_DEAD				3
#define VM_STATE_NOT_INITIALIZE		4
#define VM_STATE_ALLOCATE			5

#ifdef _DEBUG
#define VM_MAX_FUNCTION_CALL		3000
#else
#define VM_MAX_FUNCTION_CALL		9000
#endif

#ifdef TISL_VM_STRUCT

struct tVM_ {// 順番変えないで!!!
	tPOBJECT		SP;
	// state
	tINT			state;
	// TISL
	tPTISL			tisl;
	// 評価スタック
	tPOBJECT		stack;
	tINT			stack_size;
	tINT			max_stack_size;
	tPCELL			temp_stack;
	tINT			function_call_n;
	// condition handler
	tPCELL			handler_list;
	tPCELL			tag_list;
	tOBJECT			last_condition;
	tOBJECT			throw_object;
	// 外部参照
	// TISL_OBJECT 自身は移動してはいけない
	// 大域
	tPCELL			global_ref_head;
	// 局所
	tPCELL			local_ref_head;
	// Garbage Collector
	tPGC			gc;
	// current package
	tPCELL			current_package;
	// environment
	tPCELL			environment;
	// function
	tPCELL			function;
	// invoke_point
//	tPOBJECT		invoke_point;
	// 標準ストリーム
	tPCELL			standard_input;
	tPCELL			standard_output;
	tPCELL			error_output;
	// reader
	tBOOL			reader_eos_error;
	tPCELL			private_stream;
	tPCELL			private_input_stream;
	tINT			char_set;
	// writer
	tBOOL			writer_flag;
	// translator
	tTRANSLATOR		translator;
	// VM
	tPVM			next;
	// TNI
	TNI*			tni;
};

#endif

#endif
