//
// TISL/src/tisl/translator_2.c
// TISL Ver. 4.x
//

#define TISL_TRANSALTOR_2_C
#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "translator.h"
#include "command.h"

extern tTRANSLATOR vm_get_translator(tPVM vm);

// 実行可能な命令列と1対1に対応できる命令のリストを作成する．
// 実引数の実行時の位置を決定し，命令を確定する．

/////////////////////////////

// コードリストの変換
static VM_RET t2_code_list(tPVM vm, tPCELL clist);
static VM_RET t2_function_list(tPVM vm, tPCELL flist);

/////////////////////////////

VM_RET translate_pass2(tPVM vm, tPCELL code_list)
{
	VM_RET ret;
	t2_clear(vm_get_translator(vm));
	ret=t2_code_list(vm, code_list);
	t2_clear(vm_get_translator(vm));
	return ret;
}

VM_RET translate_pass2_defun(tPVM vm, tPCELL flist, tINT pnum)
{
	VM_RET ret;
	t2_clear(vm_get_translator(vm));
	t2_set_defining_function_parameter(vm, pnum);
	ret=t2_function_list(vm, flist);
	t2_clear(vm_get_translator(vm));
	return ret;
}

VM_RET translate_pass2_method(tPVM vm, tPCELL mlist)
{
	t2_clear(vm_get_translator(vm));
	// 引数
	if (t2_push_name(vm, pplist_get_plist(mlist_get_pplist(mlist)))) goto ERROR;
	switch (mlist_get_qualifier(mlist)) {
	case METHOD_AROUND:
	case METHOD_PRIMARY:
		// call-next-method と next-method-p
		if (function_name_list_is_referred(mlist_get_env(mlist))) {
			t2_set_next_method(vm);
			if (t2_push_name(vm, mlist_get_env(mlist))) goto ERROR;
		} else {
			t2_reset_next_method(vm);
		}
	case METHOD_BEFORE:
	case METHOD_AFTER:
		t2_set_method_qualifier(vm, mlist_get_qualifier(mlist));
		if (t2_code_list(vm, mlist_get_clist(mlist))) goto ERROR;
		t2_clear(vm_get_translator(vm));
		return VM_OK;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
ERROR:
	t2_clear(vm_get_translator(vm));
	return VM_ERROR;
}

/////////////////////////////

static VM_RET t2_code_list(tPVM vm, tPCELL clist)
{
	tPCELL head;
	tINT code, max;
	tINT sp;

	sp=t2_get_sp(vm);
	max=t2_get_max(vm);
	head=code_list_get_head(clist);
	t2_set_max(vm, sp);
	while (code=code_list_get_command(head),
		  !(*t2_table[code].translate)(vm, code, &head));
	code_list_set_max_sp(clist, t2_get_max(vm)-sp);
	t2_set_sp(vm, sp);
	t2_set_max(vm, max);
	//
	return vm_last_condition_is_ok(vm) ? VM_OK : VM_ERROR;
}

static VM_RET t2_function_list(tPVM vm, tPCELL flist)
{
	tPCELL plist, clist;
	tINT sp, max;

	plist=function_list_get_parameter_list(flist);
	clist=function_list_get_code_list(flist);
	sp=t2_get_sp(vm);
	max=t2_get_max(vm);
	t2_set_sp(vm, 0);
	t2_set_max(vm, 0);
	if (t2_push_name(vm, plist)) return VM_ERROR;
	if (t2_code_list(vm, clist)) return VM_ERROR;
	t2_pop_name(vm);
	t2_set_sp(vm, sp);
	t2_set_max(vm, max);
	return VM_OK;
}

/////////////////////////////

static VM_RET t2_op_n(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT n;
	tINT code;
	// 引数の取得
	t2_get_operand(h, 1, &n);
	// 新しい命令に書き換え
	switch (id) {
	case iSET_AREF:		code=iiSET_AREF; break;
	case iSET_GAREF:	code=iiSET_GAREF; break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	t2_code_list_set_operand(head, &n);
	//
	t2_pop_stack(vm, OBJECT_GET_INTEGER(&n));
	return t2_push_stack(vm, h);
}

// iDISCARD
static VM_RET t2_discard(tPVM vm, const tINT id, tPCELL* head)
{
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiDISCARD);
	// スタックの値を一つ捨てる
	t2_pop_stack(vm, 1);
	return VM_OK;
}

// iPUSH_NIL 
// iPUSH_T
static VM_RET t2_push_nil(tPVM vm, const tINT id, tPCELL* head)
{
	tINT code;
	tPCELL p=*head;

	// 新しい命令に置き換え
	code=(id==iPUSH_NIL) ? iiPUSH_NIL : iiPUSH_T;
	t2_code_list_set_command(head, code);
	// スタックに値を一つ積む
	return t2_push_stack(vm, p);
}

// iPUSH_OBJECT obj
static VM_RET t2_push_object(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL p=*head;
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiPUSH_OBJECT);
	t2_increment_head(head);// obj
	// スタックに値を一つ積む
	return t2_push_stack(vm, p);
}

// iPUSH_LOCAL_VARIABVLE symbol
static VM_RET t2_push_local_variable(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL p=*head;
	tOBJECT name, operand;
	tBOOL stack;
	tINT code, offset;
	// 引数の取得
	t2_get_operand(*head, 1, &name);


	// 新しい命令に置き換え
	if (!t2_search_variable(vm, OBJECT_GET_CELL(&name), &offset, &stack))
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (stack) {
		code=iiPUSH_STACK;
		OBJECT_SET_INTEGER(&operand, offset-t2_get_sp(vm));
	} else {
		code=iiPUSH_HEAP;
		OBJECT_SET_INTEGER(&operand, offset);
	}
	t2_code_list_set_command(head, code);
	t2_code_list_set_operand(head, &operand);
	//
	// スタックに値を一つ積む
	return t2_push_stack(vm, p);
}

