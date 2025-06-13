//
// TISL/src/tisl/vm.c
// TISL Ver 4.x
//

#define TISL_VM_STRUCT

#include <malloc.h>
#include <memory.h>
#include <time.h>
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "gc.h"
#include "translator.h"
#include "writer.h"
#include "reader.h"
#include "built_in_object.h"

///////////////////////////////////////
extern VM_RET string_stream_create(tPVM vm, const tINT io_flag, tPCELL* cell);

// TNI用の初期化引数からVM用の初期化引数の作成
void set_vm_init_args(tVM_INIT_ARGS* vm_args, TNI_INIT_ARGS* tni_args)
{
	vm_args->init_stack_size=tni_args->init_stack_size;
	vm_args->max_stack_size=tni_args->max_stack_size;
}

#define HEAP_SIZE_UNIT		65536

// VM の生成
tBOOL create_vm(tPTISL tisl, tPVM* vm, tVM_INIT_ARGS* args)
{
	*vm=malloc(sizeof(tVM));
	if (!*vm) return tFALSE;
	//
	(*vm)->state=VM_STATE_NOT_INITIALIZE;
	// tisl
	(*vm)->tisl=tisl;
	// stack
	(*vm)->stack=malloc(sizeof(tOBJECT)*args->init_stack_size);
	if (!(*vm)->stack) goto ERROR;
	(*vm)->stack_size=args->init_stack_size;
	(*vm)->SP=(*vm)->stack;
	(*vm)->max_stack_size=args->max_stack_size;
	(*vm)->temp_stack=0;
	(*vm)->function_call_n=0;
	// condition handler
	(*vm)->handler_list=0;
	(*vm)->tag_list=0;
	OBJECT_SET_UNBOUND(&(*vm)->throw_object);
	vm_set_last_condition_ok(*vm);
	// foreign reference list
	(*vm)->global_ref_head=0;
	(*vm)->local_ref_head=0;
	// function
	(*vm)->function=0;
	(*vm)->environment=0;
//	(*vm)->invoke_point=(*vm)->stack;
	//
	(*vm)->translator=0;
	(*vm)->standard_input=0;
	(*vm)->standard_output=0;
	(*vm)->error_output=0;
	(*vm)->private_stream=0;
	(*vm)->private_input_stream=0;
	// Garbage Collector
	if (!gc_create(*vm, &(*vm)->gc, HEAP_SIZE_UNIT)) goto ERROR2;
	// package
	(*vm)->current_package=0;
	// reader
	(*vm)->reader_eos_error=tFALSE;
	(*vm)->private_stream=0;
	(*vm)->char_set=TISL_CHAR_SET_SJIS;
	// writer
	(*vm)->writer_flag=tFALSE;
	// 標準ストリーム
	(*vm)->standard_input=0;
	(*vm)->standard_output=0;
	(*vm)->error_output=0;
	//
	(*vm)->next=0;
	//
	if (string_stream_create_output(*vm, &(*vm)->private_stream)) goto ERROR;
	if (string_stream_create(*vm, STREAM_INPUT, &(*vm)->private_input_stream)) goto ERROR;
	//
	if (create_translator(*vm, &(*vm)->translator)) goto ERROR3;
	// 
	if (cons_create_(*vm, &(*vm)->temp_stack, &nil, &nil)) return VM_ERROR;
	// 外部から呼出されたときのために一枚用意しておく．
	if (vm_push_local_ref(*vm)) return VM_ERROR;

	(*vm)->state=VM_STATE_OK;
	return tTRUE;
//0ERROR4:
	free_translator((*vm)->translator);
ERROR3:
	free_gc((*vm)->gc);
ERROR2:
	free((*vm)->stack);
ERROR:
	free(*vm);
	return tFALSE;
}

// VMの開放
tBOOL free_vm(tPVM vm)
{
	free_translator(vm->translator);
	free(vm->stack);
	free_gc(vm->gc);
	free(vm);
	return tTRUE;
}

tPGC vm_get_gc(tPVM vm)
{
	return vm->gc;
}

tPTISL vm_get_tisl(tPVM vm)
{
	return vm->tisl;
}

tBOOL vm_is_main(tPVM vm)
{
	return (vm==tisl_get_main_vm(vm->tisl)) ? tTRUE : tFALSE;
}

tTRANSLATOR vm_get_translator(tPVM vm)
{
	return vm->translator;
}

tINT vm_get_state(tPVM vm)
{
	return vm->state;
}

void vm_set_state_ok(tPVM vm)
{
	vm->state=VM_STATE_OK;
}

void vm_set_state_gc_wait(tPVM vm)
{
	vm->state=VM_STATE_GC_WAIT;
}

void vm_set_state_gc_run(tPVM vm)
{
	vm->state=VM_STATE_GC_RUN;
}

void vm_set_state_dead(tPVM vm)
{
	vm->state=VM_STATE_DEAD;
}

void vm_set_state(tPVM vm, const tINT state)
{
	vm->state=state;
}

tPVM vm_get_next(tPVM vm)
{
	return vm->next;
}

void vm_set_next(tPVM vm, tPVM next)
{
	vm->next=next;
}

/////////////////////////////
// package

tPCELL vm_get_top_package(tPVM vm)
{
	return tisl_get_top_package(vm->tisl);
}

tPCELL vm_get_current_package(tPVM vm)
{
	return vm->current_package;
}

void vm_set_current_package(tPVM vm, tPCELL package)
{
	vm->current_package=package;
}

tPCELL vm_get_current_function_package(tPVM vm)
{
	if (vm->function)
		return function_get_package(vm->function);
	else
		return vm->current_package;
}

