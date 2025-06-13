//
// TISL/src/tisl/object_3.c
// TISL Ver. 4.x
//

#if defined(TISL_DYNAMIC)
#if defined(TISL_WIN)

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#define load_library(lib_name)		LoadLibrary(lib_name)
#define get_procedure(lib, name)	GetProcAddress((HINSTANCE)lib, name)
#define free_library(lib)			FreeLibrary((HINSTANCE)lib)

#elif defined(TISL_DLFCN)

#include <dlfcn.h>
#define load_library(lib_name)		dlopen(lib_name, RTLD_LAZY)
#define get_procedure(lib, name)	dlsym(lib, name)
#define free_library(lib)			dlclose(lib)

#endif
#endif

#include <memory.h>

#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "writer.h"
#include "gc.h"
#include "translator.h"
#include "built_in_object.h"

/////////////////////////////////////////////////
// CELL_GENERIC_FUNCTION

#define GFUNCTION_EFFECTIVE_METHOD_TABLE_SIZE				8

#define GFUNCTION_REST										0x00000001
#define GFUNCTION_REST_										0xfffffffe
#define GFUNCTION_STANDARD									0x00000002
#define GFUNCTION_STANDARD_									0xfffffffd

#define GFUNCTION_GET_REST(gfunction)						((gfunction)->ui&GFUNCTION_REST)
#define GFUNCTION_SET_REST(gfunction)						((gfunction)->ui|=GFUNCTION_REST)
#define GFUNCTION_RESET_REST(gfunction)						((gfunction)->ui&=GFUNCTION_REST_)
#define GFUNCTION_GET_STANDARD(gfunction)					((gfunction)->ui&GFUNCTION_STANDARD)
#define GFUNCTION_SET_STANDARD(gfunction)					((gfunction)->ui|=GFUNCTION_STANDARD)
#define GFUNCTION_RESET_STANDARD(gfunction)					((gfunction)->ui&=GFUNCTION_STANDARD_)

#define GFUNCTION_GET_SIZE(gfunction)						(((gfunction)+1)->ui)
#define GFUNCTION_SET_SIZE(gfunction, size)					(((gfunction)+1)->ui=(size))
#define GFUNCTION_GET_PARAMETER_NUMBER(gfunction)			(((gfunction)+2)->i)
#define GFUNCTION_SET_PARAMETER_NUMBER(gfunction, n)		(((gfunction)+2)->i=(n))
#define GFUNCTION_GET_LAMBDA_LIST(gfunction)				(((gfunction)+3)->cell)
#define GFUNCTION_SET_LAMBDA_LIST(gfunction, list)			(((gfunction)+3)->cell=(list))

#define GFUNCTION_GET_AROUND_METHOD_LIST(gfunction)			(((gfunction)+4)->cell)
#define GFUNCTION_SET_AROUND_METHOD_LIST(gfunction, list)	(((gfunction)+4)->cell=(list))
#define GFUNCTION_GET_BEFORE_METHOD_LIST(gfunction)			(((gfunction)+5)->cell)
#define GFUNCTION_SET_BEFORE_METHOD_LIST(gfunction, list)	(((gfunction)+5)->cell=(list))
#define GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction)		(((gfunction)+6)->cell)
#define GFUNCTION_SET_PRIMARY_METHOD_LIST(gfunction, list)	(((gfunction)+6)->cell=(list))
#define GFUNCTION_GET_AFTER_METHOD_LIST(gfunction)			(((gfunction)+7)->cell)
#define GFUNCTION_SET_AFTER_METHOD_LIST(gfunction, list)	(((gfunction)+7)->cell=(list))

#define GFUNCTION_GET_EFFECTIVE_METHOD_TABLE(gfunction)		((tPCELL*)((gfunction)+8))

static tBOOL gfunction_check_agreement(tPVM vm, tPCELL list, tPCELL method);
static VM_RET gfunction_call_standard(tPVM vm, tPCELL gfunction, tPOBJECT ret);
static VM_RET gfunction_call_nil(tPVM vm, tPCELL gfunction, tPOBJECT ret);
static VM_RET gfunction_get_effective_method(tPVM vm, tPCELL gfunction, tPCELL *emethod);
static tBOOL gfunction_search_emethod(tPVM vm, tPCELL gfunction, tPCELL *emethod);
static tINT gfunction_get_emethod_key(tPVM vm, tPCELL gfunction);
static tINT gfunction_get_emethod_key_(tPCELL gfunction, tPCELL emethod);

tUINT generic_function_get_size(tPCELL gfunction)
{
	return GFUNCTION_GET_SIZE(gfunction);
}

VM_RET generic_function_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT lambda;
	tPCELL gfunction;
	gfunction=OBJECT_GET_CELL(obj);
	if (write_string(vm, stream, "#i(<standard-generic-function> ")) return VM_ERROR;
	OBJECT_SET_CONS(&lambda, GFUNCTION_GET_LAMBDA_LIST(gfunction));
	if (write_object(vm, stream, &lambda)) return VM_ERROR;
	if (write_string(vm, stream, " method-combination ")) return VM_ERROR;
	if (GFUNCTION_GET_STANDARD(gfunction)) {
		tPCELL p;
		tOBJECT method;
		if (write_string(vm, stream, "standard")) return VM_ERROR;
		if (GFUNCTION_GET_AROUND_METHOD_LIST(gfunction)) {
			if (write_string(vm, stream, " around ")) return VM_ERROR;
			for (p=GFUNCTION_GET_AROUND_METHOD_LIST(gfunction); p; p=cons_get_cdr_cons(p)) {
				cons_get_car(p, &method);
				if (write_object(vm, stream, &method)) return VM_ERROR;
			}
		}
		if (GFUNCTION_GET_BEFORE_METHOD_LIST(gfunction)) {
			if (write_string(vm, stream, " before ")) return VM_ERROR;
			for (p=GFUNCTION_GET_BEFORE_METHOD_LIST(gfunction); p; p=cons_get_cdr_cons(p)) {
				cons_get_car(p, &method);
				if (write_object(vm, stream, &method)) return VM_ERROR;
			}
		}
		if (GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction)) {
			if (write_string(vm, stream, " primary ")) return VM_ERROR;
			for (p=GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction); p; p=cons_get_cdr_cons(p)) {
				cons_get_car(p, &method);
				if (write_object(vm, stream, &method)) return VM_ERROR;
			}
		}
		if (GFUNCTION_GET_AFTER_METHOD_LIST(gfunction)) {
			if (write_string(vm, stream, " after ")) return VM_ERROR;
			for (p=GFUNCTION_GET_AFTER_METHOD_LIST(gfunction); p; p=cons_get_cdr_cons(p)) {
				cons_get_car(p, &method);
				if (write_object(vm, stream, &method)) return VM_ERROR;
			}
		}
	} else {
		tPCELL p;
		tOBJECT method;
		if (write_string(vm, stream, "nil")) return VM_ERROR;
		if (GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction)) {
			if (write_string(vm, stream, " ")) return VM_ERROR;
			for (p=GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction); p; p=cons_get_cdr_cons(p)) {
				cons_get_car(p, &method);
				if (write_object(vm, stream, &method)) return VM_ERROR;
			}
		}
	}
	return write_string(vm, stream, ")");
}

VM_RET generic_function_mark(tPVM vm, tPCELL cell)
{
	tINT i;
	if (cell_mark(vm, GFUNCTION_GET_LAMBDA_LIST(cell))) return VM_ERROR;
	if (cell_mark(vm, GFUNCTION_GET_AROUND_METHOD_LIST(cell))) return VM_ERROR;
	if (cell_mark(vm, GFUNCTION_GET_BEFORE_METHOD_LIST(cell))) return VM_ERROR;
	if (cell_mark(vm, GFUNCTION_GET_PRIMARY_METHOD_LIST(cell))) return VM_ERROR;
	if (cell_mark(vm, GFUNCTION_GET_AFTER_METHOD_LIST(cell))) return VM_ERROR;
	for (i=0; i<GFUNCTION_EFFECTIVE_METHOD_TABLE_SIZE; i++) {
		if (cell_mark(vm, GFUNCTION_GET_EFFECTIVE_METHOD_TABLE(cell)[i])) return VM_ERROR;
	}
	return VM_OK;
}

VM_RET gfunction_create_(tPVM vm, const tINT pnum, const tBOOL rest, const tBOOL standard, tPCELL lambda_list, tPCELL* cell)
{
	tUINT size;

	size=allocate_cell(vm, sizeof(tCELL)*8+sizeof(tPCELL)*GFUNCTION_EFFECTIVE_METHOD_TABLE_SIZE, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_GENERIC_FUNCTION);
	GFUNCTION_SET_SIZE(*cell, size);

	GFUNCTION_SET_PARAMETER_NUMBER(*cell, pnum);
	if (rest) GFUNCTION_SET_REST(*cell);
	else GFUNCTION_RESET_REST(*cell);
	if (standard) GFUNCTION_SET_STANDARD(*cell);
	else GFUNCTION_RESET_STANDARD(*cell);

	GFUNCTION_SET_LAMBDA_LIST(*cell, lambda_list);

	GFUNCTION_SET_AROUND_METHOD_LIST(*cell, 0);
	GFUNCTION_SET_BEFORE_METHOD_LIST(*cell, 0);
	GFUNCTION_SET_PRIMARY_METHOD_LIST(*cell, 0);
	GFUNCTION_SET_AFTER_METHOD_LIST(*cell, 0);

	gfunction_clear_emethod(*cell);

	return VM_OK;
}

void gfunction_clear_emethod(tPCELL gfunction)
{
	memset(GFUNCTION_GET_EFFECTIVE_METHOD_TABLE(gfunction), 0, sizeof(tPCELL)*GFUNCTION_EFFECTIVE_METHOD_TABLE_SIZE);
}

VM_RET gfunction_add_method(tPVM vm, tPCELL gfunction, tPCELL method)
{
	tOBJECT rest, obj;
	tPCELL list;
	OBJECT_SET_METHOD(&obj, method);
	if (vm_push(vm, &obj)) return VM_ERROR;
	// 引数の合同の検査
	if ((GFUNCTION_GET_PARAMETER_NUMBER(gfunction)!=method_get_parameter_number(method))||
		(GFUNCTION_GET_REST(gfunction)&&!method_is_rest(method))||
		(!GFUNCTION_GET_REST(gfunction)&&method_is_rest(method))) {
		vm_pop(vm);
		return signal_condition(vm, TISL_ERROR_NOT_CONGRUENT_LAMBDA_LIST);
	}
	// それぞれの修飾子の中で合致の検査
	switch (method_get_qualifier(method)) {
	case METHOD_AROUND:		list=GFUNCTION_GET_AROUND_METHOD_LIST(gfunction); break;
	case METHOD_BEFORE:		list=GFUNCTION_GET_BEFORE_METHOD_LIST(gfunction); break;
	case METHOD_PRIMARY:	list=GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction); break;
	case METHOD_AFTER:		list=GFUNCTION_GET_AFTER_METHOD_LIST(gfunction); break;
	default:				vm_pop(vm); return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	if (list) {
		if (gfunction_check_agreement(vm, list, method)) {
			// 一致するメソッドが存在した 追加は行わない
			// 古いメソッドの関数部分を入れ替える．
			vm_pop(vm);
			return VM_OK;
		}
		OBJECT_SET_CONS(&rest, list);
	} else {
		OBJECT_SET_NIL(&rest);
	}
	cell_to_object(method, &obj);
	if (cons_create(vm, &list, &obj, &rest)) { vm_pop(vm); return VM_ERROR; }
	switch (method_get_qualifier(method)) {
	case METHOD_AROUND:		GFUNCTION_SET_AROUND_METHOD_LIST(gfunction, list); break;
	case METHOD_BEFORE:		GFUNCTION_SET_BEFORE_METHOD_LIST(gfunction, list); break;
	case METHOD_PRIMARY:	GFUNCTION_SET_PRIMARY_METHOD_LIST(gfunction, list); break;
	case METHOD_AFTER:		GFUNCTION_SET_AFTER_METHOD_LIST(gfunction, list); break;
	}
	vm_pop(vm);
	return VM_OK;
}

static tBOOL gfunction_check_agreement(tPVM vm, tPCELL list, tPCELL method)
{
	tPCELL p;
	tOBJECT tmp;

	for (p=list; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		if (method_agreement_on_parameter_specializer(vm, OBJECT_GET_CELL(&tmp), method)) return tTRUE;
	}
	return tFALSE;
}

tINT gfunction_get_parameter_number(tPCELL gfunction)
{
	return GFUNCTION_GET_PARAMETER_NUMBER(gfunction);
}

tBOOL gfunction_is_rest(tPCELL gfunction)
{
	return GFUNCTION_GET_REST(gfunction) ? tTRUE : tFALSE;
}

VM_RET gfunction_call(tPVM vm, tPCELL gfunction, tPOBJECT value)
{
	if (GFUNCTION_GET_STANDARD(gfunction)) {
		return gfunction_call_standard(vm, gfunction, value);
	} else {
		return gfunction_call_nil(vm, gfunction, value);
	}
}

