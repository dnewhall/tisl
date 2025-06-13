//
// TISL/src/tisl/tisl.c
// TISL Ver 4.x
//

#include <malloc.h>
#include <string.h>
#include <signal.h>
#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "built_in_object.h"

/////////////////////////////////////////////////

struct tTISL_ {
	//
	tPVM	main_vm;
	//
	tINT	tisl_state;
	//
	tPCELL	top_package;
	//
	tOBJECT	standard_input;
	tOBJECT	standard_output;
	tOBJECT	error_output;
	//
	tPCELL	main_argument;
	tPCELL	main_environment;
	//
	tBOOL	gc_mark;
	tPVM	gc_lock[2];
	//
	tINT	init_stack_size;
	tINT	max_stack_size;
	//
	tPVM	vms;
	tPVM	dead_vms;
	//
	tPCELL*	string_table;
	tUINT	string_table_size;
	tPVM	string_table_lock[2];
	//
	tPCELL*	symbol_table;
	tUINT	symbol_table_size;
	tPVM	symboL_table_lock[2];
	tINT	gensym;
	//
	tPCELL* file_stream_table;
	tUINT	file_stream_table_size;
	tPVM	file_stream_table_lock[2];
	//
	tPVM	bind_lock[2];
	//
	TISL*	tisl;
};

/////////////////////////////////////////////////

static VM_RET initialize_standard_stream(tPTISL tisl, tPVM vm, tCSTRING name, FILE* file, const tINT io_flag, tPOBJECT stream);
static VM_RET initialize_main_arg(tPTISL tisl, int argc, char* argv[], char* envp[]);

static VM_RET tisl_lock_string_table(tPTISL tisl, tPVM vm);
static void tisl_unlock_string_table(tPTISL tisl, tPVM vm);
static tINT tisl_get_string_table_key(tPTISL tisl, tCSTRING string);
static tBOOL tisl_search_string(tPTISL tisl, tPVM vm, tCSTRING string, tPCELL* cell);
static VM_RET tisl_create_string(tPTISL tisl, tPVM vm, tCSTRING string, tPCELL* cell);

static VM_RET tisl_lock_symbol_table(tPTISL tisl, tPVM vm);
static void tisl_unlock_symbol_table(tPTISL tisl, tPVM vm);
static tINT tisl_get_symbol_table_key(tPTISL tisl, tCSTRING string);
static tBOOL tisl_search_symbol(tPTISL tisl, tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell);
static tBOOL symbol_equal_list(tPCELL symbol, tPCELL list, tBOOL complete);
static VM_RET tisl_create_symbol(tPTISL tisl, tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell);
static VM_RET tisl_create_gensym(tPTISL tisl, tPVM vm, tPCELL* cell);
static tBOOL tisl_search_simple_symbol(tPTISL tisl, tPVM vm, tPCELL string, tBOOL complete, tPCELL* cell);
static VM_RET tisl_create_simple_symbol(tPTISL tisl, tPVM vm, tPCELL string, tBOOL complete, tPCELL* cell);
static tBOOL symbol_equal_simple(tPCELL symbol, tPCELL string, tBOOL complete);

static VM_RET tisl_lock_file_stream_table(tPTISL tisl, tPVM vm);
static void tisl_unlock_file_stream_table(tPTISL tisl, tPVM vm);
static tINT tisl_get_file_stream_table_key(tPTISL tisl, FILE* file);
static VM_RET tisl_search_file_stream(tPTISL tisl, tPVM vm, tPCELL name, const tINT flags, FILE* file, tPCELL* cell);
static VM_RET tisl_create_file_stream(tPTISL tisl, tPVM vm, tPCELL name, const tINT flags, FILE* file, tPCELL* cell);

static void tisl_set_state(tPTISL tisl, const int state);

extern void gc_mark_cell(tPGC gc, tPCELL cell);

/////////////////////////////////////////////////

void tisl_set_interface(tPTISL tisl, TISL* i)
{
	tisl->tisl=i;
}

TISL* tisl_get_interface(tPTISL tisl)
{
	return tisl->tisl;
}

// TIN_INIT_ARGSからTISL_INIT_ARGSを作成する
void set_tisl_init_args(tTISL_INIT_ARGS* tisl_args, TNI_INIT_ARGS* tni_args)
{
	tisl_args->argc=tni_args->argc;
	tisl_args->argv=tni_args->argv;
	tisl_args->envp=tni_args->envp;
}

void tisl_set_vm_init_args(tPTISL tisl, tVM_INIT_ARGS* vm_args)
{
	vm_args->init_stack_size=tisl->init_stack_size;
	vm_args->max_stack_size=tisl->max_stack_size;
}