tPCELL vm_get_environment(tPVM vm)
{
	return vm->environment;
}

void vm_set_environment(tPVM vm, tPCELL environment)
{
	vm->environment=environment;
}

tPCELL vm_get_function(tPVM vm)
{
	return vm->function;
}

void vm_set_function(tPVM vm, tPCELL function)
{
	vm->function=function;
}

//tPOBJECT vm_get_invoke_point(tPVM vm)
//{
//	return vm->invoke_point;
//}

//void vm_set_invoke_point(tPVM vm, tPOBJECT point)
//{
//	vm->invoke_point=point;
//}

//void vm_set_invoke_point_sp(tPVM vm)
//{
//	vm->invoke_point=vm->SP;
//}

///////////////////
// package

tPCELL vm_get_islisp_package(tPVM vm)
{
	tPCELL bind=package_get_bind(tisl_get_top_package(vm->tisl), string_islisp);
	if (bind) {
		tOBJECT obj;
		bind_get_package(bind, &obj);
		if (OBJECT_IS_PACKAGE(&obj)) return OBJECT_GET_CELL(&obj);
	}
	return 0;
}

tPCELL vm_get_system_package(tPVM vm)
{
	tPCELL bind=package_get_bind(tisl_get_top_package(vm->tisl), string_system);
	if (bind) {
		tOBJECT obj;
		bind_get_package(bind, &obj);
		if (OBJECT_IS_PACKAGE(&obj)) return OBJECT_GET_CELL(&obj);
	}
	return 0;
}

/////////////////////////////
// 外部参照

VM_RET vm_new_global_ref(tPVM vm, TISL_OBJECT ref, TISL_OBJECT* new_ref)
{
	tPCELL cell;
	tOBJECT obj;
	tisl_object_get_object(ref, &obj);
	if (tisl_object_create(vm, &obj, &cell)) return VM_ERROR;
	tisl_object_set_next(cell, vm->global_ref_head);
	vm->global_ref_head=cell;
	*new_ref=(TISL_OBJECT)cell;
	return VM_OK;
}

void vm_delete_global_ref(tPVM vm, TISL_OBJECT ref)
{
	tPCELL p, last;
	if (!ref) return;
	for (p=vm->global_ref_head, last=0; p; last=p, p=tisl_object_get_next(p)) {
		if (p==(tPCELL)ref) {
			if (last) {
				tisl_object_set_next(last, tisl_object_get_next(p));
			} else {
				vm->global_ref_head=tisl_object_get_next(p);
			}
			return;
		}
	}
}

VM_RET vm_push_local_ref(tPVM vm)
{
	tPCELL p;
	tOBJECT tmp;
	if (vm->local_ref_head) {
		OBJECT_SET_CONS(&tmp, vm->local_ref_head);
	} else {
		OBJECT_SET_NIL(&tmp);
	}
	if (cons_create_(vm, &p, &nil, &tmp)) return VM_ERROR;
	vm->local_ref_head=p;
	return VM_OK;
}

void vm_pop_local_ref(tPVM vm)
{
	if (vm->local_ref_head)
		vm->local_ref_head=cons_get_cdr_cons(vm->local_ref_head);
}

VM_RET vm_new_local_ref(tPVM vm, tPOBJECT obj, TISL_OBJECT* new_ref)
{
	VM_RET ret;
	if (vm_push(vm, obj)) return VM_ERROR;
	ret=vm_new_local_ref_(vm, obj, new_ref);
	vm_pop(vm);
	return ret;
}

VM_RET vm_new_local_ref_(tPVM vm, tPOBJECT obj, TISL_OBJECT* new_ref)
{
	if (vm->local_ref_head) {
		tPCELL cell;
		tOBJECT tmp;
		cons_get_car(vm->local_ref_head, &tmp);
		if (tisl_object_create(vm, obj, &cell)) return VM_ERROR;
		if (OBJECT_IS_TISL_OBJECT(&tmp)) {
			tisl_object_set_next(cell, OBJECT_GET_CELL(&tmp));
		}
		OBJECT_SET_TISL_OBJECT(&tmp, cell);
		cons_set_car(vm->local_ref_head, &tmp);
		*new_ref=(TISL_OBJECT)cell;
		return VM_OK;
	} else {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
}

void vm_delete_local_ref(tPVM vm, TISL_OBJECT ref)
{
	if (vm->local_ref_head) {
		// ローカル参照がレベルを超えて参照されることはないはず
		tPCELL p, last;
		tOBJECT tmp;
		cons_get_car(vm->local_ref_head, &tmp);
		for (p=OBJECT_GET_CELL(&tmp), last=0; p; last=p, p=tisl_object_get_next(p)) {
			if (p==(tPCELL)ref) {
				if (last) {
					tisl_object_set_next(last, tisl_object_get_next(p));
				} else {
					OBJECT_SET_TISL_OBJECT(&tmp, last);
					cons_set_car(vm->local_ref_head, &tmp);
				}
				return;
			}
		}
	}
}

/////////////////////////////
// スタック

static VM_RET vm_extend_stack(tPVM vm);

VM_RET vm_push(tPVM vm, tPOBJECT obj)
{
	vm->SP++;
	if (vm->stack_size==vm->SP-vm->stack) {
		// スタックの拡張
		if (vm_extend_stack(vm)) return VM_ERROR;
	}
	*vm->SP=*obj;
	return VM_OK;
}

VM_RET vm_check_stack_overflow(tPVM vm, const tINT n)
{
	while (vm->stack_size<=vm->SP-vm->stack+n) {
		// スタックの拡張
		if (vm_extend_stack(vm)) return VM_ERROR;
	}
	return VM_OK;
}

static VM_RET vm_extend_stack(tPVM vm)
{
	if (vm->stack_size<vm->max_stack_size) {
		tPOBJECT stack;
		tINT size, old;
		tisl_gc_wait_2(vm_get_tisl(vm), vm);
		old=vm_get_state(vm);
		vm_set_state(vm, VM_STATE_ALLOCATE);
		size=vm->stack_size+1024;/*!!!*/
		if (size>vm->max_stack_size) size=vm->max_stack_size;
		stack=malloc(sizeof(tOBJECT)*size);
		if (!stack) { vm_set_state(vm, old); return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED); }
		memcpy(stack, vm->stack, sizeof(tOBJECT)*vm->stack_size);
		free(vm->stack);
		vm->stack=stack;
		vm->SP=vm->stack+vm->stack_size;
		vm->stack_size=size;
		vm_set_state(vm, old);
#ifdef _DEBUG
		if (format_l(vm, vm_get_standard_output(vm), "; extend-stack : ", 0)) return VM_ERROR;
		if (format_integer(vm, vm_get_standard_output(vm), size*sizeof(tOBJECT), 10)) return VM_ERROR;
		if (format_l(vm, vm_get_standard_output(vm), "Byte~%", 0)) return VM_ERROR;
#endif
		return VM_OK;
	} else {
		return signal_condition(vm, TISL_ERROR_STACK_OVERFLOW);
	}
}

