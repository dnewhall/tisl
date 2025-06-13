//
// TISL/src/tisl/built_in_object.c
// TISL Ver. 4.x
// 

#include <math.h>

#define TISL_BUILT_IN_OBJECT_C
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "built_in_object.h"
#include "writer.h"

/////////////////////////////

static VM_RET initialize_built_in_package(tPVM vm);
static VM_RET initialize_built_in_class(tPVM vm);
static VM_RET initialize_built_in_function(tPVM vm);
static VM_RET initialize_built_in_generic_function(tPVM vm);
static VM_RET initialize_built_in_variable(tPVM vm);
static VM_RET initialize_system_condition(tPVM vm);

static VM_RET initialize_built_in_bind(tPVM vm, const tINT package_id, tCSTRING bind_name, tPCELL* bind);
static VM_RET initialize_built_in_class_(tPVM vm, const tINT package_id, tCSTRING bind_name, tPOBJECT obj);
static VM_RET initialize_built_in_function_(tPVM vm, const tINT package_id, tCSTRING bind_name, tPOBJECT obj);

/////////////////////////////

typedef struct BUILT_IN_BIND_TABLE_	BUILT_IN_BIND_TABLE;
struct BUILT_IN_BIND_TABLE_ {
	tCSTRING	name;
	tINT		id;
	tINT		package_id;
};

#define PACKAGE_TOP			0
#define PACKAGE_ISLISP		1
#define PACKAGE_SYSTEM		2

/////////////////////////////

// 組込みクラス
const BUILT_IN_BIND_TABLE table_of_built_in_class[]={
	/* 0*/{ "<object>", CLASS_OBJECT, PACKAGE_ISLISP },
	/* 1*/{ "<basic-array>", CLASS_BASIC_ARRAY, PACKAGE_ISLISP },
	/* 2*/{ "<basic-array*>", CLASS_BASIC_ARRAY_A, PACKAGE_ISLISP },
	/* 3*/{ "<general-array*>", CLASS_GENERAL_ARRAY_A, PACKAGE_ISLISP },
	/* 4*/{ "<basic-vector>", CLASS_BASIC_VECTOR, PACKAGE_ISLISP },
	/* 5*/{ "<general-vector>", CLASS_GENERAL_VECTOR, PACKAGE_ISLISP },
	/* 6*/{ "<string>", CLASS_STRING, PACKAGE_ISLISP },
	/* 7*/{ "<built-in-class>", CLASS_BUILT_IN_CLASS, PACKAGE_ISLISP },
	/* 8*/{ "<character>", CLASS_CHARACTER, PACKAGE_ISLISP },
	/* 9*/{ "<function>", CLASS_FUNCTION, PACKAGE_ISLISP },
	/*10*/{ "<generic-function>", CLASS_GENERIC_FUNCTION, PACKAGE_ISLISP },
	/*11*/{ "<local-function>", CLASS_LOCAL_FUNCTION, PACKAGE_SYSTEM },
	/*12*/{ "<library-function>", CLASS_LIBRARY_FUNCTION, PACKAGE_SYSTEM },
	/*13*/{ "<standard-generic-function>", CLASS_STANDARD_GENERIC_FUNCTION, PACKAGE_ISLISP },
	/*14*/{ "<list>", CLASS_LIST, PACKAGE_ISLISP },
	/*15*/{ "<cons>", CLASS_CONS, PACKAGE_ISLISP },
	/*16*/{ "<number>", CLASS_NUMBER, PACKAGE_ISLISP },
	/*17*/{ "<integer>", CLASS_INTEGER, PACKAGE_ISLISP },
	/*18*/{ "<float>", CLASS_FLOAT, PACKAGE_ISLISP },
	/*19*/{ "<serious-condition>", CLASS_SERIOUS_CONDITION, PACKAGE_ISLISP },
	/*20*/{ "<error>", CLASS_ERROR, PACKAGE_ISLISP },
	/*21*/{ "<arithmetic-error>", CLASS_ARITHMETIC_ERROR, PACKAGE_ISLISP },
	/*22*/{ "<division-by-zero>", CLASS_DIVISION_BY_ZERO, PACKAGE_ISLISP },
	/*23*/{ "<floating-point-overflow>", CLASS_FLOATING_POINT_OVERFLOW, PACKAGE_ISLISP },
	/*24*/{ "<floating-point-underflow>", CLASS_FLOATING_POINT_UNDERFLOW, PACKAGE_ISLISP },
	/*25*/{ "<control-error>", CLASS_CONTROL_ERROR, PACKAGE_ISLISP },
	/*26*/{ "<parse-error>", CLASS_PARSE_ERROR, PACKAGE_ISLISP },
	/*27*/{ "<program-error>", CLASS_PROGRAM_ERROR, PACKAGE_ISLISP },
	/*28*/{ "<domain-error>", CLASS_DOMAIN_ERROR, PACKAGE_ISLISP },
	/*29*/{ "<undefined-entity>", CLASS_UNDEFINED_ENTITY, PACKAGE_ISLISP },
	/*30*/{ "<unbound-variable>", CLASS_UNBOUND_VARIABLE, PACKAGE_ISLISP },
	/*31*/{ "<undefined-function>", CLASS_UNDEFINED_FUNCTION, PACKAGE_ISLISP },
	/*32*/{ "<simple-error>", CLASS_SIMPLE_ERROR, PACKAGE_ISLISP },
	/*33*/{ "<stream-error>", CLASS_STREAM_ERROR, PACKAGE_ISLISP },
	/*34*/{ "<end-of-stream>", CLASS_END_OF_STREAM, PACKAGE_ISLISP },
	/*35*/{ "<storage-exhausted>", CLASS_STORAGE_EXHAUSTED, PACKAGE_ISLISP },
	/*36*/{ "<standard-class>", CLASS_STANDARD_CLASS, PACKAGE_ISLISP },
	/*37*/{ "<stream>", CLASS_STREAM, PACKAGE_ISLISP },
	/*38*/{ "<string-stream>", CLASS_STRING_STREAM, PACKAGE_SYSTEM },
	/*39*/{ "<file-stream>", CLASS_FILE_STREAM, PACKAGE_SYSTEM },
	/*40*/{ "<symbol>", CLASS_SYMBOL, PACKAGE_ISLISP },
	/*41*/{ "<null>", CLASS_NULL, PACKAGE_ISLISP },
	/*42*/{ "<special-operator>", CLASS_SPECIAL_OPERATOR, PACKAGE_SYSTEM },
	/*43*/{ "<macro>", CLASS_MACRO, PACKAGE_SYSTEM },
	/*44*/{ "<dynamic-link-library>", CLASS_DYNAMIC_LINK_LIBRARY, PACKAGE_SYSTEM },
	/*45*/{ "<package>", CLASS_PACKAGE, PACKAGE_SYSTEM },
	/*46*/{ "<foreign-object>", CLASS_FOREIGN_OBJECT, PACKAGE_SYSTEM },
	/*47*/{ "<primitive-function>", CLASS_PRIMITIVE_FUNCTION, PACKAGE_SYSTEM },
	/*48*/{ "<defining-operator>", CLASS_DEFINING_OPERATOR, PACKAGE_SYSTEM },
	/*49*/{ "<violation>", CLASS_VIOLATION, PACKAGE_SYSTEM },
	/*50*/{ "<foreign-class>", CLASS_FOREIGN_CLASS, PACKAGE_SYSTEM },
};