// 
tBOOL create_tisl(tPTISL* tisl, tTISL_INIT_ARGS* args, tVM_INIT_ARGS* vm_args)
{
	tPTISL tisl_;
	tPVM vm;
	// 大域変数のクリア
	clear_global_objects();
	//
	tisl_=malloc(sizeof(tTISL));
	if (!tisl_) return tFALSE;

	tisl_->main_vm=0;
	tisl_->top_package=0;
	OBJECT_SET_NIL(&tisl_->standard_input);
	OBJECT_SET_NIL(&tisl_->standard_output);
	OBJECT_SET_NIL(&tisl_->error_output);
	tisl_->init_stack_size=vm_args->init_stack_size;
	tisl_->max_stack_size=vm_args->max_stack_size;
	tisl_->vms=0;
	tisl_->dead_vms=0;
	tisl_->main_argument=0;
	tisl_->main_environment=0;
	tisl_set_state(tisl_, TISL_STATE_INITIALIZATION);

	// gc
	tisl_->gc_mark=tFALSE;
	tisl_->gc_lock[0]=0;
	tisl_->gc_lock[1]=0;
	// string table
	tisl_->string_table=calloc(sizeof(tPCELL), STRING_TABLE_SIZE);
	tisl_->string_table_size=STRING_TABLE_SIZE;
	if (!tisl_->string_table) goto ERROR;
	tisl_->string_table_lock[0]=0;
	tisl_->string_table_lock[1]=0;
	// symbol table
	tisl_->symbol_table=calloc(sizeof(tPCELL), SYMBOL_TABLE_SIZE);
	tisl_->symbol_table_size=SYMBOL_TABLE_SIZE;
	if (!tisl_->symbol_table) goto ERROR1;
	tisl_->symboL_table_lock[0]=0;
	tisl_->symboL_table_lock[1]=0;
	tisl_->gensym=0;
	// bind-list-lock
	tisl_->bind_lock[0]=0;
	tisl_->bind_lock[1]=0;
	// file stream table
	tisl_->file_stream_table=calloc(sizeof(tPCELL), FILE_STREAM_TABLE_SIZE);
	tisl_->file_stream_table_size=FILE_STREAM_TABLE_SIZE;
	if (!tisl_->file_stream_table) goto ERROR2;
	tisl_->file_stream_table_lock[0]=0;
	tisl_->file_stream_table_lock[1]=0;
	// Main VM の生成
	if (!create_vm(tisl_, &vm, vm_args)) goto ERROR3;
	tisl_->main_vm=vm;
	vm_set_gc_mark(vm, tFALSE);
	//　大域変数の初期化
	if (initialize_global_objects(tisl_)) goto ERROR4;
	// top-package
	if (package_create_(tisl_->main_vm, 0, 0, 0, 0, &tisl_->top_package)) goto ERROR4;
	// 標準ストリームの生成
	// 標準入力
	if (initialize_standard_stream(tisl_, tisl_->main_vm, "standard-input", stdin, STREAM_INPUT, &tisl_->standard_input)) goto ERROR4;
	// 標準出力
	if (initialize_standard_stream(tisl_, tisl_->main_vm, "standard-output", stdout, STREAM_OUTPUT, &tisl_->standard_output)) goto ERROR4;
	// エラー出力
	if (initialize_standard_stream(tisl_, tisl_->main_vm, "error-output", stderr, STREAM_OUTPUT, &tisl_->error_output)) goto ERROR4;
	// 組込みオブジェクトの生成
	if (initialize_built_in_object(tisl_)) goto ERROR4;
	//
	if (initialize_main_arg(tisl_, args->argc, args->argv, args->envp)) goto ERROR4;

	*tisl=tisl_;
	signal(SIGINT, tisl_signal_user_interrupt);
	tisl_set_state(tisl_, TISL_STATE_OK);
	return tTRUE;
ERROR4:
	free_vm(vm);
ERROR3:
	free(tisl_->file_stream_table);
ERROR2:
	free(tisl_->symbol_table);
ERROR1:
	free(tisl_->string_table);
ERROR:
	free(tisl_);
	return tFALSE;
}

tBOOL free_tisl(tPTISL tisl)
{/*!!!*///後で書き直す
	if (tisl) {
		free_vm(tisl->main_vm);
		free(tisl->symbol_table);
		free(tisl->string_table);
		free(tisl->file_stream_table);
		free(tisl);
	}
	return tTRUE;
}

void tisl_attach_vm(tPTISL tisl, tPVM vm)
{
	vm_set_next(vm, tisl->vms);
	tisl->vms=vm;
}

void tisl_detach_vm(tPTISL tisl, tPVM vm)
{
	tPVM p, last;
	// mainははずせない
	if (vm==tisl->main_vm) return;
	//
	for (p=tisl->vms, last=0; p&&(p!=vm); last=p, p=vm_get_next(vm));
	if (p==vm) {
		if (last) {
			vm_set_next(last, vm_get_next(vm));
		} else {
			tisl->vms=vm_get_next(vm);
		}
		vm_set_next(vm, 0);
	}
}