void vm_pop(tPVM vm)
{
#ifdef _DEBUG
	if (vm->SP-vm->stack==0) {
		signal_condition(vm, TISL_ERROR_STACK_UNDERFLOW);
	}
#endif
	vm->SP--;
}

void vm_top(tPVM vm, tPOBJECT obj)
{
	*obj=*vm->SP;
}

VM_RET vm_discard(tPVM vm, const tINT pnum)
{
#ifdef _DEBUG
	if (vm->SP-vm->stack<=pnum) {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
#endif
	vm->SP-=pnum;
	return VM_OK;
}

VM_RET vm_list(tPVM vm, const tINT n)
{
	tINT i;
	tPCELL p;
	if (vm->SP-vm->stack<=n)// system-errorか？
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tOBJECT tmp=*(vm->SP);
		if (cons_create(vm, &p, &tmp, &nil)) return VM_ERROR;
	}
	OBJECT_SET_CONS(vm->SP, p);
	for (i=1; i<n; i++) {
		tOBJECT car, cdr;
		cdr=*vm->SP--;
		car=*vm->SP;
		if (cons_create(vm, &p, &car, &cdr)) return VM_ERROR;
		OBJECT_SET_CONS(vm->SP, p);
	}
	return VM_OK;
}

VM_RET vm_push_temp(tPVM vm, tPOBJECT obj)
{
	tPCELL p;
	tOBJECT tmp;
	cons_set_car(vm->temp_stack, obj);
	OBJECT_SET_CONS(&tmp, vm->temp_stack);
	if (cons_create_(vm, &p, &nil, &tmp)) return VM_ERROR;
	vm->temp_stack=p;
	return VM_OK;
}

void vm_pop_temp(tPVM vm)
{
	tOBJECT tmp;
	cons_get_cdr(vm->temp_stack, &tmp);
	if (OBJECT_IS_CONS(&tmp)) {
		vm->temp_stack=OBJECT_GET_CELL(&tmp);
		cons_set_car(vm->temp_stack, &nil);
	}
}

void vm_temp_stack_clear(tPVM vm)
{
	cons_set_car(vm->temp_stack, &nil);
	cons_set_cdr(vm->temp_stack, &nil);
}

/////////////////////////////
// reader

tBOOL vm_get_reader_eos_error(tPVM vm)
{
	return vm->reader_eos_error;
}

void vm_set_reader_eos_error(tPVM vm)
{
	vm->reader_eos_error=tTRUE;
}

void vm_reset_reader_eos_error(tPVM vm)
{
	vm->reader_eos_error=tFALSE;
}

tBOOL is_DBCS_lead_byte(tPVM vm, tCHAR c)
{/*
	switch (vm->char_set) {
	case TISL_CHAR_SET_SJIS:
		return (((c>=0x81)&&(c<=0x9f))||((c>=0xe0)&&(c<=0xfc))) ? tTRUE : tFALSE;
	case TISL_CHAR_SET_EUC:
		return ((c>=0xa0)&&(c<=0xff)) ? tTRUE : tFALSE;
	default:
		return tFALSE;
	}
	*/
	return (c>=0x80) ? tTRUE : tFALSE;
}

//ストリーム

void vm_output_stream_clear(tPVM vm)
{
	string_stream_clear(vm->private_stream);
}

VM_RET vm_output_stream_write_char(tPVM vm, tCHAR c)
{
	return string_stream_write_char(vm, vm->private_stream, c);
}

VM_RET vm_output_stream_to_string(tPVM vm, tPCELL* string)
{
	return string_stream_to_string(vm, vm->private_stream, string);
}

extern tSTRING string_stream_get_buffer_area(tPCELL stream);
extern tINT string_stream_get_buffer_size(tPCELL stream);

VM_RET vm_strftime_to_string(tPVM vm, tCSTRING string, struct tm* t, tPCELL* cell)
{
	size_t s;
	vm_output_stream_clear(vm);
	s=strftime(string_stream_get_buffer_area(vm->private_stream), string_stream_get_buffer_size(vm->private_stream), string, t);
	if (!s) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (string_create(vm, string_stream_get_buffer_area(vm->private_stream), cell)) return VM_ERROR;
	vm_output_stream_clear(vm);
	return VM_OK;
}

