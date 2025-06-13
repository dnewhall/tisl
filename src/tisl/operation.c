//
// TISL/src/tisl/operation.c
// TISL Ver. 4.x
//

#include <time.h>
#include <math.h>
#include <string.h>
#include <memory.h>

#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "operation.h"
#include "built_in_object.h"
#include "translator.h"
#include "reader.h"
#include "writer.h"

// environment
VM_RET environment_set_argument(tPVM vm, tPCELL environment, const tINT n)
{
	tINT i;
	for (i=0; i<n; i++) {
		if (environment_set_value(vm, environment, i, vm->SP-n+1+i)) return VM_ERROR;
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_discard(tPVM vm)
{
	vm->SP--;
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_nil(tPVM vm)
{
	vm->SP++;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_t(tPVM vm)
{
	vm->SP++;
	OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_integer(tINT i, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_INTEGER(vm->SP, i);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_float(tFLOAT f, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_FLOAT(vm->SP, f);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_character(tCHAR c, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_CHARACTER(vm->SP, c);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_cons(tPCELL cell, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_CONS(vm->SP, cell);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_string(tPCELL cell, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_STRING(vm->SP, cell);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_symbol(tPCELL cell, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_SYMBOL(vm->SP, cell);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_vector(tPCELL cell, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_VECTOR(vm->SP, cell);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_array(tPCELL cell, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_ARRAY(vm->SP, cell);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_cell_object(tPCELL cell, tPVM vm)
{
	vm->SP++;
	cell_to_object(cell, vm->SP);
	return VM_OK;
}

//
VM_RET OPERATION_CALL op_push_stack(tINT offset, tPVM vm)
{
	vm->SP++;
	*vm->SP=*(vm->SP+offset);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_heap(tINT offset, tPVM vm)
{
	vm->SP++;
	return environment_get_value(vm, vm->environment, offset, vm->SP);
}

VM_RET OPERATION_CALL op_push_variable(tPCELL blist, tPVM vm)
{
	tPCELL bind=bind_list_get_bind(blist, NAMESPACE_VARIABLE, vm_get_current_package(vm));
	if (!bind) goto ERROR;
	vm->SP++;
	bind_get_variable(bind, vm->SP);
	return VM_OK;
ERROR:
	return signal_undefined_entity(vm, TISL_ERROR_UNBOUND_VARIABLE, bind_list_get_name(blist), NAMESPACE_VARIABLE);
}

VM_RET OPERATION_CALL op_call_rec(tPVM vm)
{
	tOBJECT value;
	tPCELL func;
	tINT pnum;
	func=vm_get_function(vm);
	pnum=function_get_parameter_number(func);
	if (function_is_heap(func)) {
		tPCELL env;

		if (environment_create_(vm, pnum, 0, &env)) return VM_ERROR;
		if (environment_set_argument(vm, env, pnum)) return VM_ERROR;
		if (function_call(vm, func, env, &value)) return VM_ERROR;
		vm->SP-=pnum-1;
		*vm->SP=value;
	} else {
		if (function_call(vm, func, 0, &value)) return VM_ERROR;
		vm->SP-=pnum-1;
		*vm->SP=value;
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_call_tail_rec(tPVM vm)
{
/*	tPCELL func=vm_get_function(vm);
	tINT i, pnum=function_get_parameter_number(func);

	if (function_is_heap(func)) {
		tPCELL env;

		if (environment_create_(vm, pnum, 0, &env)) return VM_ERROR;
		if (environment_set_argument(vm, env, pnum)) return VM_ERROR;
		vm->SP=vm_get_invoke_point(vm);
		vm_set_environment(vm, env);
	} else {
		tPOBJECT sp=vm->SP;
		vm->SP=vm_get_invoke_point(vm);
		for (i=-pnum+1; i<=0; i++) {
			vm->SP[i]=sp[i];
		}
	}
	return VM_OK;
	*/
	tOBJECT value;
	tPCELL func=vm_get_function(vm);
	tINT pnum=function_get_parameter_number(func), sp=vm->SP-vm->stack;;
	if (function_is_heap(func)) {
		tPCELL env;

		if (environment_create_(vm, pnum, 0, &env)) return VM_ERROR;
		if (environment_set_argument(vm, env, pnum)) return VM_ERROR;
		if (function_call(vm, func, env, &value)) return VM_ERROR;
		vm->SP=vm->stack+sp-pnum+1;
		*vm->SP=value;
	} else {
		if (function_call(vm, func, 0, &value)) return VM_ERROR;
		vm->SP=vm->stack+sp-pnum+1;
		*vm->SP=value;
	}
	return VM_OK;
}

// 大域関数の呼び出し
VM_RET OPERATION_CALL op_call(tINT anum, tPCELL blist, tPVM vm)
{
	tOBJECT function;
	tPCELL bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
	if (!bind) {
		// 関数が存在していない？
		tPCELL name=bind_list_get_name(blist);
		return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, name, NAMESPACE_FUNCTION);
	}
	bind_get_function(bind, &function);
	return function_application_form(vm, &function, bind_list_get_name(blist), anum);
}

// 大域関数の呼び出し
VM_RET OPERATION_CALL op_call_tail(tINT anum, tPCELL blist, tPVM vm)
{
	tOBJECT function;
	tPCELL bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
	if (!bind) {
		tPCELL name=bind_list_get_name(blist);
		return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, name, NAMESPACE_FUNCTION);
	}
	bind_get_function(bind, &function);
	// いまのとこ末尾呼び出しと普通の呼び出しで処理の違いはない
	return function_application_form(vm, &function, bind_list_get_name(blist), anum);
}

VM_RET OPERATION_CALL op_call_bind(tINT anum, tPCELL bind, tPVM vm)
{
	tOBJECT function;
	bind_get_function(bind, &function);
	if (OBJECT_IS_UNBOUND(&function)) {
		tPCELL name, list;
		tOBJECT tmp;
		OBJECT_SET_STRING(&tmp, bind_get_name(bind));
		if (cons_create_(vm, &list, &tmp, &nil)) return VM_ERROR;
		if (tisl_get_symbol(vm_get_tisl(vm), vm, list, tFALSE, &name)) return VM_ERROR;
		return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, name, NAMESPACE_FUNCTION);
	} else {
		return function_application_form(vm, &function, bind_get_name(bind), anum);
	}
}

VM_RET OPERATION_CALL op_call_bind_tail(tINT anum, tPCELL bind, tPVM vm)
{
	tOBJECT function;
	bind_get_function(bind, &function);
	if (OBJECT_IS_UNBOUND(&function)) {
		tPCELL name, list;
		tOBJECT tmp;
		OBJECT_SET_STRING(&tmp, bind_get_name(bind));
		if (cons_create_(vm, &list, &tmp, &nil)) return VM_ERROR;
		if (tisl_get_symbol(vm_get_tisl(vm), vm, list, tFALSE, &name)) return VM_ERROR;
		return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, name, NAMESPACE_FUNCTION);
	} else {
		return function_application_form(vm, &function, bind_get_name(bind), anum);
	}
}

VM_RET OPERATION_CALL op_call_local_stack(tPCELL func, tINT offset, tPVM vm)
{
	tPCELL env;
	tOBJECT obj;
	tINT pnum;
	if (environment_get_value(vm, vm_get_environment(vm), offset, &obj)) return VM_ERROR;
	env= OBJECT_IS_ENVIRONMENT(&obj) ? OBJECT_GET_CELL(&obj) : 0;
	if (function_call(vm, func, env, &obj)) return VM_ERROR;
	pnum=function_get_parameter_number(func);
	vm->SP-=pnum-1;
	*vm->SP=obj;
	return VM_OK;
}

VM_RET OPERATION_CALL op_call_local_heap(tPCELL func, tINT offset, tPVM vm)
{
	tPCELL env, old;
	tOBJECT obj;
	tINT pnum;
	pnum=function_get_parameter_number(func);
	old=vm_get_environment(vm);
	if (environment_get_value(vm, old, offset, &obj)) return VM_ERROR;
	env=OBJECT_IS_ENVIRONMENT(&obj) ? OBJECT_GET_CELL(&obj) : 0;
	if (environment_create_(vm, pnum, env, &env)) return VM_ERROR;
	if (environment_set_argument(vm, env, pnum)) return VM_ERROR;
	if (function_call(vm, func, env, &obj)) return VM_ERROR;
	vm->SP-=pnum-1;
	*vm->SP=obj;
	return VM_OK;
}

VM_RET OPERATION_CALL op_call_local_stack_tail(tPCELL func, tINT offset, tPVM vm)
{
	tPCELL env;
	tOBJECT obj;
	tINT pnum;
	if (environment_get_value(vm, vm_get_environment(vm), offset, &obj)) return VM_ERROR;
	env= OBJECT_IS_ENVIRONMENT(&obj) ? OBJECT_GET_CELL(&obj) : 0;
	if (function_call(vm, func, env, &obj)) return VM_ERROR;
	pnum=function_get_parameter_number(func);
	vm->SP-=pnum-1;
	*vm->SP=obj;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;// みてないけど一応
}

VM_RET OPERATION_CALL op_call_local_heap_tail(tPCELL func, tINT offset, tPVM vm)
{
	tPCELL env, old;
	tOBJECT obj;
	tINT pnum;
	pnum=function_get_parameter_number(func);
	old=vm_get_environment(vm);
	if (environment_get_value(vm, old, offset, &obj)) return VM_ERROR;
	env=OBJECT_IS_ENVIRONMENT(&obj) ? OBJECT_GET_CELL(&obj) : 0;
	if (environment_create_(vm, pnum, env, &env)) return VM_ERROR;
	if (environment_set_argument(vm, env, pnum)) return VM_ERROR;
	vm->SP++;
	OBJECT_SET_ENVIRONMENT(vm->SP, env);
	if (function_call(vm, func, env, &obj)) return VM_ERROR;
	vm->SP-=pnum;
	*vm->SP=obj;
	vm_set_last_condition_ok(vm);
	return VM_ERROR;// みてないけど一応
}

VM_RET OPERATION_CALL op_lambda_in(tPCELL plist, tPVM vm)
{	// いまのとこ何もすることはないはず
	// デバック用に実引数のマップをつくるときには何か処理がいるはず
	return VM_OK;
}

VM_RET OPERATION_CALL op_lambda_out(tPCELL plist, tPVM vm)
{
	tINT n=parameter_list_get_number(plist);
	vm->SP-=n;
	*vm->SP=*(vm->SP+n);
	return VM_OK;
}

VM_RET OPERATION_CALL op_lambda_heap_in(tPCELL plist, tPVM vm)
{
	tINT n=parameter_list_get_number(plist);
	tPCELL env;
	if (environment_create_(vm, n, vm_get_environment(vm), &env)) return VM_ERROR;
	if (environment_set_argument(vm, env, n)) return VM_ERROR;
	vm_set_environment(vm, env);
	return VM_OK;
}

VM_RET OPERATION_CALL op_lambda_heap_out(tPCELL plist, tPVM vm)
{
	tINT n=parameter_list_get_number(plist);
	vm_set_environment(vm, environment_get_environment(vm_get_environment(vm)));
	vm->SP-=n;
	*vm->SP=*(vm->SP+n);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_function(tPCELL blist, tPVM vm)
{
	tPCELL bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
	if (!bind) goto ERROR;
	vm->SP++;
	bind_get_function(bind, vm->SP);
	return VM_OK;
ERROR:
	return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, bind_list_get_name(blist), NAMESPACE_FUNCTION);
}

VM_RET OPERATION_CALL op_push_local_function(tPCELL function, tINT offset, tPVM vm)
{
	tOBJECT obj;
	tPCELL env, p;
	if (environment_get_value(vm, vm_get_environment(vm), offset, &obj)) return VM_ERROR;
	env= OBJECT_IS_ENVIRONMENT(&obj) ? OBJECT_GET_CELL(&obj) : 0;
	if (local_function_create_(vm, function, env, &p)) return VM_ERROR;
	vm->SP++;
	cell_to_object(p, vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_lambda(tPCELL func, tPVM vm)
{
	tPCELL p;
	if (local_function_create_(vm, func, vm_get_environment(vm), &p)) return VM_ERROR;
	vm->SP++;
	OBJECT_SET_LOCAL_FUNCTION(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_labels_in(tPCELL nlist, tPVM vm)
{
	tPCELL env;
	tOBJECT obj;
	if (environment_create_(vm, 1, vm_get_environment(vm), &env)) return VM_ERROR;
	OBJECT_SET_ENVIRONMENT(&obj, env);
	if (environment_set_value(vm, env, 0, &obj)) return VM_ERROR;
	vm_set_environment(vm, env);
	return VM_OK;
}

VM_RET OPERATION_CALL op_labels_out(tPVM vm)
{
	tPCELL env;
	env=vm_get_environment(vm);
	if (!env) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	vm_set_environment(vm, environment_get_environment(env));
	return VM_OK;
}

VM_RET OPERATION_CALL op_flet_in(tPCELL nlist, tPVM vm)
{
	tPCELL env;
	tOBJECT obj;
	if (environment_create_(vm, 1, vm_get_environment(vm), &env)) return VM_ERROR;
	if (vm_get_environment(vm)) {
		OBJECT_SET_ENVIRONMENT(&obj, vm_get_environment(vm));
	} else {
		OBJECT_SET_NIL(&obj);
	}
	if (environment_set_value(vm, env, 0, &obj)) return VM_ERROR;
	vm_set_environment(vm, env);
	return VM_OK;
}

VM_RET OPERATION_CALL op_flet_out(tPVM vm)
{
	tPCELL env;
	env=vm_get_environment(vm);
	if (!env) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	vm_set_environment(vm, environment_get_environment(env));
	return VM_OK;
}

VM_RET OPERATION_CALL op_and_check(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)) {
		return 0;// jmp NEXT
	} else {
		vm->SP--;
		return 1;
	}
}

VM_RET OPERATION_CALL op_or_check(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)) {
		vm->SP--;
		return 1;
	} else {
		return 0;
	}
}

VM_RET OPERATION_CALL op_set_stack(tINT offset, tPVM vm)
{
	*(vm->SP+offset)=*vm->SP;
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_heap(tINT offset, tPVM vm)
{
	return environment_set_value(vm, vm_get_environment(vm), offset, vm->SP);
}

VM_RET OPERATION_CALL op_set_variable(tPCELL blist, tPVM vm)
{
	tPCELL bind=bind_list_get_bind(blist, NAMESPACE_VARIABLE, vm_get_current_package(vm));
	tOBJECT obj;
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNBOUND_VARIABLE, bind_list_get_name(blist), NAMESPACE_VARIABLE);
	bind_get_variable(bind, &obj);
	if (OBJECT_IS_UNBOUND(&obj)) return signal_undefined_entity(vm, TISL_ERROR_UNBOUND_VARIABLE, bind_list_get_name(blist), NAMESPACE_VARIABLE);
	bind_set_variable(bind, vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_dynamic(tPCELL name, tPVM vm)
{
	tPCELL blist, bind;
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), name, &blist)) return tFALSE;
	bind=bind_list_get_bind(blist, NAMESPACE_DYNAMIC, vm_get_current_package(vm));
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, name, NAMESPACE_DYNAMIC);
	bind_set_dynamic(bind, vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_aref(tINT anum, tPVM vm)
{
	tOBJECT obj=*(vm->SP-anum+2);
	if (OBJECT_IS_ARRAY(&obj)) {
		if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(&obj))) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
		if (array_set_object(vm, OBJECT_GET_CELL(&obj), anum-2, vm->SP-anum+1)) return VM_ERROR;
		vm->SP-=anum-1;
		return VM_OK;
	} else if (OBJECT_IS_VECTOR(&obj)) {
		if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(&obj))) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
		if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
		if (!OBJECT_IS_INTEGER(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, vm->SP);
		vm->SP-=2;
		return vector_set_object(vm, OBJECT_GET_CELL(vm->SP+1), OBJECT_GET_INTEGER(vm->SP+2), vm->SP);
	} else if (OBJECT_IS_STRING(&obj)) {
		if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(&obj))) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
		if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
		if (!OBJECT_IS_INTEGER(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, vm->SP);
		vm->SP-=2;
		return string_set_character(vm, OBJECT_GET_CELL(vm->SP+1), OBJECT_GET_INTEGER(vm->SP+2), OBJECT_GET_CHARACTER(vm->SP));
	} else {
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_BASIC_ARRAY, &obj);
	}
}

VM_RET OPERATION_CALL op_set_garef(tINT anum, tPVM vm)
{
	tOBJECT obj=*(vm->SP-anum+2);
	if (!OBJECT_IS_ARRAY(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_GENERAL_ARRAY_A, &obj);
	if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(&obj))) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
	if (array_set_object(vm, OBJECT_GET_CELL(&obj), anum-2, vm->SP-anum+1)) return VM_ERROR;
	vm->SP-=anum-1;
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_elt(tPVM vm)
{
	tINT i;
	if (!OBJECT_IS_INTEGER(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, vm->SP);
	i=OBJECT_GET_INTEGER(vm->SP);
	switch (OBJECT_GET_TYPE(vm->SP-1)) {
	case OBJECT_NIL:
		if (i) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
		break;
	case OBJECT_CONS:
		if (i<0) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
		if (list_set_object(vm, OBJECT_GET_CELL(vm->SP-1), i, vm->SP-2)) return VM_ERROR;
		break;
	case OBJECT_VECTOR:
		if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(vm->SP-1))) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
		if (vector_set_object(vm, OBJECT_GET_CELL(vm->SP-1), i, vm->SP-2)) return VM_ERROR;
		break;
	case OBJECT_STRING:
		if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(vm->SP-1))) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
		if (!OBJECT_IS_CHARACTER(vm->SP-2)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, vm->SP-2);
		if (string_set_character(vm, OBJECT_GET_CELL(vm->SP-1), i, OBJECT_GET_CHARACTER(vm->SP-2))) return VM_ERROR;
		break;
	default:// expected classは？<sequence>??
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, vm->SP);
	}
	vm->SP-=2;
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_property(tPVM vm)
{
	tPCELL symbol, property_name, name, bind;
	if (!OBJECT_IS_SYMBOL(vm->SP-1)&&!OBJECT_IS_NIL(vm->SP-1)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, vm->SP-1);
	if (!OBJECT_IS_SYMBOL(vm->SP)&&!OBJECT_IS_NIL(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, vm->SP);
	symbol=OBJECT_IS_NIL(vm->SP-1) ? SYMBOL_NIL : OBJECT_GET_CELL(vm->SP-1);
	property_name=OBJECT_IS_NIL(vm->SP) ? SYMBOL_NIL : OBJECT_GET_CELL(vm->SP);
	if (!symbol_is_simple(symbol)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
	if (!symbol_is_simple(property_name)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
	if (!symbol_get_string(symbol, 0, &name)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (package_add_bind(vm, vm_get_current_function_package(vm), name, &bind)) return VM_ERROR;
	if (bind_set_property(vm, bind, property_name, vm->SP-2)) return VM_ERROR;
	vm->SP-=2;
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_car(tPVM vm)
{
	if (!OBJECT_IS_CONS(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CONS, vm->SP);
	if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(vm->SP)))
		return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
	vm->SP--;
	cons_set_car(OBJECT_GET_CELL(vm->SP+1), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_cdr(tPVM vm)
{
	if (!OBJECT_IS_CONS(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CONS, vm->SP);
	if (CELL_GET_IMMUTABLE(OBJECT_GET_CELL(vm->SP)))
		return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
	vm->SP--;
	cons_set_cdr(OBJECT_GET_CELL(vm->SP+1), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_set_accessor(tPCELL name, tPVM vm)
{
	if (!OBJECT_IS_INSTANCE(vm->SP)) {
		tOBJECT c;
		OBJECT_SET_STANDARD_CLASS(&c, sclass_standard_object);
		return signal_domain_error_(vm, TISL_ERROR_DOMAIN_ERROR, &c, vm->SP);
	}
	if (instance_set_slot(vm, OBJECT_GET_CELL(vm->SP), name, instance_get_class(OBJECT_GET_CELL(vm->SP)), vm->SP-1)) return VM_ERROR;
	vm->SP--;
	return VM_OK;
}

VM_RET OPERATION_CALL op_push_dynamic(tPCELL name, tPVM vm)
{
	tPCELL blist, bind;
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), name, &blist)) return VM_ERROR;
	bind=bind_list_get_bind(blist, NAMESPACE_DYNAMIC, vm_get_current_package(vm));
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, name, NAMESPACE_DYNAMIC);
	vm->SP++;
	bind_get_dynamic(bind, vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_dynamic_let_init(tPCELL name, tINT n, tPVM vm)
{
	tPCELL blist, bind;
	tOBJECT obj;
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), name, &blist)) return VM_ERROR;
	bind=bind_list_get_bind(blist, NAMESPACE_DYNAMIC, vm_get_current_package(vm));
	OBJECT_SET_BIND(&obj, bind);
	if (vm_push(vm, &obj)) return VM_ERROR;
	obj=*(vm->SP-n);
	bind_get_dynamic(bind, vm->SP-n);
	bind_set_dynamic(bind, &obj);
	return VM_OK;
}

VM_RET OPERATION_CALL op_dynamic_let(tPCELL function, tINT n, tPVM vm)
{
	VM_RET ret;
	tOBJECT value;
	tINT i, sp;
	tPCELL bind;

	sp=vm->SP-vm->stack;
	ret=function_call_(vm, function, &value);
	vm->SP=vm->stack+sp;// 例外時にいる？
	for (i=0; i<n; i++) {
		bind=OBJECT_GET_CELL(vm->SP-n+1+i);
		bind_set_dynamic(bind, vm->SP-n*2+1+i);
	}
	vm->SP-=n*2-1;
	*vm->SP=value;
	return ret;
}

VM_RET OPERATION_CALL op_case_check(tPCELL p, tPVM vm)
{
	tOBJECT obj;
	if (!p) return -1;
	for (; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &obj);
		if (object_eql(&obj, vm->SP)) return -1;
	}
	return 0;
}

VM_RET OPERATION_CALL op_case_result(tPCELL function, tPVM vm)
{
	tOBJECT obj;
	if(function_call_(vm, function, &obj)) {
		return VM_ERROR;
	} else {
		return vm_push(vm, &obj);
	}
}

void OPERATION_CALL op_case_end(tPVM vm)
{
	vm->SP--;
	*vm->SP=*(vm->SP+1);
	return;
}

VM_RET OPERATION_CALL op_case_using_predicate(tPCELL p, tPVM vm)
{
	if (!p) {
		vm->SP++;
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		return VM_OK;
	} else {
		tOBJECT obj;
		for (; p; p=cons_get_cdr_cons(p)) {
			cons_get_car(p, &obj);
			vm->SP++;
			*vm->SP=*(vm->SP-2);
			vm->SP++;
			*vm->SP=*(vm->SP-2);
			vm->SP++;
			*vm->SP=obj;
			if (po_funcall(vm, 3)) return VM_ERROR;
			// nil以外を返したら終了
			if (!OBJECT_IS_NIL(vm->SP)) return VM_OK;
			vm->SP--;
		}
		vm->SP++;
		OBJECT_SET_NIL(vm->SP);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_case_using_check(tPVM vm)
{
	return OBJECT_IS_NIL(vm->SP--) ? 0 : 1;
}

VM_RET OPERATION_CALL op_case_using_result(tPCELL function, tPVM vm)
{
	tOBJECT obj;
	if (function_call_(vm, function, &obj)) return VM_ERROR;
	*++(vm->SP)=obj;
	return VM_OK;
}

void OPERATION_CALL op_case_using_end(tPVM vm)
{
	vm->SP-=2;
	*vm->SP=*(vm->SP+2);
}

VM_RET OPERATION_CALL op_while_check(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)) {
		// NILの場合 while形式は直ちにnilを返す
		return VM_OK;
	} else {
		// nil以外であれば一連の評価形式を評価
		vm->SP--;
		return VM_ERROR;
	}
}

VM_RET OPERATION_CALL op_for_stack_init(tPCELL plist, tPVM vm)
{
	// 多分何もすることはない
	return VM_OK;
}

VM_RET OPERATION_CALL op_for_heap_init(tPCELL plist, tPVM vm)
{
	tINT i, n=parameter_list_get_number(plist);
	tPCELL env;
	if (environment_create_(vm, n, vm_get_environment(vm), &env)) return VM_ERROR;
	for (i=n-1; i>=0; i--) {
		if (environment_set_value(vm, env, i, vm->SP-n+1+i)) return VM_ERROR;
	}
	vm_set_environment(vm, env);
	return VM_OK;
}

VM_RET OPERATION_CALL op_for_test(tPVM vm)
{// nilの場合は繰り返し,nil以外のとき終了
	return OBJECT_IS_NIL(vm->SP--) ? VM_ERROR : VM_OK;
}

VM_RET OPERATION_CALL op_for_stack_iteration(tINT n, tPVM vm)
{
//	tINT i, n=parameter_list_get_number(plist);
//	for (i=0; i<n; i++, vm->SP--) {
//		*(vm->SP-n)=*vm->SP;
//	}
	vm->SP-=n;
	memcpy(vm->SP+1-n, vm->SP+1, sizeof(tOBJECT)*n);
	return VM_OK;
}

VM_RET OPERATION_CALL op_for_heap_iteration(tINT n, tPVM vm)
{
	tINT i;//, n=parameter_list_get_number(plist);
	tPCELL env=vm_get_environment(vm);
	for (i=n-1; i>=0; i--) {
		if (environment_set_value(vm, env, i, vm->SP--)) return VM_ERROR;
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_for_stack_result(tPCELL plist, tPVM vm)
{
	tINT n=parameter_list_get_number(plist);
	vm->SP-=n;
	*vm->SP=*(vm->SP+n);
	return VM_OK;
}

VM_RET OPERATION_CALL op_for_heap_result(tPCELL plist, tPVM vm)
{
	tINT n=parameter_list_get_number(plist);
	vm_set_environment(vm, environment_get_environment(vm_get_environment(vm)));
	vm->SP-=n;
	*vm->SP=*(vm->SP+n);
	return VM_OK;
}


VM_RET OPERATION_CALL op_block(tPCELL function, tPCELL tag, tPVM vm)
{
	tOBJECT t, obj;
	tPOBJECT sp=vm->SP;
	cell_to_object(tag, &t);
	if (vm_push_tag(vm, &t)) return VM_ERROR;
	if (function_call_(vm, function, &obj)) {
		// 何か異常終了した
		vm_get_last_condition(vm, &obj);
		if (object_eql(&t, &obj)) {
			// 自分への非局所脱出
			vm->SP=sp+1;
			vm_get_throw_object(vm, vm->SP);
			vm_set_last_condition_ok(vm);
			return VM_OK;
		} else {
			// 他への脱出
			return VM_ERROR;
		}
	} else {
		// 正常に終了した
		*++(vm->SP)=obj;
		vm_pop_tag(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_return_from(tPCELL tag, tPVM vm)
{
	tOBJECT obj;
	cell_to_object(tag, &obj);
	if (vm_search_tag(vm, &obj)) {
		// 脱出先発見
		vm_set_throw_object(vm, vm->SP);
		vm_set_last_condition(vm, &obj);
		return VM_ERROR;
	} else {
		// 脱出先を見失っている
		return signal_condition(vm, TISL_ERROR_CONTROL_ERROR);
	}
}

VM_RET OPERATION_CALL op_catch(tPCELL function, tPVM vm)
{
	tOBJECT obj;
	tPOBJECT sp=vm->SP;
	obj=*vm->SP;
	if (vm_push_tag(vm, &obj)) return VM_ERROR;
	if (function_call_(vm, function, &obj)) {
		tOBJECT last;
		// 何か異常終了した
		vm_get_last_condition(vm, &last);
		if (object_eql(sp, &last)) {
			// 自分への脱出
			vm->SP=sp;
			vm_get_throw_object(vm, vm->SP);
			vm_set_last_condition_ok(vm);
			return VM_OK;
		} else {
			//他への脱出
			return VM_ERROR;
		}
	} else {
		// 正常に終了した
		vm->SP=sp;
		*vm->SP=obj;
		vm_pop_tag(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_throw(tPVM vm)
{
	if (vm_search_tag(vm, vm->SP-1)) {
		// 脱出先発見
		vm_set_throw_object(vm, vm->SP);
		vm_set_last_condition(vm, vm->SP-1);
		return VM_ERROR;
	} else {
		// 脱出先を見失っている
		return signal_condition(vm, TISL_ERROR_CONTROL_ERROR);
	}
}

VM_RET OPERATION_CALL op_tagbody(tPCELL flist, tPCELL taglist, tPVM vm)
{
	tOBJECT obj;
	tPCELL p, start;
	tINT sp=vm->SP-vm->stack;

	cell_to_object(taglist, &obj);
	if (vm_push_tag(vm,&obj)) return VM_ERROR;
	if (tagbody_tag_list_start_tag(taglist)) {
		start=flist;
	} else {
		start=cons_get_cdr_cons(flist);
	}
	for (p=flist; p; p=cons_get_cdr_cons(p)) {
LOOP:
		cons_get_car(p, &obj);
		if (function_call_(vm, OBJECT_GET_CELL(&obj), &obj)) {
			vm_get_last_condition(vm, &obj);
			if (OBJECT_IS_CONS(&obj)&&(cons_get_cdr_cons(OBJECT_GET_CELL(&obj))==taglist)) {
				// 自分への脱出らしい
				tPCELL pp;
				tOBJECT tag, tmp;
				cons_get_car(OBJECT_GET_CELL(&obj), &tag);
				for (p=start, pp=cons_get_cdr_cons(taglist); p&&pp; p=cons_get_cdr_cons(p), pp=cons_get_cdr_cons(pp)) {
					cons_get_car(pp, &tmp);
					if (OBJECT_GET_CELL(&tmp)==OBJECT_GET_CELL(&tag)) {
//					if (object_eql(&tmp, &tag)) {
//						cell_to_object(taglist, &obj);
						OBJECT_SET_CONS(&obj, taglist);
						if (vm_push_tag(vm, &obj)) return VM_ERROR;
						vm->SP=vm->stack+sp;
						vm_set_last_condition_ok(vm);
						goto LOOP;
					}
				}
				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
			} else {
				// 何か別の非局所脱出
				return VM_ERROR;
			}
		}
	}
	vm_pop_tag(vm);
	vm->SP++;
	*vm->SP=obj;
	return VM_OK;
}

// iiGO (tag . tag-list)
VM_RET OPERATION_CALL op_go(tPCELL tag, tPVM vm)
{
	tOBJECT obj;
	if (!vm_search_tagbody_tag(vm, tag)) return signal_condition(vm, TISL_ERROR_CONTROL_ERROR);
	cell_to_object(tag, &obj);
	vm_set_last_condition(vm, &obj);
	return VM_ERROR;
}

VM_RET OPERATION_CALL op_unwind_protect(tPCELL clean_up, tPCELL body, tPVM vm)
{
	tOBJECT obj, dummy, condition;
	tPOBJECT sp=vm->SP;
	tPCELL env=vm_get_environment(vm);
	VM_RET ret;
	ret=function_call_(vm, body, &obj);
	vm_get_last_condition(vm, &condition);
	vm->SP=sp;
	if (function_call_(vm, clean_up, &dummy)) {
		// clean-up 中に脱出が発生した？
		return VM_ERROR;
	} else {
		// clean-up 終了
		vm->SP=sp+1;
		*vm->SP=obj;
		if (ret==VM_OK) {
			return VM_OK;
		} else {
			vm_set_last_condition(vm, &condition);
			return VM_ERROR;
		}
	}
}

VM_RET OPERATION_CALL op_class(tPCELL blist, tPVM vm)
{
	tPCELL bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, bind_list_get_name(blist), NAMESPACE_CLASS);
	vm->SP++;
	bind_get_class(bind, vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_the(tPCELL blist, tPVM vm)
{
	tPCELL bind;
	tOBJECT c;
	bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, bind_list_get_name(blist), NAMESPACE_CLASS);
	bind_get_class(bind, &c);
	if (!object_is_instance(vm, vm->SP, &c)) {
		tOBJECT tmp;
		tmp=*vm->SP;
		return signal_domain_error_(vm, TISL_ERROR_DOMAIN_ERROR, &c, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_assure(tPCELL blist, tPVM vm)
{
	tPCELL bind;
	tOBJECT c;
	bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, bind_list_get_name(blist), NAMESPACE_CLASS);
	bind_get_class(bind, &c);
	if (!object_is_instance(vm, vm->SP, &c)) {
		tOBJECT tmp;
		tmp=*vm->SP;
		return signal_domain_error_(vm, TISL_ERROR_DOMAIN_ERROR, &c, &tmp);
	}
	return VM_OK;
}

/////////////////////////////

VM_RET convert_to_character(tPVM vm);
VM_RET convert_to_integer(tPVM vm);
VM_RET convert_to_float(tPVM vm);
VM_RET convert_to_symbol(tPVM vm);
VM_RET convert_to_string(tPVM vm);
VM_RET convert_to_vector(tPVM vm);
VM_RET convert_to_list(tPVM vm);

VM_RET OPERATION_CALL op_convert(tPCELL blist, tPVM vm)
{
	tPCELL bind;
	tOBJECT c;
	bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, bind_list_get_name(blist), NAMESPACE_CLASS);
	bind_get_class(bind, &c);
	if (OBJECT_IS_BUILT_IN_CLASS(&c)) {
		switch (OBJECT_GET_INTEGER(&c)) {
		case CLASS_CHARACTER:		return convert_to_character(vm);
		case CLASS_INTEGER:			return convert_to_integer(vm);
		case CLASS_FLOAT:			return convert_to_float(vm);
		case CLASS_SYMBOL:			return convert_to_symbol(vm);
		case CLASS_STRING:			return convert_to_string(vm);
		case CLASS_GENERAL_VECTOR:	return convert_to_vector(vm);
		case CLASS_LIST:			return convert_to_list(vm);
		default:
			return signal_condition(vm, TISL_ERROR_INVALID_CONVERT_CLASS);
		}
	} else {
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_BUILT_IN_CLASS, &c);
	}
}

VM_RET convert_to_character(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_CHARACTER(vm->SP, (tCHAR)OBJECT_GET_INTEGER(vm->SP));
	case OBJECT_CHARACTER:
		return VM_OK;
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
		}
	}
}

VM_RET convert_to_integer(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_CHARACTER:
		OBJECT_SET_INTEGER(vm->SP, (tINT)OBJECT_GET_CHARACTER(vm->SP));
	case OBJECT_INTEGER:
		return VM_OK;
	case OBJECT_STRING:
		{
			tOBJECT obj;
			tPCELL stream;
			if (string_stream_create_input(vm, string_get_string(OBJECT_GET_CELL(vm->SP)), &stream)) return VM_ERROR;
			OBJECT_SET_STRING_STREAM(&obj, stream);
			if (vm_push(vm, &obj)) return VM_ERROR;
			if (read_form(vm, stream, &obj)) return VM_ERROR;
			if (!OBJECT_IS_INTEGER(&obj)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_INTEGER, stream);
			vm_pop(vm);
			*vm->SP=obj;
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
		}
	}
}

VM_RET convert_to_float(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)OBJECT_GET_INTEGER(vm->SP));
	case OBJECT_FLOAT:
		return VM_OK;
	case OBJECT_STRING:
		{
			tOBJECT obj;
			tPCELL stream;
			if (string_stream_create_input(vm, string_get_string(OBJECT_GET_CELL(vm->SP)), &stream)) return VM_ERROR;
			OBJECT_SET_STRING_STREAM(&obj, stream);
			if (vm_push(vm, &obj)) return VM_ERROR;
			if (read_form(vm, stream, &obj)) return VM_ERROR;
			if (!OBJECT_IS_FLOAT(&obj)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_FLOAT, stream);
			vm_pop(vm);
			*vm->SP=obj;
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FLOAT, &tmp);
		}
	}
}

VM_RET convert_to_symbol(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_SYMBOL:
		return VM_OK;
	case OBJECT_STRING:
		{
			tOBJECT obj;
			if (vm_string_to_symbol(vm, OBJECT_GET_CELL(vm->SP), &obj)) return VM_ERROR;
			*vm->SP=obj;
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		}
	}
}

VM_RET convert_to_string(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
	case OBJECT_FLOAT:
	case OBJECT_SYMBOL:
		{
			tPCELL string;
			vm_output_stream_clear(vm);
			if (format_object(vm, vm->private_stream, vm->SP)) return VM_ERROR;
			// 定数文字列を生成している! 変数文字列のほうが良い？/*!!!*/
			if (vm_output_stream_to_string(vm, &string)) return VM_ERROR;
			OBJECT_SET_STRING(vm->SP, string);
			return VM_OK;
		}
	case OBJECT_STRING:
		return VM_OK;
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
		}
	}
}

VM_RET convert_to_vector(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_VECTOR:
		return VM_OK;
	case OBJECT_STRING:
		{
			tINT i, n;
			tPCELL vec, string;
			tOBJECT obj;
			string=OBJECT_GET_CELL(vm->SP);
			n=string_get_length(string)-1;
			if (vector_create_2_(vm, n, &nil, &vec)) return VM_ERROR;
			for (i=0; i<n; i++) {
				if (string_get_character(vm, string, i, &obj)||
					vector_set_object(vm, vec, i, &obj)) return VM_ERROR;
			}
			OBJECT_SET_VECTOR(vm->SP, vec);
			return VM_OK;
		}
	case OBJECT_CONS:
		{
			tINT i, n;
			tPCELL vec, list;
			tOBJECT obj;
			list=OBJECT_GET_CELL(vm->SP);
			n=cons_get_length(list);
			if (vector_create_2_(vm, n, &nil, &vec)) return VM_ERROR;
			for (i=0; i<n; list=cons_get_cdr_cons(list), i++) {
				cons_get_car(list, &obj);
				if (vector_set_object(vm, vec, i, &obj)) return VM_ERROR;
			}
			OBJECT_SET_VECTOR(vm->SP, vec);
			return VM_OK;
		}
	case OBJECT_NIL:
		{
			tPCELL vec;
			if (vector_create_2_(vm, 0, &nil, &vec)) return VM_ERROR;
			OBJECT_SET_VECTOR(vm->SP, vec);
			return VM_OK;
		}
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_GENERAL_VECTOR, &tmp);
		}
	}
}

VM_RET convert_to_list(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_CONS:
	case OBJECT_NIL:
		return VM_OK;
	case OBJECT_STRING:
		{
			tPCELL string;
			tINT n;
			string=OBJECT_GET_CELL(vm->SP);
			n=string_get_length(string)-1;
			if (n) {
				tOBJECT obj, c;
				tINT i;
				tPCELL p, last, list;
				if (string_get_character(vm, string, 0, &c)) return VM_ERROR;
				if (cons_create_(vm, &list, &c, &nil)) return VM_ERROR;
				OBJECT_SET_CONS(&obj, list);
				if (vm_push(vm, &obj)) return VM_ERROR;
				last=list;
				for (i=1; i<n; i++) {
					if (string_get_character(vm, string, i, &c)||
						cons_create_(vm, &p, &c, &nil)) return VM_ERROR;
					OBJECT_SET_CONS(&obj, p);
					cons_set_cdr(last, &obj);
					last=p;
				}
				vm_pop(vm);
				OBJECT_SET_CONS(vm->SP, list);
			} else {
				OBJECT_SET_NIL(vm->SP);
			}
			return VM_OK;
		}
	case OBJECT_VECTOR:
		{
			tPCELL vec;
			tINT n;
			vec=OBJECT_GET_CELL(vm->SP);
			n=vector_get_length(vec);
			if (n) {
				tOBJECT obj;
				tINT i;
				tPCELL p, list, last;

				if (vector_get_object(vm, vec, 0, &obj)||
					cons_create_(vm, &list, &obj, &nil)) return VM_ERROR;
				OBJECT_SET_CONS(&obj, list);
				if (vm_push(vm, &obj)) return VM_ERROR;
				last=list;
				for (i=1; i<n; i++) {
					if (vector_get_object(vm, vec, i, &obj)||
						cons_create_(vm, &p, &obj, &nil)) return VM_ERROR;
					OBJECT_SET_CONS(&obj, p);
					cons_set_cdr(last, &obj);
					last=p;
				}
				vm_pop(vm);
				OBJECT_SET_CONS(vm->SP, list);
			} else {
				OBJECT_SET_NIL(vm->SP);
			}
			return VM_OK;
		}
	default:
		{
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_with_standard_input(tPCELL function, tPVM vm)
{
	VM_RET ret;
	tOBJECT obj;
	tPCELL stream;

	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!string_stream_is_input(stream)) {
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!file_stream_is_input(stream)) {
			tOBJECT tmp;
			tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else {
		tOBJECT tmp;
		tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM_ERROR, &tmp);
	}
	if (vm_push_standard_input(vm, stream)) return VM_ERROR;
	ret=function_call_(vm, function, &obj);
	vm_pop_standard_input(vm);
	*vm->SP=obj;
	return ret;
}

VM_RET OPERATION_CALL op_with_standard_output(tPCELL function, tPVM vm)
{
	VM_RET ret;
	tOBJECT obj;
	tPCELL stream;

	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM_ERROR, &tmp);
	}
	if (vm_push_standard_output(vm, stream)) return VM_ERROR;
	ret=function_call_(vm, function, &obj);
	vm_pop_standard_output(vm);
	*vm->SP=obj;
	return ret;
}

VM_RET OPERATION_CALL op_with_error_output(tPCELL function, tPVM vm)
{
	VM_RET ret;
	tOBJECT obj;
	tPCELL stream;

	if(OBJECT_IS_STRING_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (vm_push_error_output(vm, stream)) return VM_ERROR;
	ret=function_call_(vm, function, &obj);
	vm_pop_error_output(vm);
	*vm->SP=obj;
	return ret;
}

/////////////////////////////

static VM_RET op_with_open_file(tPCELL function, tPCELL plist, tPVM vm, const tINT flag)
{
	tOBJECT obj;
	tPCELL stream, env;
	if (!OBJECT_IS_STRING(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if (!OBJECT_IS_INTEGER(vm->SP)||
		(OBJECT_GET_INTEGER(vm->SP)!=8)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (file_stream_create(vm, flag, OBJECT_GET_CELL(vm->SP-1), &stream)) return VM_ERROR;
	vm->SP--;
	cell_to_object(stream, vm->SP);
	if (parameter_list_is_heap(plist)) {
		if (environment_create_(vm, 1, vm_get_environment(vm), &env)) goto ERROR;
		if (environment_set_value(vm, env, 0, vm->SP)) goto ERROR;
	} else {
		env=vm_get_environment(vm);
	}
	if (function_call(vm, function, env, &obj)) goto ERROR;
	*vm->SP=obj;
	if (file_stream_close(vm, stream)) return VM_ERROR;
	return VM_OK;
ERROR:
	if (file_stream_close(vm, stream)) return VM_ERROR;
	return VM_ERROR;
}

VM_RET OPERATION_CALL op_with_open_input_file(tPCELL function, tPCELL plist, tPVM vm)
{
	return op_with_open_file(function, plist, vm, STREAM_INPUT);
}

VM_RET OPERATION_CALL op_with_open_output_file(tPCELL function, tPCELL plist, tPVM vm)
{
	return op_with_open_file(function, plist, vm, STREAM_OUTPUT);
}

VM_RET OPERATION_CALL op_with_open_io_file(tPCELL function, tPCELL plist, tPVM vm)
{
	return op_with_open_file(function, plist, vm, STREAM_INPUT|STREAM_OUTPUT);
}

VM_RET OPERATION_CALL op_ignore_errors(tPCELL function, tPVM vm)
{
	tOBJECT obj;
	tINT sp=vm->SP-vm->stack;
	vm_push_ignore_errors_handler(vm);
	if (function_call_(vm, function, &obj)) {
		// 何か例外が発生した？
		vm_pop_handler(vm);
		if (vm_last_condition_is_ignore_errors(vm)) {
			// 例外が発生した
			// 無視してnilを返す
			vm->SP=vm->stack+sp+1;
			OBJECT_SET_NIL(vm->SP);
			vm_set_last_condition_ok(vm);
		} else {
			// べつの処理 非局所脱出か？
			return VM_ERROR;
		}
	} else {
		vm->SP++;
		*vm->SP=obj;
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_with_handler(tPCELL function, tPVM vm)
{
	tOBJECT obj;
	if (!OBJECT_IS_FUNCTION(vm->SP)&&
		!OBJECT_IS_LOCAL_FUNCTION(vm->SP)&&
		!OBJECT_IS_GENERIC_FUNCTION(vm->SP)&&
		!OBJECT_IS_LINKED_FUNCTION(vm->SP)) {// 関数が増えたら書き直す/*!!!*/
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FUNCTION, &tmp);
	}
	obj=*vm->SP;
	if (vm_push_handler(vm, &obj)) return VM_ERROR;
	if (function_call_(vm, function, &obj)) { vm_pop_handler(vm); return VM_ERROR; }
	vm_pop_handler(vm);
	*vm->SP=obj;
	return VM_OK;
}

VM_RET OPERATION_CALL op_continue_condition(tPVM vm)
{
	if (!OBJECT_IS_CONDITION(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SERIOUS_CONDITION, &tmp);
	}
	if (condition_is_continuable(OBJECT_GET_CELL(vm->SP-1))) {
		vm_set_last_condition(vm, vm->SP-1);
		vm_set_throw_object(vm, vm->SP);
		return VM_ERROR;
	} else {
		return signal_condition(vm, TISL_ERROR_CONTROL_ERROR);
	}
}

VM_RET OPERATION_CALL op_time(tPCELL function, tPVM vm)
{
	VM_RET ret;
	tOBJECT obj;
	clock_t start, finish;

	start=clock();
	ret=function_call_(vm, function, &obj);
	finish=clock();
	if (format_elapsed_time(vm, vm_get_standard_output(vm), ((tFLOAT)finish-start)/CLOCKS_PER_SEC)) return VM_ERROR;
	if (ret) return VM_ERROR;
	return vm_push(vm, &obj);
}

VM_RET OPERATION_CALL op_quasiquote(tPVM vm)
{
	tPCELL p;
	tOBJECT tmp=*vm->SP;
	if (quasiquote_create(vm, &tmp, &p)) return VM_ERROR;
	OBJECT_SET_QUASIQUOTE(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_quasiquote2(tPVM vm)
{
	if (OBJECT_IS_UNQUOTE_SPLICING(vm->SP-1)) {
		// car部がunquote-splicing だった場合
		tOBJECT form;
		unquote_splicing_get_form(OBJECT_GET_CELL(vm->SP-1), &form);
		if (OBJECT_IS_NIL(&form)) {
			vm->SP--;
			*vm->SP=*(vm->SP+1);
			return VM_OK;
		} else if (OBJECT_IS_CONS(&form)) {
			tPCELL p;
			tOBJECT obj;
			p=OBJECT_GET_CELL(&form);
			cons_get_cdr(p, &obj);
			while (OBJECT_IS_CONS(&obj)) {
				p=OBJECT_GET_CELL(&obj);
				cons_get_cdr(p, &obj);
			}
			if (!OBJECT_IS_NIL(&obj)) { 
				tOBJECT tmp;
				if (vm_get_function(vm))
					cell_to_object(vm_get_function(vm), &tmp);
				else
					OBJECT_SET_UNBOUND(&tmp);
				return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, &tmp);
			}
			cons_set_cdr(p, vm->SP);
			vm->SP--;
			*vm->SP=form;
			return VM_OK;
		} else {
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &form);
		}
	} else {
		tPCELL p;
		if (OBJECT_IS_UNQUOTE_SPLICING(vm->SP)) {
			// cdr部がunquote-splicing の場合
			tOBJECT form, tmp;
			unquote_splicing_get_form(OBJECT_GET_CELL(vm->SP), &form);
			tmp=*(vm->SP-1);
			if (cons_create_(vm, &p, &tmp, &form)) return VM_ERROR;
		} else {
			tOBJECT tmp, tmp2;
			tmp=*(vm->SP-1);
			tmp2=*(vm->SP);
			if (cons_create_(vm, &p, &tmp, &tmp2)) return VM_ERROR;
		}
		vm->SP--;
		OBJECT_SET_CONS(vm->SP, p);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_unquote(tPVM vm)
{
	tPCELL p;
	tOBJECT tmp;
	tmp=*vm->SP;
	if (unquote_create(vm, &tmp, &p)) return VM_ERROR;
	OBJECT_SET_UNQUOTE(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_unquote_splicing(tPVM vm)
{
	tPCELL p;
	tOBJECT tmp;
	tmp=*vm->SP;
	if (!unquote_splicing_create(vm, &tmp, &p)) return VM_ERROR;
	OBJECT_SET_UNQUOTE_SPLICING(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_unquote_splicing2(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)||OBJECT_IS_CONS(vm->SP)) {
		tPCELL p;
		tOBJECT tmp;
		tmp=*vm->SP;
		if (unquote_splicing_create(vm, &tmp, &p)) return VM_ERROR;
		OBJECT_SET_UNQUOTE_SPLICING(vm->SP, p);
		return VM_OK;
	} else {
		tOBJECT tmp;
		tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
	}
}

VM_RET OPERATION_CALL op_call_next_method_around(tINT dymmy, tINT offset, tPVM vm)
{
	tOBJECT tmp;
	tPCELL env, amethod, list;

	if (environment_get_value(vm, vm_get_environment(vm), offset, &tmp)) return VM_ERROR;
	env=OBJECT_GET_CELL(&tmp);
	if (environment_get_value(vm, env, 0, &tmp)) return VM_ERROR;
	amethod=OBJECT_GET_CELL(&tmp);
	if (environment_get_value(vm, env, 1, &tmp)) return VM_ERROR;
	list=OBJECT_GET_CELL(&tmp);
	return amethod_call_around(vm, amethod, list);
}

VM_RET OPERATION_CALL op_call_next_method_primary(tINT dummy, tINT offset, tPVM vm)
{
	tOBJECT tmp;
	tPCELL env, amethod, list;

	if (environment_get_value(vm, vm_get_environment(vm), offset, &tmp)) return VM_ERROR;
	env=OBJECT_GET_CELL(&tmp);
	if (environment_get_value(vm, env, 0, &tmp)) return VM_ERROR;
	amethod=OBJECT_GET_CELL(&tmp);
	if (environment_get_value(vm, env, 1, &tmp)) return VM_ERROR;
	list=OBJECT_GET_CELL(&tmp);
	return amethod_call_primary(vm, amethod, list);
}

VM_RET OPERATION_CALL op_next_method_p_around(tINT dummy, tINT offset, tPVM vm)
{
	vm->SP++;
	OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	return VM_OK;
}

VM_RET OPERATION_CALL op_next_method_p_primary(tINT dummy, tINT offset, tPVM vm)
{
	tOBJECT tmp;
	tPCELL env, list;
	if (environment_get_value(vm, vm_get_environment(vm), offset, &tmp)) return VM_ERROR;
	env=OBJECT_GET_CELL(&tmp);
	if (environment_get_value(vm, env, 1, &tmp)) return VM_ERROR;
	list=OBJECT_GET_CELL(&tmp);
	cons_get_cdr(list, &tmp);
	vm->SP++;
	if (OBJECT_IS_NIL(&tmp)) {
		OBJECT_SET_NIL(vm->SP);
	} else {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_functionp(tPVM vm)
{
	/*!!!*/// 後で増えるので注意
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_PRIMITIVE_OPERATOR:
	case OBJECT_FUNCTION:
	case OBJECT_GENERIC_FUNCTION:
	case OBJECT_LINKED_FUNCTION:
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		break;
	default:
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_apply(tINT anum, tPVM vm)
{
	tINT anum_;
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	// 最後のリストをスタック上に展開する
	{
		tPCELL p;
		tOBJECT arg, rest;
		anum_=anum;
		while (OBJECT_IS_CONS(vm->SP)) {
			p=OBJECT_GET_CELL(vm->SP);
			cons_get_car(p, &arg);
			cons_get_cdr(p, &rest);
			*vm->SP=arg;
			if (vm_push(vm, &rest)) return VM_ERROR;
			anum_++;
		}
		if (!OBJECT_IS_NIL(vm->SP)) return signal_condition(vm, TISL_ERROR_IMPROPER_ARGUMENT_LIST);
		vm->SP--;
		anum_--;
	}
	return op_funcall(anum_, vm);
}

VM_RET OPERATION_CALL op_funcall(tINT anum, tPVM vm)
{
	tOBJECT function;

	if (anum==0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	function=*(vm->SP-anum+1);
	if (function_application_form(vm, &function, 0, anum-1)) return VM_ERROR;
	vm->SP--;
	*vm->SP=*(vm->SP+1);
	return VM_OK;
}

VM_RET OPERATION_CALL op_eq(tPVM vm)
{
	vm->SP--;
	if (object_eql(vm->SP, vm->SP+1)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_eql(tPVM vm)
{
	vm->SP--;
	if (object_eql(vm->SP, vm->SP+1)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_equal(tPVM vm)
{
	vm->SP--;
	if (object_equal(vm->SP, vm->SP+1)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_not(tPVM vm)
{

	if (OBJECT_IS_NIL(vm->SP)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_generic_function_p(tPVM vm)
{
	if (OBJECT_IS_GENERIC_FUNCTION(vm->SP)) 
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_class_of(tPVM vm)
{
	tOBJECT clss;
	object_get_class(vm->SP, &clss);
	*vm->SP=clss;
	return VM_OK;
}

VM_RET OPERATION_CALL op_instancep(tPVM vm)
{
	if (!OBJECT_IS_BUILT_IN_CLASS(vm->SP)&&
		!OBJECT_IS_STANDARD_CLASS(vm->SP)&&
		!OBJECT_IS_FOREIGN_CLASS(vm->SP))
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STANDARD_CLASS, vm->SP);
	vm->SP--;
	if (object_is_instance(vm, vm->SP, vm->SP+1)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_subclassp(tPVM vm)
{
	if (!OBJECT_IS_BUILT_IN_CLASS(vm->SP)&&
		!OBJECT_IS_STANDARD_CLASS(vm->SP)&&
		!OBJECT_IS_FOREIGN_CLASS(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STANDARD_CLASS, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_BUILT_IN_CLASS(vm->SP)&&
		!OBJECT_IS_STANDARD_CLASS(vm->SP)&&
		!OBJECT_IS_FOREIGN_CLASS(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STANDARD_CLASS, &tmp);
	}
	if (class_is_subclass(vm, vm->SP, vm->SP+1))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_symbolp(tPVM vm)
{
	if (OBJECT_IS_SYMBOL(vm->SP)||
		OBJECT_IS_NIL(vm->SP)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_property(tINT anum, tPVM vm)
{
	if (anum==2) {
		if (vm_push(vm, &nil)) return VM_ERROR;
	} else if (anum!=3) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	{
		tPCELL symbol, property_name, name, bind;
		tOBJECT obj;

		if (!OBJECT_IS_SYMBOL(vm->SP-2)&&!OBJECT_IS_NIL(vm->SP-2)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		}
		if (!OBJECT_IS_SYMBOL(vm->SP-1)&&!OBJECT_IS_NIL(vm->SP-1)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		}
		// パッケージ修飾子は許さないことにする
		symbol=OBJECT_IS_NIL(vm->SP-2) ? SYMBOL_NIL : OBJECT_GET_CELL(vm->SP-2);
		property_name=OBJECT_IS_NIL(vm->SP-1) ? SYMBOL_NIL : OBJECT_GET_CELL(vm->SP-1);
		if (!symbol_is_simple(symbol)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		if (!symbol_is_simple(property_name)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		//
		if (!symbol_get_string(symbol, 0, &name)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		if (package_add_bind(vm, vm_get_current_function_package(vm), name, &bind)) return VM_ERROR;
		if (bind_get_property(bind, property_name, &obj)) {
			// 属性値が存在した
			vm->SP-=2;
			*vm->SP=obj;
		} else {
			// 属性値が存在しなかった場合
			vm->SP-=2;
			*vm->SP=*(vm->SP+2);
		}
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_remove_property(tINT anum, tPVM vm)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tPCELL symbol, property_name, name, bind;
		tOBJECT obj;
		if (!OBJECT_IS_SYMBOL(vm->SP-1)&&!OBJECT_IS_NIL(vm->SP-1)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		}
		if (!OBJECT_IS_SYMBOL(vm->SP)&&!OBJECT_IS_NIL(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		}
		symbol=OBJECT_IS_NIL(vm->SP-1) ? SYMBOL_NIL : OBJECT_GET_CELL(vm->SP-1);
		property_name=OBJECT_IS_NIL(vm->SP) ? SYMBOL_NIL : OBJECT_GET_CELL(vm->SP);
		if (!symbol_is_simple(symbol)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		if (!symbol_is_simple(property_name)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		if (!symbol_is_simple(symbol)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		if (!symbol_is_simple(property_name)) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		if (!symbol_get_string(symbol, 0, &name)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		if (package_add_bind(vm, vm_get_current_function_package(vm), name, &bind)) return VM_ERROR;
		bind_remove_property(bind, property_name, &obj);
		vm->SP--;
		*vm->SP=obj;
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_gensym(tPVM vm)
{
	tPCELL symbol;
	if (tisl_gensym(vm_get_tisl(vm), vm, &symbol)) return VM_ERROR;
	vm->SP++;
	OBJECT_SET_SYMBOL(vm->SP, symbol);
	return VM_OK;
}

VM_RET OPERATION_CALL op_numberp(tPVM vm)
{
	if (OBJECT_IS_INTEGER(vm->SP)||
		OBJECT_IS_FLOAT(vm->SP)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_parse_number(tPVM vm)
{
	tOBJECT obj, ret;
	tPCELL stream;
	if (!OBJECT_IS_STRING(vm->SP)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, vm->SP);
	if (string_stream_create_input(vm, string_get_string(OBJECT_GET_CELL(vm->SP)), &stream)) return VM_ERROR;
	cell_to_object(stream, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (read_form(vm, stream, &ret)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	if (!OBJECT_IS_INTEGER(&ret)&&
		!OBJECT_IS_FLOAT(&ret)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_INTEGER, stream);
	*vm->SP=ret;
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_equal(tPVM vm)
{
	tOBJECT obj;

	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(&obj)==OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_INTEGER(&obj)==OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_FLOAT(&obj)==OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(&obj)==OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_not_equal(tPVM vm)
{
	tOBJECT obj;

	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(&obj)!=OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_INTEGER(&obj)!=OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_FLOAT(&obj)!=OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(&obj)!=OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_ge(tPVM vm)
{
	tOBJECT obj;

	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(&obj)<=OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_INTEGER(&obj)<=OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_FLOAT(&obj)<=OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(&obj)<=OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_le(tPVM vm)
{
	tOBJECT obj;

	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(&obj)>=OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_INTEGER(&obj)>=OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_FLOAT(&obj)>=OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(&obj)>=OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_greater(tPVM vm)
{
	tOBJECT obj;

	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(&obj)<OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_INTEGER(&obj)<OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_FLOAT(&obj)<OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(&obj)<OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_less(tPVM vm)
{
	tOBJECT obj;

	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(&obj)>OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_INTEGER(&obj)>OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_FLOAT(&obj)>OBJECT_GET_INTEGER(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(&obj)>OBJECT_GET_FLOAT(vm->SP))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return VM_OK;
}

static VM_RET po_addition2(tPVM vm);
static VM_RET po_addition2_integer(tPVM vm, tINT i);

VM_RET OPERATION_CALL op_addition(tINT anum, tPVM vm)
{
	if (anum==2) {
		return po_addition2(vm);
	} else if (anum==0) {
		// (+)
		vm->SP++;
		OBJECT_SET_INTEGER(vm->SP, 0);
		return VM_OK;
	} else if (anum==1) {
		// (+ number)???
		if (!OBJECT_IS_INTEGER(vm->SP)&&
			!OBJECT_IS_FLOAT(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
		return VM_OK;
	} else {
		tINT sum, i=0, d;
		tFLOAT sumf=0;
		// anum>2
		switch (OBJECT_GET_TYPE(vm->SP))
		{
		case OBJECT_INTEGER:
			sum=OBJECT_GET_INTEGER(vm->SP--);
			i=2;// 最後の演算と最初の引数の取得のため
			while (OBJECT_IS_INTEGER(vm->SP)&&(i<anum)) {
				i++;
				d=OBJECT_GET_INTEGER(vm->SP--);
				sumf=(tFLOAT)sum+d;
				if ((sumf>TISL_MOST_POSITIVE_INTEGER)||(sumf<TISL_MOST_NEGATIVE_INTEGER)) goto FLOAT_SUM;
				sum+=d;
			}
			if (anum==i) return po_addition2_integer(vm, sum);
			sumf=(tFLOAT)sum;
FLOAT_SUM:
		case OBJECT_FLOAT:
			while (i<anum) {
				i++;
				switch (OBJECT_GET_TYPE(vm->SP)) {
				case OBJECT_INTEGER:
					sumf+=OBJECT_GET_INTEGER(vm->SP--);
					break;
				case OBJECT_FLOAT:
					sumf+=OBJECT_GET_FLOAT(vm->SP--);
					break;
				default:
					{
						tOBJECT tmp=*vm->SP;
						return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
					}
				}
			}
			switch (OBJECT_GET_TYPE(vm->SP)) {
			case OBJECT_INTEGER:
				OBJECT_SET_FLOAT(vm->SP, OBJECT_GET_INTEGER(vm->SP)+sumf);
				return VM_OK;
			case OBJECT_FLOAT:
				vm->SP->data.f+=sumf;
				return VM_OK;
			default:
				{
					tOBJECT tmp=*vm->SP;
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	}
}

static VM_RET po_addition2(tPVM vm)
{
	tOBJECT obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		return po_addition2_integer(vm, OBJECT_GET_INTEGER(&obj));
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			OBJECT_SET_FLOAT(vm->SP, OBJECT_GET_INTEGER(vm->SP)+OBJECT_GET_FLOAT(&obj));
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f+=OBJECT_GET_FLOAT(&obj);
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
}

static VM_RET po_addition2_integer(tPVM vm, tINT i)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		{
			tFLOAT f=(tFLOAT)OBJECT_GET_INTEGER(vm->SP)+i;
			if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
				OBJECT_SET_FLOAT(vm->SP, f);
			} else {
				vm->SP->data.i+=i;
			}
		}
		return VM_OK;
	case OBJECT_FLOAT:
		vm->SP->data.f+=i;
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

static VM_RET po_multiplication2(tPVM vm);
static VM_RET po_multiplication2_integer(tPVM vm, tINT i);

VM_RET OPERATION_CALL op_multiplication(tINT anum, tPVM vm)
{
	if (anum==2) {
		return po_multiplication2(vm);
	} else if (anum==0) {
		// (*)
		vm->SP++;
		OBJECT_SET_INTEGER(vm->SP, 1);
		return VM_OK;
	} else if (anum==1) {
		// (* number)
		if (!OBJECT_IS_INTEGER(vm->SP)&&
			!OBJECT_IS_FLOAT(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
		return VM_OK;
	} else { // anum>2
		tINT pai, i, d;
		tFLOAT paif;
		// anum>2
		switch (OBJECT_GET_TYPE(vm->SP))
		{
		case OBJECT_INTEGER:
			pai=OBJECT_GET_INTEGER(vm->SP--);
			i=2;// 最後の演算と最初の引数の取得のため
			while (OBJECT_IS_INTEGER(vm->SP)&&(i<anum)) {
				i++;
				d=OBJECT_GET_INTEGER(vm->SP--);
				paif=(tFLOAT)pai*d;
				if ((paif>TISL_MOST_POSITIVE_INTEGER)||(paif<TISL_MOST_NEGATIVE_INTEGER)) goto FLOAT_PAI;
				pai*=d;
			}
			if (anum==i) return po_multiplication2_integer(vm, pai);
			paif=(tFLOAT)pai;
			goto FLOAT_PAI;
		case OBJECT_FLOAT:
			paif=1;
			i=1;
FLOAT_PAI:
			while (i<anum) {
				i++;
				switch (OBJECT_GET_TYPE(vm->SP)) {
				case OBJECT_INTEGER:
					paif*=OBJECT_GET_INTEGER(vm->SP--);
					break;
				case OBJECT_FLOAT:
					paif*=OBJECT_GET_FLOAT(vm->SP--);
					break;
				default:
					{
						tOBJECT tmp=*vm->SP;
						return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
					}
				}
			}
			switch (OBJECT_GET_TYPE(vm->SP)) {
			case OBJECT_INTEGER:
				OBJECT_SET_FLOAT(vm->SP, OBJECT_GET_INTEGER(vm->SP)*paif);
				return VM_OK;
			case OBJECT_FLOAT:
				vm->SP->data.f*=paif;
				return VM_OK;
			default:
				{
					tOBJECT tmp=*vm->SP;
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	}
}

static VM_RET po_multiplication2(tPVM vm)
{
	tOBJECT obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		return po_multiplication2_integer(vm, OBJECT_GET_INTEGER(&obj));
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			OBJECT_SET_FLOAT(vm->SP, OBJECT_GET_INTEGER(vm->SP)*OBJECT_GET_FLOAT(&obj));
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f*=OBJECT_GET_FLOAT(&obj);
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
}

static VM_RET po_multiplication2_integer(tPVM vm, tINT i)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		{
			tFLOAT f=(tFLOAT)OBJECT_GET_INTEGER(vm->SP)*i;
			if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
				OBJECT_SET_FLOAT(vm->SP, f);
			} else {
				vm->SP->data.i*=i;
			}
		}
		return VM_OK;
	case OBJECT_FLOAT:
		vm->SP->data.f*=i;
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

static VM_RET po_subtraction2(tPVM vm);

VM_RET OPERATION_CALL op_substraction(tINT anum, tPVM vm)
{
	if (anum==2) {
		return po_subtraction2(vm);
	} else if (anum==1) {
		// (- number)
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			vm->SP->data.i=-vm->SP->data.i;// オーバフローする？
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f=-vm->SP->data.f;
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	} else if (anum>2) {
		if (po_addition(vm, anum-1)) return VM_ERROR;
		return po_subtraction2(vm);
	} else {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
}

static VM_RET po_subtraction2(tPVM vm)
{
	tOBJECT obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			{
				tFLOAT f=(tFLOAT)OBJECT_GET_INTEGER(vm->SP)-OBJECT_GET_INTEGER(&obj);
				if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
					OBJECT_SET_FLOAT(vm->SP, f);
				} else {
					vm->SP->data.i=OBJECT_GET_INTEGER(vm->SP)-OBJECT_GET_INTEGER(&obj);
				}
			}
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f-=OBJECT_GET_INTEGER(&obj);
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			OBJECT_SET_FLOAT(vm->SP, OBJECT_GET_INTEGER(vm->SP)-OBJECT_GET_FLOAT(&obj));
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f-=OBJECT_GET_FLOAT(&obj);
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
}

VM_RET OPERATION_CALL op_quotient(tINT anum, tPVM vm)
{
	tOBJECT obj;
	switch (anum) {
	case 0:
	case 1:
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	case 2:
		break;
	default:
		if (po_multiplication(vm, anum-1)) return VM_ERROR;
	}
	obj=*vm->SP--;
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_INTEGER:
		if(OBJECT_GET_INTEGER(&obj)==0) return signal_condition(vm, TISL_ERROR_DIVISION_BY_ZERO);
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			{
				tINT i;
				tFLOAT f;
				i=OBJECT_GET_INTEGER(vm->SP)/OBJECT_GET_INTEGER(&obj);
				f=(tFLOAT)OBJECT_GET_INTEGER(vm->SP)/OBJECT_GET_INTEGER(&obj);
				if (i==f)
					OBJECT_SET_INTEGER(vm->SP, i);
				else
					OBJECT_SET_FLOAT(vm->SP, f);
			}
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f/=OBJECT_GET_INTEGER(&obj);
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(&obj)==0) return signal_condition(vm, TISL_ERROR_DIVISION_BY_ZERO);
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			OBJECT_SET_FLOAT(vm->SP, OBJECT_GET_INTEGER(vm->SP)/OBJECT_GET_FLOAT(&obj));
			return VM_OK;
		case OBJECT_FLOAT:
			vm->SP->data.f/=OBJECT_GET_FLOAT(&obj);
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	default:
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &obj);
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

VM_RET OPERATION_CALL op_reciprocal(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		if (OBJECT_GET_INTEGER(vm->SP)==0) return signal_condition(vm, TISL_ERROR_DIVISION_BY_ZERO);
		if (OBJECT_GET_INTEGER(vm->SP)==1) return VM_OK;
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)1/OBJECT_GET_INTEGER(vm->SP));
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP)==0) return signal_condition(vm, TISL_ERROR_DIVISION_BY_ZERO);
		OBJECT_SET_FLOAT(vm->SP, 1/OBJECT_GET_FLOAT(vm->SP));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_max(tINT anum, tPVM vm)
{
	if (anum==0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (anum==1) return VM_OK;
	switch (OBJECT_GET_TYPE(vm->SP--)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(vm->SP)<OBJECT_GET_INTEGER(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(vm->SP)<OBJECT_GET_INTEGER(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if(OBJECT_GET_INTEGER(vm->SP)<OBJECT_GET_FLOAT(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(vm->SP)<OBJECT_GET_FLOAT(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		{
			tOBJECT tmp=*(vm->SP+1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
	return op_max(anum-1, vm);
}

VM_RET OPERATION_CALL op_min(tINT anum, tPVM vm)
{
	if (anum==0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (anum==1) return VM_OK;
	switch (OBJECT_GET_TYPE(vm->SP--)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (OBJECT_GET_INTEGER(vm->SP)>OBJECT_GET_INTEGER(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(vm->SP)>OBJECT_GET_INTEGER(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if(OBJECT_GET_INTEGER(vm->SP)>OBJECT_GET_FLOAT(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		case OBJECT_FLOAT:
			if (OBJECT_GET_FLOAT(vm->SP)>OBJECT_GET_FLOAT(vm->SP+1)) *vm->SP=*(vm->SP+1);
			break;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
		break;
	default:
		{
			tOBJECT tmp=*(vm->SP+1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
	return op_min(anum-1, vm);
}

VM_RET OPERATION_CALL op_abs(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		if (OBJECT_GET_INTEGER(vm->SP)<0) vm->SP->data.i=-vm->SP->data.i;
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP)<0) vm->SP->data.f=-vm->SP->data.f;
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_exp(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)exp(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		vm->SP->data.f=(tFLOAT)exp(OBJECT_GET_FLOAT(vm->SP));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_log(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)log(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		vm->SP->data.f=(tFLOAT)log(OBJECT_GET_FLOAT(vm->SP));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_expt(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP--)) {
	case OBJECT_INTEGER:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			{
				tINT i=OBJECT_GET_INTEGER(vm->SP+1), n=OBJECT_GET_INTEGER(vm->SP);
				if ((n==0)&&(i<0)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, vm->SP);
				if (i>=0)
					OBJECT_SET_INTEGER(vm->SP, (tINT)pow(n, i));
				else
					OBJECT_SET_FLOAT(vm->SP, (tFLOAT)pow(n, i));
				return VM_OK;
			}
		case OBJECT_FLOAT:
			if ((OBJECT_GET_FLOAT(vm->SP)==0)&&
				(OBJECT_GET_INTEGER(vm->SP+1)<0)) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
			OBJECT_SET_FLOAT(vm->SP, (tFLOAT)pow(OBJECT_GET_FLOAT(vm->SP), OBJECT_GET_INTEGER(vm->SP+1)));
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	case OBJECT_FLOAT:
		switch (OBJECT_GET_TYPE(vm->SP)) {
		case OBJECT_INTEGER:
			if (((OBJECT_GET_INTEGER(vm->SP)==0)&&(OBJECT_GET_FLOAT(vm->SP+1)==0))||
				(OBJECT_GET_INTEGER(vm->SP)<0)) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
			OBJECT_SET_FLOAT(vm->SP, (tFLOAT)pow(OBJECT_GET_INTEGER(vm->SP), OBJECT_GET_FLOAT(vm->SP+1)));
			return VM_OK;
		case OBJECT_FLOAT:
			if (((OBJECT_GET_FLOAT(vm->SP)==0)&&(OBJECT_GET_FLOAT(vm->SP+1)<0))||
				(OBJECT_GET_FLOAT(vm->SP)<0)) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
			OBJECT_SET_FLOAT(vm->SP, (tFLOAT)pow(OBJECT_GET_FLOAT(vm->SP), OBJECT_GET_FLOAT(vm->SP+1)));
			return VM_OK;
		default:
			{
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_sqrt(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		{
			tINT i;
			tFLOAT f;
			if (OBJECT_GET_INTEGER(vm->SP)<0) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
			}
			f=(tFLOAT)sqrt(OBJECT_GET_INTEGER(vm->SP));
			i=(tINT)f;
			if (i==f) 
				OBJECT_SET_INTEGER(vm->SP, i);
			else 
				OBJECT_SET_FLOAT(vm->SP, f);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP)<0) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)sqrt(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_sin(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)sin(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)sin(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_cos(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)cos(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)cos(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_tan(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)tan(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)tan(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_atan(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)atan(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)atan(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_atan2(tPVM vm)
{
	tFLOAT x1, x2;
	switch (OBJECT_GET_TYPE(vm->SP)){
	case OBJECT_INTEGER:	x2=(tFLOAT)OBJECT_GET_INTEGER(vm->SP); break;
	case OBJECT_FLOAT:		x2=OBJECT_GET_FLOAT(vm->SP); break;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
	vm->SP--;
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:	x1=(tFLOAT)OBJECT_GET_INTEGER(vm->SP); break;
	case OBJECT_FLOAT:		x1=OBJECT_GET_FLOAT(vm->SP); break;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
	OBJECT_SET_FLOAT(vm->SP, (tFLOAT)atan2(x1, x2));
	return VM_OK;
}

VM_RET OPERATION_CALL op_sinh(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)sinh(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)sinh(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_cosh(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)cosh(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)cosh(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_tanh(tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)tanh(OBJECT_GET_INTEGER(vm->SP)));
		return VM_OK;
	case OBJECT_FLOAT:
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)tanh(OBJECT_GET_FLOAT(vm->SP)));
		return VM_OK;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_atanh(tPVM vm)
{
	double z;
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_INTEGER:	z=(double)OBJECT_GET_INTEGER(vm->SP); break;
	case OBJECT_FLOAT:		z=(double)OBJECT_GET_FLOAT(vm->SP); break;
	default:
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
	z=(log(1+z)-log(1-z))/2;
	OBJECT_SET_FLOAT(vm->SP, (tFLOAT)z);
	return VM_OK;
}