#define NUMBER_OF_BUILT_IN_CLASS	51

/////////////////////////////
// 特殊演算子
const BUILT_IN_BIND_TABLE table_of_special_operator[]={
	{ "function", bFUNCTION, PACKAGE_ISLISP },
	{ "lambda", bLAMBDA, PACKAGE_ISLISP },
	{ "labels", bLABELS, PACKAGE_ISLISP },
	{ "flet", bFLET, PACKAGE_ISLISP },
	{ "and", bAND, PACKAGE_ISLISP },
	{ "or", bOR, PACKAGE_ISLISP },
	{ "quote", bQUOTE, PACKAGE_ISLISP },
	{ "setq", bSETQ, PACKAGE_ISLISP },
	{ "setf", bSETF, PACKAGE_ISLISP },
	{ "let", bLET, PACKAGE_ISLISP },
	{ "let*", bLETA, PACKAGE_ISLISP },
	{ "dynamic", bDYNAMIC, PACKAGE_ISLISP },
	{ "dynamic-let", bDYNAMIC_LET, PACKAGE_ISLISP },
	{ "if", bIF, PACKAGE_ISLISP },
	{ "cond", bCOND, PACKAGE_ISLISP },
	{ "case", bCASE, PACKAGE_ISLISP },
	{ "case-using", bCASE_USING, PACKAGE_ISLISP },
	{ "progn", bPROGN, PACKAGE_ISLISP },
	{ "while", bWHILE, PACKAGE_ISLISP },
	{ "for", bFOR, PACKAGE_ISLISP },
	{ "block", bBLOCK, PACKAGE_ISLISP },
	{ "return-from", bRETURN_FROM, PACKAGE_ISLISP },
	{ "catch", bCATCH, PACKAGE_ISLISP },
	{ "throw", bTHROW, PACKAGE_ISLISP },
	{ "tagbody", bTAGBODY, PACKAGE_ISLISP },
	{ "go", bGO, PACKAGE_ISLISP },
	{ "unwind-protect", bUNWIND_PROTECT, PACKAGE_ISLISP },
	{ "class", bCLASS, PACKAGE_ISLISP },
	{ "the", bTHE, PACKAGE_ISLISP },
	{ "assure", bASSURE, PACKAGE_ISLISP },
	{ "convert", bCONVERT, PACKAGE_ISLISP },
	{ "with-standard-input", bWITH_STANDARD_INPUT, PACKAGE_ISLISP },
	{ "with-standard-output", bWITH_STANDARD_OUTPUT, PACKAGE_ISLISP },
	{ "with-error-output", bWITH_ERROR_OUTPUT, PACKAGE_ISLISP },
	{ "with-open-input-file", bWITH_OPEN_INPUT_FILE, PACKAGE_ISLISP },
	{ "with-open-output-file", bWITH_OPEN_OUTPUT_FILE, PACKAGE_ISLISP },
	{ "with-open-io-file", bWITH_OPEN_IO_FILE, PACKAGE_ISLISP },
	{ "ignore-errors", bIGNORE_ERRORS, PACKAGE_ISLISP },
	{ "continue-condition", bCONTINUE_CONDITION, PACKAGE_ISLISP },
	{ "with-handler", bWITH_HANDLER, PACKAGE_ISLISP },
	// depend on TISL
	{ "time", bTIME, PACKAGE_SYSTEM },
	{ "in-package", bIN_PACKAGE, PACKAGE_SYSTEM },
};

