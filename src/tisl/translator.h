//
// TISL/src/tisl/translator.h
// TISL Ver. 4.x
//

#ifndef TISL_TRANSLATOR_H
#define TISL_TRANSLATOR_H

/////////////////////////////

VM_RET translate(tPVM vm, tPOBJECT form, tPCELL* func);
VM_RET translate_defun(tPVM vm, tPCELL name, tPOBJECT lambda_list, tPOBJECT forms, tPCELL* func);
VM_RET translate_method(tPVM vm, tPCELL form, tPCELL* method);

// マクロと特殊形式の展開を行い制御情報に基づいた変換を主に行う．
// また，名前の参照のされかたから値の存在期間を推論し，次のパスで利用する．
VM_RET translate_pass1(tPVM vm, tPOBJECT form, tPCELL* code_list_1);
VM_RET translate_pass1_defun(tPVM vm, tPCELL name, tPOBJECT lambda_list, tPOBJECT forms, tPCELL* code_list);
VM_RET translate_pass1_method(tPVM vm, tPCELL forms, tPOBJECT body, tPCELL* mlist);

// 実行可能な命令列と1対1に対応できる命令のリストを作成する．
// 実引数の実行時の位置を決定し，命令を確定する．
VM_RET translate_pass2(tPVM vm, tPCELL code_list);
VM_RET translate_pass2_defun(tPVM vm, tPCELL flist, tINT pnum);
VM_RET translate_pass2_method(tPVM vm, tPCELL mlist);

// コードリストから関数オブジェクトを作成する．
VM_RET translate_pass3(tPVM vm, tPCELL code_list, tPCELL* function);
VM_RET translate_pass3_defun(tPVM vm, tPCELL flist, tPCELL* function);
VM_RET translate_pass3_method(tPVM vm, tPCELL mlist, tPOBJECT form, tPCELL* method);

// GC root
VM_RET translator_mark(tPVM vm);

// マクロ展開
VM_RET t_macro_expand(tPVM vm, tPCELL macro, tPOBJECT operands, tPOBJECT ret);

void translator_set_form(tPVM vm, tPOBJECT form);
tPOBJECT translator_get_form(tPVM vm);

/////////////////////////////
// コードリストのアクセス

tBOOL list_is_variable(tPCELL list);
// ((size . tail) ...)
VM_RET head_list_create(tPVM vm, tPCELL* hlist);
VM_RET head_list_add_object(tPVM vm,tPCELL hlist, tPOBJECT obj);
tINT head_list_get_size(tPCELL hlist);

// function-list
VM_RET function_list_create(tPVM vm, tPCELL plist, tPCELL clist, tPCELL* flist);
VM_RET function_list_create_with_plist(tPVM vm, tPCELL plist, tPCELL* flist);
tINT function_list_get_parameter_number(tPCELL flist);
tPCELL function_list_get_parameter_list(tPCELL flist);
tPCELL function_list_get_code_list(tPCELL flist);
void function_list_set_code_list(tPCELL flist, tPCELL clist);
tPCELL function_list_get_function(tPCELL flist);
void function_list_set_function(tPCELL flist, tPCELL func);

// method-list
VM_RET method_list_create(tPVM vm, tPCELL pplist, tPCELL clist, tPCELL envlist, const tINT qualifier, tPCELL* mlist);
tPCELL mlist_get_pplist(tPCELL mlist);
tPCELL mlist_get_clist(tPCELL mlist);
tPCELL mlist_get_env(tPCELL mlist);
tINT mlist_get_qualifier(tPCELL mlist);

// paramter-list
VM_RET parameter_list_create(tPVM vm, tPCELL* plist);
void parameter_list_finish_initialization(tPCELL plist);
void parameter_list_set_rest(tPCELL plist);
tBOOL parameter_list_is_rest(tPCELL plist);
tPCELL parameter_list_get_name_list(tPCELL plist);
void parameter_list_set_stack(tPCELL plist, tINT sp);
void parameter_list_set_heap(tPCELL plist);
tBOOL parameter_list_is_stack(tPCELL plist);
tINT parameter_list_get_stack(tPCELL plist);
tBOOL parameter_list_is_heap(tPCELL plist);
tINT parameter_list_get_number(tPCELL plist);
VM_RET parameter_list_add_parameter(tPVM vm, tPCELL plist, tPOBJECT name);

// parameter-profiler-list
VM_RET parameter_profiler_list_create(tPVM vm, tPCELL* pplist);
void pplist_finish_initialization(tPCELL pplist);
void pplist_set_rest(tPCELL pplist);
tBOOL pplist_is_rest(tPCELL pplist);
tPCELL pplist_get_plist(tPCELL pplist);
tPCELL pplist_get_slist(tPCELL pplist);
VM_RET pplist_add_profiler(tPVM vm, tPCELL pplist, tPOBJECT name, tPOBJECT specializer);
tINT pplist_get_number(tPCELL pplist);
tPCELL pplist_get_profiler_list(tPCELL pplist);
tPCELL pplist_get_name_list(tPCELL pplist);

