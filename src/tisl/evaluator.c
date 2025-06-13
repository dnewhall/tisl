//
// TISL/src/tisl/evaluator.c
// TISL Ver. 4.x
//

#define TISL_VM_STRUCT
#define TISL_PRIMITIVE_OPERATION_TABLE
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "built_in_object.h"
#include "writer.h"
#include "reader.h"
#include "translator.h"

extern VM_RET translate(tPVM vm, tPOBJECT form, tPCELL* func);
extern VM_RET translate_defun(tPVM vm, tPCELL name, tPOBJECT lambda_list, tPOBJECT forms, tPCELL* func);
extern VM_RET execute(tPVM vm, tPCELL func);

static VM_RET vm_evaluate_compound_form(tPVM vm, tPOBJECT form, tPOBJECT ret, const tBOOL top);
static VM_RET vm_evaluate_quasiquote(tPVM vm, tPOBJECT form, tPOBJECT ret);
static VM_RET vm_evaluate_identifier(tPVM vm, tPOBJECT identifier, tPOBJECT ret);
static VM_RET vm_evaluate_special_top_form(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET compound_form_get_operator(tPVM vm, tPCELL form, tPOBJECT op);
static tINT operator_is_top_operator(tPOBJECT name, tPOBJECT value);

void vm_clear(tPVM vm)
{
	vm_set_last_condition_ok(vm);
	vm->SP=vm->stack;
	vm->tag_list=0;
}

VM_RET vm_evaluate_top_form(tPVM vm, tPOBJECT form, tPOBJECT ret)
{
	VM_RET r;
	if (vm_push(vm, form)) return VM_ERROR;
	switch (OBJECT_GET_TYPE(form)) {
	case OBJECT_CONS://複合形式
		r=vm_evaluate_compound_form(vm, form, ret, tTRUE);
		break;
	case OBJECT_SYMBOL:// 識別子
		r=vm_evaluate_identifier(vm, form, ret);
		break;
	case OBJECT_QUASIQUOTE:// 
		r=vm_evaluate_quasiquote(vm, form, ret);
		break;
	default://リテラル？
		// リテラルそのものを返す
		*ret=*form;
		r=VM_OK;
	}
	vm_pop(vm);
	return r;
}

VM_RET vm_evaluate_form(tPVM vm, tPOBJECT form, tPOBJECT ret)
{
	switch (OBJECT_GET_TYPE(form)) {
	case OBJECT_CONS:// 複合形式
		return vm_evaluate_compound_form(vm, form, ret, tFALSE);
	case OBJECT_SYMBOL:// 識別子
		return vm_evaluate_identifier(vm, form, ret);
	case OBJECT_QUASIQUOTE:
		return vm_evaluate_quasiquote(vm, form, ret);
	default:
		// リテラルそのものを返す
		*ret=*form;
		return VM_OK;
	}
}

static VM_RET vm_evaluate_identifier(tPVM vm, tPOBJECT identifier, tPOBJECT ret)
{// 変数名前空間で対応する値を返す. 
	tPCELL blist, bind;
	tPCELL symbol=OBJECT_GET_CELL(identifier);
	if (!CELL_IS_SYMBOL(symbol)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	// 束縛リストの取得
	if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), symbol, &blist)) return VM_ERROR;
	// 変数名前空間で設定されている束縛の取得
	bind=bind_list_get_bind(blist, NAMESPACE_VARIABLE, vm_get_current_package(vm));
	if (bind) {// 束縛が存在した
		bind_get_object(bind, NAMESPACE_VARIABLE, ret);
		return VM_OK;
	} else {// 束縛が存在しなかった
		return signal_undefined_entity(vm, TISL_ERROR_UNBOUND_VARIABLE, symbol, NAMESPACE_VARIABLE);
	}
}

static VM_RET vm_evaluate_compound_form(tPVM vm, tPOBJECT form, tPOBJECT ret, const tBOOL top)
{
	tOBJECT op, obj;
	tPCELL cons, func;
	VM_RET r;
	tINT top_id;

LOOP:// マクロの場合はマクロでなくなるまでマクロ展開を行う．
	cons=OBJECT_GET_CELL(form);
	if (!OBJECT_IS_CONS(form)) { return vm_evaluate_top_form(vm, form, ret); }
	if (compound_form_get_operator(vm, cons, &op)) return VM_ERROR;
	if (OBJECT_IS_MACRO(&op)) {
		tOBJECT expanded, operands;
		cons_get_cdr(cons, &operands);
		if (vm_push(vm, form)) return VM_ERROR;
		if (t_macro_expand(vm, OBJECT_GET_CELL(&op), &operands, &expanded)) { vm_pop(vm); return VM_ERROR; }
		vm_pop(vm);
		*form=expanded;
		goto LOOP;
	}
	cons_get_car(cons, &obj);
	if (top) {
		top_id=operator_is_top_operator(&obj, &op);
		if (top_id) {
			if (vm_push(vm, form)) return VM_ERROR;
			// 最上位形式の特殊な演算子の場合分け
			if (vm_evaluate_special_top_form(vm, top_id, form, ret)) { vm_pop(vm); return VM_ERROR; }
			vm_pop(vm);
			return VM_OK;
		}
	}
	// 変換して関数オブジェクトの作成
	if (translate(vm, form, &func)) return VM_ERROR;
	cell_to_object(func, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	r=function_call(vm, func, 0, ret);// トップ環境はNULL
	vm_pop(vm);
	return r;
}

static VM_RET vm_evaluate_quasiquote(tPVM vm, tPOBJECT form, tPOBJECT ret)
{
	VM_RET r;
	tPCELL func;
	tOBJECT obj;
	if (translate(vm, form, &func)) return VM_ERROR;
	cell_to_object(func, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	r=function_call(vm, func, 0, ret);
	vm_pop(vm);
	return r;
}

static VM_RET compound_form_get_operator(tPVM vm, tPCELL form, tPOBJECT op)
{
	tOBJECT obj;

	cons_get_car(form, &obj);
	if (OBJECT_IS_SYMBOL(&obj)) {
		tPCELL blist, bind;
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&obj), &blist)) return VM_ERROR;
		bind=bind_list_get_bind(blist, NAMESPACE_FUNCTION, vm_get_current_package(vm));
		if (bind) {
			bind_get_object(bind, NAMESPACE_FUNCTION, op);
		} else {
			*op=unbound;
		}
	} else {
		*op=obj;
	}
	return VM_OK;
}

