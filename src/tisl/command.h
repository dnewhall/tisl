//
// TISL/src/tisl/command.h
// TISL Ver. 4.x
//

#ifndef TISL_COMMAND_H
#define TISL_COMMAND_H

// 命令セット１(pass1が生成する命令リストで使用する)
enum {
	// スタック
	iDISCARD,				// iDISCARD							スタックの値を一つ捨てる
	// 
	iPUSH_NIL,				// iPUSH_NIL						スタックにnilオブジェクトを積む
	iPUSH_T,				// 
	iPUSH_OBJECT,			// iPUSH_OBJECT obj					スタックにobjオブジェクトを積む
	// 識別子の評価
	iPUSH_LOCAL_VARIABLE,	// iPUSH_LOCAL_VARIABLE symbol		symbolに対応する値をスタックに積む．symbolは仮引数の中にあった
	iPUSH_GLOBAL_VARIABLE,	// iPUSH_GLOBAL_VARIABLE symbol		symbolに対応する値をスタックに積む．symbolは最上位の束縛を参照
	// 複合形式
	// 関数適用形式
	iCALL_REC,				// iCALL_REC						自分自身を呼出す
	iCALL_TAIL_REC,			// iCALL_TAIL_REC					自分自身を呼出す(末尾)
	iCALL_GLOBAL,			// iCALL_GLOBAL bind-list anum		bind-listの関数をanum個の引数で呼出す
	iCALL_TAIL_GLOBAL,		// iCALL_TAIL_GLOBAL bind-list anum	bind-listの関数をanum個の引数で呼出す(末尾)
	iCALL_BIND,				// iCALL_BIND bind anum
	iCALL_TAIL_BIND,		// iCALL_TAIL_BIND bind anum
	iCALL_LOCAL,			// iCALL_LOCAL symbol flist
	iCALL_TAIL_LOCAL,		// iCALL_TAIL_LOCAL symbol flist
	iRET,					// iRET								呼出しもとに返る
	// 特殊形式
	iLAMBDA_IN,				// iLAMBDA_IN plist
	iLAMBDA_OUT,			// iLAMBDA_OUT plist
	//
	iPUSH_FUNCTION,			// iPUSH_FUNCTION name
	iPUSH_LOCAL_FUNCTION,	// iPUSH_LOCAL_FUNCTION name flist
	iPUSH_LAMBDA,			// iPUSH_LAMBDA function-list
	iLABELS_IN,				// iLABELS_IN nlist function-list ... function-list
	iLABELS_OUT,			// iLABELS_OUT
	iFLET_IN,				// iFLET_IN nlist function-list ... function-list
	iFLET_OUT,				// iFLET_OUT
	iAND,					// iAND clist
	iOR,					// iOR clist
	iSET_LOCAL_VARIABLE,	// iSET_LOCAL_VARIABLE symbol
	iSET_GLOBAL_VARIABLE,	// iSET_GLOBAL_VARIABLE symbol
	iSET_DYNAMIC,			// iSET_DYNAMIC symbol
	iSET_AREF,				// iSET_AREF n
	iSET_GAREF,				// iSET_GAREF n
	iSET_ELT,				// iSET_ELT
	iSET_PROPERTY,			// iSET_PROPERTY
	iSET_CAR,				// iSET_CAR
	iSET_CDR,				// iSET_CDR
	iACCESSOR,				// iACCESSOR name
	iPUSH_DYNAMIC,			// iPUSH_DYNAMIC name
	iDYNAMIC_LET,			// iDYNAMIC_LET n name1 ... namen code-list
	iIF,					// iIF code-list code-list
	iCASE,					// iCASE n m_1 key ... key clist ... m_n key ... key clist
	iCASE_USING,			// iCASE_USING n m_1 key ... key clist ... m_n key ... key clist
	iWHILE,					// iWHILE code-list code-list
	iFOR,					// iFOR plist endtest-clist result-clist iterate-clist
	iBLOCK,					// iBLOCK code-list block-tag
	iRETURN_FROM,			// iRETURN_FROM block-tag
	iCATCH,					// iCATCH code-list
	iTHROW,					// iTHROW
	iTAGBODY,				// iTAGBODY tag-list code-list ... code-list
	iGO,					// iGO (tag . tag-list)
	iUNWIND_PROTECT,		// iUNWIND_PROTECT code-list code-list
	iPUSH_CLASS,			// iPUSH_CLASS name
	iTHE,					// iTHE name
	iASSURE,				// iASSURE name
	iCONVERT,				// iCONVERT name
	iWITH_STANDARD_INPUT,	// iWITH_STANDARD_INPUT code-list
	iWITH_STANDARD_OUTPUT,	// iWITH_STANDARD_OUTPUT code-list
	iWITH_ERROR_OUTPUT,		// iWITH_ERROR_OUTPUT code-list
	iWITH_OPEN_INPUT_FILE,	// iWITH_OPEN_INPUT_FILE plist clist
	iWITH_OPEN_OUTPUT_FILE,	// iWITH_OPEN_OUTPUT_FILE plist clist
	iWITH_OPEN_IO_FILE,		// iWITH_OPEN_IO_FILE plist clist
	iIGNORE_ERRORS,			// iIGNORE_ERRORS clist
	iCONTINUE_CONDITION,	// iCONTINUE_CONDITION
	iWITH_HANDLER,			// iWITH_HANDLER clist
	iTIME,					// iTIME clist