VM_RET vm_string_to_symbol(tPVM vm, tPCELL string, tPOBJECT symbol)
{
	string_stream_clear(vm->private_input_stream);
	if (string_input_stream_initialize(vm, vm->private_input_stream, string_get_string(string))) return VM_ERROR;
	if (read_form(vm, vm->private_input_stream, symbol)) return VM_ERROR;
	string_stream_clear(vm->private_input_stream);
	if (!OBJECT_IS_SYMBOL(symbol)&&!OBJECT_IS_NIL(symbol)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, vm->private_stream);
	return VM_OK;
}

/////////////////////////////
// 標準ストリーム
tPCELL vm_get_standard_input(tPVM vm)
{
	if (vm->standard_input) {
		tOBJECT obj;
		cons_get_car(vm->standard_input, &obj);
		return OBJECT_GET_CELL(&obj);
	} else {
		tOBJECT obj;
		tisl_get_standard_input(vm_get_tisl(vm), &obj);
		if (OBJECT_IS_FILE_STREAM(&obj)||
			OBJECT_IS_STRING_STREAM(&obj)) 
			return OBJECT_GET_CELL(&obj);
		else
			return 0;
	}
}

tPCELL vm_get_standard_output(tPVM vm)
{
	if (vm->standard_output) {
		tOBJECT obj;
		cons_get_car(vm->standard_output, &obj);
		return OBJECT_GET_CELL(&obj);
	} else {
		tOBJECT obj;
		tisl_get_standard_output(vm_get_tisl(vm), &obj);
		if (OBJECT_IS_FILE_STREAM(&obj)||
			OBJECT_IS_STRING_STREAM(&obj))
			return OBJECT_GET_CELL(&obj);
		else
			return 0;
	}
}

tPCELL vm_get_error_output(tPVM vm)
{
	if (vm->error_output) {
		tOBJECT obj;
		cons_get_car(vm->error_output, &obj);
		return OBJECT_GET_CELL(&obj);
	} else {
		tOBJECT obj;
		tisl_get_error_output(vm_get_tisl(vm), &obj);
		if (OBJECT_IS_FILE_STREAM(&obj)||
			OBJECT_IS_STRING_STREAM(&obj))
			return OBJECT_GET_CELL(&obj);
		else
			return 0;
	}
}

static VM_RET vm_push_list_stack(tPVM vm, tPOBJECT obj, tPCELL* head);
static void vm_pop_list_stack(tPCELL* head);

VM_RET vm_push_standard_input(tPVM vm, tPCELL stream)
{
	tOBJECT obj;
	cell_to_object(stream, &obj);
	return vm_push_list_stack(vm, &obj, &vm->standard_input);
}

void vm_pop_standard_input(tPVM vm)
{
	vm_pop_list_stack(&vm->standard_input);
}

VM_RET vm_push_standard_output(tPVM vm, tPCELL stream)
{
	tOBJECT obj;
	cell_to_object(stream, &obj);
	return vm_push_list_stack(vm, &obj, &vm->standard_output);
}

void vm_pop_standard_output(tPVM vm)
{
	vm_pop_list_stack(&vm->standard_output);
}

VM_RET vm_push_error_output(tPVM vm, tPCELL stream)
{
	tOBJECT obj;
	cell_to_object(stream, &obj);
	return vm_push_list_stack(vm, &obj, &vm->error_output);
}

void vm_pop_error_output(tPVM vm)
{
	vm_pop_list_stack(&vm->error_output);
}

static VM_RET vm_push_list_stack(tPVM vm, tPOBJECT obj, tPCELL* head)
{
	tOBJECT cdr;
	tPCELL p;

	if (*head) {
		cell_to_object(*head, &cdr);
	} else {
		OBJECT_SET_NIL(&cdr);
	}
	if (cons_create(vm, &p, obj, &cdr)) return VM_ERROR;
	*head=p;
	return VM_OK;
}

static void vm_pop_list_stack(tPCELL* head)
{
	if (*head) *head=cons_get_cdr_cons(*head);
}

/////////////////////////////
// writer

tBOOL vm_get_writer_flag(tPVM vm)
{
	return vm->writer_flag;
}

void vm_set_writer_flag(tPVM vm)
{
	vm->writer_flag=tTRUE;
}

void vm_reset_writer_flag(tPVM vm)
{
	vm->writer_flag=tFALSE;
}

/////////////////////////////
// handle condition

static void handle_condition(tPVM vm, tPCELL condition);

///////////////////

typedef struct tCONDITION_TABLE_	tCONDITION_TABLE;
typedef VM_RET (*fCREATE_CONDITION)(tPVM, tINT, tPCELL*);

struct tCONDITION_TABLE_ {
	tSTRING				name;
	tINT				error_class_id;
	fCREATE_CONDITION	create;
};

static VM_RET default_condition_create(tPVM vm, tINT class_id, tPCELL* cell);
static VM_RET system_condition_create(tPVM vm, tINT class_id, tPCELL* cell);