static VM_RET initialize_standard_stream(tPTISL tisl, tPVM vm, tCSTRING name, FILE* file, const tINT io_flag, tPOBJECT stream)
{
	if (file) {
		tINT key;
		tPCELL cell, string;
		if (tisl_get_string(tisl, vm, name, &string)||
			file_stream_create_(vm, io_flag, string, file, &cell)) return VM_ERROR;
		cell_to_object(cell, stream);
		// 表に登録
		key=tisl_get_file_stream_table_key(tisl, file);
		file_stream_set_next(cell, tisl->file_stream_table[key]);
		tisl->file_stream_table[key]=cell;
	} else {
		OBJECT_SET_UNBOUND(stream);
	}

	return VM_OK;
}

static VM_RET initialize_main_arg(tPTISL tisl, int argc, char* argv[], char* envp[])
{
	tPVM vm=tisl_get_main_vm(tisl);
	// argument
	if (argc) {
		tPCELL tail, p, string;
		tOBJECT tmp;
		int i;
		if (tisl_get_string(tisl, vm, argv[0], &string)) return VM_ERROR;
		OBJECT_SET_STRING(&tmp, string);
		if (cons_create(vm, &p, &tmp, &nil)) return VM_ERROR;
		tisl->main_argument=p;
		for (i=1; i<argc; i++) {
			tail=p;
			if (cons_create_(vm, &p, &nil, &nil)) return VM_ERROR;
			OBJECT_SET_CONS(&tmp, p);
			cons_set_cdr(tail, &tmp);
			if (tisl_get_string(tisl, vm, argv[i], &string)) return VM_ERROR;
			OBJECT_SET_STRING(&tmp, string);
			cons_set_car(p, &tmp);
		}
	}
	// environment
	if (envp) {
		tPCELL tail, p, string;
		tOBJECT tmp;
		char** i;
		if (tisl_get_string(tisl, vm, envp[0], &string)) return VM_ERROR;
		OBJECT_SET_STRING(&tmp, string);
		if (cons_create(vm, &p, &tmp, &nil)) return VM_ERROR;
		tisl->main_environment=p;
		for (i=envp+1; *i; i++) {
			tail=p;
			if (cons_create(vm, &p, &nil, &nil)) return VM_ERROR;
			OBJECT_SET_CONS(&tmp, p);
			cons_set_cdr(tail, &tmp);
			if (tisl_get_string(tisl, vm, *i, &string)) return VM_ERROR;
			OBJECT_SET_STRING(&tmp, string);
			cons_set_car(p, &tmp);
		}
	}
	return VM_OK;
}

tPCELL tisl_get_argument(tPTISL tisl)
{
	return tisl->main_argument;
}

tPCELL tisl_get_environment(tPTISL tisl)
{
	return tisl->main_environment;
}

/////////////////////////////////////////////////

tPVM tisl_get_main_vm(tPTISL tisl)
{
	return tisl->main_vm;
}

tPCELL tisl_get_top_package(tPTISL tisl)
{
	return tisl->top_package;
}

/////////////////////////////////////////////////
// lock unlock

VM_RET tisl_lock(tPVM* lock, tPVM vm)
{
	// LOOPの回数でdead-lockの検査する？
LOOP:
	if (lock[0]&&(lock[0]!=vm)) { /*wait入れたい*//*!!!*/ goto LOOP; }
	lock[0]=vm;
	if (lock[1]&&(lock[1]!=vm)) { /*wait入れたい*//*!!!*/ 
		goto LOOP;
	}
	lock[1]=vm;
	if (lock[0]!=vm) {
		if (lock[1]==vm) lock[1]=0;
		goto LOOP;
	}
	return VM_OK;
}

void tisl_unlock(tPVM* lock)
{
	lock[0]=0;
	lock[1]=0;
}

// string table
static VM_RET tisl_lock_string_table(tPTISL tisl, tPVM vm)
{
	return tisl_lock(tisl->string_table_lock, vm);
}

static void tisl_unlock_string_table(tPTISL tisl, tPVM vm)
{
	tisl_unlock(tisl->string_table_lock);
}

// symbol table
static VM_RET tisl_lock_symbol_table(tPTISL tisl, tPVM vm)
{
	return tisl_lock(tisl->symboL_table_lock, vm);
}

static void tisl_unlock_symbol_table(tPTISL tisl, tPVM vm)
{
	tisl_unlock(tisl->symboL_table_lock);
}

// file stream table
static VM_RET tisl_lock_file_stream_table(tPTISL tisl, tPVM vm)
{
	return tisl_lock(tisl->file_stream_table_lock, vm);
}

static void tisl_unlock_file_stream_table(tPTISL tisl, tPVM vm)
{
	tisl_unlock(tisl->file_stream_table_lock);
}

/////////////////////////////////////////////////

