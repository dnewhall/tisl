//
// TISL/src/tisl/c/opcode.c
// TISL Ver. 4.0
//

#define TISL_VM_STRUCT
#include "../../../include/tni.h"
#include "../object.h"
#include "../vm.h"
#include "../tisl.h"
#include "opcode.h"
#include "../operation.h"
#include "../translator.h"
#include "../built_in_object.h"

extern tCELL function_get_command(tPCELL function, const tINT pc);
#define FETCH(function, pc)	(function_get_command(function, (++*(pc))))

VM_RET c_discard(tPVM vm, tPCELL function, tINT* pc)
{
	return op_discard(vm);
}

VM_RET c_push_nil(tPVM vm, tPCELL function, tINT* pc)
{
	return op_push_nil(vm);
}

VM_RET c_push_t(tPVM vm, tPCELL function, tINT* pc)
{
	return op_push_t(vm);
}

VM_RET c_push_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i=FETCH(function, pc).i;
	return op_push_integer(i, vm);
}

VM_RET c_push_float(tPVM vm, tPCELL function, tINT* pc)
{
	tFLOAT f=FETCH(function, pc).f;
	return op_push_float(f, vm);
}

VM_RET c_push_character(tPVM vm, tPCELL function, tINT* pc)
{
	tCHAR c=FETCH(function, pc).c;
	return op_push_character(c, vm);
}

VM_RET c_push_cons(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL cell=FETCH(function, pc).cell;
	return op_push_cons(cell, vm);
}

VM_RET c_push_string(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL cell=FETCH(function, pc).cell;
	return op_push_string(cell, vm);
}

VM_RET c_push_symbol(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL cell=FETCH(function, pc).cell;
	return op_push_symbol(cell, vm);
}

VM_RET c_push_vector(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL cell=FETCH(function, pc).cell;
	return op_push_vector(cell, vm);
}

VM_RET c_push_array(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL cell=FETCH(function, pc).cell;
	return op_push_array(cell, vm);
}

VM_RET c_push_cell_object(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL cell=FETCH(function, pc).cell;
	return op_push_cell_object(cell, vm);
}

VM_RET c_push_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	return op_push_stack(offset, vm);
}

VM_RET c_push_heap(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	return op_push_heap(offset, vm);
}

VM_RET c_push_variable(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_push_variable(blist, vm);
}

VM_RET c_call_rec(tPVM vm, tPCELL function, tINT* pc)
{
	return op_call_rec(vm);
}

VM_RET c_call_rec_tail(tPVM vm, tPCELL function, tINT* pc)
{
	op_call_tail_rec(vm);
	return VM_ERROR;
}

VM_RET c_call(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	tINT anum=FETCH(function, pc).i;
	return op_call(anum, blist, vm);
}

VM_RET c_call_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	tINT anum=FETCH(function, pc).i;
	op_call_tail(anum, blist, vm);
	return VM_ERROR;
}

VM_RET c_call_bind(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL bind=FETCH(function, pc).cell;
	tINT anum=FETCH(function, pc).i;
	return op_call_bind(anum, bind, vm);
}

VM_RET c_call_bind_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL bind=FETCH(function, pc).cell;
	tINT anum=FETCH(function, pc).i;
	op_call_bind_tail(anum, bind, vm);
	return VM_ERROR;
}

VM_RET c_call_local_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	tPCELL func=FETCH(function, pc).cell;
	return op_call_local_stack(func, offset, vm);
}

VM_RET c_call_local_stack_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	tPCELL func=FETCH(function, pc).cell;
	op_call_local_stack(func, offset, vm);
	return VM_ERROR;
}

VM_RET c_call_local_heap(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	tPCELL func=FETCH(function, pc).cell;
	return op_call_local_heap(func, offset, vm);
}

VM_RET c_call_local_heap_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	tPCELL func=FETCH(function, pc).cell;
	op_call_local_heap(func, offset, vm);
	return VM_ERROR;
}

VM_RET c_call_next_method_around(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	return op_call_next_method_around(dummy, offset, vm);
}

VM_RET c_call_next_method_around_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	op_call_next_method_around(dummy, offset, vm);
	return VM_ERROR;
}

VM_RET c_call_next_method_primary(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	return op_call_next_method_primary(dummy, offset, vm);
}

VM_RET c_call_next_method_primary_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	op_call_next_method_primary(dummy, offset, vm);
	return VM_ERROR;
}

VM_RET c_next_method_p_around(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	return op_next_method_p_around(dummy, offset, vm);
}

