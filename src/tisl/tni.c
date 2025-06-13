//
// TISL/src/tisl/tni.c
// TISL Ver 4.x
//

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>

#define TISL_VM_STRUCT
#define TISL_PRIMITIVE_OPERATION_PNUM_TABLE
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "reader.h"
#include "writer.h"
#include "built_in_object.h"

/////////////////////////////

// TISL interface

static tINT TISLCALL tisl_get_version(TISL* tisl);
static TISL_RET TISLCALL tisl_destroy_tisl(TISL* tisl);
static TNI* TISLCALL tisl_attach_tni(TISL* tisl);
static void TISLCALL tisl_detach_tni(TISL* tisl, TNI* tni);

// TNI interface

static tINT TISLCALL tni_get_version(TNI* tni);
static TISL* TISLCALL tni_get_tisl(TNI* tni);
static TISL_OBJECT TISLCALL tni_evaluate_top_form(TNI* tni, TISL_OBJECT form);
static TISL_OBJECT TISLCALL tni_get_last_condition(TNI* tni);
static void TISLCALL tni_clear_last_condition(TNI* tni);
static TISL_OBJECT TISLCALL tni_new_global_ref(TNI* tni, TISL_OBJECT obj);
static void TISLCALL tni_delete_global_ref(TNI* tni, TISL_OBJECT obj);
static void TISLCALL tni_delete_local_ref(TNI* tni, TISL_OBJECT obj);

