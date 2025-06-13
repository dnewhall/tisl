//
// TISL/src/tisl/built_in_object.h
// TISL Ver 4.x
//

#ifndef TISL_BUILT_IN_OBJECT_H
#define TISL_BUILT_IN_OBJECT_H

///////////////////////////////////////
// built-in-class

#define CLASS_OBJECT					0x00000000
#define CLASS_BASIC_ARRAY				0x80000000
#define CLASS_BASIC_ARRAY_A				0x80008000
#define CLASS_GENERAL_ARRAY_A			0x8000a000
#define CLASS_BASIC_VECTOR				0x80004000
#define CLASS_GENERAL_VECTOR			0x80006000
#define CLASS_STRING					0x80005000
#define CLASS_BUILT_IN_CLASS			0x40000000
#define CLASS_CHARACTER					0x20000000
#define CLASS_FUNCTION					0x10000000
#define CLASS_GENERIC_FUNCTION			0x10008000
#define CLASS_LOCAL_FUNCTION			0x10004000
#define CLASS_LIBRARY_FUNCTION			0x10002000
#define CLASS_PRIMITIVE_FUNCTION		0x10001000
#define CLASS_STANDARD_GENERIC_FUNCTION	0x10008080
#define CLASS_LIST						0x08000000
#define CLASS_CONS						0x08008000
#define CLASS_NUMBER					0x04000000
#define CLASS_INTEGER					0x04008000
#define CLASS_FLOAT						0x04004000
#define CLASS_SERIOUS_CONDITION			0x02000000
#define CLASS_ERROR						0x02008000
#define CLASS_ARITHMETIC_ERROR			0x0200a000
#define CLASS_DIVISION_BY_ZERO			0x0200a080
#define CLASS_FLOATING_POINT_OVERFLOW	0x0200a040
#define CLASS_FLOATING_POINT_UNDERFLOW	0x0200a020
#define CLASS_CONTROL_ERROR				0x02009000
#define CLASS_PARSE_ERROR				0x02008800
#define CLASS_PROGRAM_ERROR				0x02008400
#define CLASS_DOMAIN_ERROR				0x02008480
#define CLASS_UNDEFINED_ENTITY			0x02008440
#define CLASS_UNBOUND_VARIABLE			0x02008460
#define CLASS_UNDEFINED_FUNCTION		0x02008450
#define CLASS_SIMPLE_ERROR				0x02008200
#define CLASS_STREAM_ERROR				0x02008100
#define CLASS_END_OF_STREAM				0x02008180
#define CLASS_STORAGE_EXHAUSTED			0x02004000
#define CLASS_VIOLATION					0x02002000
#define CLASS_STANDARD_CLASS			0x01000000
#define CLASS_STREAM					0x00400000
#define CLASS_STRING_STREAM				0x00408000
#define CLASS_FILE_STREAM				0x00404000
#define CLASS_SYMBOL					0x00200000
#define CLASS_NULL						0x08204000
#define CLASS_SPECIAL_OPERATOR			0x00100000
#define CLASS_MACRO						0x00108000
#define CLASS_DEFINING_OPERATOR			0x00104000
#define CLASS_DYNAMIC_LINK_LIBRARY		0x00080000
#define CLASS_PACKAGE					0x00040000
#define CLASS_FOREIGN_OBJECT			0x00020000
#define CLASS_FOREIGN_CLASS				0x00010000

///////////////////////////////////////
// built-in-function

// special-operator
enum {
	bFUNCTION,
	bLAMBDA,
	bLABELS,
	bFLET,
	bAND,
	bOR,
	bQUOTE,
	bSETQ,
	bSETF,
	bLET,
	bLETA,
	bDYNAMIC,
	bDYNAMIC_LET,
	bIF,
	bCOND,
	bCASE,
	bCASE_USING,
	bPROGN,
	bWHILE,
	bFOR,
	bBLOCK,
	bRETURN_FROM,
	bCATCH,
	bTHROW,
	bTAGBODY,
	bGO,
	bUNWIND_PROTECT,
	bCLASS,
	bTHE,
	bASSURE,
	bCONVERT,
	bWITH_STANDARD_INPUT,
	bWITH_STANDARD_OUTPUT,
	bWITH_ERROR_OUTPUT,
	bWITH_OPEN_INPUT_FILE,
	bWITH_OPEN_OUTPUT_FILE,
	bWITH_OPEN_IO_FILE,
	bIGNORE_ERRORS,
	bCONTINUE_CONDITION,
	bWITH_HANDLER,
	// depend on TISL
	bTIME,
	bIN_PACKAGE,
	// end
	NUMBER_OF_SPECIAL_OPERATOR,
};

// defining-operator

enum {
	bDEFCONSTANT,
	bDEFGLOBAL,
	bDEFDYNAMIC,
	bDEFUN,
	bDEFCLASS,
	bDEFGENERIC,
	bDEFMETHOD,
	bDEFMACRO,
	bDEFPACKAGE,
	bDEFLINK,
	// end
	NUMBER_OF_DEFINING_OPERATOR,
};

// primitive-operator

enum {
	// function
	bFUNCTIONP,
	bAPPLY,
	bFUNCALL,
	// equality
	bEQ,
	bEQL,
	bEQUAL,
	bNOT,
	// generic function
	bGENERIC_FUNCTION_P,
	// class
	bCLASS_OF,
	bINSTANCEP,
	bSUBCLASSP,
	// <symbol>
	bSYMBOLP,
	bPROPERTY,
	bSET_PROPERTY,
	bREMOVE_PROPERTY,
	bGENSYM,
	// <number>
	bNUMBERP,
	bPARSE_NUMBER,
	bNUMBER_EQUAL,
	bNUMBER_NOT_EQUAL,
	bNUMBER_GE,
	bNUMBER_LE,
	bNUMBER_GREATER,
	bNUMBER_LESS,
	bADDITION,
	bMULTIPLICATION,
	bSUBTRACTION,
	bQUOTIENT,
	bRECIPROCAL,
	bMAX,
	bMIN,
	bABS,
	bEXP,
	bLOG,
	bEXPT,
	bSQRT,
	bSIN,
	bCOS,
	bTAN,
	bATAN,
	bATAN2,
	bSINH,
	bCOSH,
	bTANH,
	bATANH,
	// <float>
	bFLOATP,
	bFLOAT,
	bFLOOR,
	bCEILING,
	bTRUNCATE,
	bROUND,
	// <integer>
	bINTEGERP,
	bDIV,
	bMOD,
	bGCD,
	bLCM,
	bISQRT,
	// <character>
	bCHARACTERP,
	bCHAR_EQUAL,
	bCHAR_NOT_EQUAL,
	bCHAR_LESS,
	bCHAR_GREATER,
	bCHAR_LE,
	bCHAR_GE,
	// <list>
	// <cons>
	bCONSP,
	bCONS,
	bCAR,
	bCDR,
	bSET_CAR,
	bSET_CDR,
	// <null>
	bNULL,
	// list operations
	bLISTP,
	bCREATE_LIST,
	bLIST,
	bREVERSE,
	bNREVERSE,
	bAPPEND,
	bMEMBER,
	bMAPCAR,
	bMAPC,
	bMAPCAN,
	bMAPLIST,
	bMAPL,
	bMAPCON,
	bASSOC,
	// array
	// array oprations
	bBASIC_ARRAY_P,
	bBASIC_ARRAY_A_P,
	bGENERAL_ARRAY_A_P,
	bCREATE_ARRAY,
	bAREF,
	bGAREF,
	bSET_AREF,
	bSET_GAREF,
	bARRAY_DIMENSIONS,
	// vectors
	bBASIC_VETOR_P,
	bGENERAL_VECTOR_P,
	bCREATE_VECTOR,
	bVECTOR,
	// <string>
	bSTRINGP,
	bCREATE_STRING,
	bSTRING_EQUAL,
	bSTRING_NOT_EQUAL,
	bSTRING_LESS,
	bSTRING_GREATER,
	bSTRING_GE,
	bSTRING_LE,
	bCHAR_INDEX,
	bSTRING_INDEX,
	bSTRING_APPEND,
	// sequence functions
	bLENGTH,
	bELT,
	bSET_ELT,
	bSUBSEQ,
	bMAP_INTO,
	// <stream>
	bSTREAMP,
	bOPEN_STREAM_P,
	bINPUT_STREAM_P,
	bOUTPUT_STREAM_P,
	bSTANDARD_INPUT,
	bSTANDARD_OUTPUT,
	bERROR_OUTPUT,
	// streams to files
	bOPEN_INPUT_FILE,
	bOPEN_OUTPUT_FILE,
	bOPEN_IO_FILE,
	bCLOSE,
	bFINISH_OUTPUT,
	// other streams
	bCREATE_STRING_INPUT_STREAM,
	bCREATE_STRING_OUTPUT_STREAM,
	bGET_OUTPUT_STREAM_STRING,
	// input and output
	// character I/O
	bREAD,
	bREAD_CHAR,
	bPREVIEW_CHAR,
	bREAD_LINE,
	bSTREAM_READY_P,
	bFORMAT,
	bFORMAT_CHAR,
	bFORMAT_FLOAT,
	bFORMAT_FRESH_LINE,
	bFORMAT_INTEGER,
	bFORMAT_OBJECT,
	bFORMAT_TAB,
	// binary I/O
	bREAD_BYTE,
	bWRITE_BYTE,
	// files
	bPROBE_FILE,
	bFILE_POSITION,
	bSET_FILE_POSITION,
	bFILE_LENGTH,
	// condition system
	// signal and handle conditions
	bERROR,
	bCERROR,
	bSIGNAL_CONDITION,
	bCONDITION_CONTINUABLE,
	// data associated with condition classes
	bARITHMETIC_ERROR_OPERATION,
	bARITHMETIC_ERROR_OPERANDS,
	bDOMAIN_ERROR_OBJECT,
	bDOMAIN_ERROR_EXPECTED_CLASS,
	bPARSE_ERROR_STRING,
	bPARSE_ERROR_EXPECTED_CLASS,
	bSIMPLE_ERROR_FORMAT_STRING,
	bSIMPLE_ERROR_FORMAT_ARGUMENTS,
	bSTREAM_ERROR_STREAM,
	bUNDEFINED_ENTITY_NAME,
	bUNDEFINED_ENTITY_NAMESPACE,
	// miscellaneous
	bIDENTITY,
	bGET_UNIVERSAL_TIME,
	bGET_INTERNAL_RUN_TIME,
	bGET_INTERNAL_REAL_TIME,
	bINTERNAL_TIME_UNITS_PER_SECOND,
	// depend on TISL
	bSYSTEM,
	bEXIT,
	bSTRFTIME,
	bGET_ARGUMENT,
	bGET_ENVIRONMENT,
	bEVAL,
	// end
	NUMBER_OF_PRIMITIVE_OPERATOR,
};

