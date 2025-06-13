//
// TISL/src/tisl/translator.h
// TISL Ver. 4.x
//

#include <malloc.h>
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "translator.h"
#include "writer.h"
#include "built_in_object.h"

/////////////////////////////

extern tTRANSLATOR vm_get_translator(tPVM vm);

/////////////////////////////

struct tTRANSLATOR_ {
	// pass_1
	// 局所的な有効範囲を持つ名前参照用リスト
	tPCELL		variable;
	tPCELL		function;
	tPCELL		block;
	tPCELL		tagbody;
	//
	tPCELL		argument_point;
	//
	tINT		form_level;
	tINT		quasiquote_level;
	//
	tPCELL		defining_function_name;
	tPCELL		defining_function_plist;
	// pass 2
	tPCELL		stack;
	tINT		max_sp;
	tINT		sp;
	tINT		defining_function_parameter;
	tBOOL		next_method;
	tINT		method_qualifier;
	// method
	tPCELL		method_flist;
	tPCELL		method_name_list;
	//
	tBOOL		initialization;
	//
	tOBJECT		form;
};

/////////////////////////////

VM_RET translate(tPVM vm, tPOBJECT form, tPCELL* func)
{
	tPCELL code_list;
	tOBJECT obj;
	if (!vm_get_translator(vm)->initialization&&
		translator_initialize(vm)) return VM_ERROR;
	translator_set_form(vm, form);
	// 変換対象の評価形式の保護
	if (vm_push(vm, form)) return VM_ERROR;
	// 変換1 特殊演算子とマクロを展開し，制御情報を抜き出す．
	// また，名前の参照を調べ，存在期間の推論を行う．
	if (translate_pass1(vm, form, &code_list)) { vm_pop(vm); return VM_ERROR; }
	// 生成された中間コードリストの保護
	cell_to_object(code_list, &obj);
	if (vm_push(vm, &obj)) { vm_pop(vm); return VM_ERROR; }
	// 変換2 最終的な実行コードと1対1に対応するコードリストを作成する．
	// 前のパスで推論された存在期間の情報から実行時の変数の位置を決定し，命令を確定する．
	if (translate_pass2(vm, code_list)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	// 別passが必要な最適化処理はここで行う．後で/*!!!*/
	// 変換3 コードリストから関数オブジェクトを作成する．
	if (translate_pass3(vm, code_list, func)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);// code-listの保護の終了
	vm_pop(vm);// formの保護の終了
	function_set_body(*func, form);
	return VM_OK;
}

VM_RET translate_defun(tPVM vm, tPCELL name, tPOBJECT lambda_list, tPOBJECT forms, tPCELL* func)
{
	tPCELL flist;
	tOBJECT obj;
	if (!vm_get_translator(vm)->initialization&&
		translator_initialize(vm)) return VM_ERROR;
	translator_set_form(vm, forms);
	if (translate_pass1_defun(vm, name, lambda_list, forms, &flist)) return VM_ERROR;
	cell_to_object(flist, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (translate_pass2_defun(vm, flist, function_list_get_parameter_number(flist))) { vm_pop(vm); return VM_ERROR; }
#ifdef _DEBUG
/*	{// 変換前中間コードの確認
		tOBJECT obj;
		cell_to_object(name, &obj);
		if (format_object(vm, vm_get_standard_output(vm), &obj)) return VM_ERROR;
		if (format_fresh_line(vm, vm_get_standard_output(vm))) return VM_ERROR;
		cell_to_object(flist, &obj);
		if (format_object(vm, vm_get_standard_output(vm), &obj)) return VM_ERROR;
		if (format_fresh_line(vm, vm_get_standard_output(vm))) return VM_ERROR;
	}*/
#endif
	if (translate_pass3_defun(vm, flist, func)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	function_set_body(*func, forms);
	return VM_OK;
}

VM_RET translate_method(tPVM vm, tPCELL form, tPCELL* method)
{
	tPCELL mlist;
	tOBJECT obj, body;
	cell_to_object(form, &obj);
	translator_set_form(vm, &obj);
	if (!vm_get_translator(vm)->initialization&&
		translator_initialize(vm)) return VM_ERROR;
	if (translate_pass1_method(vm, form, &body, &mlist)) return VM_ERROR;
	cell_to_object(mlist, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (translate_pass2_method(vm, mlist)) { vm_pop(vm); return VM_ERROR; }
	if (translate_pass3_method(vm, mlist, &body, method)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

VM_RET translator_mark(tPVM vm)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	// pass 1
	if (cell_mark(vm, trans->variable)||
		cell_mark(vm, trans->function)||
		cell_mark(vm, trans->block)||
		cell_mark(vm, trans->tagbody)||
		cell_mark(vm, trans->argument_point)||
		cell_mark(vm, trans->defining_function_plist)) return VM_ERROR;
	// pass 2
	if (cell_mark(vm, trans->stack)) return VM_ERROR;
	// method
	if (cell_mark(vm, trans->method_flist)||
		cell_mark(vm, trans->method_name_list)) return VM_ERROR;
	return VM_OK;
}

void translator_set_form(tPVM vm, tPOBJECT form)
{
	vm_get_translator(vm)->form=*form;
}

tPOBJECT translator_get_form(tPVM vm)
{
	return &vm_get_translator(vm)->form;
}


/////////////////////////////
// コードリストへのアクセス

// variable ((size . { stack | heap }) ...)
// function ((n fname1 ... fnamen) function-list1 .. function-listn)
tBOOL list_is_variable(tPCELL list)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_car_cons(list), &obj);
	return OBJECT_IS_CONS(&obj) ? tFALSE : tTRUE;
}

VM_RET head_list_create(tPVM vm, tPCELL* hlist)
{
	tPCELL head;
	tOBJECT obj, obj2;

	if (cons_create_(vm, hlist, &nil, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, *hlist);
	OBJECT_SET_INTEGER(&obj2, 0);
	if (cons_create(vm, &head, &obj2, &obj)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, head);
	cons_set_car(*hlist, &obj);
	return VM_OK;
}

VM_RET head_list_add_object(tPVM vm,tPCELL hlist, tPOBJECT obj)
{
	tPCELL p;
	tOBJECT o;
	if (cons_create(vm, &p, obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&o, p);
	cons_set_cdr(cons_get_cdr_cons(cons_get_car_cons(hlist)), &o);
	cons_set_cdr(cons_get_car_cons(hlist), &o);
	// size++;
	cons_get_car(cons_get_car_cons(hlist), &o);
	OBJECT_SET_INTEGER(&o, OBJECT_GET_INTEGER(&o)+1);
	cons_set_car(cons_get_car_cons(hlist), &o);
	return VM_OK;
}

tINT head_list_get_size(tPCELL hlist)
{
	tOBJECT obj;
	cons_get_car(cons_get_car_cons(hlist), &obj);
	return OBJECT_GET_INTEGER(&obj);
}

// function-list
// (plist clist . function)

VM_RET function_list_create(tPVM vm, tPCELL plist, tPCELL clist, tPCELL* flist)
{
	tOBJECT obj, obj1, obj2;
	tPCELL p;

	if (plist) cell_to_object(plist, &obj1);
	else OBJECT_SET_NIL(&obj1);
	if (clist) cell_to_object(clist, &obj2);
	else OBJECT_SET_NIL(&obj2);
	if (vm_push(vm, &obj1)) return VM_ERROR;
	if (vm_push(vm, &obj2)) { vm_pop(vm); return VM_ERROR; }
	if (cons_create_(vm, &p, &obj2, &nil)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	OBJECT_SET_CONS(&obj, p);
	if (cons_create(vm, flist, &obj1, &obj)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	vm_pop(vm);
	return VM_OK;
}

VM_RET function_list_create_with_plist(tPVM vm, tPCELL plist, tPCELL* flist)
{
	tOBJECT obj, obj1;
	tPCELL p;

	cell_to_object(plist, &obj1);
	if (vm_push(vm, &obj1)) return VM_ERROR;
	if (cons_create_(vm, &p, &nil, &nil)) { vm_pop(vm); return VM_ERROR; }
	OBJECT_SET_CONS(&obj, p);
	if (cons_create(vm, flist, &obj1, &obj)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

tINT function_list_get_parameter_number(tPCELL flist)
{
	return parameter_list_get_number(function_list_get_parameter_list(flist));
}

tPCELL function_list_get_parameter_list(tPCELL flist)
{
	return cons_get_car_cons(flist);
}

tPCELL function_list_get_code_list(tPCELL flist)
{
	return cons_get_car_cons(cons_get_cdr_cons(flist));
}

void function_list_set_code_list(tPCELL flist, tPCELL clist)
{
	tOBJECT obj;
	cell_to_object(clist, &obj);
	cons_set_car(cons_get_cdr_cons(flist), &obj);
}

tPCELL function_list_get_function(tPCELL flist)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_cdr_cons(flist), &obj);
	return OBJECT_GET_CELL(&obj);
}

void function_list_set_function(tPCELL flist, tPCELL function)
{
	tOBJECT obj;
	cell_to_object(function, &obj);
	cons_set_cdr(cons_get_cdr_cons(flist), &obj);
}

// method-list
// (envlist pplist clist . qualifier)

VM_RET method_list_create(tPVM vm, tPCELL pplist, tPCELL clist, tPCELL envlist, const tINT qualifier, tPCELL* mlist)
{
	tOBJECT obj1, obj2, obj3, tmp;
	tPCELL p;
	if (pplist) OBJECT_SET_CONS(&obj1, pplist);
	else OBJECT_SET_NIL(&obj1);
	if (clist) OBJECT_SET_CONS(&obj2, clist);
	else OBJECT_SET_NIL(&obj2);
	if (envlist) OBJECT_SET_CONS(&obj3, envlist);
	else OBJECT_SET_NIL(&obj3);
	if (vm_push(vm, &obj1)) return VM_ERROR;
	if (vm_push(vm, &obj2)) { vm_pop(vm); return VM_ERROR; }
	if (vm_push(vm, &obj3)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	OBJECT_SET_INTEGER(&tmp, qualifier);
	if (cons_create_(vm, &p, &obj2, &tmp)) goto ERROR;
	OBJECT_SET_CONS(&tmp, p);
	if (cons_create(vm, &p, &obj1, &tmp)) goto ERROR;
	OBJECT_SET_CONS(&tmp, p);
	if (cons_create(vm, mlist, &obj3, &tmp)) goto ERROR;
	vm_pop(vm);
	vm_pop(vm);
	vm_pop(vm);
	return VM_OK;
ERROR:
	vm_pop(vm);
	vm_pop(vm);
	vm_pop(vm);
	return VM_ERROR;
}

tPCELL mlist_get_pplist(tPCELL mlist)
{
	return cons_get_car_cons(cons_get_cdr_cons(mlist));
}

tPCELL mlist_get_clist(tPCELL mlist)
{
	return cons_get_car_cons(cons_get_cdr_cons(cons_get_cdr_cons(mlist)));
}

tPCELL mlist_get_env(tPCELL mlist)
{
	return cons_get_car_cons(mlist);
}

tINT mlist_get_qualifier(tPCELL mlist)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_cdr_cons(cons_get_cdr_cons(mlist)), &obj);
	return OBJECT_GET_INTEGER(&obj);
}

// paramter-list
// 作成中 ((size . tail) ....)
// 確定後 ((size . { stack | heap }) ...)
VM_RET parameter_list_create(tPVM vm, tPCELL* plist)
{
	return head_list_create(vm, plist);
}

// 初期化の終了
void parameter_list_finish_initialization(tPCELL plist)
{
	parameter_list_set_stack(plist, 0);
}

void parameter_list_set_rest(tPCELL plist)
{
	tOBJECT obj;
	tINT i;
	cons_get_car(cons_get_car_cons(plist), &obj);
	i=OBJECT_GET_INTEGER(&obj);
	if (i>0) {
		OBJECT_SET_INTEGER(&obj, -i);
		cons_set_car(cons_get_car_cons(plist), &obj);
	}
}

tBOOL parameter_list_is_rest(tPCELL plist)
{
	tOBJECT obj;
	cons_get_car(cons_get_car_cons(plist), &obj);
	return (OBJECT_GET_INTEGER(&obj)<0) ? tTRUE : tFALSE;
}

tPCELL parameter_list_get_name_list(tPCELL plist)
{
	return cons_get_cdr_cons(plist);
}

void parameter_list_set_stack(tPCELL plist, tINT sp)
{
	tOBJECT obj;
	OBJECT_SET_INTEGER(&obj, sp+MAX_PARAMETER_SIZE);
	cons_set_cdr(cons_get_car_cons(plist), &obj);
}

tBOOL parameter_list_is_stack(tPCELL plist)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_car_cons(plist), &obj);
	return (OBJECT_GET_INTEGER(&obj)>=0) ? tTRUE : tFALSE;
}

tINT parameter_list_get_stack(tPCELL plist)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_car_cons(plist), &obj);
	return OBJECT_GET_INTEGER(&obj)-MAX_PARAMETER_SIZE;
}

void parameter_list_set_heap(tPCELL plist)
{
	tOBJECT obj;
	if (parameter_list_is_stack(plist)) {
		cons_get_cdr(cons_get_car_cons(plist), &obj);
		OBJECT_SET_INTEGER(&obj, -OBJECT_GET_INTEGER(&obj));
		cons_set_cdr(cons_get_car_cons(plist), &obj);
	}
}

tBOOL parameter_list_is_heap(tPCELL plist)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_car_cons(plist), &obj);
	return (OBJECT_GET_INTEGER(&obj)<0) ? tTRUE : tFALSE;
}