// 文字列
VM_RET tisl_get_string(tPTISL tisl, tPVM vm, tCSTRING string, tPCELL* cell)
{
	// 既に登録されているものから検索
	if (tisl_search_string(tisl, vm, string, cell)) {
		return VM_OK;
	} else {
		// 新規に作成
		return tisl_create_string(tisl, vm, string, cell);
	}
}

static VM_RET tisl_remove_string_(tPTISL tisl, tPVM vm, tPCELL string);

VM_RET tisl_remove_string(tPTISL tisl, tPVM vm, tPCELL string)
{
	VM_RET ret;
	if (tisl_lock_string_table(tisl, vm)) return VM_ERROR;
	ret=tisl_remove_string_(tisl, vm, string);
	tisl_unlock_string_table(tisl, vm);
	return ret;
}

static tINT tisl_get_string_table_key(tPTISL tisl, tCSTRING string)
{
	// てきとー あとで
	/*!!!*/
	tINT key=0, i, len=strlen(string);
	for (i=0; i<len; i++) {
		key+=string[i];
		key<<=1;
	}
	key=key/13%tisl->string_table_size;
	return (key<0) ? -key : key;
}

static tBOOL tisl_search_string(tPTISL tisl, tPVM vm, tCSTRING string, tPCELL* cell)
{
	tINT key;
	tPCELL p;
	// 表から検索
	if (tisl_lock_string_table(tisl, vm)) return tFALSE;
	key=tisl_get_string_table_key(tisl, string);
	for (p=tisl->string_table[key]; p; p=string_get_next(p)) {
		if (strcmp((char*)string, (char*)string_get_string(p))==0) {
			*cell=p;
			gc_mark_cell(tisl->main_vm->gc, p);
			tisl_unlock_string_table(tisl, vm);
			return tTRUE;
		}
	}
	tisl_unlock_string_table(tisl, vm);
	return tFALSE;
}

static VM_RET tisl_remove_string_(tPTISL tisl, tPVM vm, tPCELL string)
{
	tINT key=tisl_get_string_table_key(tisl, string_get_string(string));
	tPCELL p, last;
	for (p=tisl->string_table[key], last=0; p; last=p, p=string_get_next(p)) {
		if (p==string) {
			if (last) {
				string_set_next(last, string_get_next(p));
			} else {
				tisl->string_table[key]=string_get_next(p);
			}
			return VM_OK;
		}
	}
	return VM_OK;
}

static VM_RET tisl_create_string(tPTISL tisl, tPVM vm, tCSTRING string, tPCELL* cell)
{
	tINT key=tisl_get_string_table_key(tisl, string);
	// とりあえず、要求のあったVM上で文字列を作成する
	if (string_create(vm, string, cell)) return VM_ERROR;
	// 表に登録
	if (tisl_lock_string_table(tisl, vm)) return VM_ERROR;
	string_set_next(*cell, tisl->string_table[key]);
	tisl->string_table[key]=*cell;
	tisl_unlock_string_table(tisl, vm);
	// VMにTISLからの参照の通知
	return VM_OK;
}

/////////////////////////////////////////////////
// 記号
VM_RET tisl_get_symbol(tPTISL tisl, tPVM vm, tPCELL list, const tBOOL complete, tPCELL* cell)
{
	// 既に登録されているものから検索
	if (tisl_search_symbol(tisl, vm, list, complete, cell)) {
		return VM_OK;
	} else {
		// 新規に作成
		return tisl_create_symbol(tisl, vm, list, complete, cell);
	}
}

VM_RET tisl_get_simple_symbol(tPTISL tisl, tPVM vm, tPCELL string, const tBOOL complete, tPCELL* cell)
{
	if (tisl_search_simple_symbol(tisl, vm, string, complete, cell)) {
		return VM_OK;
	} else {
		return tisl_create_simple_symbol(tisl, vm, string, complete, cell);
	}
}

static VM_RET tisl_remove_symbol_(tPTISL tisl, tPVM vm, tPCELL symbol);

VM_RET tisl_remove_symbol(tPTISL tisl, tPVM vm, tPCELL symbol)
{
	VM_RET ret;
	if (tisl_lock_symbol_table(tisl, vm)) return VM_ERROR;
	ret=tisl_remove_symbol_(tisl, vm, symbol);
	tisl_unlock_symbol_table(tisl, vm);
	return ret;
}

VM_RET tisl_gensym(tPTISL tisl, tPVM vm, tPCELL* cell)
{
	return tisl_create_gensym(tisl, vm, cell);
}

static tINT tisl_get_symbol_table_key(tPTISL tisl, tCSTRING string)
{
	// てきとー
	/*!!!*/
	tINT key=0, i, len=strlen(string);
	for (i=0; i<len; i++) {
		key+=string[i];
		key<<=1;
	}
	key=key/13%tisl->string_table_size;
	return (key<0) ? -key : key;
}