// iiPUSH_VARIABLE bind-list
static VM_RET t2_push_global_variable(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL blist, h=*head;
	tOBJECT name, operand;
	// 引数の取得
	t2_get_operand(*head, 1, &name);
	// 新しい命令に置き換え
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&name), &blist)) return VM_ERROR;
	OBJECT_SET_BIND_LIST(&operand, blist);
	t2_code_list_set_command(head, iiPUSH_VARIABLE);
	t2_code_list_set_operand(head, &operand);
	// スタックに値を一つ積む
	return t2_push_stack(vm, h);
}

// iCALL_REC
static VM_RET t2_call_rec(tPVM vm, const tINT id, tPCELL* head)
{// defunの中にしかないはず
	tPCELL h=*head;
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiCALL_REC);
	//引数の消費
	t2_pop_stack(vm, t2_get_defining_function_parameter(vm));
	return t2_push_stack(vm, h);
}

// iiCALL_TAIL_REC
static VM_RET t2_call_tail_rec(tPVM vm, const tINT id, tPCELL* head)
{// defunの中にしかないはず
	tPCELL h=*head;
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiCALL_TAIL_REC);
	// 引数の消費
	t2_pop_stack(vm, t2_get_defining_function_parameter(vm));
	// 戻り値一つ
	t2_push_stack(vm, h);
	// この命令より後ろは変換する必要がない．
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiCALL bind-list anum
static VM_RET t2_call_global(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT blist, anum;
	// 引数の取得
	t2_get_operand(*head, 1, &blist);
	t2_get_operand(*head, 2, &anum);
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiCALL);
	t2_code_list_set_operand(head, &blist);
	t2_code_list_set_operand(head, &anum);
	// 引数の消費
	t2_pop_stack(vm, OBJECT_GET_INTEGER(&anum));
	// 戻り値一つ
	return t2_push_stack(vm, h);
}

// iiCALL_TAIL bind-list anum
static VM_RET t2_call_tail_global(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT blist, anum;
	// 引数の取得
	t2_get_operand(*head, 1, &blist);
	t2_get_operand(*head, 2, &anum);
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiCALL_TAIL);
	t2_code_list_set_operand(head, &blist);
	t2_code_list_set_operand(head, &anum);
	// 引数の消費
	t2_pop_stack(vm, OBJECT_GET_INTEGER(&anum));
	// 戻り値一つ
	t2_push_stack(vm, h);
	// この命令より後ろは変換する必要がない．
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t2_call_bind(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT bind, anum;
	// 引数の取得
	t2_get_operand(*head, 1, &bind);
	t2_get_operand(*head, 2, &anum);
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiCALL_BIND);
	t2_code_list_set_operand(head, &bind);
	t2_code_list_set_operand(head, &anum);
	// 引数の消費
	t2_pop_stack(vm, OBJECT_GET_INTEGER(&anum));
	// 戻り値一つ
	return t2_push_stack(vm, h);
}

static VM_RET t2_call_tail_bind(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT bind, anum;
	// 引数の取得
	t2_get_operand(*head, 1, &bind);
	t2_get_operand(*head, 2, &anum);
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiCALL_TAIL_BIND);
	t2_code_list_set_operand(head, &bind);
	t2_code_list_set_operand(head, &anum);
	// 引数の消費
	t2_pop_stack(vm, OBJECT_GET_INTEGER(&anum));
	// 戻り値一つ
	t2_push_stack(vm, h);
	// この命令より後ろは変換する必要がない．
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiCALL_LOCAL offset flist anum
static VM_RET t2_call_local(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL dummy, h=*head;
	tOBJECT name, flist, offset;
	tINT i, pnum;
	// 引数の取得
	t2_get_operand(*head, 1, &name);
	t2_get_operand(*head, 2, &flist);
	// 新しい命令に置き換え
	if (!t2_search_function(vm, OBJECT_GET_CELL(&name), &dummy, &i))
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	OBJECT_SET_INTEGER(&offset, i);
	t2_code_list_set_command(head, iiCALL_LOCAL);
	t2_code_list_set_operand(head, &offset);
	t2_code_list_set_operand(head, &flist);
	// 引数の消費
	pnum=function_list_get_parameter_number(OBJECT_GET_CELL(&flist));
	t2_pop_stack(vm, pnum);
	// 戻り値一つ
	return t2_push_stack(vm, h);
}

