//
// TISL/src/tisl/translator_1.c
// TISL Ver. 4.x
//

#define TISL_PRIMITIVE_TRANSLATOR
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "built_in_object.h"
#include "translator.h"
#include "command.h"

extern tTRANSLATOR vm_get_translator(tPVM vm);

/////////////////////////////

// body部がformの場合
static VM_RET t1_code_list(tPVM vm, tPOBJECT form, tPCELL* clist, const tBOOL tail);
// body部がform*の場合
static VM_RET t1_code_list_(tPVM vm, tPOBJECT form, tPCELL* clist, const tBOOL tail);
// 評価形式の変換
static VM_RET t1_evaluation_form(tPVM vm, tPOBJECT form, tPCELL clist, const tBOOL tail);
// 逐次実行の変換 form*
static VM_RET t1_sequence(tPVM vm, tPOBJECT form, tPCELL clist, const tBOOL tail);
// 複合形式の変換(op argument*)
static VM_RET t1_compound_form(tPVM vm, tPOBJECT form, tPCELL clist, const tBOOL tail);
// 逆引用符構文の変換
static VM_RET t1_quasiquote(tPVM vm, tPOBJECT form, tPCELL clist);
// 関数適用形式
static VM_RET t1_function_application_form(tPVM vm, tPCELL blist, tPOBJECT operands, tPCELL clist, const tBOOL tail);
// (局所)関数適用形式
static VM_RET t1_local_function_application_form(tPVM vm, tPCELL fname, tPCELL flist, tPOBJECT operands, tPCELL clist, const tBOOL tail);
// 関数適用形式(lambda形式)
static VM_RET t1_lambda_operator(tPVM vm, tPOBJECT lambda, tPOBJECT operands, tPCELL clist, const tBOOL tail);
// 引数の評価
static VM_RET t1_operands(tPVM vm, tPOBJECT operands, tINT* anum, tPCELL clist);
// 引数の調整
static VM_RET t1_arity_check(tPVM vm, tINT pnum, tINT anum, tPCELL clist);
// lambda-listから引数リストを作成
static VM_RET t1_lambda_list(tPVM vm, tPOBJECT lambda_list, tPCELL* plist);
// 特殊形式
static VM_RET t1_special_form(tPVM vm, const tINT special_operator_id, tPOBJECT operands, tPCELL clist, const tBOOL tail);

/////////////////////////////

static VM_RET t1_lambda_to_function_list(tPVM vm, tPOBJECT operands, tPCELL* flist);
static VM_RET t1_syntax_check(tPVM vm, tPOBJECT operands, const tINT n);
// labels flet
static VM_RET t1_no_local_function(tPVM vm, tPCELL p, tPCELL clist, const tBOOL tail);
static VM_RET t1_check_local_function_head(tPVM vm, tPCELL head, tPCELL nlist);
// setq setf
static VM_RET t1_setq_(tPVM vm, tPOBJECT var, tPOBJECT form, tPCELL clist);
// setf
static VM_RET t1_setf_(tPVM vm, tPOBJECT place, tPOBJECT form, tPCELL clist);
static VM_RET t1_setf_dynamic(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist);
static VM_RET t1_setf_aref(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist);
static VM_RET t1_setf_elt(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist);
static VM_RET t1_setf_cons(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist);
static VM_RET t1_setf_accessor(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist);
// let*
static VM_RET t1_letb(tPVM vm, tPOBJECT parameter, tPOBJECT body, tPCELL clist, const tBOOL tail);

// method
static VM_RET t1_method_qualifier(tPVM vm, tPCELL* head, tINT* qualifier);
static VM_RET t1_parameter_profiler(tPVM vm, tPCELL* head, tPCELL* pplist);

/////////////////////////////

