//
// TISL/src/tisl/tisl.h
// TISL Ver 4.x
//

#ifndef TISL_H
#define TISL_H

#define STRING_TABLE_SIZE		2003
#define SYMBOL_TABLE_SIZE		2003
#define FILE_STREAM_TABLE_SIZE	256

typedef struct tTISL_INIT_ARGS_		tTISL_INIT_ARGS;

struct tTISL_INIT_ARGS_ {
	//
	int			argc;
	char**		argv;
	char**		envp;
};

/////////////////////////////
// TIN_INIT_ARGSからTISL_INIT_ARGSを作成する
void set_tisl_init_args(tTISL_INIT_ARGS* tisl_args, TNI_INIT_ARGS* tni_args);
// TISLの生成 と main VMの生成
tBOOL create_tisl(tPTISL* tisl, tTISL_INIT_ARGS* args, tVM_INIT_ARGS* vm_args);
tBOOL free_tisl(tPTISL tisl);

void tisl_attach_vm(tPTISL tisl, tPVM vm);
void tisl_detach_vm(tPTISL tisl, tPVM vm);

void tisl_set_vm_init_args(tPTISL tisl, tVM_INIT_ARGS* vm_args);

// main VM 
tPVM tisl_get_main_vm(tPTISL tisl);
// top package
tPCELL tisl_get_top_package(tPTISL tisl);
// 文字列
VM_RET tisl_get_string(tPTISL tisl, tPVM vm, tCSTRING string, tPCELL* cell);
VM_RET tisl_remove_string(tPTISL tisl, tPVM vm, tPCELL string);
// 記号
VM_RET tisl_get_symbol(tPTISL tisl, tPVM vm, tPCELL list, const tBOOL complete, tPCELL* cell);
VM_RET tisl_get_simple_symbol(tPTISL tisl, tPVM vm, tPCELL string, const tBOOL complete, tPCELL* cell);
VM_RET tisl_remove_symbol(tPTISL tisl, tPVM vm, tPCELL symbol);
VM_RET tisl_gensym(tPTISL tisl, tPVM vm, tPCELL* cell);
// ファイルストリーム
VM_RET tisl_get_file_stream(tPTISL tisl, tPVM vm, tPCELL name, const tINT flags, tPCELL* cell);
// 束縛
VM_RET tisl_get_bind(tPTISL tisl, tPVM vm, tPCELL package, tPCELL name, tPCELL* bind);
// 標準ストリーム
void tisl_get_standard_input(tPTISL tisl, tPOBJECT stream);
void tisl_get_standard_output(tPTISL tisl, tPOBJECT stream);
void tisl_get_error_output(tPTISL tisl, tPOBJECT stream);
// 束縛
#define NAMESPACE_VARIABLE		0
#define NAMESPACE_FUNCTION		1
#define NAMESPACE_DYNAMIC		2
#define NAMESPACE_CLASS			3
#define NAMESPACE_PACKAGE		4

VM_RET tisl_get_bind_list(tPTISL tisl, tPVM vm, tPCELL package, tPCELL name, tPCELL* blist);
VM_RET tisl_get_variable(tPTISL tisl, tPVM vm, tPCELL name, tPOBJECT obj);
VM_RET tisl_get_class(tPTISL tisl, tPVM vm, tPCELL name, tPOBJECT obj);

//

#define RETURN_CODE_IS_CR_LF	0
#define RETURN_CODE_IS_CR		1
#define RETURN_CODE_IS_LF		2

int tisl_get_return_code_mode(tPTISL tisl);

// GC
VM_RET tisl_signal_gc_start(tPTISL tisl, tPVM vm);
void tisl_reverse_gc_mark(tPTISL tisl);
void tisl_gc_wait_2(tPTISL tisl, tPVM vm);

tPCELL tisl_get_argument(tPTISL tisl);
tPCELL tisl_get_environment(tPTISL tisl);

void tisl_set_interface(tPTISL tisl, TISL* i);
TISL* tisl_get_interface(tPTISL tisl);

// ctrl-Cによる中断
void tisl_signal_user_interrupt(int sig);
int tisl_get_user_interrupt_flag(void);
void tisl_reset_user_interrupt_flag(void);
// ユーザによる中断の検査と
VM_RET vm_check_tisl_state(tPVM vm);

#define TISL_STATE_INITIALIZATION	0
#define TISL_STATE_OK				1
#define TISL_STATE_GC_WAIT			2

int tisl_get_state(tPTISL tisl);

#endif// #ifndef TISL_H