static TISL_OBJECT TISLCALL tni_get_variable(TNI* tni, tCSTRING name);
static void TISLCALL tni_set_variable(TNI* tni, tCSTRING name, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_get_function(TNI* tni, tCSTRING name);
static TISL_OBJECT TISLCALL tni_get_dynamic(TNI* tni, tCSTRING name);
static void TISLCALL tni_set_dynamic(TNI* tni, tCSTRING name, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_get_class(TNI* tni, tCSTRING name);
static void TISLCALL tni_in_package(TNI* tni, tCSTRING name);

static TISL_OBJECT TISLCALL tni_create_integer(TNI* tni, tINT i);
static tINT TISLCALL tni_object_get_integer(TNI* tni, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_create_float(TNI* tni, tFLOAT f);
static tFLOAT TISLCALL tni_object_get_float(TNI* tni, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_create_character(TNI* tni, tINT c);
static tINT TISLCALL tni_object_get_character(TNI* tni, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_create_string(TNI* tni, tCSTRING s);
static tCSTRING TISLCALL tni_object_get_string(TNI* tni, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_create_symbol(TNI* tni, tCSTRING s);
static tCSTRING TISLCALL tni_object_get_symbol(TNI* tni, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_create_foreign_object(TNI* tni, void* fobj, void (*release)(TNI* tni, void* fobj));
static void* TISLCALL tni_object_get_foreign_object(TNI* tni, TISL_OBJECT obj);

static TISL_OBJECT TISLCALL tni_function_call(TNI* tni, TISL_OBJECT function, ...);
static TISL_OBJECT TISLCALL tni_function_call_l(TNI* tni, TISL_OBJECT function, TISL_OBJECT arg_list);

static void TISLCALL tni_throw(TNI* tni, TISL_OBJECT tag, TISL_OBJECT obj);
static TISL_OBJECT TISLCALL tni_convert(TNI* tni, TISL_OBJECT obj, tCSTRING clss);
static TISL_OBJECT TISLCALL tni_assure(TNI* tni, tCSTRING clss, TISL_OBJECT obj);
static void TISLCALL tni_load(TNI* tni, tCSTRING file_name);

static TISL_OBJECT TISLCALL tni_evaluate(TNI* tni, const void* buffer, const tUINT size);
static TISL_OBJECT TISLCALL tni_create_foreign_object_ex(TNI* tni, void* fobj, void (*release)(TNI*, void*), tCSTRING clss);
static void TISLCALL tni_defclass_foreign_class(TNI* tni, tCSTRING name, tCSTRING super);

/////////////////////////////

static tPTISL tisl_get_tisl(TISL* tisl);
static TISL_OBJECT tni_get_bind(TNI* tni, tCSTRING name, const tINT namespace_id, const tBOOL static_p);
static void tni_set_bind(TNI* tni, tCSTRING name, TISL_OBJECT tobj, const tINT namespace_id, const tBOOL static_p);

/////////////////////////////
// TNI

static TNI* vm_set_tni(tPVM vm);

/////////////////////////////

typedef struct TISL_*	PTISL;
typedef struct TNI_*	PTNI;

struct TISL_ tisl_interface={
	tisl_destroy_tisl,
	tisl_attach_tni,
	tisl_detach_tni,
};

struct TNI_ tni_interface={
	tni_get_version,
	tni_get_tisl,
	tni_get_last_condition,
	tni_clear_last_condition,

	tni_new_global_ref,
	tni_delete_global_ref,
	tni_delete_local_ref,

	tni_get_variable,
	tni_set_variable,
	tni_get_function,
	tni_get_dynamic,
	tni_set_dynamic,
	tni_get_class,

	tni_create_integer,
	tni_object_get_integer,

	tni_create_float,
	tni_object_get_float,

	tni_create_character,
	tni_object_get_character,

	tni_create_string,
	tni_object_get_string,

	tni_create_symbol,
	tni_object_get_symbol,

	tni_create_foreign_object,
	tni_object_get_foreign_object,

	tni_function_call,
	tni_function_call_l,

	tni_throw,
	tni_assure,
	tni_convert,
	tni_load,
	tni_evaluate_top_form,
	tni_in_package,

	tni_evaluate,
	tni_create_foreign_object_ex,
};

// create_tni
TISL_IMPORT_OR_EXPORT TISL_RET
create_tni(TISL** tisl, TNI** tni, TNI_INIT_ARGS* args)
{
	PTISL	*tisl_;
	tPTISL	tisl__;
	tTISL_INIT_ARGS	tisl_args;
	tVM_INIT_ARGS	vm_args;

	//
	tisl_=malloc(sizeof(PTISL)+sizeof(tPTISL));
	if (!tisl_) goto ERROR_1;
	// インタフェース関数テーブルの設定
	tisl_[0]=&tisl_interface;
	// TISLとmain VMの作成
	set_tisl_init_args(&tisl_args, args);
	set_vm_init_args(&vm_args, args);
	if (!create_tisl(&tisl__, &tisl_args, &vm_args)) goto ERROR_2;
	tisl_[1]=(PTISL)tisl__;
	tisl_set_interface(tisl__, tisl_);
	// VMからTNIを作成
	*tni=vm_set_tni(tisl_get_main_vm(tisl__));
	if (!*tni) goto ERROR_2;
	*tisl=(TISL*)tisl_;
	return TISL_OK;
	// エラー時の後処理
ERROR_2:
	free(tisl_);
ERROR_1:
	return TISL_ERROR;
}

// TISLのデフォルト初期化引数の設定
// set_default_tni_init_args
TISL_IMPORT_OR_EXPORT TISL_RET
set_default_tni_init_args(TNI_INIT_ARGS* args)
{
	args->argc=0;
	args->argv=0;
	args->envp=0;
	args->init_stack_size=TISL_INIT_STACK_SIZE;
	args->max_stack_size=TISL_MAX_STACK_SIZE;
	return TISL_OK;
}

/////////////////////////////

// TISL interface

static tINT TISLCALL tisl_get_version(TISL* tisl)
{
	return TISL_VERSION_4_8;
}

static TISL_RET TISLCALL tisl_destroy_tisl(TISL* tisl)
{
	return free_tisl(tisl_get_tisl(tisl)) ? TISL_OK : TISL_ERROR;
}

static TNI* TISLCALL tisl_attach_tni(TISL* tisl)
{
	tPVM vm;
	tVM_INIT_ARGS	vm_init_args;
	tPTISL tisl_=tisl_get_tisl(tisl);
	tisl_set_vm_init_args(tisl_, &vm_init_args);
	if (!create_vm(tisl_, &vm, &vm_init_args)) return NULL;
	//
	vm->current_package=vm_get_islisp_package(tisl_get_main_vm(tisl_));
	tisl_attach_vm(tisl_, vm);
	return vm_set_tni(vm);
}

static void TISLCALL tisl_detach_tni(TISL* tisl, TNI* tni)
{
	tPTISL tisl_=tisl_get_tisl(tisl);
	tPVM vm=tni_get_vm(tni);
	// GC状態になっているときはGCを行ってから
	vm_check_tisl_state(vm);
	// 
	tisl_detach_vm(tisl_, vm);
	// tniの開放はVMを開放するとき
}

// TNI interface

static tINT TISLCALL tni_get_version(TNI* tni)
{
	return TNI_VERSION_1_1;
}

static TISL* TISLCALL tni_get_tisl(TNI* tni)
{
	tPTISL tisl=vm_get_tisl(tni_get_vm(tni));
	// プロセスに一つなら大域につくっておく？
	return tisl_get_interface(tisl);
}

static TISL_OBJECT TISLCALL tni_evaluate_top_form(TNI* tni, TISL_OBJECT form)
{
	tOBJECT obj, ret;
	TISL_OBJECT tobj;
	tPVM vm=tni_get_vm(tni);
	tINT sp=vm->SP-vm->stack;

	if (!vm_last_condition_is_ok(vm)) goto ERROR;

	tisl_object_get_object(form, &obj);
	if (vm_evaluate_top_form(vm, &obj, &ret)) goto ERROR;
	if (vm_new_local_ref(vm, &ret, &tobj)) goto ERROR;
	vm->SP=vm->stack+sp;
	return tobj;
ERROR:
	vm->SP=vm->stack+sp;
	return NULL;
}

static TISL_OBJECT TISLCALL tni_get_last_condition(TNI* tni)
{
	tPVM vm=tni_get_vm(tni);
	if (vm_last_condition_is_ok(vm)) {
		return NULL;/*!!!*/// こっちを変える？
	} else {
		tOBJECT condition;
		TISL_OBJECT tobj;
		vm_get_last_condition(vm, &condition);
		// storage-exhaustedどうしよう
		if (vm_new_local_ref(vm, &condition, &tobj)) return (TISL_OBJECT)tisl_object_storage_exhausted;
		return tobj;
	}
}

static void TISLCALL tni_clear_last_condition(TNI* tni)
{
	tPVM vm=tni_get_vm(tni);
	vm_set_last_condition_ok(vm);
}

static TISL_OBJECT TISLCALL tni_new_global_ref(TNI* tni, TISL_OBJECT obj)
{
	TISL_OBJECT ret;

	if (vm_new_global_ref(tni_get_vm(tni), obj, &ret)) {
		return NULL;
	} else {
		return ret;
	}
}

static void TISLCALL tni_delete_global_ref(TNI* tni, TISL_OBJECT obj)
{
	vm_delete_global_ref(tni_get_vm(tni), obj);
}

static void TISLCALL tni_delete_local_ref(TNI* tni, TISL_OBJECT obj)
{
	vm_delete_local_ref(tni_get_vm(tni), obj);
}

static TISL_OBJECT TISLCALL tni_get_variable(TNI* tni, tCSTRING name)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;
	return tni_get_bind(tni, name, NAMESPACE_VARIABLE, tTRUE);
}

static void TISLCALL tni_set_variable(TNI* tni, tCSTRING name, TISL_OBJECT obj)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return;
	tni_set_bind(tni, name, obj, NAMESPACE_VARIABLE, tTRUE);
}

static TISL_OBJECT TISLCALL tni_get_function(TNI* tni, tCSTRING name)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;
	return tni_get_bind(tni, name, NAMESPACE_FUNCTION, tTRUE);
}

static TISL_OBJECT TISLCALL tni_get_dynamic(TNI* tni, tCSTRING name)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;
	return tni_get_bind(tni, name, NAMESPACE_DYNAMIC, tFALSE);
}

static void TISLCALL tni_set_dynamic(TNI* tni, tCSTRING name, TISL_OBJECT obj)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return;
	tni_set_bind(tni, name, obj, NAMESPACE_DYNAMIC, tFALSE);
}

static TISL_OBJECT TISLCALL tni_get_class(TNI* tni, tCSTRING name)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;
	return tni_get_bind(tni, name, NAMESPACE_CLASS, tTRUE);
}

static TISL_OBJECT TISLCALL tni_create_integer(TNI* tni, tINT i)
{
	tOBJECT obj;
	TISL_OBJECT ret;
	tPVM vm=tni_get_vm(tni);

	if (!vm_last_condition_is_ok(vm)) return NULL;

	OBJECT_SET_INTEGER(&obj, i);
	return (vm_new_local_ref(tni_get_vm(tni), &obj, &ret)==VM_OK) ? ret : NULL;
}

static tINT TISLCALL tni_object_get_integer(TNI* tni, TISL_OBJECT obj)
{
	tOBJECT o;
	tPVM vm=tni_get_vm(tni);

	if (!vm_last_condition_is_ok(vm)) return 0;
	tisl_object_get_object(obj, &o);
	if (!OBJECT_IS_INTEGER(&o)) { signal_domain_error(tni_get_vm(tni), TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &o); return 0; }
	return OBJECT_GET_INTEGER(&o);
}

static TISL_OBJECT TISLCALL tni_create_float(TNI* tni, tFLOAT f)
{
	tOBJECT obj;
	TISL_OBJECT ret;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;

	OBJECT_SET_FLOAT(&obj, f);
	return (vm_new_local_ref(tni_get_vm(tni), &obj, &ret)==VM_OK) ? ret : NULL;
}

static tFLOAT TISLCALL tni_object_get_float(TNI* tni, TISL_OBJECT obj)
{
	tOBJECT o;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return 0;

	tisl_object_get_object(obj, &o);
	if (!OBJECT_IS_FLOAT(&o)) { signal_domain_error(tni_get_vm(tni), TISL_ERROR_DOMAIN_ERROR, CLASS_FLOAT, &o); return 0; }
	return OBJECT_GET_FLOAT(&o);
}

static TISL_OBJECT TISLCALL tni_create_character(TNI* tni, tINT c)
{
	tOBJECT obj;
	TISL_OBJECT ret;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;

	OBJECT_SET_CHARACTER(&obj, (tCHAR)c);
	return (vm_new_local_ref(tni_get_vm(tni), &obj, &ret)==VM_OK) ? ret : NULL;
}

static tINT TISLCALL tni_object_get_character(TNI* tni, TISL_OBJECT obj)
{
	tOBJECT o;

	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return 0;
	tisl_object_get_object(obj, &o);
	if (!OBJECT_IS_CHARACTER(&o)) { signal_domain_error(tni_get_vm(tni), TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &o); return '\0'; }
	return (tINT)OBJECT_GET_CHARACTER(&o);
}

static TISL_OBJECT TISLCALL tni_create_string(TNI* tni, tCSTRING s)
{
	tOBJECT obj;
	TISL_OBJECT ret;
	tPCELL cell;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;

	if (tisl_get_string(vm_get_tisl(vm), vm, s, &cell)) return NULL;
	CELL_SET_IMMUTABLE(cell);
	OBJECT_SET_STRING(&obj, cell);
	return (vm_new_local_ref(vm, &obj, &ret)==VM_OK) ? ret : NULL;
}

static tCSTRING TISLCALL tni_object_get_string(TNI* tni, TISL_OBJECT obj)
{
	tOBJECT o;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;

	tisl_object_get_object(obj, &o);
	if (!OBJECT_IS_STRING(&o)) { signal_domain_error(tni_get_vm(tni), TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &o); return NULL; }
	return string_get_string(OBJECT_GET_CELL(&o));
}

static TISL_OBJECT TISLCALL tni_create_symbol(TNI* tni, tCSTRING s)
{
	tPVM vm=tni_get_vm(tni);
	tPCELL stream;
	tOBJECT obj;
	TISL_OBJECT ret;

	if (!vm_last_condition_is_ok(vm)) return NULL;

	if (string_stream_create_input(vm, s, &stream)) return NULL;
	if (read_form(vm, stream, &obj)) return NULL;
	if (!OBJECT_IS_SYMBOL(&obj)&&!OBJECT_IS_NIL(&obj)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj); return NULL; }
	return (vm_new_local_ref(vm, &obj, &ret)==VM_OK) ? ret : NULL;
}