///////////////////////////////////////
// 関数適用汎用
// evaluator.cで定義
VM_RET function_application_form(tPVM vm, tPOBJECT function, tPCELL name, const tINT anum);

///////////////////////////////////////
// primitive_operation.cで定義

VM_RET po_functionp(tPVM vm, const tINT anum);
VM_RET po_apply(tPVM vm, const tINT anum);
VM_RET po_funcall(tPVM vm, const tINT anum);
VM_RET po_eq(tPVM vm, const tINT anum);
VM_RET po_eql(tPVM vm, const tINT anum);
VM_RET po_equal(tPVM vm, const tINT anum);
VM_RET po_not(tPVM vm, const tINT anum);
VM_RET po_generic_function_p(tPVM vm, const tINT anum);
VM_RET po_class_of(tPVM vm, const tINT anum);
VM_RET po_instancep(tPVM vm, const tINT anum);
VM_RET po_subclassp(tPVM vm, const tINT anum);
VM_RET po_symbolp(tPVM vm, const tINT anum);
VM_RET po_property(tPVM vm, const tINT anum);
VM_RET po_set_property(tPVM vm, const tINT anum);
VM_RET po_remove_property(tPVM vm, const tINT anum);
VM_RET po_gensym(tPVM vm, const tINT anum);
VM_RET po_numberp(tPVM vm, const tINT anum);
VM_RET po_parse_number(tPVM vm, const tINT anum);
VM_RET po_number_equal(tPVM vm, const tINT anum);
VM_RET po_number_not_equal(tPVM vm, const tINT anum);
VM_RET po_number_ge(tPVM vm, const tINT anum);
VM_RET po_number_le(tPVM vm, const tINT anum);
VM_RET po_number_greater(tPVM vm, const tINT anum);
VM_RET po_number_less(tPVM vm, const tINT anum);
VM_RET po_addition(tPVM vm, const tINT anum);
VM_RET po_multiplication(tPVM vm, const tINT anum);
VM_RET po_subtraction(tPVM vm, const tINT anum);
VM_RET po_quotient(tPVM vm, const tINT anum);
VM_RET po_reciprocal(tPVM vm, const tINT anum);
VM_RET po_max(tPVM vm, const tINT anum);
VM_RET po_min(tPVM vm, const tINT anum);
VM_RET po_abs(tPVM vm, const tINT anum);
VM_RET po_exp(tPVM vm, const tINT anum);
VM_RET po_log(tPVM vm, const tINT anum);
VM_RET po_expt(tPVM vm, const tINT anum);
VM_RET po_sqrt(tPVM vm, const tINT anum);
VM_RET po_sin(tPVM vm, const tINT anum);
VM_RET po_cos(tPVM vm, const tINT anum);
VM_RET po_tan(tPVM vm, const tINT anum);
VM_RET po_atan(tPVM vm, const tINT anum);
VM_RET po_atan2(tPVM vm, const tINT anum);
VM_RET po_sinh(tPVM vm, const tINT anum);
VM_RET po_cosh(tPVM vm, const tINT anum);
VM_RET po_tanh(tPVM vm, const tINT anum);
VM_RET po_atanh(tPVM vm, const tINT anum);
VM_RET po_floatp(tPVM vm, const tINT anum);
VM_RET po_float(tPVM vm, const tINT anum);
VM_RET po_floor(tPVM vm, const tINT anum);
VM_RET po_ceiling(tPVM vm, const tINT anum);
VM_RET po_truncate(tPVM vm, const tINT anum);
VM_RET po_round(tPVM vm, const tINT anum);
VM_RET po_integerp(tPVM vm, const tINT anum);
VM_RET po_div(tPVM vm, const tINT anum);
VM_RET po_mod(tPVM vm, const tINT anum);
VM_RET po_gcd(tPVM vm, const tINT anum);
VM_RET po_lcm(tPVM vm, const tINT anum);
VM_RET po_isqrt(tPVM vm, const tINT anum);
VM_RET po_characterp(tPVM vm, const tINT anum);
VM_RET po_char_euqal(tPVM vm, const tINT anum);
VM_RET po_char_not_equal(tPVM vm, const tINT anum);
VM_RET po_char_less(tPVM vm, const tINT anum);
VM_RET po_char_greater(tPVM vm, const tINT anum);
VM_RET po_char_le(tPVM vm, const tINT anum);
VM_RET po_char_ge(tPVM vm, const tINT anum);
VM_RET po_consp(tPVM vm, const tINT anum);
VM_RET po_cons(tPVM vm, const tINT anum);
VM_RET po_car(tPVM vm, const tINT anum);
VM_RET po_cdr(tPVM vm, const tINT anum);
VM_RET po_set_car(tPVM vm, const tINT anum);
VM_RET po_set_cdr(tPVM vm, const tINT anum);
VM_RET po_null(tPVM vm, const tINT anum);
VM_RET po_listp(tPVM vm, const tINT anum);
VM_RET po_create_list(tPVM vm, const tINT anum);
VM_RET po_list(tPVM vm, const tINT anum);
VM_RET po_reverse(tPVM vm, const tINT anum);
VM_RET po_nreverse(tPVM vm, const tINT anum);
VM_RET po_append(tPVM vm, const tINT anum);
VM_RET po_member(tPVM vm, const tINT anum);
VM_RET po_mapcar(tPVM vm, const tINT anum);
VM_RET po_mapc(tPVM vm, const tINT anum);
VM_RET po_mapcan(tPVM vm, const tINT anum);
VM_RET po_maplist(tPVM vm, const tINT anum);
VM_RET po_mapl(tPVM vm, const tINT anum);
VM_RET po_mapcon(tPVM vm, const tINT anum);
VM_RET po_assoc(tPVM vm, const tINT anum);
VM_RET po_basic_array_p(tPVM vm, const tINT anum);
VM_RET po_basic_array_a_p(tPVM vm, const tINT anum);
VM_RET po_general_array_a_p(tPVM vm, const tINT anum);
VM_RET po_create_array(tPVM vm, const tINT anum);
VM_RET po_aref(tPVM vm, const tINT anum);
VM_RET po_garef(tPVM vm, const tINT anum);
VM_RET po_set_aref(tPVM vm, const tINT anum);
VM_RET po_set_garef(tPVM vm, const tINT anum);
VM_RET po_array_dimensions(tPVM vm, const tINT anum);
VM_RET po_basic_vector_p(tPVM vm, const tINT anum);
VM_RET po_general_vector_p(tPVM vm, const tINT anum);
VM_RET po_create_vector(tPVM vm, const tINT anum);
VM_RET po_vector(tPVM vm, const tINT anum);
VM_RET po_stringp(tPVM vm, const tINT anum);
VM_RET po_create_string(tPVM vm, const tINT anum);
VM_RET po_string_equal(tPVM vm, const tINT anum);
VM_RET po_string_not_equal(tPVM vm, const tINT anum);
VM_RET po_string_less(tPVM vm, const tINT anum);
VM_RET po_string_greater(tPVM vm, const tINT anum);
VM_RET po_string_ge(tPVM vm, const tINT anum);
VM_RET po_string_le(tPVM vm, const tINT anum);
VM_RET po_char_index(tPVM vm, const tINT anum);
VM_RET po_string_index(tPVM vm, const tINT anum);
VM_RET po_string_append(tPVM vm, const tINT anum);
VM_RET po_length(tPVM vm, const tINT anum);
VM_RET po_elt(tPVM vm, const tINT anum);
VM_RET po_set_elt(tPVM vm, const tINT anum);
VM_RET po_subseq(tPVM vm, const tINT anum);
VM_RET po_map_into(tPVM vm, const tINT anum);
VM_RET po_streamp(tPVM vm, const tINT anum);
VM_RET po_open_stream_p(tPVM vm, const tINT anum);
VM_RET po_input_stream_p(tPVM vm, const tINT anum);
VM_RET po_output_stream_p(tPVM vm, const tINT anum);
VM_RET po_standard_input(tPVM vm, const tINT anum);
VM_RET po_standard_output(tPVM vm, const tINT anum);
VM_RET po_error_output(tPVM vm, const tINT anum);
VM_RET po_open_input_file(tPVM vm, const tINT anum);
VM_RET po_open_output_file(tPVM vm, const tINT anum);
VM_RET po_open_io_file(tPVM vm, const tINT anum);
VM_RET po_close(tPVM vm, const tINT anum);
VM_RET po_finish_output(tPVM vm, const tINT anum);
VM_RET po_create_string_input_stream(tPVM vm, const tINT anum);
VM_RET po_create_string_output_stream(tPVM vm, const tINT anum);
VM_RET po_get_output_stream_string(tPVM vm, const tINT anum);
VM_RET po_read(tPVM vm, const tINT anum);
VM_RET po_read_char(tPVM vm, const tINT anum);
VM_RET po_preview_char(tPVM vm, const tINT anum);
VM_RET po_read_line(tPVM vm, const tINT anum);
VM_RET po_stream_ready_p(tPVM vm, const tINT anum);
VM_RET po_format(tPVM vm, const tINT anum);
VM_RET po_format_char(tPVM vm, const tINT anum);
VM_RET po_format_float(tPVM vm, const tINT anum);
VM_RET po_format_fresh_line(tPVM vm, const tINT anum);
VM_RET po_format_integer(tPVM vm, const tINT anum);
VM_RET po_format_object(tPVM vm, const tINT anum);
VM_RET po_format_tab(tPVM vm, const tINT anum);
VM_RET po_read_byte(tPVM vm, const tINT anum);
VM_RET po_write_byte(tPVM vm, const tINT anum);
VM_RET po_probe_file(tPVM vm, const tINT anum);
VM_RET po_file_position(tPVM vm, const tINT anum);
VM_RET po_set_file_position(tPVM vm, const tINT anum);
VM_RET po_file_length(tPVM vm, const tINT anum);
VM_RET po_error(tPVM vm, const tINT anum);
VM_RET po_cerror(tPVM vm, const tINT anum);
VM_RET po_signal_condition(tPVM vm, const tINT anum);
VM_RET po_condition_continuable(tPVM vm, const tINT anum);
VM_RET po_arithmetic_error_operation(tPVM vm, const tINT anum);
VM_RET po_arithmetic_error_operand(tPVM vm, const tINT anum);
VM_RET po_domain_error_object(tPVM vm, const tINT anum);
VM_RET po_domain_error_expected_class(tPVM vm, const tINT anum);
VM_RET po_parse_error_string(tPVM vm, const tINT anum);
VM_RET po_parse_error_expected_class(tPVM vm, const tINT anum);
VM_RET po_simple_error_format_string(tPVM vm, const tINT anum);
VM_RET po_simple_error_format_arguments(tPVM vm, const tINT anum);
VM_RET po_stream_error_stream(tPVM vm, const tINT anum);
VM_RET po_undefined_entity_name(tPVM vm, const tINT anum);
VM_RET po_undefined_entity_namespace(tPVM vm, const tINT anum);
VM_RET po_identity(tPVM vm, const tINT anum);
VM_RET po_get_universal_time(tPVM vm, const tINT anum);
VM_RET po_get_internal_run_time(tPVM vm, const tINT anum);
VM_RET po_get_internal_real_time(tPVM vm, const tINT anum);
VM_RET po_get_internal_time_units_per_second(tPVM vm, const tINT anum);
VM_RET po_system(tPVM vm, const tINT anum);
VM_RET po_exit(tPVM vm, const tINT anum);
VM_RET po_strftime(tPVM vm, const tINT anum);
VM_RET po_get_argument(tPVM vm, const tINT anum);
VM_RET po_get_environment(tPVM vm, const tINT anum);
VM_RET po_eval(tPVM vm, const tINT anum);