// 定義演算子
const BUILT_IN_BIND_TABLE table_of_defining_operator[]={
	{ "defconstant", bDEFCONSTANT, PACKAGE_ISLISP },
	{ "defglobal", bDEFGLOBAL, PACKAGE_ISLISP },
	{ "defdynamic", bDEFDYNAMIC, PACKAGE_ISLISP },
	{ "defun", bDEFUN, PACKAGE_ISLISP },
	{ "defclass", bDEFCLASS, PACKAGE_ISLISP },
	{ "defgeneric", bDEFGENERIC, PACKAGE_ISLISP },
	{ "defmethod", bDEFMETHOD, PACKAGE_ISLISP },
	{ "defmacro", bDEFMACRO, PACKAGE_ISLISP },
	{ "defpackage", bDEFPACKAGE, PACKAGE_SYSTEM },
	{ "deflink", bDEFLINK, PACKAGE_SYSTEM },
};

const BUILT_IN_BIND_TABLE table_of_primitive_operator[]={
	// function
	{ "functionp", bFUNCTIONP, PACKAGE_ISLISP },
	{ "apply", bAPPLY, PACKAGE_ISLISP },
	{ "funcall", bFUNCALL, PACKAGE_ISLISP },
	// equality
	{ "eq", bEQ, PACKAGE_ISLISP },
	{ "eql", bEQL, PACKAGE_ISLISP },
	{ "equal", bEQUAL, PACKAGE_ISLISP },
	{ "not", bNOT, PACKAGE_ISLISP },
	// generic function
	{ "generic-function-p", bGENERIC_FUNCTION_P, PACKAGE_ISLISP },
	// class
	{ "class-of", bCLASS_OF, PACKAGE_ISLISP },
	{ "instancep", bINSTANCEP, PACKAGE_ISLISP },
	{ "subclassp", bSUBCLASSP, PACKAGE_ISLISP },
	// <sym{ "", bol>
	{ "symbolp", bSYMBOLP, PACKAGE_ISLISP },
	{ "property", bPROPERTY, PACKAGE_ISLISP },
	{ "set-property", bSET_PROPERTY, PACKAGE_ISLISP },
	{ "remove-property", bREMOVE_PROPERTY, PACKAGE_ISLISP },
	{ "gensym", bGENSYM, PACKAGE_ISLISP },
	// <num{ "", ber>
	{ "numberp", bNUMBERP, PACKAGE_ISLISP },
	{ "parse-number", bPARSE_NUMBER, PACKAGE_ISLISP },
	{ "=", bNUMBER_EQUAL, PACKAGE_ISLISP },
	{ "/=", bNUMBER_NOT_EQUAL, PACKAGE_ISLISP },
	{ ">=", bNUMBER_GE, PACKAGE_ISLISP },
	{ "<=", bNUMBER_LE, PACKAGE_ISLISP },
	{ ">", bNUMBER_GREATER, PACKAGE_ISLISP },
	{ "<", bNUMBER_LESS, PACKAGE_ISLISP },
	{ "+", bADDITION, PACKAGE_ISLISP },
	{ "*", bMULTIPLICATION, PACKAGE_ISLISP },
	{ "-", bSUBTRACTION, PACKAGE_ISLISP },
	{ "quotient", bQUOTIENT, PACKAGE_ISLISP },
	{ "reciprocal", bRECIPROCAL, PACKAGE_ISLISP },
	{ "max", bMAX, PACKAGE_ISLISP },
	{ "min", bMIN, PACKAGE_ISLISP },
	{ "abs", bABS, PACKAGE_ISLISP },
	{ "exp", bEXP, PACKAGE_ISLISP },
	{ "log", bLOG, PACKAGE_ISLISP },
	{ "expt", bEXPT, PACKAGE_ISLISP },
	{ "sqrt", bSQRT, PACKAGE_ISLISP },
	{ "sin", bSIN, PACKAGE_ISLISP },
	{ "cos", bCOS, PACKAGE_ISLISP },
	{ "tan", bTAN, PACKAGE_ISLISP },
	{ "atan", bATAN, PACKAGE_ISLISP },
	{ "atan2", bATAN2, PACKAGE_ISLISP },
	{ "sinh", bSINH, PACKAGE_ISLISP },
	{ "cosh", bCOSH, PACKAGE_ISLISP },
	{ "tanh", bTANH, PACKAGE_ISLISP },
	{ "atanh", bATANH, PACKAGE_ISLISP },
	// <float>
	{ "floatp", bFLOATP, PACKAGE_ISLISP },
	{ "float", bFLOAT, PACKAGE_ISLISP },
	{ "floor", bFLOOR, PACKAGE_ISLISP },
	{ "ceiling", bCEILING, PACKAGE_ISLISP },
	{ "truncate", bTRUNCATE, PACKAGE_ISLISP },
	{ "round", bROUND, PACKAGE_ISLISP },
	// <integer>
	{ "integerp", bINTEGERP, PACKAGE_ISLISP },
	{ "div", bDIV, PACKAGE_ISLISP },
	{ "mod", bMOD, PACKAGE_ISLISP },
	{ "gcd", bGCD, PACKAGE_ISLISP },
	{ "lcm", bLCM, PACKAGE_ISLISP },
	{ "isqrt", bISQRT, PACKAGE_ISLISP },
	// <character>
	{ "characterp", bCHARACTERP, PACKAGE_ISLISP },
	{ "char=", bCHAR_EQUAL, PACKAGE_ISLISP },
	{ "char/=", bCHAR_NOT_EQUAL, PACKAGE_ISLISP },
	{ "char<", bCHAR_LESS, PACKAGE_ISLISP },
	{ "char>", bCHAR_GREATER, PACKAGE_ISLISP },
	{ "char<=", bCHAR_LE, PACKAGE_ISLISP },
	{ "char>=", bCHAR_GE, PACKAGE_ISLISP },
	// <list>
	// <cons>
	{ "consp", bCONSP, PACKAGE_ISLISP },
	{ "cons", bCONS, PACKAGE_ISLISP },
	{ "car", bCAR, PACKAGE_ISLISP },
	{ "cdr", bCDR, PACKAGE_ISLISP },
	{ "set-car", bSET_CAR, PACKAGE_ISLISP },
	{ "set-cdr", bSET_CDR, PACKAGE_ISLISP },
	// <null>
	{ "null", bNULL, PACKAGE_ISLISP },
	// list operations
	{ "listp", bLISTP, PACKAGE_ISLISP },
	{ "create-list", bCREATE_LIST, PACKAGE_ISLISP },
	{ "list", bLIST, PACKAGE_ISLISP },
	{ "reverse", bREVERSE, PACKAGE_ISLISP },
	{ "nreverse", bNREVERSE, PACKAGE_ISLISP },
	{ "append", bAPPEND, PACKAGE_ISLISP },
	{ "member", bMEMBER, PACKAGE_ISLISP },
	{ "mapcar", bMAPCAR, PACKAGE_ISLISP },
	{ "mapc", bMAPC, PACKAGE_ISLISP },
	{ "mapcan", bMAPCAN, PACKAGE_ISLISP },
	{ "maplist", bMAPLIST, PACKAGE_ISLISP },
	{ "mapl", bMAPL, PACKAGE_ISLISP },
	{ "mapcon", bMAPCON, PACKAGE_ISLISP },
	{ "assoc", bASSOC, PACKAGE_ISLISP },
	// array
	// array oprations
	{ "basic-array-p", bBASIC_ARRAY_P, PACKAGE_ISLISP },
	{ "basic-array*-p", bBASIC_ARRAY_A_P, PACKAGE_ISLISP },
	{ "general-array*-p", bGENERAL_ARRAY_A_P, PACKAGE_ISLISP },
	{ "create-array", bCREATE_ARRAY, PACKAGE_ISLISP },
	{ "aref", bAREF, PACKAGE_ISLISP },
	{ "garef", bGAREF, PACKAGE_ISLISP },
	{ "set-aref", bSET_AREF, PACKAGE_ISLISP },
	{ "set-garef", bSET_GAREF, PACKAGE_ISLISP },
	{ "array-dimensions", bARRAY_DIMENSIONS, PACKAGE_ISLISP },
	// vectors
	{ "basic-vector-p", bBASIC_VETOR_P, PACKAGE_ISLISP },
	{ "general-vector-p", bGENERAL_VECTOR_P, PACKAGE_ISLISP },
	{ "create-vector", bCREATE_VECTOR, PACKAGE_ISLISP },
	{ "vector", bVECTOR, PACKAGE_ISLISP },
	// <string>
	{ "stringp", bSTRINGP, PACKAGE_ISLISP },
	{ "create-string", bCREATE_STRING, PACKAGE_ISLISP },
	{ "string=", bSTRING_EQUAL, PACKAGE_ISLISP },
	{ "string/=", bSTRING_NOT_EQUAL, PACKAGE_ISLISP },
	{ "string<", bSTRING_LESS, PACKAGE_ISLISP },
	{ "string>", bSTRING_GREATER, PACKAGE_ISLISP },
	{ "string>=", bSTRING_GE, PACKAGE_ISLISP },
	{ "string<=", bSTRING_LE, PACKAGE_ISLISP },
	{ "char-index", bCHAR_INDEX, PACKAGE_ISLISP },
	{ "string-index", bSTRING_INDEX, PACKAGE_ISLISP },
	{ "string-append", bSTRING_APPEND, PACKAGE_ISLISP },
	// sequence functions
	{ "length", bLENGTH, PACKAGE_ISLISP },
	{ "elt", bELT, PACKAGE_ISLISP },
	{ "set-elt", bSET_ELT, PACKAGE_ISLISP },
	{ "subseq", bSUBSEQ, PACKAGE_ISLISP },
	{ "map-into", bMAP_INTO, PACKAGE_ISLISP },
	// <stream>
	{ "streamp", bSTREAMP, PACKAGE_ISLISP },
	{ "open-stream-p", bOPEN_STREAM_P, PACKAGE_ISLISP },
	{ "input-stream-p", bINPUT_STREAM_P, PACKAGE_ISLISP },
	{ "output-stream-p", bOUTPUT_STREAM_P, PACKAGE_ISLISP },
	{ "standard-input", bSTANDARD_INPUT, PACKAGE_ISLISP },
	{ "standard-output", bSTANDARD_OUTPUT, PACKAGE_ISLISP },
	{ "error-output", bERROR_OUTPUT, PACKAGE_ISLISP },
	// streams to files
	{ "open-input-file", bOPEN_INPUT_FILE, PACKAGE_ISLISP },
	{ "open-output-file", bOPEN_OUTPUT_FILE, PACKAGE_ISLISP },
	{ "open-io-file", bOPEN_IO_FILE, PACKAGE_ISLISP },
	{ "close", bCLOSE, PACKAGE_ISLISP },
	{ "finish-output", bFINISH_OUTPUT, PACKAGE_ISLISP },
	// other streams
	{ "create-string-input-stream", bCREATE_STRING_INPUT_STREAM, PACKAGE_ISLISP },
	{ "create-string-output-stream", bCREATE_STRING_OUTPUT_STREAM, PACKAGE_ISLISP },
	{ "get-output-stream-string", bGET_OUTPUT_STREAM_STRING, PACKAGE_ISLISP },
	// input and output
	// character I/O
	{ "read", bREAD, PACKAGE_ISLISP },
	{ "read-char", bREAD_CHAR, PACKAGE_ISLISP },
	{ "preview-char", bPREVIEW_CHAR, PACKAGE_ISLISP },
	{ "read-line", bREAD_LINE, PACKAGE_ISLISP },
	{ "stream-ready-p", bSTREAM_READY_P, PACKAGE_ISLISP },
	{ "format", bFORMAT, PACKAGE_ISLISP },
	{ "format-char", bFORMAT_CHAR, PACKAGE_ISLISP },
	{ "format-float", bFORMAT_FLOAT, PACKAGE_ISLISP },
	{ "format-fresh-line", bFORMAT_FRESH_LINE, PACKAGE_ISLISP },
	{ "format-integer", bFORMAT_INTEGER, PACKAGE_ISLISP },
	{ "format-object", bFORMAT_OBJECT, PACKAGE_ISLISP },
	{ "format-tab", bFORMAT_TAB, PACKAGE_ISLISP },
	// binary I/O
	{ "read-byte", bREAD_BYTE, PACKAGE_ISLISP },
	{ "write-byte", bWRITE_BYTE, PACKAGE_ISLISP },
	// files
	{ "probe-file", bPROBE_FILE, PACKAGE_ISLISP },
	{ "file-position", bFILE_POSITION, PACKAGE_ISLISP },
	{ "set-file-position", bSET_FILE_POSITION, PACKAGE_ISLISP },
	{ "file-length", bFILE_LENGTH, PACKAGE_ISLISP },
	// condition system
	// signal and handle conditions
	{ "error", bERROR, PACKAGE_ISLISP },
	{ "cerror", bCERROR, PACKAGE_ISLISP },
	{ "signal-condition", bSIGNAL_CONDITION, PACKAGE_ISLISP },
	{ "condition-continuable", bCONDITION_CONTINUABLE, PACKAGE_ISLISP },
	// data associated with condition classes
	{ "arithmetic-error-operation", bARITHMETIC_ERROR_OPERATION, PACKAGE_ISLISP },
	{ "arithmetic-error-operands", bARITHMETIC_ERROR_OPERANDS, PACKAGE_ISLISP },
	{ "domain-error-object", bDOMAIN_ERROR_OBJECT, PACKAGE_ISLISP },
	{ "domain-error-expected-class", bDOMAIN_ERROR_EXPECTED_CLASS, PACKAGE_ISLISP },
	{ "parse-error-string", bPARSE_ERROR_STRING, PACKAGE_ISLISP },
	{ "parse-error-expected-class", bPARSE_ERROR_EXPECTED_CLASS, PACKAGE_ISLISP },
	{ "simple-error-format-string", bSIMPLE_ERROR_FORMAT_STRING, PACKAGE_ISLISP },
	{ "simple-error-format-arguments", bSIMPLE_ERROR_FORMAT_ARGUMENTS, PACKAGE_ISLISP },
	{ "stream-error-stream", bSTREAM_ERROR_STREAM, PACKAGE_ISLISP },
	{ "undefined-entity-name", bUNDEFINED_ENTITY_NAME, PACKAGE_ISLISP },
	{ "undefined-entity-namespace", bUNDEFINED_ENTITY_NAMESPACE, PACKAGE_ISLISP },
	// miscellaneous
	{ "identity", bIDENTITY, PACKAGE_ISLISP },
	{ "get-universal-time", bGET_UNIVERSAL_TIME, PACKAGE_ISLISP },
	{ "get-internal-run-time", bGET_INTERNAL_RUN_TIME, PACKAGE_ISLISP },
	{ "get-internal-real-time", bGET_INTERNAL_REAL_TIME, PACKAGE_ISLISP },
	{ "internal-time-units-per-second", bINTERNAL_TIME_UNITS_PER_SECOND, PACKAGE_ISLISP },
	// depend on TISL
	{ "system", bSYSTEM, PACKAGE_SYSTEM },
	{ "exit", bEXIT, PACKAGE_SYSTEM },
	{ "strftime", bSTRFTIME, PACKAGE_SYSTEM },
	{ "get-argument", bGET_ARGUMENT, PACKAGE_SYSTEM },
	{ "get-environment", bGET_ENVIRONMENT, PACKAGE_SYSTEM },
	{ "eval", bEVAL, PACKAGE_SYSTEM },
};