	// 構文
	iQUASIQUOTE,			// iQUASIQUOTE						スタック上の値一つを使用してquasiquoteを作成する
	iQUASIQUOTE2,			// iQUASIQUOTE2						スタック上の値二つを使用してquasiquoteを作成する
	iUNQUOTE,				// iUNQUOTE
	iUNQUOTE_SPLICING,		// iUNQUOTE_SPLICING
	iUNQUOTE_SPLICING2,		// iUNQUOTE_SPLICING2

	//
	iFUNCTIONP,
	iAPPLY,
	iFUNCALL,
	iEQ,
	iEQL,
	iEQUAL,
	iNOT,
	iGENERIC_FUNCTION_P,
	iCLASS_OF,
	iINSTANCEP,
	iSUBCLASSP,
	iSYMBOLP,
	iPROPERTY,
	iREMOVE_PROPERTY,
	iGENSYM,
	iNUMBERP,
	iPARSE_NUMBER,
	iNUMBER_EQUAL,
	iNUMBER_NOT_EQUAL,
	iNUMBER_GE,
	iNUMBER_LE,
	iNUMBER_GREATER,
	iNUMBER_LESS,
	iADDITION,
	iMULTIPLICATION,
	iSUBSTRACTION,
	iQUOTIENT,
	iRECIPROCAL,
	iMAX,
	iMIN,
	iABS,
	iEXP,
	iLOG,
	iEXPT,
	iSQRT,
	iSIN,
	iCOS,
	iTAN,
	iATAN,
	iATAN2,
	iSINH,
	iCOSH,
	iTANH,
	iATANH,
	iFLOATP,
	iFLOAT,
	iFLOOR,
	iCEILING,
	iTRUNCATE,
	iROUND,
	iINTEGERP,
	iDIV,
	iMOD,
	iGCD,
	iLCM,
	iISQRT,
	iCHARACTERP,
	iCHAR_EQUAL,
	iCHAR_NOT_EQUAL,
	iCHAR_LESS,
	iCHAR_GREATER,
	iCHAR_LE,
	iCHAR_GE,
	iCONSP,
	iCONS,
	iCAR,
	iCDR,
	iNULL,
	iLISTP,
	iCREATE_LIST,
	iLIST,
	iREVERSE,
	iNREVERSE,
	iAPPEND,
	iMEMBER,
	iMAPCAR,
	iMAPC,
	iMAPCAN,
	iMAPLIST,
	iMAPL,
	iMAPCON,
	iASSOC,
	iBASIC_ARRAY_P,
	iBASIC_ARRAY_A_P,
	iGENERAL_ARRAY_A_P,
	iCREATE_ARRAY,
	iAREF,
	iGAREF,
	iARRAY_DIMENSIONS,
	iBASIC_VECTOR_P,
	iGENERAL_VECTOR_P,
	iCREATE_VECTOR,
	iVECTOR,
	iSTRINGP,
	iCREATE_STRING,
	iSTRING_EQUAL,
	iSTRING_NOT_EQUAL,
	iSTRING_LESS,
	iSTRING_GREATER,
	iSTRING_GE,
	iSTRING_LE,
	iCHAR_INDEX,
	iSTRING_INDEX,
	iSTRING_APPEND,
	iLENGTH,
	iELT,
	iSUBSEQ,
	iMAP_INTO,
	iSTREAMP,
	iOPEN_STREAM_P,
	iINPUT_STREAM_P,
	iOUTPUT_STREAM_P,
	iSTANDARD_INPUT,
	iSTANDARD_OUTPUT,
	iERROR_OUTPUT,
	iOPEN_INPUT_FILE,
	iOPEN_OUTPUT_FILE,
	iOPEN_IO_FILE,
	iCLOSE,
	iFINISH_OUTPUT,
	iCREATE_STRING_INPUT_STREAM,
	iCREATE_STRING_OUTPUT_STREAM,
	iGET_OUTPUT_STREAM_STRING,
	iREAD,
	iREAD_CHAR,
	iPREVIEW_CHAR,
	iREAD_LINE,
	iSTREAM_READY_P,
	iFORMAT,
	iFORMAT_CHAR,
	iFORMAT_FLOAT,
	iFORMAT_FRESH_LINE,
	iFORMAT_INTEGER,
	iFORMAT_OBJECT,
	iFORMAT_TAB,
	iREAD_BYTE,
	iWRITE_BYTE,
	iPROBE_FILE,
	iFILE_POSITION,
	iSET_FILE_POSITION,
	iFILE_LENGTH,
	iERROR,
	iCERROR,
	iSIGNAL_CONDITION,
	iCONDITION_CONTINUABLE,
	iARITHMETIC_ERROR_OPERATION,
	iARITHMETIC_ERROR_OPERAND,
	iDOMAIN_ERROR_OBJECT,
	iDOMAIN_ERROR_EXPECTED_CLASS,
	iPARSE_ERROR_STRING,
	iPARSE_ERROR_EXPECTED_CLASS,
	iSIMPLE_ERROR_FORMAT_STRING,
	iSIMPLE_ERROR_FORMAT_ARGUMENTS,
	iSTREAM_ERROR_STREAM,
	iUNDEFINED_ENTITY_NAME,
	iUNDEFINED_ENTITY_NAMESPACE,
	iIDENTITY,
	iGET_UNIVERSAL_TIME,
	iGET_INTERNAL_RUN_TIME,
	iGET_INTERNAL_REAL_TIME,
	iGET_INTERNAL_TIME_UNITS_PER_SECOND,
	iSYSTEM,
	iEXIT,
	iSTRFTIME,
	iGET_ARGUMENT,
	iGET_ENVIRONMENT,
	iARITY_ERROR,
	iEVAL,
	NUMBER_OF_COMMAND_1,
};