static VM_RET arity_check(tPVM vm, const tINT pnum, const tBOOL rest, const tINT anum);
static VM_RET call_function(tPVM vm, tPCELL function, tPCELL name, const tINT anum);
static VM_RET call_local_function(tPVM vm, tPCELL lfunction, tPCELL name, const tINT anum);
static VM_RET call_generic_function(tPVM vm, tPCELL gfunction, tPCELL name, const tINT anum);
static VM_RET call_linked_function(tPVM vm, tPCELL function, tPCELL name, const tINT anum);

// 関数適用形式汎用
// built_in_object.hで宣言
VM_RET function_application_form(tPVM vm, tPOBJECT function, tPCELL name, const tINT anum)
{
	//
/*	if (name) {
		tOBJECT tmp;
		cell_to_object(name, &tmp);
		if (format_object(vm, vm_get_standard_output(vm), &tmp)) return VM_ERROR;
		if (format_fresh_line(vm, vm_get_standard_output(vm))) return VM_ERROR;
	}*/
	//
	switch (OBJECT_GET_TYPE(function)) {
	case OBJECT_PRIMITIVE_OPERATOR:
		return (*primitive_operation_table[OBJECT_GET_INTEGER(function)].operation)(vm, anum);
	case OBJECT_FUNCTION:// ユーザ定義大域関数
		return call_function(vm, OBJECT_GET_CELL(function), name, anum);
		// 包括関数は後で/*!!!*/
	case OBJECT_LOCAL_FUNCTION:// 局所関数
		return call_local_function(vm, OBJECT_GET_CELL(function), name, anum);
	case OBJECT_GENERIC_FUNCTION:
		return call_generic_function(vm, OBJECT_GET_CELL(function), name, anum);
	case OBJECT_LINKED_FUNCTION:
		return call_linked_function(vm, OBJECT_GET_CELL(function), name, anum);
	case OBJECT_SPECIAL_OPERATOR: // 特殊形式，定義形式は関数適用形式ではないので
	case OBJECT_DEFINING_OPERATOR:// 例外が発生する
	default:
		if (name) {
			if (CELL_IS_STRING(name)) {
				tPCELL list;
				tOBJECT tmp;
				OBJECT_SET_STRING(&tmp, name);
				if (cons_create_(vm, &list, &tmp, &nil)) return VM_ERROR;
				if (tisl_get_symbol(vm_get_tisl(vm), vm, list, tFALSE, &name)) return VM_ERROR;
			} else if (!CELL_IS_SYMBOL(name)) {
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FUNCTION, function);
			}
			return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, name, NAMESPACE_FUNCTION);
		} else
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FUNCTION, function);
	}
}