VM_RET c_next_method_p_around_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	op_next_method_p_around(dummy, offset, vm);
	return VM_ERROR;
}

VM_RET c_next_method_p_primary(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	return op_next_method_p_primary(dummy, offset, vm);
}

VM_RET c_next_method_p_primary_tail(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 dummy=FETCH(function, pc).i;
	op_next_method_p_primary(dummy, offset, vm);
	return VM_ERROR;
}

VM_RET c_ret(tPVM vm, tPCELL function, tINT* pc)
{
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

VM_RET c_lambda_in(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell;
	return op_lambda_in(plist, vm);
}

VM_RET c_lambda_out(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell;
	return op_lambda_out(plist, vm);
}

VM_RET c_lambda_heap_in(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell;
	return op_lambda_heap_in(plist, vm);
}

VM_RET c_lambda_heap_out(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell;
	return op_lambda_heap_out(plist, vm);
}

VM_RET c_push_function(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_push_function(blist, vm);
}

VM_RET c_push_lfunction(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	tPCELL f=FETCH(function, pc).cell;
	return op_push_local_function(f, offset, vm);
}

VM_RET c_push_lambda(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL lambda=FETCH(function, pc).cell;
	return op_push_lambda(lambda, vm);
}

VM_RET c_labels_in(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL nlist=FETCH(function, pc).cell;
	return op_labels_in(nlist, vm);
}

VM_RET c_labels_out(tPVM vm, tPCELL function, tINT* pc)
{
	return op_labels_out(vm);
}

VM_RET c_flet_in(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL nlist=FETCH(function, pc).cell;
	return op_flet_in(nlist, vm);
}

VM_RET c_flet_out(tPVM vm, tPCELL function, tINT* pc)
{
	return op_flet_out(vm);
}

VM_RET c_and(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	if (!OBJECT_IS_NIL(vm->SP)) {
		tOBJECT tmp;
		vm->SP--;
		if (function_call_(vm, f, &tmp)) return VM_ERROR;
		*++(vm->SP)=tmp;
	}
	return VM_OK;
}

VM_RET c_or(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	if (OBJECT_IS_NIL(vm->SP)) {
		tOBJECT tmp;
		vm->SP--;
		if (function_call_(vm, f, &tmp)) return VM_ERROR;
		*++(vm->SP)=tmp;
	}
	return VM_OK;
}

VM_RET c_set_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	return op_set_stack(offset, vm);
}

VM_RET c_set_heap(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i;
	return op_set_heap(offset, vm);
}

VM_RET c_set_variable(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_set_variable(blist, vm);
}

VM_RET c_set_dynamic(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL name=FETCH(function, pc).cell;
	return op_set_dynamic(name, vm);
}

VM_RET c_set_aref(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_set_aref(anum, vm);
}

VM_RET c_set_garef(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_set_garef(anum, vm);
}

VM_RET c_set_elt(tPVM vm, tPCELL function, tINT* pc)
{
	return op_set_elt(vm);
}

VM_RET c_set_property(tPVM vm, tPCELL function, tINT* pc)
{
	return op_set_property(vm);
}

VM_RET c_set_car(tPVM vm, tPCELL function, tINT* pc)
{
	return op_set_car(vm);
}

VM_RET c_set_cdr(tPVM vm, tPCELL function, tINT* pc)
{
	return op_set_cdr(vm);
}

VM_RET c_set_accessor(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL name=FETCH(function, pc).cell;
	return op_set_accessor(name, vm);
}

VM_RET c_push_dynamic(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL name=FETCH(function, pc).cell;
	return op_push_dynamic(name, vm);
}

VM_RET c_dynamic_let(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i, n, sp;
	VM_RET ret;
	tPCELL body;
	tOBJECT obj;
	tPCELL bind;

	n=FETCH(function, pc).i;
	/**/
	for (i=1; i<=n; i++) {
		tPCELL blist, name;
		name=FETCH(function, pc).cell;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), name, &blist)) return VM_ERROR;
		bind=bind_list_get_bind(blist, NAMESPACE_DYNAMIC, vm_get_current_package(vm));
		OBJECT_SET_BIND(&obj, bind);
		if (vm_push(vm, &obj)) return VM_ERROR;
		obj=*(vm->SP-n);
		bind_get_dynamic(bind, vm->SP-n);
		bind_set_dynamic(bind, &obj);
	}
	/*body部の実行*/
	body=FETCH(function, pc).cell;
	sp=vm->SP-vm->stack;
	ret=function_call_(vm, body, &obj);
	vm->SP=vm->stack+sp;
	for (i=0; i<n; i++) {
		bind=OBJECT_GET_CELL(vm->SP-n+1+i);
		bind_set_dynamic(bind, vm->SP-n*2+1+i);
	}
	vm->SP-=n*2-1;
	*vm->SP=obj;
	return ret;
}