// iiCALL_LOCAL_TAIL offset flist 
static VM_RET t2_call_tail_local(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL dummy, h=*head;
	tOBJECT name, flist, offset;
	tINT i, pnum;
	// 引数の取得
	t2_get_operand(*head, 1, &name);
	t2_get_operand(*head, 2, &flist);
	// 新しい命令に置き換え
	if (!t2_search_function(vm, OBJECT_GET_CELL(&name), &dummy, &i))
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	OBJECT_SET_INTEGER(&offset, i);
	t2_code_list_set_command(head, iiCALL_LOCAL_TAIL);
	t2_code_list_set_operand(head, &offset);
	t2_code_list_set_operand(head, &flist);
	// 引数の消費
	pnum=function_list_get_parameter_number(OBJECT_GET_CELL(&flist));
	t2_pop_stack(vm, pnum);
	// 戻り値一つ
	t2_push_stack(vm, h);
	// ここから後ろは変換する必要がない
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiRET
static VM_RET t2_ret(tPVM vm, const tINT id, tPCELL* head)
{
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiRET);
	// 終わり
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

// iiLAMBDA_IN plist | iiLAMBDA_HEAP_IN plist
static VM_RET t2_lambda_in(tPVM vm, const tINT id, tPCELL* head)
{
	tOBJECT plist;
	tINT code;
	// 引数の取得
	t2_get_operand(*head, 1, &plist);
	// 仮引数リストの登録
	if (t2_push_name(vm, OBJECT_GET_CELL(&plist))) return VM_ERROR;
	// 新しい命令に置き換え
	code=parameter_list_is_stack(OBJECT_GET_CELL(&plist)) ? iiLAMBDA_IN : iiLAMBDA_HEAP_IN;
	t2_code_list_set_command(head, code);
	t2_code_list_set_operand(head, &plist);
	// スタックは動かない
	return VM_OK;
}

// iiLAMBDA_OUT plist | iiLAMBDA_HEAP_OUT plist
static VM_RET t2_lambda_out(tPVM vm, const tINT id, tPCELL* head)
{
	tOBJECT plist;
	tPCELL h=*head;
	tINT code;
	// 引数の取得
	t2_get_operand(*head, 1, &plist);
	// 仮引数リストの削除
	t2_pop_name(vm);
	// 新しい命令に置き換え
	code=parameter_list_is_stack(OBJECT_GET_CELL(&plist)) ? iiLAMBDA_OUT : iiLAMBDA_HEAP_OUT;
	t2_code_list_set_command(head, code);
	t2_code_list_set_operand(head, &plist);
	//引数下げる
	t2_pop_stack(vm, parameter_list_get_number(OBJECT_GET_CELL(&plist)));
	// 後処理のために切る
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

// iiPUSH_FUNCTON bind-list
static VM_RET t2_push_function(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL blist, h=*head;
	tOBJECT name, obj;
	// 引数の取得
	t2_get_operand(*head, 1, &name);
	// 新しい命令に書き換え
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&name), &blist)) return VM_ERROR;
	OBJECT_SET_BIND_LIST(&obj, blist);
	t2_code_list_set_command(head, iiPUSH_FUNCTION);
	t2_code_list_set_operand(head, &obj);
	// 戻り値一つ
	return t2_push_stack(vm, h);
}

// iiPUSH_LOCAL_FUNCTION offset flist
static VM_RET t2_push_local_function(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL dummy, h=*head;
	tOBJECT name, flist, offset;
	tINT i;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	t2_get_operand(h, 2, &flist);
	// 新しい命令に書き換え
	if (!t2_search_function(vm, OBJECT_GET_CELL(&name), &dummy, &i))
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	OBJECT_SET_INTEGER(&offset, i);
	t2_code_list_set_command(head, iiPUSH_LOCAL_FUNCTION);
	t2_code_list_set_operand(head, &offset);
	t2_code_list_set_operand(head, &flist);
	// 戻り値一つ
	return t2_push_stack(vm, h);
}

// iiPUSH_LAMBDA flist
static VM_RET t2_push_lambda(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT flist;
	// 引数の取得
	t2_get_operand(h, 1, &flist);
	// 新しい命令に書き換え
	if (t2_function_list(vm, OBJECT_GET_CELL(&flist))) return VM_ERROR;
	t2_code_list_set_command(head, iiPUSH_LAMBDA);
	t2_code_list_set_operand(head, &flist);
	// 戻り値一つ
	return t2_push_stack(vm, h);
}

// iiLABELS_IN nlist function-list_1 ... function-list_n
static VM_RET t2_labels_in(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL fnl;
	tOBJECT nlist;
	tINT i, n;
	//
	fnl=cons_get_cdr_cons(*head);
	// 引数の取得
	t2_get_operand(*head, 1, &nlist);
	// 新しい関数名の追加
	if (t2_push_name(vm, fnl)) return VM_ERROR;
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiLABELS_IN);
	t2_code_list_set_operand(head, &nlist);
	n=function_name_list_get_size(fnl);
	for (i=0; i<n; i++) {
		tOBJECT flist;
		cons_get_car(*head, &flist);
		if (t2_function_list(vm, OBJECT_GET_CELL(&flist))) return VM_ERROR;
		t2_code_list_set_operand(head, &flist);
	}
	// スタックは動かず
	return VM_OK;
}

// iiLABELS_OUT
static VM_RET t2_labels_out(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tINT code;
	// 関数の名前の削除
	t2_pop_name(vm);
	// 新しい命令に書き換え
	code=(id==iLABELS_OUT) ? iiLABELS_OUT : iiFLET_OUT;
	t2_code_list_set_command(head, code);
	// スタックは動かず
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_flet_in(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL fnl;
	tOBJECT nlist;
	tINT i, n;
	//
	fnl=cons_get_cdr_cons(*head);
	// 引数の取得
	t2_get_operand(*head, 1, &nlist);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiFLET_IN);
	t2_code_list_set_operand(head, &nlist);
	n=function_name_list_get_size(fnl);
	for (i=0; i<n; i++) {
		tOBJECT flist;
		cons_get_car(*head, &flist);
		if (t2_function_list(vm, OBJECT_GET_CELL(&flist))) return VM_ERROR;
		t2_code_list_set_operand(head, &flist);
	}
	// 関数名前の登録
	if (t2_push_name(vm, fnl)) return VM_ERROR;
	// スタックは動かず
	return VM_OK;
}