static tCSTRING TISLCALL tni_object_get_symbol(TNI* tni, TISL_OBJECT obj)
{
	tOBJECT o, o2;
	tPCELL stream, string;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;

	tisl_object_get_object(obj, &o);
	if (!OBJECT_IS_SYMBOL(&o)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &o); return NULL; }
	if (string_stream_create_output(vm, &stream)) return NULL;
	cell_to_object(stream, &o2);
	if (vm_push(vm, &o2)) return NULL;
	if (format_object(vm, stream, &o)) { vm_pop(vm); return NULL; }
	if (string_stream_to_string(vm, stream, &string)) { vm_pop(vm); return NULL; }
	vm_pop(vm);
	return string_get_string(string);
}

static TISL_OBJECT TISLCALL tni_create_foreign_object(TNI* tni, void* fobj, void (*release)(TNI* tni, void* fobj))
{
	tPVM vm=tni_get_vm(tni);
	tPCELL cell;
	tOBJECT obj;
	TISL_OBJECT ret;
	if (!vm_last_condition_is_ok(vm)) return NULL;

	if (foreign_object_create(vm, fobj, release, 0, &cell)) return NULL;
	OBJECT_SET_FOREIGN_OBJECT(&obj, cell);
	return (vm_new_local_ref(vm, &obj, &ret)==VM_OK) ? ret : NULL;
}

