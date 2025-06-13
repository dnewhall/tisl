//
// TISL/src/console_app/main.c
// TISL Ver 4.x
//
#include <stdio.h>
#include "../../include/tni.h"

///////////////////////////////////////

#if defined(TISL_A)
#define TISL_START_UP_1	"; TISL Ver. 4.08A (%s)\n"
#elif defined(TISL_B)
#define TISL_START_UP_1	"; TISL Ver. 4.08B (%s)\n"
#else
#define TISL_START_UP_1	"; TISL Ver. 4.08C (%s)\n"
#endif

///////////////////////////////////////

TISL_IMPORT_OR_EXPORT void TISLCALL tisl_interpreter_top_loop(TNI* tni);
TISL_IMPORT_OR_EXPORT void TISLCALL tisl_interpreter_file(TNI* tni, const char* file_name);

///////////////////////////////////////

int main(int argc, char *argv[], char *envp[])
{
	if (argc==1) {// TISLインタプリタの起動
		TISL*	tisl;
		TNI*	tni;
		TNI_INIT_ARGS init_args;

		printf(TISL_START_UP_1, __DATE__);
		// 処理系とインタフェースの初期化
		if (set_default_tni_init_args(&init_args)) return -1;
		// 引数と環境
		init_args.argc=argc;
		init_args.argv=argv;
		init_args.envp=envp;
		if (create_tni(&tisl, &tni, &init_args)) return -1;

		tisl_interpreter_top_loop(tni);

		return (*tisl)->destroy_tisl(tisl);
	} else {// スクリプト実行か？
		TISL*			tisl;
		TNI*			tni;
		TNI_INIT_ARGS	init_args;

		// 処理系のインタフェースの初期化
		if (set_default_tni_init_args(&init_args)) return -1;
		//
		init_args.argc=argc;
		init_args.argv=argv;
		init_args.envp=envp;
		//
		if (create_tni(&tisl, &tni, &init_args)) return -1;
		// 細かい引数の処理は省略
		tisl_interpreter_file(tni, argv[1]);

		return (*tisl)->destroy_tisl(tisl);
	}
}