static VM_RET arity_check(tPVM vm, const tINT pnum, const tBOOL rest, const tINT anum)
{
	// 引数の調整
	if (rest) {// restあり
		if (anum<pnum-1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
		if (anum==pnum-1) {
			// 足りない引数をnilとして追加
			if (vm_push(vm, &nil)) return VM_ERROR;
		} else {
			tINT i, n=anum-pnum+1;
			tPCELL p;
			tOBJECT tmp, tmp2;
			tmp=*vm->SP;
			if (cons_create(vm, &p, &tmp, &nil)) return VM_ERROR;
			OBJECT_SET_CONS(vm->SP, p);
			for (i=1; i<n; i++) {
				tmp=*(vm->SP-1);
				tmp2=*vm->SP;
				if (cons_create(vm, &p, &tmp, &tmp2)) return VM_ERROR;
				vm->SP--;
				OBJECT_SET_CONS(vm->SP, p);
			}
		}
	} else {// rest無し
		if (anum!=pnum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	return VM_OK;
}

static VM_RET call_function(tPVM vm, tPCELL function, tPCELL name, const tINT anum)
{
	tOBJECT value;
	tINT sp=vm->SP-vm->stack;
	tINT pnum=function_get_parameter_number(function);
	if (arity_check(vm, pnum, function_is_rest(function), anum)) return VM_ERROR;
	// 
	if (function_is_heap(function)) {
		tPCELL env;
		tOBJECT tmp;
		if (environment_create_(vm, pnum, 0, &env)||
			environment_set_argument(vm, env, pnum)) return VM_ERROR;
		OBJECT_SET_ENVIRONMENT(&tmp, env);
		if (vm_push_temp(vm, &tmp)) return VM_ERROR;
		if (function_call(vm, function, env, &value)) { vm_pop_temp(vm); return VM_ERROR; }
		vm_pop_temp(vm);
	} else {
		if (function_call(vm, function, 0, &value)) return VM_ERROR;
	}
	vm->SP=vm->stack+sp-anum+1;
	*vm->SP=value;
	return VM_OK;	
}

static VM_RET call_local_function(tPVM vm, tPCELL lfunction, tPCELL name, const tINT anum)
{
	tPCELL f=local_function_get_function(lfunction),
		   e=local_function_get_environment(lfunction);
	tINT pnum=function_get_parameter_number(f);
	tINT sp=vm->SP-vm->stack;
	tOBJECT value;
	//
	if (arity_check(vm, pnum, function_is_rest(f), anum)) return VM_ERROR;
	//
	if (function_is_heap(f)) {
		tPCELL env;
		tOBJECT tmp;
		if (environment_create_(vm, pnum, e, &env)) return VM_ERROR;
		if (environment_set_argument(vm, env, pnum)) return VM_ERROR;
		OBJECT_SET_ENVIRONMENT(&tmp, env);
		if (vm_push_temp(vm, &tmp)) return VM_ERROR;
		if (function_call(vm, f, env, &value)) { vm_pop_temp(vm); return VM_ERROR; }
		vm_pop_temp(vm);
	} else {
		if (function_call(vm, f, e, &value)) return VM_ERROR;
	}
	vm->SP=vm->stack+sp-anum+1;
	*vm->SP=value;
	return VM_OK;
}

static VM_RET call_generic_function(tPVM vm, tPCELL gfunction, tPCELL name, const tINT anum)
{
	tINT sp=vm->SP-vm->stack;
	tINT pnum=gfunction_get_parameter_number(gfunction);
	tOBJECT value;
	//
	if (arity_check(vm, pnum, gfunction_is_rest(gfunction), anum)) return VM_ERROR;
	//
	if (gfunction_call(vm, gfunction, &value)) return VM_ERROR;
	vm->SP=vm->stack+sp-anum+1;
	*vm->SP=value;
	return VM_OK;
}

static VM_RET call_linked_function(tPVM vm, tPCELL function, tPCELL name, const tINT anum)
{
	tINT sp=vm->SP-vm->stack;
	tINT pnum=linked_function_get_parameter_number(function);
	tOBJECT value;
	if (arity_check(vm, pnum, linked_function_is_rest(function), anum)) return VM_ERROR;
	if (linked_function_call(vm, function, &value)) return VM_ERROR;
	vm->SP=vm->stack+sp-anum+1;
	*vm->SP=value;
	return VM_OK;
}

///////////////////////////////////////
// top special operator
// 最上位形式の制限のついている形式

enum {
	top_NOT_TOP_OPERATOR=0,
	top_DEFCLASS,
	top_DEFCONSTANT,
	top_DEFDYNAMIC,
	top_DEFGENERIC,
	top_DEFGLOBAL,
	top_DEFMACRO,
	top_DEFMETHOD,
	top_DEFUN,
	top_DEFPACKAGE,
	top_DEFLINK,
	top_IN_PACKAGE,
	top_LOAD,
	top_PROGN,
};

/////////////////////////////

static VM_RET top_defclass(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_defgeneric(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_define(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_defmethod(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_defun(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_defpackage(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_in_package(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_load(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_progn(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);
static VM_RET top_deflink(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret);

/////////////////////////////

typedef VM_RET (*TOP_OPERATOR)(tPVM, const tINT, tPOBJECT, tPOBJECT);
const TOP_OPERATOR top_operator_table[]={
	top_progn,// dummy
	top_defclass,
	top_define,
	top_define,
	top_defgeneric,
	top_define,
	top_defun,
	top_defmethod,
	top_defun,
	top_defpackage,
	top_deflink,
	top_in_package,
	top_load,
	top_progn,
};

/////////////////////////////

static tINT operator_is_top_operator(tPOBJECT name, tPOBJECT value)
{
	if (OBJECT_IS_SYMBOL(name)) {
		tPCELL p=OBJECT_GET_CELL(name);
		if (p==global_symbol[sDEFCLASS]) return top_DEFCLASS;
		if (p==global_symbol[sDEFCONSTANT]) return top_DEFCONSTANT;
		if (p==global_symbol[sDEFDYNAMIC]) return top_DEFDYNAMIC;
		if (p==global_symbol[sDEFGENERIC]) return top_DEFGENERIC;
		if (p==global_symbol[sDEFGLOBAL]) return top_DEFGLOBAL;
		if (p==global_symbol[sDEFMACRO]) return top_DEFMACRO;
		if (p==global_symbol[sDEFMETHOD]) return top_DEFMETHOD;
		if (p==global_symbol[sDEFUN]) return top_DEFUN;
		if (p==global_symbol[sDEFPACKAGE]) return top_DEFPACKAGE;
		if (p==global_symbol[sDEFLINK]) return top_DEFLINK;
		if (p==global_symbol[sIN_PACKAGE]) return top_IN_PACKAGE;
		if (p==global_symbol[sLOAD]) return top_LOAD;
		if (p==global_symbol[sPROGN]) return top_PROGN;
	}
	// 定義形式は全部 最上位形式でなければならない．
	if (OBJECT_IS_DEFINING_OPERATOR(value)) {
		switch (OBJECT_GET_INTEGER(value)) {
		case bDEFCLASS:		return top_DEFCLASS;
		case bDEFCONSTANT:	return top_DEFCONSTANT;
		case bDEFDYNAMIC:	return top_DEFDYNAMIC;
		case bDEFGENERIC:	return top_DEFGENERIC;
		case bDEFGLOBAL:	return top_DEFGLOBAL;
		case bDEFMACRO:		return top_DEFMACRO;
		case bDEFMETHOD:	return top_DEFMETHOD;
		case bDEFUN:		return top_DEFUN;
		case bDEFPACKAGE:	return top_DEFPACKAGE;
		case bDEFLINK:		return top_DEFLINK;
		default:			return top_NOT_TOP_OPERATOR;
		}
	}
	// load in-package progn
	if (OBJECT_IS_SPECIAL_OPERATOR(value)) {
		switch (OBJECT_GET_INTEGER(value)) {
		case bPROGN:		return top_PROGN;
		case bIN_PACKAGE:	return top_IN_PACKAGE;
		}
	}
/*	if (OBJECT_IS_PRIMITIVE_OPERATOR(value)) {
		if (OBJECT_GET_INTEGER(value)==bIN_PACKAGE) return top_IN_PACKAGE;
		if (OBJECT_GET_INTEGER(value)==bLOAD) return top_LOAD;
	}*/
	// どれにも当てはまらなければ
	// 通常に処理できる．
	return top_NOT_TOP_OPERATOR;
}

static VM_RET vm_evaluate_special_top_form(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
	return (*top_operator_table[id])(vm, id, form, ret);
}

// (defconstant name [access-qualifier] form)
// (defglobal name [access-qualifier] form)
// (defdynamic name [access-qualifier] form)
static VM_RET top_define(tPVM vm, const tINT id, tPOBJECT top, tPOBJECT ret)
{
	tPCELL p;
	tINT len;
	tOBJECT name, qualifier, form, value;
	tPCELL bind, name_string;
	tBOOL public_p;

	// 引数の読込み
	p=OBJECT_GET_CELL(top);
	len=cons_get_length(p);
	if ((len!=3)&&(len!=4)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	p=cons_get_cdr_cons(p);
	cons_get_car(p, &name);
	if (!OBJECT_IS_SYMBOL(&name)||// nameが記号でない
		!symbol_is_simple(OBJECT_GET_CELL(&name)))// nameにパッケージ修飾子がついている
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	if (len==4) {
		p=cons_get_cdr_cons(p);
		cons_get_car(p, &qualifier);
		if (!OBJECT_IS_SYMBOL(&qualifier)||
			((OBJECT_GET_CELL(&qualifier)!=KEYWORD_PRIVATE)&&
			 (OBJECT_GET_CELL(&qualifier)!=KEYWORD_PUBLIC)))
			 return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		public_p=(OBJECT_GET_CELL(&qualifier)==KEYWORD_PUBLIC) ? tTRUE : tFALSE;
	} else {
		public_p=tFALSE;
	}
	p=cons_get_cdr_cons(p);
	cons_get_car(p, &form);

	symbol_get_string(OBJECT_GET_CELL(&name), 0, &name_string);
	if (tisl_get_bind(vm_get_tisl(vm), vm, vm_get_current_package(vm), name_string, &bind)) return VM_ERROR;

	if (vm_evaluate_form(vm, &form, &value)) return VM_ERROR;

	switch (id) {
	case top_DEFCONSTANT:
	case top_DEFGLOBAL:
		bind_set_variable(bind, &value);
		if (public_p) bind_set_variable_public(bind);
		else bind_set_variable_private(bind);
		break;
	case top_DEFDYNAMIC:
		bind_set_dynamic(bind, &value);
		if (public_p) bind_set_dynamic_public(bind);
		else bind_set_dynamic_private(bind);
		break;
	}
	*ret=name;

	return VM_OK;
}

//////////////////////////////

// (defun name [access-qualifier] lambda-list form*)
static VM_RET top_defun(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
	tOBJECT obj, name, lambda_list, forms;
	tBOOL public_p;
	tPCELL p, func, name_string, bind;
	// 引数の読み取り
	cons_get_cdr(OBJECT_GET_CELL(form), &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &name);
	cons_get_cdr(p, &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	p=OBJECT_GET_CELL(&obj);
	cons_get_car(p, &obj);
	public_p=tFALSE;
	if (OBJECT_IS_SYMBOL(&obj)) {
		if (OBJECT_GET_CELL(&obj)==KEYWORD_PUBLIC) 
			public_p=tTRUE;
		else if (OBJECT_GET_CELL(&obj)!=KEYWORD_PRIVATE)
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		cons_get_cdr(p, &obj);
		if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		p=OBJECT_GET_CELL(&obj);
		cons_get_car(p, &obj);
	}
	if (OBJECT_IS_CONS(&obj)||OBJECT_IS_NIL(&obj)) {
		lambda_list=obj;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	}
	cons_get_cdr(p, &forms);
	if (!OBJECT_IS_CONS(&forms)&&!OBJECT_IS_NIL(&forms)) {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	}
	//
	if (!OBJECT_IS_SYMBOL(&name)||// 名前が記号でない
		!symbol_is_simple(OBJECT_GET_CELL(&name)))//パッケージ修飾子がついている
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	if (symbol_is_built_in_function(vm, OBJECT_GET_CELL(&name))) return VM_ERROR;
	symbol_get_string(OBJECT_GET_CELL(&name), 0, &name_string);
	if (tisl_get_bind(vm_get_tisl(vm), vm, vm_get_current_package(vm), name_string, &bind)) return VM_ERROR;

	if (translate_defun(vm, OBJECT_GET_CELL(&name), &lambda_list, &forms, &func)) return VM_ERROR;
	cell_to_object(func, &obj);
	bind_set_function(bind, &obj);
	if (public_p) bind_set_function_public(bind);
	else bind_set_function_private(bind);
	if (id==top_DEFMACRO) function_set_macro(func);
	*ret=name;
	
	return VM_OK;
}

static VM_RET top_load(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
	tOBJECT obj, name;
	tPCELL p, stream;
	// 引数の検査
	cons_get_cdr(OBJECT_GET_CELL(form), &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	p=OBJECT_GET_CELL(&obj);
	cons_get_cdr(p, &obj);
	if (!OBJECT_IS_NIL(&obj)) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	cons_get_car(p, &obj);
	if (vm_evaluate_top_form(vm, &obj, &name)) return VM_ERROR;
	if (!OBJECT_IS_STRING(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &name);
	// ファイルの作成
	if (file_stream_create(vm, STREAM_INPUT, OBJECT_GET_CELL(&name), &stream)) return VM_ERROR;
	if (vm_load(vm, stream, &obj)) {
		OBJECT_SET_INTEGER(&obj, file_stream_get_y(stream));
		if (cons_create(vm, &p, &name, &obj)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, p);
		if (format_object(vm, vm_get_error_output(vm), &obj)) { file_stream_close(vm, stream); return VM_ERROR; }
		file_stream_close(vm, stream);
		return VM_ERROR;
	} else {
		if (file_stream_close(vm, stream)) return VM_ERROR;
		*ret=name;
		return VM_OK;
	}
}

static VM_RET top_progn(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
	tPCELL p;
	tOBJECT obj;
	cons_get_cdr(OBJECT_GET_CELL(form), &obj);
	if (!OBJECT_IS_CONS(&obj)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
	p=OBJECT_GET_CELL(&obj);
	while (p) {
		cons_get_car(p, &obj);
		if (vm_evaluate_top_form(vm, &obj, ret)) return VM_ERROR;
		cons_get_cdr(p, &obj);
		if (OBJECT_IS_CONS(&obj))
			p=OBJECT_GET_CELL(&obj);
		else if (OBJECT_IS_NIL(&obj))
			p=0;
		else
			return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
	}
	return VM_OK;
}

///////////////////////////////////////

// (defclass class-name (sc-name*) (slot-spec*) class-opt*)
// slot-spec ::= slot-name | (slot-name slot-opt*)
static VM_RET top_defclass(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
	tOBJECT name, tmp, access, abstract, metaclass;
	tPCELL p, sc_name_list, slot_spec_list, bind, string, sclass;
	tBOOL f;

	// class-name
	cons_get_cdr(OBJECT_GET_CELL(form), &tmp);
	if (!OBJECT_IS_CONS(&tmp)) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	p=OBJECT_GET_CELL(&tmp);
	cons_get_car(p, &name);
	if (!OBJECT_IS_SYMBOL(&name)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	if (!symbol_is_simple(OBJECT_GET_CELL(&name))) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
	// access-qualifier
	cons_get_cdr(p, &tmp);
	if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	p=OBJECT_GET_CELL(&tmp);
	cons_get_car(p, &access);
	if (OBJECT_IS_SYMBOL(&access)) {
		if ((OBJECT_GET_CELL(&access)!=KEYWORD_PRIVATE)&&
			(OBJECT_GET_CELL(&access)!=KEYWORD_PUBLIC))
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		p=OBJECT_GET_CELL(&tmp);
	} else {
		OBJECT_SET_SYMBOL(&access, KEYWORD_PRIVATE);
	}
	// (sc-name*)
	cons_get_car(p, &tmp);
	if (OBJECT_IS_CONS(&tmp)) {
		tPCELL pp;
		sc_name_list=OBJECT_GET_CELL(&tmp);
		for (pp=sc_name_list; pp; pp=cons_get_cdr_cons(pp)) {
			cons_get_car(pp, &tmp);
			if (!OBJECT_IS_SYMBOL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp)&&!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
		}
	} else if (OBJECT_IS_NIL(&tmp)) {
		sc_name_list=0;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	}
	// (slot-spec*)
	cons_get_cdr(p, &tmp);
	if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	p=OBJECT_GET_CELL(&tmp);
	cons_get_car(p, &tmp);
	if (OBJECT_IS_CONS(&tmp)) {
		tPCELL pp;
		slot_spec_list=OBJECT_GET_CELL(&tmp);
		for (pp=slot_spec_list; pp; pp=cons_get_cdr_cons(pp)) {
			cons_get_car(pp, &tmp);
			if (OBJECT_IS_CONS(&tmp)) {
				tPCELL ppp;
				// (slot-name slot-opt*)
				ppp=OBJECT_GET_CELL(&tmp);
				cons_get_car(ppp, &tmp);
				if (!OBJECT_IS_SYMBOL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
				if (!symbol_is_simple(OBJECT_GET_CELL(&tmp))) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
				cons_get_cdr(ppp, &tmp);
				if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
				for (ppp=OBJECT_GET_CELL(&tmp); ppp; ppp=cons_get_cdr_cons(ppp)) {
					cons_get_car(ppp, &tmp);
					if (!OBJECT_IS_SYMBOL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
					if ((OBJECT_GET_CELL(&tmp)==KEYWORD_WRITER)||
						(OBJECT_GET_CELL(&tmp)==KEYWORD_READER)||
						(OBJECT_GET_CELL(&tmp)==KEYWORD_ACCESSOR)||
						(OBJECT_GET_CELL(&tmp)==KEYWORD_BOUNDP)||
						(OBJECT_GET_CELL(&tmp)==KEYWORD_INITARG)) {
						cons_get_cdr(ppp, &tmp);
						if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
						ppp=OBJECT_GET_CELL(&tmp);
						cons_get_car(ppp, &tmp);
						if (!OBJECT_IS_SYMBOL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
						if (!symbol_is_simple(OBJECT_GET_CELL(&tmp))) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
					} else if (OBJECT_GET_CELL(&tmp)==KEYWORD_INITFORM) {
						cons_get_cdr(ppp, &tmp);
						if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
						ppp=OBJECT_GET_CELL(&tmp);
					}
					cons_get_cdr(ppp, &tmp);
					if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
				}
			} else if (!OBJECT_IS_SYMBOL(&tmp)) {
				return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			}
			//
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp)&&!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
		}
	} else if (OBJECT_IS_NIL(&tmp)) {
		slot_spec_list=0;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	}
	// class-opt*
	OBJECT_SET_NIL(&abstract);
	OBJECT_SET_SYMBOL(&metaclass, SYMBOL_STANDARD_CLASS_CLASS);
	cons_get_cdr(p, &tmp);
	if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
	for (p=OBJECT_GET_CELL(&tmp); p; p=cons_get_cdr_cons(p)) {
		tPCELL pp;
		cons_get_car(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		pp=OBJECT_GET_CELL(&tmp);
		cons_get_car(pp, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR, form);
		if (OBJECT_GET_CELL(&tmp)==KEYWORD_METACLASS) {
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			pp=OBJECT_GET_CELL(&tmp);
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			cons_get_car(pp, &tmp);
			if (!OBJECT_IS_SYMBOL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			// metaclassにゆるすのは次の二つのみ
			// <standard-class>
			// <foreign-class>
			if ((OBJECT_GET_CELL(&tmp)!=SYMBOL_STANDARD_CLASS_CLASS)&&
				(OBJECT_GET_CELL(&tmp)!=SYMBOL_FOREIGN_CLASS_CLASS))
				return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			metaclass=tmp;
		} else if (OBJECT_GET_CELL(&tmp)==KEYWORD_ABSTRACTP) {
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
			pp=OBJECT_GET_CELL(&tmp);
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			cons_get_car(pp, &abstract);
			if (!OBJECT_IS_NIL(&abstract)&&
				(!OBJECT_IS_SYMBOL(&abstract)||(OBJECT_GET_CELL(&abstract)!=SYMBOL_T)))
				return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		} else {
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		}
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_NIL(&tmp)&&!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
	}
	if (OBJECT_GET_CELL(&metaclass)==SYMBOL_STANDARD_CLASS_CLASS) {
		// 標準クラスの生成
		f=OBJECT_IS_NIL(&abstract) ? tFALSE : tTRUE;
		if (standard_class_create_(vm, OBJECT_GET_CELL(&name), sc_name_list, slot_spec_list, f, &sclass)) return VM_ERROR;
	} else if (OBJECT_GET_CELL(&metaclass)==SYMBOL_FOREIGN_CLASS_CLASS) {
		if (foreign_class_create_(vm, OBJECT_GET_CELL(&name), sc_name_list, &sclass)) return VM_ERROR;
	} else {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	cell_to_object(sclass, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// 束縛の設定
	symbol_get_string(OBJECT_GET_CELL(&name), 0, &string);
	if (package_add_bind(vm, vm_get_current_package(vm), string, &bind)) { vm_pop(vm); return VM_ERROR; }
	bind_get_class(bind, &tmp);
	if (OBJECT_IS_STANDARD_CLASS(&tmp)) {
		standard_class_set_invalid(OBJECT_GET_CELL(&tmp));
	} else if (OBJECT_IS_BUILT_IN_CLASS(&tmp)) {
		return signal_condition(vm, TISL_ERROR_IMMUTABLE_BINDING);
	} else if (OBJECT_IS_FOREIGN_CLASS(&tmp)) {
		foreign_class_set_super(OBJECT_GET_CELL(&tmp), foreign_class_get_super(sclass));
		if (OBJECT_GET_CELL(&access)==KEYWORD_PRIVATE)
			bind_set_class_private(bind);
		else
			bind_set_class_public(bind);
		vm_pop(vm);
		*ret=name;
		return VM_OK;
	}
	cell_to_object(sclass, &tmp);
	bind_set_class(bind, &tmp);
	if (OBJECT_GET_CELL(&access)==KEYWORD_PRIVATE)
		bind_set_class_private(bind);
	else
		bind_set_class_public(bind);
	vm_pop(vm);
	*ret=name;
	return VM_OK;
}

///////////////////////////////////////

static VM_RET top_defgeneric_option(tPVM vm, tPCELL p, tPCELL method_list, tBOOL* standard, tPOBJECT top);

// (defgeneric func-spec lambda-list { option | method-desc}*)
static VM_RET top_defgeneric(tPVM vm, const tINT id, tPOBJECT top, tPOBJECT ret)
{
	tPCELL p, pp, method_list, gfunction, bind, string;
	tOBJECT tmp, name, lambda_list, access;
	tINT pnum;
	tBOOL rest, standard;

	// 引数の読み取り 構文検査
	cons_get_cdr(OBJECT_GET_CELL(top), &tmp);
	if (!OBJECT_IS_CONS(&tmp)) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	p=OBJECT_GET_CELL(&tmp);
	cons_get_car(p, &name);
	if (OBJECT_IS_CONS(&name)) {
		// (setf identifier)
		pp=OBJECT_GET_CELL(&name);
		cons_get_car(pp, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)||
			(OBJECT_GET_CELL(&tmp)!=SYMBOL_SETF)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		cons_get_cdr(pp, &tmp);
		if (!OBJECT_IS_CONS(&tmp))
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		pp=OBJECT_GET_CELL(&tmp);
		cons_get_car(pp, &name);
		cons_get_cdr(pp, &tmp);
		if (!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	}
	if (!OBJECT_IS_SYMBOL(&name)||
		!symbol_is_simple(OBJECT_GET_CELL(&name))) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	p=cons_get_cdr_cons(p);
	if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	cons_get_car(p, &access);
	if (OBJECT_IS_SYMBOL(&access)) {
		// アクセス修飾子
		if ((OBJECT_GET_CELL(&access)!=KEYWORD_PRIVATE)&&
			(OBJECT_GET_CELL(&access)!=KEYWORD_PUBLIC)) {
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		}
		p=cons_get_cdr_cons(p);
		if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	} else {
		OBJECT_SET_SYMBOL(&access, KEYWORD_PRIVATE);
	}
	// lambda-list
	cons_get_car(p, &lambda_list);
	if (!OBJECT_IS_CONS(&lambda_list)&&!OBJECT_IS_NIL(&lambda_list)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	// 構文の検査のみ
	rest=tFALSE;
	pnum=0;
	for (pp=cons_get_car_cons(p); pp; pp=cons_get_cdr_cons(pp)) {
		cons_get_car(pp, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)||
			!symbol_is_simple(OBJECT_GET_CELL(&tmp))) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		if (OBJECT_GET_CELL(&tmp)==KEYWORD_REST) {
			// rest
			pp=cons_get_cdr_cons(pp);
			cons_get_car(pp, &tmp);
			if (!OBJECT_IS_SYMBOL(&tmp)||
				!symbol_is_simple(OBJECT_GET_CELL(&tmp))) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp))
				return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
			pnum++;
			rest=tTRUE;
		} else {
			pnum++;
		}
		cons_get_cdr(pp, &tmp);
		if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, top);
	}
	// { option | method-des}*
	p=cons_get_cdr_cons(p);
	standard=tTRUE;
	if (cons_create_(vm, &method_list, &nil, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, method_list);
	cons_set_car(method_list, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	for (; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		if (OBJECT_IS_CONS(&tmp)) {
			if (top_defgeneric_option(vm, OBJECT_GET_CELL(&tmp), method_list, &standard, top)) { vm_pop(vm); return VM_ERROR; }
		} else {
			vm_pop(vm);
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		}
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, top); }
	}
	if (symbol_is_built_in_function(vm, OBJECT_GET_CELL(&name))) return VM_ERROR;
	// 包括関数の作成
	if (gfunction_create_(vm, pnum, rest, standard, OBJECT_IS_CONS(&lambda_list) ? OBJECT_GET_CELL(&lambda_list) : 0, &gfunction)) { vm_pop(vm); return VM_ERROR; }
	cell_to_object(gfunction, &tmp);
	if (vm_push(vm, &tmp)) { vm_pop(vm); return VM_ERROR; }
	// メソッドの登録
	for (p=cons_get_cdr_cons(method_list); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		if (gfunction_add_method(vm, gfunction, OBJECT_GET_CELL(&tmp))) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	}
	// 束縛の設定
	symbol_get_string(OBJECT_GET_CELL(&name), 0, &string);
	if (tisl_get_bind(vm_get_tisl(vm), vm, vm_get_current_package(vm), string, &bind)) { vm_pop(vm); vm_pop(vm); return VM_ERROR; }
	cell_to_object(gfunction, &tmp);
	bind_set_function(bind, &tmp);
	if (OBJECT_GET_CELL(&access)==KEYWORD_PRIVATE)
		bind_set_function_private(bind);
	else
		bind_set_function_public(bind);
	vm_pop(vm);
	vm_pop(vm);
	*ret=name;
	return VM_OK;
}

//	option | method-desc
//	option		::= (:method-combination { identifier | keyword }) | (:generic-function-class class-name)
//	method-desc	::=	(:method method-qualifier* parameter-profile form*)
static VM_RET top_defgeneric_option(tPVM vm, tPCELL p, tPCELL method_list, tBOOL* standard, tPOBJECT top)
{
	tOBJECT key, tmp;
	cons_get_car(p, &key);
	if (!OBJECT_IS_SYMBOL(&key)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	if (OBJECT_GET_CELL(&key)==KEYWORD_METHOD_COMBINATION) {
		// (:method-combination { standard | nil} )		
		p=cons_get_cdr_cons(p);
		if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		cons_get_car(p, &tmp);
		if (!OBJECT_IS_NIL(&tmp)) {
			*standard=tFALSE;
		} else if (OBJECT_IS_SYMBOL(&tmp)&&(OBJECT_GET_CELL(&tmp)==SYMBOL_STANDARD)) {
			*standard=tTRUE;
		} else {
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		}
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		return VM_OK;
	} else if (OBJECT_GET_CELL(&key)==KEYWORD_GENERIC_FUNCTION_CLASS) {
		// (:generic-function-class <standard-generic-function>)
		// TISLではこのクラスを指定することはできない
		return signal_condition(vm, TISL_ERROR_GENERIC_FUNCTION_CLASS);
	} else if (OBJECT_GET_CELL(&key)==KEYWORD_METHOD) {
		// 
		tPCELL method, pp;
		p=cons_get_cdr_cons(p);
		if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
		if (translate_method(vm, p, &method)) return VM_ERROR;
		cell_to_object(method, &tmp);
		if (cons_create(vm, &pp, &tmp, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&tmp, pp);
		cons_set_cdr(cons_get_car_cons(method_list), &tmp);
		cons_set_car(method_list, &tmp);
		return VM_OK;
	} else {
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, top);
	}
}

// (defmethod func-spec method-qualifier* parameter-profile form*)
static VM_RET top_defmethod(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
	tOBJECT tmp, name;
	tPCELL p, method, bind, string;
	p=cons_get_cdr_cons(OBJECT_GET_CELL(form));
	if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	// func-spec
	cons_get_car(p, &name);
	if (OBJECT_IS_CONS(&name)) {
		tPCELL pp=OBJECT_GET_CELL(&name);
		cons_get_car(pp, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)||(OBJECT_GET_CELL(&tmp)!=SYMBOL_SETF))
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		pp=cons_get_cdr_cons(pp);
		if (!pp) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		cons_get_car(pp, &name);
		if (!OBJECT_IS_SYMBOL(&name)) {
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
		} else if (!symbol_is_simple(OBJECT_GET_CELL(&name))) {
			return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
		}
		cons_get_cdr(pp, &tmp);
		if (!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	} else if (!OBJECT_IS_SYMBOL(&name)) {
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &name);
	} else if (!symbol_is_simple(OBJECT_GET_CELL(&name))) {
		return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
	}
	// メソッドの作成
	p=cons_get_cdr_cons(p);
	if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	if (translate_method(vm, p, &method)) return VM_ERROR;
	cell_to_object(method, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// 登録する包括関数の検索
	symbol_get_string(OBJECT_GET_CELL(&name), 0, &string);
	bind=package_get_bind(vm_get_current_package(vm), string);
	if (!bind) { vm_pop(vm); return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, OBJECT_GET_CELL(&name), NAMESPACE_FUNCTION); }
	bind_get_function(bind, &tmp);
	if (OBJECT_IS_UNBOUND(&tmp)) { vm_pop(vm); return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_FUNCTION, OBJECT_GET_CELL(&name), NAMESPACE_FUNCTION); }
	if (!OBJECT_IS_GENERIC_FUNCTION(&tmp)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STANDARD_GENERIC_FUNCTION, &tmp); }
	// メソッドの追加
	if (gfunction_add_method(vm, OBJECT_GET_CELL(&tmp), method)) { vm_pop(vm); return VM_ERROR; }
	gfunction_clear_emethod(OBJECT_GET_CELL(&tmp));
	vm_pop(vm);
	*ret=name;
	return VM_OK;
}

static VM_RET top_defpackage(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{// (defpackage name [access] (use-package))
	tOBJECT name, access, tmp;
	tPCELL p, use_package_list, bind, package, string;
	// 構文の検査
	p=cons_get_cdr_cons(OBJECT_GET_CELL(form));
	if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	cons_get_car(p, &name);
	if (!OBJECT_IS_SYMBOL(&name)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	if (!symbol_is_simple(OBJECT_GET_CELL(&name))) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
	cons_get_cdr(p, &tmp);
	if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form);
	p=OBJECT_GET_CELL(&tmp);
	cons_get_car(p, &access);
	if (OBJECT_IS_SYMBOL(&access)) {
		if ((OBJECT_GET_CELL(&access)!=KEYWORD_PRIVATE)&&
			(OBJECT_GET_CELL(&access)!=KEYWORD_PUBLIC)) {
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		}
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		p=OBJECT_GET_CELL(&tmp);
	} else {
		OBJECT_SET_SYMBOL(&access, KEYWORD_PRIVATE);
	}
	cons_get_cdr(p, &tmp);
	if (!OBJECT_IS_NIL(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	if (head_list_create(vm, &use_package_list)) return VM_ERROR;
	cell_to_object(use_package_list, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	cons_get_car(p, &tmp);
	if (OBJECT_IS_CONS(&tmp)) {
		tPCELL p;
		for (p=OBJECT_GET_CELL(&tmp); p; p=cons_get_cdr_cons(p)) {
			tPCELL blist, bind;
			cons_get_car(p, &tmp);
			if (!OBJECT_IS_SYMBOL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
			if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&tmp), &blist)) { vm_pop(vm); return VM_ERROR; }
			bind=bind_list_get_bind(blist, NAMESPACE_PACKAGE, vm_get_current_package(vm));
			if (!bind) { vm_pop(vm); return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, OBJECT_GET_CELL(&tmp), NAMESPACE_PACKAGE); }
			bind_get_package(bind, &tmp);
			if (head_list_add_object(vm, use_package_list, &tmp)) { vm_pop(vm); return VM_ERROR; }
			cons_get_cdr(p, &tmp);
			if (!OBJECT_IS_NIL(&tmp)&&!OBJECT_IS_CONS(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_DOT_LIST, form); }
		}
	} else if (OBJECT_IS_NIL(&tmp)) {
		use_package_list=0;
	} else {
		vm_pop(vm);
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	}
	//
	symbol_get_string(OBJECT_GET_CELL(&name), 0, &string);
	if (package_add_bind(vm, vm_get_current_package(vm), string, &bind)) { vm_pop(vm); return VM_ERROR; }
	p=use_package_list ? cons_get_cdr_cons(use_package_list) : 0;
	bind_get_package(bind, &tmp);
	if (OBJECT_IS_UNBOUND(&tmp)) {
		if (package_create_(vm, bind, p, string, vm_get_current_package(vm), &package)) { vm_pop(vm); return VM_ERROR; }
		cell_to_object(package, &tmp);
		bind_set_package(bind, &tmp);
	} else {// 既にパッケージが用意されている
		package=OBJECT_GET_CELL(&tmp);
		if ((package==vm_get_top_package(vm))||
			(package==vm_get_islisp_package(vm))||
			(package==vm_get_system_package(vm))) {
			vm_pop(vm);
			return signal_condition(vm, TISL_ERROR_IMMUTABLE_BINDING);
		}
		if (package_reset(vm, p, package)) { vm_pop(vm); return VM_ERROR; }
	}
	if (OBJECT_GET_CELL(&access)==KEYWORD_PRIVATE)
		bind_set_package_private(bind);
	else
		bind_set_package_public(bind);
	vm_pop(vm);
	*ret=name;
	return VM_OK;
}

static VM_RET top_in_package(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{// (in-package name)
	tOBJECT tmp;
	tPCELL p;
	p=cons_get_cdr_cons(OBJECT_GET_CELL(form));
	if (!p) {// (in-package)
		vm_set_current_package(vm, vm_get_top_package(vm));
		OBJECT_SET_NIL(ret);
	} else {
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_NIL(&tmp)) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
		cons_get_car(p, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &tmp);
		if (vm_in_package(vm, OBJECT_GET_CELL(&tmp))) return VM_ERROR;
		*ret=tmp;
	}
	return VM_OK;
}

// (deflink name [accessor] parameter-profile dll-name dll-procedure [ret-type]) -> name
static VM_RET top_deflink(tPVM vm, const tINT id, tPOBJECT form, tPOBJECT ret)
{
#if defined(TISL_DYNAMIC)
	tPCELL p, string, bind, pp, profile_list;
	tOBJECT tmp, name, accessor, lambda_list, dll_name, dll_procedure;
	tINT n;
	tBOOL rest, voidp;
	// 構文の検査と引数の取得
	p=cons_get_cdr_cons(OBJECT_GET_CELL(form));
	if (!p) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	// nameの読み取り
	cons_get_car(p, &name);
	if (!OBJECT_IS_SYMBOL(&name)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	if (!symbol_is_simple(OBJECT_GET_CELL(&name))) return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER);
	cons_get_cdr(p, &tmp);
	if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	p=OBJECT_GET_CELL(&tmp);
	cons_get_car(p, &accessor);
	if (OBJECT_IS_SYMBOL(&accessor)) {
		// アクセス修飾子の読み取り
		if ((OBJECT_GET_CELL(&accessor)!=KEYWORD_PRIVATE)&&
			(OBJECT_GET_CELL(&accessor)!=KEYWORD_PUBLIC))
			return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		cons_get_cdr(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)) return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
		p=OBJECT_GET_CELL(&tmp);
	} else {
		OBJECT_SET_SYMBOL(&accessor, KEYWORD_PRIVATE);
	}
	cons_get_car(p, &lambda_list);
	if (head_list_create(vm, &profile_list)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, profile_list);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	if (OBJECT_IS_CONS(&lambda_list)) {
		// lambda-listの検査と引数の数及びrestの有無の読み取り
		tPCELL pp;
		rest=tFALSE;
		n=0;
		for (pp=OBJECT_GET_CELL(&lambda_list); pp; pp=cons_get_cdr_cons(pp)) {
			cons_get_car(pp, &tmp);
			if (OBJECT_IS_CONS(&tmp)) {
				// (name class-name)
				tOBJECT pname, cname;
				tPCELL p3;
				p3=OBJECT_GET_CELL(&tmp);
				cons_get_car(p3, &pname);
				cons_get_cdr(p3, &tmp);
				if (!OBJECT_IS_CONS(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
				p3=OBJECT_GET_CELL(&tmp);
				cons_get_car(p3, &cname);
				cons_get_cdr(p3, &tmp);
				if (!OBJECT_IS_NIL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
				//
				if (!OBJECT_IS_SYMBOL(&pname)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &pname); }
				if (!symbol_is_simple(OBJECT_GET_CELL(&pname))) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER); }
				//
				if (!OBJECT_IS_SYMBOL(&cname)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SYMBOL, &pname); }
				n++;
				if (head_list_add_object(vm, profile_list, &cname)) { vm_pop(vm); return VM_ERROR; }
				cons_get_cdr(pp, &tmp);
				if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
			} else if (OBJECT_IS_SYMBOL(&tmp)) {
				if (OBJECT_GET_CELL(&tmp)==KEYWORD_REST) {
					cons_get_cdr(pp, &tmp);
					if (!OBJECT_IS_CONS(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
					pp=OBJECT_GET_CELL(&tmp);
					cons_get_cdr(pp, &tmp);
					if (!OBJECT_IS_NIL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
					cons_get_car(pp, &tmp);
					if (!OBJECT_IS_SYMBOL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
					if (!symbol_is_simple(OBJECT_GET_CELL(&tmp))) { vm_pop(vm); return signal_violation(vm, TISL_ERROR_PACKAGE_QUALIFIER, form); }
					n++;
					OBJECT_SET_BUILT_IN_CLASS(&tmp, CLASS_LIST);
					if (head_list_add_object(vm, profile_list, &tmp)) { vm_pop(vm); return VM_ERROR; }
					rest=tTRUE;
				} else {
					if (!symbol_is_simple(OBJECT_GET_CELL(&tmp))) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_PACKAGE_QUALIFIER); }
					n++;
					OBJECT_SET_BUILT_IN_CLASS(&tmp, CLASS_OBJECT);
					if (head_list_add_object(vm, profile_list, &tmp)) { vm_pop(vm); return VM_ERROR; }
					cons_get_cdr(pp, &tmp);
					if (!OBJECT_IS_CONS(&tmp)&&!OBJECT_IS_NIL(&tmp)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
				}
			} else {
				vm_pop(vm);
				return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
			}
		}
	} else if (OBJECT_IS_NIL(&lambda_list)) {
		n=0;
		rest=tFALSE;
	} else {
		vm_pop(vm);
		return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form);
	}
	// dll-name
	p=cons_get_cdr_cons(p);
	if (!p) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
	cons_get_car(p, &dll_name);
	if (!OBJECT_IS_STRING(&dll_name)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &dll_name); }
	// dll-procedure
	p=cons_get_cdr_cons(p);
	if (!p) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
	cons_get_car(p, &dll_procedure);
	if (!OBJECT_IS_STRING(&dll_procedure)) { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &dll_procedure); }
	// [ret-type]
	p=cons_get_cdr_cons(p);
	if (p) {
		cons_get_car(p, &tmp);
		if (!OBJECT_IS_SYMBOL(&tmp)||(OBJECT_GET_CELL(&tmp)!=KEYWORD_VOID)) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
		voidp=tTRUE;
		p=cons_get_cdr_cons(p);
		if (p) { vm_pop(vm); return signal_violation(vm, TRANSLATOR_ERROR_SYNTAX_ERROR, form); }
	} else {
		voidp=tFALSE;
	}
	//
	if (symbol_is_built_in_function(vm, OBJECT_GET_CELL(&name))) { vm_pop(vm); return VM_ERROR; }
	symbol_get_string(OBJECT_GET_CELL(&name), 0, &string);
	if (package_add_bind(vm, vm_get_current_package(vm), string, &bind)) { vm_pop(vm); return VM_ERROR; }
	p=OBJECT_IS_CONS(&lambda_list) ? OBJECT_GET_CELL(&lambda_list) : 0;
	if (linked_function_create_(vm, n, rest, OBJECT_GET_CELL(&dll_name), OBJECT_GET_CELL(&dll_procedure), voidp, p, profile_list, &pp)) { vm_pop(vm); return VM_ERROR; }
	cell_to_object(pp, &tmp);
	bind_set_function(bind, &tmp);
	if (OBJECT_GET_CELL(&accessor)==KEYWORD_PRIVATE)
		bind_set_function_private(bind);
	else
		bind_set_function_public(bind);
	*ret=name;
	vm_pop(vm);
	return VM_OK;
#else
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
#endif
}