static void* TISLCALL tni_object_get_foreign_object(TNI* tni, TISL_OBJECT obj)
{
	tOBJECT o;
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return NULL;

	tisl_object_get_object(obj, &o);
	if (!OBJECT_IS_FOREIGN_OBJECT(&o)) { signal_domain_error(tni_get_vm(tni), TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_OBJECT, &o); return NULL; }
	return foreign_object_get_object(OBJECT_GET_CELL(&o));
}

static TISL_OBJECT TISLCALL tni_function_call(TNI* tni, TISL_OBJECT function, ...)
{
	TISL_OBJECT tobj=NULL;
	tPVM vm=tni_get_vm(tni);
	tINT i, anum, pnum, sp=vm->SP-vm->stack;
	tBOOL rest;
	tOBJECT op, tmp;
	va_list ap;

	va_start(ap,  function);

	if (!vm_last_condition_is_ok(vm)) { va_end(ap); return NULL; }

	tisl_object_get_object(function, &op);
	switch (OBJECT_GET_TYPE(&op)) {
	case OBJECT_PRIMITIVE_OPERATOR:
		pnum=primitive_operation_pnum_table[OBJECT_GET_INTEGER(&op)];
		if (pnum<0) { rest=tTRUE; pnum=-pnum; }
		else rest=tFALSE;
		break;
	case OBJECT_FUNCTION:
		pnum=function_get_parameter_number(OBJECT_GET_CELL(&op));
		rest=function_is_rest(OBJECT_GET_CELL(&op));
		break;
	case OBJECT_GENERIC_FUNCTION:
		pnum=gfunction_get_parameter_number(OBJECT_GET_CELL(&op));
		rest=gfunction_is_rest(OBJECT_GET_CELL(&op));
		break;
	case OBJECT_LINKED_FUNCTION:
		pnum=linked_function_get_parameter_number(OBJECT_GET_CELL(&op));
		rest=linked_function_is_rest(OBJECT_GET_CELL(&op));
		break;
	case OBJECT_LOCAL_FUNCTION:
		pnum=function_get_parameter_number(local_function_get_function(OBJECT_GET_CELL(&op)));
		rest=function_is_rest(local_function_get_function(OBJECT_GET_CELL(&op)));
		break;
	default:
		signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FUNCTION, &op);
		goto ERROR;
	}
	if (rest) {
		// restあり
		TISL_OBJECT to;
		for (anum=0, to=va_arg(ap, TISL_OBJECT); to; anum++) {
			// 終了条件が危険？
			tisl_object_get_object(to, &tmp);
			if (vm_push(vm, &tmp)) goto ERROR;
			to=va_arg(ap, TISL_OBJECT);
		}
		/*signal_condition(vm, TISL_ERROR_REST_FUNCTION);
		goto ERROR;*/
	} else {
		anum=0;
		// restなし
		for (i=0; i<pnum; i++) {
			TISL_OBJECT to;
			to=va_arg(ap, TISL_OBJECT);
			if (to) {
				tisl_object_get_object(to, &tmp);
			} else {
				OBJECT_SET_NIL(&tmp);
			}
			if (vm_push(vm, &tmp)) goto ERROR;
			anum=pnum;
		}
		// 引数の数の検査する？
	}
	if (function_application_form(vm, &op, global_symbol[sFUNCALL], anum)) goto ERROR;
	vm_top(vm, &tmp);
	// 戻り値から局所参照の作成
	if (vm_new_local_ref(vm, &tmp, &tobj)) goto ERROR;
	vm_pop(vm);
	va_end(ap);
	vm->SP=vm->stack+sp;
	return tobj;
ERROR:
	va_end(ap);
	vm->SP=vm->stack+sp;
	return NULL;
}