static VM_RET gfunction_call_standard(tPVM vm, tPCELL gfunction, tPOBJECT ret)
{
	tPCELL emethod;
	// 実効メソッドの取得
	if (gfunction_get_effective_method(vm, gfunction, &emethod)) return VM_ERROR;
	// 適用可能な主メソッドが存在しなかった
	if (!emethod_get_primary_method_list(emethod)) return signal_condition(vm, TISL_ERROR_NO_APPLICABLE_METHOD);
	// aroundメソッドの存在しない場合
	if (!emethod_get_around_method_list(emethod)) return emethod_call_no_around(vm, emethod, ret);
	// 最も優先度の高いaround メソッドから呼出す
	return emethod_call_around(vm, emethod, ret);
}

static VM_RET gfunction_call_nil(tPVM vm, tPCELL gfunction, tPOBJECT ret)
{
	tPCELL emethod;
	// 実効メソッドの取得
	if (gfunction_get_effective_method(vm, gfunction, &emethod)) return VM_ERROR;
	// 適用可能なメソッドが存在しない
	if (emethod_get_primary_method_list(emethod)) return signal_condition(vm, TISL_ERROR_NO_APPLICABLE_METHOD);
	// 主メソッドを呼出す
	return emethod_call_primary(vm, emethod, ret);
}

static VM_RET gfunction_get_effective_method(tPVM vm, tPCELL gfunction, tPCELL *emethod)
{
	// 既に登録されているものから検索
	if (gfunction_search_emethod(vm, gfunction, emethod)) return VM_OK;
	// 新規に作成する
	return effective_method_create_(vm, gfunction, emethod);
}

static tBOOL gfunction_search_emethod(tPVM vm, tPCELL gfunction, tPCELL *emethod)
{
	tPCELL p;
	tINT key;

	key=gfunction_get_emethod_key(vm, gfunction);
	for (p=GFUNCTION_GET_EFFECTIVE_METHOD_TABLE(gfunction)[key]; p; p=emethod_get_next(p)) {
		if (emethod_is_applicable(p, vm)) { *emethod=p; return tTRUE; }
	}
	return tFALSE;
}

static tINT gfunction_get_emethod_key(tPVM vm, tPCELL gfunction)
{
	tINT key, i, n=GFUNCTION_GET_PARAMETER_NUMBER(gfunction);
	tOBJECT clss;
	key=0;
	for (i=n-1; i>=0; i--) {
		object_get_class(vm->SP-i, &clss);
		key<<=1;
		key+=(tINT)clss.data.p;
	}
	key=(key/23)%GFUNCTION_EFFECTIVE_METHOD_TABLE_SIZE;
	return (key<0) ? -key : key;
}

tPCELL gfunction_get_around_method_list(tPCELL gfunction)
{
	return GFUNCTION_GET_AROUND_METHOD_LIST(gfunction);
}

tPCELL gfunction_get_before_method_list(tPCELL gfunction)
{
	return GFUNCTION_GET_BEFORE_METHOD_LIST(gfunction);
}

tPCELL gfunction_get_primary_method_list(tPCELL gfunction)
{
	return GFUNCTION_GET_PRIMARY_METHOD_LIST(gfunction);
}

tPCELL gfunction_get_after_method_list(tPCELL gfunction)
{
	return GFUNCTION_GET_AFTER_METHOD_LIST(gfunction);
}

VM_RET gfunction_add_emethod(tPVM vm, tPCELL gfunction, tPCELL emethod)
{
	tINT key=gfunction_get_emethod_key_(gfunction, emethod);
	emethod_set_next(emethod, GFUNCTION_GET_EFFECTIVE_METHOD_TABLE(gfunction)[key]);
	GFUNCTION_GET_EFFECTIVE_METHOD_TABLE(gfunction)[key]=emethod;
	return VM_OK;
}

/////////////////////////////////////////////////

static VM_RET gfunction_create_accessor(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass, const tINT kind);
static VM_RET gfunction_accessor_get_gfunction(tPVM vm, tPCELL name, const tINT kind, tPCELL* gfunction);

VM_RET gfunction_create_reader(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass)
{
	return gfunction_create_accessor(vm, name, slot_name, sclass, SYSTEM_METHOD_READER);
}

VM_RET gfunction_create_writer(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass)
{
	return gfunction_create_accessor(vm, name, slot_name, sclass, SYSTEM_METHOD_WRITER);
}

VM_RET gfunction_create_boundp(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass)
{
	return gfunction_create_accessor(vm, name, slot_name, sclass, SYSTEM_METHOD_BOUNDP);
}

static VM_RET gfunction_create_accessor(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass, const tINT kind)
{
	tPCELL gfunction, method;
	tOBJECT tmp;
	// 包括関数の取得または生成
	if (gfunction_accessor_get_gfunction(vm, name, kind, &gfunction)) return VM_ERROR;
	// メソッドの作成
	if (method_create_system(vm, slot_name, sclass, kind, &method)) return VM_ERROR;
	// メソッドの登録
	cell_to_object(method, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	if (gfunction_add_method(vm, gfunction, method)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

static VM_RET gfunction_accessor_get_gfunction(tPVM vm, tPCELL name, const tINT kind, tPCELL* gfunction)
{
	tPCELL string, bind;
	tOBJECT tmp;
	tINT n;
	if (symbol_is_built_in_function(vm, name)) return VM_ERROR;
	if (!symbol_get_string(name, 0, &string)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (package_add_bind(vm, vm_get_current_package(vm), string, &bind)) return VM_ERROR;
	bind_get_function(bind, &tmp);
	if (OBJECT_IS_GENERIC_FUNCTION(&tmp)) {
		// 既に包括関数によって束縛されている場合
		// 引数の形の検査をする
		*gfunction=OBJECT_GET_CELL(&tmp);
		if (gfunction_is_rest(*gfunction)) return signal_condition(vm, TISL_ERROR_NOT_CONGRUENT_LAMBDA_LIST);
		n=gfunction_get_parameter_number(*gfunction);
		switch (kind) {
		case SYSTEM_METHOD_READER:
		case SYSTEM_METHOD_BOUNDP:
			if (n!=1) return signal_condition(vm, TISL_ERROR_NOT_CONGRUENT_LAMBDA_LIST);
			break;
		case SYSTEM_METHOD_WRITER:
			if (n!=2) return signal_condition(vm, TISL_ERROR_NOT_CONGRUENT_LAMBDA_LIST);
			break;
		default:
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	} else {
		tPCELL lambda_list;
		// 包括関数を作成し，束縛を設定する. 
		switch (kind) {
		case SYSTEM_METHOD_READER:
		case SYSTEM_METHOD_BOUNDP:// (instance)
			n=1;
			lambda_list=cons_get_cdr_cons(list_object_instance);
			break;
		case SYSTEM_METHOD_WRITER:// (object instance)
			n=2;
			lambda_list=list_object_instance;
			break;
		default:
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
		if (gfunction_create_(vm, n, tFALSE, tTRUE, lambda_list, gfunction)) return VM_ERROR;
		cell_to_object(*gfunction, &tmp);
		bind_set_function(bind, &tmp);
	}
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_METHOD

#define METHOD_PARAMETER					0x000000ff
#define METHOD_PARAMETER_					0xffffff00
#define METHOD_QUALIFIER					0x00000f00
#define METHOD_QUALIFIER_					0xfffff0ff
#define METHOD_REST							0x00001000
#define METHOD_REST_						0xffffefff
#define METHOD_NEXT							0x00002000
#define METHOD_NEXT_						0xffffdfff
#define METHOD_STACK						0x00004000
#define METHOD_STACK_						0xffffbfff
#define METHOD_SYSTEM						0x00008000
#define METHOD_SYSTEM_						0xffff7fff

#define METHOD_GET_QUALIFIER(method)		(((method)->ui&METHOD_QUALIFIER)>>8)
#define METHOD_SET_QUALIFIER(method, id)	((method)->ui&=METHOD_QUALIFIER_, (method)->ui|=((id)<<8)&METHOD_QUALIFIER)
#define METHOD_GET_PARAMETER(method)		((method)->ui&METHOD_PARAMETER)
#define METHOD_SET_PARAMETER(method, pnum)	((method)->ui&=METHOD_PARAMETER_, (method)->ui|=(pnum)&METHOD_PARAMETER)
#define METHOD_GET_REST(method)				((method)->ui&METHOD_REST)
#define METHOD_SET_REST(method)				((method)->ui|=METHOD_REST)
#define METHOD_RESET_REST(method)			((method)->ui&=METHOD_REST_)
#define METHOD_GET_NEXT(method)				((method)->ui&METHOD_NEXT)
#define METHOD_SET_NEXT(method)				((method)->ui|=METHOD_NEXT)
#define METHOD_RESET_NEXT(method)			((method)->ui&=METHOD_NEXT_)
#define METHOD_GET_STACK(method)			((method)->ui&METHOD_STACK)
#define METHOD_SET_STACK(method)			((method)->ui|=METHOD_STACK)
#define METHOD_RESET_STACK(method)			((method)->ui&=METHOD_STACK_)
#define METHOD_GET_SYSTEM(method)			((method)->ui&METHOD_SYSTEM)
#define METHOD_SET_SYSTEM(method)			((method)->ui|=METHOD_SYSTEM)
#define METHOD_RESET_SYSTEM(method)			((method)->ui&=METHOD_SYSTEM_)

#define METHOD_GET_SIZE(method)				(((method)+1)->ui)
#define METHOD_SET_SIZE(method, size)		(((method)+1)->ui=(size))
#define METHOD_GET_FUNCTION(method)			(((method)+2)->cell)
#define METHOD_SET_FUNCTION(method, func)	(((method)+2)->cell=(func))
#define METHOD_GET_BODY(method)				((tPOBJECT)((method)+3))
#define METHOD_SET_BODY(method, form)		(*METHOD_GET_BODY(method)=*(form))
#define METHOD_GET_PARAMETER_NAME(method)	((tPCELL*)((method)+5))
#define METHOD_GET_SPECIALIZER(method)		((tPCELL*)(METHOD_GET_PARAMETER_NAME(method)+METHOD_GET_PARAMETER(method)))

#define METHOD_GET_SYSTEM_KIND(method)			(((method)+2)->i)
#define METHOD_SET_SYSTEM_KIND(method, kind)	(((method)+2)->i=(kind))
#define METHOD_GET_ACCESS_SLOT_NAME(method)		(((method)+3)->cell)
#define METHOD_SET_ACCESS_SLOT_NAME(method, p)	(((method)+3)->cell=(p))
#define METHOD_GET_ACCESS_CLASS(method)			(((method)+4)->cell)
#define METHOD_SET_ACCESS_CLASS(method, p)		(((method)+4)->cell=(p))

VM_RET method_create_(tPVM vm, tPCELL function, tPCELL pplist, const tBOOL next, const tINT qualifier, tPOBJECT form, tPCELL* cell)
{
	tUINT size;
	tINT i, n;
	tPCELL p1, p2;
	tOBJECT tmp;

	n=pplist_get_number(pplist);
	size=allocate_cell(vm, sizeof(tCELL)*6+(sizeof(tPCELL)+sizeof(tPCELL))*n, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_METHOD);
	METHOD_SET_SIZE(*cell, size);
	METHOD_RESET_SYSTEM(*cell);
	METHOD_SET_PARAMETER(*cell, pplist_get_number(pplist));
	if (pplist_is_rest(pplist)) METHOD_SET_REST(*cell);
	else METHOD_RESET_REST(*cell);
	if (next) METHOD_SET_NEXT(*cell);
	else METHOD_RESET_NEXT(*cell);
	METHOD_SET_QUALIFIER(*cell, qualifier);
	METHOD_SET_FUNCTION(*cell, function);
	METHOD_SET_BODY(*cell, form);
	p1=pplist_get_name_list(pplist);
	p2=pplist_get_profiler_list(pplist);
	for (i=0; i<n; i++, p1=cons_get_cdr_cons(p1), p2=cons_get_cdr_cons(p2)) {
		cons_get_car(p1, &tmp);
		METHOD_GET_PARAMETER_NAME(*cell)[i]=OBJECT_GET_CELL(&tmp);
		cons_get_car(p2, &tmp);
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&tmp), METHOD_GET_SPECIALIZER(*cell)+i)) return VM_ERROR;
	}
	if (parameter_list_is_stack(pplist_get_plist(pplist)))
		METHOD_SET_STACK(*cell);
	else
		METHOD_RESET_STACK(*cell);
	return VM_OK;
}

VM_RET method_create_system(tPVM vm, tPCELL slot_name, tPCELL sclass, const tINT kind, tPCELL* cell)
{
	tINT n;
	tUINT size;
	tBOOL rest;
	switch (kind) {
	case SYSTEM_METHOD_READER:
	case SYSTEM_METHOD_BOUNDP:
		n=1;
		rest=tFALSE;
		break;
	case SYSTEM_METHOD_WRITER:
	case SYSTEM_METHOD_INITIALIZE_OBJECT:
	case SYSTEM_METHOD_REPORT_CONDITION:
		n=2;
		rest=tFALSE;
		break;
	case SYSTEM_METHOD_CREATE:
		n=2;
		rest=tTRUE;
		break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	size=allocate_cell(vm, sizeof(tCELL)*6+(sizeof(tPCELL)+sizeof(tOBJECT))*n, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_METHOD);
	METHOD_SET_SYSTEM(*cell);
	METHOD_SET_SIZE(*cell, size);
	METHOD_SET_PARAMETER(*cell, n);
	if (rest) METHOD_SET_REST(*cell);
	else METHOD_RESET_REST(*cell);
	METHOD_RESET_NEXT(*cell);
	METHOD_SET_QUALIFIER(*cell, METHOD_PRIMARY);
	METHOD_SET_STACK(*cell);
	METHOD_SET_SYSTEM_KIND(*cell, kind);
	METHOD_SET_ACCESS_SLOT_NAME(*cell, slot_name);
	METHOD_SET_ACCESS_CLASS(*cell, sclass);
	switch (kind) {
	case SYSTEM_METHOD_READER:
	case SYSTEM_METHOD_BOUNDP:
		METHOD_GET_PARAMETER_NAME(*cell)[0]=SYMBOL_INSTANCE;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), standard_class_get_name(sclass), METHOD_GET_SPECIALIZER(*cell))) return VM_ERROR;
		break;
	case SYSTEM_METHOD_WRITER:
		METHOD_GET_PARAMETER_NAME(*cell)[0]=SYMBOL_OBJECT;
		METHOD_GET_PARAMETER_NAME(*cell)[1]=SYMBOL_INSTANCE;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_OBJECT_CLASS, METHOD_GET_SPECIALIZER(*cell))) return VM_ERROR;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), standard_class_get_name(sclass), METHOD_GET_SPECIALIZER(*cell)+1)) return VM_ERROR;
		break;
	case SYSTEM_METHOD_CREATE:
		METHOD_GET_PARAMETER_NAME(*cell)[0]=SYMBOL_CLASS;
		METHOD_GET_PARAMETER_NAME(*cell)[1]=SYMBOL_LIST;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_STANDARD_CLASS_CLASS, METHOD_GET_SPECIALIZER(*cell))) return VM_ERROR;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_LIST_CLASS, METHOD_GET_SPECIALIZER(*cell)+1)) return VM_ERROR;
		break;
	case SYSTEM_METHOD_INITIALIZE_OBJECT:
		METHOD_GET_PARAMETER_NAME(*cell)[0]=SYMBOL_INSTANCE;
		METHOD_GET_PARAMETER_NAME(*cell)[1]=SYMBOL_LIST;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_STANDARD_OBJECT_CLASS, METHOD_GET_SPECIALIZER(*cell))) return VM_ERROR;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_LIST_CLASS, METHOD_GET_SPECIALIZER(*cell)+1)) return VM_ERROR;
		break;
	case SYSTEM_METHOD_REPORT_CONDITION:
		METHOD_GET_PARAMETER_NAME(*cell)[0]=SYMBOL_CONDITION;
		METHOD_GET_PARAMETER_NAME(*cell)[1]=SYMBOL_STREAM;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_SERIOUS_CONDITION_CLASS, METHOD_GET_SPECIALIZER(*cell))) return VM_ERROR;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_islisp_package(vm), SYMBOL_STREAM_CLASS, METHOD_GET_SPECIALIZER(*cell)+1)) return VM_ERROR;
	}
	return VM_OK;
}