// code-list
VM_RET code_list_create(tPVM vm, tPCELL* clist);
VM_RET code_list_add_command_1(tPVM vm, tPCELL clist, const tINT code);
VM_RET code_list_add_argument(tPVM vm, tPCELL clist, tPOBJECT obj);
tPCELL code_list_get_head(tPCELL clist);
void code_list_increment_head(tPCELL* head);
tINT code_list_get_command(tPCELL head);
tINT code_list_get_max_sp(tPCELL clist);
void code_list_set_max_sp(tPCELL clist, tINT max);

// function-name-list

void function_name_list_set_number(tPCELL flist, const tINT n);
void function_name_list_increment_number(tPCELL flist);
void function_name_list_finish_initialization(tPCELL flist);
tBOOL function_name_list_is_referred(tPCELL flist);
tINT function_name_list_get_size(tPCELL flist);
void function_name_list_set_referred(tPCELL flist);

// tagbody_tag_list

void tagbody_tag_list_set_start_tag(tPCELL tlist);
tINT tagbody_tag_list_get_size(tPCELL tlist);
tBOOL tagbody_tag_list_start_tag(tPCELL tlist);

// method-env
VM_RET t1_create_method_env(tPVM vm, tPCELL* env);

/////////////////////////////
// 変換機

VM_RET create_translator(tPVM vm, tTRANSLATOR* translator);
void free_translator(tTRANSLATOR translator);
VM_RET translator_initialize(tPVM vm);

tBOOL flist_is_call_next_method(tPVM vm, tPCELL flist);
tBOOL flist_is_next_method_p(tPVM vm, tPCELL flist);

///////////////////
// pass1

void t1_clear(tTRANSLATOR trans);
//
void t1_increment_form_level(tPVM vm);
void t1_decrement_form_level(tPVM vm);
void t1_increment_quasiquote_level(tPVM vm);
void t1_decrement_quasiquote_level(tPVM vm);
tINT t1_get_quasiquote_level(tPVM vm);
tPCELL t1_get_defining_function_name(tPVM vm);
void t1_set_defining_function_name(tPVM vm, tPCELL name);
tPCELL t1_get_defining_function_parameter_list(tPVM vm);
void t1_set_defining_function_parameter_list(tPVM vm, tPCELL plist);
tPCELL t1_get_argument_point(tPVM vm);
void t1_set_argument_point(tPVM vm, tPCELL ap);
// 変数
VM_RET t1_push_variable(tPVM vm, tPCELL parameter_list);
void t1_pop_variable(tPVM vm);
tBOOL t1_search_variable(tPVM vm, tPCELL name);
// 関数
VM_RET t1_push_function(tPVM vm, tPCELL func_list);
void t1_pop_function(tPVM vm);
tBOOL t1_search_function(tPVM vm, tPCELL name, tPCELL* flist);
// block
VM_RET t1_push_block(tPVM vm, tPOBJECT block);
void t1_pop_block(tPVM vm, tPCELL* block_list);
tBOOL t1_search_block(tPVM vm, tPOBJECT name, tPCELL* block_list);
// tagbody
VM_RET t1_push_tagbody(tPVM vm, tPCELL tagbodytag);
void t1_pop_tagbody(tPVM vm);
tBOOL t1_search_tagbody(tPVM vm, tPCELL name, tPCELL* ret);

///////////////////
// pass2

void t2_clear(tTRANSLATOR trans);
tINT t2_get_defining_function_parameter(tPVM vm);
void t2_set_defining_function_parameter(tPVM vm, tINT pnum);
// 辞書
VM_RET t2_push_name(tPVM vm, tPCELL list);
void t2_pop_name(tPVM vm);
tBOOL t2_search_variable(tPVM vm, tPCELL name, tINT* offset, tBOOL* stack);
tBOOL t2_search_function(tPVM vm, tPCELL name, tPCELL* f, tINT* i);
// スタックのトレース
VM_RET t2_push_stack(tPVM vm, tPCELL command);
VM_RET t2_pop_stack(tPVM vm, const tINT x);
tINT t2_get_sp(tPVM vm);
void t2_set_sp(tPVM vm, tINT sp);
tINT t2_get_max(tPVM vm);
void t2_set_max(tPVM vm, tINT max);
void t2_marge_max(tPVM vm, tINT max);
void t2_set_next_method(tPVM vm);
void t2_reset_next_method(tPVM vm);
tBOOL t2_use_next_method(tPVM vm);
void t2_set_method_qualifier(tPVM vm, tINT qualifier);
tINT t2_get_method_qualifier(tPVM vm);
// 命令の置き換え
void t2_code_list_set_command(tPCELL* head, tINT command);
void t2_increment_head(tPCELL* head);
void t2_code_list_set_operand(tPCELL* head, tPOBJECT operand);
void t2_code_list_close(tPCELL head);
// 命令の引数の取得
void t2_get_operand(tPCELL head, const tINT x, tPOBJECT operand);

///////////////////
// pass3

void t3_clear(tTRANSLATOR trans);

#endif