/////////////////////////////

void clear_global_objects(void)
{
	int i;
	OBJECT_SET_NIL(&nil);
	OBJECT_SET_UNBOUND(&unbound);

	string_plus=0;
	string_minus=0;
	string_system=0;
	string_islisp=0;

	for (i=0; i<NUMBER_OF_GLOBAL_SYMBOL; i++) {
		global_symbol[i]=0;
	}
	list_object_instance=0;
	list_islisp_system=0;
	gfunction_initialize_object=0;
	condition_system_error=0;
	condition_storage_exhausted=0;
	condition_stack_overflow=0;
	tisl_object_storage_exhausted=0;
}

VM_RET initialize_global_objects(tPTISL tisl)
{
	tINT i;
	tPCELL cons, string, p;
	tOBJECT obj;
	tPVM vm=tisl_get_main_vm(tisl);
	// string
	if (tisl_get_string(tisl, vm, "+", &string_plus)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "-", &string_minus)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "system", &string_system)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "islisp", &string_islisp)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "Continue with no special action.", &string_continue_condition)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "simple-error", &string_simple_error)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "system-error", &string_system_error)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "storage-exhausted", &string_storage_exhausted)) return VM_ERROR;
	if (tisl_get_string(tisl, vm, "stack-overflow", &string_stack_overflow)) return VM_ERROR;
	// symbol
	if (cons_create(vm, &cons, &nil, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, cons);
	if (vm_push(vm, &obj)) return VM_ERROR;
	//
	for (i=0; i<NUMBER_OF_GLOBAL_SYMBOL; i++) {
		if (tisl_get_string(tisl, vm, global_symbol_table[i].string, &string)) { vm_pop(vm); return VM_ERROR; }
		OBJECT_SET_STRING(&obj, string);
		cons_set_car(cons, &obj);
		if (tisl_get_symbol(tisl, vm, cons, global_symbol_table[i].complete, global_symbol+i)) { vm_pop(vm); return VM_ERROR; }
	}
	// list
	// (object instance)
	OBJECT_SET_SYMBOL(&obj, SYMBOL_INSTANCE);
	if (cons_create_(vm, &p, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	cons_set_cdr(cons, &obj);
	OBJECT_SET_SYMBOL(&obj, SYMBOL_OBJECT);
	cons_set_car(cons, &obj);
	list_object_instance=cons;
	vm_pop(vm);
	

	return VM_OK;
}

VM_RET initialize_built_in_object(tPTISL tisl)
{
	tPVM vm=tisl_get_main_vm(tisl);
	if (initialize_built_in_package(vm)) return VM_ERROR;
	if (initialize_built_in_class(vm)) return VM_ERROR;
	if (initialize_built_in_variable(vm)) return VM_ERROR;
	if (initialize_built_in_function(vm)) return VM_ERROR;
	if (initialize_system_condition(vm)) return VM_ERROR;
	return VM_OK;
}

/////////////////////////////

static VM_RET initialize_built_in_package(tPVM vm)
{
	tPCELL list, islisp, system, bind, name;
	tOBJECT obj;
	// system package
	if (tisl_get_string(vm_get_tisl(vm), vm, "system", &name)) return VM_ERROR;
	if (package_add_bind(vm, vm_get_top_package(vm), name, &bind)) return VM_ERROR;
	bind_set_package_public(bind);
	if (package_create(vm, bind, 0, name, vm_get_top_package(vm), &system)) return VM_ERROR;
	cell_to_object(system, &obj);
	bind_set_package(bind, &obj);
	// islisp package
	if (tisl_get_string(vm_get_tisl(vm), vm, "islisp", &name)) return VM_ERROR;
	if (package_add_bind(vm, vm_get_top_package(vm), name, &bind)) return VM_ERROR;
	bind_set_package_public(bind);
	if (cons_create(vm, &list, &obj, &nil)) return VM_ERROR;
	if (package_create(vm, bind, list, name, vm_get_top_package(vm), &islisp)) return VM_ERROR;
	cell_to_object(islisp, &obj);
	bind_set_package(bind, &obj);
	/*!!!*/	vm_set_current_package(vm, islisp);/*!!!*/
	{
		tOBJECT tmp, tmp2;
		tPCELL p;
		OBJECT_SET_PACKAGE(&tmp, system);
		if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;
		OBJECT_SET_PACKAGE(&tmp, islisp);
		OBJECT_SET_CONS(&tmp2, p);
		if (cons_create(vm, &list_islisp_system, &tmp, &tmp)) return VM_ERROR;
	}
	return VM_OK;
}

static VM_RET initialize_built_in_class(tPVM vm)
{
	tOBJECT obj;
	tINT i;
	// 組込みクラス
	for (i=0; i<NUMBER_OF_BUILT_IN_CLASS; i++) {
		OBJECT_SET_BUILT_IN_CLASS(&obj, table_of_built_in_class[i].id);
		if (initialize_built_in_class_(vm, table_of_built_in_class[i].package_id, table_of_built_in_class[i].name, &obj)) return VM_ERROR;
	}
	// 標準クラス <standard-object>
	{
		tPCELL name, bind;
		if (tisl_get_string(vm_get_tisl(vm), vm, "<standard-object>", &name)) return VM_ERROR;
		if (package_add_bind(vm, vm_get_islisp_package(vm), name, &bind)) return VM_ERROR;
		if (standard_class_create_(vm, name, 0, 0, tTRUE, &sclass_standard_object)) return VM_ERROR;
		OBJECT_SET_STANDARD_CLASS(&obj, sclass_standard_object);
		bind_set_class(bind, &obj);
		bind_set_class_public(bind);
	}
	return VM_OK;
}

static VM_RET initialize_built_in_function(tPVM vm)
{
	tOBJECT obj;
	tINT i;

	//special operator
	for (i=0; i<NUMBER_OF_SPECIAL_OPERATOR; i++) {
		OBJECT_SET_SPECIAL_OPERATOR(&obj, table_of_special_operator[i].id);
		if (initialize_built_in_function_(vm, table_of_special_operator[i].package_id, table_of_special_operator[i].name, &obj)) return VM_ERROR;
	}
	// defining operator
	for (i=0; i<NUMBER_OF_DEFINING_OPERATOR; i++) {
		OBJECT_SET_DEFINING_OPERATOR(&obj, table_of_defining_operator[i].id);
		if (initialize_built_in_function_(vm, table_of_defining_operator[i].package_id, table_of_defining_operator[i].name, &obj)) return VM_ERROR;
	}
	// primitive operator
	for (i=0; i<NUMBER_OF_PRIMITIVE_OPERATOR; i++) {
		OBJECT_SET_PRIMITIVE_OPERATOR(&obj, table_of_primitive_operator[i].id);
		if (initialize_built_in_function_(vm, table_of_primitive_operator[i].package_id, table_of_primitive_operator[i].name, &obj)) return VM_ERROR;
	}
	// generic function
	return initialize_built_in_generic_function(vm);
}

/////////////////////////////

static VM_RET initialize_built_in_variable(tPVM vm)
{
	tOBJECT obj;
	tPCELL bind;

	// t
	if (initialize_built_in_bind(vm, PACKAGE_ISLISP, "t", &bind)) return VM_ERROR;
	OBJECT_SET_SYMBOL(&obj, SYMBOL_T);
	bind_set_variable(bind, &obj);
	bind_set_variable_public(bind);
	// nil
	if (initialize_built_in_bind(vm, PACKAGE_ISLISP, "nil", &bind)) return VM_ERROR;
	OBJECT_SET_NIL(&obj);
	bind_set_variable(bind, &obj);
	bind_set_variable_public(bind);
	// *pi*
	if (initialize_built_in_bind(vm, PACKAGE_ISLISP, "*pi*", &bind)) return VM_ERROR;
	OBJECT_SET_FLOAT(&obj, (tFLOAT)(atan(1)*4));
	bind_set_variable(bind, &obj);
	bind_set_variable_public(bind);
	// *most-positive-float*
	if (initialize_built_in_bind(vm, PACKAGE_ISLISP, "*most-positive-float*", &bind)) return VM_ERROR;
	OBJECT_SET_FLOAT(&obj, TISL_MOST_POSITIVE_FLOAT);
	bind_set_variable(bind, &obj);
	bind_set_variable_public(bind);
	// *most-negative-float*
	if (initialize_built_in_bind(vm, PACKAGE_ISLISP, "*most-negative-float*", &bind)) return VM_ERROR;
	OBJECT_SET_FLOAT(&obj, TISL_MOST_NEGATIVE_FLOAT);
	bind_set_variable(bind, &obj);
	bind_set_variable_public(bind);

	return VM_OK;
}

static VM_RET initialize_built_in_bind(tPVM vm, const tINT package_id, tCSTRING bind_name, tPCELL* bind)
{
	tPCELL package, name;

	switch (package_id) {
	case PACKAGE_ISLISP:
		package=vm_get_islisp_package(vm);
		break;
	case PACKAGE_SYSTEM:
		package=vm_get_system_package(vm);
		break;
	case PACKAGE_TOP:
		package=vm_get_top_package(vm);
		break;
	default:
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
	if (tisl_get_string(vm_get_tisl(vm), vm, bind_name, &name)) return VM_ERROR;
	*bind=package_get_bind(package, name);
	if ((!*bind)&&
		package_add_bind(vm, package, name, bind)) {
		return VM_ERROR;
	} else {
		return VM_OK;
	}
}

static VM_RET initialize_built_in_class_(tPVM vm, const tINT package_id, tCSTRING bind_name, tPOBJECT obj)
{
	tPCELL bind;
	if (initialize_built_in_bind(vm, package_id, bind_name, &bind)) return VM_ERROR;
	bind_set_class(bind, obj);
	bind_set_class_public(bind);
	return VM_OK;
}

static VM_RET initialize_built_in_function_(tPVM vm, const tINT package_id, tCSTRING bind_name, tPOBJECT obj)
{
	tPCELL bind;
	if (initialize_built_in_bind(vm, package_id, bind_name, &bind)) return VM_ERROR;
	bind_set_function(bind, obj);
	bind_set_function_public(bind);
	return VM_OK;
}

/////////////////////////////

static VM_RET initialize_built_in_generic_function(tPVM vm)
{
	tPCELL lambda_list, gfunction, bind, name, p, method;
	tOBJECT tmp, tmp2;
	lambda_list=0;
	// create (class list)
	if (tisl_get_string(vm_get_tisl(vm), vm, "create", &name)) return VM_ERROR;
	if (package_add_bind(vm, vm_get_islisp_package(vm), name, &bind)) return VM_ERROR;
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_LIST);
	if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_CLASS);
	OBJECT_SET_CONS(&tmp2, p);
	if (cons_create(vm, &lambda_list, &tmp, &tmp2)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, lambda_list);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	if (gfunction_create_(vm, 2, tTRUE, tTRUE, lambda_list, &gfunction)) return VM_ERROR;
	vm_pop(vm);
	OBJECT_SET_GENERIC_FUNCTION(&tmp, gfunction);
	bind_set_function(bind, &tmp);
	bind_set_function_public(bind);
	if (method_create_system(vm, 0, 0, SYSTEM_METHOD_CREATE, &method)) return VM_ERROR;
	if (gfunction_add_method(vm, gfunction, method)) return VM_ERROR;
	// initialize-object (instance list)
	if (tisl_get_string(vm_get_tisl(vm), vm, "initialize-object", &name)) return VM_ERROR;
	if (package_add_bind(vm, vm_get_islisp_package(vm), name, &bind)) return VM_ERROR;
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_INSTANCE);
	OBJECT_SET_CONS(&tmp2, p);
	if (cons_create_(vm, &lambda_list, &tmp, &tmp2)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, lambda_list);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	if (gfunction_create_(vm, 2, tFALSE, tTRUE, lambda_list, &gfunction)) return VM_ERROR;
	vm_pop(vm);
	OBJECT_SET_GENERIC_FUNCTION(&tmp, gfunction);
	bind_set_function(bind, &tmp);
	bind_set_function_public(bind);
	if (method_create_system(vm, 0, 0, SYSTEM_METHOD_INITIALIZE_OBJECT, &method)) return VM_ERROR;
	if (gfunction_add_method(vm, gfunction, method)) return VM_ERROR;
	gfunction_initialize_object=gfunction;
	// report-condition (condition stream)
	if (tisl_get_string(vm_get_tisl(vm), vm, "report-condition", &name)) return VM_ERROR;
	if (package_add_bind(vm, vm_get_islisp_package(vm), name, &bind)) return VM_ERROR;
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_STREAM);
	if (cons_create_(vm, &lambda_list, &tmp, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp2, lambda_list);
	OBJECT_SET_SYMBOL(&tmp, SYMBOL_CONDITION);
	if (cons_create(vm, &lambda_list, &tmp, &tmp2)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, lambda_list);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	if (gfunction_create_(vm, 2, tFALSE, tTRUE, lambda_list, &gfunction)) return VM_ERROR;
	vm_pop(vm);
	OBJECT_SET_GENERIC_FUNCTION(&tmp, gfunction);
	bind_set_function_public(bind);
	bind_set_function(bind, &tmp);
	if (method_create_system(vm, 0, 0, SYSTEM_METHOD_REPORT_CONDITION, &method)) return VM_ERROR;
	if (gfunction_add_method(vm, gfunction, method)) return VM_ERROR;
	return VM_OK;
}