tUINT method_get_size(tPCELL method)
{
	return METHOD_GET_SIZE(method);
}

VM_RET method_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT tmp;
	tINT i, n;
	tPCELL method=OBJECT_GET_CELL(obj);
	if (write_string(vm, stream, "((")) return VM_ERROR;
	// pplist
	n=METHOD_GET_PARAMETER(method);
	for (i=0; i<n; i++) {
		if (method_is_rest(method)&&(i==n-1)) {
			// :rest
			OBJECT_SET_SYMBOL(&tmp, KEYWORD_REST);
			if (write_object(vm, stream, &tmp)) return VM_ERROR;
			if (write_string(vm, stream, " ")) return VM_ERROR;
			OBJECT_SET_SYMBOL(&tmp, METHOD_GET_PARAMETER_NAME(method)[i]);
			if (write_object(vm, stream, &tmp)) return VM_ERROR;
		} else {
			if (write_string(vm, stream, "(")) return VM_ERROR;
			OBJECT_SET_SYMBOL(&tmp, METHOD_GET_PARAMETER_NAME(method)[i]);
			if (write_object(vm, stream, &tmp)) return VM_ERROR;
			if (write_string(vm, stream, " ")) return VM_ERROR;
			OBJECT_SET_SYMBOL(&tmp, bind_list_get_name(METHOD_GET_SPECIALIZER(method)[i]));
			if (write_object(vm, stream, &tmp)) return VM_ERROR;
			if (write_string(vm, stream, ")")) return VM_ERROR;
			if ((i!=n-1)&&write_string(vm, stream, " ")) return VM_ERROR;
		}
	}
	if (write_string(vm, stream, ")")) return VM_ERROR;
	if (METHOD_GET_SYSTEM(method)) {
		switch (METHOD_GET_SYSTEM_KIND(method)) {
		case SYSTEM_METHOD_READER:
			if (write_string(vm, stream, " #tisl(:reader)")) return VM_ERROR;
			break;
		case SYSTEM_METHOD_WRITER:
			if (write_string(vm, stream, " #tisl(:writer)")) return VM_ERROR;
			break;
		case SYSTEM_METHOD_BOUNDP:
			if (write_string(vm, stream, " #tisl(:boundp)")) return VM_ERROR;
			break;
		case SYSTEM_METHOD_CREATE:
			if (write_string(vm, stream, " #tisl(create)")) return VM_ERROR;
			break;
		case SYSTEM_METHOD_INITIALIZE_OBJECT:
			if (write_string(vm, stream, " #tisl(initialize-object)")) return VM_ERROR;
			break;
		case SYSTEM_METHOD_REPORT_CONDITION:
			if (write_string(vm, stream, " #tisl(report-condition)")) return VM_ERROR;
			break;
		default:
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	} else {
		// body
		if (write_object(vm, stream, METHOD_GET_BODY(method))) return VM_ERROR;
	}
	return write_string(vm, stream, ")");
}

VM_RET method_mark(tPVM vm, tPCELL cell)
{
	if (!METHOD_GET_SYSTEM(cell)) {
		if (cell_mark(vm, METHOD_GET_FUNCTION(cell))) return VM_ERROR;
		if (gc_push(vm, METHOD_GET_BODY(cell))) return VM_ERROR;
	}
	// parameter specializerのマーキングはいらない？
	return VM_OK;
}

tINT method_get_qualifier(tPCELL method)
{
	return METHOD_GET_QUALIFIER(method);
}

tINT method_get_parameter_number(tPCELL method)
{
	return METHOD_GET_PARAMETER(method);
}

tBOOL method_is_rest(tPCELL method)
{
	return METHOD_GET_REST(method) ? tTRUE : tFALSE;
}

tBOOL method_is_next(tPCELL method)
{
	return METHOD_GET_NEXT(method) ? tTRUE : tFALSE;
}

tBOOL method_is_stack(tPCELL method)
{
	return METHOD_GET_STACK(method) ? tTRUE : tFALSE;
}

tBOOL method_agreement_on_parameter_specializer(tPVM vm, tPCELL old_method, tPCELL new_method)
{
	tINT i, n;
	if ((method_is_rest(old_method)&&!method_is_rest(new_method))||
		(!method_is_rest(old_method)&&method_is_rest(new_method))) return tFALSE;
	if (method_get_parameter_number(old_method)!=method_get_parameter_number(new_method)) return tFALSE;
	n=method_get_parameter_number(old_method);
	for (i=0; i<n; i++) {
		if (METHOD_GET_SPECIALIZER(old_method)[i]!=METHOD_GET_SPECIALIZER(new_method)[i]) return tFALSE;
	}
	//合致した？
	if (METHOD_GET_NEXT(new_method))
		METHOD_SET_NEXT(old_method);
	else
		METHOD_RESET_NEXT(old_method);
	if (METHOD_GET_STACK(new_method))
		METHOD_SET_STACK(old_method);
	else
		METHOD_RESET_STACK(old_method);
	if (METHOD_GET_SYSTEM(new_method)) {
		//
		METHOD_SET_SYSTEM(old_method);
		METHOD_SET_SYSTEM_KIND(old_method, METHOD_GET_SYSTEM_KIND(new_method));
		METHOD_SET_ACCESS_SLOT_NAME(old_method, METHOD_GET_ACCESS_SLOT_NAME(new_method));
		METHOD_SET_ACCESS_CLASS(old_method, METHOD_GET_ACCESS_CLASS(new_method));
	} else {
		// 関数部分を入れ替え
		METHOD_RESET_SYSTEM(old_method);
		METHOD_SET_FUNCTION(old_method, METHOD_GET_FUNCTION(new_method));
		METHOD_SET_BODY(old_method, METHOD_GET_BODY(new_method));
	}
	return tTRUE;
}

VM_RET method_call(tPVM vm, tPCELL method, tPCELL env, tPOBJECT ret)
{
	if (METHOD_GET_SYSTEM(method)) {
		switch (METHOD_GET_SYSTEM_KIND(method)) {
		case SYSTEM_METHOD_READER:
			return instance_get_slot(vm, OBJECT_GET_CELL(vm->SP), METHOD_GET_ACCESS_SLOT_NAME(method), METHOD_GET_ACCESS_CLASS(method), ret);
		case SYSTEM_METHOD_WRITER:
			if (instance_set_slot(vm, OBJECT_GET_CELL(vm->SP), METHOD_GET_ACCESS_SLOT_NAME(method), METHOD_GET_ACCESS_CLASS(method), vm->SP-1)) return VM_ERROR;
			*ret=*(vm->SP-1);
			return VM_OK;
		case SYSTEM_METHOD_BOUNDP:
			return instance_check_slot(vm, OBJECT_GET_CELL(vm->SP), METHOD_GET_ACCESS_SLOT_NAME(method), METHOD_GET_ACCESS_CLASS(method), ret);
		case SYSTEM_METHOD_CREATE:
			{
				tOBJECT tmp;
				tPCELL instance;
				if (instance_create_(vm, OBJECT_GET_CELL(vm->SP-1), &instance)) return VM_ERROR;
				OBJECT_SET_INSTANCE(&tmp, instance);
				if (vm_push(vm, &tmp)) return VM_ERROR;
				if (vm_push(vm, vm->SP-1)) return VM_ERROR;
				return gfunction_call(vm, gfunction_initialize_object, ret);
			}
		case SYSTEM_METHOD_INITIALIZE_OBJECT:
			if (instance_initialize(vm, OBJECT_GET_CELL(vm->SP-1), OBJECT_GET_CELL(vm->SP))) return VM_ERROR;
			*ret=*(vm->SP-1);
			return VM_OK;
		case SYSTEM_METHOD_REPORT_CONDITION:
			{
				tPCELL stream;
				if (OBJECT_IS_STRING_STREAM(vm->SP)||
					OBJECT_IS_FILE_STREAM(vm->SP))
					stream=OBJECT_GET_CELL(vm->SP);
				else
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, vm->SP);
				if (format_object(vm, stream, vm->SP-1)) return VM_ERROR;
				vm->SP--;
				OBJECT_SET_NIL(ret);
				return VM_OK;
			}
		default:
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	} else {
		return function_call(vm, METHOD_GET_FUNCTION(method), env, ret);
	}
}

/////////////////////////////////////////////////
// CELL_EFFECTIVE_METHOD

#define EMETHOD_PARAMETER								0x000000ff
#define EMETHOD_PARAMETER_								0xffffff00

#define EMETHOD_GET_PARAMETER(emethod)					((emethod)->ui&EMETHOD_PARAMETER)
#define EMETHOD_SET_PARAMETER(emethod, n)				((emethod)->ui&=EMETHOD_PARAMETER_, (emethod)->ui|=(n)&EMETHOD_PARAMETER)
#define EMETHOD_GET_SIZE(emethod)						(((emethod)+1)->ui)
#define EMETHOD_SET_SIZE(emethod, size)					(((emethod)+1)->ui=(size))
#define EMETHOD_GET_AROUND_METHOD_LIST(emethod)			(((emethod)+2)->cell)
#define EMETHOD_SET_AROUND_METHOD_LIST(emethod, list)	(((emethod)+2)->cell=(list))
#define EMETHOD_GET_BEFORE_METHOD_LIST(emethod)			(((emethod)+3)->cell)
#define EMETHOD_SET_BEFORE_METHOD_LIST(emethod, list)	(((emethod)+3)->cell=(list))
#define EMETHOD_GET_PRIMARY_METHOD_LIST(emethod)		(((emethod)+4)->cell)
#define EMETHOD_SET_PRIMARY_METHOD_LIST(emethod, list)	(((emethod)+4)->cell=(list))
#define EMETHOD_GET_AFTER_METHOD_LIST(emethod)			(((emethod)+5)->cell)
#define EMETHOD_SET_AFTER_METHOD_LIST(emethod, list)	(((emethod)+5)->cell=(list))
#define EMETHOD_GET_NEXT(emethod)						(((emethod)+6)->cell)
#define EMETHOD_SET_NEXT(emethod, next)					(((emethod)+6)->cell=(next))
#define EMETHOD_GET_PARAMETER_PROFILER(emethod)			((tOBJECT*)((emethod)+7))