static TISL_OBJECT TISLCALL tni_function_call_l(TNI* tni, TISL_OBJECT function, TISL_OBJECT arg_list)
{
	TISL_OBJECT tobj=NULL;
	tOBJECT func, tmp;
	tPVM vm=tni_get_vm(tni);
	tINT anum=0, sp=vm->SP-vm->stack;

	if (!vm_last_condition_is_ok(vm)) return NULL;

	// リスト上の引数を評価スタック上に展開しつつ引数の数のカウントを行う
	if (arg_list) {
		tisl_object_get_object(arg_list, &tmp);
		if (OBJECT_IS_CONS(&tmp)) {
			tPCELL p;
			for (p=OBJECT_GET_CELL(&tmp); p; p=cons_get_cdr_cons(p)) {
				cons_get_car(p, &tmp);
				if (vm_push(vm, &tmp)) goto ERROR;
				anum++;
				cons_get_cdr(p, &tmp);
				if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) {
					signal_condition(vm, TISL_ERROR_IMPROPER_ARGUMENT_LIST);
					goto ERROR;
				}
			}
		} else if (!OBJECT_IS_NIL(&tmp)) {
			// arg_listがリストでなかった.
			signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
			goto ERROR;
		}
		vm_pop_temp(vm);
	}
	// 関数の呼び出し
	tisl_object_get_object(function, &func);
	if (function_application_form(vm, &func, global_symbol[sAPPLY], anum)) goto ERROR;
	vm_top(vm, &tmp);
	// 戻り値から局所参照の作成
	if (vm_new_local_ref(vm, &tmp, &tobj)) goto ERROR;
	vm_pop(vm);
	vm->SP=vm->stack+sp;// いらない？
	return tobj;
ERROR:
	vm->SP=vm->stack+sp;
	return NULL;
}

static void TISLCALL tni_throw(TNI* tni, TISL_OBJECT tag, TISL_OBJECT obj)
{
	tOBJECT tag_, obj_;
	tPVM vm=tni_get_vm(tni);

	if (!vm_last_condition_is_ok(vm)) return;

	tisl_object_get_object(tag, &tag_);
	tisl_object_get_object(obj, &obj_);

	if (vm_search_tag(vm, &tag_)) {
		// 脱出先発見
		vm_set_throw_object(vm, &obj_);
		vm_set_last_condition(vm, &tag_);
	} else {
		// 脱出先を見失っている
		signal_condition(vm, TISL_ERROR_CONTROL_ERROR);
	}
}

static TISL_OBJECT TISLCALL tni_assure(TNI* tni, tCSTRING clss, TISL_OBJECT tobj)
{
	tOBJECT obj, c;
	tPCELL	stream, symbol, bind, blist;
	tPVM vm=tni_get_vm(tni);
	tPTISL tisl=vm_get_tisl(vm);

	if (!vm_last_condition_is_ok(vm)) return NULL;

	// name
	if (string_stream_create_input(vm, clss, &stream)) return NULL;
	if (read_form(vm, stream, &obj)) return NULL;
	if (!OBJECT_IS_SYMBOL(&obj)&&!OBJECT_IS_NIL(&obj)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj); return NULL; }
	symbol=OBJECT_IS_SYMBOL(&obj) ? OBJECT_GET_CELL(&obj) : SYMBOL_NIL;
	// bind_list
	if (tisl_get_bind_list(tisl, vm, vm_get_current_package(vm), OBJECT_GET_CELL(&obj), &blist)) return NULL;
	if (!blist) return NULL;
	bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
	if (bind) {
		bind_get_object(bind, NAMESPACE_CLASS, &c);
		if (OBJECT_IS_UNBOUND(&c)) {
			tPCELL string;
			if (tisl_get_string(tisl, vm, clss, &string)) return NULL;
			signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, string, NAMESPACE_CLASS);
			return NULL;
		}
	} else {
		tPCELL string;
		if (tisl_get_string(tisl, vm, clss, &string)) return NULL;
		signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, string, NAMESPACE_CLASS);
		return NULL;
	}

	tisl_object_get_object(tobj, &obj);

	if (object_is_instance(vm, &obj, &c)) {
		return tobj;
	} else {
		signal_domain_error_(vm, TISL_ERROR_DOMAIN_ERROR, &c, &obj);
		return NULL;
	}
}

///////////////////

extern VM_RET convert_to_character(tPVM vm);
extern VM_RET convert_to_integer(tPVM vm);
extern VM_RET convert_to_float(tPVM vm);
extern VM_RET convert_to_symbol(tPVM vm);
extern VM_RET convert_to_string(tPVM vm);
extern VM_RET convert_to_vector(tPVM vm);
extern VM_RET convert_to_list(tPVM vm);