// iiADN clist
static VM_RET t2_and(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT cl;
	//　引数の取得
	t2_get_operand(*head, 1, &cl);
	//スタックが一つ下がった状態でformの実行時はいる
	t2_pop_stack(vm, 1);
	// 新しい命令に書き換え
	if (t2_code_list(vm, OBJECT_GET_CELL(&cl))) return VM_ERROR;
	t2_code_list_set_command(head, iiAND);
	t2_code_list_set_operand(head, &cl);
	// 
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&cl)));
	//スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_or(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT cl;
	// 引数の取得
	t2_get_operand(h, 1, &cl);
	// スタックが一つ下がった状態でformの実行に入る
	t2_pop_stack(vm, 1);
	// 新しい命令に書き換え
	if (t2_code_list(vm, OBJECT_GET_CELL(&cl))) return VM_ERROR;
	t2_code_list_set_command(head, iiOR);
	t2_code_list_set_operand(head, &cl);
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&cl)));
	// スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_set_local_variable(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT name, offset;
	tINT code, i;
	tBOOL stack;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	if (!t2_search_variable(vm, OBJECT_GET_CELL(&name), &i, &stack))
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (stack) {
		code=iiSET_STACK;
		OBJECT_SET_INTEGER(&offset, i-t2_get_sp(vm)+1);
	} else {
		code=iiSET_HEAP;
		OBJECT_SET_INTEGER(&offset, i);
	}
	t2_code_list_set_command(head, code);
	t2_code_list_set_operand(head, &offset);
	//
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_set_global_variable(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL blist, h=*head;
	tOBJECT name, obj;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&name), &blist)) return VM_ERROR;
	OBJECT_SET_BIND_LIST(&obj, blist);
	t2_code_list_set_command(head, iiSET_VARIABLE);
	t2_code_list_set_operand(head, &obj);
	//
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_set_dynamic(tPVM vm, const tINT id, tPCELL* head)
{
	tOBJECT name;
	tPCELL h=*head;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiSET_DYNAMIC);
	t2_code_list_set_operand(head, &name);
	//
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_set_elt(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tINT code;
	// 引数無し
	// 新しい命令に書き換え
	switch (id) {
	case iSET_ELT:		code=iiSET_ELT; break;
	case iSET_PROPERTY:	code=iiSET_PROPERTY; break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	//
	t2_pop_stack(vm, 3);
	return t2_push_stack(vm, h);
}

static VM_RET t2_set_car(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tINT code;
	// 引数無し
	// 新しい命令に書き換え
	switch (id) {
	case iSET_CAR:		code=iiSET_CAR; break;
	case iSET_CDR:		code=iiSET_CDR; break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	//
	t2_pop_stack(vm, 2);
	return t2_push_stack(vm, h);
}

static VM_RET t2_accessor(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT name;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiACCESSOR);
	t2_code_list_set_operand(head, &name);
	// スタック
	t2_pop_stack(vm, 2);
	return t2_push_stack(vm, h);
}

static VM_RET t2_push_dynamic(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT name;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiPUSH_DYNAMIC);
	t2_code_list_set_operand(head, &name);
	// スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_dynamic_let(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT N, clist;
	tINT i, n;
	// 引数の取得
	t2_get_operand(h, 1, &N);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiDYNAMIC_LET);
	t2_code_list_set_operand(head, &N);
	n=OBJECT_GET_INTEGER(&N);
	for (i=0; i<n; i++) t2_increment_head(head);
	cons_get_car(*head, &clist);
	for (i=0; i<n; i++) {
		if (t2_push_stack(vm, h)) return VM_ERROR;
	}
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_code_list_set_operand(head, &clist);
	// スタック
	t2_pop_stack(vm, n*2);
	return t2_push_stack(vm, h);
}

static VM_RET t2_if(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT then_list, else_list;
	// 引数の取得
	t2_get_operand(h, 1, &then_list);
	t2_get_operand(h, 2, &else_list);
	// スタックが一つ下がった状態でthenまたはelseの実行に入る
	t2_pop_stack(vm, 1);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiIF);
	if (t2_code_list(vm, OBJECT_GET_CELL(&then_list))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&then_list)));
	t2_increment_head(head);
	if (t2_code_list(vm, OBJECT_GET_CELL(&else_list))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&else_list)));
	t2_increment_head(head);
	// スタック
	return t2_push_stack(vm, h);
}

// iiCASE n key-list1 clist1 ... key-listn clistn
static VM_RET t2_case(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT N, keylist, clist;
	tINT i, n, m, code;
	// 引数の取得
	t2_get_operand(h, 1, &N);
	// 新しい命令に書き換え
	switch (id) {
	case iCASE:			code=iiCASE;		break;
	case iCASE_USING:	code=iiCASE_USING;	break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);	// iiCASE
	t2_increment_head(head);				// N
	n=OBJECT_GET_INTEGER(&N);
	for (i=0; i<n; i++) {
		cons_get_car(*head, &keylist);
		m=OBJECT_GET_INTEGER(&keylist);
		t2_increment_head(head);			// keylist
		cons_get_car(*head, &clist);
		if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
		t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
		t2_increment_head(head);// clist
	}
	// スタック
	switch (id) {
	case iCASE:			i=1; break;
	case iiCASE_USING:	i=2; break;
	}
	t2_pop_stack(vm, i);
	return t2_push_stack(vm, h);
}

static VM_RET t2_while(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT test, body;
	// 引数の取得
	t2_get_operand(h, 1, &test);
	t2_get_operand(h, 2, &body);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiWHILE);
	if (t2_code_list(vm, OBJECT_GET_CELL(&test))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&test)));
	t2_increment_head(head);
	if (t2_code_list(vm, OBJECT_GET_CELL(&body))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&body)));
	t2_increment_head(head);
	// スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_for(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT plist, end, result, iteration;
	tINT code;
	// 引数の取得
	t2_get_operand(h, 1, &plist);
	t2_get_operand(h, 2, &end);
	t2_get_operand(h, 3, &result);
	t2_get_operand(h, 4, &iteration);
	// 新しい命令に書き換え
	code=parameter_list_is_stack(OBJECT_GET_CELL(&plist)) ? iiFOR_STACK : iiFOR_HEAP;
	t2_code_list_set_command(head, code);	// iiFOR
	t2_increment_head(head);				// plist
	// 名前の追加
	if (t2_push_name(vm, OBJECT_GET_CELL(&plist))) return VM_ERROR;
	if (t2_code_list(vm, OBJECT_GET_CELL(&end))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&end)));
	t2_increment_head(head);				// test
	if (t2_code_list(vm, OBJECT_GET_CELL(&result))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&result)));
	t2_increment_head(head);				// result
	if (t2_code_list(vm, OBJECT_GET_CELL(&iteration))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&iteration)));
	t2_increment_head(head);				// iteration
	// 名前の削除
	t2_pop_name(vm);
	// スタック
	t2_pop_stack(vm, parameter_list_get_number(OBJECT_GET_CELL(&plist)));
	return t2_push_stack(vm, h);
}