// 命令セット2(pass2が生成するコードで使用する)
enum {
	iiDISCARD,					// iiDISCARD
	iiPUSH_NIL,					// iiPUSH_NIL
	iiPUSH_T,					// iiPUSH_T
	iiPUSH_OBJECT,				// iiPUSH_OBJECT	obj
	iiPUSH_STACK,				// iiPUSH_STACK		offset
	iiPUSH_HEAP,				// iiPUSH_HEAP		offset
	iiPUSH_VARIABLE,			// iiPUSH_VARIABLE	bind-list
	iiCALL_REC,					// iiCALL_REC
	iiCALL_TAIL_REC,			// iiCALL_TAIL_REC
	iiCALL,						// iiCALL bind-list anum
	iiCALL_TAIL,				// iiCALL_TAIL bind-list anum
	iiCALL_BIND,				// iiCALL_BIND bind anum
	iiCALL_TAIL_BIND,			// iiCALL_TAIL_BIND bind anum
	iiCALL_LOCAL,				// iCALL_LOCAL offset flist
	iiCALL_LOCAL_TAIL,			// iCALL_LOCAL_TAIL offset flist
	//
	iiRET,						// iiRET
	iiLAMBDA_IN,				// iiLAMBDA_IN plist
	iiLAMBDA_OUT,				// iiLAMBDA_OUT plist
	iiLAMBDA_HEAP_IN,			// iiLAMBDA_HEAP_IN plist
	iiLAMBDA_HEAP_OUT,			// iiLAMBDA_HEAP_OUT plist
	iiPUSH_FUNCTION,			// iiPUSH_FUNCTION bind-list
	iiPUSH_LOCAL_FUNCTION,		// iiPUSH_FUNCTION offset flist
	iiPUSH_LAMBDA,				// iiPUSH_LAMBDA flist
	iiLABELS_IN,				// iiLABELS_IN nlist function-list-1 ... function-list-n
	iiLABELS_OUT,				// iiLABELS_OUT 
	iiFLET_IN,					// iiFLET_IN nlist function-list-1 ... function-list-n
	iiFLET_OUT,					// iiFLET_OUT
	iiAND,						// iiAND clist
	iiOR,						// iiOR clist
	iiSET_STACK,				// iiSET_STACK offset
	iiSET_HEAP,					// iiSET_HEAP offset
	iiSET_VARIABLE,				// iiSET_VARIABLE bind-list
	iiSET_DYNAMIC,				// iiSET_DYNAMIC symbol
	iiSET_AREF,					// iiSET_AREF n
	iiSET_GAREF,				// iiSET_GAREF n
	iiSET_ELT,					// iiSET_ELT
	iiSET_PROPERTY,				// iiSET_PROPERTY
	iiSET_CAR,					// iiSET_CAR
	iiSET_CDR,					// iiSET_CDR
	iiACCESSOR,					// iiSET_ACCESSOR name
	iiPUSH_DYNAMIC,				// iiPUSH_DYNAMIC name
	iiDYNAMIC_LET,				// iiDYNAMIC_LET n name-1 ... name-n code-list
	iiIF,						// iiIF code-list code-list
	iiCASE,						// iiCASE n key-list1 clist1 ... key-listn clistn
	iiCASE_USING,				// iiCASE_USING n key-list1 clist1 ... key-listn clistn
	iiWHILE,					// iiWHILE test-clist body-clist
	iiFOR_STACK,				// iiFOR_STACK plist endtest-clist result-clist iterate-clist
	iiFOR_HEAP,					// iiFOR_HEAP plist endtest-clist result-clist iterate-clist
	iiBLOCK,					// iiBLOCK clist tag
	iiRETURN_FROM,				// iiRETURN_FROM tag
	iiCATCH,					// iiCATCH clist
	iiTHROW,					// iiTHROW
	iiTAGBODY,					// iiTAGBODY tag-list code-list ... code-list
	iiGO,						// iiGO (tag . tag-list)
	iiUNWIND_PROTECT,			// iiUNWIND_PROTECT clist clist
	iiPUSH_CLASS,				// iiPUSH_CLASS bind-list
	iiTHE,						// iiTHE bind-list
	iiASSURE,					// iiASSURE bind-list
	iiCONVERT,					// iiCONVERT bind-list
	iiWITH_STANDARD_INPUT,		// iiWITH_STANDARD_INPUT code-list
	iiWITH_STANDARD_OUTPUT,		// iiWITH_STANDARD_OUTPUT code-list
	iiWITH_ERROR_OUTPUT,		// iiWITH_ERROR_OUTPUT code-list
	iiWITH_OPEN_INPUT_FILE,		// iiWITH_OPEN_INPUT_FILE plist clist
	iiWITH_OPEN_OUTPUT_FILE,	// iiWITH_OPEN_OUTPUT_FILE plist clist
	iiWITH_OPEN_IO_FILE,		// iiWITH_OPEN_IO_FILE plist clist
	iiIGNORE_ERRORS,			// iiIGNORE_ERRORS clist
	iiCONTINUE_CONDITION,		// iiCONTINUE_CONDITON
	iiWITH_HANDLER,				// iiWITH_HANDLER clist
	iiTIME,						// iiTIME clist
	iiQUASIQUOTE,				// iiQUASIQUOTE
	iiQUASIQUOTE2,				// iiQUASIQUOTE2
	iiUNQUOTE,					// iiUNQUOTE
	iiUNQUOTE_SPLICING,			// iiUNQUOTE_SPLICING
	iiUNQUOTE_SPLICING2,		// iiUNQUOTE_SPLICING2
	iiFUNCTIONP,
	iiAPPLY,
	iiFUNCALL,
	iiEQ,
	iiEQL,
	iiEQUAL,
	iiNOT,
	iiGENERIC_FUNCTION_P,
	iiCLASS_OF,
	iiINSTANCEP,
	iiSUBCLASSP,
	iiSYMBOLP,
	iiPROPERTY,
	iiREMOVE_PROPERTY,
	iiGENSYM,
	iiNUMBERP,
	iiPARSE_NUMBER,
	iiNUMBER_EQUAL,
	iiNUMBER_NOT_EQUAL,
	iiNUMBER_GE,
	iiNUMBER_LE,
	iiNUMBER_GREATER,
	iiNUMBER_LESS,
	iiADDITION,
	iiMULTIPLICATION,
	iiSUBSTRACTION,
	iiQUOTIENT,
	iiRECIPROCAL,
	iiMAX,
	iiMIN,
	iiABS,
	iiEXP,
	iiLOG,
	iiEXPT,
	iiSQRT,
	iiSIN,
	iiCOS,
	iiTAN,
	iiATAN,
	iiATAN2,
	iiSINH,
	iiCOSH,
	iiTANH,
	iiATANH,
	iiFLOATP,
	iiFLOAT,
	iiFLOOR,
	iiCEILING,
	iiTRUNCATE,
	iiROUND,
	iiINTEGERP,
	iiDIV,
	iiMOD,
	iiGCD,
	iiLCM,
	iiISQRT,
	iiCHARACTERP,
	iiCHAR_EQUAL,
	iiCHAR_NOT_EQUAL,
	iiCHAR_LESS,
	iiCHAR_GREATER,
	iiCHAR_LE,
	iiCHAR_GE,
	iiCONSP,
	iiCONS,
	iiCAR,
	iiCDR,
	iiNULL,
	iiLISTP,
	iiCREATE_LIST,
	iiLIST,
	iiREVERSE,
	iiNREVERSE,
	iiAPPEND,
	iiMEMBER,
	iiMAPCAR,
	iiMAPC,
	iiMAPCAN,
	iiMAPLIST,
	iiMAPL,
	iiMAPCON,
	iiASSOC,
	iiBASIC_ARRAY_P,
	iiBASIC_ARRAY_A_P,
	iiGENERAL_ARRAY_A_P,
	iiCREATE_ARRAY,
	iiAREF,
	iiGAREF,
	iiARRAY_DIMENSIONS,
	iiBASIC_VECTOR_P,
	iiGENERAL_VECTOR_P,
	iiCREATE_VECTOR,
	iiVECTOR,
	iiSTRINGP,
	iiCREATE_STRING,
	iiSTRING_EQUAL,
	iiSTRING_NOT_EQUAL,
	iiSTRING_LESS,
	iiSTRING_GREATER,
	iiSTRING_GE,
	iiSTRING_LE,
	iiCHAR_INDEX,
	iiSTRING_INDEX,
	iiSTRING_APPEND,
	iiLENGTH,
	iiELT,
	iiSUBSEQ,
	iiMAP_INTO,
	iiSTREAMP,
	iiOPEN_STREAM_P,
	iiINPUT_STREAM_P,
	iiOUTPUT_STREAM_P,
	iiSTANDARD_INPUT,
	iiSTANDARD_OUTPUT,
	iiERROR_OUTPUT,
	iiOPEN_INPUT_FILE,
	iiOPEN_OUTPUT_FILE,
	iiOPEN_IO_FILE,
	iiCLOSE,
	iiFINISH_OUTPUT,
	iiCREATE_STRING_INPUT_STREAM,
	iiCREATE_STRING_OUTPUT_STREAM,
	iiGET_OUTPUT_STREAM_STRING,
	iiREAD,
	iiREAD_CHAR,
	iiPREVIEW_CHAR,
	iiREAD_LINE,
	iiSTREAM_READY_P,
	iiFORMAT,
	iiFORMAT_CHAR,
	iiFORMAT_FLOAT,
	iiFORMAT_FRESH_LINE,
	iiFORMAT_INTEGER,
	iiFORMAT_OBJECT,
	iiFORMAT_TAB,
	iiREAD_BYTE,
	iiWRITE_BYTE,
	iiPROBE_FILE,
	iiFILE_POSITION,
	iiSET_FILE_POSITION,
	iiFILE_LENGTH,
	iiERROR,
	iiCERROR,
	iiSIGNAL_CONDITION,
	iiCONDITION_CONTINUABLE,
	iiARITHMETIC_ERROR_OPERATION,
	iiARITHMETIC_ERROR_OPERAND,
	iiDOMAIN_ERROR_OBJECT,
	iiDOMAIN_ERROR_EXPECTED_CLASS,
	iiPARSE_ERROR_STRING,
	iiPARSE_ERROR_EXPECTED_CLASS,
	iiSIMPLE_ERROR_FORMAT_STRING,
	iiSIMPLE_ERROR_FORMAT_ARGUMENTS,
	iiSTREAM_ERROR_STREAM,
	iiUNDEFINED_ENTITY_NAME,
	iiUNDEFINED_ENTITY_NAMESPACE,
	iiIDENTITY,
	iiGET_UNIVERSAL_TIME,
	iiGET_INTERNAL_RUN_TIME,
	iiGET_INTERNAL_REAL_TIME,
	iiGET_INTERNAL_TIME_UNITS_PER_SECOND,
	iiSYSTEM,
	iiEXIT,
	iiSTRFTIME,
	iiGET_ARGUMENT,
	iiGET_ENVIRONMENT,
	iiARITY_ERROR,
	iiEVAL,