static TISL_OBJECT TISLCALL tni_convert(TNI* tni, TISL_OBJECT obj, tCSTRING clss)
{
	tOBJECT c, obj_;
	TISL_OBJECT ret;
	tPCELL blist, bind, symbol, stream;
	tPVM vm=tni_get_vm(tni);
	tPTISL tisl=vm_get_tisl(vm);

	if (!vm_last_condition_is_ok(vm)) return NULL;

	// パッケージ修飾子に対応していない？
	if (string_stream_create_input(vm, clss, &stream)) return NULL;
	if (read_form(vm, stream, &obj_)) return NULL;
	if (!OBJECT_IS_SYMBOL(&obj_)&&!OBJECT_IS_NIL(&obj_)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj_); return NULL; }
	symbol=OBJECT_IS_SYMBOL(&obj_) ? OBJECT_GET_CELL(&obj_) : SYMBOL_NIL;

	if (tisl_get_bind_list(tisl, vm, vm_get_current_package(vm), symbol, &blist)) return NULL;
	if (!blist) return NULL;
	bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
	if (bind) {
		bind_get_class(bind, &c);
		if (OBJECT_IS_UNBOUND(&c)) {
			tPCELL string;
			if (tisl_get_string(tisl, vm, clss, &string)) return NULL;
			signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, string, NAMESPACE_CLASS);
			return NULL;
		}
	} else {
		tPCELL string;
		if (tisl_get_string(tisl, vm, clss, &string)) return NULL;
		signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, string, NAMESPACE_CLASS);
		return NULL;
	}

	tisl_object_get_object(obj, &obj_);

	if (vm_push(vm, &obj_)) return NULL;
	if (OBJECT_IS_BUILT_IN_CLASS(&c)) {
		switch (OBJECT_GET_INTEGER(&c)) {
		case CLASS_CHARACTER:
			if (convert_to_character(vm)) return NULL;
			break;
		case CLASS_INTEGER:			
			if (convert_to_integer(vm)) return NULL;
			break;
		case CLASS_FLOAT:			
			if (convert_to_float(vm)) return NULL;
		case CLASS_SYMBOL:			
			if (convert_to_symbol(vm)) return NULL;
			break;
		case CLASS_STRING:
			if (convert_to_string(vm)) return NULL;
			break;
		case CLASS_GENERAL_VECTOR:
			if (convert_to_vector(vm)) return NULL;
			break;
		case CLASS_LIST:
			if (convert_to_list(vm)) return NULL;
			break;
		default:
			signal_condition(vm, TISL_ERROR_INVALID_CONVERT_CLASS);
			return NULL;
		}
		vm_top(vm, &obj_);
		if (vm_new_local_ref(vm, &obj_, &ret)) return NULL;
		vm_pop(vm);
		return ret;
	} else {
		signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_BUILT_IN_CLASS, &c);
		return NULL;
	}
}

static void TISLCALL tni_load(TNI* tni, tCSTRING file_name)
{
	tPCELL stream, name;
	tPVM vm=tni_get_vm(tni);
	tPTISL tisl=vm_get_tisl(vm);
	tINT sp=vm->stack-vm->SP;
	tOBJECT tmp;

	if (!vm_last_condition_is_ok(vm)) return;

	if (tisl_get_string(tisl, vm, file_name, &name)) return;
	if (file_stream_create(vm, STREAM_INPUT, name, &stream)) return;
	vm_load(vm, stream, &tmp);
	file_stream_close(vm, stream);
	vm->SP=vm->stack+sp;
}

static void TISLCALL tni_in_package(TNI* tni, tCSTRING name)
{
	tPVM vm=tni_get_vm(tni);
	if (!vm_last_condition_is_ok(vm)) return;
	if (name) {
		// NULLのときはトップパッケージへ
		vm_set_current_package(vm, vm_get_top_package(vm));
	} else {
		// 
		tPCELL stream;
		tOBJECT tmp;
		if (string_stream_create_input(vm, name, &stream)) return;
		if (read_form(vm, stream, &tmp)) return;
		if (!OBJECT_IS_SYMBOL(&tmp)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp); return; }
		vm_in_package(vm, OBJECT_GET_CELL(&tmp));
	}
}

static TISL_OBJECT TISLCALL tni_evaluate(TNI* tni, const void* buffer, const tUINT buffersize)
{
	tPVM vm=tni_get_vm(tni);
	unsigned char* b;
	tPCELL stream;
	tOBJECT tmp;
	tINT sp=vm->SP-vm->stack;
	TISL_OBJECT tobj;

	if (!vm_last_condition_is_ok(vm)) return NULL;

	b=(unsigned char*)malloc(buffersize+1);
	if (!b) { signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED); return NULL; }
	memcpy(b, buffer, buffersize);
	b[buffersize]='\0';
	if (string_stream_create_input(vm, b, &stream)) { free(b); return NULL; }
	free(b);

	cell_to_object(stream, &tmp);
	if (vm_push_temp(vm, &tmp)) return NULL;
	if (vm_load(vm, stream, &tmp)) { vm_pop_temp(vm); goto ERROR; }
	vm_pop_temp(vm);
	if (vm_new_local_ref(vm, &tmp, &tobj)) goto ERROR;
	vm->SP=vm->stack+sp;
	return tobj;
ERROR:
	vm->SP=vm->stack+sp;
	return NULL;
}