static VM_RET t2_block(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT clist, tag;
	// 引数の取得
	t2_get_operand(h, 1, &clist);
	t2_get_operand(h, 2, &tag);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiBLOCK);
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_increment_head(head);
	t2_increment_head(head);
	// スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_return_from(tPVM vm, const tINT id, tPCELL* head)
{
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiRETURN_FROM);
	t2_increment_head(head);
	// この命令の後ろに制御が移ることはない
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t2_catch(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT clist;
	// 引数の取得
	t2_get_operand(h, 1, &clist);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiCATCH);
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_increment_head(head);
	// スタック
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_throw(tPVM vm, const tINT id, tPCELL* head)
{
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiTHROW);
	// この命令の後ろに制御が移ることはない
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t2_tagbody(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL p, h=*head;
	tOBJECT clist_list, clist;
	// 引数の取得
	t2_get_operand(h, 2, &clist_list);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiTAGBODY);
	t2_increment_head(head);
	t2_increment_head(head);
	for (p=OBJECT_GET_CELL(&clist_list); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &clist);
		if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
		t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	}
	// スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_go(tPVM vm, const tINT id, tPCELL* head)
{
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiGO);
	t2_increment_head(head);
	// この命令の後ろに制御が移ることはない
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t2_unwind_protect(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT form, cleanup;
	// 引数の取得
	t2_get_operand(h, 1, &form);
	t2_get_operand(h, 2, &cleanup);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, iiUNWIND_PROTECT);
	if (t2_code_list(vm, OBJECT_GET_CELL(&form))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&form)));
	t2_increment_head(head);
	if (t2_code_list(vm, OBJECT_GET_CELL(&cleanup))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&cleanup)));
	t2_increment_head(head);
	if (t2_push_stack(vm, h)) return VM_ERROR;
	//
	return VM_OK;
}