///////////////////////////////////////

#ifdef TISL_PRIMITIVE_OPERATION_PNUM_TABLE
const tINT primitive_operation_pnum_table[]={
	// function
	1,	//	bFUNCTIONP,
	-3,	//	bAPPLY,
	-2,	//	bFUNCALL,
	// equality
	2,	//	bEQ,
	2,	//	bEQL,
	2,	//	bEQUAL,
	1,	//	bNOT,
	// generic function
	1,	//	bGENERIC_FUNCTION_P,
	// class
	1,	//	bCLASS_OF,
	2,	//	bINSTANCEP,
	2,	//	bSUBCLASSP,
	// <symbol>
	1,	//	bSYMBOLP,
	3,	//	bPROPERTY, (property symbol property-name [obj])
	3,	//	bSET_PROPERTY,
	2,	//	bREMOVE_PROPERTY,
	0,	//	bGENSYM,
	// <number>
	1,	//	bNUMBERP,
	1,	//	bPARSE_NUMBER,
	2,	//	bNUMBER_EQUAL,
	2,	//	bNUMBER_NOT_EQUAL,
	2,	//	bNUMBER_GE,
	2,	//	bNUMBER_LE,
	2,	//	bNUMBER_GREATER,
	2,	//	bNUMBER_LESS,
	-1,	//	bADDITION,
	-1,	//	bMULTIPLICATION,
	-2,	//	bSUBTRACTION,
	-3,	//	bQUOTIENT,
	1,	//	bRECIPROCAL,
	-2,	//	bMAX,
	-2,	//	bMIN,
	1,	//	bABS,
	1,	//	bEXP,
	1,	//	bLOG,
	2,	//	bEXPT,
	1,	//	bSQRT,
	1,	//	bSIN,
	1,	//	bCOS,
	1,	//	bTAN,
	1,	//	bATAN,
	2,	//	bATAN2,
	1,	//	bSINH,
	1,	//	bCOSH,
	1,	//	bTANH,
	1,	//	bATANH,
	// <float>
	1,	//	bFLOATP,
	1,	//	bFLOAT,
	1,	//	bFLOOR,
	1,	//	bCEILING,
	1,	//	bTRUNCATE,
	1,	//	bROUND,
	// <integer>
	1,	//	bINTEGERP,
	2,	//	bDIV,
	2,	//	bMOD,
	2,	//	bGCD,
	2,	//	bLCM,
	1,	//	bISQRT,
	// <character>
	1,	//	bCHARACTERP,
	2,	//	bCHAR_EQUAL,
	2,	//	bCHAR_NOT_EQUAL,
	2,	//	bCHAR_LESS,
	2,	//	bCHAR_GREATER,
	2,	//	bCHAR_LE,
	2,	//	bCHAR_GE,
	// <list>
	// <cons>
	1,	//	bCONSP,
	2,	//	bCONS,
	1,	//	bCAR,
	1,	//	bCDR,
	2,	//	bSET_CAR,
	2,	//	bSET_CDR,
	// <null>
	1,	//	bNULL,
	// list operations
	1,	//	bLISTP,
	2,	//	bCREATE_LIST,	(create-list i [initiali-element])
	-1,	//	bLIST,
	1,	//	bREVERSE,
	1,	//	bNREVERSE,
	-1,	//	bAPPEND,
	2,	//	bMEMBER,
	-3,	//	bMAPCAR,
	-3,	//	bMAPC,
	-3,	//	bMAPCAN,
	-3,	//	bMAPLIST,
	-3,	//	bMAPL,
	-3,	//	bMAPCON,
	2,	//	bASSOC,
	// array
	// array oprations
	1,	//	bBASIC_ARRAY_P,
	1,	//	bBASIC_ARRAY_A_P,
	1,	//	bGENERAL_ARRAY_A_P,
	2,	//	bCREATE_ARRAY,
	-2,	//	bAREF,
	-2,	//	bGAREF,
	-3,	//	bSET_AREF,
	-3,	//	bSET_GAREF,
	1,	//	bARRAY_DIMENSIONS,
	// vectors
	1,	//	bBASIC_VETOR_P,
	1,	//	bGENERAL_VECTOR_P,
	2,	//	bCREATE_VECTOR,
	-1,	//	bVECTOR,
	// <string>
	1,	//	bSTRINGP,
	2,	//	bCREATE_STRING,
	2,	//	bSTRING_EQUAL,
	2,	//	bSTRING_NOT_EQUAL,
	2,	//	bSTRING_LESS,
	2,	//	bSTRING_GREATER,
	2,	//	bSTRING_GE,
	2,	//	bSTRING_LE,
	3,	//	bCHAR_INDEX,
	3,	//	bSTRING_INDEX,
	-1,	//	bSTRING_APPEND,
	// sequence functions
	1,	//	bLENGTH,
	2,	//	bELT,
	3,	//	bSET_ELT,
	3,	//	bSUBSEQ,
	-3,	//	bMAP_INTO,
	// <stream>
	1,	//	bSTREAMP,
	1,	//	bOPEN_STREAM_P,
	1,	//	bINPUT_STREAM_P,
	1,	//	bOUTPUT_STREAM_P,
	0,	//	bSTANDARD_INPUT,
	0,	//	bSTANDARD_OUTPUT,
	0,	//	bERROR_OUTPUT,
	// streams to files
	2,	//	bOPEN_INPUT_FILE,
	2,	//	bOPEN_OUTPUT_FILE,
	2,	//	bOPEN_IO_FILE,
	1,	//	bCLOSE,
	1,	//	bFINISH_OUTPUT,
	// other streams
	1,	//	bCREATE_STRING_INPUT_STREAM,
	0,	//	bCREATE_STRING_OUTPUT_STREAM,
	1,	//	bGET_OUTPUT_STREAM_STRING,
	// input and output
	// character I/O
	3,	//	bREAD,
	3,	//	bREAD_CHAR,
	3,	//	bPREVIEW_CHAR,
	3,	//	bREAD_LINE,
	3,	//	bSTREAM_READY_P,
	-3,	//	bFORMAT,
	2,	//	bFORMAT_CHAR,
	2,	//	bFORMAT_FLOAT,
	1,	//	bFORMAT_FRESH_LINE,
	3,	//	bFORMAT_INTEGER,
	3,	//	bFORMAT_OBJECT,
	2,	//	bFORMAT_TAB,
	// binary I/O
	3,	//	bREAD_BYTE,
	2,	//	bWRITE_BYTE,
	// files
	1,	//	bPROBE_FILE,
	1,	//	bFILE_POSITION,
	2,	//	bSET_FILE_POSITION,
	2,	//	bFILE_LENGTH,
	// condition system
	// signal and handle conditions
	-2,	//	bERROR,
	-3,	//	bCERROR,
	2,	//	bSIGNAL_CONDITION,
	1,	//	bCONDITION_CONTINUABLE,
	// data associated with condition classes
	1,	//	bARITHMETIC_ERROR_OPERATION,
	1,	//	bARITHMETIC_ERROR_OPERANDS,
	1,	//	bDOMAIN_ERROR_OBJECT,
	1,	//	bDOMAIN_ERROR_EXPECTED_CLASS,
	1,	//	bPARSE_ERROR_STRING,
	1,	//	bPARSE_ERROR_EXPECTED_CLASS,
	1,	//	bSIMPLE_ERROR_FORMAT_STRING,
	1,	//	bSIMPLE_ERROR_FORMAT_ARGUMENTS,
	1,	//	bSTREAM_ERROR_STREAM,
	1,	//	bUNDEFINED_ENTITY_NAME,
	1,	//	bUNDEFINED_ENTITY_NAMESPACE,
	// miscellaneous
	1,	//	bIDENTITY,
	0,	//	bGET_UNIVERSAL_TIME,
	0,	//	bGET_INTERNAL_RUN_TIME,
	0,	//	bGET_INTERNAL_REAL_TIME,
	0,	//	bINTERNAL_TIME_UNITS_PER_SECOND,
	// depend on TISL
	1,	//	bSYSTEM,
	0,	//	bEXIT,
	-2,	//	bSTRFTIME,
	0,	//	bGET_ARGUMENT,
	0,	//	bGET_ENVIRONMENT,
	1,	//	bEVAL
	// end
	NUMBER_OF_PRIMITIVE_OPERATOR,
};
#endif