tINT parameter_list_get_number(tPCELL plist)
{
	tOBJECT obj;
	tINT i;
	cons_get_car(cons_get_car_cons(plist), &obj);
	i=OBJECT_GET_INTEGER(&obj);
	return (i<0) ? -i : i;
}

VM_RET parameter_list_add_parameter(tPVM vm, tPCELL plist, tPOBJECT name)
{
	tOBJECT obj;
	tPCELL p;
	//
	for (p=cons_get_cdr_cons(plist); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &obj);
		if (OBJECT_GET_CELL(&obj)==OBJECT_GET_CELL(name)) {
			// 同じ名前が仮引数リストの中に二つある
			return signal_violation(vm, TRANSLATOR_ERROR_SAME_NAME_PARAMETER, translator_get_form(vm));
		}
	}
	//
	cons_get_cdr(cons_get_car_cons(plist), &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA, translator_get_form(vm));
	return head_list_add_object(vm, plist, name);
}

// parameter-profiler-list

// paramter-list
// 作成中 ((size . tail) ....)
// 確定後 ((size . { stack | heap }) ...)

// ( parameter-lsit . profile-list )
// profile-list : head-list

VM_RET parameter_profiler_list_create(tPVM vm, tPCELL* pplist)
{
	tPCELL plist, slist;
	tOBJECT tmp, tmp2;

	if (parameter_list_create(vm, &plist)) return VM_ERROR;
	cell_to_object(plist, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	if (head_list_create(vm, &slist)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	cell_to_object(slist, &tmp2);
	return cons_create(vm, pplist, &tmp, &tmp2);
}

void pplist_finish_initialization(tPCELL pplist)
{
	parameter_list_finish_initialization(pplist_get_plist(pplist));
}

tPCELL pplist_get_plist(tPCELL pplist)
{
	return cons_get_car_cons(pplist);
}

tPCELL pplist_get_slist(tPCELL pplist)
{
	return cons_get_cdr_cons(pplist);
}

void pplist_set_rest(tPCELL pplist)
{
	parameter_list_set_rest(pplist_get_plist(pplist));
}

tBOOL pplist_is_rest(tPCELL pplist)
{
	return parameter_list_is_rest(pplist_get_plist(pplist));
}

VM_RET pplist_add_profiler(tPVM vm, tPCELL pplist, tPOBJECT name, tPOBJECT specializer)
{
	if (vm_push(vm, name)) return VM_ERROR;
	if (vm_push(vm, specializer)) { vm_pop(vm); return VM_ERROR; }
	if (parameter_list_add_parameter(vm, pplist_get_plist(pplist), name)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	if (head_list_add_object(vm, pplist_get_slist(pplist), specializer)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	vm_pop(vm);
	return VM_OK;
}

tINT pplist_get_number(tPCELL pplist)
{
	return parameter_list_get_number(pplist_get_plist(pplist));
}

tPCELL pplist_get_profiler_list(tPCELL pplist)
{
	return cons_get_cdr_cons(cons_get_cdr_cons(pplist));
}

tPCELL pplist_get_name_list(tPCELL pplist)
{
	return parameter_list_get_name_list(pplist_get_plist(pplist));
}

// code-list

// ((size . tail) .....)

VM_RET code_list_create(tPVM vm, tPCELL* clist)
{
	return head_list_create(vm, clist);
}

VM_RET code_list_add_command_1(tPVM vm, tPCELL clist, const tINT code)
{
	tOBJECT obj;
	OBJECT_SET_INTEGER(&obj, code);
	return head_list_add_object(vm, clist, &obj);
}

VM_RET code_list_add_argument(tPVM vm, tPCELL clist, tPOBJECT obj)
{
	return head_list_add_object(vm, clist, obj);
}

tPCELL code_list_get_head(tPCELL clist)
{
	return cons_get_cdr_cons(clist);
}

void code_list_increment_head(tPCELL* head)
{
	*head=cons_get_cdr_cons(*head);
}

tINT code_list_get_command(tPCELL head)
{
	tOBJECT obj;
	cons_get_car(head, &obj);
	return OBJECT_GET_INTEGER(&obj);
}

tINT code_list_get_max_sp(tPCELL clist)
{
	tOBJECT obj;
	cons_get_cdr(cons_get_car_cons(clist), &obj);
	return OBJECT_GET_INTEGER(&obj);
}

void code_list_set_max_sp(tPCELL clist, tINT max)
{
	tOBJECT obj;
	OBJECT_SET_INTEGER(&obj, max);
	cons_set_cdr(cons_get_car_cons(clist), &obj);
}

// function-name-list
// ((n fname1 ... fnamen) function-list1 .. function-listn)
void function_name_list_set_number(tPCELL flist, const tINT n)
{
	tOBJECT obj;
	OBJECT_SET_INTEGER(&obj, n);
	cons_set_car(cons_get_car_cons(flist), &obj);
}

void function_name_list_increment_number(tPCELL flist)
{
	tOBJECT obj;
	cons_get_car(cons_get_car_cons(flist), &obj);
	OBJECT_SET_INTEGER(&obj, OBJECT_GET_INTEGER(&obj)+1);
	cons_set_car(cons_get_car_cons(flist), &obj);
}

// tagbody_tag_list

void tagbody_tag_list_set_start_tag(tPCELL tlist)
{
	tOBJECT obj;
	cons_get_car(cons_get_car_cons(tlist), &obj);
	if (OBJECT_GET_INTEGER(&obj)>0) {
		OBJECT_SET_INTEGER(&obj, -OBJECT_GET_INTEGER(&obj));
		cons_set_car(cons_get_car_cons(tlist), &obj);
	}
}

tINT tagbody_tag_list_get_size(tPCELL tlist)
{
	tOBJECT obj;
	tINT i;
	cons_get_car(cons_get_car_cons(tlist), &obj);
	i=OBJECT_GET_INTEGER(&obj);
	return (i<0) ? -1-i : i;
}

tBOOL tagbody_tag_list_start_tag(tPCELL tlist)
{
	tOBJECT obj;
	cons_get_car(cons_get_car_cons(tlist), &obj);
	return (OBJECT_GET_INTEGER(&obj)<0) ? tTRUE : tFALSE;
}


/////////////////////////////
// 変換器

VM_RET create_translator(tPVM vm, tTRANSLATOR* translator)
{
	*translator=malloc(sizeof(struct tTRANSLATOR_));
	if (!*translator) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	// pass1
	t1_clear(*translator);
	// pass2
	t2_clear(*translator);
	// method
	(*translator)->method_flist=0;
	(*translator)->method_name_list=0;
	//
	(*translator)->initialization=tFALSE;
	(*translator)->form=unbound;
	return VM_OK;
}

VM_RET translator_initialize(tPVM vm)
{
	tPCELL plist, flist, p;
	tOBJECT tmp, tmp2;
	// flist
	if (parameter_list_create(vm, &plist)) return VM_ERROR;
	parameter_list_finish_initialization(plist);
	if (function_list_create(vm, plist, 0, &flist)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, flist);
	if (cons_create(vm, &p, &tmp, &nil)) return VM_ERROR;
	vm_get_translator(vm)->method_flist=p;
	if (function_list_create(vm, plist, 0, &flist)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, flist);
	if (cons_create(vm, &p, &tmp, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, p);
	cons_set_cdr(vm_get_translator(vm)->method_flist, &tmp);
	// name-list
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_NEXT_METHOD_P);
	if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_CALL_NEXT_METHOD);
	OBJECT_SET_CONS(&tmp2, p);
	if (cons_create(vm, &p, &tmp, &tmp2)) return VM_ERROR;
	vm_get_translator(vm)->method_name_list=p;
	//
	vm_get_translator(vm)->initialization=tTRUE;
	return VM_OK;
}

tBOOL flist_is_call_next_method(tPVM vm, tPCELL flist)
{
	tPCELL p;
	p=cons_get_car_cons(vm_get_translator(vm)->method_flist);
	return (p==flist) ? tTRUE : tFALSE;
}

tBOOL flist_is_next_method_p(tPVM vm, tPCELL flist)
{
	tPCELL p;
	p=cons_get_car_cons(cons_get_cdr_cons(vm_get_translator(vm)->method_flist));
	return (p==flist) ? tTRUE : tFALSE;
}

void free_translator(tTRANSLATOR translator)
{
	free(translator);
}

///////////////////
// pass1

static VM_RET t1_push_name_list(tPVM vm, tPOBJECT obj, tPCELL* list);
static void t1_pop_name_list(tPVM vm, tPCELL* list);

void t1_clear(tTRANSLATOR trans)
{
	// pass 1
	trans->variable=0;
	trans->function=0;
	trans->block=0;
	trans->tagbody=0;
	trans->argument_point=0;
	trans->form_level=0;
	trans->quasiquote_level=0;
	trans->defining_function_name=0;
	trans->defining_function_plist=0;
}

void t1_increment_form_level(tPVM vm)
{
	vm_get_translator(vm)->form_level++;
}

void t1_decrement_form_level(tPVM vm)
{
	vm_get_translator(vm)->form_level--;
}

void t1_increment_quasiquote_level(tPVM vm)
{
	vm_get_translator(vm)->quasiquote_level++;
}

void t1_decrement_quasiquote_level(tPVM vm)
{
	vm_get_translator(vm)->quasiquote_level--;
}

tINT t1_get_quasiquote_level(tPVM vm)
{
	return vm_get_translator(vm)->quasiquote_level;
}

tPCELL t1_get_defining_function_name(tPVM vm)
{
	return vm_get_translator(vm)->defining_function_name;
}

void t1_set_defining_function_name(tPVM vm, tPCELL name)
{
	vm_get_translator(vm)->defining_function_name=name;
}

tPCELL t1_get_defining_function_parameter_list(tPVM vm)
{
	return vm_get_translator(vm)->defining_function_plist;
}

void t1_set_defining_function_parameter_list(tPVM vm, tPCELL plist)
{
	vm_get_translator(vm)->defining_function_plist=plist;
}

tPCELL t1_get_argument_point(tPVM vm)
{
	return vm_get_translator(vm)->argument_point;
}

void t1_set_argument_point(tPVM vm, tPCELL ap)
{
	vm_get_translator(vm)->argument_point=ap;
}

static VM_RET t1_push_name_list(tPVM vm, tPOBJECT obj, tPCELL* list)
{
	tOBJECT rest;
	tPCELL p;
	if (*list) {
		cell_to_object(*list, &rest);
	} else {
		OBJECT_SET_NIL(&rest);
	}
	if (cons_create(vm, &p, obj, &rest)) return VM_ERROR;
	*list=p;
	return VM_OK;
}

static void t1_pop_name_list(tPVM vm, tPCELL* list)
{
	if (*list) {
		*list=cons_get_cdr_cons(*list);
	}
}

// 変数
VM_RET t1_push_variable(tPVM vm, tPCELL parameter_list)
{
	tOBJECT obj;
	cell_to_object(parameter_list, &obj);
	return t1_push_name_list(vm, &obj, &vm_get_translator(vm)->variable);
}

void t1_pop_variable(tPVM vm)
{
	t1_pop_name_list(vm, &vm_get_translator(vm)->variable);
}

tBOOL t1_search_variable(tPVM vm, tPCELL name)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	tPCELL p, plist;
	tOBJECT obj;
	int flag=1;

	for (p=trans->variable; p; p=cons_get_cdr_cons(p)) {
		plist=cons_get_car_cons(p);
		// 自分よりも局所的な変数の参照の場合，flagは負になる
		if ((flag==0)||(plist==trans->argument_point)) flag--;
		if (parameter_list_get_number(plist)) {
			tPCELL pp;
			for (pp=parameter_list_get_name_list(plist); pp; pp=cons_get_cdr_cons(pp)) {
				cons_get_car(pp, &obj);
				if (OBJECT_GET_CELL(&obj)==name) {
					if (flag<0) {
						// 自分よりも局所的な関数によって
						// 参照されたことを記録
						parameter_list_set_heap(plist);
					}
					return tTRUE;
				}
			}
		}
	}
	// 局所的なスコープの中にはこの名前の変数が存在しなかった
	return tFALSE;
}

// 関数
VM_RET t1_push_function(tPVM vm, tPCELL func_list)
{
	tOBJECT obj;
	cell_to_object(func_list, &obj);
	return t1_push_name_list(vm, &obj, &vm_get_translator(vm)->function);
}

void t1_pop_function(tPVM vm)
{
	t1_pop_name_list(vm, &vm_get_translator(vm)->function);
}

// trans->function (((n f-name-1 ... f-name-n) f-list-1 ... f-list-n) ....)
tBOOL t1_search_function(tPVM vm, tPCELL name, tPCELL* flist)
{
	tPCELL p, pp, ppp, pf;
	tOBJECT obj;
	tTRANSLATOR trans=vm_get_translator(vm);
	for (p=trans->function; p; p=cons_get_cdr_cons(p)) {
		pp=cons_get_car_cons(p);
		ppp=cons_get_cdr_cons(cons_get_car_cons(pp));
		pf=cons_get_cdr_cons(pp);
		for (; ppp; ppp=cons_get_cdr_cons(ppp), pf=cons_get_cdr_cons(pf)) {
			cons_get_car(ppp, &obj);
			if (OBJECT_GET_CELL(&obj)==name) {
				*flist=cons_get_car_cons(pf);
				function_name_list_set_referred(pp);
				return tTRUE;
			}
		}
	}
	return tFALSE;
}

tBOOL function_name_list_is_referred(tPCELL flist)
{
	if (flist) {
		tOBJECT obj;
		cons_get_car(cons_get_car_cons(flist), &obj);
		return (OBJECT_GET_INTEGER(&obj)<0) ? tTRUE : tFALSE;
	} else {
		return tFALSE;
	}
}

tINT function_name_list_get_size(tPCELL flist)
{
	tOBJECT obj;
	tINT i;
	cons_get_car(cons_get_car_cons(flist), &obj);
	i=OBJECT_GET_INTEGER(&obj);
	if (i<0) i=-i;
	return i;
}

void function_name_list_set_referred(tPCELL flist)
{
	tOBJECT obj;
	cons_get_car(cons_get_car_cons(flist), &obj);
	if (OBJECT_GET_INTEGER(&obj)>0) {
		OBJECT_SET_INTEGER(&obj, -OBJECT_GET_INTEGER(&obj));
		cons_set_car(cons_get_car_cons(flist), &obj);
	}
}

// block
VM_RET t1_push_block(tPVM vm, tPOBJECT block)
{
	return t1_push_name_list(vm, block, &vm_get_translator(vm)->block);
}

void t1_pop_block(tPVM vm, tPCELL* block_list)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	if (trans->block) {
		*block_list=trans->block;
		trans->block=cons_get_cdr_cons(trans->block);
	}
}

tBOOL t1_search_block(tPVM vm, tPOBJECT name, tPCELL* block_list)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	tPCELL p;
	tOBJECT obj;
	for (p=trans->block; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &obj);
		if (OBJECT_GET_CELL(&obj)==OBJECT_GET_CELL(name)) {
			*block_list=p;
			return tTRUE;
		}
	}
	return tFALSE;
}