static void emethod_initialize_parameter_profiler(tPCELL emethod, tPVM vm);
static VM_RET emethod_initialize_method_list(tPCELL emethod, tPVM vm, tPCELL gfunction);
static VM_RET emethod_initialize_method_list_f(tPVM vm, tPCELL emethod, tPCELL glist, tPCELL* elist);
static VM_RET emethod_initialize_method_list_r(tPVM vm, tPCELL emethod, tPCELL glist, tPCELL* elist);
static VM_RET emethod_add_method_f(tPVM vm, tPCELL emethod, tPCELL method, tPCELL elist);
static VM_RET emethod_add_method_r(tPVM vm, tPCELL emethod, tPCELL method, tPCELL elist);
static tBOOL emethod_method_is_applicable(tPVM vm, tPCELL emethod, tPCELL method);
static tBOOL emethod_compare_method(tPVM vm, tPCELL emethod, tPCELL method1, tPCELL method2);
static VM_RET emethod_call_method_list(tPVM vm, tPCELL emethod, tPCELL list, tPOBJECT ret);
static VM_RET emethod_call_before_after_method(tPVM vm, tPCELL emethod, tPCELL list, tPOBJECT ret);

VM_RET effective_method_create_(tPVM vm, tPCELL gfunction, tPCELL* cell)
{
	tUINT size;
	tINT n;
	tOBJECT tmp;

	n=gfunction_get_parameter_number(gfunction);
	size=allocate_cell(vm, sizeof(tCELL)*7+sizeof(tOBJECT)*n, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_EFFECTIVE_METHOD);
	EMETHOD_SET_SIZE(*cell, size);
	EMETHOD_SET_PARAMETER(*cell, n);
	EMETHOD_SET_AROUND_METHOD_LIST(*cell, 0);
	EMETHOD_SET_BEFORE_METHOD_LIST(*cell, 0);
	EMETHOD_SET_PRIMARY_METHOD_LIST(*cell, 0);
	EMETHOD_SET_AFTER_METHOD_LIST(*cell, 0);
	EMETHOD_SET_NEXT(*cell, 0);

	OBJECT_SET_EFFECTIVE_METHOD(&tmp, *cell);
	if (vm_push_temp(vm, &tmp)) return VM_ERROR;
	// 引数クラス組み合わせの初期化
	emethod_initialize_parameter_profiler(*cell, vm);
	// 包括関数に登録されているメソッドの中で
	// 適用可能なメソッドを優先度順にならべたリストを作成する
	if (emethod_initialize_method_list(*cell, vm, gfunction)) { vm_pop(vm); return VM_ERROR; }
	vm_pop_temp(vm);
	return VM_OK;
}

static void emethod_initialize_parameter_profiler(tPCELL emethod, tPVM vm)
{
	tINT i;
	const tINT n=EMETHOD_GET_PARAMETER(emethod);
	for (i=0; i<n; i++) {
		object_get_class(vm->SP-n+1+i, EMETHOD_GET_PARAMETER_PROFILER(emethod)+i);
	}
}

static VM_RET emethod_initialize_method_list(tPCELL emethod, tPVM vm, tPCELL gfunction)
{
	tPCELL list;
	// around
	if (gfunction_get_around_method_list(gfunction)) {
		if (emethod_initialize_method_list_f(vm, emethod, gfunction_get_around_method_list(gfunction), &list)) return VM_ERROR;
		EMETHOD_SET_AROUND_METHOD_LIST(emethod, list);
	}
	// before
	if (gfunction_get_before_method_list(gfunction)) {
		if (emethod_initialize_method_list_f(vm, emethod, gfunction_get_before_method_list(gfunction), &list)) return VM_ERROR;
		EMETHOD_SET_BEFORE_METHOD_LIST(emethod, list);
	}
	// primary
	if (gfunction_get_primary_method_list(gfunction)) {
		if (emethod_initialize_method_list_f(vm, emethod, gfunction_get_primary_method_list(gfunction), &list)) return VM_ERROR;
		EMETHOD_SET_PRIMARY_METHOD_LIST(emethod, list);
	}
	// after
	if (gfunction_get_after_method_list(gfunction)) {
		if (emethod_initialize_method_list_r(vm, emethod, gfunction_get_after_method_list(gfunction), &list)) return VM_ERROR;
		EMETHOD_SET_AFTER_METHOD_LIST(emethod, list);

	}
	// 包括関数に登録
	return gfunction_add_emethod(vm, gfunction, emethod);
}

static VM_RET emethod_initialize_method_list_f(tPVM vm, tPCELL emethod, tPCELL glist, tPCELL* elist)
{
	tOBJECT tmp;
	tPCELL method;
	*elist=0;
	for (; glist; glist=cons_get_cdr_cons(glist)) {
		cons_get_car(glist, &tmp);
		method=OBJECT_GET_CELL(&tmp);
		// この実効メソッドにメソッドが適用可能か否かを判定
		if (emethod_method_is_applicable(vm, emethod, method)) {
			if (!*elist) {
				if (cons_create_(vm, elist, &nil, &nil)) return VM_ERROR;
				OBJECT_SET_CONS(&tmp, *elist);
				if (vm_push(vm, &tmp)) return VM_ERROR;
			}
			// 適用可能なメソッドをリストに追加
			if (emethod_add_method_f(vm, emethod, method, *elist)) { vm_pop(vm); return VM_ERROR; }
		}
	}
	if (*elist) vm_pop(vm);
	return VM_OK;
}

static VM_RET emethod_initialize_method_list_r(tPVM vm, tPCELL emethod, tPCELL glist, tPCELL* elist)
{
	tOBJECT tmp;
	tPCELL method;
	*elist=0;
	for (; glist; glist=cons_get_cdr_cons(glist)) {
		cons_get_car(glist, &tmp);
		method=OBJECT_GET_CELL(&tmp);
		// この実効メソッドにメソッドが適用可能か否かを判定
		if (emethod_method_is_applicable(vm, emethod, method)) {
			if (!*elist) {
				if (cons_create_(vm, elist, &nil, &nil)) return VM_ERROR;
				OBJECT_SET_CONS(&tmp, *elist);
				if (vm_push(vm, &tmp)) return VM_ERROR;
			}
			// 適用可能なメソッドをリストに追加
			if (emethod_add_method_r(vm, emethod, method, *elist)) { vm_pop(vm); return VM_ERROR; }
		}
	}
	if (*elist) vm_pop(vm);
	return VM_OK;
}

// 優先度の高い順にメソッドをリストに追加する
static VM_RET emethod_add_method_f(tPVM vm, tPCELL emethod, tPCELL method, tPCELL elist)
{
	tOBJECT tmp;
	cons_get_car(elist, &tmp);
	if (OBJECT_IS_NIL(&tmp)) {
		// 一つ目のメソッド
		cell_to_object(method, &tmp);
		cons_set_car(elist, &tmp);
	} else {
		tPCELL last, p, pp;
		tOBJECT next;
		for (p=elist, last=0; p; last=p, p=cons_get_cdr_cons(p)) {
			cons_get_car(p, &tmp);
			// emethodの引数のクラスのクラス優先度リストにおいて
			// methodがリストのメソッドよりも優先度が高ければtTRUEを返す
			if (emethod_compare_method(vm, emethod, method, OBJECT_GET_CELL(&tmp))) break;
		}
		if (last) {
			// lastの後ろにmethodを追加
			cons_get_cdr(last, &next);
			cell_to_object(method, &tmp);
			if (cons_create_(vm, &pp, &tmp, &next)) return VM_ERROR;
			OBJECT_SET_CONS(&tmp, pp);
			cons_set_cdr(last, &tmp);
		} else {
			// 先頭に追加
			cons_get_car(elist, &tmp);
			cons_get_cdr(elist, &next);
			if (cons_create_(vm, &pp, &tmp, &next)) return VM_ERROR;
			cell_to_object(method, &tmp);
			OBJECT_SET_CONS(&next, pp);
			cons_set_car(elist, &tmp);
			cons_set_cdr(elist, &next);
		}
	}
	return VM_OK;
}

// 優先度の低い順にリストに追加する
static VM_RET emethod_add_method_r(tPVM vm, tPCELL emethod, tPCELL method, tPCELL elist)
{
	tOBJECT tmp;
	cons_get_car(elist, &tmp);
	if (OBJECT_IS_NIL(&tmp)) {
		// 一つ目のメソッド
		cell_to_object(method, &tmp);
		cons_set_car(elist, &tmp);
	} else {
		tPCELL last, p, pp;
		tOBJECT next;
		for (p=elist, last=0; p; last=p, p=cons_get_cdr_cons(p)) {
			cons_get_car(p, &tmp);
			// fと順番が逆!!!
			if (emethod_compare_method(vm, emethod, OBJECT_GET_CELL(&tmp), method)) break;
		}
		if (last) {
			// lastの後ろにmethodを追加
			cons_get_cdr(last, &next);
			cell_to_object(method, &tmp);
			if (cons_create_(vm, &pp, &tmp, &next)) return VM_ERROR;
			OBJECT_SET_CONS(&tmp, pp);
			cons_set_cdr(last, &tmp);
		} else {
			// 先頭に追加
			cons_get_car(elist, &tmp);
			cons_get_cdr(elist, &next);
			if (cons_create_(vm, &pp, &tmp, &next)) return VM_ERROR;
			cell_to_object(method, &tmp);
			OBJECT_SET_CONS(&next, pp);
			cons_set_car(elist, &tmp);
			cons_set_cdr(elist, &next);
		}
	}
	return VM_OK;
}

tUINT effective_method_get_size(tPCELL emethod)
{
	return EMETHOD_GET_SIZE(emethod);
}

VM_RET effective_method_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_string(vm, stream, "#tisl([effective-method])");
}

VM_RET effective_method_mark(tPVM vm, tPCELL cell)
{
	tINT i, n;
	if (cell_mark(vm, EMETHOD_GET_AROUND_METHOD_LIST(cell))||
		cell_mark(vm, EMETHOD_GET_BEFORE_METHOD_LIST(cell))||
		cell_mark(vm, EMETHOD_GET_PRIMARY_METHOD_LIST(cell))||
		cell_mark(vm, EMETHOD_GET_AFTER_METHOD_LIST(cell))||
		cell_mark(vm, EMETHOD_GET_NEXT(cell))) return VM_ERROR;
	n=EMETHOD_GET_PARAMETER(cell);
	for (i=0; i<n; i++) {
		if (object_mark(vm, EMETHOD_GET_PARAMETER_PROFILER(cell)+i)) return VM_ERROR;
	}
	return VM_OK;
}

tPCELL emethod_get_around_method_list(tPCELL emethod)
{
	return EMETHOD_GET_AROUND_METHOD_LIST(emethod);
}

tPCELL emethod_get_before_method_list(tPCELL emethod)
{
	return EMETHOD_GET_BEFORE_METHOD_LIST(emethod);
}

tPCELL emethod_get_primary_method_list(tPCELL emethod)
{
	return EMETHOD_GET_PRIMARY_METHOD_LIST(emethod);
}

tPCELL emethod_get_after_method_list(tPCELL emethod)
{
	return EMETHOD_GET_AFTER_METHOD_LIST(emethod);
}