static VM_RET t1_function(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_lambda(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_labels(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_flet(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_and_or(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_quote(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_setq(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_setf(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_let(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_leta(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_dynamic(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_dynamic_let(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_if(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_cond(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_case(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_case_using(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_progn(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_while(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_for(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_block(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_return_from(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_catch(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_throw(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_tagbody(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_go(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_unwind_protect(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_class(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_the(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_convert(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_with_standard(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_with_open_file(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_ignore_errors(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_continue_condition(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_with_handler(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_time(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);
static VM_RET t1_in_package(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail);

static VM_RET t1_primitive_function(tPVM vm, const tINT id, const tINT anum, tPCELL clist);
/////////////////////////////

typedef VM_RET (*T1_SPECIAL_OPERATOR)(tPVM, const tINT, tPOBJECT, tPCELL, const tBOOL);

T1_SPECIAL_OPERATOR t1_special_operator_table[]={
	t1_function,
	t1_lambda,
	t1_labels,
	t1_flet,
	t1_and_or,
	t1_and_or,
	t1_quote,
	t1_setq,
	t1_setf,
	t1_let,
	t1_leta,
	t1_dynamic,
	t1_dynamic_let,
	t1_if,
	t1_cond,
	t1_case,
	t1_case_using,
	t1_progn,
	t1_while,
	t1_for,
	t1_block,
	t1_return_from,
	t1_catch,
	t1_throw,
	t1_tagbody,
	t1_go,
	t1_unwind_protect,
	t1_class,
	t1_the,
	t1_the,
	t1_convert,
	t1_with_standard,
	t1_with_standard,
	t1_with_standard,
	t1_with_open_file,
	t1_with_open_file,
	t1_with_open_file,
	t1_ignore_errors,
	t1_continue_condition,
	t1_with_handler,
	t1_time,
	t1_in_package,
};

/////////////////////////////

// マクロと特殊形式の展開を行い制御情報に基づいた変換を主に行う．
// また，名前の参照のされかたから値の存在期間を推論し，次のパスで利用する．
VM_RET translate_pass1(tPVM vm, tPOBJECT form, tPCELL* code_list_1)
{
	VM_RET ret;
	t1_clear(vm_get_translator(vm));// いらない?
	ret=t1_code_list(vm, form, code_list_1, tTRUE);
	t1_clear(vm_get_translator(vm));
	return ret;
}

VM_RET translate_pass1_defun(tPVM vm, tPCELL name, tPOBJECT lambda_list, tPOBJECT forms, tPCELL* flist)
{
	tPCELL plist, clist;

	t1_clear(vm_get_translator(vm));
	if (t1_lambda_list(vm, lambda_list, &plist)) return VM_ERROR;
	t1_set_defining_function_name(vm, name);
	t1_set_defining_function_parameter_list(vm, plist);
	t1_set_argument_point(vm, plist);
	if (t1_push_variable(vm, plist)) return VM_ERROR;
	if (t1_code_list_(vm, forms, &clist, tTRUE)) goto ERROR;
	if (function_list_create(vm, plist, clist, flist)) goto ERROR;
	t1_pop_variable(vm);
	t1_clear(vm_get_translator(vm));
	return VM_OK;
ERROR:
	t1_pop_variable(vm);// いらない？
	t1_clear(vm_get_translator(vm));
	return VM_ERROR;
}

VM_RET translate_pass1_method(tPVM vm, tPCELL form, tPOBJECT body, tPCELL* mlist)
{// form : (method-qualifier* parameter-profile form*)
	tPCELL p=form, pplist, clist, envlist;
	tINT qualifier;
	tOBJECT obj;
	t1_clear(vm_get_translator(vm));
	if (t1_method_qualifier(vm, &p, &qualifier)) return VM_ERROR;
	if (t1_parameter_profiler(vm, &p, &pplist)) return VM_ERROR;
	cell_to_object(pplist, &obj);
	if (p) cell_to_object(p, body);
	else OBJECT_SET_NIL(body);
	// 引数
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (t1_push_variable(vm, pplist_get_plist(pplist))) { vm_pop(vm); goto ERROR; }
	t1_set_argument_point(vm, pplist_get_plist(pplist));
	if ((qualifier==METHOD_AROUND)||(qualifier==METHOD_PRIMARY)) {
		// around primary メソッド
		// call-next-method next-method-p 用関数を登録
		if (t1_create_method_env(vm, &envlist)) { vm_pop(vm); goto ERROR; }
		if (t1_push_function(vm, envlist)) { vm_pop(vm); goto ERROR; }
		if (code_list_create(vm, &clist)) { vm_pop(vm); goto ERROR; }
		cell_to_object(clist, &obj);
		if (vm_push(vm, &obj)) { vm_pop(vm); goto ERROR; }
		if (t1_sequence(vm, body, clist, tFALSE)) { vm_pop(vm); vm_pop(vm); goto ERROR; }
		if (code_list_add_command_1(vm, clist, iRET)) { vm_pop(vm); vm_pop(vm); goto ERROR; }
		vm_pop(vm);
		vm_pop(vm);
		if (method_list_create(vm, pplist, clist, envlist, qualifier, mlist)) goto ERROR;
		t1_pop_function(vm);
	} else {// before after メソッド
		if (code_list_create(vm, &clist)) { vm_pop(vm); goto ERROR; }
		cell_to_object(clist, &obj);
		if (vm_push(vm, &obj)) { vm_pop(vm); goto ERROR; }
		if (t1_sequence(vm, body, clist, tFALSE)) { vm_pop(vm); vm_pop(vm); goto ERROR; }
		if (code_list_add_command_1(vm, clist, iRET)) { vm_pop(vm); vm_pop(vm); goto ERROR; }
		vm_pop(vm);
		vm_pop(vm);
		if (method_list_create(vm, pplist, clist, 0, qualifier, mlist)) goto ERROR;
	}
	t1_pop_variable(vm);
	t1_clear(vm_get_translator(vm));
	return VM_OK;
ERROR:
	t1_clear(vm_get_translator(vm));
	return VM_ERROR;
}

/////////////////////////////

// body部がformの場合
static VM_RET t1_code_list(tPVM vm, tPOBJECT form, tPCELL* clist, const tBOOL tail)
{
	tOBJECT obj;

	// 空のコードリストの作成
	if (code_list_create(vm, clist)) return VM_ERROR;
	// コードリストの保護
	cell_to_object(*clist, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	// formを評価するための命令をclistに追加していく
	if (t1_evaluation_form(vm, form, *clist, tail)) { vm_pop(vm); return VM_ERROR; }
	if (code_list_add_command_1(vm, *clist, iRET)) { vm_pop(vm); return VM_ERROR; }
	// コードリスト保護の終了
	vm_pop(vm);
	return VM_OK;
}

// body部がform*の場合
static VM_RET t1_code_list_(tPVM vm, tPOBJECT form, tPCELL* clist, const tBOOL tail)
{
	tOBJECT obj;

	if (code_list_create(vm, clist)) return VM_ERROR;
	cell_to_object(*clist, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (t1_sequence(vm, form, *clist, tail)) { vm_pop(vm); return VM_ERROR; }
	if (code_list_add_command_1(vm, *clist, iRET)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return VM_OK;
}

// 評価形式の変換
static VM_RET t1_evaluation_form(tPVM vm, tPOBJECT form, tPCELL clist, const tBOOL tail)
{
	switch (OBJECT_GET_TYPE(form)) {
		// リテラルの場合 : リテラルそのものを返す
	case OBJECT_NIL:
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	case OBJECT_INTEGER:
	case OBJECT_FLOAT:
	case OBJECT_CHARACTER:
	case OBJECT_STRING:
	case OBJECT_VECTOR:
	case OBJECT_ARRAY:
		if (code_list_add_command_1(vm, clist, iPUSH_OBJECT)) return VM_ERROR;
		return code_list_add_argument(vm, clist, form);
		// 識別子の場合 : 
		//  現在の静的環境での変数名前空間で
		//  その識別子が指定するオブジェクトを結果をする
	case OBJECT_SYMBOL:
		{
			tPCELL symbol=OBJECT_GET_CELL(form);
			tINT code;
			if (symbol_is_simple(symbol)&&
				t1_search_variable(vm, symbol)) {
				// 局所的なスコープをもつ変数の参照
				code=iPUSH_LOCAL_VARIABLE;
			} else {
				// 最上位のスコープを持つ変数の参照
				code=iPUSH_GLOBAL_VARIABLE;
			}
			return code_list_add_command_1(vm, clist, code)||
				code_list_add_argument(vm, clist, form);
		}
		// 複合形式の場合 :
	case OBJECT_CONS:
		{
			VM_RET r;
			t1_increment_form_level(vm);
			r=t1_compound_form(vm, form, clist, tail);
			t1_decrement_form_level(vm);
			return r;
		}
		// 逆引用関係の構文の場合
	case OBJECT_QUASIQUOTE:
		{
			VM_RET r;
			tOBJECT obj;
			quasiquote_get_form(OBJECT_GET_CELL(form), &obj);
			t1_increment_quasiquote_level(vm);
			r=t1_quasiquote(vm, &obj, clist);
			t1_decrement_quasiquote_level(vm);
			return r;
		}
	case OBJECT_UNQUOTE:
	case OBJECT_UNQUOTE_SPLICING:
		// 逆引用符に囲まれていないunquote構文
		return signal_violation(vm, TRANSLATOR_ERROR_UNQUOTE, translator_get_form(vm));
	}
	// 未知の評価形式
	return signal_violation(vm, TRANSLATOR_ERROR_UNKNOWN_OBJECT, translator_get_form(vm));
}

// 逐次実行の変換 form*
static VM_RET t1_sequence(tPVM vm, tPOBJECT form, tPCELL clist, const tBOOL tail)
{
	tPCELL p;
	tOBJECT obj, cdr;
	tBOOL b;
	// 空リスト
	if (OBJECT_IS_NIL(form))
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	// ドットリスト
	if (!OBJECT_IS_CONS(form))
		return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	p=OBJECT_GET_CELL(form);
	while (1) {// nilでリストが終了するまでループを続ける
		cons_get_car(p, &obj);
		cons_get_cdr(p, &cdr);
		b=OBJECT_IS_NIL(&cdr) ? tail : tFALSE;
		if (t1_evaluation_form(vm, &obj, clist, b)) return VM_ERROR;
		// リストがnilで終了した
		if (OBJECT_IS_NIL(&cdr)) return VM_OK;
		// ドットリスト
		if (!OBJECT_IS_CONS(&cdr))
			return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		p=OBJECT_GET_CELL(&cdr);
		// 各評価命令の間にDISCARD命令を挿入
		if (code_list_add_command_1(vm, clist, iDISCARD)) return VM_ERROR;
	}
}

// 複合形式の変換(op argument*)
static VM_RET t1_compound_form(tPVM vm, tPOBJECT form, tPCELL clist, const tBOOL tail)
{
	tOBJECT op, operands;
	cons_get_car(OBJECT_GET_CELL(form), &op);
	cons_get_cdr(OBJECT_GET_CELL(form), &operands);
	if (OBJECT_IS_SYMBOL(&op)) {
		tPCELL fname, flist;
		fname=OBJECT_GET_CELL(&op);
		if (symbol_is_simple(fname)&&
			t1_search_function(vm, fname, &flist)) {
			// 局所関数
			return t1_local_function_application_form(vm, fname, flist, &operands, clist, tail);
		} else {
			// 大域関数
			tOBJECT obj;
			tPCELL bind, blist;
			// 定義中の関数か否か
			if (symbol_is_simple(fname)&&
				(t1_get_defining_function_name(vm)==fname)) {
				tINT anum, pnum, code;
				// 現在定義中の関数
				if (t1_operands(vm, &operands, &anum, clist)) return VM_ERROR;
				pnum=parameter_list_get_number(t1_get_defining_function_parameter_list(vm));
				if (parameter_list_is_rest(t1_get_defining_function_parameter_list(vm))) pnum=-pnum;
				if (t1_arity_check(vm, pnum, anum, clist)) return VM_ERROR;
				code=tail ? iCALL_TAIL_REC : iCALL_REC;
				if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
				return VM_OK;
			}
			// 大域関数の呼び出し
			if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), fname, &blist)) return VM_ERROR;
			bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
			if (bind)
				bind_get_object(bind, NAMESPACE_FUNCTION, &obj);
			else
				OBJECT_SET_UNBOUND(&obj);
			if (OBJECT_IS_MACRO(&obj)) {
				// 演算子がマクロだった
				tOBJECT expanded;
				VM_RET ret;
				if (t_macro_expand(vm, OBJECT_GET_CELL(&obj), &operands, &expanded)) return VM_ERROR;
				// マクロ展開後の評価形式を変換
				t1_decrement_form_level(vm);
				if (vm_push(vm, &expanded)) { t1_increment_form_level(vm); return VM_ERROR; }
				ret=t1_evaluation_form(vm, &expanded, clist, tail);
				vm_pop(vm);
				t1_increment_form_level(vm);
				return ret;
			} else if (symbol_is_special_operator(fname)) {
				return t1_special_form(vm, symbol_get_special_operator_id(fname), &operands, clist, tail);
			} else if (OBJECT_IS_SPECIAL_OPERATOR(&obj)) {
				// 特殊演算子
				return t1_special_form(vm, OBJECT_GET_INTEGER(&obj), &operands, clist, tail);
			} else if (OBJECT_IS_DEFINING_OPERATOR(&obj)) {
				return signal_violation(vm, TRANSLATOR_ERROR_NOT_TOP_FORM, translator_get_form(vm));
			} else {
				// 最上位有効範囲を持つ名前を束縛している関数の呼び出し
				return t1_function_application_form(vm, blist, &operands, clist, tail);
			}
		}
	} else if (OBJECT_IS_CONS(&op)) {
		// 演算子がlambda形式の場合
		tOBJECT lambda;
		cons_get_car(OBJECT_GET_CELL(&op), &lambda);
		if (OBJECT_IS_SYMBOL(&lambda)) {
			if (OBJECT_GET_CELL(&lambda)==global_symbol[sLAMBDA]) {
				return t1_lambda_operator(vm, &op, &operands, clist, tail);
			} else {
				tPCELL blist, bind;
				tOBJECT obj;
				if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&lambda), &blist)) return VM_ERROR;
				bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
				if (bind) {
					bind_get_object(bind, NAMESPACE_FUNCTION, &obj);
					if (OBJECT_IS_SPECIAL_OPERATOR(&obj)&&(OBJECT_GET_INTEGER(&obj)==bLAMBDA)) {
						return t1_lambda_operator(vm, &op, &operands, clist, tail);
					}
				}
			}
		}
	}
	// 演算子が変
	return signal_violation(vm, TRANSLATOR_ERROR_BAD_OPERATOR, translator_get_form(vm));
}

// 逆引用符構文の変換
static VM_RET t1_quasiquote(tPVM vm, tPOBJECT form, tPCELL clist)
{
	switch (OBJECT_GET_TYPE(form)) {
	case OBJECT_NIL:// リテラル
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	case OBJECT_INTEGER:
	case OBJECT_FLOAT:
	case OBJECT_CHARACTER:
	case OBJECT_STRING:
	case OBJECT_VECTOR:
	case OBJECT_ARRAY:
	case OBJECT_SYMBOL:
		if (code_list_add_command_1(vm, clist, iPUSH_OBJECT)) return VM_ERROR;
		return code_list_add_argument(vm, clist, form);
	case OBJECT_CONS:
		{// コンス
			tOBJECT car, cdr;
			cons_get_car(OBJECT_GET_CELL(form), &car);
			if (t1_quasiquote(vm, &car, clist)) return VM_ERROR;
			cons_get_cdr(OBJECT_GET_CELL(form), &cdr);
			if (t1_quasiquote(vm, &cdr, clist)) return VM_ERROR;
			return code_list_add_command_1(vm, clist, iQUASIQUOTE2);
		}
	case OBJECT_QUASIQUOTE:
		{
			tOBJECT obj;
			quasiquote_get_form(OBJECT_GET_CELL(form), &obj);
			t1_increment_quasiquote_level(vm);
			if (t1_quasiquote(vm, &obj, clist)) { t1_decrement_quasiquote_level(vm); return VM_ERROR; }
			t1_decrement_quasiquote_level(vm);
			return code_list_add_command_1(vm, clist, iQUASIQUOTE);
		}
	case OBJECT_UNQUOTE:
		{
			tOBJECT obj;
			unquote_get_form(OBJECT_GET_CELL(form), &obj);
			if (t1_get_quasiquote_level(vm)==1) {
				VM_RET ret;
				t1_decrement_quasiquote_level(vm);
				ret=t1_evaluation_form(vm, &obj, clist, tFALSE);
				t1_increment_quasiquote_level(vm);
				return ret;
			} else {
				t1_decrement_quasiquote_level(vm);
				if (t1_quasiquote(vm, &obj, clist)) { t1_increment_quasiquote_level(vm); return VM_ERROR; }
				t1_increment_quasiquote_level(vm);
				return code_list_add_command_1(vm, clist, iUNQUOTE);
			}
		}
	case OBJECT_UNQUOTE_SPLICING:
		{
			tOBJECT obj;
			unquote_splicing_get_form(OBJECT_GET_CELL(form), &obj);
			if (t1_get_quasiquote_level(vm)==1) {
				t1_decrement_quasiquote_level(vm);
				if (t1_evaluation_form(vm, &obj, clist, tFALSE)) { t1_increment_quasiquote_level(vm); return VM_ERROR; }
				t1_increment_quasiquote_level(vm);
				return code_list_add_command_1(vm, clist, iUNQUOTE_SPLICING2);
			} else {
				t1_decrement_quasiquote_level(vm);
				if (t1_quasiquote(vm, &obj, clist)) { t1_increment_quasiquote_level(vm); return VM_ERROR; }
				t1_increment_quasiquote_level(vm);
				return code_list_add_command_1(vm, clist, iUNQUOTE_SPLICING);
			}
		}
	default:
		return signal_violation(vm, TRANSLATOR_ERROR, translator_get_form(vm));
	}
}

// マクロ展開
VM_RET t_macro_expand(tPVM vm, tPCELL macro, tPOBJECT operands, tPOBJECT ret)
{
	tPCELL p;
	tOBJECT obj;
	tINT i, anum, pnum;
	pnum=macro_get_parameter_number(macro);
	if (OBJECT_IS_CONS(operands)) {
		p=OBJECT_GET_CELL(operands);
		anum=cons_get_length(p);
	} else {
		p=0;
		anum=0;
	}
	for (i=0; i<anum; i++) {
		// 引数をスタックに積む
		cons_get_car(p, &obj);
		if (vm_push(vm, &obj)) return VM_ERROR;
		p=cons_get_cdr_cons(p);
	}
	if (macro_get_rest(macro)) {
		// rest有り
		if (anum<pnum-1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
		if (anum==pnum-1) {
			if (vm_push(vm, &nil)) return VM_ERROR;
		} else {// 余分な引数をリストにまとめる
			if (vm_list(vm, anum-pnum+1)) return VM_ERROR;
		}
	} else {
		// rest無し
		if (anum!=pnum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	// マクロ展開の実行
	if (macro_expand(vm, macro, ret)) return VM_ERROR;
	// 評価スタックを元に戻す
	if (vm_discard(vm, pnum)) return VM_ERROR;
	return VM_OK;
}

// 関数適用形式
static VM_RET t1_function_application_form(tPVM vm, tPCELL blist, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{// iCALL_GLOBAL bind-list anum
	tINT anum, code;
	tOBJECT obj;
	tPCELL bind;
	if (t1_operands(vm, operands, &anum, clist)) return VM_ERROR;
	bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
	if (bind) {
		bind_get_function(bind, &obj);
		// 組込み関数の変換
		if (OBJECT_IS_PRIMITIVE_OPERATOR(&obj)) {
			return t1_primitive_function(vm, OBJECT_GET_INTEGER(&obj), anum, clist);
		}
	}
	if (bind_list_bind_is_head(blist, bind)) {
		code = tail ? iCALL_TAIL_BIND : iCALL_BIND;
		if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
		cell_to_object(bind, &obj);
		if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
		OBJECT_SET_INTEGER(&obj, anum);
		return code_list_add_argument(vm, clist, &obj);
	} else {
		code = tail ? iCALL_TAIL_GLOBAL : iCALL_GLOBAL;
		if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
		cell_to_object(blist, &obj);
		if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
		OBJECT_SET_INTEGER(&obj, anum);
		return code_list_add_argument(vm, clist, &obj);
	}
}

// (局所)関数適用形式
static VM_RET t1_local_function_application_form(tPVM vm, tPCELL fname, tPCELL flist, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{// iCALL_LOCAL symbol flist
	tINT anum, pnum, code;
	tOBJECT obj;

	if (t1_operands(vm, operands, &anum, clist)) return VM_ERROR;
	pnum=function_list_get_parameter_number(flist);
	if (t1_arity_check(vm, pnum, anum, clist)) return VM_ERROR;
	code = tail ? iCALL_TAIL_LOCAL : iCALL_LOCAL;
	if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
	cell_to_object(fname, &obj);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	cell_to_object(flist, &obj);
	return code_list_add_argument(vm, clist, &obj);
}

// 関数適用形式(lambda形式)
static VM_RET t1_lambda_operator(tPVM vm, tPOBJECT lambda, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj, obj2, lambda_list, forms;
	tINT anum, pnum;
	tPCELL plist;

	// 引数の変換
	if (t1_operands(vm, operands, &anum, clist)) return VM_ERROR;
	// 演算子のlambda形式から関数オブジェクトを作成する
	cons_get_cdr(OBJECT_GET_CELL(lambda), &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA, translator_get_form(vm));
	cons_get_car(OBJECT_GET_CELL(&obj), &lambda_list);
	// 引数リストの作成
	if (t1_lambda_list(vm, &lambda_list, &plist)) return VM_ERROR;
	if (t1_push_variable(vm, plist)) return VM_ERROR;
	pnum=parameter_list_get_number(plist);
	if (parameter_list_is_rest(plist)) pnum=-pnum;
	if (t1_arity_check(vm, pnum, anum, clist)) goto ERROR;
	if (pnum<0) pnum=-pnum;
	OBJECT_SET_CONS(&obj2, plist);
	t1_increment_form_level(vm);
		// iLAMBDA_IN plist
	cons_get_cdr(OBJECT_GET_CELL(&obj), &forms);
	if (code_list_add_command_1(vm, clist, iLAMBDA_IN)||
		code_list_add_argument(vm, clist, &obj2)||
		// sequence
		t1_sequence(vm, &forms, clist, tail)||
		// iLAMBDA_OUT plist
		code_list_add_command_1(vm, clist, iLAMBDA_OUT)||
		code_list_add_argument(vm, clist, &obj2)) goto ERROR2;
	t1_decrement_form_level(vm);
	t1_pop_variable(vm);
	return VM_OK;
	// エラー発生時の後処理
ERROR2:
	t1_decrement_form_level(vm);
ERROR:
	t1_pop_variable(vm);
	return VM_ERROR;
}

// 引数の評価
static VM_RET t1_operands(tPVM vm, tPOBJECT operands, tINT* anum, tPCELL clist)
{
	tOBJECT obj;
	tPCELL p;
	*anum=0;
	if (!OBJECT_IS_CONS(operands)) {
		if (!OBJECT_IS_NIL(operands))
			return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		else
			return VM_OK;
	}
	p=OBJECT_GET_CELL(operands);
	while (1) {
		// 引数の変換
		cons_get_car(p, &obj);
		if (t1_evaluation_form(vm, &obj, clist, tFALSE)) return VM_ERROR;
		++*anum;
		// 次の引数へ
		cons_get_cdr(p, &obj);
		if (!OBJECT_IS_CONS(&obj)) {
			if (OBJECT_IS_NIL(&obj))
				// リストがnilで終了
				return VM_OK;
			else
				// ドットリスト?
				return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
		// 次へ
		p=cons_get_cdr_cons(p);
	}
}

// 引数の調整
static VM_RET t1_arity_check(tPVM vm, tINT pnum, tINT anum, tPCELL clist)
{ 
	if (pnum<0) {
		// rest有り
		pnum=-pnum;
		if (anum<pnum-1) return signal_violation(vm, TRANSLATOR_ERROR_ARITY_ERROR, translator_get_form(vm));
		if (anum==pnum-1) {
			if (code_list_add_command_1(vm, clist, iPUSH_NIL)) return VM_ERROR;
		} else {
			// 余分な引数からリストを作成する
			tOBJECT num;
			OBJECT_SET_INTEGER(&num, anum-pnum+1);
			if (code_list_add_command_1(vm, clist, iLIST)) return VM_ERROR;
			if (code_list_add_argument(vm, clist, &num)) return VM_ERROR;
		}
	} else {
		// rest1無し
		if (anum!=pnum) return signal_violation(vm, TRANSLATOR_ERROR_ARITY_ERROR, translator_get_form(vm));
	}
	return VM_OK;
}

// lambda-listから引数リストを作成
static VM_RET t1_lambda_list(tPVM vm, tPOBJECT lambda_list, tPCELL* plist)
{
	tOBJECT obj, name;
	tPCELL p;
	// 引数リストの作成
	if (parameter_list_create(vm, plist)) return VM_ERROR;
	cell_to_object(*plist, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	//
	if (OBJECT_IS_NIL(lambda_list)) {
		//　引数無し
		vm_pop(vm);
		parameter_list_finish_initialization(*plist);
		return VM_OK;
	}
	if (!OBJECT_IS_CONS(lambda_list)) { vm_pop(vm); return VM_ERROR; }
	p=OBJECT_GET_CELL(lambda_list);
	while (1) {// 引数の読込み
		cons_get_car(p, &name);
		// 仮引数は記号
		if (!OBJECT_IS_SYMBOL(&name)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA_LIST, translator_get_form(vm)); }
		// 仮引数にパッケージ修飾子をつけることはできない．
		if (!symbol_is_simple(OBJECT_GET_CELL(&name))) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA_LIST, translator_get_form(vm)); }
		if (OBJECT_GET_CELL(&name)==KEYWORD_REST) {
			// :rest
			cons_get_cdr(p, &obj);
			if (!OBJECT_IS_CONS(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA_LIST, translator_get_form(vm)); }
			p=OBJECT_GET_CELL(&obj);
			cons_get_car(p, &name);
			cons_get_cdr(p, &obj);
			// nameは記号でなければならない
			// :restの後ろは一つの記号のみ
			if (!OBJECT_IS_NIL(&obj)||
				!OBJECT_IS_SYMBOL(&name)||
				!symbol_is_simple(OBJECT_GET_CELL(&name))) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA_LIST, translator_get_form(vm)); }
			if (parameter_list_add_parameter(vm, *plist, &name)) { vm_pop(vm); return VM_ERROR; }
			// rest付き
			parameter_list_set_rest(*plist);
			// 引数リストの初期化終了
			parameter_list_finish_initialization(*plist);
			vm_pop(vm);
			return VM_OK;
		}
		if (parameter_list_add_parameter(vm, *plist, &name)) { vm_pop(vm); return VM_ERROR; }
		cons_get_cdr(p, &obj);
		if (OBJECT_IS_NIL(&obj)) {
			// nilでリストが終了した
			parameter_list_finish_initialization(*plist);
			vm_pop(vm);
			return VM_OK;
		}
		if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_LAMBDA_LIST, translator_get_form(vm));
		// 次の仮引数の読込み
		p=OBJECT_GET_CELL(&obj);
	}
}

// (lambda-list form*)
static VM_RET t1_lambda_to_function_list(tPVM vm, tPOBJECT operands, tPCELL* flist)
{
	tPCELL plist, clist, p, old_defining_function_name, old_defining_function_plist, old_argument_point;
	tOBJECT lambda_list, form, obj;
	// 書き直したい/*!!!*/
	old_argument_point=t1_get_argument_point(vm);
	old_defining_function_name=t1_get_defining_function_name(vm);
	old_defining_function_plist=t1_get_defining_function_parameter_list(vm);
	if (old_defining_function_name) {
		cell_to_object(old_defining_function_name, &obj);
		if (vm_push(vm, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
		cell_to_object(old_defining_function_plist, &obj);
		if (vm_push(vm, &obj)) { vm_pop(vm); t1_pop_variable(vm); return VM_ERROR; }
		t1_set_defining_function_name(vm, 0);
		t1_set_defining_function_parameter_list(vm, 0);
	}
	// 書き直したい/*!!!*/
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &lambda_list);
	cons_get_cdr(p, &form);
	if (t1_lambda_list(vm, &lambda_list, &plist)) return VM_ERROR;
	if (t1_push_variable(vm, plist)) return VM_ERROR;
	t1_set_argument_point(vm, plist);
	if (t1_code_list_(vm, &form, &clist, tTRUE)) { if (old_defining_function_name) { vm_pop(vm); vm_pop(vm); } t1_pop_variable(vm); return VM_ERROR; }
	t1_pop_variable(vm);
	t1_set_argument_point(vm, old_argument_point);

	if (old_defining_function_name) {
		t1_set_defining_function_name(vm, old_defining_function_name);
		t1_set_defining_function_parameter_list(vm, old_defining_function_plist);
		vm_pop(vm);
		vm_pop(vm);
	}

	return function_list_create(vm, plist, clist, flist);
}

// method

static VM_RET t1_method_qualifier(tPVM vm, tPCELL* head, tINT* qualifier)
{
	tOBJECT tmp;
	tINT i;

	if (!*head) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	if (!CELL_IS_CONS(*head)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	*qualifier=METHOD_PRIMARY;
	i=0;
	while (1) {
		cons_get_car(*head, &tmp);
		if (OBJECT_IS_SYMBOL(&tmp)) {
			if (OBJECT_GET_CELL(&tmp)==KEYWORD_AROUND) {
				*qualifier=METHOD_AROUND;
			} else if (OBJECT_GET_CELL(&tmp)==KEYWORD_BEFORE) {
				*qualifier=METHOD_BEFORE;
			} else if (OBJECT_GET_CELL(&tmp)==KEYWORD_AFTER) {
				*qualifier=METHOD_AFTER;
			} else {
				return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
			}
			if (i) return signal_violation(vm, TRANSLATOR_ERROR_METHOD_QUALIFIERS, translator_get_form(vm));
			i=1;
		} else if (OBJECT_IS_CONS(&tmp)) {
			return VM_OK;
		}
		*head=cons_get_cdr_cons(*head);
		if (!*head) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
}

static VM_RET t1_parameter_profiler(tPVM vm, tPCELL* head, tPCELL* pplist)
{
	tOBJECT tmp;
	tPCELL p;
	// paramenter-specializer ;;= ({ var | (var parameter-specializer-name)}* [:rest var])
	if (parameter_profiler_list_create(vm, pplist)) return VM_ERROR;
	cell_to_object(*pplist, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	cons_get_car(*head, &tmp);
	// メソッドでnilのはずがない!?
	if (!OBJECT_IS_CONS(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
	for (p=OBJECT_GET_CELL(&tmp); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		if (OBJECT_IS_CONS(&tmp)) {
			tPCELL pp;
			tOBJECT name, specializer;
			pp=OBJECT_GET_CELL(&tmp);
			cons_get_car(pp, &name);
			if (!OBJECT_IS_SYMBOL(&name)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name); }
			if (!symbol_is_simple(OBJECT_GET_CELL(&name))) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER); }
			pp=cons_get_cdr_cons(pp);
			if (!pp) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
			cons_get_car(pp, &tmp);
			if (!OBJECT_IS_SYMBOL(&tmp)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp); }
			specializer=tmp;
			if (pplist_add_profiler(vm, *pplist, &name, &specializer)) { vm_pop(vm); return VM_ERROR; }
		} else if (OBJECT_IS_SYMBOL(&tmp)) {
			if (OBJECT_GET_CELL(&tmp)==KEYWORD_REST) {
				tOBJECT specializer;
				tPCELL pp;
				// :rest 変数
				p=cons_get_cdr_cons(p);
				if (!p) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
				cons_get_car(p, &tmp);
				if (!OBJECT_IS_SYMBOL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
				pp=cons_get_cdr_cons(p);
				if (pp) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
				// 修飾子は<list>にしとく
				OBJECT_SET_SYMBOL(&specializer, SYMBOL_LIST_CLASS);
				if (pplist_add_profiler(vm, *pplist, &tmp, &specializer)) { vm_pop(vm); return VM_ERROR; }
				pplist_set_rest(*pplist);
			} else {
				// :rest以外
				tOBJECT specializer;
				if (!OBJECT_IS_SYMBOL(&tmp)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp); }
				if (!symbol_is_simple(OBJECT_GET_CELL(&tmp))) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER); }
				// 省略時は<object>
				OBJECT_SET_SYMBOL(&specializer, SYMBOL_OBJECT_CLASS);
				if (pplist_add_profiler(vm, *pplist, &tmp, &specializer)) { vm_pop(vm); return VM_ERROR; }
			}
		} else {
			vm_pop(vm);
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		}
	}
	pplist_finish_initialization(*pplist);
	*head=cons_get_cdr_cons(*head);
	vm_pop(vm);
	return VM_OK;
}

// 特殊形式
static VM_RET t1_special_form(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	return (*t1_special_operator_table[id])(vm, id, operands, clist, tail);
}

/////////////////////////////

static VM_RET t1_syntax_check(tPVM vm, tPOBJECT operands, const tINT n)
{
	tINT i=0;
	tPCELL p;
	if (OBJECT_IS_CONS(operands)) {
		tOBJECT obj;
		for (p=OBJECT_GET_CELL(operands), i=1; p;) {
			cons_get_cdr(p, &obj);
			if (OBJECT_IS_NIL(&obj)) {
				p=0;
			} else if (OBJECT_IS_CONS(&obj)) {
				p=OBJECT_GET_CELL(&obj);
				i++;
			} else {
				return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
			}
		}
	} else if (OBJECT_IS_NIL(operands)) {
		i=0;
		p=0;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	}
	//引数の数が合わない
	if (i!=n) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	return VM_OK;
}

/////////////////////////////

// (function _name_)
static VM_RET t1_function(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT name;
	tPCELL flist;
	tINT code;
	// 構文の検査
	if (t1_syntax_check(vm, operands, 1)) return VM_ERROR;
	//
	cons_get_car(OBJECT_GET_CELL(operands), &name);
	// nameは記号
	if (!OBJECT_IS_SYMBOL(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	if (symbol_is_simple(OBJECT_GET_CELL(&name))&&
		t1_search_function(vm, OBJECT_GET_CELL(&name), &flist)) {
		tOBJECT obj;
		OBJECT_SET_CONS(&obj, flist);
		return code_list_add_command_1(vm, clist, iPUSH_LOCAL_FUNCTION)||
			   code_list_add_argument(vm, clist, &name)||
			   code_list_add_argument(vm, clist, &obj);
	} else {
		code=iPUSH_FUNCTION;
		return code_list_add_command_1(vm, clist, iPUSH_FUNCTION)||
			   code_list_add_argument(vm, clist, &name);
	}
}

static VM_RET t1_lambda(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL flist;
	VM_RET ret;
	tOBJECT obj;

	if (t1_lambda_to_function_list(vm, operands, &flist)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, flist);
	if (vm_push(vm, &obj)) return VM_ERROR;
	ret=code_list_add_command_1(vm, clist, iPUSH_LAMBDA)||
		code_list_add_argument(vm, clist, &obj);
	vm_pop(vm);
	return ret;
}

static VM_RET t1_no_local_function(tPVM vm, tPCELL p, tPCELL clist, const tBOOL tail)
{// flet labelsで局所関数を作成しない場合
	tOBJECT obj;
	cons_get_cdr(p, &obj);
	if (OBJECT_IS_NIL(&obj)) {
		// (labels ())
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	} else if (OBJECT_IS_CONS(&obj)) {
		// -> (progn form*)
		return t1_progn(vm, bPROGN, &obj, clist, tail);
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
}

static VM_RET t1_check_local_function_head(tPVM vm, tPCELL head, tPCELL nlist)
{
	tOBJECT obj, obj2;
	tPCELL func, list, pp;

	cons_get_car(head, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	func=OBJECT_GET_CELL(&obj);
	// function-name
	cons_get_car(func, &obj);
	if (!OBJECT_IS_SYMBOL(&obj)||
		!symbol_is_simple(OBJECT_GET_CELL(&obj))) {
		//構文エラー？
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj);
	}
	cons_get_cdr(head, &obj2);
	if (!OBJECT_IS_CONS(&obj2)&&!OBJECT_IS_NIL(&obj2)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	// 名前の追加
	for (pp=nlist; cons_get_cdr_cons(pp); pp=cons_get_cdr_cons(pp)) {
		cons_get_car(cons_get_cdr_cons(pp), &obj2);
		if (OBJECT_GET_CELL(&obj)==OBJECT_GET_CELL(&obj2)) return signal_violation(vm, TRANSLATOR_ERROR_SAME_NAME_FUNCTION, translator_get_form(vm));
	}
	if (cons_create(vm, &list, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, list);
	cons_set_cdr(pp, &obj);
	return VM_OK;
}

// (labels ((fname lambda-list form*)*) form*)
static VM_RET t1_labels(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p, pp, flist, pf;
	tOBJECT obj;
	tINT i, n;

	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (OBJECT_IS_NIL(&obj)) return t1_no_local_function(vm, p, clist, tail);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));

	if (code_list_add_command_1(vm, clist, iLABELS_IN)) return VM_ERROR;
	OBJECT_SET_INTEGER(&obj, 0);
	if (cons_create(vm, &pp, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, pp);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	// pf : function-name-list
	/*!!!*/pf=cons_get_cdr_cons(cons_get_car_cons(clist));/*!!!*/
	// local-functions
	// 最初に関数名とラムダリストの部分までの解析を行う．
	// 局所関数の本体部は一度全部の局所関数を見た後で変換を行う．
	for (flist=cons_get_car_cons(p), n=0; flist; flist=cons_get_cdr_cons(flist), n++) {
		tPCELL p3, p4, plist;
		if (t1_check_local_function_head(vm, flist, pp)) return VM_ERROR;
		// 関数の引数の読込み
		p3=cons_get_car_cons(flist);
		cons_get_cdr(p3, &obj);
		if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		p3=OBJECT_GET_CELL(&obj);
		cons_get_car(p3, &obj);
		if (t1_lambda_list(vm, &obj, &plist)) return VM_ERROR;
		if (function_list_create_with_plist(vm, plist, &p4)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, p4);
		if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	}
	// 新しい局所環境のもとで局所関数の本体部を変換する
	if (t1_push_function(vm, pf)) return VM_ERROR;
	flist=cons_get_car_cons(p);
	function_name_list_set_number(pf, n);
	for (i=0; i<n; i++) {
		tPCELL p3, p4, pl, pa=t1_get_argument_point(vm), cl;
		p3=cons_get_cdr_cons(cons_get_car_cons(flist));
		pf=cons_get_cdr_cons(pf);
		// 関数の追加
		cons_get_cdr(p3, &obj);
		p4=cons_get_car_cons(pf);
		pl=function_list_get_parameter_list(p4);
		t1_set_argument_point(vm, pl);
		if (t1_push_variable(vm, pl)) { t1_set_argument_point(vm, pa); t1_pop_function(vm); return VM_ERROR; }
		if (t1_code_list_(vm, &obj, &cl, tTRUE)) { t1_set_argument_point(vm, pa); t1_pop_variable(vm); t1_pop_function(vm); return VM_ERROR; }
		//
		function_list_set_code_list(p4, cl);
		//
		t1_set_argument_point(vm, pa);
		t1_pop_variable(vm);
		//次へ
		flist=cons_get_cdr_cons(flist);
	}
	// 本体部
	cons_get_cdr(p, &obj);
	if (t1_progn(vm, bPROGN, &obj, clist, tFALSE)) { t1_pop_function(vm); return VM_ERROR; }
	t1_pop_function(vm);
	return code_list_add_command_1(vm, clist, iLABELS_OUT);
}

// (flet ((function-name lambda-list form*)*) form*)
static VM_RET t1_flet(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p, pp, flist, pf;
	tOBJECT obj;
	
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (OBJECT_IS_NIL(&obj)) return t1_no_local_function(vm, p, clist, tail);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	if (code_list_add_command_1(vm, clist, iFLET_IN)) return VM_ERROR;
	OBJECT_SET_INTEGER(&obj, 0);
	if (cons_create(vm, &pp, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, pp);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	// pf : function-name-list
	/*!!!*/pf=cons_get_cdr_cons(cons_get_car_cons(clist));/*!!!*/
	// local-functions
	for (flist=cons_get_car_cons(p); flist; flist=cons_get_cdr_cons(flist)) {
		tPCELL p3, p4;
		if (t1_check_local_function_head(vm, flist, pp)) return VM_ERROR;
		// 関数の追加
		p3=cons_get_car_cons(flist);
		cons_get_cdr(p3, &obj);
		if (t1_lambda_to_function_list(vm, &obj, &p4)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, p4);
		if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
		function_name_list_increment_number(pf);
	}
	if (t1_push_function(vm, pf)) return VM_ERROR;
	cons_get_cdr(p, &obj);
	if (t1_progn(vm, bPROGN, &obj, clist, tFALSE)) { t1_pop_function(vm); return VM_ERROR; }
	t1_pop_function(vm);
	return code_list_add_command_1(vm, clist, iFLET_OUT);
}

static VM_RET t1_and_or(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p, pf;
	tOBJECT obj, obj2;
	tINT code;
	if (OBJECT_IS_NIL(operands)) {
		// (and) or (or)
		code=(id==bAND) ? iPUSH_T : iPUSH_NIL;
		return code_list_add_command_1(vm, clist, code);
	}
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	cons_get_cdr(p, &obj2);
	if (OBJECT_IS_NIL(&obj2))
		// (and form)
		return t1_evaluation_form(vm, &obj, clist, tail);
	// (and form1 form2 ... formn)
	// -> form1 iAND (and form2 ... formn)
	if (t1_evaluation_form(vm, &obj, clist, tFALSE)) return VM_ERROR;
	code = (id==bAND) ? iAND : iOR;
	if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
	// (and form2 .. formn)
	if (code_list_create(vm, &pf)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, pf);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	//
	if (t1_and_or(vm, id, &obj2, pf, tail)) return VM_ERROR;
	if (code_list_add_command_1(vm, pf, iRET)) return VM_ERROR;
	return VM_OK;
}

static VM_RET t1_quote(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj;
	if (t1_syntax_check(vm, operands, 1)) return VM_ERROR;
	cons_get_car(OBJECT_GET_CELL(operands), &obj);
	if (OBJECT_IS_NIL(&obj)) {
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	} else {
		return code_list_add_command_1(vm, clist, iPUSH_OBJECT)||
			   code_list_add_argument(vm, clist, &obj);
	}
}

// (setq var form)
static VM_RET t1_setq(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT var, form;
	tPCELL p;
	// 構文検査
	if (t1_syntax_check(vm, operands, 2)) return VM_ERROR;
	// 引数の抜き出し
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &var);
	cons_get_car(cons_get_cdr_cons(p), &form);
	// 
	return t1_setq_(vm, &var, &form, clist);
}

// setfのために分離されている
static VM_RET t1_setq_(tPVM vm, tPOBJECT var, tPOBJECT form, tPCELL clist)
{// 変数varをformの評価結果の値にする
	tINT code;
	if (!OBJECT_IS_SYMBOL(var)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, var);
	if (t1_evaluation_form(vm, form, clist, tFALSE)) return VM_ERROR;
	if (symbol_is_simple(OBJECT_GET_CELL(var))&&
		t1_search_variable(vm, OBJECT_GET_CELL(var))) {
		code=iSET_LOCAL_VARIABLE;
	} else {
		code=iSET_GLOBAL_VARIABLE;
	}
	return code_list_add_command_1(vm, clist, code)||
		   code_list_add_argument(vm, clist, var);
}

// (setf place form)
static VM_RET t1_setf(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p;
	tOBJECT form, place;
	// 構文検査
	if (t1_syntax_check(vm, operands, 2)) return VM_ERROR;
	// 引数の抜き出し
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &place);
	cons_get_car(cons_get_cdr_cons(p), &form);
	// マクロ展開のために分ける
	return t1_setf_(vm, &place, &form, clist);
}

static VM_RET t1_setf_(tPVM vm, tPOBJECT place, tPOBJECT form, tPCELL clist)
{
	tOBJECT obj;

	if (OBJECT_IS_SYMBOL(place)) {
		// placeが識別子の場合
		return t1_setq_(vm, place, form, clist);
	} else if (OBJECT_IS_CONS(place)) {
		tPCELL p;
		// place
		cons_get_car(OBJECT_GET_CELL(place), &obj);
		if (!OBJECT_IS_SYMBOL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj);
		p=OBJECT_GET_CELL(&obj);
		if (p==SYMBOL_DYNAMIC) {
			// set-dynamic
			return t1_setf_dynamic(vm, OBJECT_GET_CELL(place), form, clist);
		} else if ((p==SYMBOL_AREF)||(p==SYMBOL_GAREF)) {
			return t1_setf_aref(vm, OBJECT_GET_CELL(place), form, clist);
		} else if ((p==SYMBOL_ELT)||(p==SYMBOL_PROPERTY)) {
			return t1_setf_elt(vm, OBJECT_GET_CELL(place), form, clist);
		} else if ((p==SYMBOL_CAR)||(p==SYMBOL_CDR)) {
			return t1_setf_cons(vm, OBJECT_GET_CELL(place), form, clist);
		}
		// マクロの場合
		{
			tPCELL blist, bind;
			if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), p, &blist)) return VM_ERROR;
			bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
			if (bind) {
				bind_get_object(bind, NAMESPACE_FUNCTION, &obj);
				if (OBJECT_IS_MACRO(&obj)) {
					tOBJECT arg;
					// マクロ
					cons_get_cdr(OBJECT_GET_CELL(place), &arg);
					if (t_macro_expand(vm, OBJECT_GET_CELL(&obj), &arg, &obj)) return VM_ERROR;
					// 展開して分岐しなおす
					return t1_setf_(vm, &obj, form, clist);
				}
			}
		}
		// マクロでもない場合
		if (!symbol_is_simple(p)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		return t1_setf_accessor(vm, OBJECT_GET_CELL(place), form, clist);
	} else {// placeが変
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
}

// (sef (dynamic var) form)
static VM_RET t1_setf_dynamic(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist)
{
	tOBJECT obj;
	cons_get_cdr(place, &obj);
	if (t1_syntax_check(vm, &obj, 1)) return VM_ERROR;
	cons_get_car(cons_get_cdr_cons(place), &obj);
	if (!OBJECT_IS_SYMBOL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj);
	// formの評価
	if (t1_evaluation_form(vm, form, clist, tFALSE)) return VM_ERROR;
	// iSET_DYNAMIC name
	return code_list_add_command_1(vm, clist, iSET_DYNAMIC)||
		   code_list_add_argument(vm, clist, &obj);
}

// (setf (aref array z1 ... zn) form)
static VM_RET t1_setf_aref(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist)
{
	tOBJECT array, z, obj;
	tINT code, n;
	tPCELL p;
	cons_get_cdr(place, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &array);
	// form
	if (t1_evaluation_form(vm, form, clist, tFALSE)) return VM_ERROR;
	// array
	if (t1_evaluation_form(vm, &array, clist, tFALSE)) return VM_ERROR;
	// z1 .. zn
	n=2;
	cons_get_cdr(p, &obj);
	if (OBJECT_IS_CONS(&obj)) {
		p=OBJECT_GET_CELL(&obj);
		while (p) {
			n++;
			cons_get_car(p, &obj);
			if (t1_evaluation_form(vm, &obj, clist, tFALSE)) return VM_ERROR;
			cons_get_cdr(p, &obj);
			if (OBJECT_IS_CONS(&obj))
				p=OBJECT_GET_CELL(&obj);
			else if (OBJECT_IS_NIL(&obj))
				p=0;
			else
				return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
	} else if (!OBJECT_IS_NIL(&obj)) {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
	// iSET_AREF n+2
	cons_get_car(place, &obj);
	code = (OBJECT_GET_CELL(&obj)==SYMBOL_AREF) ? iSET_AREF : iSET_GAREF;
	OBJECT_SET_INTEGER(&z, n);
	return code_list_add_command_1(vm, clist, code)||
		   code_list_add_argument(vm, clist, &z);

}

// (setf (elt seq z) form)
static VM_RET t1_setf_elt(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist)
{
	tOBJECT obj, seq, z;
	tPCELL p;
	tINT command;

	cons_get_cdr(place, &obj);
	if (t1_syntax_check(vm, &obj, 2)) return VM_ERROR;
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &seq);
	cons_get_car(cons_get_cdr_cons(p), &z);
	// form seq z
	if (t1_evaluation_form(vm, form, clist, tFALSE)) return VM_ERROR;
	if (t1_evaluation_form(vm, &seq, clist, tFALSE)) return VM_ERROR;
	if (t1_evaluation_form(vm, &z, clist, tFALSE)) return VM_ERROR;
	// iSET_ELT | iSET_PRPERTY
	cons_get_car(place, &obj);
	command= (OBJECT_GET_CELL(&obj)==SYMBOL_ELT) ? iSET_ELT : iSET_PROPERTY;
	return code_list_add_command_1(vm, clist, command);
}

// (setf (car cons) form)
static VM_RET t1_setf_cons(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist)
{
	tOBJECT obj, cons;
	tINT command;

	cons_get_cdr(place, &obj);
	if (t1_syntax_check(vm, &obj, 1)) return VM_ERROR;
	cons_get_car(OBJECT_GET_CELL(&obj), &cons);
	// form cons
	if (t1_evaluation_form(vm, form, clist, tFALSE)) return VM_ERROR;
	if (t1_evaluation_form(vm, &cons, clist, tFALSE)) return VM_ERROR;
	// iSET_CAR | iSET_CDR
	cons_get_car(place, &obj);
	command=(OBJECT_GET_CELL(&obj)==SYMBOL_CAR) ? iSET_CAR : iSET_CDR;
	return code_list_add_command_1(vm, clist, command);
}

// (setf (reader-function-name instance) form)
static VM_RET t1_setf_accessor(tPVM vm, tPCELL place, tPOBJECT form, tPCELL clist)
{
	tOBJECT obj, name, instance;

	cons_get_cdr(place, &obj);
	if (t1_syntax_check(vm, &obj, 1)) return VM_ERROR;
	//
	cons_get_car(place, &name);
	cons_get_car(cons_get_cdr_cons(place), &instance);
	// nameは単純記号 パッケージ修飾子は不可(今のところ)
	if (!OBJECT_IS_SYMBOL(&name)||
		!symbol_is_simple(OBJECT_GET_CELL(&name))) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	// form instance
	if (t1_evaluation_form(vm, form, clist, tFALSE)) return VM_ERROR;
	if (t1_evaluation_form(vm, &instance, clist, tFALSE)) return VM_ERROR;
	//
	return code_list_add_command_1(vm, clist, iACCESSOR)||
		   code_list_add_argument(vm, clist, &name);
}

// (let ((name form)*) form*)
static VM_RET t1_let(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p, pp, p3, plist;
	tOBJECT obj;

	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (OBJECT_IS_NIL(&obj)) {
		// (let () form*) -> (progn form*)
		cons_get_cdr(p, &obj);
		return t1_progn(vm, bPROGN, &obj, clist, tail);
	}
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	// parameter-list
	if (parameter_list_create(vm, &plist)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, plist);
	if (vm_push(vm, &obj)) return VM_ERROR;
	pp=cons_get_car_cons(p);
	while (pp) {
		tOBJECT var, form;
		cons_get_car(pp, &obj);
		if (!OBJECT_IS_CONS(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
		p3=OBJECT_GET_CELL(&obj);
		// var
		cons_get_car(p3, &var);
		if (!OBJECT_IS_SYMBOL(&var)||!symbol_is_simple(OBJECT_GET_CELL(&var))) {
			vm_pop(vm);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &var);
		}
		// form
		cons_get_cdr(p3, &obj);
		if (OBJECT_IS_NIL(&obj)) {
			// nil
			if (code_list_add_command_1(vm, clist, iPUSH_NIL)) { vm_pop(vm); return VM_ERROR; }
		} else if (OBJECT_IS_CONS(&obj)) {
			cons_get_cdr(OBJECT_GET_CELL(&obj), &form);
			if (!OBJECT_IS_NIL(&form)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
			cons_get_car(OBJECT_GET_CELL(&obj), &form);
			if (t1_evaluation_form(vm, &form, clist, tFALSE)) { vm_pop(vm); return VM_ERROR; }
		}
		// 仮引数リストに追加
		if (parameter_list_add_parameter(vm, plist, &var)) { vm_pop(vm); return VM_ERROR; }
		// 次へ
		cons_get_cdr(pp, &obj);
		if (OBJECT_IS_NIL(&obj)) {
			pp=0;
		} else if (OBJECT_IS_CONS(&obj)) {
			pp=OBJECT_GET_CELL(&obj);
		} else {
			vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
	}
	// parameter-listの作成終了
	vm_pop(vm);
	parameter_list_finish_initialization(plist);
	// 仮引数を環境に追加
	if (t1_push_variable(vm, plist)) return VM_ERROR;
	// iLAMBDA_IN plist
	OBJECT_SET_CONS(&obj, plist);
	if (code_list_add_command_1(vm, clist, iLAMBDA_IN)||
		code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
	// form*
	cons_get_cdr(p, &obj);
	if (t1_sequence(vm, &obj, clist, tFALSE)) { t1_pop_variable(vm); return VM_ERROR; }
	// 仮引数削除
	t1_pop_variable(vm);
	OBJECT_SET_CONS(&obj, plist);
	// iLAMBDA_OUT plist
	return code_list_add_command_1(vm, clist, iLAMBDA_OUT)||
		   code_list_add_argument(vm, clist, &obj);
}

static VM_RET t1_leta(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj;
	tPCELL p;

	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (OBJECT_IS_NIL(&obj)) {
		// (let* () form*) -> (progn form*)
		cons_get_cdr(p, &obj);
		return t1_progn(vm, bPROGN, &obj, clist, tail);
	} else if (OBJECT_IS_CONS(&obj)) {
		tOBJECT body;
		cons_get_cdr(p, &body);
		return t1_letb(vm, &obj, &body, clist, tFALSE);
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	}
}

static VM_RET t1_letb(tPVM vm, tPOBJECT parameter, tPOBJECT body, tPCELL clist, const tBOOL tail)
{
	if (OBJECT_IS_CONS(parameter)) {
		// 仮引数を一つ変換する
		tPCELL p, pp, plist;
		tOBJECT obj, name, form;
		// 仮引数リストを作成し，保護しておく
		if (parameter_list_create(vm, &plist)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, plist);
		if (vm_push(vm, &obj)) return VM_ERROR;
		// 各引数の抜き出しと構文チェック
		p=OBJECT_GET_CELL(parameter);
		cons_get_car(p, &obj);
		if (!OBJECT_IS_CONS(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
		pp=OBJECT_GET_CELL(&obj);
		cons_get_car(pp, &name);
		cons_get_cdr(pp, &obj);
		if (!OBJECT_IS_CONS(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
		pp=OBJECT_GET_CELL(&obj);
		cons_get_car(pp, &form);
		cons_get_cdr(pp, &obj);
		if (!OBJECT_IS_NIL(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
		// form
		if (t1_evaluation_form(vm, &form, clist, tFALSE)) { vm_pop(vm); return VM_ERROR; }
		// 仮引数リストの作成
		if (parameter_list_add_parameter(vm, plist, &name)) { vm_pop(vm); return VM_ERROR; }
		parameter_list_finish_initialization(plist);
		// iLAMBDA_IN plist
		vm_pop(vm);
		if (t1_push_variable(vm, plist)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, plist);
		if (code_list_add_command_1(vm, clist, iLAMBDA_IN)||
			code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
		// 次の引数へ
		cons_get_cdr(p, &obj);
		if (t1_letb(vm, &obj, body, clist, tail)) { t1_pop_variable(vm); return VM_ERROR; }
		// iLAMBDA_OUT plist
		OBJECT_SET_CONS(&obj, plist);
		t1_pop_variable(vm);
		return code_list_add_command_1(vm, clist, iLAMBDA_OUT)||
			code_list_add_argument(vm, clist, &obj);
	} else if (OBJECT_IS_NIL(parameter)) {
		// letの本体部の展開
		return t1_sequence(vm, body, clist, tail);
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
}

// (dynamic name)
static VM_RET t1_dynamic(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT name;

	if (t1_syntax_check(vm, operands, 1)) return VM_ERROR;
	cons_get_car(OBJECT_GET_CELL(operands), &name);
	if (!OBJECT_IS_SYMBOL(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	// iPUSH_DYNAMIC name
	return code_list_add_command_1(vm, clist, iPUSH_DYNAMIC)||
		   code_list_add_argument(vm, clist, &name);
}

// (dynamic-let ((var form)*) form*)
static VM_RET t1_dynamic_let(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p, pl;
	tOBJECT obj, form;
	tINT n;
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	cons_get_cdr(p, &form);
	if (OBJECT_IS_NIL(&obj)) {
		// (dynamic-let () ...) -> (progn ...)
		return t1_sequence(vm, &form, clist, tail);
	} if (!OBJECT_IS_CONS(&obj)) {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
	//
	n=0;
	pl=OBJECT_GET_CELL(&obj);
	while (pl) {
		tPCELL pp;
		// (var form)
		cons_get_car(pl, &obj);
		if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		pp=OBJECT_GET_CELL(&obj);
		// var
		n++;
		cons_get_car(pp, &obj);
		if (!OBJECT_IS_SYMBOL(&obj)||!symbol_is_simple(OBJECT_GET_CELL(&obj)))
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &obj);
		// (form)
		cons_get_cdr(pp, &obj);
		if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		pp=OBJECT_GET_CELL(&obj);
		cons_get_car(pp, &obj);
		if (t1_evaluation_form(vm, &obj, clist, tFALSE)) return VM_ERROR;
		cons_get_cdr(pp, &obj);
		if (!OBJECT_IS_NIL(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		//
		cons_get_cdr(pl, &obj);
		if (OBJECT_IS_CONS(&obj)) {
			pl=OBJECT_GET_CELL(&obj);
		} else if (OBJECT_IS_NIL(&obj)) {
			pl=0;
		} else {
			return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
	}
	// iDYNAMIC_LET n symbol1 ... symboln code-list
	if (code_list_add_command_1(vm, clist, iDYNAMIC_LET)) return VM_ERROR;
	OBJECT_SET_INTEGER(&obj, n);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	for (pl=cons_get_car_cons(p); pl; pl=cons_get_cdr_cons(pl))  {
		cons_get_car(cons_get_car_cons(pl), &obj);
		if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	}
	if (t1_code_list_(vm, &form, &p, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	return code_list_add_argument(vm, clist, &obj);
}

// (if test then else)
static VM_RET t1_if(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT test_form, then_form, else_form, obj;
	tPCELL p, then_list, else_list;
	// 構文検査と引数の抜き出し
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &test_form);
	//
	cons_get_cdr(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &then_form);
	//
	cons_get_cdr(p, &obj);
	if (OBJECT_IS_CONS(&obj)) {
		p=cons_get_cdr_cons(p);
		cons_get_car(p, &else_form);
		//
		cons_get_cdr(p, &obj);
		if (!OBJECT_IS_NIL(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	} else if (OBJECT_IS_NIL(&obj)) {
		else_form=nil;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
	// test
	if (t1_evaluation_form(vm, &test_form, clist, tFALSE)) return VM_ERROR;
	// iIF then_list else_list 
	if (code_list_add_command_1(vm, clist, iIF)) return VM_ERROR;
	// then_list
	if (code_list_create(vm, &then_list)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, then_list);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	if (t1_evaluation_form(vm, &then_form, then_list, tail)) return VM_ERROR;
	/*!!!*/if (code_list_add_command_1(vm, then_list, iRET)) return VM_ERROR;
	// else_list
	if (code_list_create(vm, &else_list)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, else_list);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	if (t1_evaluation_form(vm, &else_form, else_list, tail)) return VM_ERROR;
	/*!!!*/if (code_list_add_command_1(vm, else_list, iRET)) return VM_ERROR;

	return VM_OK;
}

static VM_RET t1_cond(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj, test;
	tPCELL p, p2, cl;

	if (OBJECT_IS_NIL(operands)) // (cond)
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	if (!OBJECT_IS_CONS(operands))
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p2=OBJECT_GET_CELL(&obj);
	cons_get_car(p2, &test);
	cons_get_cdr(p2, &obj);
	if (OBJECT_IS_NIL(&obj)) {
		// (cond (test) (..)...)  -> (or test (cond (...) ...))
		if (t1_evaluation_form(vm, &test, clist, tFALSE)) return VM_ERROR;
		if (code_list_add_command_1(vm, clist, iOR)) return VM_ERROR;
		// or_code_list
	} else {
		// (cond (test ...) (...) ...) -> (if test1 (progn ...) (cond (...) ...)
		if (t1_evaluation_form(vm, &test, clist, tFALSE)) return VM_ERROR;
		if (code_list_add_command_1(vm, clist, iIF)) return VM_ERROR;
		//
		if (t1_code_list_(vm, &obj, &cl, tail)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, cl);
		// then_code_list
		if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
		// else_code_list
	}
	// (cond (...) ...)
	if (code_list_create(vm, &cl)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	// 残りを新しいcond形式として変換
	cons_get_cdr(p, &obj);
	if (t1_cond(vm, bCOND, &obj, cl, tail)) return VM_ERROR;
	if (code_list_add_command_1(vm, cl, iRET)) return VM_ERROR;

	return VM_OK;
}

// (case keyform ((key*) form*)* [(t form*)])
static VM_RET t1_case(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj;
	tPCELL p, pp;
	tINT code, i, n;

	// keyform
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (t1_evaluation_form(vm, &obj, clist, tFALSE)) return VM_ERROR;
	//
	cons_get_cdr(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) {
		if (!OBJECT_IS_NIL(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		// (case keyform)?
		return code_list_add_command_1(vm, clist, iDISCARD)||
			   code_list_add_command_1(vm, clist, iPUSH_NIL);
	}
	// ((key*) form*)* [(t form*)]
	p=OBJECT_GET_CELL(&obj);
	// iCASE n key-list clist .... key-list clist
	code=(id==bCASE) ? iCASE : iCASE_USING;
	if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
	// ((key*)form*)*の数
	n=cons_get_length(p);
	OBJECT_SET_INTEGER(&obj, n);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	//
	for (i=0; i<n; i++) {
		tOBJECT keylist, form;
		cons_get_car(p, &obj);
		if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		pp=OBJECT_GET_CELL(&obj);
		cons_get_car(pp, &keylist);
		cons_get_cdr(pp, &form);
		if (OBJECT_IS_CONS(&keylist)) {
			pp=OBJECT_GET_CELL(&keylist);
			// key-listの検査
			while (pp) {
				cons_get_cdr(pp, &obj);
				if (OBJECT_IS_CONS(&obj)) pp=OBJECT_GET_CELL(&obj);
				else if (OBJECT_IS_NIL(&obj)) pp=0;
				else return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
			}
			if (code_list_add_argument(vm, clist, &keylist)) return VM_ERROR;
			if (t1_code_list_(vm, &form, &pp, tFALSE)) return VM_ERROR;
			OBJECT_SET_CONS(&obj, pp);
			if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
		} else if ((i==n-1)&&OBJECT_IS_SYMBOL(&keylist)&&(OBJECT_GET_CELL(&keylist)==SYMBOL_T)) {
			OBJECT_SET_INTEGER(&obj, 0);
			if (code_list_add_argument(vm, clist, &nil)) return VM_ERROR;
			if (t1_code_list_(vm, &form, &pp, tFALSE)) return VM_ERROR;
			OBJECT_SET_CONS(&obj, pp);
			if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
		} else {
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
		}
		p=cons_get_cdr_cons(p);
	}
	return VM_OK;
}
// (case-using predform keyform ((key*) form*)* [(r form*)])
static VM_RET t1_case_using(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT predform, obj;
	tPCELL p;
	if (!OBJECT_IS_CONS(operands)) return VM_ERROR;
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &predform);
	if (t1_evaluation_form(vm, &predform, clist, tFALSE)) return VM_ERROR;
	cons_get_cdr(p, &obj);
	// 以下はcase-usingと同じ
	return t1_case(vm, id, &obj, clist, tail);
}

static VM_RET t1_progn(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	return t1_sequence(vm, operands, clist, tail);
}

// (while test body*)
static VM_RET t1_while(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT test, body, obj;
	tPCELL p;
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &test);
	cons_get_cdr(p, &body);
	//
	if (code_list_add_command_1(vm, clist, iWHILE)) return VM_ERROR;
	//
	if (t1_code_list(vm, &test, &p, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	//
	if (t1_code_list_(vm, &body, &p, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;

	return VM_OK;
}

// (for (iteration-spec*) (end-test result*) form*)
static VM_RET t1_for(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj, form, endtest, result;
	tPCELL p, pp, iteration_spec, end, plist, cl;

	// 構文の検査と引数の抽出
	// (iteration-spec*)
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	iteration_spec=OBJECT_GET_CELL(&obj);
	// (end-test result*)
	cons_get_cdr(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	end=OBJECT_GET_CELL(&obj);
	// form*
	cons_get_cdr(p, &form);
	//
	cons_get_car(end, &endtest);
	cons_get_cdr(end, &result);
	//
	if (parameter_list_create(vm, &plist)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, plist);
	if (vm_push(vm, &obj)) return VM_ERROR;
	// initを変換しながらplistにvarを追加していくstepは後で
	p=iteration_spec;
	while (p) {// iteration-spec ::= (var init [step])
		tOBJECT var, init;
		cons_get_car(p, &obj);
		/// var
		if (!OBJECT_IS_CONS(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
		pp=OBJECT_GET_CELL(&obj);
		cons_get_car(pp, &var);
		if (!OBJECT_IS_SYMBOL(&var)||!symbol_is_simple(OBJECT_GET_CELL(&var))) {
			vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &var);
		}
		// 追加
		if (parameter_list_add_parameter(vm, plist, &var)) { vm_pop(vm); return VM_ERROR; }
		// init
		cons_get_cdr(pp, &obj);
		if (!OBJECT_IS_CONS(&obj)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
		pp=OBJECT_GET_CELL(&obj);
		cons_get_car(pp, &init);
		if (t1_evaluation_form(vm, &init, clist, tFALSE)) { vm_pop(vm); return VM_ERROR; }
		// stepは後で
		cons_get_cdr(p, &obj);
		if (OBJECT_IS_CONS(&obj)) {
			p=OBJECT_GET_CELL(&obj);
		} else if (OBJECT_IS_NIL(&obj)) {
			p=0;
		} else {
			vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
	}
	//
	parameter_list_finish_initialization(plist);
	vm_pop(vm);
	if (t1_push_variable(vm, plist)) return VM_ERROR;
	// iFOR plist endtest-clist result-clist iterate-clist
	if (code_list_add_command_1(vm, clist, iFOR)) { t1_pop_variable(vm); return VM_ERROR; }
	// plist
	OBJECT_SET_CONS(&obj, plist);
	if (code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
	// end-test-clist
	if (t1_code_list(vm, &endtest, &cl, tFALSE)) { t1_pop_variable(vm); return VM_ERROR; }
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
	// result-clist
	if (t1_code_list_(vm, &result, &cl, tFALSE)) { t1_pop_variable(vm); return VM_ERROR; }
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
	// iterate-clist
	if (code_list_create(vm, &cl)) { t1_pop_variable(vm); return VM_ERROR; }
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
	if (t1_sequence(vm, &form, cl, tFALSE)||
		code_list_add_command_1(vm, cl, iiDISCARD)) { t1_pop_variable(vm); return VM_ERROR; }
	for (p=iteration_spec; p; p=cons_get_cdr_cons(p)) {
		tOBJECT var;
		pp=cons_get_car_cons(p);
		cons_get_car(pp, &var);
		pp=cons_get_cdr_cons(pp);
		cons_get_cdr(pp, &obj);
		if (OBJECT_IS_NIL(&obj)) {
			// step省略時
			if (t1_evaluation_form(vm, &var, cl, tFALSE)) { t1_pop_variable(vm); return VM_ERROR; }
		} else if (OBJECT_IS_CONS(&obj)) {
			tOBJECT step;
			pp=OBJECT_GET_CELL(&obj);
			cons_get_car(pp, &step);
			cons_get_cdr(pp, &obj);
			if (!OBJECT_IS_NIL(&obj)) { t1_pop_variable(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm)); }
			if (t1_evaluation_form(vm, &step, cl, tFALSE)) { t1_pop_variable(vm); return VM_ERROR; }
		} else {
			t1_pop_variable(vm);
			return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
	}
	t1_pop_variable(vm);
	return code_list_add_command_1(vm, cl, iRET);
}

// (block name form*)
static VM_RET t1_block(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT name, form, obj;
	tPCELL p, cl;
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &name);
	cons_get_cdr(p, &form);
	// nameは単純記号
	if (!OBJECT_IS_SYMBOL(&name)||
		!symbol_is_simple(OBJECT_GET_CELL(&name))) {
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	}
	if (t1_push_block(vm, &name)) return VM_ERROR;
	// iBLOCK clist block-tag
	if (code_list_add_command_1(vm, clist, iBLOCK)) return VM_ERROR;
	if (t1_code_list_(vm, &form, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	t1_pop_block(vm, &p);
	OBJECT_SET_CONS(&obj, p);
	return code_list_add_argument(vm, clist, &obj);
}

// (return-from name result-form)
static VM_RET t1_return_from(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT name, form, obj;
	tPCELL p, block;
	// 構文検査
	if (t1_syntax_check(vm, operands, 2)) return VM_ERROR;
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &name);
	cons_get_car(cons_get_cdr_cons(p), &form);
	// 脱出先の静的な確認
	if (!t1_search_block(vm, &name, &block)) {
		return signal_violation(vm, TRANSLATOR_ERROR_BAD_BLOCK, translator_get_form(vm));
	}
	// formの変換
	if (t1_evaluation_form(vm, &form, clist, tFALSE)) return VM_ERROR;
	// iRETURN_FROM block-tag
	OBJECT_SET_CONS(&obj, block);
	return code_list_add_command_1(vm, clist, iRETURN_FROM)||
		   code_list_add_argument(vm, clist, &obj);
}

// (catch tag-form form*)
static VM_RET t1_catch(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p, cl;
	tOBJECT obj, tag, form;
	// 構文検査
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &tag);
	cons_get_cdr(p, &form);
	// tagの評価
	if (t1_evaluation_form(vm, &tag, clist, tFALSE)) return VM_ERROR;
	// iCATCH code-list
	if (code_list_add_command_1(vm, clist, iCATCH)) return VM_ERROR;
	if (t1_code_list_(vm, &form, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	return code_list_add_argument(vm, clist, &obj);
}

// (throw tag-form result-form)
static VM_RET t1_throw(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT tag, form;
	tPCELL p;
	// 構文検査
	if (t1_syntax_check(vm, operands, 2)) return VM_ERROR;
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &tag);
	cons_get_car(cons_get_cdr_cons(p), &form);
	// tagの評価
	if (t1_evaluation_form(vm, &tag, clist, tFALSE)) return VM_ERROR;
	// resultの評価
	if (t1_evaluation_form(vm, &form, clist, tFALSE)) return VM_ERROR;
	// iTHROW
	return code_list_add_command_1(vm, clist, iTHROW);
}

// (tagbody { tagbody-tag | form }*)
static VM_RET t1_tagbody(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL tag_list, p;
	tOBJECT obj;
	tINT i, n, f;
	//
	if (OBJECT_IS_NIL(operands)) {
		// (tagbody)
		return code_list_add_command_1(vm, clist, iPUSH_NIL);
	} else if (!OBJECT_IS_CONS(operands)) {
		return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	}
	p=OBJECT_GET_CELL(operands);
	// tagbodytag_list の作成
	if (head_list_create(vm, &tag_list)) return VM_ERROR;
	if (t1_push_tagbody(vm, tag_list)) return VM_ERROR;
	//
	i=1;// tagから始まるか否か
	while (p) {
		cons_get_car(p, &obj);
		if (OBJECT_IS_SYMBOL(&obj)&&symbol_is_simple(OBJECT_GET_CELL(&obj))) {
			// tagの追加
			// tagから始まっている
			if (p==OBJECT_GET_CELL(operands)) i=-1;
			if (head_list_add_object(vm, tag_list, &obj)) { t1_pop_tagbody(vm); return VM_ERROR; }
		}
		//
		cons_get_cdr(p, &obj);
		if (OBJECT_IS_CONS(&obj)) {
			p=OBJECT_GET_CELL(&obj);
		} else if (OBJECT_IS_NIL(&obj)) {
			p=0;
		} else {
			return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
		}
	}
	n=head_list_get_size(tag_list);
	//
	if (n) {
		tPCELL cl, list, pp;
		//
		if (i<0) tagbody_tag_list_set_start_tag(tag_list);
		// iTAGBOYD taglist (code-list .. code-list)
		if (code_list_add_command_1(vm, clist, iTAGBODY)) { t1_pop_tagbody(vm); return VM_ERROR; }
		OBJECT_SET_CONS(&obj, tag_list);
		if (code_list_add_argument(vm, clist, &obj)) { t1_pop_tagbody(vm); return VM_ERROR; }
		//
		f=0; i=0;
		if (code_list_create(vm, &cl)) { t1_pop_tagbody(vm); return VM_ERROR; }
		OBJECT_SET_CONS(&obj, cl);
		if (cons_create(vm, &list, &obj, &nil)) { t1_pop_tagbody(vm); return VM_ERROR; }
		OBJECT_SET_CONS(&obj, list);
		if (code_list_add_argument(vm, clist, &obj)) { t1_pop_tagbody(vm); return VM_ERROR; }
		for (p=OBJECT_GET_CELL(operands); p; p=cons_get_cdr_cons(p)) {
			cons_get_car(p, &obj);
			if (OBJECT_IS_SYMBOL(&obj)&&symbol_is_simple(OBJECT_GET_CELL(&obj))) {
				// tag
				if (f||i) {
					// tagが続いているときにはpush-nilを入れておく
					if (!f&&i&&code_list_add_command_1(vm, cl, iPUSH_NIL)) { t1_pop_tagbody(vm); return VM_ERROR; }
					if (code_list_add_command_1(vm, cl, iRET)) { t1_pop_tagbody(vm); return VM_ERROR; }
					if (code_list_create(vm, &cl)) { t1_pop_tagbody(vm); return VM_ERROR; }
					OBJECT_SET_CONS(&obj, cl);
					if (cons_create(vm, &pp, &obj, &nil)) { t1_pop_tagbody(vm); return VM_ERROR; }
					OBJECT_SET_CONS(&obj, pp);
					cons_set_cdr(list, &obj);
					list=pp;
				}
				f=0;// 最後はtag
				i=1;
			} else {
				// form
				if (f&&code_list_add_command_1(vm, cl, iDISCARD)) { t1_pop_tagbody(vm); return VM_ERROR; }
				if (t1_evaluation_form(vm, &obj, cl, tFALSE)) { t1_pop_tagbody(vm); return VM_ERROR; }
				f=1;// 最後はtag以外
			}
		}
		//
		if (!f&&code_list_add_command_1(vm, cl, iPUSH_NIL)) { t1_pop_tagbody(vm); return VM_ERROR; }
		if (code_list_add_command_1(vm, cl, iRET)) { t1_pop_tagbody(vm); return VM_ERROR; }
		//
		t1_pop_tagbody(vm);
		return VM_OK;
	} else {// tagが一つもなかった
		t1_pop_tagbody(vm);
		return t1_sequence(vm, operands, clist, tail);
	}
}

// (go tagbody-tag)
static VM_RET t1_go(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT tag, obj;
	tPCELL p, taglist;
	if (t1_syntax_check(vm, operands, 1)) return VM_ERROR;
	cons_get_car(OBJECT_GET_CELL(operands), &tag);
	if (!OBJECT_IS_SYMBOL(&tag)||!symbol_is_simple(OBJECT_GET_CELL(&tag)))
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tag);
	//
	if (!t1_search_tagbody(vm, OBJECT_GET_CELL(&tag), &taglist)) 
		return signal_violation(vm, TRANSLATOR_ERROR_BAD_TAGBODY_TAG, translator_get_form(vm));
	//
	OBJECT_SET_CONS(&obj, taglist);
	if (cons_create(vm, &p, &tag, &obj)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	return code_list_add_command_1(vm, clist, iGO)||
		   code_list_add_argument(vm, clist, &obj);
}

// (unwind-protect form cleanup-form*)
static VM_RET t1_unwind_protect(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT form, cleanup, obj;
	tPCELL p, cl;
	// 構文検査
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &form);
	cons_get_cdr(p, &cleanup);
	// iUNWIND_PROTECT form-code-list cleanup-code-list
	if (code_list_add_command_1(vm, clist, iUNWIND_PROTECT)) return VM_ERROR;
	if (t1_code_list(vm, &form, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	if (t1_code_list_(vm, &cleanup, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	return code_list_add_argument(vm, clist, &obj);
}

// (class name)
static VM_RET t1_class(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT name;
	//
	if (t1_syntax_check(vm, operands, 1)) return VM_ERROR;
	cons_get_car(OBJECT_GET_CELL(operands), &name);
	if (!OBJECT_IS_SYMBOL(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	//
	return code_list_add_command_1(vm, clist, iPUSH_CLASS)||
		   code_list_add_argument(vm, clist, &name);
}

// (assure class-name form)
static VM_RET t1_the(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT name, form;
	tPCELL p;
	tINT code;
	// 構文の検査
	if (t1_syntax_check(vm, operands, 2)) return VM_ERROR;
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &name);
	cons_get_car(cons_get_cdr_cons(p), &form);
	//
	if (!OBJECT_IS_SYMBOL(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	// 
	if (t1_evaluation_form(vm, &form, clist, tFALSE)) return VM_ERROR;
	code = (id==bTHE) ? iTHE : iASSURE;
	return code_list_add_command_1(vm, clist, code)||
		   code_list_add_argument(vm, clist, &name);
}

// (convert obj class-name)
static VM_RET t1_convert(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj, name;
	tPCELL p;
	// 構文の検査
	if (t1_syntax_check(vm, operands, 2)) return VM_ERROR;
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &obj);
	cons_get_car(cons_get_cdr_cons(p), &name);
	if (!OBJECT_IS_SYMBOL(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	//
	if (t1_evaluation_form(vm, &obj, clist, tFALSE)) return VM_ERROR;
	return code_list_add_command_1(vm, clist, iCONVERT)||
		   code_list_add_argument(vm, clist, &name);
}

static VM_RET t1_with_standard(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT stream, form, obj;
	tPCELL cl;
	tINT code;
	//　構文の検査
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	cl=OBJECT_GET_CELL(operands);
	cons_get_car(cl, &stream);
	cons_get_cdr(cl, &form);
	//
	if (t1_evaluation_form(vm, &stream, clist, tFALSE)) return VM_ERROR;
	switch (id) {
	case bWITH_STANDARD_INPUT:	code=iWITH_STANDARD_INPUT; break;
	case bWITH_STANDARD_OUTPUT:	code=iWITH_STANDARD_OUTPUT; break;
	case bWITH_ERROR_OUTPUT:	code=iWITH_ERROR_OUTPUT; break;
	default: return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
	if (t1_code_list_(vm, &form, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	return code_list_add_argument(vm, clist, &obj);
}

// (with-open-input-file (name filename [element-class]) form*)
static VM_RET t1_with_open_file(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT obj, form, name, file_name, element_class;
	tPCELL p, pl, cl;
	tINT code;
	// 構文の検査と引数の抜き出し
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_cdr(p, &form);
	cons_get_car(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &name);
	cons_get_cdr(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &file_name);
	cons_get_cdr(p, &obj);
	if (OBJECT_IS_CONS(&obj)) {
		p=OBJECT_GET_CELL(&obj);
		cons_get_car(p, &element_class);
		cons_get_cdr(p, &obj);
		if (!OBJECT_IS_NIL(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	} else if (OBJECT_IS_NIL(&obj)) {
		// [element-class]省略時は8にしておく
		OBJECT_SET_INTEGER(&element_class, 8);
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	}
	if (!OBJECT_IS_SYMBOL(&name)||!symbol_is_simple(OBJECT_GET_CELL(&name)))
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	//
	if (t1_evaluation_form(vm, &file_name, clist, tFALSE)) return VM_ERROR;
	if (t1_evaluation_form(vm, &element_class, clist, tFALSE)) return VM_ERROR;
	// iWITH_OPEN_FILE plist clist
	switch (id) {
	case bWITH_OPEN_INPUT_FILE:		code=iWITH_OPEN_INPUT_FILE; break;
	case bWITH_OPEN_OUTPUT_FILE:	code=iWITH_OPEN_OUTPUT_FILE; break;
	case bWITH_OPEN_IO_FILE:		code=iWITH_OPEN_IO_FILE; break;
	default: return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	// code
	if (code_list_add_command_1(vm, clist, code)) return VM_ERROR;
	// plist
	if (parameter_list_create(vm, &pl)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, pl);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (parameter_list_add_parameter(vm, pl, &name)) { vm_pop(vm); return VM_ERROR; }
	parameter_list_finish_initialization(pl);
	vm_pop(vm);
	OBJECT_SET_CONS(&obj, pl);
	if (code_list_add_argument(vm, clist, &obj)) return VM_ERROR;
	if (t1_push_variable(vm, pl)) return VM_ERROR;
	// clist
	if (t1_code_list_(vm, &form, &cl, tFALSE)) { t1_pop_variable(vm); return VM_ERROR; }
	OBJECT_SET_CONS(&obj, cl);
	if (code_list_add_argument(vm, clist, &obj)) { t1_pop_variable(vm); return VM_ERROR; }
	t1_pop_variable(vm);

	return VM_OK;
}

// (ignore-errors form*)
static VM_RET t1_ignore_errors(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL cl;
	tOBJECT obj;
	if (code_list_add_command_1(vm, clist, iIGNORE_ERRORS)) return VM_ERROR;
	if (t1_code_list_(vm, operands, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	return code_list_add_argument(vm, clist, &obj);
}

// (continue-condition conditon [value])
static VM_RET t1_continue_condition(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p;
	tOBJECT obj, condition, value;
	// 構文の検査および引数の抜き出し
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &condition);
	cons_get_cdr(p, &obj);
	if (OBJECT_IS_CONS(&obj)) {
		p=OBJECT_GET_CELL(&obj);
		cons_get_car(p, &value);
		cons_get_cdr(p, &obj);
		if (!OBJECT_IS_NIL(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	} else if (OBJECT_IS_NIL(&obj)) {
		//　省略時 value nil
		value=nil;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, translator_get_form(vm));
	}
	//引数は左から右へ
	if (t1_evaluation_form(vm, &condition, clist, tFALSE)) return VM_ERROR;
	if (t1_evaluation_form(vm, &value, clist, tFALSE)) return VM_ERROR;
	//
	return code_list_add_command_1(vm, clist, iCONTINUE_CONDITION);
}

// (with-handler handler form*)
static VM_RET t1_with_handler(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tPCELL p;
	tOBJECT handler, form, obj;
	// 構文の検査および引数の抜き出し
	if (!OBJECT_IS_CONS(operands)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, translator_get_form(vm));
	p=OBJECT_GET_CELL(operands);
	cons_get_car(p, &handler);
	cons_get_cdr(p, &form);
	//
	if (t1_evaluation_form(vm, &handler, clist, tFALSE)) return VM_ERROR;
	if (code_list_add_command_1(vm, clist, iWITH_HANDLER)) return VM_ERROR;
	if (t1_code_list_(vm, &form, &p, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	return code_list_add_argument(vm, clist, &obj);
}

// (time form)
static VM_RET t1_time(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	tOBJECT form, obj;
	tPCELL cl;

	if (t1_syntax_check(vm, operands, 1)) return VM_ERROR;
	cons_get_car(OBJECT_GET_CELL(operands), &form);
	if (code_list_add_command_1(vm, clist, iTIME)) return VM_ERROR;
	if (t1_code_list(vm, &form, &cl, tFALSE)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cl);
	return code_list_add_argument(vm, clist, &obj);
}

static VM_RET t1_in_package(tPVM vm, const tINT id, tPOBJECT operands, tPCELL clist, const tBOOL tail)
{
	return signal_violation(vm, TRANSLATOR_ERROR_NOT_TOP_FORM, translator_get_form(vm));
}

/////////////////////////////////////////////////

static VM_RET t1_primitive_function(tPVM vm, const tINT id, const tINT anum, tPCELL clist)
{
	return (*t1_primitive_table[id].translate)(vm, id, anum, clist);
}

VM_RET t1_primitive(tPVM vm, const tINT code, const tINT anum, tPCELL clist)
{
	tOBJECT obj;
	if (code_list_add_command_1(vm, clist, t1_primitive_table[code].t1_data)) return VM_ERROR;
	OBJECT_SET_INTEGER(&obj, anum);
	return code_list_add_argument(vm, clist, &obj);
}

VM_RET t1_set_property_elt(tPVM vm, const tINT id, const tINT anum, tPCELL clist)
{
	tINT code;
	if (anum!=3) {
		code=iARITY_ERROR;
	} else {
		switch (id) {
		case bSET_PROPERTY:
			code=iSET_PROPERTY;
			break;
		case bSET_ELT:
			code=iSET_ELT;
			break;
		default:
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	}
	return code_list_add_command_1(vm, clist, code);
}

VM_RET t1_set_car_cdr(tPVM vm, const tINT id, const tINT anum, tPCELL clist)
{
	tINT code;
	if (anum!=2) {
		code=iARITY_ERROR;
	} else {
		switch (id) {
		case bSET_CAR:
			code=iSET_CAR;
			break;
		case bSET_CDR:
			code=iSET_CDR;
			break;
		default:
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	}
	return code_list_add_command_1(vm, clist, code);
}