/////////////////////////////

static VM_RET initialize_system_condition(tPVM vm)
{
	tOBJECT tmp;
	if (condition_create(vm, CLASS_SERIOUS_CONDITION, string_system_error, &unbound, &unbound, &unbound, &nil, &condition_system_error)) return VM_ERROR;
	if (condition_create(vm, CLASS_STORAGE_EXHAUSTED, string_stack_overflow, &unbound, &unbound, &unbound, &nil, &condition_stack_overflow)) return VM_ERROR;
	if (condition_create(vm, CLASS_STORAGE_EXHAUSTED, string_storage_exhausted, &unbound, &unbound, &unbound, &nil, &condition_storage_exhausted)) return VM_ERROR;
	OBJECT_SET_CONDITION(&tmp, condition_storage_exhausted);
	if (tisl_object_create(vm, &tmp, &tisl_object_storage_exhausted)) return VM_ERROR;
	return VM_OK;
}

/////////////////////////////

VM_RET special_operator_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (write_string(vm, stream, "#i(<special-operator> ")) return VM_ERROR;
	if (write_string(vm, stream, table_of_special_operator[OBJECT_GET_INTEGER(obj)].name)) return VM_ERROR;
	return write_char(vm, stream, ')');
}

VM_RET defining_operator_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (write_string(vm, stream, "#i(<defining-operator> ")) return VM_ERROR;
	if (write_string(vm, stream, table_of_defining_operator[OBJECT_GET_INTEGER(obj)].name)) return VM_ERROR;
	return write_char(vm, stream, ')');
}