///////////////////////////////////////

#ifdef TISL_PRIMITIVE_OPERATION_TABLE
typedef VM_RET (*PRIMITIVE_OPERATION)(tPVM, const tINT);
typedef struct PO_PRIMITIVE_FUNCTION_ PO_PRIMITIVE_FUNCTION;
struct PO_PRIMITIVE_FUNCTION_ {
	PRIMITIVE_OPERATION	operation;
};

const PO_PRIMITIVE_FUNCTION primitive_operation_table[]={
	{ po_functionp },
	{ po_apply },
	{ po_funcall },
	{ po_eq },
	{ po_eql },
	{ po_equal },
	{ po_not },
	{ po_generic_function_p },
	{ po_class_of },
	{ po_instancep },
	{ po_subclassp },
	{ po_symbolp },
	{ po_property },
	{ po_set_property },
	{ po_remove_property },
	{ po_gensym },
	{ po_numberp },
	{ po_parse_number },
	{ po_number_equal },
	{ po_number_not_equal },
	{ po_number_ge },
	{ po_number_le },
	{ po_number_greater },
	{ po_number_less },
	{ po_addition },
	{ po_multiplication },
	{ po_subtraction },
	{ po_quotient },
	{ po_reciprocal },
	{ po_max },
	{ po_min },
	{ po_abs },
	{ po_exp },
	{ po_log },
	{ po_expt },
	{ po_sqrt },
	{ po_sin },
	{ po_cos },
	{ po_tan },
	{ po_atan },
	{ po_atan2 },
	{ po_sinh },
	{ po_cosh },
	{ po_tanh },
	{ po_atanh },
	{ po_floatp },
	{ po_float },
	{ po_floor },
	{ po_ceiling },
	{ po_truncate },
	{ po_round },
	{ po_integerp },
	{ po_div },
	{ po_mod },
	{ po_gcd },
	{ po_lcm },
	{ po_isqrt },
	{ po_characterp },
	{ po_char_euqal },
	{ po_char_not_equal },
	{ po_char_less },
	{ po_char_greater },
	{ po_char_le },
	{ po_char_ge },
	{ po_consp },
	{ po_cons },
	{ po_car },
	{ po_cdr },
	{ po_set_car },
	{ po_set_cdr },
	{ po_null },
	{ po_listp },
	{ po_create_list },
	{ po_list },
	{ po_reverse },
	{ po_nreverse },
	{ po_append },
	{ po_member },
	{ po_mapcar },
	{ po_mapc },
	{ po_mapcan },
	{ po_maplist },
	{ po_mapl },
	{ po_mapcon },
	{ po_assoc },
	{ po_basic_array_p },
	{ po_basic_array_a_p },
	{ po_general_array_a_p },
	{ po_create_array },
	{ po_aref },
	{ po_garef },
	{ po_set_aref },
	{ po_set_garef },
	{ po_array_dimensions },
	{ po_basic_vector_p },
	{ po_general_vector_p },
	{ po_create_vector },
	{ po_vector },
	{ po_stringp },
	{ po_create_string },
	{ po_string_equal },
	{ po_string_not_equal },
	{ po_string_less },
	{ po_string_greater },
	{ po_string_ge },
	{ po_string_le },
	{ po_char_index },
	{ po_string_index },
	{ po_string_append },
	{ po_length },
	{ po_elt },
	{ po_set_elt },
	{ po_subseq },
	{ po_map_into },
	{ po_streamp },
	{ po_open_stream_p },
	{ po_input_stream_p },
	{ po_output_stream_p },
	{ po_standard_input },
	{ po_standard_output },
	{ po_error_output },
	{ po_open_input_file },
	{ po_open_output_file },
	{ po_open_io_file },
	{ po_close },
	{ po_finish_output },
	{ po_create_string_input_stream },
	{ po_create_string_output_stream },
	{ po_get_output_stream_string },
	{ po_read },
	{ po_read_char },
	{ po_preview_char },
	{ po_read_line },
	{ po_stream_ready_p },
	{ po_format },
	{ po_format_char },
	{ po_format_float },
	{ po_format_fresh_line },
	{ po_format_integer },
	{ po_format_object },
	{ po_format_tab },
	{ po_read_byte },
	{ po_write_byte },
	{ po_probe_file },
	{ po_file_position },
	{ po_set_file_position },
	{ po_file_length },
	{ po_error },
	{ po_cerror },
	{ po_signal_condition },
	{ po_condition_continuable },
	{ po_arithmetic_error_operation },
	{ po_arithmetic_error_operand },
	{ po_domain_error_object },
	{ po_domain_error_expected_class },
	{ po_parse_error_string },
	{ po_parse_error_expected_class },
	{ po_simple_error_format_string },
	{ po_simple_error_format_arguments },
	{ po_stream_error_stream },
	{ po_undefined_entity_name },
	{ po_undefined_entity_namespace },
	{ po_identity },
	{ po_get_universal_time },
	{ po_get_internal_run_time },
	{ po_get_internal_real_time },
	{ po_get_internal_time_units_per_second },
	{ po_system },
	{ po_exit },
	{ po_strftime },
	{ po_get_argument },
	{ po_get_environment },
	{ po_eval },
};
#endif// #ifdef TISL_PRIMITIVE_OPERATION_TABLE

#ifdef TISL_PRIMITIVE_TRANSLATOR

#include "command.h"

///////////////////////////////////////

VM_RET t1_primitive(tPVM vm, const tINT code, const tINT anum, tPCELL clist);
VM_RET t1_set_property_elt(tPVM vm, const tINT code, const tINT anum, tPCELL clist);
VM_RET t1_set_car_cdr(tPVM vm, const tINT code, const tINT anum, tPCELL clist);

///////////////////////////////////////

typedef VM_RET (*T1_TRANSLATE)(tPVM, const tINT, const tINT, tPCELL);
typedef struct T1_PRIMITIVE_FUNCTION_ T1_PRIMITIVE_FUNCTION;
struct T1_PRIMITIVE_FUNCTION_ {
	T1_TRANSLATE			translate;
	tINT					t1_data;
};