// tagbody
VM_RET t1_push_tagbody(tPVM vm, tPCELL tagbodytag)
{
	tOBJECT obj;
	cell_to_object(tagbodytag, &obj);
	return t1_push_name_list(vm, &obj, &vm_get_translator(vm)->tagbody);
}

void t1_pop_tagbody(tPVM vm)
{
	t1_pop_name_list(vm, &vm_get_translator(vm)->tagbody);
}

tBOOL t1_search_tagbody(tPVM vm, tPCELL name, tPCELL* ret)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	tPCELL p, pp;
	tOBJECT obj;
	for (p=trans->tagbody; p; p=cons_get_cdr_cons(p)) {
		pp=cons_get_cdr_cons(cons_get_car_cons(p));
		for (; pp; pp=cons_get_cdr_cons(pp)) {
			cons_get_car(pp, &obj);
			if (OBJECT_GET_CELL(&obj)==name) { 
				*ret=cons_get_car_cons(p);
				return tTRUE;
			}
		}
	}
	return tFALSE;
}

// method-env
VM_RET t1_create_method_env(tPVM vm, tPCELL* env)
{
	tOBJECT tmp, tmp2;
	tPCELL p;
	OBJECT_SET_CONS(&tmp, vm_get_translator(vm)->method_name_list);
	OBJECT_SET_INTEGER(&tmp2, 2);
	if (cons_create_(vm, &p, &tmp2, &tmp)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, p);
	OBJECT_SET_CONS(&tmp2, vm_get_translator(vm)->method_flist);
	return cons_create(vm, env, &tmp, &tmp2);
}