VM_RET primitive_operator_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (write_string(vm, stream, "#i(<primitive-funtion> ")) return VM_ERROR;
	if (write_string(vm, stream, table_of_primitive_operator[OBJECT_GET_INTEGER(obj)].name)) return VM_ERROR;
	return write_char(vm, stream, ')');
}

/////////////////////////////

tBOOL symbol_is_special_operator(tPCELL symbol)
{
	int i;
	for (i=sAND; i<=sWITH_STANDARD_OUTPUT; i++) {
		if (symbol==global_symbol[i]) return tTRUE;
	}
	return tFALSE;
}

tINT symbol_get_special_operator_id(tPCELL symbol)
{
	int i;
	for (i=sAND; i<=sWITH_STANDARD_OUTPUT; i++) {
		if (symbol==global_symbol[i]) {
			return global_symbol_table[i].special_oepator_id;
		}
	}
	return 0;// 負の値にする？/*!!!*/
}

VM_RET symbol_is_built_in_function(tPVM vm, tPCELL symbol)
{
	int i;
	for (i=sAND; i<sSYSTEM/*=sGET_ENVIRONMENT*/; i++) {
		if (symbol==global_symbol[i]) {
			return signal_condition(vm, TISL_ERROR_IMMUTABLE_BINDING);
		}
	}
	return VM_OK;
}