const T1_PRIMITIVE_FUNCTION t1_primitive_table[]={
	{ t1_primitive,		iFUNCTIONP			 },
	{ t1_primitive,		iAPPLY				 },
	{ t1_primitive,		iFUNCALL			 },
	{ t1_primitive,		iEQ					 },
	{ t1_primitive,		iEQL				 },
	{ t1_primitive,		iEQUAL				 },
	{ t1_primitive,		iNOT				 },
	{ t1_primitive,		iGENERIC_FUNCTION_P  },
	{ t1_primitive,		iCLASS_OF			 },
	{ t1_primitive,		iINSTANCEP			 },
	{ t1_primitive,		iSUBCLASSP			 },
	{ t1_primitive,		iSYMBOLP			 },
	{ t1_primitive,		iPROPERTY			 },
	{ t1_set_property_elt,	iSET_PROPERTY		 },
	{ t1_primitive,		iREMOVE_PROPERTY	 },
	{ t1_primitive,		iGENSYM				 },
	{ t1_primitive,		iNUMBERP			 },
	{ t1_primitive,		iPARSE_NUMBER		 },
	{ t1_primitive,		iNUMBER_EQUAL		 },
	{ t1_primitive,		iNUMBER_NOT_EQUAL	 },
	{ t1_primitive,		iNUMBER_GE			 },
	{ t1_primitive,		iNUMBER_LE			 },
	{ t1_primitive,		iNUMBER_GREATER		 },
	{ t1_primitive,		iNUMBER_LESS		 },
	{ t1_primitive,		iADDITION			 },
	{ t1_primitive,		iMULTIPLICATION		 },
	{ t1_primitive,		iSUBSTRACTION		 },
	{ t1_primitive,		iQUOTIENT			 },
	{ t1_primitive,		iRECIPROCAL			 },
	{ t1_primitive,		iMAX				 },
	{ t1_primitive,		iMIN				 },
	{ t1_primitive,		iABS				 },
	{ t1_primitive,		iEXP				 },
	{ t1_primitive,		iLOG				 },
	{ t1_primitive,		iEXPT				 },
	{ t1_primitive,		iSQRT				 },
	{ t1_primitive,		iSIN				 },
	{ t1_primitive,		iCOS				 },
	{ t1_primitive,		iTAN				 },
	{ t1_primitive,		iATAN				 },
	{ t1_primitive,		iATAN2				 },
	{ t1_primitive,		iSINH				 },
	{ t1_primitive,		iCOSH				 },
	{ t1_primitive,		iTANH				 },
	{ t1_primitive,		iATANH				 },
	{ t1_primitive,		iFLOATP				 },
	{ t1_primitive,		iFLOAT				 },
	{ t1_primitive,		iFLOOR				 },
	{ t1_primitive,		iCEILING			 },
	{ t1_primitive,		iTRUNCATE			 },
	{ t1_primitive,		iROUND				 },
	{ t1_primitive,		iINTEGERP			 },
	{ t1_primitive,		iDIV				 },
	{ t1_primitive,		iMOD				 },
	{ t1_primitive,		iGCD				 },
	{ t1_primitive,		iLCM				 },
	{ t1_primitive,		iISQRT				 },
	{ t1_primitive,		iCHARACTERP			 },
	{ t1_primitive,		iCHAR_EQUAL			 },
	{ t1_primitive,		iCHAR_NOT_EQUAL		 },
	{ t1_primitive,		iCHAR_LESS			 },
	{ t1_primitive,		iCHAR_GREATER		 },
	{ t1_primitive,		iCHAR_LE			 },
	{ t1_primitive,		iCHAR_GE			 },
	{ t1_primitive,		iCONSP				 },
	{ t1_primitive,		iCONS				 },
	{ t1_primitive,		iCAR				 },
	{ t1_primitive,		iCDR				 },
	{ t1_set_car_cdr,	iSET_CAR			 },
	{ t1_set_car_cdr,	iSET_CDR			 },
	{ t1_primitive,		iNULL				 },
	{ t1_primitive,		iLISTP				 },
	{ t1_primitive,		iCREATE_LIST		 },
	{ t1_primitive,		iLIST				 },
	{ t1_primitive,		iREVERSE			 },
	{ t1_primitive,		iNREVERSE			 },
	{ t1_primitive,		iAPPEND				 },
	{ t1_primitive,		iMEMBER				 },
	{ t1_primitive,		iMAPCAR				 },
	{ t1_primitive,		iMAPC				 },
	{ t1_primitive,		iMAPCAN				 },
	{ t1_primitive,		iMAPLIST			 },
	{ t1_primitive,		iMAPL				 },
	{ t1_primitive,		iMAPCON				 },
	{ t1_primitive,		iASSOC				 },
	{ t1_primitive,		iBASIC_ARRAY_P		 },
	{ t1_primitive,		iBASIC_ARRAY_A_P	 },
	{ t1_primitive,		iGENERAL_ARRAY_A_P	 },
	{ t1_primitive,		iCREATE_ARRAY		 },
	{ t1_primitive,		iAREF				 },
	{ t1_primitive,		iGAREF				 },
	{ t1_primitive,		iSET_AREF			 },
	{ t1_primitive,		iSET_GAREF			 },
	{ t1_primitive,		iARRAY_DIMENSIONS	 },
	{ t1_primitive,		iBASIC_VECTOR_P		 },
	{ t1_primitive,		iGENERAL_VECTOR_P	 },
	{ t1_primitive,		iCREATE_VECTOR		 },
	{ t1_primitive,		iVECTOR				 },
	{ t1_primitive,		iSTRINGP			 },
	{ t1_primitive,		iCREATE_STRING		 },
	{ t1_primitive,		iSTRING_EQUAL		 },
	{ t1_primitive,		iSTRING_NOT_EQUAL	 },
	{ t1_primitive,		iSTRING_LESS		 },
	{ t1_primitive,		iSTRING_GREATER		 },
	{ t1_primitive,		iSTRING_GE			 },
	{ t1_primitive,		iSTRING_LE			 },
	{ t1_primitive,		iCHAR_INDEX			 },
	{ t1_primitive,		iSTRING_INDEX		 },
	{ t1_primitive,		iSTRING_APPEND		 },
	{ t1_primitive,		iLENGTH				 },
	{ t1_primitive,		iELT				 },
	{ t1_set_property_elt,	iSET_ELT			 },
	{ t1_primitive,		iSUBSEQ				 },
	{ t1_primitive,		iMAP_INTO			 },
	{ t1_primitive,		iSTREAMP			 },
	{ t1_primitive,		iOPEN_STREAM_P		 },
	{ t1_primitive,		iINPUT_STREAM_P		 },
	{ t1_primitive,		iOUTPUT_STREAM_P	 },
	{ t1_primitive,		iSTANDARD_INPUT		 },
	{ t1_primitive,		iSTANDARD_OUTPUT	 },
	{ t1_primitive,		iERROR_OUTPUT		 },
	{ t1_primitive,		iOPEN_INPUT_FILE	 },
	{ t1_primitive,		iOPEN_OUTPUT_FILE	 },
	{ t1_primitive,		iOPEN_IO_FILE		 },
	{ t1_primitive,		iCLOSE				 },
	{ t1_primitive,		iFINISH_OUTPUT		 },
	{ t1_primitive,	iCREATE_STRING_INPUT_STREAM		 },
	{ t1_primitive,	iCREATE_STRING_OUTPUT_STREAM	 },
	{ t1_primitive,	iGET_OUTPUT_STREAM_STRING		 },
	{ t1_primitive,		iREAD				 },
	{ t1_primitive,		iREAD_CHAR			 },
	{ t1_primitive,		iPREVIEW_CHAR		 },
	{ t1_primitive,		iREAD_LINE			 },
	{ t1_primitive,		iSTREAM_READY_P		 },
	{ t1_primitive,		iFORMAT				 },
	{ t1_primitive,		iFORMAT_CHAR		 },
	{ t1_primitive,		iFORMAT_FLOAT		 },
	{ t1_primitive,		iFORMAT_FRESH_LINE	 },
	{ t1_primitive,		iFORMAT_INTEGER		 },
	{ t1_primitive,		iFORMAT_OBJECT		 },
	{ t1_primitive,		iFORMAT_TAB			 },
	{ t1_primitive,		iREAD_BYTE			 },
	{ t1_primitive,		iWRITE_BYTE			 },
	{ t1_primitive,		iPROBE_FILE			 },
	{ t1_primitive,		iFILE_POSITION		 },
	{ t1_primitive,		iSET_FILE_POSITION	 },
	{ t1_primitive,		iFILE_LENGTH		 },
	{ t1_primitive,		iERROR				 },
	{ t1_primitive,		iCERROR				 },
	{ t1_primitive,		iSIGNAL_CONDITION	 },
	{ t1_primitive,	iCONDITION_CONTINUABLE				 },
	{ t1_primitive,	iARITHMETIC_ERROR_OPERATION			 },
	{ t1_primitive,	iARITHMETIC_ERROR_OPERAND			 },
	{ t1_primitive,	iDOMAIN_ERROR_OBJECT				 },
	{ t1_primitive,	iDOMAIN_ERROR_EXPECTED_CLASS		 },
	{ t1_primitive,	iPARSE_ERROR_STRING					 },
	{ t1_primitive,	iPARSE_ERROR_EXPECTED_CLASS			 },
	{ t1_primitive,	iSIMPLE_ERROR_FORMAT_STRING			 },
	{ t1_primitive,	iSIMPLE_ERROR_FORMAT_ARGUMENTS		 },
	{ t1_primitive,	iSTREAM_ERROR_STREAM				 },
	{ t1_primitive,	iUNDEFINED_ENTITY_NAME				 },
	{ t1_primitive,	iUNDEFINED_ENTITY_NAMESPACE			 },
	{ t1_primitive,	iIDENTITY							 },
	{ t1_primitive,	iGET_UNIVERSAL_TIME					 },
	{ t1_primitive,	iGET_INTERNAL_RUN_TIME				 },
	{ t1_primitive,	iGET_INTERNAL_REAL_TIME				 },
	{ t1_primitive,	iGET_INTERNAL_TIME_UNITS_PER_SECOND	 },
	{ t1_primitive,	iSYSTEM								 },
	{ t1_primitive,	iEXIT								 },
	{ t1_primitive,	iSTRFTIME							 },
	{ t1_primitive,	iGET_ARGUMENT						 },
	{ t1_primitive,	iGET_ENVIRONMENT					 },
	{ t1_primitive,	iEVAL								 },
};
#endif

///////////////////////////////////////
// object

extern tOBJECT nil;
extern tOBJECT unbound;

// string

extern tPCELL string_plus;
extern tPCELL string_minus;
extern tPCELL string_system;
extern tPCELL string_islisp;
extern tPCELL string_continue_condition;
extern tPCELL string_simple_error;
extern tPCELL string_system_error;
extern tPCELL string_stack_overflow;
extern tPCELL string_storage_exhausted;