static tBOOL tisl_search_symbol(tPTISL tisl, tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell)
{
	tINT key;
	tOBJECT obj;
	tPCELL p;
	if (tisl_lock_symbol_table(tisl, vm)) return tFALSE;
	// keyの取得
	cons_get_car(list, &obj);
	if (!OBJECT_IS_STRING(&obj)) { tisl_unlock_symbol_table(tisl, vm); return tFALSE; }
	key=tisl_get_symbol_table_key(tisl, string_get_string(OBJECT_GET_CELL(&obj)));
	//
	for (p=tisl->symbol_table[key]; p; p=symbol_get_next(p)) {
		if (symbol_equal_list(p, list, complete)) {
			*cell=p;
			gc_mark_cell(tisl->main_vm->gc, p);
			tisl_unlock_symbol_table(tisl, vm);
			return tTRUE;
		}
	}
	tisl_unlock_symbol_table(tisl, vm);
	return tFALSE;
}

static tBOOL tisl_search_simple_symbol(tPTISL tisl, tPVM vm, tPCELL string, tBOOL complete, tPCELL* cell)
{
	tINT key;
	tPCELL p;

	if (!CELL_IS_STRING(string)) return tFALSE;
	if (tisl_lock_symbol_table(tisl, vm)) return tFALSE;
	key=tisl_get_symbol_table_key(tisl, string_get_string(string));
	for (p=tisl->symbol_table[key]; p; p=symbol_get_next(p)) {
		if (symbol_equal_simple(p, string, complete)) {
			*cell=p;
			gc_mark_cell(tisl->main_vm->gc, p);
			tisl_unlock_symbol_table(tisl, vm);
			return tTRUE;
		}
	}
	tisl_unlock_symbol_table(tisl, vm);
	return tFALSE;
}

static tBOOL symbol_equal_list(tPCELL symbol, tPCELL list, tBOOL complete)
{
	tINT i, len;
	tPCELL p, string;
	tOBJECT obj;
	// 
	if ((symbol_is_complete(symbol)&&!complete)||
		(!symbol_is_complete(symbol)&&complete)) return tFALSE;
	// 長さの比較
	len=cons_get_length(list);
	if (len!=symbol_get_length(symbol)) return tFALSE;
	// 各要素の比較
	for (i=0, p=list; p; i++, p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &obj);
		if (!OBJECT_IS_STRING(&obj)) return tFALSE;
		if (!symbol_get_string(symbol, i, &string)) return tFALSE;
		if (OBJECT_GET_CELL(&obj)!=string) return tFALSE;
	}
	return (i==len) ? tTRUE : tFALSE;
}

static tBOOL symbol_equal_simple(tPCELL symbol, tPCELL string, tBOOL complete)
{
	tPCELL p;
	if ((symbol_is_complete(symbol)&&!complete)||
		(!symbol_is_complete(symbol)&&complete)) return tFALSE;
	if (symbol_get_length(symbol)!=1) return tFALSE;
	if (!symbol_get_string(symbol, 0, &p)) return tFALSE;
	return (p==string) ? tTRUE : tFALSE;
}