const tCONDITION_TABLE condition_table[]={
	{ "TISL-OK", CLASS_ERROR, system_condition_create },
	// <storage-exhausted>
	{ "storage-exhausted",			CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "cannot-create-cons",			CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "cannot-create-string",		CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "cannot-create-symbol",		CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "cannot-create-vector",		CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "cannot-create-array",		CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "stack-overflow",				CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	{ "stack-underflow",			CLASS_STORAGE_EXHAUSTED,	system_condition_create },
	// <serious-condition>
	{ "system-error",				CLASS_SERIOUS_CONDITION,	system_condition_create },
	{ "unnamed-error",				CLASS_SERIOUS_CONDITION,	default_condition_create },
	// 別の場所か？
	{ "convert-error(array)",		CLASS_SERIOUS_CONDITION,	default_condition_create },
	// <error>
	// <arithmetic-error>
	// <division-by-zero>
	{ "division-by-zero",			CLASS_DIVISION_BY_ZERO,		default_condition_create },
	// <program-error>
	{ "index-out-of-range",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "cannot-open-file",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "cannot-close-file",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "unknown-format-control",		CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "arity-error",				CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "immutable-object",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "immutable-binding",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "improper-argument-list",		CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "invalid-convert-class",		CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "pacakge-qualifier",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "generic-function-class",		CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "not-congtuent-lambda-list",	CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "no-applicable-method",		CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "metaclass",					CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "iniforms",					CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "unbound-slot",				CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "abstract-class",				CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "invalid-class",				CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "cannot-link-foreign-procedure", CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "cannot-open-library",		CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "bad-rest-function",			CLASS_PROGRAM_ERROR,		default_condition_create },
	{ "unknown-superclass",			CLASS_PROGRAM_ERROR,		default_condition_create },
	// <control-error>
	{ "control-error",				CLASS_CONTROL_ERROR,		default_condition_create },
	// <domain-error>
	{ "domain-error",				CLASS_DOMAIN_ERROR,			default_condition_create },
	{ "not-an-input-stream",		CLASS_DOMAIN_ERROR,			default_condition_create },
	{ "not-an-output-stream",		CLASS_DOMAIN_ERROR,			default_condition_create },
	// <stream-error>
	{ "stream-error",				CLASS_STREAM_ERROR,			default_condition_create },
	{ "end-of-stream",				CLASS_STREAM_ERROR,			default_condition_create },
	{ "stream-error-bad-flag",		CLASS_STREAM_ERROR,			default_condition_create },
	{ "cannot-close-stream",		CLASS_STREAM_ERROR,			default_condition_create },
	{ "stream-is-closed",			CLASS_STREAM_ERROR,			default_condition_create },
	// <undefined-entity>
	{ "undefined-entity",			CLASS_UNDEFINED_ENTITY,		default_condition_create },
	// <unbound-variable>
	{ "unbound-variable",			CLASS_UNBOUND_VARIABLE,		default_condition_create },
	// <undefined-function>
	{ "undefined-function",			CLASS_UNDEFINED_FUNCTION,	default_condition_create },
	// parse-error
	{ "parse-error",						CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : function",				CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : integer",				CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : float",				CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : string",				CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : array",				CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : character",			CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : symbol",				CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : )",					CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : delimiter",			CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : .",					CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : unknown-character",	CLASS_PARSE_ERROR,			default_condition_create },
	{ "parse-error : #?",					CLASS_PARSE_ERROR,			default_condition_create },
	// translator-error
	{ "violation",						CLASS_VIOLATION,	default_condition_create },
	{ "violation : syntax-error",		CLASS_VIOLATION,	default_condition_create },
	{ "violation : unknown-object",		CLASS_VIOLATION,	default_condition_create },
	{ "violation : unquote",				CLASS_VIOLATION,	default_condition_create },
	{ "violation : dot-list",			CLASS_VIOLATION,	default_condition_create },
	{ "violation : bad-operator",		CLASS_VIOLATION,	default_condition_create },
	{ "violation : lambda",				CLASS_VIOLATION,	default_condition_create },
	{ "violation : lambda-list",			CLASS_VIOLATION,	default_condition_create },
	{ "violation : same-name-parameter",	CLASS_VIOLATION,	default_condition_create },
	{ "violation : same-name-function",	CLASS_VIOLATION,	default_condition_create },
	{ "violation : bad-block",			CLASS_VIOLATION,	default_condition_create },
	{ "violation : bad-tagbody-tag",		CLASS_VIOLATION,	default_condition_create },
	{ "violation : arity-error",			CLASS_VIOLATION,	default_condition_create },
	{ "violation : method-qualifiers",	CLASS_VIOLATION,	default_condition_create },
	{ "violation : not-top-form",		CLASS_VIOLATION,	default_condition_create },
};

#define ERROR_IS_SYSTEM_ERROR(id)		((id)<=TISL_ERROR_SYSTEM_ERROR)

///////////////////

// タグ
VM_RET vm_push_tag(tPVM vm, tPOBJECT tag)
{
	tPCELL p;
	tOBJECT next;
	if (vm->tag_list) {
		cell_to_object(vm->tag_list, &next);
	} else {
		OBJECT_SET_NIL(&next);
	}
	if (cons_create_(vm, &p, tag, &next)) return VM_ERROR;
	vm->tag_list=p;
	return VM_OK;
}

void vm_pop_tag(tPVM vm)
{
	if (vm->tag_list) {
		vm->tag_list=cons_get_cdr_cons(vm->tag_list);
	}
}

void vm_set_throw_object(tPVM vm, tPOBJECT obj)
{
	vm->throw_object=*obj;
}

void vm_get_throw_object(tPVM vm, tPOBJECT obj)
{
	*obj=vm->throw_object;
}

void vm_clear_throw_object(tPVM vm)
{
	OBJECT_SET_UNBOUND(&vm->throw_object);
}

tBOOL vm_search_tag(tPVM vm, tPOBJECT tag)
{
	tPCELL p;
	tOBJECT obj;
	for (p=vm->tag_list; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &obj);
		if (object_eql(&obj, tag)) {
			// 自分よりも後に設定されたタグを無効にする
			vm->tag_list=cons_get_cdr_cons(p);
			return tTRUE;
		}
	}
	return tFALSE;
}

tBOOL vm_search_tagbody_tag(tPVM vm, tPCELL tag_pair)
{
	tPCELL p;
	tOBJECT obj, tag;
	cons_get_cdr(tag_pair, &tag);
	for (p=vm->tag_list; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &obj);
		if (object_eql(&tag, &obj)) {
			vm->tag_list=cons_get_cdr_cons(p);
			return tTRUE;
		}
	}
	return tFALSE;
}