// standard class
extern tPCELL sclass_standard_object;

// list
extern tPCELL list_object_instance;
extern tPCELL list_islisp_system;

// gfunction
extern tPCELL gfunction_initialize_object;

// symbol

enum {
	sPLUS,
	sMINUS,
	sONE_PLUS,
	sONE_MINUS,
	sVARIABLE,
	sDYNAMIC_VARIABLE,
	sPACKAGE,
	//
	sAND,
	sASSURE,
	sBLOCK,
	sCASE,
	sCASE_USING,
	sCATCH,
	sCLASS,
	sCOND,
	sCONVERT,
	sDYNAMIC,
	sDYNAMIC_LET,
	sFLET,
	sFOR,
	sFUNCTION,
	sGO,
	sIF,
	sIGNORE_ERRORS,
	sLABELS,
	sLAMBDA,
	sLET,
	sLET_A,
	sOR,
	sPROGN,
	sQUOTE,
	sRETURN_FROM,
	sSETF,
	sSETQ,
	sTAGBODY,
	sTHE,
	sTHROW,
	sUNWIND_PROTECT,
	sWHILE,
	sWITH_ERROR_OUTPUT,
	sWITH_HANDLER,
	sWITH_OPEN_INPUT_FILE,
	sWITH_OPEN_IO_FILE,
	sWITH_OPEN_OUTPUT_FILE,
	sWITH_STANDARD_INPUT,
	sWITH_STANDARD_OUTPUT,
	//
	sDEFCLASS,
	sDEFCONSTANT,
	sDEFDYNAMIC,
	sDEFGENERIC,
	sDEFGLOBAL,
	sDEFMACRO,
	sDEFMETHOD,
	sDEFUN,
	//
	sDEFPACKAGE,
	sDEFLINK,
	//
	sLOAD,
	sIN_PACKAGE,
	// 組み込み関数
	// function
	sFUNCTIONP,
	sAPPLY,
	sFUNCALL,
	// equality
	sEQ,
	sEQL,
	sEQUAL,
	sNOT,
	// generic function
	sGENERIC_FUNCTION_P,
	// class
	sCLASS_OF,
	sINSTANCEP,
	sSUBCLASSP,
	// <symsol>
	sSYMBOLP,
	sPROPERTY,
	sSET_PROPERTY,
	sREMOVE_PROPERTY,
	sGENSYM,
	// <numser>
	sNUMBERP,
	sPARSE_NUMBER,
	sNUMBER_EQUAL,
	sNUMBER_NOT_EQUAL,
	sNUMBER_GE,
	sNUMBER_LE,
	sNUMBER_GREATER,
	sNUMBER_LESS,
	sADDITION,
	sMULTIPLICATION,
	sSUBTRACTION,
	sQUOTIENT,
	sRECIPROCAL,
	sMAX,
	sMIN,
	sABS,
	sEXP,
	sLOG,
	sEXPT,
	sSQRT,
	sSIN,
	sCOS,
	sTAN,
	sATAN,
	sATAN2,
	sSINH,
	sCOSH,
	sTANH,
	sATANH,
	// <float>
	sFLOATP,
	sFLOAT,
	sFLOOR,
	sCEILING,
	sTRUNCATE,
	sROUND,
	// <integer>
	sINTEGERP,
	sDIV,
	sMOD,
	sGCD,
	sLCM,
	sISQRT,
	// <character>
	sCHARACTERP,
	sCHAR_EQUAL,
	sCHAR_NOT_EQUAL,
	sCHAR_LESS,
	sCHAR_GREATER,
	sCHAR_LE,
	sCHAR_GE,
	// <list>
	// <cons>
	sCONSP,
	sCONS,
	sCAR,
	sCDR,
	sSET_CAR,
	sSET_CDR,
	// <null>
	sNULL,
	// list operations
	sLISTP,
	sCREATE_LIST,
	sLIST,
	sREVERSE,
	sNREVERSE,
	sAPPEND,
	sMEMBER,
	sMAPCAR,
	sMAPC,
	sMAPCAN,
	sMAPLIST,
	sMAPL,
	sMAPCON,
	sASSOC,
	// array
	// array oprations
	sBASIC_ARRAY_P,
	sBASIC_ARRAY_A_P,
	sGENERAL_ARRAY_A_P,
	sCREATE_ARRAY,
	sAREF,
	sGAREF,
	sSET_AREF,
	sSET_GAREF,
	sARRAY_DIMENSIONS,
	// vectors
	sBASIC_VECTOR_P,
	sGENERAL_VECTOR_P,
	sCREATE_VECTOR,
	sVECTOR,
	// <string>
	sSTRINGP,
	sCREATE_STRING,
	sSTRING_EQUAL,
	sSTRING_NOT_EQUAL,
	sSTRING_LESS,
	sSTRING_GREATER,
	sSTRING_GE,
	sSTRING_LE,
	sCHAR_INDEX,
	sSTRING_INDEX,
	sSTRING_APPEND,
	// sequence functions
	sLENGTH,
	sELT,
	sSET_ELT,
	sSUBSEQ,
	sMAP_INTO,
	// <stream>
	sSTREAMP,
	sOPEN_STREAM_P,
	sINPUT_STREAM_P,
	sOUTPUT_STREAM_P,
	sSTANDARD_INPUT,
	sSTANDARD_OUTPUT,
	sERROR_OUTPUT,
	// streams to files
	sOPEN_INPUT_FILE,
	sOPEN_OUTPUT_FILE,
	sOPEN_IO_FILE,
	sCLOSE,
	sFINISH_OUTPUT,
	// other streams
	sCREATE_STRING_INPUT_STREAM,
	sCREATE_STRING_OUTPUT_STREAM,
	sGET_OUTPUT_STREAM_STRING,
	// input and output
	// character I/O
	sREAD,
	sREAD_CHAR,
	sPREVIEW_CHAR,
	sREAD_LINE,
	sSTREAM_READY_P,
	sFORMAT,
	sFORMAT_CHAR,
	sFORMAT_FLOAT,
	sFORMAT_FRESH_LINE,
	sFORMAT_INTEGER,
	sFORMAT_OBJECT,
	sFORMAT_TAB,
	// sinary I/O
	sREAD_BYTE,
	sWRITE_BYTE,
	// files
	sPROBE_FILE,
	sFILE_POSITION,
	sSET_FILE_POSITION,
	sFILE_LENGTH,
	// condition system
	// signal and handle conditions
	sERROR,
	sCERROR,
	sSIGNAL_CONDITION,
	sCONDITION_CONTINUABLE,
	// data associated with condition classes
	sARITHMETIC_ERROR_OPERATION,
	sARITHMETIC_ERROR_OPERANDS,
	sDOMAIN_ERROR_OBJECT,
	sDOMAIN_ERROR_EXPECTED_CLASS,
	sPARSE_ERROR_STRING,
	sPARSE_ERROR_EXPECTED_CLASS,
	sSIMPLE_ERROR_FORMAT_STRING,
	sSIMPLE_ERROR_FORMAT_ARGUMENTS,
	sSTREAM_ERROR_STREAM,
	sUNDEFINED_ENTITY_NAME,
	sUNDEFINED_ENTITY_NAMESPACE,
	// miscellaneous
	sIDENTITY,
	sGET_UNIVERSAL_TIME,
	sGET_INTERNAL_RUN_TIME,
	sGET_INTERNAL_REAL_TIME,
	sGET_INTERNAL_TIME_UNITS_PER_SECOND,
	// depend on TISL
	sSYSTEM,
	sEXIT,
	sSTRFTIME,
	sGET_ARGUMENT,
	sGET_ENVIRONMENT,
	sEVAL,
	//
	sT,
	sNIL,
	sSTANDARD,
	//
	sREST,
	sPUBLIC,
	sPRIVATE,
	sMETHOD,
	sMETHOD_COMBINATION,
	sGENERIC_FUNCTION_CLASS,
	sAROUND,
	sBEFORE,
	sAFTER,
	sREADER,
	sWRITER,
	sACCESSOR,
	sBOUNDP,
	sINITFORM,
	sINITARG,
	sMETACLASS,
	sABSTRACTP,
	sVOID,
	//
	sERROR_HANDLER,
	sOBJECT_CLASS,
	sCALL_NEXT_METHOD,
	sNEXT_METHOD_P,
	sOBJECT,
	sINSTANCE,
	sCONDITION,
	sSTANDARD_CLASS_CLASS,
	sSTANDARD_OBJECT_CLASS,
	sSERIOUS_CONDITION_CLASS,
	sLIST_CLASS,
	sSTREAM_CLASS,
	sSTREAM,
	sFOREIGN_CLASS_CLASS,
	//
	NUMBER_OF_GLOBAL_SYMBOL,
};

extern tPCELL global_symbol[NUMBER_OF_GLOBAL_SYMBOL];

#define SYMBOL_PLUS			(global_symbol[sPLUS])
#define SYMBOL_MINUS		(global_symbol[sMINUS])
#define SYMBOL_ONE_PLUS		(global_symbol[sONE_PLUS])
#define SYMBOL_ONE_MINUS	(global_symbol[sONE_MINUS])
#define SYMBOL_FUNCTION		(global_symbol[sFUNCTION])
#define SYMBOL_QUOTE		(global_symbol[sQUOTE])