static VM_RET read_symbol_from_string(tPVM vm, tCSTRING string, tPCELL* cell)
{
	tPCELL stream;
	tOBJECT tmp;
	if (string_stream_create_input(vm, string, &stream)) return VM_ERROR;
	OBJECT_SET_STRING_STREAM(&tmp, stream);
	if (vm_push_temp(vm, &tmp)) return VM_ERROR;
	if (read_form(vm, stream, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
	vm_pop_temp(vm);
	if (OBJECT_IS_SYMBOL(&tmp)) {
		*cell=OBJECT_GET_CELL(&tmp);
	} else if (OBJECT_IS_NIL(&tmp)) {
		*cell=SYMBOL_NIL;
	} else {
		// parse-error?
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
	}
	return VM_OK;
}

static TISL_OBJECT TISLCALL tni_create_foreign_object_ex(TNI* tni, void* fobj, void (*release)(TNI*, void*), tCSTRING clss)
{
	tPVM vm=tni_get_vm(tni);
	tPCELL cell, p, symbol;
	tOBJECT obj, tmp;
	TISL_OBJECT ret;
	if (!vm_last_condition_is_ok(vm)) return NULL;

	if (read_symbol_from_string(vm, clss, &symbol)) return NULL;
	OBJECT_SET_SYMBOL(&tmp, symbol);
	if (vm_push_temp(vm, &tmp)) return NULL;
	if (tisl_get_class(vm_get_tisl(vm), vm, symbol, &obj)) { vm_pop_temp(vm); return NULL; }
	vm_pop_temp(vm);
	if (OBJECT_IS_FOREIGN_CLASS(&obj)) {
		p=OBJECT_GET_CELL(&obj);
	} else if (OBJECT_IS_BUILT_IN_CLASS(&obj)&&
		(OBJECT_GET_INTEGER(&obj)==CLASS_FOREIGN_OBJECT)) {
		p=0;
	} else {
		signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_CLASS, &obj);
		return NULL;
	}
	if (foreign_object_create(vm, fobj, release, p, &cell)) return NULL;
	OBJECT_SET_FOREIGN_OBJECT(&obj, cell);
	return (vm_new_local_ref(vm, &obj, &ret)==VM_OK) ? ret : NULL;
}

static void TISLCALL tni_defclass_foreign_class(TNI* tni, tCSTRING name, tCSTRING super)
{
	tPCELL name_symbol, super_class, name_string, bind, new_class;
	tOBJECT tmp;
	tPVM vm=tni_get_vm(tni);

	if (!vm_last_condition_is_ok(vm)) return;
	if (super) {
		if (read_symbol_from_string(vm, super, &name_symbol)) return;
		OBJECT_SET_SYMBOL(&tmp, name_symbol);
		if (vm_push_temp(vm, &tmp)) return;
		if (tisl_get_class(vm_get_tisl(vm), vm, name_symbol, &tmp)) { vm_pop_temp(vm); return; }
		vm_pop_temp(vm);
		if (OBJECT_IS_FOREIGN_CLASS(&tmp)) {
			super_class=OBJECT_GET_CELL(&tmp);
		} else if (OBJECT_IS_BUILT_IN_CLASS(&tmp)&&
			(OBJECT_GET_INTEGER(&tmp)==CLASS_FOREIGN_OBJECT)) {
			super_class=0;
		} else {
			signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_CLASS, &tmp);
			return;
		}
	} else {
		super_class=0;
	}
	if (read_symbol_from_string(vm, name, &name_symbol)) return;
	if (!symbol_is_simple(name_symbol)) { signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER); return; }
	OBJECT_SET_SYMBOL(&tmp, name_symbol);
	if (vm_push_temp(vm, &tmp)) return;
	symbol_get_string(name_symbol, 0, &name_string);
	if (package_add_bind(vm, vm_get_current_package(vm), name_string, &bind)) { vm_pop_temp(vm); return; }
	bind_get_class(bind, &tmp);
	if (OBJECT_IS_STANDARD_CLASS(&tmp)) {
		// 標準クラスを無効にして外部クラスを定義
		if (foreign_class_create2_(vm, name_symbol, super_class, &new_class)) { vm_pop_temp(vm); return; }
		standard_class_set_invalid(OBJECT_GET_CELL(&tmp));
		OBJECT_SET_FOREIGN_CLASS(&tmp, new_class);
		bind_set_class(bind, &tmp);
		bind_set_class_private(bind);
		vm_pop_temp(vm);
		return;
	} else if (OBJECT_IS_BUILT_IN_CLASS(&tmp)) {
		// 組込みクラスの再定義は許さない
		vm_pop_temp(vm);
		signal_condition(vm, TISL_ERROR_IMMUTABLE_BINDING);
		return;
	} else if (OBJECT_IS_FOREIGN_CLASS(&tmp)) {
		// 外部クラスの設定を変更
		foreign_class_set_super(OBJECT_GET_CELL(&tmp), super_class);
		vm_pop_temp(vm);
		return;
	} else {
		// unbound? 新規に作成
		if (foreign_class_create2_(vm, name_symbol, super_class, &new_class)) { vm_pop_temp(vm); return; }
		OBJECT_SET_FOREIGN_CLASS(&tmp, new_class);
		bind_set_class(bind, &tmp);
		bind_set_class_private(bind);
		vm_pop_temp(vm);
		return;
	}
}

/////////////////////////////

static tPTISL tisl_get_tisl(TISL* tisl)
{
	return (tPTISL)tisl[1];
}

extern TNI* vm_get_tni(tPVM vm)
{
	return vm->tni;
}

tPVM tni_get_vm(TNI* tni)
{
	return (tPVM)(tni[1]);
}

static TISL_OBJECT tni_get_bind(TNI* tni, tCSTRING name, const tINT namespace_id, const tBOOL static_p)
{
	tPVM	vm=tni_get_vm(tni);
	tPTISL tisl=vm_get_tisl(vm);
	tPCELL	symbol, bind, package, blist;
	tOBJECT obj;

	// name
	if (read_symbol_from_string(vm, name, &symbol)) return NULL;
	// package
	package=static_p ? vm_get_current_function_package(vm) : vm_get_current_package(vm);
	if (tisl_get_bind_list(tisl, vm, package, symbol, &blist)) return NULL;
	bind=bind_list_get_bind(blist, namespace_id, package);
	if (bind) {
		bind_get_object(bind, namespace_id, &obj);
		if (!OBJECT_IS_UNBOUND(&obj)) {
			TISL_OBJECT ret;
			if (vm_new_local_ref(vm, &obj, &ret)) return NULL;
			return ret;
		}
	}
	signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, symbol, namespace_id);
	return NULL;
}