void vm_clear_tag(tPVM vm)
{
	vm->tag_list=0;
}

// ハンドラ関数の管理

#define SYSTEM_HANDLER_TOP					0
#define SYSTEM_HANDLER_FOREIGN_FUNCTION		1
#define SYSTEM_HANDLER_IGNORE_ERRORS		2

VM_RET vm_push_handler(tPVM vm, tPOBJECT handler)
{
	tPCELL p;
	tOBJECT next;
	if (vm->handler_list) {
		cell_to_object(vm->handler_list, &next);
	} else {
		OBJECT_SET_NIL(&next);
	}
	if (cons_create_(vm, &p, handler, &next)) return VM_ERROR;
	vm->handler_list=p;
	return VM_OK;
}

void vm_pop_handler(tPVM vm)
{
	if (vm->handler_list) {
		vm->handler_list=cons_get_cdr_cons(vm->handler_list);
	}
}

void vm_get_handler(tPVM vm, tPOBJECT handler)
{
	if (vm->handler_list) {
		cons_get_car(vm->handler_list, handler);
	} else {
		// 処理系定義デフォルトハンドラ
		OBJECT_SET_INTEGER(handler, SYSTEM_HANDLER_TOP);
	}
}

// 特殊例外ハンドラ
VM_RET vm_push_ignore_errors_handler(tPVM vm)
{
	tOBJECT handler;
	OBJECT_SET_INTEGER(&handler, SYSTEM_HANDLER_IGNORE_ERRORS);
	return vm_push_handler(vm, &handler);
}

VM_RET vm_push_foreign_function_handler(tPVM vm)
{
	tOBJECT handler;
	OBJECT_SET_INTEGER(&handler, SYSTEM_HANDLER_FOREIGN_FUNCTION);
	return vm_push_handler(vm, &handler);
}

// ハンドラの同定
tBOOL handler_is_system_handler(tPOBJECT handler)
{
	return OBJECT_IS_INTEGER(handler) ? tTRUE : tFALSE;
}

tBOOL handler_is_ignore_errors(tPOBJECT handler)
{
	return (OBJECT_IS_INTEGER(handler)&&
		(OBJECT_GET_INTEGER(handler)==SYSTEM_HANDLER_IGNORE_ERRORS))
		? tTRUE : tFALSE;
}

tBOOL handler_is_foreign_function(tPOBJECT handler)
{
	return (OBJECT_IS_INTEGER(handler)&&
		(OBJECT_GET_INTEGER(handler)==SYSTEM_HANDLER_FOREIGN_FUNCTION))
		? tTRUE : tFALSE;
}

// VMの状態

#define VM_CONDITION_EOS_ERROR		1
#define VM_CONDITION_IGNORE_ERRORS	2
#define VM_CONDITION_USER_INTERRUPT	3

void vm_set_last_condition(tPVM vm, tPOBJECT obj)
{
	vm->last_condition=*obj;
}

void vm_set_last_condition_ok(tPVM vm)
{
	vm->last_condition=unbound;
}

void vm_get_last_condition(tPVM vm, tPOBJECT condition)
{
	*condition=vm->last_condition;
}

tBOOL vm_last_condition_is_ok(tPVM vm)
{
	return OBJECT_IS_UNBOUND(&vm->last_condition) ? tTRUE : tFALSE;
}

void vm_set_last_condition_eos_error(tPVM vm)
{
	OBJECT_SET_INTEGER(&vm->last_condition, VM_CONDITION_EOS_ERROR);
}

tBOOL vm_last_condition_is_eos_error(tPVM vm)
{
	return (OBJECT_IS_INTEGER(&vm->last_condition)&&
		(OBJECT_GET_INTEGER(&vm->last_condition)==VM_CONDITION_EOS_ERROR))
		? tTRUE : tFALSE;
}

void vm_set_last_condition_ignore_errors(tPVM vm)
{
	OBJECT_SET_INTEGER(&vm->last_condition, VM_CONDITION_IGNORE_ERRORS);
}

tBOOL vm_last_condition_is_ignore_errors(tPVM vm)
{
	return (OBJECT_IS_INTEGER(&vm->last_condition)&&
		(OBJECT_GET_INTEGER(&vm->last_condition)==VM_CONDITION_IGNORE_ERRORS))
		? tTRUE : tFALSE;
}

void vm_set_last_condition_user_interrupt(tPVM vm)
{
	OBJECT_SET_INTEGER(&vm->last_condition, VM_CONDITION_USER_INTERRUPT);
}

tBOOL vm_last_condition_is_user_interrupt(tPVM vm)
{
	return (OBJECT_IS_INTEGER(&vm->last_condition)&&
		(OBJECT_GET_INTEGER(&vm->last_condition)==VM_CONDITION_USER_INTERRUPT))
		? tTRUE : tFALSE;
}

VM_RET signal_condition(tPVM vm, const int error_id)
{
	tPCELL condition;
	tOBJECT obj;

	if (ERROR_IS_SYSTEM_ERROR(error_id)) {
		// 例外処理の難しい例外
		if ((*condition_table[error_id].create)(vm, error_id, &condition)) return VM_ERROR;
		cell_to_object(condition, &obj);
		vm_set_last_condition(vm, &obj);
		// 最上位まで脱出
		return VM_ERROR;
	} else {
		// 例外処理の対象となる例外
		if ((*condition_table[error_id].create)(vm, error_id, &condition)) return VM_ERROR;
		handle_condition(vm, condition);
		// ハンドラ関数は正常終了しない
		return VM_ERROR;
	}
}