// 引数がスタック上に置かれている場合
// メソッド呼出し語のスタック上の引数の処理は行わない．
static VM_RET emethod_call_method_list(tPVM vm, tPCELL emethod, tPCELL list, tPOBJECT ret)
{
	tINT n;
	tPCELL method;
	tOBJECT tmp;

	cons_get_car(list, &tmp);
	method=OBJECT_GET_CELL(&tmp);
	n=method_get_parameter_number(method);
	if (method_is_next(method)) {
		tPCELL env, amethod;
		tINT i;
		// 次のメソッドを用意する必要がある
		// 引数の初期化用のため実引数を環境に保存しておく
		if (environment_create_(vm, n, 0, &env)) return VM_ERROR;
		for (i=0; i<n; i++) {
			if (environment_set_value(vm, env, i, vm->SP-n+1+i)) return VM_ERROR;
		}
		if (applicable_method_create(vm, emethod, env, &amethod)) return VM_ERROR;
		OBJECT_SET_APPLICABLE_METHOD(&tmp, amethod);
		if (vm_push(vm, &tmp)) return VM_ERROR;

		if (method_is_stack(method)) {
			// 引数をスタック上で扱える場合
			tPCELL env2, env3;
			// 環境の設定
			if (environment_create_(vm, 1, 0, &env3)) { vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_ENVIRONMENT(&tmp, env3);
			if (vm_push_temp(vm, &tmp)) { vm_pop(vm); return VM_ERROR; }
			if (environment_create_(vm, 2, 0, &env2)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_APPLICABLE_METHOD(&tmp, amethod);
			if (environment_set_value(vm, env2, 0, &tmp)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_CONS(&tmp, list);
			if (environment_set_value(vm, env2, 1, &tmp)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_ENVIRONMENT(&tmp, env2);
			if (environment_set_value(vm, env3, 0, &tmp)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			vm_pop(vm);
			// メソッドの実行
			if (method_call(vm, method, env3, ret)) { vm_pop_temp(vm); return VM_ERROR; }
			vm_pop_temp(vm);
		} else {
			// 引数をスタック上で扱えない場合
			tPCELL env2, env3;
			// 環境の設定
			if (environment_create_(vm, 1+n, 0, &env3)) { vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_ENVIRONMENT(&tmp, env3);
			if (vm_push_temp(vm, &tmp)) { vm_pop(vm); return VM_ERROR; }
			if (environment_create_(vm, 2, 0, &env2)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_APPLICABLE_METHOD(&tmp, amethod);
			if (environment_set_value(vm, env2, 0, &tmp)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_CONS(&tmp, list);
			if (environment_set_value(vm, env2, 1, &tmp)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			OBJECT_SET_ENVIRONMENT(&tmp, env2);
			if (environment_set_value(vm, env3, 0, &tmp)) { vm_pop_temp(vm); vm_pop(vm); return VM_ERROR; }
			vm_pop(vm);
			for (i=0; i<n; i++) {
				if (environment_set_value(vm, env3, i+1, vm->SP-n+1+i)) return VM_ERROR;
			}
			// メソッドの実行
			if (method_call(vm, method, env3, ret)) { vm_pop_temp(vm); return VM_ERROR; }
			vm_pop_temp(vm);
		}
	} else {
		// 次のメソッドを用意する必要はない．
		if (method_is_stack(method)) {
			// 引数をスタック上で扱える場合
			if (method_call(vm, method, 0, ret)) return VM_ERROR;
		} else {
			// 引数をスタック上で扱えない場合
			tPCELL env;
			tINT i;
			// 環境の作成
			if (environment_create_(vm, n, 0, &env)) return VM_ERROR;
			for (i=0; i<n; i++) {
				if (environment_set_value(vm, env, i, vm->SP-n+1+i)) return VM_ERROR;
			}
			// メソッドの実行
			OBJECT_SET_ENVIRONMENT(&tmp, env);
			if (vm_push_temp(vm, &tmp)) return VM_ERROR;
			if (method_call(vm, method, env, ret)) { vm_pop_temp(vm); return VM_ERROR; }
			vm_pop_temp(vm);
		}
	}
	return VM_OK;
}

static VM_RET emethod_call_before_after_method(tPVM vm, tPCELL emethod, tPCELL list, tPOBJECT ret)
{
	tINT sp=vm->SP-vm->stack;
	for (; list; list=cons_get_cdr_cons(list)) {
		tPCELL method;
		tOBJECT tmp;
		tINT i, n;
		cons_get_car(list, &tmp);
		method=OBJECT_GET_CELL(&tmp);
		n=METHOD_GET_PARAMETER(method);
		if (method_is_stack(method)) {
			// 引数がスタック上で扱える場合
			// 引数のコピー
			for (i=n-1; i>=0; i--) {
				tmp=vm->stack[sp-i];
				if (vm_push(vm, &tmp)) return VM_ERROR;
			}
			if (method_call(vm, method, 0, &tmp)) return VM_ERROR;
			vm->SP=vm->stack+sp;
		} else {
			// 引数がスタックで扱えない場合
			tPCELL env;
			if (environment_create_(vm, n, 0, &env)) return VM_ERROR;
			for (i=n-1; i>=0; i--) {
				if (environment_set_value(vm, env, n-1-i, vm->stack+sp-i)) return VM_ERROR;
			}
			OBJECT_SET_ENVIRONMENT(&tmp, env);
			if (vm_push_temp(vm, &tmp)) return VM_ERROR;
			if (method_call(vm, method, env, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			vm_pop_temp(vm);
		}
	}
	return VM_OK;
}

// 最も優先度の高いaroundメソッドを呼び出す
VM_RET emethod_call_around(tPVM vm, tPCELL emethod, tPOBJECT value)
{
	return emethod_call_method_list(vm, emethod, EMETHOD_GET_AROUND_METHOD_LIST(emethod), value);
}

// aroundメソッドが存在しなかった
// before primary afterのメソッドの呼び出し
VM_RET emethod_call_no_around(tPVM vm, tPCELL emethod, tPOBJECT value)
{
	tOBJECT dummy;
	// beforeメソッドの呼び出し
	if (EMETHOD_GET_BEFORE_METHOD_LIST(emethod)&&
		emethod_call_before_after_method(vm, emethod, EMETHOD_GET_BEFORE_METHOD_LIST(emethod), &dummy)) return VM_ERROR;
	// 主メソッドの呼び出し
	if (EMETHOD_GET_PRIMARY_METHOD_LIST(emethod)) {
		if (emethod_call_method_list(vm, emethod, EMETHOD_GET_PRIMARY_METHOD_LIST(emethod), value)) return VM_ERROR;
	} else {
		return signal_condition(vm, TISL_ERROR_NO_APPLICABLE_METHOD);
	}
	if (vm_push_temp(vm, value)) return VM_ERROR;
	// afterメソッドの呼び出し
	if (EMETHOD_GET_AFTER_METHOD_LIST(emethod)&&
		emethod_call_before_after_method(vm, emethod, EMETHOD_GET_AFTER_METHOD_LIST(emethod), &dummy)) { vm_pop_temp(vm); return VM_ERROR; }
	vm_pop_temp(vm);
	return VM_OK;
}

VM_RET emethod_call_primary(tPVM vm, tPCELL emethod, tPOBJECT value)
{
	return emethod_call_method_list(vm, emethod, EMETHOD_GET_PRIMARY_METHOD_LIST(emethod), value);
}

tPCELL emethod_get_next(tPCELL emethod)
{
	return EMETHOD_GET_NEXT(emethod);
}

void emethod_set_next(tPCELL emethod, tPCELL next)
{
	EMETHOD_SET_NEXT(emethod, next);
}

// 実効メソッドが現在の引数に対して適用可能か否かを調べる
tBOOL emethod_is_applicable(tPCELL emethod, tPVM vm)
{
	tINT i, n;
	n=EMETHOD_GET_PARAMETER(emethod);
	// 実効メソッドの引数特殊化指定子と
	// スタック上の引数のクラスが一致した場合のみ再利用可能
	for (i=0; i<n; i++) {
		tOBJECT clss;
		object_get_class(vm->SP-n+1+i, &clss);
		if (!object_eql(EMETHOD_GET_PARAMETER_PROFILER(emethod)+i, &clss)) return tFALSE;
	}
	return tTRUE;
}

static tBOOL emethod_method_is_applicable(tPVM vm, tPCELL emethod, tPCELL method)
{
	// methodの特殊化指定子のクラスが
	// emethodの引数のクラスの上位クラスまたは同じクラスならば適用可能
	tINT i, n;
	tOBJECT eclss, clss;
	tPCELL bind, bind_list;

	n=EMETHOD_GET_PARAMETER(emethod);
	for (i=0; i<n; i++) {
		bind_list=METHOD_GET_SPECIALIZER(method)[i];
		bind=bind_list_get_bind(bind_list, NAMESPACE_CLASS, vm_get_current_package(vm));
		if (!bind) return tFALSE;
		bind_get_class(bind, &clss);
		eclss=EMETHOD_GET_PARAMETER_PROFILER(emethod)[i];
		if (!object_eql(&eclss, &clss)&&
			!class_is_subclass(vm, &eclss, &clss)) return tFALSE;
	}
	return tTRUE;
}

static tINT gfunction_get_emethod_key_(tPCELL gfunction, tPCELL emethod)
{
	tINT key, i, n=GFUNCTION_GET_PARAMETER_NUMBER(gfunction);
	key=0;
	for (i=0; i<n; i++) {
		key<<=1;
		key+=(tINT)EMETHOD_GET_PARAMETER_PROFILER(emethod)[i].data.p;
	}
	key=(key/23)%GFUNCTION_EFFECTIVE_METHOD_TABLE_SIZE;
	return (key<0) ? -key : key;
}

static tBOOL emethod_compare_method(tPVM vm, tPCELL emethod, tPCELL method1, tPCELL method2)
{
	// emethodのクラス優先度の中で
	// method1の優先度と
	// method2の優先度を比較する
	// method1の法が優先度が高ければtTRUEを返す
	tINT p1, p2, i, n;
	tPCELL blist, bind;
	tOBJECT clss1, clss2;

	n=EMETHOD_GET_PARAMETER(emethod);
	for (i=0; i<n; i++) {
		blist=METHOD_GET_SPECIALIZER(method1)[i];
		bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
		bind_get_class(bind, &clss1);
		blist=METHOD_GET_SPECIALIZER(method2)[i];
		bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
		bind_get_class(bind, &clss2);
		p1=class_get_precedence(vm, EMETHOD_GET_PARAMETER_PROFILER(emethod)+i, &clss1);
		p2=class_get_precedence(vm, EMETHOD_GET_PARAMETER_PROFILER(emethod)+i, &clss2);
		if (p1>p2) return tTRUE;
		if (p1<p2) return tFALSE;
	}
	// 合致するメソッドはないはず．
	return tFALSE;
}

/////////////////////////////////////////////////
// CELL_APPLICABLE_METHOD

#define AMETHOD_GET_EMETHOD(amethod)			(((amethod)+1)->cell)
#define AMETHOD_SET_EMETHOD(amethod, emethod)	(((amethod)+1)->cell=(emethod))
#define AMETHOD_GET_ENV(amethod)				(((amethod)+2)->cell)
#define AMETHOD_SET_ENV(amethod, env)			(((amethod)+2)->cell=(env))

static VM_RET amethod_call_method_list(tPVM vm, tPCELL amethod, tPCELL list);
static VM_RET amethod_call_before_after_method_list(tPVM vm, tPCELL amethod, tPCELL list);

VM_RET applicable_method_create(tPVM vm, tPCELL emethod, tPCELL env, tPCELL* cell)
{
	if (!allocate_cell(vm, sizeof(tCELL)*CELL_UNIT, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_APPLICABLE_METHOD);
	AMETHOD_SET_EMETHOD(*cell, emethod);
	AMETHOD_SET_ENV(*cell, env);

	return VM_OK;
}

tUINT applicable_method_get_size(tPCELL amethod)
{
	return CELL_UNIT;
}

VM_RET applicable_method_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_string(vm, stream, "#tisl([applicable-method])");
}

VM_RET applicable_method_mark(tPVM vm, tPCELL cell)
{
	tOBJECT tmp;
	cell_to_object(AMETHOD_GET_EMETHOD(cell), &tmp);
	if (gc_push(vm, &tmp)) return VM_ERROR;
	cell_to_object(AMETHOD_GET_ENV(cell), &tmp);
	if (gc_push(vm, &tmp)) return VM_ERROR;
	return VM_OK;
}


VM_RET amethod_call_primary(tPVM vm, tPCELL amethod, tPCELL list)
{
	tOBJECT tmp;
	cons_get_cdr(list, &tmp);
	if (OBJECT_IS_NIL(&tmp)) {
		// 次のメソッドは存在しない
		return signal_condition(vm, TISL_ERROR_NO_APPLICABLE_METHOD);
	} else {
		return amethod_call_method_list(vm, amethod, cons_get_cdr_cons(list));
	}
}

VM_RET amethod_call_around(tPVM vm, tPCELL amethod, tPCELL list)
{
	tOBJECT tmp;
	cons_get_cdr(list, &tmp);
	if (OBJECT_IS_NIL(&tmp)) {
		// 次の around method は存在しない
		tPCELL emethod=AMETHOD_GET_EMETHOD(amethod);
		// before
		if (emethod_get_before_method_list(emethod)&&
			amethod_call_before_after_method_list(vm, amethod, emethod_get_before_method_list(emethod))) return VM_ERROR;
		// primary
		if (emethod_get_primary_method_list(emethod)) {
			if (amethod_call_method_list(vm, amethod, emethod_get_primary_method_list(emethod))) return VM_ERROR;
		} else return signal_condition(vm, TISL_ERROR_NO_APPLICABLE_METHOD);
		// after
		if (emethod_get_after_method_list(emethod)&&
			amethod_call_before_after_method_list(vm, amethod, emethod_get_after_method_list(emethod))) return VM_ERROR;
		return VM_OK;
	} else {
		//
		return amethod_call_method_list(vm, amethod, cons_get_cdr_cons(list));
	}
}

static VM_RET amethod_call_method_list(tPVM vm, tPCELL amethod, tPCELL list)
{
	tINT sp=vm->SP-vm->stack;
	tPCELL method, env=AMETHOD_GET_ENV(amethod);
	tINT n;
	tOBJECT tmp;

	cons_get_car(list, &tmp);
	method=OBJECT_GET_CELL(&tmp);
	n=method_get_parameter_number(method);
	if (method_is_next(method)) {
		// 次のメソッドを用意する必要がある
		tINT i;
		tPCELL env2, env3;
		if (method_is_stack(method)) {
			// 引数をスタック上で扱える場合
			// 環境の作成
			if (environment_create_(vm, 1, 0, &env3)) return VM_ERROR;
			OBJECT_SET_ENVIRONMENT(&tmp, env3);
			if (vm_push_temp(vm, &tmp)) return VM_ERROR;
			if (environment_create_(vm, 2, 0, &env2)) { vm_pop_temp(vm); return VM_ERROR; }
			OBJECT_SET_APPLICABLE_METHOD(&tmp, amethod);
			if (environment_set_value(vm, env2, 0, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			OBJECT_SET_CONS(&tmp, list);
			if (environment_set_value(vm, env2, 1, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			OBJECT_SET_ENVIRONMENT(&tmp, env2);
			if (environment_set_value(vm, env3, 0, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			// 引数の展開
			for (i=0; i<n; i++) {
				if (environment_get_value(vm, env, i, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
				if (vm_push(vm, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			}
		} else {
			// 引数をスタック上で扱えない場合
			// 環境の作成
			if (environment_create_(vm, n+1, 0, &env3)) return VM_ERROR;
			OBJECT_SET_ENVIRONMENT(&tmp, env3);
			if (vm_push_temp(vm, &tmp)) return VM_ERROR;
			if (environment_create_(vm, 2, 0, &env2)) { vm_pop_temp(vm); return VM_ERROR; }
			OBJECT_SET_APPLICABLE_METHOD(&tmp, amethod);
			if (environment_set_value(vm, env2, 0, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			OBJECT_SET_CONS(&tmp, list);
			if (environment_set_value(vm, env2, 1, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			OBJECT_SET_ENVIRONMENT(&tmp, env2);
			if (environment_set_value(vm, env3, 0, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			// 引数
			for (i=0; i<n; i++) {
				if (environment_get_value(vm, env, i, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
				if (environment_set_value(vm, env3, i+1, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			}
		}
		if (method_call(vm, method, env3, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
		vm_pop_temp(vm);
		vm->SP=vm->stack+sp+1;
		*vm->SP=tmp;
	} else {
		tINT i;
		// 次のメソッドを用意する必要ない
		if (method_is_stack(method)) {
			// 引数をスタック上で扱える
			for (i=0; i<n; i++) {
				if (environment_get_value(vm, env, i, &tmp)) return VM_ERROR;
				if (vm_push(vm, &tmp)) return VM_ERROR;
			}
			// メソッドの実行
			if (method_call(vm, method, 0, &tmp)) return VM_ERROR;
			vm->SP=vm->stack+sp+1;
			*vm->SP=tmp;
		} else {
			// 引数をスタック上で扱えない
			tPCELL e;
			// 環境の作成
			if (environment_create_(vm, n, 0, &e)) return VM_ERROR;
			for (i=0; i<n; i++) {
				if (environment_get_value(vm, env, i, &tmp)) return VM_ERROR;
				if (environment_set_value(vm, e, i, &tmp)) return VM_ERROR;
			}
			OBJECT_SET_ENVIRONMENT(&tmp, e);
			if (vm_push_temp(vm, &tmp)) return VM_ERROR;
			// メソッドの実行
			if (method_call(vm, method, e, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			vm_pop_temp(vm);
			if (vm_push(vm, &tmp)) return VM_ERROR;
			vm->SP=vm->stack+1;
			*vm->SP=tmp;
		}
	}
	return VM_OK;
}

static VM_RET amethod_call_before_after_method_list(tPVM vm, tPCELL amethod, tPCELL list)
{
	tINT sp=vm->SP-vm->stack;
	tPCELL env=AMETHOD_GET_ENV(amethod);
	for (; list; list=cons_get_cdr_cons(list)) {
		tPCELL method;
		tOBJECT tmp;
		tINT i, n;

		cons_get_car(list, &tmp);
		method=OBJECT_GET_CELL(&tmp);
		n=METHOD_GET_PARAMETER(method);
		if (method_is_stack(method)) {
			// 引数をスタック上で扱える場合
			for (i=0; i<n; i++) {
				if (environment_get_value(vm, env, i, &tmp)) return VM_ERROR;
				if (vm_push(vm, &tmp)) return VM_ERROR;
			}
			if (method_call(vm, method, 0, &tmp)) return VM_ERROR;
			vm->SP=vm->stack+sp;
		} else {
			// 引数をスタック上で圧かない場合
			tPCELL e;
			if (environment_create_(vm, n, 0, &e)) return VM_ERROR;
			for (i=0; i<n; i++) {
				if (environment_get_value(vm, env, i, &tmp)) return VM_ERROR;
				if (environment_set_value(vm, e, i, &tmp)) return VM_ERROR;
			}
			OBJECT_SET_ENVIRONMENT(&tmp, e);
			if (vm_push_temp(vm, &tmp)) return VM_ERROR;
			if (method_call(vm, method, e, &tmp)) { vm_pop_temp(vm); return VM_ERROR; }
			vm_pop_temp(vm);
			vm->SP=vm->stack+sp;
		}
	}
	return VM_OK;
}

/////////////////////////////////////////////////
//CELL_LINKED_FUNCTION

#define LINKED_FUNCTION_PARAMETER				0x000000ff
#define LINKED_FUNCTION_PARAMETER_				0xffffff00
#define LINKED_FUNCTION_REST					0x00000100
#define LINKED_FUNCTION_REST_					0xfffffeff
#define LINKED_FUNCTION_VOIDP					0x00000200
#define LINKED_FUNCTION_VOIDP_					0xfffffdff

#define LINKED_FUNCTION_GET_PARAMETER(lf)		((lf)->ui&LINKED_FUNCTION_PARAMETER)
#define LINKED_FUNCTION_SET_PARAMETER(lf, n)	((lf)->ui&=LINKED_FUNCTION_PARAMETER_, (lf)->ui|=(n)&LINKED_FUNCTION_PARAMETER)
#define LINKED_FUNCTION_GET_REST(lf)			((lf)->ui&LINKED_FUNCTION_REST)
#define LINKED_FUNCTION_SET_REST(lf)			((lf)->ui|=LINKED_FUNCTION_REST)
#define LINKED_FUNCTION_RESET_REST(lf)			((lf)->ui&=LINKED_FUNCTION_REST_)
#define LINKED_FUNCTION_GET_VOIDP(lf)			((lf)->ui&LINKED_FUNCTION_VOIDP)
#define LINKED_FUNCTION_SET_VOIDP(lf)			((lf)->ui|=LINKED_FUNCTION_VOIDP)
#define LINKED_FUNCTION_RESET_VOIDP(lf)			((lf)->ui&=LINKED_FUNCTION_VOIDP_)
#define LINKED_FUNCTION_GET_SIZE(func)			(((func)+1)->ui)
#define LINKED_FUNCTION_SET_SIZE(func, size)	(((func)+1)->ui=(size))
#define LINKED_FUNCTION_GET_LIBRARY(func)		(((func)+2)->cell)
#define LINKED_FUNCTION_SET_LIBRARY(func, lib)	(((func)+2)->cell=(lib))
#define LINKED_FUNCTION_GET_FUNCTION(func)		(((func)+3)->p)
#define LINKED_FUNCTION_SET_FUNCTION(func, pp)	(((func)+3)->p=(pp))
#define LINKED_FUNCTION_GET_NAME(lf)			(((lf)+4)->cell)
#define LINKED_FUNCTION_SET_NAME(lf, name)		(((lf)+4)->cell=(name))
#define LINKED_FUNCTION_GET_LAMBDA_LIST(lf)		(((lf)+5)->cell)
#define LINKED_FUNCTION_SET_LAMBDA_LIST(lf, l)	(((lf)+5)->cell=(l))
#define LINKED_FUNCTION_GET_PROFILE_LIST(lf)	(((lf)+6)->cell)
#define LINKED_FUNCTION_SET_PROFILE_LIST(lf, l)	(((lf)+6)->cell=(l))

VM_RET linked_function_create_(tPVM vm, const tINT pnum, const tBOOL rest, tPCELL dll_name, tPCELL procedure_name, const tBOOL voidp, tPCELL lambda_list, tPCELL profile_list, tPCELL* cell)
{
#if defined(TISL_DYNAMIC)
	tUINT size;
	tPCELL lib;
	tOBJECT tmp;
	void* p;
	// DLLのロード
	if (linked_library_create_(vm, dll_name, &lib)) return VM_ERROR;
	cell_to_object(lib, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// 
	size=allocate_cell(vm, sizeof(tCELL)*7, cell);
	if (!size) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED); }
	vm_pop(vm);
	CELL_SET_TYPE(*cell, CELL_LINKED_FUNCTION);
	LINKED_FUNCTION_SET_SIZE(*cell, size);
	LINKED_FUNCTION_SET_PARAMETER(*cell, pnum);
	if (rest) LINKED_FUNCTION_SET_REST(*cell);
	else LINKED_FUNCTION_RESET_REST(*cell);
	if (voidp) LINKED_FUNCTION_SET_VOIDP(*cell);
	else LINKED_FUNCTION_RESET_VOIDP(*cell);
	LINKED_FUNCTION_SET_LIBRARY(*cell, lib);
	LINKED_FUNCTION_SET_NAME(*cell, procedure_name);
	LINKED_FUNCTION_SET_LAMBDA_LIST(*cell, lambda_list);
	LINKED_FUNCTION_SET_PROFILE_LIST(*cell, profile_list);
	p=get_procedure(linked_library_get_handle(lib), string_get_string(procedure_name));
	if (!p) return signal_condition(vm, TISL_ERROR_CANNOT_LINK_FOREIGN_PROCEDURE);
	LINKED_FUNCTION_SET_FUNCTION(*cell, p);
	return VM_OK;
#else
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
#endif
}

tUINT linked_function_get_size(tPCELL function)
{
	return LINKED_FUNCTION_GET_SIZE(function);
}

VM_RET linked_function_mark(tPVM vm, tPCELL cell)
{
	if (cell_mark(vm, LINKED_FUNCTION_GET_LIBRARY(cell))) return VM_ERROR;
	if (cell_mark(vm, LINKED_FUNCTION_GET_LAMBDA_LIST(cell))) return VM_ERROR;
	if (cell_mark(vm, LINKED_FUNCTION_GET_NAME(cell))) return VM_ERROR;
	if (cell_mark(vm, LINKED_FUNCTION_GET_PROFILE_LIST(cell))) return VM_ERROR;
	return VM_OK;
}

VM_RET linked_function_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT tmp;
	tPCELL lf=OBJECT_GET_CELL(obj);
	if (write_string(vm, stream, "#i(<library-function> ")) return VM_ERROR;
	if (LINKED_FUNCTION_GET_LAMBDA_LIST(lf)) {
		OBJECT_SET_CONS(&tmp, LINKED_FUNCTION_GET_LAMBDA_LIST(lf));
		if (write_object(vm, stream, &tmp)) return VM_ERROR;
		if (write_string(vm, stream, " ")) return VM_ERROR;
	} else {
		if (write_string(vm, stream, "() ")) return VM_ERROR;
	}
	OBJECT_SET_LINKED_LIBRARY(&tmp, LINKED_FUNCTION_GET_LIBRARY(lf));
	if (write_object(vm, stream, &tmp)) return VM_ERROR;
	if (write_string(vm, stream, " ")) return VM_ERROR;
	if (write_string(vm, stream, string_get_string(LINKED_FUNCTION_GET_NAME(lf)))) return VM_ERROR;
	if (write_string(vm, stream, ")")) return VM_ERROR;
	return VM_OK;
}

tBOOL linked_function_is_rest(tPCELL lf)
{
	return LINKED_FUNCTION_GET_REST(lf) ? tTRUE : tFALSE;
}

tINT linked_function_get_parameter_number(tPCELL lf)
{
	return LINKED_FUNCTION_GET_PARAMETER(lf);
}

/////////////////////////////////////////////////
// linked_function_call

typedef union FARG_		FARG;

union FARG_ {
	TISL_OBJECT	l;
	tINT		i;
	tFLOAT		f;
	tCSTRING	s;
	void*		p;
};

//なんか無駄 アセンブラで書く？ ...で良い？
typedef TISL_OBJECT (*LF_FF)(TNI*, ...);
typedef TISL_OBJECT (*LF_00)(TNI*);
typedef TISL_OBJECT (*LF_01)(TNI*, FARG);
typedef TISL_OBJECT (*LF_02)(TNI*, FARG, FARG);
typedef TISL_OBJECT (*LF_03)(TNI*, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_04)(TNI*, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_05)(TNI*, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_06)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_07)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_08)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_09)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_10)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_11)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_12)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_13)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_14)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_15)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_16)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_17)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_18)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_19)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef TISL_OBJECT (*LF_20)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_FF_V)(TNI*, ...);
typedef void (*LF_00_V)(TNI*);
typedef void (*LF_01_V)(TNI*, FARG);
typedef void (*LF_02_V)(TNI*, FARG, FARG);
typedef void (*LF_03_V)(TNI*, FARG, FARG, FARG);
typedef void (*LF_04_V)(TNI*, FARG, FARG, FARG, FARG);
typedef void (*LF_05_V)(TNI*, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_06_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_07_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_08_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_09_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_10_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_11_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_12_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_13_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_14_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_15_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_16_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_17_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_18_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_19_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);
typedef void (*LF_20_V)(TNI*, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG, FARG);

static VM_RET make_farg(tPVM vm, tPCELL pprofile, tPOBJECT arg, FARG* farg);

static tPCELL lf_call_00(tPVM vm, tPCELL function);
static tPCELL lf_call_01(tPVM vm, tPCELL function);
static tPCELL lf_call_02(tPVM vm, tPCELL function);
static tPCELL lf_call_03(tPVM vm, tPCELL function);
static tPCELL lf_call_04(tPVM vm, tPCELL function);
static tPCELL lf_call_05(tPVM vm, tPCELL function);
static tPCELL lf_call_06(tPVM vm, tPCELL function);
static tPCELL lf_call_07(tPVM vm, tPCELL function);
static tPCELL lf_call_08(tPVM vm, tPCELL function);
static tPCELL lf_call_09(tPVM vm, tPCELL function);
static tPCELL lf_call_10(tPVM vm, tPCELL function);
static tPCELL lf_call_11(tPVM vm, tPCELL function);
static tPCELL lf_call_12(tPVM vm, tPCELL function);
static tPCELL lf_call_13(tPVM vm, tPCELL function);
static tPCELL lf_call_14(tPVM vm, tPCELL function);
static tPCELL lf_call_15(tPVM vm, tPCELL function);
static tPCELL lf_call_16(tPVM vm, tPCELL function);
static tPCELL lf_call_17(tPVM vm, tPCELL function);
static tPCELL lf_call_18(tPVM vm, tPCELL function);
static tPCELL lf_call_19(tPVM vm, tPCELL function);
static tPCELL lf_call_20(tPVM vm, tPCELL function);
static void lf_call_00_v(tPVM vm, tPCELL function);
static void lf_call_01_v(tPVM vm, tPCELL function);
static void lf_call_02_v(tPVM vm, tPCELL function);
static void lf_call_03_v(tPVM vm, tPCELL function);
static void lf_call_04_v(tPVM vm, tPCELL function);
static void lf_call_05_v(tPVM vm, tPCELL function);
static void lf_call_06_v(tPVM vm, tPCELL function);
static void lf_call_07_v(tPVM vm, tPCELL function);
static void lf_call_08_v(tPVM vm, tPCELL function);
static void lf_call_09_v(tPVM vm, tPCELL function);
static void lf_call_10_v(tPVM vm, tPCELL function);
static void lf_call_11_v(tPVM vm, tPCELL function);
static void lf_call_12_v(tPVM vm, tPCELL function);
static void lf_call_13_v(tPVM vm, tPCELL function);
static void lf_call_14_v(tPVM vm, tPCELL function);
static void lf_call_15_v(tPVM vm, tPCELL function);
static void lf_call_16_v(tPVM vm, tPCELL function);
static void lf_call_17_v(tPVM vm, tPCELL function);
static void lf_call_18_v(tPVM vm, tPCELL function);
static void lf_call_19_v(tPVM vm, tPCELL function);
static void lf_call_20_v(tPVM vm, tPCELL function);

typedef tPCELL (*LF_CALL_N)(tPVM, tPCELL);
typedef void (*LF_CALL_N_V)(tPVM, tPCELL);

const LF_CALL_N lf_call_n_table[]={
	lf_call_00,
	lf_call_01,
	lf_call_02,
	lf_call_03,
	lf_call_04,
	lf_call_05,
	lf_call_06,
	lf_call_07,
	lf_call_08,
	lf_call_09,
	lf_call_10,
	lf_call_11,
	lf_call_12,
	lf_call_13,
	lf_call_14,
	lf_call_15,
	lf_call_16,
	lf_call_17,
	lf_call_18,
	lf_call_19,
	lf_call_20,
};

const LF_CALL_N_V lf_call_n_v_table[]={
	lf_call_00_v,
	lf_call_01_v,
	lf_call_02_v,
	lf_call_03_v,
	lf_call_04_v,
	lf_call_05_v,
	lf_call_06_v,
	lf_call_07_v,
	lf_call_08_v,
	lf_call_09_v,
	lf_call_10_v,
	lf_call_11_v,
	lf_call_12_v,
	lf_call_13_v,
	lf_call_14_v,
	lf_call_15_v,
	lf_call_16_v,
	lf_call_17_v,
	lf_call_18_v,
	lf_call_19_v,
	lf_call_20_v,
};

VM_RET linked_function_call(tPVM vm, tPCELL function, tPOBJECT ret)
{
	tPCELL value;
	// 局所参照の記録
	if (vm_push_local_ref(vm)) return VM_ERROR;
	// 例外ハンドラの設定
	if (vm_push_foreign_function_handler(vm)) { vm_pop_local_ref(vm); return VM_ERROR; }
	// 呼び出し
	if (LINKED_FUNCTION_GET_VOIDP(function)) {
		(*lf_call_n_v_table[LINKED_FUNCTION_GET_PARAMETER(function)])(vm, function);
		value=0;
	} else {
		tPCELL old_package=vm_get_current_package(vm);
		value=(*lf_call_n_table[LINKED_FUNCTION_GET_PARAMETER(function)])(vm, function);
		vm_set_current_package(vm, old_package);
	}
	// 後処理
	vm_pop_handler(vm);
	vm_pop_local_ref(vm);
	if (vm_last_condition_is_ok(vm)) {
		if (value) {
			tisl_object_get_object((TISL_OBJECT)value, ret);
		} else {
			OBJECT_SET_NIL(ret);
		}
		return VM_OK;
	} else {
		return VM_ERROR;
	}
}

static VM_RET make_farg(tPVM vm, tPCELL pprofile, tPOBJECT arg, FARG* farg)
{
	tOBJECT tmp;
	TISL_OBJECT tobj;

	cons_get_car(pprofile, &tmp);
	if (OBJECT_IS_SYMBOL(&tmp)) {
		tPCELL blist, bind;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&tmp), &blist)) return VM_ERROR;
		bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
		if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, OBJECT_GET_CELL(&tmp), NAMESPACE_CLASS);
		bind_get_class(bind, &tmp);
	}
	if (OBJECT_IS_BUILT_IN_CLASS(&tmp)) {
		switch (OBJECT_GET_INTEGER(&tmp)) {
		case CLASS_INTEGER:
			if (!OBJECT_IS_INTEGER(arg)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, arg);
			farg->i=OBJECT_GET_INTEGER(arg);
			break;
		case CLASS_FLOAT:
			if (!OBJECT_IS_FLOAT(arg)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FLOAT, arg);
			farg->f=OBJECT_GET_FLOAT(arg);
			break;
		case CLASS_STRING:
			if (!OBJECT_IS_STRING(arg)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, arg);
			farg->s=string_get_string(OBJECT_GET_CELL(arg));
			break;
		case CLASS_FOREIGN_OBJECT:
			if (!OBJECT_IS_FOREIGN_OBJECT(arg)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_OBJECT, arg);
			farg->p=foreign_object_get_object(OBJECT_GET_CELL(arg));
			break;
		default:
			{
				TISL_OBJECT tobj;
				if (vm_new_local_ref_(vm, arg, &tobj)) return VM_ERROR;
				farg->l=tobj;
			}
		}
		return VM_OK;
	} else if (OBJECT_IS_FOREIGN_CLASS(&tmp)) {
		tOBJECT clss;
		if (!OBJECT_IS_FOREIGN_OBJECT(arg)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_OBJECT, arg);
		foreign_object_get_class(OBJECT_GET_CELL(arg), &clss);
		// <foreign-object>
		if (OBJECT_IS_BUILT_IN_CLASS(&clss)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_OBJECT, arg);
		if (!OBJECT_IS_FOREIGN_CLASS(&clss)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		if (!foreign_class_is_subclass(OBJECT_GET_CELL(&clss), OBJECT_GET_CELL(&tmp)))
			return signal_domain_error_(vm, TISL_ERROR_DOMAIN_ERROR, &tmp, arg);
		farg->p=foreign_object_get_object(OBJECT_GET_CELL(arg));
		return VM_OK;
	}
	if (vm_new_local_ref_(vm, arg, &tobj)) return VM_ERROR;
	farg->l=tobj;
	return VM_OK;
}

static tPCELL lf_call_00(tPVM vm, tPCELL function)
{
	return (tPCELL)(*((LF_00)LINKED_FUNCTION_GET_FUNCTION(function)))(vm_get_tni(vm));
}

static tPCELL lf_call_01(tPVM vm, tPCELL function)
{
	FARG arg01;
	tOBJECT tmp;
	tPCELL pp;
	tmp=*vm->SP;
	pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), &tmp, &arg01)) return 0;
	return (tPCELL)(*((LF_01)LINKED_FUNCTION_GET_FUNCTION(function)))(vm_get_tni(vm), arg01);
}

static tPCELL lf_call_02(tPVM vm, tPCELL function)
{
	FARG arg01, arg02;
	tINT sp=vm->SP-vm->stack-1;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg02)) return 0;
	return (tPCELL)(*(LF_02)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02);
}

static tPCELL lf_call_03(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03;
	tINT sp=vm->SP-vm->stack-2;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg03)) return 0;
	return (tPCELL)(*(LF_03)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03);
}

static tPCELL lf_call_04(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04;
	tINT sp=vm->SP-vm->stack-3;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg04)) return 0;
	return (tPCELL)(*(LF_04)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04);
}

static tPCELL lf_call_05(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05;
	tINT sp=vm->SP-vm->stack-4;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg05)) return 0;
	return (tPCELL)(*(LF_05)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05);
}

static tPCELL lf_call_06(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06;
	tINT sp=vm->SP-vm->stack-5;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg06)) return 0;
	return (tPCELL)(*(LF_06)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06);
}

static tPCELL lf_call_07(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07;
	tINT sp=vm->SP-vm->stack-6;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg07)) return 0;
	return (tPCELL)(*(LF_07)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07);
}

static tPCELL lf_call_08(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08;
	tINT sp=vm->SP-vm->stack-7;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg08)) return 0;
	return (tPCELL)(*(LF_08)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08);
}