VM_RET c_if(tPVM vm, tPCELL function, tINT* pc)
{
	tOBJECT tmp;
	tPCELL then_f=FETCH(function, pc).cell,
		   else_f=FETCH(function, pc).cell;
	if (OBJECT_IS_NIL(vm->SP--)) {
		if (function_call_(vm, else_f, &tmp)) return VM_ERROR;
	} else {
		if (function_call_(vm, then_f, &tmp)) return VM_ERROR;
	}
	*++(vm->SP)=tmp;
	return VM_OK;
}

VM_RET c_case(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i, n=FETCH(function, pc).i;
	tBOOL f=tTRUE;
	for (i=0; i<n; i++) {
		tPCELL keylist=FETCH(function, pc).cell,
			   form=FETCH(function, pc).cell;
		if (f) {
			tBOOL ff=tFALSE;
			if (keylist) {
				tPCELL p;
				for (p=keylist; p; p=cons_get_cdr_cons(p)) {
					tOBJECT tmp;
					cons_get_car(p, &tmp);
					if (object_eql(&tmp, vm->SP)) { ff=tTRUE; break; }
				}
			} else {
				ff=tTRUE;
			}
			if (ff) {
				tOBJECT tmp;
				if (function_call_(vm, form, &tmp)) return VM_ERROR;
				f=tFALSE;
				*vm->SP=tmp;
			}
		}
	}
	if (f) {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET c_case_using(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i, n=FETCH(function, pc).i;
	tBOOL f=tTRUE;
	for (i=0; i<n; i++) {
		tPCELL keylist=FETCH(function, pc).cell,
			   form=FETCH(function, pc).cell;
		if (f) {
			tBOOL ff=tFALSE;
			if (keylist) {
				tPCELL p;
				for (p=keylist; p; p=cons_get_cdr_cons(p)) {
					tOBJECT tmp;
					cons_get_car(p, &tmp);
					vm->SP++;
					*vm->SP=*(vm->SP-2);
					vm->SP++;
					*vm->SP=*(vm->SP-2);
					vm->SP++;
					*vm->SP=tmp;
					if (po_funcall(vm, 3)) return VM_ERROR;
					// nil以外を返したら終了
					if (!OBJECT_IS_NIL(vm->SP)) { ff=tTRUE; vm->SP--; break; }
					vm->SP--;
			}
			} else {
				ff=tTRUE;
			}
			if (ff) {
				tOBJECT tmp;
				if (function_call_(vm, form, &tmp)) return VM_ERROR;
				f=tFALSE;
				*vm->SP=tmp;
			}
		}
	}
	if (f) {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET c_while(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL test=FETCH(function, pc).cell,
		   body=FETCH(function, pc).cell;
	while (1) {
		tOBJECT tmp;
		if (function_call_(vm, test, &tmp)) return VM_ERROR;
		if (OBJECT_IS_NIL(&tmp)) { *++(vm->SP)=tmp; return VM_OK; }
		if (function_call_(vm, body, &tmp)) return VM_ERROR;
	}
}

VM_RET c_for_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i, n;
	tPCELL plist=FETCH(function, pc).cell,
		   endtest=FETCH(function, pc).cell,
		   result=FETCH(function, pc).cell,
		   iteration=FETCH(function, pc).cell;
	n=parameter_list_get_number(plist);
	while (1) {
		tOBJECT tmp;
		if (function_call_(vm, endtest, &tmp)) return VM_ERROR;
		if (OBJECT_IS_NIL(&tmp)) {
			if (function_call_(vm, iteration, &tmp)) return VM_ERROR;
			*++(vm->SP)=tmp;/*!!!*/
			for (i=0; i<n; i++, vm->SP--) {
				*(vm->SP-n)=*(vm->SP);
			}
		} else {
			if (function_call_(vm, result, &tmp)) return VM_ERROR;
			vm->SP-=n-1;
			*vm->SP=tmp;
			return VM_OK;
		}
	}
}

VM_RET c_for_heap(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i, n;
	tPCELL plist=FETCH(function, pc).cell,
		   endtest=FETCH(function, pc).cell,
		   result=FETCH(function, pc).cell,
		   iteration=FETCH(function, pc).cell,
		   env, old;
	n=parameter_list_get_number(plist);
	old=vm_get_environment(vm);
	if (environment_create_(vm, n, old, &env)) return VM_ERROR;
	for (i=n-1; i>=0; i--) {
		if (environment_set_value(vm, env, i, vm->SP-n+1+i)) return VM_ERROR;
	}
	vm_set_environment(vm, env);
	while (1) {
		tOBJECT tmp;
		if (function_call_(vm, endtest, &tmp)) { vm_set_environment(vm, old); return VM_ERROR; }
		if (OBJECT_IS_NIL(&tmp)) {
			if (function_call_(vm, iteration, &tmp)) { vm_set_environment(vm, old); return VM_ERROR; }
			*++(vm->SP)=tmp;/*!!!*/
			for (i=n-1; i>=0; i--) {
				if (environment_set_value(vm, env, i, vm->SP--)) { vm_set_environment(vm, old); return VM_ERROR; }
			}
		} else {
			if (function_call_(vm, result, &tmp)) { vm_set_environment(vm, old); return VM_ERROR; }
			vm->SP-=n-1;
			*vm->SP=tmp;
			vm_set_environment(vm, old);
			return VM_OK;
		}
	}
}

VM_RET c_block(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell,
		   tag=FETCH(function, pc).cell;
	return op_block(f, tag, vm);
}

VM_RET c_return_from(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL tag=FETCH(function, pc).cell;
	return op_return_from(tag, vm);
}

VM_RET c_catch(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	return op_catch(f, vm);
}

VM_RET c_throw(tPVM vm, tPCELL functin, tINT* pc)
{
	return op_throw(vm);
}

VM_RET c_tagbody(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL taglist=FETCH(function, pc).cell,
		   flist=FETCH(function, pc).cell;
	return op_tagbody(flist, taglist, vm);
}

VM_RET c_go(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL tag=FETCH(function, pc).cell;
	return op_go(tag, vm);
}

VM_RET c_unwind_protect(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL body=FETCH(function, pc).cell,
		   cleanup=FETCH(function, pc).cell;
	return op_unwind_protect(cleanup, body, vm);
}

VM_RET c_class(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_class(blist, vm);
}

VM_RET c_the(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_the(blist, vm);
}

VM_RET c_assure(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_assure(blist, vm);
}

VM_RET c_convert(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL blist=FETCH(function, pc).cell;
	return op_convert(blist, vm);
}

VM_RET c_with_standard_input(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	return op_with_standard_input(f, vm);
}

VM_RET c_with_standard_output(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL p=FETCH(function, pc).cell;
	return op_with_standard_output(p, vm);
}

VM_RET c_with_error_output(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL p=FETCH(function, pc).cell;
	return op_with_error_output(p, vm);
}

VM_RET c_with_open_input_file(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell,
		   f=FETCH(function, pc).cell;
	return op_with_open_input_file(f, plist, vm);
}

VM_RET c_with_open_output_file(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell,
		   f=FETCH(function, pc).cell;
	return op_with_open_output_file(f, plist, vm);
}

VM_RET c_with_open_io_file(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL plist=FETCH(function, pc).cell,
		   f=FETCH(function, pc).cell;
	return op_with_open_io_file(f, plist, vm);
}

VM_RET c_ignore_errors(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	return op_ignore_errors(f, vm);
}

VM_RET c_continue_condition(tPVM vm, tPCELL function, tINT* pc)
{
	return op_continue_condition(vm);
}

VM_RET c_with_handler(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	return op_with_handler(f, vm);
}

VM_RET c_time(tPVM vm, tPCELL function, tINT* pc)
{
	tPCELL f=FETCH(function, pc).cell;
	return op_time(f, vm);
}

VM_RET c_quasiquote(tPVM vm, tPCELL function, tINT* pc)
{
	return op_quasiquote(vm);
}

VM_RET c_quasiquote2(tPVM vm, tPCELL function, tINT* pc)
{
	return op_quasiquote2(vm);
}

VM_RET c_unquote(tPVM vm, tPCELL function, tINT* pc)
{
	return op_unquote(vm);
}

VM_RET c_unquote_splicing(tPVM vm, tPCELL function, tINT* pc)
{
	return op_unquote_splicing(vm);
}

VM_RET c_unquote_splicing2(tPVM vm, tPCELL function, tINT* pc)
{
	return op_unquote_splicing2(vm);
}

VM_RET c_functionp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_functionp(vm);
}

VM_RET c_apply(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_apply(anum, vm);
}

VM_RET c_funcall(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_funcall(anum, vm);
}

VM_RET c_eq(tPVM vm, tPCELL function, tINT* pc)
{
	return op_eq(vm);
}

VM_RET c_eql(tPVM vm, tPCELL function, tINT* pc)
{
	return op_eql(vm);
}

VM_RET c_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_equal(vm);
}

VM_RET c_not(tPVM vm, tPCELL function, tINT* pc)
{
	return op_not(vm);
}

VM_RET c_generic_function_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_generic_function_p(vm);
}