static void tni_set_bind(TNI* tni, tCSTRING name, TISL_OBJECT tobj, const tINT namespace_id, const tBOOL static_p)
{
	tPVM	vm=tni_get_vm(tni);
	tPTISL tisl=vm_get_tisl(vm);
	tPCELL	string, symbol, bind, package, blist;
	tOBJECT obj;

	// name
	if (read_symbol_from_string(vm, name, &symbol)) return;
	// package
	package=static_p ? vm_get_current_function_package(vm) : vm_get_current_package(vm);
	cell_to_object(symbol, &obj);
	if (vm_push_temp(vm, &obj)) return;
	if (tisl_get_bind_list(tisl, vm, package, symbol, &blist)) { vm_pop_temp(vm); return; }
	vm_pop_temp(vm);
	bind=bind_list_get_bind(blist, namespace_id, package);
	tisl_object_get_object(tobj, &obj);
	if (bind) {
		bind_set_object(bind, namespace_id, &obj);
	} else {
		symbol_get_string(symbol, 0, &string);
		if (package_add_bind(vm, package, string, &bind)) return;
		bind_set_object(bind, namespace_id, &obj);
	}
}

static TNI* vm_set_tni(tPVM vm)
{
	PTNI	*tni_;
	//
	tni_=malloc(sizeof(PTNI)+sizeof(tPVM));
	if (!tni_) return NULL;
	// インタフェース関数テーブルの設定
	tni_[0]=&tni_interface;
	// VMの設定
	tni_[1]=(PTNI)vm;
	vm->tni=(TNI*)tni_;
	return vm->tni;
}

///////////////////////////////////////

TISL_IMPORT_OR_EXPORT TISL_OBJECT bit_and(TNI* tni, TISL_OBJECT obj1, TISL_OBJECT obj2)
{
	tINT i1, i2;
	i1=(*tni)->object_get_integer(tni, obj1);
	if ((*tni)->get_last_condition(tni)) return NULL;
	i2=(*tni)->object_get_integer(tni, obj2);
	if ((*tni)->get_last_condition(tni)) return NULL;
	return (*tni)->create_integer(tni, i1&i2);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT bit_or(TNI* tni, TISL_OBJECT obj1, TISL_OBJECT obj2)
{
	tINT i1, i2;
	i1=(*tni)->object_get_integer(tni, obj1);
	if ((*tni)->get_last_condition(tni)) return NULL;
	i2=(*tni)->object_get_integer(tni, obj2);
	if ((*tni)->get_last_condition(tni)) return NULL;
	return (*tni)->create_integer(tni, i1|i2);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT test_car(TNI* tni, TISL_OBJECT arg)
{
	TISL_OBJECT car;
	car=(*tni)->get_function(tni, "car");
	return (*tni)->function_call(tni, car, arg);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT fsum(TNI* tni, TISL_OBJECT list)
{
	TISL_OBJECT sum;
	sum=(*tni)->get_function(tni, "+");
	return (*tni)->function_call_l(tni, sum, list);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT fcons(TNI* tni, TISL_OBJECT car, TISL_OBJECT cdr)
{
	TISL_OBJECT cons;
	cons=(*tni)->get_function(tni, "cons");
	return (*tni)->function_call(tni, cons, car, cdr);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT isquare(TNI* tni, TISL_OBJECT obj) {
	tINT i;
	i=(*tni)->object_get_integer(tni, obj);
	if ((*tni)->get_last_condition(tni)) {
		(*tni)->clear_last_condition(tni);
		i=0;
	}
	return (*tni)->create_integer(tni, i*i);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT i3(TNI* tni, tINT i)
{
	return (*tni)->create_integer(tni, i*i*i);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT f3(TNI* tni, tFLOAT f)
{
	return (*tni)->create_float(tni, f*f*f);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT fi(TNI* tni, tFLOAT f, tINT i)
{
	return (*tni)->create_float(tni, f*i);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT string_to_symbol(TNI* tni, tCSTRING string)
{
	return (*tni)->create_symbol(tni, string);
}

typedef struct FOBJ_ *FOBJ;
struct FOBJ_ {
	TISL_OBJECT obj;
};
void fobj_release(TNI* tni, void* obj) {
	FOBJ fobj=(FOBJ)obj;
	(*tni)->delete_global_ref(tni, fobj->obj);
	free(fobj);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT make_foreign_object(TNI* tni, TISL_OBJECT arg)
{
	FOBJ fobj;
	fobj=(FOBJ)malloc(sizeof(struct FOBJ_));
	fobj->obj=(*tni)->new_global_ref(tni, arg);
	return (*tni)->create_foreign_object(tni, fobj, fobj_release);
}

TISL_IMPORT_OR_EXPORT TISL_OBJECT in_package_test(TNI* tni, TISL_OBJECT arg)
{
	(*tni)->in_package(tni, ":system");
	(*tni)->set_variable(tni, "dummy", arg);
	return NULL;
}