static tPCELL lf_call_09(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09;
	tINT sp=vm->SP-vm->stack-8;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg09)) return 0;
	return (tPCELL)(*(LF_09)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09);
}

static tPCELL lf_call_10(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10;
	tINT sp=vm->SP-vm->stack-9;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg10)) return 0;
	return (tPCELL)(*(LF_10)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10);
}

static tPCELL lf_call_11(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11;
	tINT sp=vm->SP-vm->stack-10;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg11)) return 0;
	return (tPCELL)(*(LF_11)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11);
}

static tPCELL lf_call_12(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12;
	tINT sp=vm->SP-vm->stack-11;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg12)) return 0;
	return (tPCELL)(*(LF_12)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12);
}

static tPCELL lf_call_13(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13;
	tINT sp=vm->SP-vm->stack-12;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg13)) return 0;
	return (tPCELL)(*(LF_13)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13);
}

static tPCELL lf_call_14(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14;
	tINT sp=vm->SP-vm->stack-13;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg14)) return 0;
	return (tPCELL)(*(LF_14)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14);
}

static tPCELL lf_call_15(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15;
	tINT sp=vm->SP-vm->stack-14;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg15)) return 0;
	return (tPCELL)(*(LF_15)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15);
}

static tPCELL lf_call_16(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16;
	tINT sp=vm->SP-vm->stack-15;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg16)) return 0;
	return (tPCELL)(*(LF_16)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16);
}