///////////////////
// pass2

void t2_clear(tTRANSLATOR trans)
{
	// pass 2
	trans->stack=0;
	trans->max_sp=0;
	trans->sp=0;
	trans->defining_function_parameter=0;
	trans->next_method=tFALSE;
	trans->method_qualifier=METHOD_PRIMARY;
}

tINT t2_get_defining_function_parameter(tPVM vm)
{
	return vm_get_translator(vm)->defining_function_parameter;
}

void t2_set_defining_function_parameter(tPVM vm, tINT pnum)
{
	vm_get_translator(vm)->defining_function_parameter=pnum;
}

// variable ((n { stack | heap}) name-1 ... name-n)
// function ((n f-name-1 ... f-name-n) f-list-1 ... f-list-n)
VM_RET t2_push_name(tPVM vm, tPCELL list)
{
	tOBJECT obj, cdr;
	tPCELL p;
	tTRANSLATOR trans=vm_get_translator(vm);

	if (list_is_variable(list)) {
		// 変数
		if (parameter_list_is_stack(list)) {
			parameter_list_set_stack(list, t2_get_sp(vm)-parameter_list_get_number(list));
		} else {
			parameter_list_set_stack(list, t2_get_sp(vm)-parameter_list_get_number(list));
			parameter_list_set_heap(list);
		}
	}
	//
	OBJECT_SET_CONS(&obj, list);
	if (trans->stack) {
		OBJECT_SET_CONS(&cdr, trans->stack);
	} else {
		OBJECT_SET_NIL(&cdr);
	}
	if (cons_create(vm, &p, &obj, &cdr)) return VM_ERROR;
	trans->stack=p;
	return VM_OK;
}