	iiNUMBER_EQUAL_STACK_INTEGER,
	iiNUMBER_EQUAL_STACK_STACK,
	iiNUMBER_NOT_EQUAL_STACK_INTEGER,
	iiNUMBER_NOT_EQUAL_STACK_STACK,
	iiNUMBER_LESS_STACK_INTEGER,
	iiNUMBER_LESS_INTEGER_STACK,
	iiNUMBER_LESS_STACK_STACK,
	iiNUMBER_LE_STACK_INTEGER,
	iiNUMBER_LE_INTEGER_STACK,
	iiNUMBER_LE_STACK_STACK,

	iiADDITION_STACK_INTEGER,
	iiADDITION_STACK_STACK,
	iiSUBSTRACTION_STACK_INTEGER,
	iiSUBSTRACTION_INTEGER_STACK,
	iiSUBSTRACTION_STACK_STACK,

	iiEQ_STACK_INTEGER,
	iiEQ_STACK_STACK,
	iiEQL_STACK_INTEGER,
	iiEQL_STACK_STACK,
	iiEQUAL_STACK_INTEGER,
	iiEQUAL_STACK_STACK,
};

#ifdef TISL_TRANSALTOR_2_C

static VM_RET t2_op_n(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_discard(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_nil(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_object(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_local_variable(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_global_variable(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_rec(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_tail_rec(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_global(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_tail_global(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_bind(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_tail_bind(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_local(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_call_tail_local(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_ret(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_lambda_in(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_lambda_out(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_function(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_local_function(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_lambda(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_labels_in(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_labels_out(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_flet_in(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_and(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_or(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_set_local_variable(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_set_global_variable(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_set_dynamic(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_set_elt(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_set_car(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_accessor(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_dynamic(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_dynamic_let(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_if(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_case(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_while(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_for(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_block(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_return_from(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_catch(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_throw(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_tagbody(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_go(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_unwind_protect(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_push_class(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_the(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_with_standard_stream(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_with_open_file(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_ignore_errors(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_continue_condition(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_with_handler(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_op_1(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_op_2(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_primitive(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_primitive_0(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_primitive_1(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_primitive_2(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_primitive_3(tPVM vm, const tINT id, tPCELL* head);

static VM_RET t2_number_equal(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_number_not_equal(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_number_less(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_number_greater(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_number_le(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_number_ge(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_addition(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_substraction(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_eq(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_eql(tPVM vm, const tINT id, tPCELL* head);
static VM_RET t2_equal(tPVM vm, const tINT id, tPCELL* head);

typedef VM_RET (*T2_TRANSLATE)(tPVM, const tINT, tPCELL*);
typedef struct T2_TABLE_	T2_TABLE;
struct T2_TABLE_ {
	T2_TRANSLATE	translate;
	tINT			t2_data;
};

const T2_TABLE t2_table[]={
	{ t2_discard, 0 },
	{ t2_push_nil, 0 },
	{ t2_push_nil, 0 },// iPUSH_T
	{ t2_push_object, 0 },
	{ t2_push_local_variable, 0 },
	{ t2_push_global_variable, 0 },
	{ t2_call_rec, 0 },
	{ t2_call_tail_rec, 0 },
	{ t2_call_global, 0 },
	{ t2_call_tail_global, 0 },
	{ t2_call_bind, 0 },
	{ t2_call_tail_bind, 0 },
	{ t2_call_local, 0 },
	{ t2_call_tail_local, 0 },
	{ t2_ret, 0 },
	{ t2_lambda_in, 0 },
	{ t2_lambda_out, 0 },
	{ t2_push_function, 0 },
	{ t2_push_local_function, 0 },
	{ t2_push_lambda, 0 },
	{ t2_labels_in, 0 },
	{ t2_labels_out, 0 },
	{ t2_flet_in, 0 },
	{ t2_labels_out, 0 },
	{ t2_and, 0 },
	{ t2_or, 0 },
	{ t2_set_local_variable, 0 },
	{ t2_set_global_variable, 0 },
	{ t2_set_dynamic, 0 },
	{ t2_op_n, 0 },
	{ t2_op_n, 0 },
	{ t2_set_elt, 0 },
	{ t2_set_elt, 0 },
	{ t2_set_car, 0 },
	{ t2_set_car, 0 },
	{ t2_accessor, 0 },
	{ t2_push_dynamic, 0 },
	{ t2_dynamic_let, 0 },
	{ t2_if, 0 },
	{ t2_case, 0 },
	{ t2_case, 0 },
	{ t2_while, 0 },
	{ t2_for, 0 },
	{ t2_block, 0 },
	{ t2_return_from, 0 },
	{ t2_catch, 0 },
	{ t2_throw, 0 },
	{ t2_tagbody, 0 },
	{ t2_go, 0 },
	{ t2_unwind_protect, 0 },
	{ t2_push_class, 0 },
	{ t2_the, 0 },
	{ t2_the, 0 },
	{ t2_the, 0 },
	{ t2_with_standard_stream, 0 },
	{ t2_with_standard_stream, 0 },
	{ t2_with_standard_stream, 0 },
	{ t2_with_open_file, 0 },
	{ t2_with_open_file, 0 },
	{ t2_with_open_file, 0 },
	{ t2_ignore_errors, 0 },
	{ t2_continue_condition, 0 },
	{ t2_with_handler, 0 },
	{ t2_ignore_errors, 0 },// iTIME
	{ t2_op_1, 0 },	// iQUASIQUOTE
	{ t2_op_2, 0 },	// iQUASIQUOTE2
	{ t2_op_1, 0 },	// iUNQUOTE
	{ t2_op_1, 0 },	// iUNQUOTE_SPLICING
	{ t2_op_1, 0 },	// iUNQUOTE_SPLICING2
	{ t2_primitive_1, iiFUNCTIONP }, // iFUNCTIONP
	{ t2_primitive, iiAPPLY }, // iAPPLY
	{ t2_primitive, iiFUNCALL }, // iFUNCALL
	{ t2_eq, iiEQ },
	{ t2_eql, iiEQL },
	{ t2_equal, iiEQUAL },
	{ t2_primitive_1, iiNOT },
	{ t2_primitive_1, iiGENERIC_FUNCTION_P },
	{ t2_primitive_1, iiCLASS_OF },
	{ t2_primitive_2, iiINSTANCEP },
	{ t2_primitive_2, iiSUBCLASSP },
	{ t2_primitive_1, iiSYMBOLP },
	{ t2_primitive, iiPROPERTY },
	{ t2_primitive, iiREMOVE_PROPERTY },
	{ t2_primitive_0, iiGENSYM },
	{ t2_primitive_1, iiNUMBERP },
	{ t2_primitive_1, iiPARSE_NUMBER },
	{ t2_number_equal, iiNUMBER_EQUAL },
	{ t2_number_not_equal, iiNUMBER_NOT_EQUAL },
	{ t2_number_ge, iiNUMBER_GE },
	{ t2_number_le, iiNUMBER_LE },
	{ t2_number_greater, iiNUMBER_GREATER },
	{ t2_number_less, iiNUMBER_LESS },
	{ t2_addition, iiADDITION },
	{ t2_primitive, iiMULTIPLICATION },
	{ t2_substraction, iiSUBSTRACTION },
	{ t2_primitive, iiQUOTIENT },
	{ t2_primitive_1, iiRECIPROCAL },
	{ t2_primitive, iiMAX },
	{ t2_primitive, iiMIN },
	{ t2_primitive_1, iiABS },
	{ t2_primitive_1, iiEXP },
	{ t2_primitive_1, iiLOG },
	{ t2_primitive_2, iiEXPT },
	{ t2_primitive_1, iiSQRT },
	{ t2_primitive_1, iiSIN },
	{ t2_primitive_1, iiCOS },
	{ t2_primitive_1, iiTAN },
	{ t2_primitive_1, iiATAN },
	{ t2_primitive_2, iiATAN2 },
	{ t2_primitive_1, iiSINH },
	{ t2_primitive_1, iiCOSH },
	{ t2_primitive_1, iiTANH },
	{ t2_primitive_1, iiATANH },
	{ t2_primitive_1, iiFLOATP },
	{ t2_primitive_1, iiFLOAT },
	{ t2_primitive_1, iiFLOOR },
	{ t2_primitive_1, iiCEILING },
	{ t2_primitive_1, iiTRUNCATE },
	{ t2_primitive_1, iiROUND },
	{ t2_primitive_1, iiINTEGERP },
	{ t2_primitive_2, iiDIV },
	{ t2_primitive_2, iiMOD },
	{ t2_primitive_2, iiGCD },
	{ t2_primitive_2, iiLCM },
	{ t2_primitive_1, iiISQRT },
	{ t2_primitive_1, iiCHARACTERP },
	{ t2_primitive_2, iiCHAR_EQUAL },
	{ t2_primitive_2, iiCHAR_NOT_EQUAL },
	{ t2_primitive_2, iiCHAR_LESS },
	{ t2_primitive_2, iiCHAR_GREATER },
	{ t2_primitive_2, iiCHAR_LE },
	{ t2_primitive_2, iiCHAR_GE },
	{ t2_primitive_1, iiCONSP },
	{ t2_primitive_2, iiCONS },
	{ t2_primitive_1, iiCAR },
	{ t2_primitive_1, iiCDR },
	{ t2_primitive_1, iiNULL },
	{ t2_primitive_1, iiLISTP },
	{ t2_primitive, iiCREATE_LIST },
	{ t2_primitive, iiLIST },
	{ t2_primitive_1, iiREVERSE },
	{ t2_primitive_1, iiNREVERSE },
	{ t2_primitive, iiAPPEND },
	{ t2_primitive_2, iiMEMBER },
	{ t2_primitive, iiMAPCAR },
	{ t2_primitive, iiMAPC },
	{ t2_primitive, iiMAPCAN },
	{ t2_primitive, iiMAPLIST },
	{ t2_primitive, iiMAPL },
	{ t2_primitive, iiMAPCON },
	{ t2_primitive_2, iiASSOC },
	{ t2_primitive_1, iiBASIC_ARRAY_P },
	{ t2_primitive_1, iiBASIC_ARRAY_A_P },
	{ t2_primitive_1, iiGENERAL_ARRAY_A_P },
	{ t2_primitive, iiCREATE_ARRAY },
	{ t2_primitive, iiAREF },
	{ t2_primitive, iiGAREF },
	{ t2_primitive_1, iiARRAY_DIMENSIONS },
	{ t2_primitive_1, iiBASIC_VECTOR_P },
	{ t2_primitive_1, iiGENERAL_VECTOR_P },
	{ t2_primitive, iiCREATE_VECTOR },
	{ t2_primitive, iiVECTOR },
	{ t2_primitive_1, iiSTRINGP },
	{ t2_primitive, iiCREATE_STRING },
	{ t2_primitive_2, iiSTRING_EQUAL },
	{ t2_primitive_2, iiSTRING_NOT_EQUAL },
	{ t2_primitive_2, iiSTRING_LESS },
	{ t2_primitive_2, iiSTRING_GREATER },
	{ t2_primitive_2, iiSTRING_GE },
	{ t2_primitive_2, iiSTRING_LE },
	{ t2_primitive, iiCHAR_INDEX },
	{ t2_primitive, iiSTRING_INDEX },
	{ t2_primitive, iiSTRING_APPEND },
	{ t2_primitive_1, iiLENGTH },
	{ t2_primitive_2, iiELT },
	{ t2_primitive_3, iiSUBSEQ },
	{ t2_primitive, iiMAP_INTO },
	{ t2_primitive_1, iiSTREAMP },
	{ t2_primitive_1, iiOPEN_STREAM_P },
	{ t2_primitive_1, iiINPUT_STREAM_P },
	{ t2_primitive_1, iiOUTPUT_STREAM_P },
	{ t2_primitive_0, iiSTANDARD_INPUT },
	{ t2_primitive_0, iiSTANDARD_OUTPUT },
	{ t2_primitive_0, iiERROR_OUTPUT },
	{ t2_primitive, iiOPEN_INPUT_FILE },
	{ t2_primitive, iiOPEN_OUTPUT_FILE },
	{ t2_primitive, iiOPEN_IO_FILE },
	{ t2_primitive_1, iiCLOSE },
	{ t2_primitive_1, iiFINISH_OUTPUT },
	{ t2_primitive_1, iiCREATE_STRING_INPUT_STREAM },
	{ t2_primitive_0, iiCREATE_STRING_OUTPUT_STREAM },
	{ t2_primitive_1, iiGET_OUTPUT_STREAM_STRING },
	{ t2_primitive, iiREAD },
	{ t2_primitive, iiREAD_CHAR },
	{ t2_primitive, iiPREVIEW_CHAR },
	{ t2_primitive, iiREAD_LINE },
	{ t2_primitive_1, iiSTREAM_READY_P },
	{ t2_primitive, iiFORMAT },
	{ t2_primitive_2, iiFORMAT_CHAR },
	{ t2_primitive_2, iiFORMAT_FLOAT },
	{ t2_primitive_1, iiFORMAT_FRESH_LINE },
	{ t2_primitive_3, iiFORMAT_INTEGER },
	{ t2_primitive_3, iiFORMAT_OBJECT },
	{ t2_primitive_2, iiFORMAT_TAB },
	{ t2_primitive, iiREAD_BYTE },
	{ t2_primitive_2, iiWRITE_BYTE },
	{ t2_primitive_1, iiPROBE_FILE },
	{ t2_primitive_1, iiFILE_POSITION },
	{ t2_primitive_2, iiSET_FILE_POSITION },
	{ t2_primitive_2, iiFILE_LENGTH },
	{ t2_primitive, iiERROR },
	{ t2_primitive, iiCERROR },
	{ t2_primitive, iiSIGNAL_CONDITION },
	{ t2_primitive, iiCONDITION_CONTINUABLE },
	{ t2_primitive, iiARITHMETIC_ERROR_OPERATION },
	{ t2_primitive, iiARITHMETIC_ERROR_OPERAND },
	{ t2_primitive, iiDOMAIN_ERROR_OBJECT },
	{ t2_primitive, iiDOMAIN_ERROR_EXPECTED_CLASS },
	{ t2_primitive, iiPARSE_ERROR_STRING },
	{ t2_primitive, iiPARSE_ERROR_EXPECTED_CLASS },
	{ t2_primitive, iiSIMPLE_ERROR_FORMAT_STRING },
	{ t2_primitive, iiSIMPLE_ERROR_FORMAT_ARGUMENTS },
	{ t2_primitive, iiSTREAM_ERROR_STREAM },
	{ t2_primitive, iiUNDEFINED_ENTITY_NAME },
	{ t2_primitive, iiUNDEFINED_ENTITY_NAMESPACE },
	{ t2_primitive, iiIDENTITY },
	{ t2_primitive, iiGET_UNIVERSAL_TIME },
	{ t2_primitive, iiGET_INTERNAL_RUN_TIME },
	{ t2_primitive, iiGET_INTERNAL_REAL_TIME },
	{ t2_primitive, iiGET_INTERNAL_TIME_UNITS_PER_SECOND },
	{ t2_primitive, iiSYSTEM },
	{ t2_primitive, iiEXIT },
	{ t2_primitive, iiSTRFTIME },
	{ t2_primitive, iiGET_ARGUMENT },
	{ t2_primitive, iiGET_ENVIRONMENT },
	{ t2_op_1, iiARITY_ERROR },
	{ t2_primitive, iiEVAL },
	//
};

#endif // #ifdef TISL_TRANSALTOR_2_C

#endif// #ifndef TISL_COMMAND_H