static tPCELL lf_call_17(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17;
	tINT sp=vm->SP-vm->stack-16;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg17)) return 0;
	return (tPCELL)(*(LF_17)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17);
}

static tPCELL lf_call_18(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18;
	tINT sp=vm->SP-vm->stack-17;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg17)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg18)) return 0;
	return (tPCELL)(*(LF_18)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18);
}

static tPCELL lf_call_19(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19;
	tINT sp=vm->SP-vm->stack-18;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg17)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg18)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg19)) return 0;
	return (tPCELL)(*(LF_19)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19);
}

static tPCELL lf_call_20(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20;
	tINT sp=vm->SP-vm->stack-19;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg17)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg18)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg19)) return 0;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp,   &arg20)) return 0;
	return (tPCELL)(*(LF_20)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20);
}

static void lf_call_00_v(tPVM vm, tPCELL function)
{
	(*(LF_00_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm));
}

static void lf_call_01_v(tPVM vm, tPCELL function)
{
	FARG arg01;
	tINT sp=vm->SP-vm->stack;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg01)) return;
	(*(LF_01_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01);
}

static void lf_call_02_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02;
	tINT sp=vm->SP-vm->stack-1;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg02)) return;
	(*(LF_02_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02);
}

static void lf_call_03_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03;
	tINT sp=vm->SP-vm->stack-2;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg03)) return;
	(*(LF_03_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03);
}