VM_RET signal_condition_(tPVM vm, tPCELL condition, tPOBJECT continuable)
{
	tOBJECT slot1, slot2, c, place;
	tPCELL cell;
	condition_get_slot1(condition, &slot1);
	condition_get_slot2(condition, &slot2);
	if (OBJECT_IS_NIL(continuable))
		OBJECT_SET_UNBOUND(&c);
	else
		c=*continuable;
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (condition_create(vm, condition_get_class_id(condition), condition_get_name(condition), &slot1, &slot2, &c, &place, &cell)) return VM_ERROR;
	handle_condition(vm, cell);
	return vm_last_condition_is_ok(vm) ? VM_OK : VM_ERROR;
}

VM_RET signal_simple_error_(tPVM vm, tPCELL string, tPCELL list, tPOBJECT continuable)
{
	tPCELL cell;
	tOBJECT obj, place;
	if (OBJECT_IS_NIL(continuable)) OBJECT_SET_UNBOUND(continuable);
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (condition_create(vm, CLASS_SIMPLE_ERROR, string_simple_error, &unbound, &unbound, continuable, &place, &cell)) return VM_ERROR;
	//
	OBJECT_SET_STRING(&obj, string);
	simple_error_set_format_string(cell, &obj);
	if (list) 
		OBJECT_SET_CONS(&obj, list);
	else 
		OBJECT_SET_NIL(&obj);
	simple_error_set_format_argument(cell, &obj);
	handle_condition(vm, cell);
	return vm_last_condition_is_ok(vm) ? VM_OK : VM_ERROR;
}

VM_RET signal_domain_error(tPVM vm, const int error_id, const tINT expected_class_id, tPOBJECT obj)
{
	tOBJECT c;
	OBJECT_SET_BUILT_IN_CLASS(&c, expected_class_id);
	return signal_domain_error_(vm, error_id, &c, obj);
}

VM_RET signal_domain_error_(tPVM vm, const int error_id, tPOBJECT c, tPOBJECT obj)
{
	tPCELL string, condition;
	tOBJECT place;
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (tisl_get_string(vm_get_tisl(vm), vm, condition_table[error_id].name, &string)) return VM_ERROR;
	if (condition_create(vm, condition_table[error_id].error_class_id, string, &unbound, &unbound, &unbound, &place, &condition)) return VM_ERROR;
	// スロット設定
	domain_error_set_expected_class(condition, c);
	domain_error_set_object(condition, obj);

	handle_condition(vm, condition);
	return VM_ERROR;
}

VM_RET signal_stream_error(tPVM vm, const int error_id, tPCELL stream)
{
	tOBJECT obj, place;
	tPCELL string, condition;
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (tisl_get_string(vm_get_tisl(vm), vm, condition_table[error_id].name, &string)) return VM_ERROR;
	if (condition_create(vm, condition_table[error_id].error_class_id, string, &unbound, &unbound, &unbound, &place, &condition)) return VM_ERROR;
	// スロット設定
	cell_to_object(stream, &obj);
	stream_error_set_stream(condition, &obj);

	handle_condition(vm, condition);
	return VM_ERROR;
}

VM_RET signal_parse_error(tPVM vm, const int error_id, tPCELL stream)
{
	tOBJECT obj, place;
	tPCELL string, condition;
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (tisl_get_string(vm_get_tisl(vm), vm, condition_table[error_id].name, &string)) return VM_ERROR;
	if (condition_create(vm, condition_table[error_id].error_class_id, string, &unbound, &unbound, &unbound, &place, &condition)) return VM_ERROR;
	// スロット設定
	cell_to_object(stream, &obj);
	stream_error_set_stream(condition, &obj);

	handle_condition(vm, condition);
	return VM_ERROR;
}

VM_RET signal_undefined_entity(tPVM vm, const int error_id, tPCELL name, const tINT namespace_id)
{
	tOBJECT obj, place;
	tPCELL string, condition;
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (tisl_get_string(vm_get_tisl(vm), vm, condition_table[error_id].name, &string)) return VM_ERROR;
	if (condition_create(vm, condition_table[error_id].error_class_id, string, &unbound, &unbound, &unbound, &place, &condition)) return VM_ERROR;
	// スロット設定
	cell_to_object(name, &obj);
	undefined_entity_set_name(condition, &obj);
	undefined_entity_set_namespace(condition, namespace_id);

	handle_condition(vm, condition);
	return VM_ERROR;
}

VM_RET signal_violation(tPVM vm, const int error_id, tPOBJECT place)
{
	tPCELL string, condition;
	if (tisl_get_string(vm_get_tisl(vm), vm, condition_table[error_id].name, &string)) return VM_ERROR;
	if (condition_create(vm, condition_table[error_id].error_class_id, string, &unbound, &unbound, &unbound, place, &condition)) return VM_ERROR;
	handle_condition(vm, condition);
	return VM_ERROR;
}

///////////////////