VM_RET c_class_of(tPVM vm, tPCELL function, tINT* pc)
{
	return op_class_of(vm);
}

VM_RET c_instancep(tPVM vm, tPCELL function, tINT* pc)
{
	return op_instancep(vm);
}

VM_RET c_subclassp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_subclassp(vm);
}

VM_RET c_symbolp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_symbolp(vm);
}

VM_RET c_property(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_property(anum, vm);
}

VM_RET c_remove_property(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_remove_property(anum, vm);
}

VM_RET c_gensym(tPVM vm, tPCELL function, tINT* pc)
{
	return op_gensym(vm);
}

VM_RET c_numberp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_numberp(vm);
}

VM_RET c_parse_number(tPVM vm, tPCELL function, tINT* pc)
{
	return op_parse_number(vm);
}

VM_RET c_number_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_number_equal(vm);
}

VM_RET c_number_not_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_number_not_equal(vm);
}

VM_RET c_number_ge(tPVM vm, tPCELL function, tINT* pc)
{
	return op_number_ge(vm);
}

VM_RET c_number_le(tPVM vm, tPCELL function, tINT* pc)
{
	return op_number_le(vm);
}

VM_RET c_number_greater(tPVM vm, tPCELL function, tINT* pc)
{
	return op_number_greater(vm);
}