static VM_RET tisl_create_symbol(tPTISL tisl, tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell)
{
	tINT key;
	tOBJECT obj;

	cons_get_car(list, &obj);
	if (!OBJECT_IS_STRING(&obj)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	key=tisl_get_symbol_table_key(tisl, string_get_string(OBJECT_GET_CELL(&obj)));
	// 要求のあったVM上で記号を作成
	if (symbol_create(vm, list, complete, cell)) return VM_ERROR;
	// 表に登録
	if (tisl_lock_symbol_table(tisl, vm)) return VM_ERROR;
	symbol_set_next(*cell, tisl->symbol_table[key]);
	symbol_set_key(*cell, key);
	tisl->symbol_table[key]=*cell;
	tisl_unlock_symbol_table(tisl, vm);
	return VM_OK;
}

static VM_RET tisl_create_simple_symbol(tPTISL tisl, tPVM vm, tPCELL string, tBOOL complete, tPCELL* cell)
{
	tINT key;
	key=tisl_get_symbol_table_key(tisl, string_get_string(string));
	if (symbol_create_simple(vm, string, complete, cell)) return VM_ERROR;
	if (tisl_lock_symbol_table(tisl, vm)) return VM_ERROR;
	symbol_set_next(*cell, tisl->symbol_table[key]);
	symbol_set_key(*cell, key);
	tisl->symbol_table[key]=*cell;
	tisl_unlock_symbol_table(tisl, vm);
	return VM_OK;
}

static VM_RET tisl_create_gensym(tPTISL tisl, tPVM vm, tPCELL* cell)
{
	tINT key;
	tPCELL string;
	char	buffer[256];
	++tisl->gensym;
	key=tisl->gensym%tisl->symbol_table_size;
	sprintf(buffer, "#g%d", tisl->gensym);
	if (tisl_get_string(tisl, vm, buffer, &string)) return VM_ERROR;
	if (symbol_create_gensym(vm, string, cell)) return VM_ERROR;
	if (tisl_lock_symbol_table(tisl, vm)) return VM_ERROR;
	symbol_set_next(*cell, tisl->symbol_table[key]);
	symbol_set_key(*cell, key);
	tisl->symbol_table[key]=*cell;
	tisl_unlock_symbol_table(tisl, vm);
	return VM_OK;
}

static VM_RET tisl_remove_symbol_(tPTISL tisl, tPVM vm, tPCELL symbol)
{
	tINT key;
	tPCELL p, last;
	key=symbol_get_key(symbol);
	for (p=tisl->symbol_table[key], last=0; p; last=p, p=symbol_get_next(p)) {
		if (p==symbol) {
			if (last) {
				symbol_set_next(last, symbol_get_next(p));
			} else {
				tisl->symbol_table[key]=symbol_get_next(p);
			}
			return VM_OK;
		}
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

///////////////////////////////////////
// bind-listの取得


VM_RET tisl_get_bind_list(tPTISL tisl, tPVM vm, tPCELL package, tPCELL name, tPCELL* blist)
{
	VM_RET ret;
	if (tisl_lock(tisl->bind_lock, vm)) return VM_ERROR;
	// packageからnameで参照できる束縛のリストの作成
	*blist=package_get_bind_list(package, name);
	if (!*blist&&
		package_add_bind_list(vm, package, name, blist)) {
		ret=VM_ERROR;
	} else {
		ret=VM_OK;
	}
	tisl_unlock(tisl->bind_lock);
	return ret;
}

// 束縛の取得
VM_RET tisl_get_bind(tPTISL tisl, tPVM vm, tPCELL package, tPCELL name, tPCELL* bind)
{
	VM_RET ret;
	if (tisl_lock(tisl->bind_lock, vm)) return VM_ERROR;
	*bind=package_get_bind(package, name);
	if (!*bind) {
		ret=package_add_bind(vm, package, name, bind);
	} else {
		ret=VM_OK;
	}
	tisl_unlock(tisl->bind_lock);
	return ret;
}

VM_RET tisl_get_variable(tPTISL tisl, tPVM vm, tPCELL name, tPOBJECT obj)
{
	tPCELL blist, bind, package;
	package=vm_get_current_package(vm);
	if (tisl_get_bind_list(tisl, vm, package, name, &blist)) return VM_ERROR;
	bind=bind_list_get_bind(blist, NAMESPACE_VARIABLE, package);
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNBOUND_VARIABLE, name, NAMESPACE_VARIABLE);
	bind_get_variable(bind, obj);
	return VM_OK;
}

VM_RET tisl_get_class(tPTISL tisl, tPVM vm, tPCELL name, tPOBJECT obj)
{
	tPCELL blist, bind, package;
	package=vm_get_current_package(vm);
	if (tisl_get_bind_list(tisl, vm, package, name, &blist)) return VM_ERROR;
	bind=bind_list_get_bind(blist, NAMESPACE_CLASS, package);
	if (!bind) return signal_undefined_entity(vm, TISL_ERROR_UNDEFINED_ENTITY, name, NAMESPACE_CLASS);
	bind_get_class(bind, obj);
	return VM_OK;
}

///////////////////////////////////////
// ファイルストリーム

VM_RET tisl_get_file_stream(tPTISL tisl, tPVM vm, tPCELL name, const tINT flags, tPCELL* cell)
{
	VM_RET ret;
	FILE* file;
	if (tisl_lock_file_stream_table(tisl, vm)) return VM_ERROR;

	// ファイルのopen
	if ((flags&STREAM_INPUT)&&(flags&STREAM_OUTPUT)) {
		file=fopen(string_get_string(name), "w+");
	} else if (flags&STREAM_OUTPUT) {
		file=fopen(string_get_string(name), "w");
	} else if (flags&STREAM_INPUT) {
		file=fopen(string_get_string(name), "r");
	} else {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	if (!file) return signal_condition(vm, TISL_ERROR_CANNOT_OPEN_FILE);
	// 既に登録されているものから検索
	if (tisl_search_file_stream(tisl, vm, name, flags, file, cell)) {
		ret=VM_ERROR;
	} else {
		if (*cell) {
			ret=VM_OK;
		} else {
			// 新規に作成
			ret=tisl_create_file_stream(tisl, vm, name, flags, file, cell);
		}
	}
	tisl_unlock_file_stream_table(tisl, vm);
	return ret;
}

static tINT tisl_get_file_stream_table_key(tPTISL tisl, FILE* file)
{
	// てきとー
	/*!!!*/
	tINT key=(*(tUINT*)file)/13%tisl->file_stream_table_size;
	return (key<0) ? -key : key;
}

static VM_RET tisl_search_file_stream(tPTISL tisl, tPVM vm, tPCELL name, const tINT flags, FILE* file, tPCELL* cell)
{
	tINT key;
	tPCELL p;
	// keyの取得
	key=tisl_get_file_stream_table_key(tisl, file);
	//
	for (p=tisl->file_stream_table[key]; p; p=file_stream_get_next(p)) {
		if (file_stream_get_file(p)==file) {
			if ((file_stream_is_output(p)&&!(flags&STREAM_OUTPUT))||
				(!file_stream_is_output(p)&&(flags&STREAM_OUTPUT))||
				(file_stream_is_input(p)&&!(flags&STREAM_INPUT))||
				(!file_stream_is_input(p)&&(flags&STREAM_INPUT))) {
				// 既に開いているファイルストリームと入出力フラグの異なる
				// ファイルを開こうとしている？しかもFILEポインタが同じ!?
				// 許される？/*!!!*/// ありうる？
				//fclose(file);
				return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
			} else {
				*cell=p;
				return VM_OK;
			}
		}
	}
	*cell=0;
	return VM_OK;
}

static VM_RET tisl_create_file_stream(tPTISL tisl, tPVM vm, tPCELL name, const tINT flags, FILE* file, tPCELL* cell)
{
	tINT key;
	// 要求のあったVM上で作成
	if (file_stream_create_(vm, flags, name, file, cell)) return VM_ERROR;
	key=tisl_get_file_stream_table_key(tisl, file);
	// 表に登録
	file_stream_set_next(*cell, tisl->file_stream_table[key]);
	tisl->file_stream_table[key]=*cell;
	return VM_OK;
}

// 標準ストリーム
void tisl_get_standard_input(tPTISL tisl, tPOBJECT stream)
{
	*stream=tisl->standard_input;
}

void tisl_get_standard_output(tPTISL tisl, tPOBJECT stream)
{
	*stream=tisl->standard_output;
}

void tisl_get_error_output(tPTISL tisl, tPOBJECT stream)
{
	*stream=tisl->error_output;
}

/////////////////////////////
// GC
// gc.cに移動する？

static VM_RET tisl_gc_lock(tPTISL tisl, tPVM vm);
static void tisl_gc_wait(tPTISL tisl);
static void tisl_allocate_wait(tPTISL tisl);
extern tBOOL vm_mark(tPVM vm, const tINT sp);
extern tBOOL vm_sweep(tPVM vm);

VM_RET tisl_signal_gc_start(tPTISL tisl, tPVM vm)
{
	vm_set_state_gc_wait(vm);
	if (tisl_gc_lock(tisl, vm)) {
		// 他のVMがGCを起動しているらしい
		// GCが終了するまで待つ
		tisl_gc_wait(tisl);
		return VM_OK;
	} else {
		// 自分がGCの起動VMとなる
		tPCELL* p;
		tUINT i;
		int old;
		tPVM pvm;
		tINT sp2, sp=vm->SP-vm->stack;

		old=tisl_get_state(tisl);
		tisl_set_state(tisl, TISL_STATE_GC_WAIT);
		tisl_allocate_wait(tisl);
		if (!vm_sweep(tisl->main_vm)) goto ERROR;
		for (pvm=tisl->vms; pvm; pvm=vm_get_next(pvm)) {
			if (!vm_sweep(pvm)) goto ERROR;
		}
		for (pvm=tisl->dead_vms; pvm; pvm=vm_get_next(pvm)) {
			if (!vm_sweep(pvm)) goto ERROR;
		}
		// マークの反転
		tisl_reverse_gc_mark(tisl);
		vm_set_state_gc_run(vm);
		// 起動VMはTISLの持つデータへのマーキングを行う
		if (cell_mark(vm, tisl->top_package)||
			object_mark(vm, &tisl->standard_input)||
			object_mark(vm, &tisl->standard_output)||
			object_mark(vm, &tisl->error_output)) goto ERROR;
		// 
		p=tisl->file_stream_table;
		for (i=0; i<tisl->file_stream_table_size; i++) {
			if (cell_mark(vm, *p++)) goto ERROR;
		}
		// 組込みオブジェクト
		if (cell_mark(vm, list_islisp_system)) goto ERROR;
		//
		if (cell_mark(vm, tisl->main_argument)) goto ERROR;
		if (cell_mark(vm, tisl->main_environment)) goto ERROR;
		if (cell_mark(vm, condition_system_error)) goto ERROR;
		if (cell_mark(vm, condition_storage_exhausted)) goto ERROR;
		if (cell_mark(vm, condition_stack_overflow)) goto ERROR;
		if (cell_mark(vm, tisl_object_storage_exhausted)) goto ERROR;
		// string
		if (cell_mark(vm, string_plus)||
			cell_mark(vm, string_minus)||
			cell_mark(vm, string_system)||
			cell_mark(vm, string_islisp)||
			cell_mark(vm, string_continue_condition)||
			cell_mark(vm, string_simple_error)||
			cell_mark(vm, string_system_error)||
			cell_mark(vm, string_stack_overflow)||
			cell_mark(vm, string_storage_exhausted)) goto ERROR;
		// symbol
		for (i=0; i<NUMBER_OF_GLOBAL_SYMBOL; i++) {
			if (cell_mark(vm, global_symbol[i])) goto ERROR;
		}
		//VMのマーキング
		sp2 = (tisl->main_vm==vm) ? sp : (tisl->main_vm->SP-tisl->main_vm->stack);
		if (!vm_mark(tisl->main_vm, sp2)) goto ERROR;
		for (pvm=tisl->vms; pvm; pvm=vm_get_next(pvm)) {
			sp2 = (pvm==vm) ? sp : (pvm->SP-pvm->stack);
			if (!vm_mark(pvm, sp2)) goto ERROR;
		}
		// 廃棄VMのヒープを回収するように変更すべき/*!!!*/
		for (pvm=tisl->dead_vms; pvm; pvm=vm_get_next(pvm)) {
			if (!vm_mark(pvm, pvm->SP-pvm->stack)) goto ERROR;
		}
		vm_set_state_ok(vm);
		tisl_set_state(tisl, old);
		tisl->gc_lock[1]=0;
		tisl->gc_lock[0]=0;
		return VM_OK;
ERROR:
		vm_set_state_ok(vm);
		tisl_set_state(tisl, old);
		tisl->gc_lock[1]=0;
		tisl->gc_lock[0]=0;
		return VM_ERROR;
	}
}

void tisl_reverse_gc_mark(tPTISL tisl)
{
	tPVM vm;
	tisl->gc_mark=tisl->gc_mark ? tFALSE : tTRUE;
	vm_set_gc_mark(tisl->main_vm, tisl->gc_mark);
	for (vm=tisl->vms; vm; vm=vm_get_next(vm)) {
		vm_set_gc_mark(vm, tisl->gc_mark);
	}
}

static VM_RET tisl_gc_lock(tPTISL tisl, tPVM vm)
{
LOOP:
	if ((tisl->gc_lock[0]!=0)&&
		(tisl->gc_lock[0]!=vm)) return VM_ERROR;
	tisl->gc_lock[0]=vm;
	if ((tisl->gc_lock[1]!=0)&&
		(tisl->gc_lock[1]!=vm)) goto LOOP;
	tisl->gc_lock[1]=vm;
	if (tisl->gc_lock[0]!=vm) {
		if (tisl->gc_lock[1]==vm) tisl->gc_lock[1]=0;
		goto LOOP;
	}
	return VM_OK;
}

static void tisl_gc_wait(tPTISL tisl)
{
	tPVM vm;
LOOP:
	// busy waitになっている!!!/*!!!*/
	if (vm_get_state(tisl->main_vm)!=VM_STATE_OK) goto LOOP;
	for (vm=tisl->vms; vm; vm=vm_get_next(vm)) {
		if (vm_get_state(vm)!=VM_STATE_OK) goto LOOP;
	}
}

static void tisl_allocate_wait(tPTISL tisl)
{
	tPVM vm;
LOOP:
	if (vm_get_state(tisl->main_vm)==VM_STATE_ALLOCATE) goto LOOP;
	for (vm=tisl->vms; vm; vm=vm_get_next(vm)) {
		if (vm_get_state(tisl->main_vm)==VM_STATE_ALLOCATE) goto LOOP;
	}
}

void tisl_gc_wait_2(tPTISL tisl, tPVM vm)
{
LOOP:// busy waitになっている何かない？
	if ((tisl->gc_lock[0]!=vm)&&
		(tisl->tisl_state==TISL_STATE_GC_WAIT)) goto LOOP;
}

// ctrl-Cによる中断
static int user_interrupt_flag;

void tisl_signal_user_interrupt(int sig)
{
	user_interrupt_flag=1;
	signal(SIGINT, tisl_signal_user_interrupt);
}

int tisl_get_user_interrupt_flag(void)
{
	return user_interrupt_flag;
}

void tisl_reset_user_interrupt_flag(void)
{
	user_interrupt_flag=0;
}

extern tBOOL garbage_collection(tPVM vm, tUINT size);

VM_RET vm_check_tisl_state(tPVM vm)
{
	tPTISL tisl=vm_get_tisl(vm);
	// 他のVMにより起動されたGC
	if (tisl_get_state(tisl)==TISL_STATE_GC_WAIT) {
		// 誰かがGCを起動しているらしい．
		tisl_gc_wait(tisl);
	}
	// ユーザによる中断？
	if (tisl_get_user_interrupt_flag()) {
		vm_set_last_condition_user_interrupt(vm);
		return VM_ERROR;
	}
	return VM_OK;
}

int tisl_get_state(tPTISL tisl)
{
	return tisl->tisl_state;
}

static void tisl_set_state(tPTISL tisl, const int state)
{
	tisl->tisl_state=state;
}