static VM_RET t2_push_class(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL blist, h=*head;
	tOBJECT name, obj;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&name), &blist)) return VM_ERROR;
	OBJECT_SET_BIND_LIST(&obj, blist);
	t2_code_list_set_command(head, iiPUSH_CLASS);
	t2_code_list_set_operand(head, &obj);
	//スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_the(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL blist, h=*head;
	tOBJECT name, obj;
	tINT code;
	// 引数の取得
	t2_get_operand(h, 1, &name);
	// 新しい命令に書き換え
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&name), &blist)) return VM_ERROR;
	OBJECT_SET_BIND_LIST(&obj, blist);
	switch (id) {
	case iTHE:		code=iiTHE; break;
	case iASSURE:	code=iiASSURE; break;
	case iCONVERT:	code=iiCONVERT; break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	t2_code_list_set_operand(head, &obj);
	// スタック
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_with_standard_stream(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT clist;
	tINT code;
	// 引数の取得
	t2_get_operand(h, 1, &clist);
	// 新しい命令に置き換え
	switch (id) {
	case iWITH_STANDARD_INPUT:	code=iiWITH_STANDARD_INPUT;		break;
	case iWITH_STANDARD_OUTPUT:	code=iiWITH_STANDARD_OUTPUT;	break;
	case iWITH_ERROR_OUTPUT:		code=iiWITH_ERROR_OUTPUT;	break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_increment_head(head);
	// スタック
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_with_open_file(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT clist, plist;
	tINT code;
	// 引数の取得
	t2_get_operand(h, 1, &plist);
	t2_get_operand(h, 2, &clist);
	// 新しい命令に置き換え
	switch (id) {
	case iWITH_OPEN_INPUT_FILE:		code=iiWITH_OPEN_INPUT_FILE;	break;
	case iWITH_OPEN_OUTPUT_FILE:	code=iiWITH_OPEN_OUTPUT_FILE;	break;
	case iWITH_OPEN_IO_FILE:		code=iiWITH_OPEN_IO_FILE;		break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	t2_increment_head(head);
	t2_pop_stack(vm, 2);
	if (t2_push_stack(vm, h)) return VM_ERROR;// 命令が違うopen-input-file？
	// 名前の登録
	if (t2_push_name(vm, OBJECT_GET_CELL(&plist))) return VM_ERROR;
	// スタックを二つ消費し，ストリームを一つ積んだ状態で
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_increment_head(head);
	// 名前の削除
	t2_pop_name(vm);
	// スタック　
	// 最後に一つさげて戻り値つむからこのままにしておこう
	return VM_OK;
}

static VM_RET t2_ignore_errors(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT clist;
	tINT code;
	switch (id) {
	case iIGNORE_ERRORS:	code=iiIGNORE_ERRORS;	break;
	case iTIME:				code=iiTIME;			break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	// 引数の取得
	t2_get_operand(h, 1, &clist);
	// 新しい命令に置き換え
	t2_code_list_set_command(head, code);
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_increment_head(head);
	// スタック
	return t2_push_stack(vm, h);
}

static VM_RET t2_continue_condition(tPVM vm, const tINT id, tPCELL* head)
{
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiCONTINUE_CONDITION);
	// この命令の後ろに制御が移ることはない
	t2_code_list_close(*head);
	vm_set_last_condition_ok(vm);
	return VM_ERROR;
}

static VM_RET t2_with_handler(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT clist;
	// 引数の取得
	t2_get_operand(h, 1, &clist);
	// 新しい命令に置き換え
	t2_code_list_set_command(head, iiWITH_HANDLER);
	if (t2_code_list(vm, OBJECT_GET_CELL(&clist))) return VM_ERROR;
	t2_marge_max(vm, code_list_get_max_sp(OBJECT_GET_CELL(&clist)));
	t2_increment_head(head);
	// スタック
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_op_1(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tINT code;
	switch (id) {
	case iQUASIQUOTE:			code=iiQUASIQUOTE;			break;
	case iUNQUOTE:				code=iiUNQUOTE;				break;
	case iUNQUOTE_SPLICING:		code=iiUNQUOTE_SPLICING;	break;
	case iUNQUOTE_SPLICING2:	code=iiUNQUOTE_SPLICING2;	break;
	case iARITY_ERROR:			code=iiARITY_ERROR;			break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	// スタック
	t2_pop_stack(vm, 1);
	return t2_push_stack(vm, h);
}

static VM_RET t2_op_2(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tINT code;
	switch (id) {
	case iQUASIQUOTE2:				code=iiQUASIQUOTE2;	break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	t2_code_list_set_command(head, code);
	// スタック
	t2_pop_stack(vm, 2);
	return t2_push_stack(vm, h);
}

static VM_RET t2_primitive(tPVM vm, const tINT id, tPCELL* head)
{
	tPCELL h=*head;
	tOBJECT n;
	// 引数の取得
	t2_get_operand(h, 1, &n);
	// 新しい命令に書き換え
	t2_code_list_set_command(head, t2_table[id].t2_data);
	t2_code_list_set_operand(head, &n);
	//
	t2_pop_stack(vm, OBJECT_GET_INTEGER(&n));
	return t2_push_stack(vm, h);
}

static VM_RET t2_primitive_n(tPVM vm, const tINT id, tPCELL* head, const tINT m)
{
	tPCELL h=*head;
	tOBJECT n;
	t2_get_operand(h, 1, &n);
	if (!OBJECT_IS_INTEGER(&n)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (OBJECT_GET_INTEGER(&n)==m) {
		tOBJECT tmp;
		OBJECT_SET_INTEGER(&tmp, t2_table[id].t2_data);
		cons_set_car(*head, &tmp);
		cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
		cons_set_cdr(*head, &tmp);
		*head=cons_get_cdr_cons(*head);
		t2_pop_stack(vm, m);
		return t2_push_stack(vm, h);
	} else {
		t2_code_list_set_command(head, iiARITY_ERROR);
		// この命令の後ろに制御が移ることはない
		t2_code_list_close(*head);
		vm_set_last_condition_ok(vm);
		return VM_ERROR;
	}
}

static VM_RET t2_primitive_0(tPVM vm, const tINT id, tPCELL* head)
{
	return t2_primitive_n(vm, id, head, 0);
}

static VM_RET t2_primitive_1(tPVM vm, const tINT id, tPCELL* head)
{
	return t2_primitive_n(vm, id, head, 1);
}

static VM_RET t2_primitive_2(tPVM vm, const tINT id, tPCELL* head)
{
	return t2_primitive_n(vm, id, head, 2);
}

static VM_RET t2_primitive_3(tPVM vm, const tINT id, tPCELL* head)
{
	return t2_primitive_n(vm, id, head, 3);
}

static VM_RET op_stack_integer(tPVM vm, const tINT command1, tPCELL* head)
{
	tPCELL h1, h2;
	tOBJECT offset, i, tmp;
	// iiPUSH_STACK offset
	// iiPUSH_OBJECT i
	// com
	//
	// com_STACK_INGEGER offset+1 i
	h1=OBJECT_GET_CELL(vm->SP-1);
	h2=OBJECT_GET_CELL(vm->SP);
	cons_get_car(cons_get_cdr_cons(h1), &offset);
	cons_get_car(cons_get_cdr_cons(h2), &i);
	OBJECT_SET_INTEGER(&offset, OBJECT_GET_INTEGER(&offset)+1);

	OBJECT_SET_INTEGER(&tmp, command1);
	cons_set_car(h1, &tmp);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &i);
	// headの移動
	cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
	cons_set_cdr(h1, &tmp);
	*head=cons_get_cdr_cons(h1);

	// 二つ積んであったはずだから一つはずす.
	t2_pop_stack(vm, 1);
	return VM_OK;
}

static VM_RET op_stack_integer_(tPVM vm, const tINT command1, tPCELL* head)
{
	tPCELL h1, h2;
	tOBJECT offset, i, tmp;
	// iiPUSH_STACK offset
	// iiPUSH_OBJECT i;
	// com
	// ->
	// com_INTEGER_STACK i offset+1
	h1=OBJECT_GET_CELL(vm->SP-1);
	h2=OBJECT_GET_CELL(vm->SP);
	cons_get_car(cons_get_cdr_cons(h1), &offset);
	cons_get_car(cons_get_cdr_cons(h2), &i);
	OBJECT_SET_INTEGER(&offset, OBJECT_GET_INTEGER(&offset)+1);

	OBJECT_SET_INTEGER(&tmp, command1);
	cons_set_car(h1, &tmp);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &i);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset);
	//
	cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
	cons_set_cdr(h1, &tmp);
	*head=cons_get_cdr_cons(h1);

	t2_pop_stack(vm, 1);
	return VM_OK;
}

static VM_RET op_integer_stack(tPVM vm, const tINT command, tPCELL* head)
{
	tPCELL h1, h2;
	tOBJECT offset, i, tmp;
	// iiPUSH_OBJECT i
	// iiPUSH_STACK offset
	// com
	// ->
	// com_INTEGER_STACK i offset+2
	h1=OBJECT_GET_CELL(vm->SP-1);
	h2=OBJECT_GET_CELL(vm->SP);
	cons_get_car(cons_get_cdr_cons(h1), &i);
	cons_get_car(cons_get_cdr_cons(h2), &offset);
	OBJECT_SET_INTEGER(&offset, OBJECT_GET_INTEGER(&offset)+2);

	OBJECT_SET_INTEGER(&tmp, command);
	cons_set_car(h1, &tmp);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &i);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset);
	//
	cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
	cons_set_cdr(h1, &tmp);
	*head=cons_get_cdr_cons(h1);

	t2_pop_stack(vm, 1);
	return VM_OK;
}

static VM_RET op_integer_stack_(tPVM vm, const tINT command1, tPCELL* head)
{
	tPCELL h1, h2;
	tOBJECT i, offset, tmp;
	// iiPUSH_OBJECT i
	// iiPUSH_STACK offset
	// com
	// ->
	// com_STACK_INTEGER offset+2 i
	h1=OBJECT_GET_CELL(vm->SP-1);
	h2=OBJECT_GET_CELL(vm->SP);
	cons_get_car(cons_get_cdr_cons(h1), &i);
	cons_get_car(cons_get_cdr_cons(h2), &offset);
	OBJECT_SET_INTEGER(&offset, OBJECT_GET_INTEGER(&offset)+2);

	OBJECT_SET_INTEGER(&tmp, command1);
	cons_set_car(h1, &tmp);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &i);
	//
	cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
	cons_set_cdr(h1, &tmp);
	*head=cons_get_cdr_cons(h1);

	t2_pop_stack(vm, 1);
	return VM_OK;
}

static VM_RET op_stack_stack(tPVM vm, const tINT command, tPCELL *head)
{
	tPCELL h1, h2;
	tOBJECT offset1, offset2, tmp;
	// PUSH_STACK offset1
	// PUSH_STACK offset2
	// com
	// ->
	// com_STACK_STACK offset1+1 offset2+2
	h1=OBJECT_GET_CELL(vm->SP-1);
	h2=OBJECT_GET_CELL(vm->SP);
	cons_get_car(cons_get_cdr_cons(h1), &offset1);
	cons_get_car(cons_get_cdr_cons(h2), &offset2);
	OBJECT_SET_INTEGER(&offset1, OBJECT_GET_INTEGER(&offset1)+1);
	OBJECT_SET_INTEGER(&offset2, OBJECT_GET_INTEGER(&offset2)+2);

	OBJECT_SET_INTEGER(&tmp, command);
	cons_set_car(h1, &tmp);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset1);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset2);

	cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
	cons_set_cdr(h1, &tmp);
	*head=cons_get_cdr_cons(h1);

	t2_pop_stack(vm, 1);
	return VM_OK;
}