VM_RET c_number_less(tPVM vm, tPCELL function, tINT* pc)
{
	return op_number_less(vm);
}

VM_RET c_addition(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_addition(anum, vm);
}

VM_RET c_multiplication(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_multiplication(anum, vm);
}

VM_RET c_substraction(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_substraction(anum, vm);
}

VM_RET c_quotient(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_quotient(anum, vm);
}

VM_RET c_reciprocal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_reciprocal(vm);
}

VM_RET c_max(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_max(anum, vm);
}

VM_RET c_min(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_min(anum, vm);
}

VM_RET c_abs(tPVM vm, tPCELL function, tINT* pc)
{
	return op_abs(vm);
}

VM_RET c_exp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_exp(vm);
}

VM_RET c_log(tPVM vm, tPCELL function, tINT* pc)
{
	return op_log(vm);
}

VM_RET c_expt(tPVM vm, tPCELL function, tINT* pc)
{
	return op_expt(vm);
}

VM_RET c_sqrt(tPVM vm, tPCELL function, tINT* pc)
{
	return op_sqrt(vm);
}

VM_RET c_sin(tPVM vm, tPCELL function, tINT* pc)
{
	return op_sin(vm);
}

VM_RET c_cos(tPVM vm, tPCELL function, tINT* pc)
{
	return op_cos(vm);
}

VM_RET c_tan(tPVM vm, tPCELL function, tINT* pc)
{
	return op_tan(vm);
}

VM_RET c_atan(tPVM vm, tPCELL function, tINT* pc)
{
	return op_atan(vm);
}

VM_RET c_atan2(tPVM vm, tPCELL function, tINT* pc)
{
	return op_atan2(vm);
}

VM_RET c_sinh(tPVM vm, tPCELL function, tINT* pc)
{
	return op_sinh(vm);
}

VM_RET c_cosh(tPVM vm, tPCELL function, tINT* pc)
{
	return op_cosh(vm);
}

VM_RET c_tanh(tPVM vm, tPCELL function, tINT* pc)
{
	return op_tanh(vm);
}

VM_RET c_atanh(tPVM vm, tPCELL function, tINT* pc)
{
	return op_atanh(vm);
}

VM_RET c_floatp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_floatp(vm);
}

VM_RET c_float(tPVM vm, tPCELL function, tINT* pc)
{
	return op_float(vm);
}

VM_RET c_floor(tPVM vm, tPCELL function, tINT* pc)
{
	return op_floor(vm);
}

VM_RET c_ceiling(tPVM vm, tPCELL function, tINT* pc)
{
	return op_ceiling(vm);
}

