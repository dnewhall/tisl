//
// TISL/includ/tni.h
// TISL Ver 4.x
//

#ifndef TNI_H
#define TNI_H

// TISL type
#include "tisl_type.h"

#ifdef _WIN32
#define TISL_IMPORT		__declspec(dllimport)
#define TISL_EXPORT		__declspec(dllexport)
#define TISLCALL		__stdcall
#else
#define TISL_IMPORT
#define TISL_EXPORT
#define TISLCALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TISL_IMPLEMENTATION)
#define TISL_IMPORT_OR_EXPORT	TISL_EXPORT
#elif !defined(TISL_TEST)
#define TISL_IMPORT_OR_EXPORT	TISL_IMPORT
#else
#define TISL_IMPORT_OR_EXPORT
#endif

// interface type
typedef struct TNI_INIT_ARGS_	TNI_INIT_ARGS;
typedef const struct TISL_*		TISL;
typedef const struct TNI_*		TNI;

// return type
typedef tINT					TISL_RET;
#define TISL_OK		0
#define TISL_ERROR	-1

// TISLの生成

// create_tisl
TISL_IMPORT_OR_EXPORT TISL_RET
create_tni(TISL** tisl, TNI** tni, TNI_INIT_ARGS* args);

// TISLのデフォルト初期化引数の設定
// set_default_tni_init_args
TISL_IMPORT_OR_EXPORT TISL_RET
set_default_tni_init_args(TNI_INIT_ARGS* args);

// TISL処理系初期化用引数
struct TNI_INIT_ARGS_ {
	int		argc;
	char**	argv;
	char**	envp;

	int		init_stack_size;
	int		max_stack_size;
	int		user_interrupt;
};

#define TISL_VERSION_4_8	0x00040008
//
#define TNI_VERSION_1_0		0x00010000
#define TNI_VERSION_1_1		0x00010001
//
#define TISL_INIT_STACK_SIZE		(8*1024)
#define TISL_MAX_STACK_SIZE			(16*1024)

struct TISL_ {
	// destroy_tisl
	TISL_RET (TISLCALL *destroy_tisl)(TISL* tisl);
	// attach_tni
	TNI* (TISLCALL *attach_tni)(TISL* tisl);
	// detach_tni
	void (TISLCALL *detach_tni)(TISL* tisl, TNI* tni);
};

struct TNI_ {
	// get_version
	tINT (TISLCALL *get_version)(TNI* tni);
	// get_tisl
	TISL* (TISLCALL *get_tisl)(TNI* tni);
	// get_last_condition
	TISL_OBJECT (TISLCALL *get_last_condition)(TNI* tni);
	// clear_last_condition
	void (TISLCALL *clear_last_condition)(TNI* tni);
	// 大域参照の通知
	TISL_OBJECT (TISLCALL *new_global_ref)(TNI* tni, TISL_OBJECT obj);
	void (TISLCALL *delete_global_ref)(TNI* tni, TISL_OBJECT obj);
	// 局所参照の通知
	void (TISLCALL *delete_local_ref)(TNI* tni, TISL_OBJECT obj);
	// 名前から指定されるオブジェクトの取得
	// 変数名前空間
	TISL_OBJECT (TISLCALL *get_variable)(TNI* tni, tCSTRING name);
	void (TISLCALL *set_variable)(TNI* tni, tCSTRING name, TISL_OBJECT obj);
	// 関数名前空間
	TISL_OBJECT (TISLCALL *get_function)(TNI* tni, tCSTRING name);
	// 動的変数名前空間
	TISL_OBJECT (TISLCALL *get_dynamic)(TNI* tni, tCSTRING name);
	void (TISLCALL *set_dynamic)(TNI* tni, tCSTRING name, TISL_OBJECT obj);
	// クラス名前空間
	TISL_OBJECT (TISLCALL *get_class)(TNI* tni, tCSTRING name);
	// 組込みオブジェクトの生成とデータの取得
	// <integer>
	TISL_OBJECT (TISLCALL *create_integer)(TNI* tni, tINT i);
	tINT (TISLCALL *object_get_integer)(TNI* tni, TISL_OBJECT obj);
	// <float>
	TISL_OBJECT (TISLCALL *create_float)(TNI* tni, tFLOAT f);
	tFLOAT (TISLCALL *object_get_float)(TNI* tni, TISL_OBJECT obj);
	// <character>
	TISL_OBJECT (TISLCALL *create_character)(TNI* tni, tINT c);
	tINT (TISLCALL *object_get_character)(TNI* tni, TISL_OBJECT obj);
	// <string>
	TISL_OBJECT (TISLCALL *create_string)(TNI* tni, tCSTRING s);
	tCSTRING (TISLCALL *object_get_string)(TNI* tni, TISL_OBJECT obj);
	// <symbol>
	TISL_OBJECT (TISLCALL *create_symbol)(TNI* tni, tCSTRING s);
	tCSTRING (TISLCALL *object_get_symbol)(TNI* tni, TISL_OBJECT obj);
	// <foreign-object>
	TISL_OBJECT (TISLCALL *create_foreign_object)(TNI* tni, void* fobj, void (*release)(TNI* tni, void* fobj));
	void* (TISLCALL *object_get_foreign_object)(TNI* tni, TISL_OBJECT obj);
	// 関数呼出し
	TISL_OBJECT (TISLCALL *function_call)(TNI* tni, TISL_OBJECT function, ...);
	TISL_OBJECT (TISLCALL *function_call_l)(TNI* tni, TISL_OBJECT function, TISL_OBJECT arg_list);
	// 非局所脱出
	void (TISLCALL* tisl_throw)(TNI* tni, TISL_OBJECT tag, TISL_OBJECT obj);
	// 型検査
	TISL_OBJECT (TISLCALL* assure)(TNI* tni, tCSTRING clss, TISL_OBJECT obj);
	// 型変換
	TISL_OBJECT (TISLCALL* convert)(TNI* tni, TISL_OBJECT obj, tCSTRING clss);
	// ファイルのロード
	void (TISLCALL* load)(TNI* tni, tCSTRING file_name);
	// evaluate_top_form
	TISL_OBJECT (TISLCALL *evaluate_top_form)(TNI* tni, TISL_OBJECT form);
	// パッケージ
	void (TISLCALL *in_package)(TNI* tni, tCSTRING name);
	//
	TISL_OBJECT (TISLCALL *evaluate)(TNI* tni, const void* buffer, const tUINT buffer_size);
	//
	TISL_OBJECT (TISLCALL *create_foreign_object_ex)(TNI* tni, void* fobj, void (*release)(TNI*, void*), tCSTRING clss);
};

#ifdef __cplusplus
}// extern "C" {
#endif
#endif // #ifndef TNI_H