static VM_RET op_stack_stack_(tPVM vm, const tINT command, tPCELL* head)
{
	tPCELL h1, h2;
	tOBJECT offset1, offset2, tmp;
	// PUSH_STACK offset1
	// PUSH_STACK offset2
	// com
	// ->
	// com_STACK_STACK offset2+2 offset1+1
	h1=OBJECT_GET_CELL(vm->SP-1);
	h2=OBJECT_GET_CELL(vm->SP);
	cons_get_car(cons_get_cdr_cons(h1), &offset1);
	cons_get_car(cons_get_cdr_cons(h2), &offset2);
	OBJECT_SET_INTEGER(&offset1, OBJECT_GET_INTEGER(&offset1)+1);
	OBJECT_SET_INTEGER(&offset2, OBJECT_GET_INTEGER(&offset2)+2);

	OBJECT_SET_INTEGER(&tmp, command);
	cons_set_car(h1, &tmp);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset2);
	h1=cons_get_cdr_cons(h1);
	cons_set_car(h1, &offset1);

	cons_get_cdr(cons_get_cdr_cons(*head), &tmp);
	cons_set_cdr(h1, &tmp);
	*head=cons_get_cdr_cons(h1);

	t2_pop_stack(vm, 1);
	return VM_OK;
}

static VM_RET op_number_equal_(tPVM vm, const tINT id, tPCELL* head, const tINT com1, const tINT com2)
{
	tOBJECT tmp;
	tINT arg1, arg2;
	cons_get_car(OBJECT_GET_CELL(vm->SP-1), &tmp);
	arg1=OBJECT_GET_INTEGER(&tmp);
	cons_get_car(OBJECT_GET_CELL(vm->SP), &tmp);
	arg2=OBJECT_GET_INTEGER(&tmp);
	if ((arg1==iiPUSH_OBJECT)&&(arg2==iiPUSH_STACK)) {
		tOBJECT i;
		cons_get_car(cons_get_cdr_cons(OBJECT_GET_CELL(vm->SP-1)), &i);
		if (OBJECT_IS_INTEGER(&i))
			return op_integer_stack_(vm, com1, head);
	} else if ((arg1==iiPUSH_STACK)&&(arg2==iiPUSH_OBJECT)) {
		tOBJECT i;
		cons_get_car(cons_get_cdr_cons(OBJECT_GET_CELL(vm->SP)), &i);
		if (OBJECT_IS_INTEGER(&i))
			return op_stack_integer(vm, com1, head);
	} else if ((arg1==iiPUSH_STACK)&&(arg2==iiPUSH_STACK)) {
		return op_stack_stack(vm, com2, head);
	}
	if (id==iADDITION)
		return t2_primitive(vm, id, head);
	else
		return t2_primitive_2(vm, id, head);
}