void t2_pop_name(tPVM vm)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	if (trans->stack)
		trans->stack=cons_get_cdr_cons(trans->stack);
}

tBOOL t2_search_variable(tPVM vm, tPCELL name, tINT* offset, tBOOL* stack)
{
	tINT heap_offset=0;
	tPCELL p;
	tTRANSLATOR trans=vm_get_translator(vm);
	for (p=trans->stack; p; p=cons_get_cdr_cons(p)) {
		tPCELL list=cons_get_car_cons(p);
		if (list_is_variable(list)) {
			tPCELL pp;
			tINT i;
			tOBJECT obj;
			for (i=0, pp=parameter_list_get_name_list(list); pp; i++, pp=cons_get_cdr_cons(pp)) {
				cons_get_car(pp, &obj);
				if (OBJECT_GET_CELL(&obj)==name) {
					if (parameter_list_is_stack(list)) {
						// stack
						*offset=parameter_list_get_stack(list)+i;
						*stack=tTRUE;
					} else {
						// heap
						*offset=heap_offset+i;
						*stack=tFALSE;
					}
					return tTRUE;
				}
			}
			if (parameter_list_is_heap(list)) {
				heap_offset+=parameter_list_get_number(list);
			}
		} else {//関数
			heap_offset++;//作成時のヒープ環境
		}
	}
	return tFALSE;//例外？
}