VM_RET c_truncate(tPVM vm, tPCELL function, tINT* pc)
{
	return op_truncate(vm);
}

VM_RET c_round(tPVM vm, tPCELL function, tINT* pc)
{
	return op_round(vm);
}

VM_RET c_integerp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_integerp(vm);
}

VM_RET c_div(tPVM vm, tPCELL function, tINT* pc)
{
	return op_div(vm);
}

VM_RET c_mod(tPVM vm, tPCELL function, tINT* pc)
{
	return op_mod(vm);
}

VM_RET c_gcd(tPVM vm, tPCELL function, tINT* pc)
{
	return op_gcd(vm);
}

VM_RET c_lcm(tPVM vm, tPCELL function, tINT* pc)
{
	return op_lcm(vm);
}

VM_RET c_isqrt(tPVM vm, tPCELL function, tINT* pc)
{
	return op_isqrt(vm);
}

VM_RET c_characterp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_characterp(vm);
}

VM_RET c_char_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_char_equal(vm);
}

VM_RET c_char_not_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_char_not_equal(vm);
}

VM_RET c_char_less(tPVM vm, tPCELL function, tINT* pc)
{
	return op_char_less(vm);
}

VM_RET c_char_greater(tPVM vm, tPCELL function, tINT* pc)
{
	return op_char_greater(vm);
}

VM_RET c_char_le(tPVM vm, tPCELL function, tINT* pc)
{
	return op_char_le(vm);
}

VM_RET c_char_ge(tPVM vm, tPCELL function, tINT* pc)
{
	return op_char_ge(vm);
}

VM_RET c_consp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_consp(vm);
}

VM_RET c_cons(tPVM vm, tPCELL function, tINT* pc)
{
	return op_cons(vm);
}

VM_RET c_car(tPVM vm, tPCELL function, tINT* pc)
{
	return op_car(vm);
}

VM_RET c_cdr(tPVM vm, tPCELL function, tINT* pc)
{
	return op_cdr(vm);
}

VM_RET c_null(tPVM vm, tPCELL function, tINT* pc)
{
	return op_null(vm);
}

VM_RET c_listp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_listp(vm);
}

VM_RET c_create_list(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_create_list(anum, vm);
}

VM_RET c_list(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_list(anum, vm);
}

VM_RET c_reverse(tPVM vm, tPCELL function, tINT* pc)
{
	return op_reverse(vm);
}

VM_RET c_nreverse(tPVM vm, tPCELL function, tINT* pc)
{
	return op_nreverse(vm);
}

VM_RET c_append(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_append(anum, vm);
}

VM_RET c_member(tPVM vm, tPCELL function, tINT* pc)
{
	return op_member(vm);
}

VM_RET c_mapcar(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_mapcar(anum, vm);
}

VM_RET c_mapc(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_mapc(anum, vm);
}

VM_RET c_mapcan(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_mapcan(anum, vm);
}

VM_RET c_maplist(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_maplist(anum, vm);
}

VM_RET c_mapl(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_mapl(anum, vm);
}

VM_RET c_mapcon(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_mapcon(anum, vm);
}

VM_RET c_assoc(tPVM vm, tPCELL function, tINT* pc)
{
	return op_assoc(vm);
}

VM_RET c_basic_array_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_basic_array_p(vm);
}

VM_RET c_basic_array_a_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_basic_array_a_p(vm);
}

VM_RET c_general_array_a_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_general_array_a_p(vm);
}

VM_RET c_create_array(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_create_array(anum, vm);
}

VM_RET c_aref(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_aref(anum, vm);
}

VM_RET c_garef(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_garef(anum, vm);
}

VM_RET c_array_dimensions(tPVM vm, tPCELL function, tINT* pc)
{
	return op_array_dimensions(vm);
}

VM_RET c_basic_vector_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_basic_vector_p(vm);
}

VM_RET c_general_vector_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_general_vector_p(vm);
}

VM_RET c_create_vector(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_create_vector(anum, vm);
}

VM_RET c_vector(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_vector(anum, vm);
}

VM_RET c_stringp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_stringp(vm);
}

VM_RET c_create_string(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_create_string(anum, vm);
}

VM_RET c_string_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_string_equal(vm);
}

VM_RET c_string_not_equal(tPVM vm, tPCELL function, tINT* pc)
{
	return op_string_not_equal(vm);
}

VM_RET c_string_less(tPVM vm, tPCELL function, tINT* pc)
{
	return op_string_less(vm);
}

VM_RET c_string_greater(tPVM vm, tPCELL function, tINT* pc)
{
	return op_string_greater(vm);
}