static void lf_call_04_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04;
	tINT sp=vm->SP-vm->stack-3;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg04)) return;
	(*(LF_04_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04);
}

static void lf_call_05_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05;
	tINT sp=vm->SP-vm->stack-4;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg05)) return;
	(*(LF_05_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05);
}

static void lf_call_06_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06;
	tINT sp=vm->SP-vm->stack-5;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg06)) return;
	(*(LF_06_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06);
}

static void lf_call_07_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07;
	tINT sp=vm->SP-vm->stack-6;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg07)) return;
	(*(LF_07_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07);
}

static void lf_call_08_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08;
	tINT sp=vm->SP-vm->stack-7;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg08)) return;
	(*(LF_08_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08);
}

static void lf_call_09_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09;
	tINT sp=vm->SP-vm->stack-8;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg09)) return;
	(*(LF_09_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09);
}

static void lf_call_10_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10;
	tINT sp=vm->SP-vm->stack-9;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg10)) return;
	(*(LF_10_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10);
}

static void lf_call_11_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11;
	tINT sp=vm->SP-vm->stack-10;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg11)) return;
	(*(LF_11_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11);
}

static void lf_call_12_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12;
	tINT sp=vm->SP-vm->stack-11;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg12)) return;
	(*(LF_12_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12);
}

static void lf_call_13_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13;
	tINT sp=vm->SP-vm->stack-12;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg13)) return;
	(*(LF_13_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13);
}

static void lf_call_14_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14;
	tINT sp=vm->SP-vm->stack-13;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg14)) return;
	(*(LF_14_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14);
}

static void lf_call_15_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15;
	tINT sp=vm->SP-vm->stack-14;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg15)) return;
	(*(LF_15_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15);
}

static void lf_call_16_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16;
	tINT sp=vm->SP-vm->stack-15;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg16)) return;
	(*(LF_16_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16);
}

static void lf_call_17_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17;
	tINT sp=vm->SP-vm->stack-16;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg17)) return;
	(*(LF_17_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17);
}

static void lf_call_18_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18;
	tINT sp=vm->SP-vm->stack-17;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg17)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg18)) return;
	(*(LF_18_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18);
}

static void lf_call_19_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19;
	tINT sp=vm->SP-vm->stack-18;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg17)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg18)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg19)) return;
	(*(LF_19_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19);
}

static void lf_call_20_v(tPVM vm, tPCELL function)
{
	FARG arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20;
	tINT sp=vm->SP-vm->stack-19;
	tPCELL pp=LINKED_FUNCTION_GET_PROFILE_LIST(function);
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg01)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg02)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg03)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg04)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg05)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg06)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg07)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg08)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg09)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg10)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg11)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg12)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg13)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg14)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg15)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg16)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg17)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg18)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp++, &arg19)) return;
	if (make_farg(vm, pp=cons_get_cdr_cons(pp), vm->stack+sp  , &arg20)) return;
	(*(LF_20_V)LINKED_FUNCTION_GET_FUNCTION(function))(vm_get_tni(vm), arg01, arg02, arg03, arg04, arg05, arg06, arg07, arg08, arg09, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20);
}

/////////////////////////////////////////////////
// CELL_LINKED_LIBRARY

#define LINKED_LIBRARY_GET_NAME(ll)				(((ll)+1)->cell)
#define LINKED_LIBRARY_SET_NAME(ll, name)		(((ll)+1)->cell=(name))
#define LINKED_LIBRARY_GET_LIBRARY(ll)			(((ll)+2)->p)
#define LINKED_LIBRARY_SET_LIBRARY(ll, lib)		(((ll)+2)->p=(lib))

VM_RET linked_library_create_(tPVM vm, tPCELL name, tPCELL* cell)
{
#if defined(TISL_DYNAMIC)
	void* hDLL;
	if (!allocate_cell(vm, sizeof(tCELL)*CELL_UNIT, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_LINKED_LIBRARY);
	LINKED_LIBRARY_SET_NAME(*cell, name);
	hDLL=load_library(string_get_string(name));
	if (!hDLL) return signal_condition(vm, TISL_ERROR_CANNOT_OPEN_LIBRARY);
	LINKED_LIBRARY_SET_LIBRARY(*cell, hDLL);
	return VM_OK;
#else
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
#endif
}

tUINT linked_library_get_size(tPCELL ll)
{
	return CELL_UNIT;
}

VM_RET linked_library_mark(tPVM vm, tPCELL cell)
{
	// stringへのマーキング
	if (cell_mark(vm, LINKED_LIBRARY_GET_NAME(cell))) return VM_ERROR;
	return VM_OK;
}

VM_RET linked_library_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (write_string(vm, stream, "#i(<dynamic-link-library> name ")) return VM_ERROR;
	if (write_string(vm, stream, string_get_string(LINKED_LIBRARY_GET_NAME(OBJECT_GET_CELL(obj))))) return VM_ERROR;
	return write_string(vm, stream, ")");
}

VM_RET linked_library_destroy(tPVM vm, tPCELL ll)
{
#if defined(TISL_DYNAMIC)
	free_library(LINKED_LIBRARY_GET_LIBRARY(ll));
#endif
	return VM_OK;
}

void* linked_library_get_handle(tPCELL ll)
{
	return LINKED_LIBRARY_GET_LIBRARY(ll);
}
/////////////////////////////////////////////////
// CELL_TISL_OBJECT

#define TISL_OBJECT_GET_SIZE(tobj)			(((tobj)+1)->ui)
#define TISL_OBJECT_SET_SIZE(tobj, size)	(((tobj)+1)->ui=(size))
#define TISL_OBJECT_GET_NEXT(tobj)			(((tobj)+2)->cell)
#define TISL_OBJECT_SET_NEXT(tobj, next)	(((tobj)+2)->cell=(next))
#define TISL_OBJECT_GET_OBJECT(tobj, obj)	(*(obj)=*(tPOBJECT)((tobj)+3))
#define TISL_OBJECT_SET_OBJECT(tobj, obj)	(*(tPOBJECT)((tobj)+3)=*(obj))

VM_RET tisl_object_create(tPVM vm, tPOBJECT obj, tPCELL* cell)
{
	tUINT size;
	tOBJECT tmp;
	if (obj) {
		tmp=*obj;
	} else {
		OBJECT_SET_NIL(&tmp);
	}
	if (vm_push(vm, &tmp)) return VM_ERROR;
	size=allocate_cell(vm, sizeof(tCELL)*3+sizeof(tOBJECT), cell);
	if (!size) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED); }
	CELL_SET_TYPE(*cell, CELL_TISL_OBJECT);
	TISL_OBJECT_SET_SIZE(*cell, size);
	TISL_OBJECT_SET_OBJECT(*cell, &tmp);
	TISL_OBJECT_SET_NEXT(*cell, 0);
	vm_pop(vm);
	return VM_OK;
}

tUINT tisl_object_get_size(tPCELL tobj)
{
	return TISL_OBJECT_GET_SIZE(tobj);
}

VM_RET tisl_object_mark(tPVM vm, tPCELL cell)
{
	tOBJECT obj;
	TISL_OBJECT_GET_OBJECT(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	return cell_mark(vm, TISL_OBJECT_GET_NEXT(cell));
}

VM_RET tisl_object_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT tmp;
	TISL_OBJECT_GET_OBJECT(OBJECT_GET_CELL(obj), &tmp);
	return write_object(vm, stream, &tmp);
}

void tisl_object_get_object(TISL_OBJECT tobj, tPOBJECT obj)
{
	TISL_OBJECT_GET_OBJECT((tPCELL)tobj, obj);
}

void tisl_object_set_object(TISL_OBJECT tobj, tPOBJECT obj)
{
	TISL_OBJECT_SET_OBJECT((tPCELL)tobj, obj);
}

tPCELL tisl_object_get_next(tPCELL tobj)
{
	return TISL_OBJECT_GET_NEXT(tobj);
}

void tisl_object_set_next(tPCELL tobj, tPCELL next)
{
	TISL_OBJECT_SET_NEXT(tobj, next);
}

/////////////////////////////////////////////////
// CELL_FOREIGN_CLASS

// 名前
// 上位クラス(一つ) NULLのときは<foreign-object>

#define FOREIGN_CLASS_GET_NAME(fclass)			(((fclass)+1)->cell)
#define FOREIGN_CLASS_SET_NAME(fclass, name)	(((fclass)+1)->cell=(name))
#define FOREIGN_CLASS_GET_SUPER(fclass)			(((fclass)+2)->cell)
#define FOREIGN_CLASS_SET_SUPER(fclass, super)	(((fclass)+2)->cell=(super))

VM_RET foreign_class_create_(tPVM vm, tPCELL name, tPCELL super_list, tPCELL* cell)
{
	tPCELL super;
	if (super_list) {
		tOBJECT tmp;
		cons_get_car(super_list, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		if (tisl_get_class(vm_get_tisl(vm), vm, OBJECT_GET_CELL(&tmp), &tmp)) return VM_ERROR;
		if (OBJECT_IS_FOREIGN_CLASS(&tmp)) {
			super=OBJECT_GET_CELL(&tmp);
		} else if (OBJECT_IS_BUILT_IN_CLASS(&tmp)&&
			(OBJECT_GET_INTEGER(&tmp)==CLASS_FOREIGN_OBJECT)) {
			super=0;
		} else {
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FOREIGN_CLASS, &tmp);
		}
	} else {
		super=0;
	}
	if (!allocate_cell(vm, sizeof(tCELL)*3, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_FOREIGN_CLASS);
	FOREIGN_CLASS_SET_NAME(*cell, name);
	FOREIGN_CLASS_SET_SUPER(*cell, super);
	return VM_OK;
}

VM_RET foreign_class_create2_(tPVM vm, tPCELL name, tPCELL super, tPCELL* cell)
{
	if (!allocate_cell(vm, sizeof(tCELL)*3, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_FOREIGN_CLASS);
	FOREIGN_CLASS_SET_NAME(*cell, name);
	FOREIGN_CLASS_SET_SUPER(*cell, super);
	return VM_OK;
}

tPCELL foreign_class_get_name(tPCELL fclass)
{
	return FOREIGN_CLASS_GET_NAME(fclass);
}

tPCELL foreign_class_get_super(tPCELL fclass)
{
	return FOREIGN_CLASS_GET_SUPER(fclass);
}

void foreign_class_set_super(tPCELL fclass, tPCELL super)
{
	tPCELL p;
	// loop-check
	for (p=super; p; p=FOREIGN_CLASS_GET_SUPER(p)) {
		if (p==fclass) return;
	}
	FOREIGN_CLASS_SET_SUPER(fclass, super);
}

tUINT foreign_class_get_size(tPCELL fclass)
{
	return CELL_UNIT;
}

VM_RET foreign_class_mark(tPVM vm, tPCELL cell)
{
	return cell_mark(vm, FOREIGN_CLASS_GET_NAME(cell))&&
		   cell_mark(vm, FOREIGN_CLASS_GET_SUPER(cell));
}

VM_RET foreign_class_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT tmp;
	if (write_string(vm, stream, "#i(<foreign-class> name ")) return VM_ERROR;
	OBJECT_SET_SYMBOL(&tmp, FOREIGN_CLASS_GET_NAME(OBJECT_GET_CELL(obj)));
	if (write_object(vm, stream, &tmp)) return VM_ERROR;
	if (FOREIGN_CLASS_GET_SUPER(OBJECT_GET_CELL(obj))) {
		if (write_string(vm, stream, " super ")) return VM_ERROR;
		OBJECT_SET_SYMBOL(&tmp, foreign_class_get_name(foreign_class_get_super(OBJECT_GET_CELL(obj))));
		if (write_object(vm, stream, &tmp)) return VM_ERROR;
		return write_string(vm, stream, ")");
	} else {
		return write_string(vm, stream, " super <foreign-object>)");
	}
}

tBOOL foreign_class_is_subclass(tPCELL fclass, tPCELL super)
{
	tPCELL p;
	for (p=fclass; p; p=FOREIGN_CLASS_GET_SUPER(p)) {
		if (p==super) return tTRUE;
	}
	return tFALSE;
}