tBOOL t2_search_function(tPVM vm, tPCELL name, tPCELL* f, tINT* i)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	tPCELL p;
	*i=0;
	for (p=trans->stack; p; p=cons_get_cdr_cons(p)) {
		tPCELL list=cons_get_car_cons(p);
		if (list_is_variable(list)) {
			// 変数
			if (parameter_list_is_heap(list)) {
				*i+=parameter_list_get_number(list);
			}
		} else {
			// 関数
			tPCELL pp, ppp, pf;
			tOBJECT obj;
			pp=cons_get_car_cons(p);
			ppp=cons_get_cdr_cons(cons_get_car_cons(pp));
			pf=cons_get_cdr_cons(pp);
			for (; ppp; ppp=cons_get_cdr_cons(ppp), pf=cons_get_cdr_cons(pf)) {
				cons_get_car(ppp, &obj);
				if (OBJECT_GET_CELL(&obj)==name) {
					*f=cons_get_car_cons(pf);
					return tTRUE;
				}
			}
			++*i;
		}
	}
	return tFALSE;//例外？
}

// スタックのトレース
VM_RET t2_push_stack(tPVM vm, tPCELL command)
{
	tOBJECT obj;
	tTRANSLATOR trans=vm_get_translator(vm);

	trans->sp++;
	if (trans->sp>trans->max_sp) trans->max_sp=trans->sp;

	OBJECT_SET_CONS(&obj, command);
	return vm_push(vm, &obj);
}