static VM_RET op_number_less_(tPVM vm, const tINT id, tPCELL* head, const tINT com1, const tINT com2, const tINT com3)
{
	tOBJECT tmp;
	tINT arg1, arg2;
	cons_get_car(OBJECT_GET_CELL(vm->SP-1), &tmp);
	arg1=OBJECT_GET_INTEGER(&tmp);
	cons_get_car(OBJECT_GET_CELL(vm->SP), &tmp);
	arg2=OBJECT_GET_INTEGER(&tmp);
	if ((arg1==iiPUSH_STACK)&&(arg2==iiPUSH_OBJECT)) {
		tOBJECT i;
		cons_get_car(cons_get_cdr_cons(OBJECT_GET_CELL(vm->SP)), &i);
		if (OBJECT_IS_INTEGER(&i))
			return op_stack_integer(vm, com1, head);
	} else if ((arg1==iiPUSH_OBJECT)&&(arg2==iiPUSH_STACK)) {
		tOBJECT i;
		cons_get_car(cons_get_cdr_cons(OBJECT_GET_CELL(vm->SP-1)), &i);
		if (OBJECT_IS_INTEGER(&i))
			return op_integer_stack(vm, com2, head);
	} else if ((arg1==iiPUSH_STACK)&&(arg2==iiPUSH_STACK)) {
		return op_stack_stack(vm, com3, head);
	}
	if (id==iSUBSTRACTION) 
		return t2_primitive(vm, id, head);
	else
		return t2_primitive_2(vm, id, head);
}

static VM_RET op_number_greater_(tPVM vm, const tINT id, tPCELL* head, const tINT com1, const tINT com2, const tINT com3)
{
	tOBJECT tmp;
	tINT arg1, arg2;
	cons_get_car(OBJECT_GET_CELL(vm->SP-1), &tmp);
	arg1=OBJECT_GET_INTEGER(&tmp);
	cons_get_car(OBJECT_GET_CELL(vm->SP), &tmp);
	arg2=OBJECT_GET_INTEGER(&tmp);
	if ((arg1==iiPUSH_STACK)&&(arg2==iiPUSH_OBJECT)) {
		tOBJECT i;
		cons_get_car(cons_get_cdr_cons(OBJECT_GET_CELL(vm->SP)), &i);
		if (OBJECT_IS_INTEGER(&i))
			return op_stack_integer_(vm, com1, head);
	} else if ((arg1==iiPUSH_OBJECT)&&(arg2==iiPUSH_STACK)) {
		tOBJECT i;
		cons_get_car(cons_get_cdr_cons(OBJECT_GET_CELL(vm->SP-1)), &i);
		if (OBJECT_IS_INTEGER(&i))
			return op_integer_stack_(vm, com2, head);
	} else if ((arg1==iiPUSH_STACK)&&(arg2==iiPUSH_STACK)) {
		return op_stack_stack_(vm, com3, head);
	}
	return t2_primitive_2(vm, id, head);
}

static VM_RET t2_number_equal(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_equal_(vm, id, head, iiNUMBER_EQUAL_STACK_INTEGER, iiNUMBER_EQUAL_STACK_STACK);
}

static VM_RET t2_number_not_equal(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_equal_(vm, id, head, iiNUMBER_NOT_EQUAL_STACK_INTEGER, iiNUMBER_NOT_EQUAL_STACK_STACK);
}

static VM_RET t2_number_less(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_less_(vm, id, head, iiNUMBER_LESS_STACK_INTEGER, iiNUMBER_LESS_INTEGER_STACK, iiNUMBER_LESS_STACK_STACK);
}

static VM_RET t2_number_greater(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_greater_(vm, id, head, iiNUMBER_LESS_INTEGER_STACK, iiNUMBER_LESS_STACK_INTEGER, iiNUMBER_LESS_STACK_STACK);
}

static VM_RET t2_number_le(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_less_(vm, id, head, iiNUMBER_LE_STACK_INTEGER, iiNUMBER_LE_INTEGER_STACK, iiNUMBER_LE_STACK_STACK);
}

static VM_RET t2_number_ge(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_greater_(vm, id, head, iiNUMBER_LE_INTEGER_STACK, iiNUMBER_LE_STACK_INTEGER, iiNUMBER_LE_STACK_STACK);
}

static VM_RET t2_addition(tPVM vm, const tINT id, tPCELL* head)
{
	tOBJECT n;
	cons_get_car(cons_get_cdr_cons(*head), &n);
	if (OBJECT_GET_INTEGER(&n)==2) {
		return op_number_equal_(vm, id, head, iiADDITION_STACK_INTEGER, iiADDITION_STACK_STACK);
	}
	return t2_primitive(vm, id, head);
}

static VM_RET t2_substraction(tPVM vm, const tINT id, tPCELL* head)
{
	tOBJECT n;
	cons_get_car(cons_get_cdr_cons(*head), &n);
	if (OBJECT_GET_INTEGER(&n)==2) {
		return op_number_less_(vm, id, head, iiSUBSTRACTION_STACK_INTEGER, iiSUBSTRACTION_INTEGER_STACK, iiSUBSTRACTION_STACK_STACK);
	}
	return t2_primitive(vm, id, head);
}

static VM_RET t2_eq(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_equal_(vm, id, head, iiEQ_STACK_INTEGER, iiEQ_STACK_STACK);
}

static VM_RET t2_eql(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_equal_(vm, id, head, iiEQL_STACK_INTEGER, iiEQL_STACK_STACK);
}

static VM_RET t2_equal(tPVM vm, const tINT id, tPCELL* head)
{
	return op_number_equal_(vm, id, head, iiEQUAL_STACK_INTEGER, iiEQUAL_STACK_STACK);
}