#define SYMBOL_SETF			(global_symbol[sSETF])
#define SYMBOL_DYNAMIC		(global_symbol[sDYNAMIC])
#define SYMBOL_AREF			(global_symbol[sAREF])
#define SYMBOL_GAREF		(global_symbol[sGAREF])
#define SYMBOL_ELT			(global_symbol[sELT])
#define SYMBOL_PROPERTY		(global_symbol[sPROPERTY])
#define SYMBOL_CAR			(global_symbol[sCAR])
#define SYMBOL_CDR			(global_symbol[sCDR])
#define SYMBOL_T			(global_symbol[sT])
#define SYMBOL_NIL			(global_symbol[sNIL])
#define SYMBOL_STANDARD		(global_symbol[sSTANDARD])

#define KEYWORD_REST					(global_symbol[sREST])
#define KEYWORD_PUBLIC					(global_symbol[sPUBLIC])
#define KEYWORD_PRIVATE					(global_symbol[sPRIVATE])
#define KEYWORD_METHOD					(global_symbol[sMETHOD])
#define KEYWORD_METHOD_COMBINATION		(global_symbol[sMETHOD_COMBINATION])
#define KEYWORD_GENERIC_FUNCTION_CLASS	(global_symbol[sGENERIC_FUNCTION_CLASS])
#define KEYWORD_AROUND					(global_symbol[sAROUND])
#define KEYWORD_BEFORE					(global_symbol[sBEFORE])
#define KEYWORD_AFTER					(global_symbol[sAFTER])
#define KEYWORD_READER					(global_symbol[sREADER])
#define KEYWORD_WRITER					(global_symbol[sWRITER])
#define KEYWORD_ACCESSOR				(global_symbol[sACCESSOR])
#define KEYWORD_BOUNDP					(global_symbol[sBOUNDP])
#define KEYWORD_INITFORM				(global_symbol[sINITFORM])
#define KEYWORD_INITARG					(global_symbol[sINITARG])
#define KEYWORD_METACLASS				(global_symbol[sMETACLASS])
#define KEYWORD_ABSTRACTP				(global_symbol[sABSTRACTP])
#define KEYWORD_VOID					(global_symbol[sVOID])

#define SYMBOL_ERROR_HANDLER	(global_symbol[sERROR_HANDLER])
#define SYMBOL_FUNCALL			(global_symbol[sFUNCALL])
#define SYMBOL_OBJECT_CLASS		(global_symbol[sOBJECT_CLASS])
#define SYMBOL_CALL_NEXT_METHOD	(global_symbol[sCALL_NEXT_METHOD])
#define SYMBOL_NEXT_METHOD_P	(global_symbol[sNEXT_METHOD_P])
#define SYMBOL_OBJECT			(global_symbol[sOBJECT])
#define SYMBOL_INSTANCE			(global_symbol[sINSTANCE])
#define SYMBOL_CLASS			(global_symbol[sCLASS])
#define SYMBOL_LIST				(global_symbol[sLIST])
#define SYMBOL_CONDITION		(global_symbol[sCONDITION])
#define SYMBOL_STANDARD_CLASS_CLASS		(global_symbol[sSTANDARD_CLASS_CLASS])
#define SYMBOL_STANDARD_OBJECT_CLASS	(global_symbol[sSTANDARD_OBJECT_CLASS])
#define SYMBOL_SERIOUS_CONDITION_CLASS	(global_symbol[sSERIOUS_CONDITION_CLASS])
#define SYMBOL_LIST_CLASS				(global_symbol[sLIST_CLASS])
#define SYMBOL_STREAM_CLASS				(global_symbol[sSTREAM_CLASS])
#define SYMBOL_STREAM					(global_symbol[sSTREAM])
#define SYMBOL_FOREIGN_CLASS_CLASS		(global_symbol[sFOREIGN_CLASS_CLASS])

/////////////////////////////

#ifdef TISL_BUILT_IN_OBJECT_C

typedef struct GLOBAL_SYMBOL_TABLE_ GLOBAL_SYMBOL_TABLE;
struct GLOBAL_SYMBOL_TABLE_ {
	tCSTRING	string;
	tBOOL		complete;
	tINT		special_oepator_id;
};