void t2_marge_max(tPVM vm, tINT max)
{
	tTRANSLATOR trans=vm_get_translator(vm);
	if (trans->sp+max>trans->max_sp) trans->max_sp=trans->sp+max;
}

VM_RET t2_pop_stack(tPVM vm, const tINT x)
{
	tINT i;
	tTRANSLATOR trans=vm_get_translator(vm);

	trans->sp-=x;
	for (i=0; i<x; i++) vm_pop(vm);

	return VM_OK;
}

tINT t2_get_sp(tPVM vm)
{
	return vm_get_translator(vm)->sp;
}

void t2_set_sp(tPVM vm, tINT sp)
{
	vm_get_translator(vm)->sp=sp;
}

tINT t2_get_max(tPVM vm)
{
	return vm_get_translator(vm)->max_sp;
}

void t2_set_max(tPVM vm, tINT max)
{
	vm_get_translator(vm)->max_sp=max;
}

// 命令の置き換え
void t2_code_list_set_command(tPCELL* head, tINT command)
{
	tOBJECT obj;
	OBJECT_SET_INTEGER(&obj, command);
	cons_set_car(*head, &obj);
	*head=cons_get_cdr_cons(*head);
}

void t2_increment_head(tPCELL* head)
{
	*head=cons_get_cdr_cons(*head);
}

void t2_code_list_set_operand(tPCELL* head, tPOBJECT operand)
{
	cons_set_car(*head, operand);
	*head=cons_get_cdr_cons(*head);
}

// 命令の引数の取得
void t2_get_operand(tPCELL head, const tINT x, tPOBJECT operand)
{
	tPCELL p;
	tINT i;
	for (i=0, p=head; i<x; i++, p=cons_get_cdr_cons(p));
	cons_get_car(p, operand);
}

void t2_code_list_close(tPCELL head)
{
	cons_set_cdr(head, &nil);
}

void t2_set_next_method(tPVM vm)
{
	vm_get_translator(vm)->next_method=tTRUE;
}

void t2_reset_next_method(tPVM vm)
{
	vm_get_translator(vm)->next_method=tFALSE;
}

tBOOL t2_use_next_method(tPVM vm)
{
	return vm_get_translator(vm)->next_method;
}

void t2_set_method_qualifier(tPVM vm, tINT qualifier)
{
	vm_get_translator(vm)->method_qualifier=qualifier;
}

tINT t2_get_method_qualifier(tPVM vm)
{
	return vm_get_translator(vm)->method_qualifier;
}

///////////////////
// pass3

void t3_clear(tTRANSLATOR trans)
{
}