VM_RET c_string_ge(tPVM vm, tPCELL function, tINT* pc)
{
	return op_string_ge(vm);
}

VM_RET c_string_le(tPVM vm, tPCELL function, tINT* pc)
{
	return op_string_le(vm);
}

VM_RET c_char_index(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_char_index(anum, vm);
}

VM_RET c_string_index(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_string_index(anum, vm);
}

VM_RET c_string_append(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_string_append(anum, vm);
}

VM_RET c_length(tPVM vm, tPCELL function, tINT* pc)
{
	return op_length(vm);
}

VM_RET c_elt(tPVM vm, tPCELL function, tINT* pc)
{
	return op_elt(vm);
}

VM_RET c_subseq(tPVM vm, tPCELL function, tINT* pc)
{
	return op_subseq(vm);
}

VM_RET c_map_into(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_map_into(anum, vm);
}

VM_RET c_streamp(tPVM vm, tPCELL function, tINT* pc)
{
	return op_streamp(vm);
}

VM_RET c_open_stream_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_open_stream_p(vm);
}

VM_RET c_input_stream_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_input_stream_p(vm);
}

VM_RET c_output_stream_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_output_stream_p(vm);
}

VM_RET c_standard_input(tPVM vm, tPCELL function, tINT* pc)
{
	return op_standard_input(vm);
}

VM_RET c_standard_output(tPVM vm, tPCELL function, tINT* pc)
{
	return op_standard_output(vm);
}

VM_RET c_error_output(tPVM vm, tPCELL function, tINT* pc)
{
	return op_error_output(vm);
}

VM_RET c_open_input_file(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_open_input_file(anum, vm);
}

VM_RET c_open_output_file(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_open_output_file(anum, vm);
}

VM_RET c_open_io_file(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_open_io_file(anum, vm);
}

VM_RET c_close(tPVM vm, tPCELL function, tINT* pc)
{
	return op_close(vm);
}

VM_RET c_finish_output(tPVM vm, tPCELL function, tINT* pc)
{
	return op_finish_output(vm);
}

VM_RET c_create_string_input_stream(tPVM vm, tPCELL function, tINT* pc)
{
	return op_create_string_input_stream(vm);
}

VM_RET c_create_string_output_stream(tPVM vm, tPCELL function, tINT* pc)
{
	return op_create_string_output_stream(vm);
}

VM_RET c_get_output_stream_string(tPVM vm, tPCELL function, tINT* pc)
{
	return op_get_output_stream_string(vm);
}

VM_RET c_read(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_read(anum, vm);
}

VM_RET c_read_char(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_read_char(anum, vm);
}

VM_RET c_preview_char(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_preview_char(anum, vm);
}

VM_RET c_read_line(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_read_line(anum, vm);
}

VM_RET c_stream_ready_p(tPVM vm, tPCELL function, tINT* pc)
{
	return op_stream_ready_p(vm);
}

VM_RET c_format(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_format(anum, vm);
}

VM_RET c_format_char(tPVM vm, tPCELL function, tINT* pc)
{
	return op_format_char(vm);
}

VM_RET c_format_float(tPVM vm, tPCELL function, tINT* pc)
{
	return op_format_float(vm);
}

VM_RET c_format_fresh_line(tPVM vm, tPCELL function, tINT* pc)
{
	return op_format_fresh_line(vm);
}

VM_RET c_format_integer(tPVM vm, tPCELL function, tINT* pc)
{
	return op_format_integer(vm);
}

VM_RET c_format_object(tPVM vm, tPCELL function, tINT* pc)
{
	return op_format_object(vm);
}

VM_RET c_format_tab(tPVM vm, tPCELL function, tINT* pc)
{
	return op_format_tab(vm);
}

VM_RET c_read_byte(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_read_byte(anum, vm);
}

VM_RET c_write_byte(tPVM vm, tPCELL function, tINT* pc)
{
	return op_write_byte(vm);
}

VM_RET c_probe_file(tPVM vm, tPCELL function, tINT* pc)
{
	return op_probe_file(vm);
}

VM_RET c_file_position(tPVM vm, tPCELL function, tINT* pc)
{
	return op_file_position(vm);
}

VM_RET c_set_file_position(tPVM vm, tPCELL function, tINT* pc)
{
	return op_set_file_position(vm);
}

VM_RET c_file_length(tPVM vm, tPCELL function, tINT* pc)
{
	return op_file_length(vm);
}

VM_RET c_error(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_error(anum, vm);
}