const GLOBAL_SYMBOL_TABLE global_symbol_table[]={
	{ "+",						tFALSE, 0						 },
	{ "-",						tFALSE, 0						 },
	{ "1+",						tFALSE, 0						 },
	{ "1-",						tFALSE, 0						 },
	{ "variable",				tFALSE, 0						 },
	{ "dynamic-variable",		tFALSE, 0						 },
	{ "package",				tFALSE, 0						 },
	//
	{ "and",					tFALSE, bAND					 },
	{ "assure",					tFALSE, bASSURE					 },
	{ "block",					tFALSE, bBLOCK					 },
	{ "case",					tFALSE, bCASE					 },
	{ "case-using",				tFALSE, bCASE_USING				 },
	{ "catch",					tFALSE, bCATCH					 },
	{ "class",					tFALSE, bCLASS					 },
	{ "cond",					tFALSE, bCOND					 },
	{ "convert",				tFALSE, bCONVERT				 },
	{ "dynamic",				tFALSE, bDYNAMIC				 },
	{ "dynamic-let",			tFALSE, bDYNAMIC_LET			 },
	{ "flet",					tFALSE, bFLET					 },
	{ "for",					tFALSE, bFOR					 },
	{ "function",				tFALSE, bFUNCTION				 },
	{ "go",						tFALSE, bGO						 },
	{ "if",						tFALSE, bIF						 },
	{ "ignore-errors",			tFALSE, bIGNORE_ERRORS			 },
	{ "labels",					tFALSE, bLABELS					 },
	{ "lambda",					tFALSE, bLAMBDA					 },
	{ "let",					tFALSE, bLET					 },
	{ "let*",					tFALSE, bLETA					 },
	{ "or",						tFALSE, bOR						 },
	{ "progn",					tFALSE, bPROGN					 },
	{ "quote",					tFALSE, bQUOTE					 },
	{ "return-from",			tFALSE, bRETURN_FROM			 },
	{ "setf",					tFALSE, bSETF					 },
	{ "setq",					tFALSE, bSETQ					 },
	{ "tagbody",				tFALSE, bTAGBODY				 },
	{ "the",					tFALSE, bTHE					 },
	{ "throw",					tFALSE, bTHROW					 },
	{ "unwind-protect",			tFALSE, bUNWIND_PROTECT			 },
	{ "while",					tFALSE, bWHILE					 },
	{ "with-error-output",		tFALSE, bWITH_ERROR_OUTPUT		 },
	{ "with-handler",			tFALSE, bWITH_HANDLER			 },
	{ "with-open-input-file",	tFALSE, bWITH_OPEN_INPUT_FILE	 },
	{ "with-open-io-file",		tFALSE, bWITH_OPEN_IO_FILE		 },
	{ "with-open-output-file",	tFALSE, bWITH_OPEN_OUTPUT_FILE	 },
	{ "with-standard-input",	tFALSE, bWITH_STANDARD_INPUT	 },
	{ "with-standard-output",	tFALSE, bWITH_STANDARD_OUTPUT	 },
	//
	{ "defclass",				tFALSE, bDEFCLASS				 },
	{ "defconstant",			tFALSE, bDEFCONSTANT			 },
	{ "defdynamic",				tFALSE, bDEFDYNAMIC				 },
	{ "defgeneric",				tFALSE, bDEFGENERIC				 },
	{ "defglobal",				tFALSE, bDEFGLOBAL				 },
	{ "defmacro",				tFALSE, bDEFMACRO				 },
	{ "defmethod",				tFALSE, bDEFMETHOD				 },
	{ "defun",					tFALSE, bDEFUN					 },
	{ "defpackage",				tFALSE, bDEFPACKAGE				 },
	{ "deflink",				tFALSE, bDEFLINK				 },
	//
	{ "load",					tFALSE, 0						 },
	{ "in-package",				tFALSE, 0						 },
	//
	{ "functionp",				tFALSE, 0						 },
	{ "apply",					tFALSE,	0 },
	{ "funcall",				tFALSE,	0 },
	{ "eq",						tFALSE,	0 },
	{ "eql",					tFALSE,	0 },
	{ "equal",					tFALSE,	0 },
	{ "not",					tFALSE,	0 },
	//
	{ "generic-function-p",		tFALSE,	0 },
	//
	{ "class-of",				tFALSE,	0 },
	{ "instancep",				tFALSE,	0 },
	{ "subclassp",				tFALSE,	0 },
	//
	{ "symbolp",				tFALSE,	0 },
	{ "property",				tFALSE,	0 },
	{ "set-property",			tFALSE,	0 },
	{ "remove-property",		tFALSE,	0 },
	{ "gensym",					tFALSE,	0 },
	// <number>
	{ "numberp",				tFALSE,	0 },
	{ "parse-numver",			tFALSE,	0 },
	{ "=",						tFALSE,	0 },
	{ "/=",						tFALSE,	0 },
	{ ">=",						tFALSE,	0 },
	{ "<=",						tFALSE,	0 },
	{ ">",						tFALSE,	0 },
	{ "<",						tFALSE,	0 },
	{ "+",						tFALSE,	0 },
	{ "*",						tFALSE,	0 },
	{ "-",						tFALSE,	0 },
	{ "quotient",				tFALSE,	0 },
	{ "reciprocal",				tFALSE,	0 },
	{ "max",					tFALSE,	0 },
	{ "min",					tFALSE,	0 },
	{ "abs",					tFALSE,	0 },
	{ "exp",					tFALSE,	0 },
	{ "log",					tFALSE,	0 },
	{ "expt",					tFALSE,	0 },
	{ "sqrt",					tFALSE,	0 },
	{ "sin",					tFALSE,	0 },
	{ "cos",					tFALSE,	0 },
	{ "tan",					tFALSE,	0 },
	{ "atan",					tFALSE,	0 },
	{ "atan2",					tFALSE,	0 },
	{ "sinh",					tFALSE,	0 },
	{ "cosh",					tFALSE,	0 },
	{ "tanh",					tFALSE,	0 },
	{ "atanh",					tFALSE,	0 },
	// <float>
	{ "floatp",					tFALSE,	0 },
	{ "float",					tFALSE,	0 },
	{ "floor",					tFALSE,	0 },
	{ "ceiling",				tFALSE,	0 },
	{ "truncate",				tFALSE,	0 },
	{ "round",					tFALSE,	0 },
	// <integer>
	{ "integerp",				tFALSE,	0 },
	{ "div",					tFALSE,	0 },
	{ "mod",					tFALSE,	0 },
	{ "gcd",					tFALSE,	0 },
	{ "lcm",					tFALSE,	0 },
	{ "isqrt",					tFALSE,	0 },
	// <character>
	{ "characterp",				tFALSE,	0 },
	{ "char=",					tFALSE,	0 },
	{ "char/=",					tFALSE,	0 },
	{ "char<",					tFALSE,	0 },
	{ "char>",					tFALSE,	0 },
	{ "char<=",					tFALSE,	0 },
	{ "char>=",					tFALSE,	0 },
	// <list>
	// <cons>
	{ "consp",					tFALSE,	0 },
	{ "cons",					tFALSE,	0 },
	{ "car",					tFALSE,	0 },
	{ "cdr",					tFALSE,	0 },
	{ "set-car",				tFALSE,	0 },
	{ "set-cdr",				tFALSE,	0 },
	// <null>
	{ "null",					tFALSE,	0 },
	// list operations
	{ "listp",					tFALSE,	0 },
	{ "create-list",			tFALSE,	0 },
	{ "list",					tFALSE,	0 },
	{ "reverse",				tFALSE,	0 },
	{ "nreverse",				tFALSE,	0 },
	{ "append",					tFALSE,	0 },
	{ "member",					tFALSE,	0 },
	{ "mapcar",					tFALSE,	0 },
	{ "mapc",					tFALSE,	0 },
	{ "mapcan",					tFALSE,	0 },
	{ "maplist",				tFALSE,	0 },
	{ "mapl",					tFALSE,	0 },
	{ "mapcon",					tFALSE,	0 },
	{ "assoc",					tFALSE,	0 },
	// array
	// array operations
	{ "basic-array-p",			tFALSE,	0 },
	{ "basic-array*-p",			tFALSE,	0 },
	{ "general-array*-p",		tFALSE,	0 },
	{ "create-array",			tFALSE,	0 },
	{ "aref",					tFALSE,	0 },
	{ "garef",					tFALSE,	0 },
	{ "set-aref",				tFALSE,	0 },
	{ "set-garef",				tFALSE,	0 },
	{ "array-dimensions",		tFALSE,	0 },
	{ "basic-vector-p",			tFALSE,	0 },
	{ "general-vector-p",		tFALSE,	0 },
	{ "create-vector",			tFALSE,	0 },
	{ "vector",					tFALSE,	0 },
	{ "stringp",				tFALSE,	0 },
	{ "create-string",			tFALSE,	0 },
	{ "string=",				tFALSE,	0 },
	{ "string/=",				tFALSE,	0 },
	{ "string<",				tFALSE,	0 },
	{ "string>",				tFALSE,	0 },
	{ "string>=",				tFALSE,	0 },
	{ "string<=",				tFALSE,	0 },
	{ "char-index",				tFALSE,	0 },
	{ "string-index",			tFALSE,	0 },
	{ "string-append",			tFALSE,	0 },
	{ "length",					tFALSE,	0 },
	{ "elt",					tFALSE,	0 },
	{ "set-elt",				tFALSE,	0 },
	{ "subseq",					tFALSE,	0 },
	{ "map-into",				tFALSE,	0 },
	{ "streamp",				tFALSE,	0 },
	{ "open-stream-p",			tFALSE,	0 },
	{ "input-stream-p",			tFALSE,	0 },
	{ "output-stream-p",		tFALSE,	0 },
	{ "standard-input",			tFALSE,	0 },
	{ "standard-output",		tFALSE,	0 },
	{ "error-output",			tFALSE,	0 },
	{ "open-input-file",		tFALSE,	0 },
	{ "open-output-file",		tFALSE,	0 },
	{ "open-io-file",			tFALSE,	0 },
	{ "close",					tFALSE,	0 },
	{ "finish-output",			tFALSE,	0 },
	{ "create-string-input-stream",	tFALSE,	0 },
	{ "create-string-output-stream",	tFALSE,	0 },
	{ "get-output-stream-string",	tFALSE,	0 },
	{ "read",					tFALSE,	0 },
	{ "read-char",				tFALSE,	0 },
	{ "preview-char",			tFALSE,	0 },
	{ "read-line",				tFALSE,	0 },
	{ "stream-ready-p",			tFALSE,	0 },
	{ "format",					tFALSE,	0 },
	{ "format-char",			tFALSE,	0 },
	{ "formar-float",			tFALSE,	0 },
	{ "format-fresh-line",		tFALSE,	0 },
	{ "format-integer",			tFALSE,	0 },
	{ "format-object",			tFALSE,	0 },
	{ "format-tab",				tFALSE,	0 },
	{ "read-byte",				tFALSE,	0 },
	{ "write-byte",				tFALSE,	0 },
	{ "probe-file",				tFALSE,	0 },
	{ "file-position",			tFALSE,	0 },
	{ "set-file-position",		tFALSE,	0 },
	{ "file-length",			tFALSE,	0 },
	{ "error",					tFALSE,	0 },
	{ "cerror",					tFALSE,	0 },
	{ "signal-condition",		tFALSE,	0 },
	{ "condition-continuable",	tFALSE,	0 },
	{ "arithmetic-error-operation",	tFALSE,	0 },
	{ "arithmetic-error-operands",	tFALSE,	0 },
	{ "domain-error-object",	tFALSE,	0 },
	{ "domain-error-expected-class",	tFALSE,	0 },
	{ "parse-error-string",	tFALSE,	0 },
	{ "parse-error-expected-class",	tFALSE,	0 },
	{ "simple-error-format-string",	tFALSE,	0 },
	{ "simple-error-format-arguments",	tFALSE,	0 },
	{ "stream-error-stream",	tFALSE,	0 },
	{ "undefined-entity-name",	tFALSE,	0 },
	{ "undefined-entity-namesace",	tFALSE,	0 },
	{ "identity",	tFALSE,	0 },
	{ "get-universal-time",	tFALSE,	0 },
	{ "get-internal-run-time",	tFALSE,	0 },
	{ "get-internal-real-time",	tFALSE,	0 },
	{ "get-internal-time-units-per-second",	tFALSE,	0 },
	{ "system",	tFALSE,	0 },
	{ "exit",	tFALSE,	0 },
	{ "strftime",	tFALSE,	0 },
	{ "get-argument",	tFALSE,	0 },
	{ "get-environment",	tFALSE,	0 },
	{ "eval", tFALSE, 0 },
	//
	{ "t",						tFALSE, 0						 },
	{ "nil",					tFALSE, 0						 },
	{ "standard",				tFALSE, 0						 },
	//
	{ "rest",					tTRUE,  0						 },
	{ "public",					tTRUE,	0						 },
	{ "private",				tTRUE,	0						 },
	{ "method",					tTRUE,	0						 },
	{ "method-combination",		tTRUE,	0						 },
	{ "generic-function-class",	tTRUE,	0						 },
	{ "around",					tTRUE,	0						 },
	{ "before",					tTRUE,	0						 },
	{ "after",					tTRUE,	0						 },
	{ "reader",					tTRUE,	0						 },
	{ "writer",					tTRUE,	0						 },
	{ "accessor",				tTRUE,	0						 },
	{ "boundp",					tTRUE,	0						 },
	{ "initform",				tTRUE,	0						 },
	{ "initarg",				tTRUE,	0						 },
	{ "metaclass",				tTRUE,	0						 },
	{ "abstractp",				tTRUE,	0						 },
	{ "void",					tTRUE,	0						 },
	//
	{ "error-handler",			tFALSE,	0						 },
	{ "<object>",				tFALSE, 0						 },
	{ "call-next-method",		tFALSE,	0						 },
	{ "next-method-p",			tFALSE,	0						 },
	{ "object",					tFALSE, 0						 },
	{ "instance",				tFALSE, 0						 },
	{ "condition",				tFALSE, 0						 },
	{ "<standard-class>",		tFALSE, 0						 },
	{ "<standard-object>",		tFALSE, 0						 },
	{ "<serious-condition>",	tFALSE, 0						 },
	{ "<list>",					tFALSE, 0						 },
	{ "<stream>",				tFALSE, 0						 },
	{ "stream",					tFALSE, 0						 },
	{ "<foreign-class>",		tFALSE, 0						 },
};

#endif

/////////////////////////////
// CONDITION

extern tPCELL	condition_system_error;
extern tPCELL	condition_storage_exhausted;
extern tPCELL	condition_stack_overflow;
extern tPCELL	tisl_object_storage_exhausted;

/////////////////////////////

void clear_global_objects(void);
VM_RET initialize_global_objects(tPTISL tisl);
VM_RET initialize_built_in_object(tPTISL tisl);

#define MAX_PARAMETER_SIZE		256

#endif