static void handle_condition(tPVM vm, tPCELL p)
{
	tOBJECT handler, condition;
	cell_to_object(p, &condition);// conditionの保存に注意

	vm_get_handler(vm, &handler);
	if (handler_is_system_handler(&handler)) {
		// 処理系の用意したハンドラの場合
		if (handler_is_ignore_errors(&handler)) {
			// ignore-errorsで設定したハンドラの場合
			vm_set_last_condition_ignore_errors(vm);
		} else if (handler_is_foreign_function(&handler)) {
			// 外部関数の実行中のデフォルトハンドラの場合
			// 状態を例外状態にして
			// 外部関数に制御を移す
			vm_set_last_condition(vm, &condition);
		} else {
			// デフォルトのハンドラの場合
			// 状態を例外状態にして最上位まで脱出
			// デバックしにくい．なにか用意する？/*!!!*/
			vm_clear_tag(vm);// 途中のタグをすべて無効に
			vm_set_last_condition(vm, &condition);
		}
	} else {
		// ユーザによって設定されたハンドラの場合
		tPOBJECT sp=vm->SP;// 必要ないかも/*!!!*/
		tOBJECT obj;
		// continue-conditionで帰ってくるときのためのタグの設定
		if (vm_push_tag(vm, &condition)) return;
		// 例外ハンドラ実行中はひとつ上（下?）のハンドラが有効になる
		if (vm_push(vm, &handler)) return;
		vm_pop_handler(vm);
		// 引数（例外）をスタックにつんでハンドラの呼び出し
		if (vm_push(vm, &condition)) return;
		if (function_application_form(vm, &handler, SYMBOL_ERROR_HANDLER, 1)==VM_OK) {
			// ハンドラ関数が正常終了している
			// ハンドラ関数は正常終了してはならない
			vm_pop_tag(vm);
			// 制御エラー状態にして終了
			signal_condition(vm, TISL_ERROR_CONTROL_ERROR);
		}
		if (vm_push_handler(vm, &handler)) return;
		vm_get_last_condition(vm, &obj);
		if (object_eql(&condition, &obj)) {
			// continue-conditionによって戻ってきた
			// 状態をクリアして終了
			vm_set_last_condition_ok(vm);
		}
		// 登録したタグは脱出命令の方で回収する
		// 既に無効となっているところへの脱出を防ぐため
		// 
		vm->SP=sp;// 必要ないかも/*!!!*/
	}
}

static VM_RET default_condition_create(tPVM vm, tINT error_id, tPCELL* cell)
{
	tPCELL string;
	tOBJECT place;
	if (vm_get_function(vm)) {
		cell_to_object(vm_get_function(vm), &place);
	} else {
		OBJECT_SET_UNBOUND(&place);
	}
	if (tisl_get_string(vm_get_tisl(vm), vm, condition_table[error_id].name, &string)) return VM_ERROR;
	if (condition_create(vm, condition_table[error_id].error_class_id, string, &unbound, &unbound, &unbound, &place, cell)) return VM_ERROR;
	return VM_OK;
}

static VM_RET system_condition_create(tPVM vm, tINT error_id, tPCELL* cell)
{
	*cell=0;
	if (error_id==TISL_ERROR_SYSTEM_ERROR) {
		*cell=condition_system_error;
	} else if ((error_id==TISL_ERROR_STACK_OVERFLOW)||(error_id==TISL_ERROR_STACK_UNDERFLOW)) {
		*cell=condition_stack_overflow;
	} else {
		*cell=condition_storage_exhausted;
	}
	return VM_OK;
}

///////////////////////////////////////

VM_RET vm_load(tPVM vm, tPCELL stream, tPOBJECT last)
{
	tOBJECT obj;
	tPCELL package;
	tBOOL old_eos_error;
	tINT sp=vm->SP-vm->stack;
	//
	OBJECT_SET_NIL(last);
	cell_to_object(stream, &obj);
	if (vm_push_temp(vm, &obj)) return VM_ERROR;
	old_eos_error=vm_get_reader_eos_error(vm);
	package=vm_get_current_package(vm);
	vm_set_reader_eos_error(vm);
	while (!check_eos(stream)) {
		vm->SP=vm->stack+sp;
		if (read_form(vm, stream, &obj)) {
			if (vm_last_condition_is_eos_error(vm)) {
				vm_set_last_condition_ok(vm);
			} else {
				goto ERROR;
			}
		} else {
			if (vm_evaluate_top_form(vm, &obj, last)) goto ERROR;
		}
	}
	vm->SP=vm->stack+sp;
	vm_pop_temp(vm);
	if (!old_eos_error) vm_reset_reader_eos_error(vm);
	vm_set_current_package(vm, package);
	return VM_OK;
ERROR:
	vm_pop_temp(vm);
	if (!old_eos_error) vm_reset_reader_eos_error(vm);
	vm_set_current_package(vm, package);
	return VM_ERROR;
}

// nameは記号
VM_RET vm_in_package(tPVM vm, tPCELL name)
{
	tINT i, n;
	tPCELL package, string, bind;
	tOBJECT tmp;

	n=symbol_get_length(name);
	package=symbol_is_complete(name) ? vm_get_top_package(vm) : vm_get_current_package(vm);
	for (i=0; i<n; i++) {
		if (!symbol_get_string(name, i, &string)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		if (package_add_bind(vm, package, string, &bind)) return VM_ERROR;
		// package の private はとりあえず無視しておく
		bind_get_package(bind, &tmp);
		if (OBJECT_IS_PACKAGE(&tmp)) {
			package=OBJECT_GET_CELL(&tmp);
		} else if (OBJECT_IS_UNBOUND(&tmp)) {
			tPCELL pp;
			if (package_create_(vm, bind, list_islisp_system, string, package, &pp)) return VM_ERROR;
			OBJECT_SET_PACKAGE(&tmp, pp);
			bind_set_package(bind, &tmp);
			package=pp;
		} else {
			// ありうる???
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	}
	vm_set_current_package(vm, package);
	return VM_OK;
}