VM_RET c_cerror(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_cerror(anum, vm);
}

VM_RET c_signal_condition(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_signal_condition(anum, vm);
}

VM_RET c_condition_continuable(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_condition_continuable(anum, vm);
}

VM_RET c_arithmetic_error_operation(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_arithmetic_error_operation(anum, vm);
}

VM_RET c_arithmetic_error_operand(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_arithmetic_error_operand(anum, vm);
}

VM_RET c_domain_error_object(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_domain_error_object(anum, vm);
}

VM_RET c_domain_error_expected_class(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_domain_error_expected_class(anum, vm);
}

VM_RET c_parse_error_string(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_parse_error_string(anum, vm);
}

VM_RET c_parse_error_expected_class(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_parse_error_expected_class(anum, vm);
}

VM_RET c_simple_error_format_string(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_simple_error_format_string(anum, vm);
}

VM_RET c_simple_error_format_arguments(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_simple_error_format_arguments(anum, vm);
}

VM_RET c_stream_error_stream(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_stream_error_stream(anum, vm);
}

VM_RET c_undefined_entity_name(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_undefined_entity_name(anum, vm);
}

VM_RET c_undefined_entity_namespace(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_undefined_entity_namespace(anum, vm);
}

VM_RET c_identity(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_identity(anum, vm);
}

VM_RET c_get_universal_time(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_get_universal_time(anum, vm);
}

VM_RET c_get_internal_run_time(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_get_internal_run_time(anum, vm);
}

VM_RET c_get_internal_real_time(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_get_internal_real_time(anum, vm);
}

VM_RET c_get_internal_time_units_per_second(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_get_internal_time_units_per_second(anum, vm);
}

VM_RET c_system(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_system(anum, vm);
}

VM_RET c_exit(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_exit(anum, vm);
}

VM_RET c_strftime(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_strftime(anum, vm);
}

VM_RET c_get_argument(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_get_argument(anum, vm);
}

VM_RET c_get_environment(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_get_environment(anum, vm);
}

VM_RET c_arity_error(tPVM vm, tPCELL function, tINT* pc)
{
	return op_arity_error(vm);
}


VM_RET c_eval(tPVM vm, tPCELL function, tINT* pc)
{
	tINT anum=FETCH(function, pc).i;
	return op_eval(anum, vm);
}

VM_RET c_number_equal_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_number_equal_stack_integer(i, offset, vm);
}

VM_RET c_number_equal_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_number_equal_stack_stack(offset2, offset1, vm);
}

VM_RET c_number_not_equal_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_number_not_equal_stack_integer(i, offset, vm);
}

VM_RET c_number_not_equal_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_number_not_equal_stack_stack(offset2, offset1, vm);
}

VM_RET c_number_less_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_number_less_stack_integer(i, offset, vm);
}

VM_RET c_number_less_integer_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i=FETCH(function, pc).i,
		 offset=FETCH(function, pc).i;
	return op_number_less_integer_stack(offset, i, vm);
}

VM_RET c_number_less_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_number_less_stack_stack(offset2, offset1, vm);
}

VM_RET c_number_le_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_number_le_stack_integer(i, offset, vm);
}

VM_RET c_number_le_integer_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i=FETCH(function, pc).i,
		 offset=FETCH(function, pc).i;
	return op_number_le_integer_stack(offset, i, vm);
}

VM_RET c_number_le_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_number_le_stack_stack(offset2, offset1, vm);
}

VM_RET c_addition_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_addition_stack_integer(i, offset, vm);
}

VM_RET c_addition_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_addition_stack_stack(offset2, offset1, vm);
}

VM_RET c_substraction_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_substraction_stack_integer(i, offset, vm);
}

VM_RET c_substraction_integer_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT i=FETCH(function, pc).i,
		 offset=FETCH(function, pc).i;
	return op_substraction_integer_stack(offset, i, vm);
}

VM_RET c_substraction_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_substraction_stack_stack(offset2, offset1, vm);
}

VM_RET c_eq_stack_integer(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset=FETCH(function, pc).i,
		 i=FETCH(function, pc).i;
	return op_eq_stack_integer(i, offset, vm);
}

VM_RET c_eq_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_eq_stack_stack(offset2, offset1, vm);
}

VM_RET c_equal_stack_stack(tPVM vm, tPCELL function, tINT* pc)
{
	tINT offset1=FETCH(function, pc).i,
		 offset2=FETCH(function, pc).i;
	return op_equal_stack_stack(offset2, offset1, vm);
}

