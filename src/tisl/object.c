//
// TISL/src/tisl/object.c
// TISL Ver 4.x
//

#define TISL_OBJECT_C

#include <string.h>
#include <malloc.h>

#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "gc.h"
#include "writer.h"
#include "built_in_object.h"

// TISL/src/tisl/tisl.c
extern VM_RET tisl_lock(tPVM* lock, tPVM vm);
extern void tisl_unlock(tPVM* lock);

////////////////////////////////////////////////
// tOBJECT

VM_RET unbound_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{// 例外？/*!!!*/
	return write_string(vm, stream, "#unbound");
}

VM_RET nil_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_string(vm, stream, "nil");
}

VM_RET integer_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_integer(vm, stream, OBJECT_GET_INTEGER(obj), 10);
}

VM_RET float_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_float(vm, stream, OBJECT_GET_FLOAT(obj));
}

VM_RET character_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tCHAR c=OBJECT_GET_CHARACTER(obj);
	if (vm_get_writer_flag(vm)) {
		if (write_string(vm, stream, "#\\")) return VM_ERROR;
		if (c==' ') {
			return write_string(vm, stream, "space");
		} else if (c=='\n') {
			return write_string(vm, stream, "newline");
		}
	}
	return write_char(vm, stream, c);
}

tCSTRING built_in_class_get_name(const tINT id)
{
	tCSTRING string;
	switch (id) {
	case CLASS_OBJECT:						string="<object>"; break;
	case CLASS_BASIC_ARRAY:					string="<basic-array>"; break;
	case CLASS_BASIC_ARRAY_A:				string="<basic-array*>"; break;
	case CLASS_GENERAL_ARRAY_A:				string="<general-array*>"; break;
	case CLASS_BASIC_VECTOR:				string="<basic-vector>"; break;
	case CLASS_GENERAL_VECTOR:				string="<general-vector>"; break;
	case CLASS_STRING:						string="<string>"; break;
	case CLASS_BUILT_IN_CLASS:				string="<built-in-class>"; break;
	case CLASS_CHARACTER:					string="<character>"; break;
	case CLASS_FUNCTION:					string="<function>"; break;
	case CLASS_GENERIC_FUNCTION:			string="<generic-function>"; break;
	case CLASS_LOCAL_FUNCTION:				string="<local-function>"; break;
	case CLASS_LIBRARY_FUNCTION:			string="<library-function>"; break;
	case CLASS_PRIMITIVE_FUNCTION:			string="<primitive-function>"; break;
	case CLASS_STANDARD_GENERIC_FUNCTION:	string="<standard-generic-function>"; break;
	case CLASS_LIST:						string="<list>"; break;
	case CLASS_CONS:						string="<cons>"; break;
	case CLASS_NUMBER:						string="<number>"; break;
	case CLASS_INTEGER:						string="<integer>"; break;
	case CLASS_FLOAT:						string="<float>"; break;
	case CLASS_SERIOUS_CONDITION:			string="<serious-condition>"; break;
	case CLASS_ERROR:						string="<error>"; break;
	case CLASS_ARITHMETIC_ERROR:			string="<arithmetic-error>"; break;
	case CLASS_DIVISION_BY_ZERO:			string="<division-by-zero>"; break;
	case CLASS_FLOATING_POINT_OVERFLOW:		string="<floating-point-overflow>"; break;
	case CLASS_FLOATING_POINT_UNDERFLOW:	string="<floating-point-underflow>"; break;
	case CLASS_CONTROL_ERROR:				string="<control-error>"; break;
	case CLASS_PARSE_ERROR:					string="<parse-error>"; break;
	case CLASS_PROGRAM_ERROR:				string="<program-error>"; break;
	case CLASS_DOMAIN_ERROR:				string="<domain-error>"; break;
	case CLASS_UNDEFINED_ENTITY:			string="<undefined-entity>"; break;
	case CLASS_UNBOUND_VARIABLE:			string="<unbound-variable>"; break;
	case CLASS_UNDEFINED_FUNCTION:			string="<undefined-function>"; break;
	case CLASS_SIMPLE_ERROR:				string="<simple-error>"; break;
	case CLASS_STREAM_ERROR:				string="<stream-error>"; break;
	case CLASS_END_OF_STREAM:				string="<end-of-stream>"; break;
	case CLASS_STORAGE_EXHAUSTED:			string="<strorage-exhausted>"; break;
	case CLASS_STANDARD_CLASS:				string="<standard-class>"; break;
	case CLASS_STREAM:						string="<stream>"; break;
	case CLASS_STRING_STREAM:				string="<string-stream>"; break;
	case CLASS_FILE_STREAM:					string="<file-stream>"; break;
	case CLASS_SYMBOL:						string="<symbol>"; break;
	case CLASS_NULL:						string="<null>"; break;
	case CLASS_SPECIAL_OPERATOR:			string="<special-operator>"; break;
	case CLASS_MACRO:						string="<macro>"; break;
	case CLASS_DEFINING_OPERATOR:			string="<defining-operator>"; break;
	case CLASS_DYNAMIC_LINK_LIBRARY:		string="<dynamic-link-library>"; break;
	case CLASS_PACKAGE:						string="<package>"; break;
	case CLASS_FOREIGN_OBJECT:				string="<foreign-object>"; break;
	case CLASS_VIOLATION:					string="<violation>"; break;
	case CLASS_FOREIGN_CLASS:				string="<foreign-class>"; break;
	default:/*!!!*//*例外？*/				string="<object>";
	}
	return string;
}

VM_RET built_in_class_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (OBJECT_GET_INTEGER(obj)==CLASS_FOREIGN_OBJECT) {
		return write_string(vm, stream, "#i(<foreign-class> name <foreign-object>)");
	} else {
		tCSTRING string=built_in_class_get_name(OBJECT_GET_INTEGER(obj));
		return write_string(vm, stream, "#i(<built-in-class> ")||
			write_string(vm, stream, string)||
			write_char(vm, stream, ')');
	}
}

///////////////////

tBOOL object_eql(tPOBJECT obj1, tPOBJECT obj2)
{
	if (OBJECT_GET_TYPE(obj1)!=OBJECT_GET_TYPE(obj2)) return tFALSE;
	switch (OBJECT_GET_TYPE(obj1)) {
	case OBJECT_UNBOUND:
	case OBJECT_NIL:
		return tTRUE;
	case OBJECT_INTEGER:
	case OBJECT_BUILT_IN_CLASS:
	case OBJECT_SPECIAL_OPERATOR:
	case OBJECT_DEFINING_OPERATOR:
	case OBJECT_PRIMITIVE_OPERATOR:
		return (OBJECT_GET_INTEGER(obj1)==OBJECT_GET_INTEGER(obj2)) ? tTRUE : tFALSE;
	case OBJECT_FLOAT:
		return (OBJECT_GET_FLOAT(obj1)==OBJECT_GET_FLOAT(obj2)) ? tTRUE : tFALSE;
	case OBJECT_CHARACTER:
		return (OBJECT_GET_CHARACTER(obj1)==OBJECT_GET_CHARACTER(obj2)) ? tTRUE : tFALSE;
	default:
		return (OBJECT_GET_CELL(obj1)==OBJECT_GET_CELL(obj2)) ? tTRUE : tFALSE;
	}
}

tBOOL object_equal(tPOBJECT obj1, tPOBJECT obj2)
{
	if (OBJECT_GET_TYPE(obj1)!=OBJECT_GET_TYPE(obj2)) return tFALSE;
	switch (OBJECT_GET_TYPE(obj1)) {
	case OBJECT_UNBOUND:
	case OBJECT_NIL:
		return tTRUE;
	case OBJECT_INTEGER:
	case OBJECT_BUILT_IN_CLASS:
	case OBJECT_SPECIAL_OPERATOR:
	case OBJECT_DEFINING_OPERATOR:
	case OBJECT_PRIMITIVE_OPERATOR:
		return (OBJECT_GET_INTEGER(obj1)==OBJECT_GET_INTEGER(obj2)) ? tTRUE : tFALSE;
	case OBJECT_FLOAT:
		return (OBJECT_GET_FLOAT(obj1)==OBJECT_GET_FLOAT(obj2)) ? tTRUE : tFALSE;
	case OBJECT_CHARACTER:
		return (OBJECT_GET_CHARACTER(obj1)==OBJECT_GET_CHARACTER(obj2)) ? tTRUE : tFALSE;
	case OBJECT_CONS:
		return cons_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	case OBJECT_VECTOR:
		return vector_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	case OBJECT_STRING:
		return string_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	case OBJECT_ARRAY:
		return array_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	case OBJECT_QUASIQUOTE:
		return quasiquote_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	case OBJECT_UNQUOTE:
		return unquote_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	case OBJECT_UNQUOTE_SPLICING:
		return unquote_splicing_equal(OBJECT_GET_CELL(obj1), OBJECT_GET_CELL(obj2));
	default:// 標準オブジェクトとかも関数用意する？
		return (OBJECT_GET_CELL(obj1)==OBJECT_GET_CELL(obj2)) ? tTRUE : tFALSE;
	}
}

tBOOL object_is_instance(tPVM vm, tPOBJECT obj, tPOBJECT clss)
{
	tOBJECT clss1;
	object_get_class(obj, &clss1);
	// 直接のインスタンス
	if (object_eql(&clss1, clss)) return tTRUE;
	// 下位クラスのインスタンス
	if (class_is_subclass(vm, &clss1, clss)) return tTRUE;
	return tFALSE;
}

void object_get_class(tPOBJECT obj, tPOBJECT clss)
{
	switch (OBJECT_GET_TYPE(obj)) {
	case OBJECT_NIL:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_NULL);				break;
	case OBJECT_INTEGER:			OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_INTEGER);				break;
	case OBJECT_FLOAT:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_FLOAT);				break;
	case OBJECT_CHARACTER:			OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_CHARACTER);			break;
	case OBJECT_BUILT_IN_CLASS:
		{
			if (OBJECT_GET_INTEGER(obj)==CLASS_FOREIGN_OBJECT)
				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_FOREIGN_CLASS);
			else
				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_BUILT_IN_CLASS);
			break;
		}
	case OBJECT_SPECIAL_OPERATOR:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_SPECIAL_OPERATOR);	break;
	case OBJECT_DEFINING_OPERATOR:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_DEFINING_OPERATOR);	break;
	case OBJECT_PRIMITIVE_OPERATOR:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_PRIMITIVE_FUNCTION);	break;
	case OBJECT_CONS:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_CONS);				break;
	case OBJECT_STRING:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_STRING);				break;
	case OBJECT_SYMBOL:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_SYMBOL);				break;
	case OBJECT_VECTOR:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_GENERAL_VECTOR);		break;
	case OBJECT_ARRAY:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_GENERAL_ARRAY_A);		break;
	case OBJECT_QUASIQUOTE:			// このへんは何クラスになるんだろう？
	case OBJECT_UNQUOTE:
	case OBJECT_UNQUOTE_SPLICING:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_OBJECT);				break;
	case OBJECT_STRING_STREAM:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_STRING_STREAM);		break;
	case OBJECT_FILE_STREAM:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_FILE_STREAM);			break;
	case OBJECT_CONDITION:			OBJECT_SET_BUILT_IN_CLASS(clss, condition_get_class_id(OBJECT_GET_CELL(obj)));	break;
	case OBJECT_PACKAGE:			OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_PACKAGE);				break;
	case OBJECT_FOREIGN_OBJECT:		foreign_object_get_class(OBJECT_GET_CELL(obj), clss);		break;
	case OBJECT_FUNCTION:			OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_FUNCTION);			break;
	case OBJECT_ENVIRONMENT:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_OBJECT);				break;
	case OBJECT_LOCAL_FUNCTION:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_LOCAL_FUNCTION);		break;
	case OBJECT_STANDARD_CLASS:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_STANDARD_CLASS);		break;
	case OBJECT_GENERIC_FUNCTION:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_STANDARD_GENERIC_FUNCTION);	break;
	case OBJECT_METHOD:				// first classには現われてはこないはず
	case OBJECT_APPLICABLE_METHOD:
	case OBJECT_EFFECTIVE_METHOD:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_OBJECT); break;
	case OBJECT_INSTANCE:			OBJECT_SET_STANDARD_CLASS(clss, instance_get_class(OBJECT_GET_CELL(obj))); break;
	case OBJECT_SLOT:				OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_OBJECT);				break;
	case OBJECT_LINKED_FUNCTION:	OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_LIBRARY_FUNCTION);	break;
	case OBJECT_LINKED_LIBRARY:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_DYNAMIC_LINK_LIBRARY);break;
	case OBJECT_FOREIGN_CLASS:		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_FOREIGN_CLASS);		break;
	case OBJECT_TISL_OBJECT:// first classにくる？
	default:						OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_OBJECT);
	}
}

tBOOL class_is_subclass(tPVM vm, tPOBJECT clss, tPOBJECT super)
{
	if (OBJECT_IS_BUILT_IN_CLASS(clss)&&OBJECT_IS_BUILT_IN_CLASS(super)) {
		// 両方とも組込みクラス
		return ((OBJECT_GET_INTEGER(clss)&OBJECT_GET_INTEGER(super))==OBJECT_GET_INTEGER(super)) ? tTRUE : tFALSE;
	} else if (OBJECT_IS_STANDARD_CLASS(clss)&&OBJECT_IS_STANDARD_CLASS(super)) {
		return standard_class_is_subclass(vm, OBJECT_GET_CELL(clss), OBJECT_GET_CELL(super));
	} else if (OBJECT_IS_STANDARD_CLASS(clss)&&OBJECT_IS_BUILT_IN_CLASS(super)) {
		// 標準クラスの上位の組込みクラスは<object>のみ
		if (OBJECT_GET_INTEGER(super)==CLASS_OBJECT)
			return tTRUE;
		else
			return tFALSE;
	} else if (OBJECT_IS_BUILT_IN_CLASS(clss)&&OBJECT_IS_STANDARD_CLASS(super)) {
		// ありえない
		return tFALSE;
	} else if (OBJECT_IS_FOREIGN_CLASS(clss)&&OBJECT_IS_FOREIGN_CLASS(clss)) {
		return foreign_class_is_subclass(OBJECT_GET_CELL(clss), OBJECT_GET_CELL(super));
	} else if (OBJECT_IS_FOREIGN_CLASS(clss)&&OBJECT_IS_BUILT_IN_CLASS(super)) {
		if ((OBJECT_GET_INTEGER(super)==CLASS_OBJECT)||
			(OBJECT_GET_INTEGER(super)==CLASS_FOREIGN_OBJECT))
			return tTRUE;
		else
			return tFALSE;
	} else if (OBJECT_IS_BUILT_IN_CLASS(clss)&&OBJECT_IS_FOREIGN_CLASS(super)) {
		// ありえない
		return tFALSE;
	} else {// domain-error?
		return tFALSE;
	}
}

static tINT built_in_class_get_precedence(tINT clss1, tINT clss2);
static tINT built_in_class_get_depth(tINT clss1);

// class1のクラス優先度におけるclass2の優先度の取得
tINT class_get_precedence(tPVM vm, tPOBJECT clss1, tPOBJECT clss2)
{
	if (OBJECT_IS_STANDARD_CLASS(clss1)) {
		// 標準クラスの優先度
		return standard_class_get_precedence(vm, OBJECT_GET_CELL(clss1), clss2);
	} else if (OBJECT_IS_BUILT_IN_CLASS(clss1)) {
		if (OBJECT_IS_BUILT_IN_CLASS(clss2))
			return built_in_class_get_precedence(OBJECT_GET_INTEGER(clss1), OBJECT_GET_INTEGER(clss2));
		else
			return -1;
	} else {
		return -1;
	}
}

static tINT built_in_class_get_precedence(tINT clss1, tINT clss2)
{
	if ((clss1&clss2)!=clss2) return -1;

	if (clss1==CLASS_NULL) {
		// <null> <symbol> <list> <object>
		switch (clss2) {
		case CLASS_OBJECT:	return 0;
		case CLASS_LIST:	return 1;
		case CLASS_SYMBOL:	return 2;
		case CLASS_NULL:	return 3;
		default:			return -1;
		}
	} else {
		return built_in_class_get_depth(clss2);
	}
}

static tINT built_in_class_get_depth(tINT clss1)
{
	switch (clss1) {
	case CLASS_OBJECT:
		return 0;
	case CLASS_BASIC_ARRAY:
	case CLASS_BUILT_IN_CLASS:
	case CLASS_CHARACTER:
	case CLASS_FUNCTION:
	case CLASS_LIST:
	case CLASS_NUMBER:
	case CLASS_SERIOUS_CONDITION:
	case CLASS_STANDARD_CLASS:
	case CLASS_STREAM:
	case CLASS_SYMBOL:
	case CLASS_SPECIAL_OPERATOR:
		//
	case CLASS_DYNAMIC_LINK_LIBRARY:
	case CLASS_PACKAGE:
	case CLASS_FOREIGN_OBJECT:
	case CLASS_FOREIGN_CLASS:
		return 1;
	case CLASS_BASIC_ARRAY_A:
	case CLASS_BASIC_VECTOR:
	case CLASS_GENERIC_FUNCTION:
	case CLASS_LOCAL_FUNCTION:
	case CLASS_LIBRARY_FUNCTION:
	case CLASS_PRIMITIVE_FUNCTION:
	case CLASS_CONS:
	case CLASS_INTEGER:
	case CLASS_FLOAT:
	case CLASS_ERROR:
	case CLASS_STORAGE_EXHAUSTED:
	case CLASS_STRING_STREAM:
	case CLASS_FILE_STREAM:
	case CLASS_MACRO:
	case CLASS_DEFINING_OPERATOR:
		return 2;
	case CLASS_GENERAL_ARRAY_A:
	case CLASS_GENERAL_VECTOR:
	case CLASS_STRING:
	case CLASS_STANDARD_GENERIC_FUNCTION:
	case CLASS_ARITHMETIC_ERROR:
	case CLASS_CONTROL_ERROR:
	case CLASS_PARSE_ERROR:
	case CLASS_PROGRAM_ERROR:
	case CLASS_SIMPLE_ERROR:
	case CLASS_STREAM_ERROR:
	case CLASS_NULL://???
		return 3;
	case CLASS_DIVISION_BY_ZERO:
	case CLASS_FLOATING_POINT_OVERFLOW:
	case CLASS_FLOATING_POINT_UNDERFLOW:
	case CLASS_DOMAIN_ERROR:
	case CLASS_UNDEFINED_ENTITY:
	case CLASS_END_OF_STREAM:
		return 4;
	case CLASS_UNBOUND_VARIABLE:
	case CLASS_UNDEFINED_FUNCTION:
		return 5;
	default:
		return -1;
	}
}

VM_RET object_mark(tPVM vm, tPOBJECT obj)
{
	if (OBJECT_IS_CELL(obj)) 
		return cell_mark(vm, OBJECT_GET_CELL(obj));
	return VM_OK;
}

/////////////////////////////////////////////////
// tCELL

tUINT cell_get_size(tPCELL cell)
{
	return (*cell_get_size_table[CELL_GET_TYPE_INDEX(cell)])(cell);
}

VM_RET cell_destroy(tPVM vm, tPCELL cell)
{
	switch (CELL_GET_TYPE(cell)) {
	case CELL_CONS:
		return VM_OK;
	case CELL_STRING:
		return string_destroy(vm, cell);
	case CELL_SYMBOL:
		return symbol_destroy(vm, cell);
	case CELL_STRING_STREAM:
		return string_stream_destroy(vm, cell);
	case CELL_FILE_STREAM:
		return file_stream_destroy(vm, cell);
	case CELL_FOREIGN_OBJECT:
		return foreign_object_destroy(vm, cell);
	case CELL_LINKED_LIBRARY:
		return linked_library_destroy(vm, cell);
	default:
		return VM_OK;
	}
//	return (*cell_destroy_table[CELL_GET_TYPE_INDEX(cell)])(vm, cell);
}

VM_RET cell_destroy_dummy(tPVM vm, tPCELL cell)
{
	return VM_OK;
}

void cell_to_object(tPCELL cell, tPOBJECT obj)
{
	if (!cell) {
		OBJECT_SET_UNBOUND(obj);
	} else {
		switch (CELL_GET_TYPE(cell)) {
		case CELL_FREE:					OBJECT_SET_FREE(obj, cell); break;
		case CELL_CONS:					OBJECT_SET_CONS(obj, cell); break;
		case CELL_STRING:				OBJECT_SET_STRING(obj, cell); break;
		case CELL_SYMBOL:				OBJECT_SET_SYMBOL(obj, cell); break;
		case CELL_VECTOR:				OBJECT_SET_VECTOR(obj, cell); break;
		case CELL_ARRAY:				OBJECT_SET_ARRAY(obj, cell); break;
		case CELL_QUASIQUOTE:			OBJECT_SET_QUASIQUOTE(obj, cell); break;
		case CELL_UNQUOTE:				OBJECT_SET_UNQUOTE(obj, cell); break;
		case CELL_UNQUOTE_SPLICING:		OBJECT_SET_UNQUOTE_SPLICING(obj, cell); break;
		case CELL_STRING_STREAM:		OBJECT_SET_STRING_STREAM(obj, cell); break;
		case CELL_FILE_STREAM:			OBJECT_SET_FILE_STREAM(obj, cell); break;
		case CELL_CONDITION:			OBJECT_SET_CONDITION(obj, cell); break;
		case CELL_PACKAGE:				OBJECT_SET_PACKAGE(obj, cell); break;
		case CELL_BIND:					OBJECT_SET_BIND(obj, cell); break;
		case CELL_BIND_LIST:			OBJECT_SET_BIND_LIST(obj, cell); break;
		case CELL_FOREIGN_OBJECT:		OBJECT_SET_FOREIGN_OBJECT(obj, cell); break;
		case CELL_FUNCTION:				OBJECT_SET_FUNCTION(obj, cell); break;
		case CELL_ENVIRONMENT:			OBJECT_SET_ENVIRONMENT(obj, cell); break;
		case CELL_LOCAL_FUNCTION:		OBJECT_SET_LOCAL_FUNCTION(obj, cell); break;
		case CELL_STANDARD_CLASS:		OBJECT_SET_STANDARD_CLASS(obj, cell); break;
		case CELL_GENERIC_FUNCTION:		OBJECT_SET_GENERIC_FUNCTION(obj, cell); break;
		case CELL_METHOD:				OBJECT_SET_METHOD(obj, cell); break;
		case CELL_EFFECTIVE_METHOD:		OBJECT_SET_EFFECTIVE_METHOD(obj, cell); break;
		case CELL_APPLICABLE_METHOD:	OBJECT_SET_APPLICABLE_METHOD(obj, cell); break;
		case CELL_INSTANCE:				OBJECT_SET_INSTANCE(obj, cell); break;
		case CELL_SLOT:					OBJECT_SET_SLOT(obj, cell); break;
		case CELL_LINKED_FUNCTION:		OBJECT_SET_LINKED_FUNCTION(obj, cell); break;
		case CELL_LINKED_LIBRARY:		OBJECT_SET_LINKED_LIBRARY(obj, cell); break;
		case CELL_TISL_OBJECT:			OBJECT_SET_TISL_OBJECT(obj, cell); break;
		case CELL_FOREIGN_CLASS:		OBJECT_SET_FOREIGN_CLASS(obj, cell); break;
		default:						OBJECT_SET_UNBOUND(obj);
		}
	}
}

/////////////////////////////////////////////////
// CELL_FREE

#define FREE_GET_SIZE(cell)			(((cell)+1)->ui)
#define FREE_SET_SIZE(cell, size)	(((cell)+1)->ui=(size))
#define FREE_GET_NEXT(cell)			(((cell)+2)->cell)
#define FREE_SET_NEXT(cell, next)	(((cell)+2)->cell=(next))

void free_initialize(tPCELL cell, tUINT size, tPCELL next)
{
	CELL_SET_TYPE(cell, CELL_FREE);
	FREE_SET_SIZE(cell, size);
	FREE_SET_NEXT(cell, next);
}

tUINT free_get_size(tPCELL cell)
{
	return FREE_GET_SIZE(cell);
}

tPCELL free_get_next(tPCELL cell)
{
	return FREE_GET_NEXT(cell);
}

void free_set_next(tPCELL cell, tPCELL next)
{
	FREE_SET_NEXT(cell, next);
}

VM_RET free_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{// 例外?/*!!!*/
	return write_string(vm, stream, "#garbage");
}

VM_RET free_mark(tPVM vm, tPCELL cell)
{
	if (FREE_GET_NEXT(cell)) {
		tOBJECT tmp;
		OBJECT_SET_FREE(&tmp, FREE_GET_NEXT(cell));
		return gc_push(vm, &tmp);
	} else {
		return VM_OK;
	}
}

/////////////////////////////////////////////////
// CELL_CONS

#define CONS_SIZE						CELL_UNIT

//static tINT cons_get_length_(tPCELL cons, const tINT len);
static VM_RET cons_write_(tPVM vm, tPCELL stream, tPCELL cons);

///////////////////

#define CONS_CAR_TYPE					0x000000ff
#define CONS_CAR_TYPE_					0xffffff00
#define CONS_CDR_TYPE					0x0000ff00
#define CONS_CDR_TYPE_					0xffff00ff
#define CONS_GET_CAR_TYPE(cons)			((cons)->ui&CONS_CAR_TYPE)
#define CONS_SET_CAR_TYPE(cons, type)	((cons)->ui&=CONS_CAR_TYPE_, (cons)->ui|=(type)&CONS_CAR_TYPE)
#define CONS_GET_CDR_TYPE(cons)			(((cons)->ui&CONS_CDR_TYPE)>>8)
#define CONS_SET_CDR_TYPE(cons, type)	((cons)->ui&=CONS_CDR_TYPE_, (cons)->ui|=((type)<<8)&CONS_CDR_TYPE)
#define CONS_GET_CAR_DATA(cons)			((cons)[1])
#define CONS_SET_CAR_DATA(cons, data)	((cons)[1]=(data))
#define CONS_GET_CDR_DATA(cons)			((cons)[2])
#define CONS_SET_CDR_DATA(cons, data)	((cons)[2]=(data))
#define CONS_GET_CAR(cons, obj)			(OBJECT_SET_TYPE(obj, CONS_GET_CAR_TYPE(cons)), (obj)->data=CONS_GET_CAR_DATA(cons))
#define CONS_SET_CAR(cons, obj)			(CONS_SET_CAR_TYPE(cons, OBJECT_GET_TYPE(obj)), CONS_SET_CAR_DATA(cons, (obj)->data))
#define CONS_GET_CDR(cons, obj)			(OBJECT_SET_TYPE(obj, CONS_GET_CDR_TYPE(cons)), (obj)->data=CONS_GET_CDR_DATA(cons))
#define CONS_SET_CDR(cons, obj)			(CONS_SET_CDR_TYPE(cons, OBJECT_GET_TYPE(obj)), CONS_SET_CDR_DATA(cons, (obj)->data))

// car cdr の保護あり
VM_RET cons_create(tPVM vm, tPCELL* cons, tPOBJECT car, tPOBJECT cdr)
{
	VM_RET ret;

	if (vm_push(vm, car)) return VM_ERROR;
	if (vm_push(vm, cdr)) { vm_pop(vm); return VM_ERROR; }
	ret=cons_create_(vm, cons, car, cdr);
	vm_pop(vm);
	vm_pop(vm);
	return ret;
}

// car cdr の保護無し
VM_RET cons_create_(tPVM vm, tPCELL* cons, tPOBJECT car, tPOBJECT cdr)
{
	if (!allocate_cell(vm, CONS_SIZE, cons)) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_CONS);

	CELL_SET_TYPE(*cons, CELL_CONS);
	CONS_SET_CAR(*cons, car);
	CONS_SET_CDR(*cons, cdr);

	return VM_OK;
}

tUINT cons_get_size(tPCELL cons)
{
	return CONS_SIZE;
}

void cons_get_car(tPCELL cons, tPOBJECT obj)
{
	CONS_GET_CAR(cons, obj);
}

void cons_set_car(tPCELL cons, tPOBJECT obj)
{
	CONS_SET_CAR(cons, obj);
}

void cons_get_cdr(tPCELL cons, tPOBJECT obj)
{
	CONS_GET_CDR(cons, obj);
}

void cons_set_cdr(tPCELL cons, tPOBJECT obj)
{
	CONS_SET_CDR(cons, obj);
}

tPCELL cons_get_car_cons(tPCELL cons)
{
	return (CONS_GET_CAR_TYPE(cons)==OBJECT_CONS) ? CONS_GET_CAR_DATA(cons).cell : 0;
}

tPCELL cons_get_cdr_cons(tPCELL cons)
{
	return (CONS_GET_CDR_TYPE(cons)==OBJECT_CONS) ? CONS_GET_CDR_DATA(cons).cell : 0;
}
/*
tINT cons_get_length(tPCELL cons)
{
	if (!cons) return 0;
	return CELL_IS_CONS(cons) ? cons_get_length_(cons, 1) : 0;
}

static tINT cons_get_length_(tPCELL cons, const tINT len)
{
	return (CONS_GET_CDR_TYPE(cons)==OBJECT_CONS) ? cons_get_length_(cons_get_cdr_cons(cons), len+1) : len;
}*/

tINT cons_get_length(tPCELL cons)
{
	if (cons&&CELL_IS_CONS(cons)) {
		tINT len;
		for (len=1; CONS_GET_CDR_TYPE(cons)==OBJECT_CONS; cons=cons_get_cdr_cons(cons), len++);
		return len;
	} else {
		return 0;
	}
}

VM_RET cons_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (write_char(vm, stream, '(')) return VM_ERROR;
	if (cons_write_(vm, stream, OBJECT_GET_CELL(obj))) return VM_ERROR;
	return write_char(vm, stream, ')');
}

// /*!!!*/
// cdr方向の長いリストの表示に対応させる
// car方向のリストには対応していない．長いとそのうち落ちるかも
// (Cの)スタックがあふれそうな所は再帰呼び出しで処理を書くのはまずい
// 似たような理由でおちそうなところがあちこちにある
// (リーダ，ライタ，次元の大きい配列，equal，…)
// 次に処理系を書くときには，そのへんをもっと意識して書くこと!!!
// (あるいは，制限をつける)
// /*!!!*/
static void cons_clear_loop(tPCELL cons);

static VM_RET cons_write_(tPVM vm, tPCELL stream, tPCELL cons)
{
	tPCELL p;
	tOBJECT obj;

	p=cons;
LOOP:
	if (!p) { cons_clear_loop(cons); return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR); }
	if (CELL_GET_LOOP(p)) {
		if (write_string(vm, stream, "#cons-loop")) goto ERROR;
		if (write_integer(vm, stream, (tINT)p, 16)) goto ERROR;
		goto RET;
	}
	CELL_SET_LOOP(p);
	// car
	CONS_GET_CAR(p, &obj);
	if (write_object(vm, stream, &obj)) goto ERROR;
	// cdr
	CONS_GET_CDR(p, &obj);
	switch (OBJECT_GET_TYPE(&obj)) {
	case OBJECT_NIL:
		break;
	case OBJECT_CONS:
		if (write_char(vm, stream, ' ')) goto ERROR;
		p=OBJECT_GET_CELL(&obj);
		goto LOOP;
	default:
		if (write_string(vm, stream, " . ")) goto ERROR;
		if (write_object(vm, stream, &obj)) goto ERROR;
	}
RET:
	cons_clear_loop(cons);
	return VM_OK;
ERROR:
	cons_clear_loop(cons);
	return VM_ERROR;
}

static void cons_clear_loop(tPCELL cons)
{
	for (; cons&&CELL_GET_LOOP(cons); cons=cons_get_cdr_cons(cons)) {
		CELL_RESET_LOOP(cons);
	}
}

/*
static VM_RET cons_write_(tPVM vm, tPCELL stream, tPCELL cons)
{
	if (!cons) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (CELL_GET_LOOP(cons)) {
		if (write_string(vm, stream, "#cons-loop")) return VM_ERROR;
		return write_integer(vm, stream, (tINT)cons, 16);
	} else {
		tOBJECT obj;
		CELL_SET_LOOP(cons);
		// car
		CONS_GET_CAR(cons, &obj);
		if (write_object(vm, stream, &obj)) goto ERROR;
		// cdr
		CONS_GET_CDR(cons, &obj);
		switch (OBJECT_GET_TYPE(&obj)) {
		case OBJECT_NIL:
			break;
		case OBJECT_CONS:
			if (write_char(vm, stream, ' ')) goto ERROR;
			if (cons_write_(vm, stream, OBJECT_GET_CELL(&obj))) goto ERROR;
			break;
		default:
			if (write_string(vm, stream, " . ")) goto ERROR;
			if (write_object(vm, stream, &obj)) goto ERROR;
		}
		CELL_RESET_LOOP(cons);
		return VM_OK;
	}
ERROR:
	CELL_RESET_LOOP(cons);
	return VM_ERROR;
}*/

VM_RET list_get_object(tPVM vm, tPCELL p, const tINT i, tPOBJECT obj)
{
	tINT x;
	tOBJECT tmp;
	for (x=0; x<i; x++) {
		CONS_GET_CDR(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
		p=OBJECT_GET_CELL(&tmp);
	}
	CONS_GET_CAR(p, obj);
	return VM_OK;
}

VM_RET list_set_object(tPVM vm, tPCELL p, const tINT i, tPOBJECT obj)
{
	tINT x;
	tOBJECT tmp;
	for (x=0; x<i; x++) {
		CONS_GET_CDR(p, &tmp);
		if (!OBJECT_IS_CONS(&tmp)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
		p=OBJECT_GET_CELL(&tmp);
	}
	if (CELL_GET_IMMUTABLE(p)) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
	CONS_SET_CAR(p, obj);
	return VM_OK;
}

tBOOL cons_equal(tPCELL cons1, tPCELL cons2)
{
	if (cons1==cons2) return tTRUE;
	{// loopしてるとOUT
		tOBJECT car1, car2, cdr1, cdr2;
LOOP:
		CONS_GET_CAR(cons1, &car1);
		CONS_GET_CDR(cons1, &cdr1);
		CONS_GET_CAR(cons2, &car2);
		CONS_GET_CDR(cons2, &cdr2);
		if (object_equal(&car1, &car2)) {
			if (OBJECT_IS_CONS(&cdr1)&&OBJECT_IS_CONS(&cdr2)) {
				cons1=OBJECT_GET_CELL(&cdr1);
				cons2=OBJECT_GET_CELL(&cdr2);
				if (cons1==cons2) return tTRUE;
				goto LOOP;
			} else {
				return object_equal(&cdr1, &cdr2) ? tTRUE : tFALSE;
			}
		} else {
			return tFALSE;
		}
/*		return (object_equal(&car1, &car2)&&object_equal(&cdr1, &cdr2))
			? tTRUE : tFALSE;*/
	}
}

VM_RET cons_mark(tPVM vm, tPCELL cell)
{
	tOBJECT obj;
	// スタックへ
	CONS_GET_CAR(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	CONS_GET_CDR(cell, &obj);
	return gc_push(vm, &obj);
}

/////////////////////////////////////////////////
// CELL_STRING

#define STRING_GET_SIZE(string)				(((string)+1)->ui)
#define STRING_SET_SIZE(string, size)		(((string)+1)->ui=(size))
#define STRING_GET_LENGTH(string)			(((string)+2)->i)
#define STRING_SET_LENGTH(string, len)		(((string)+2)->i=(len))
#define STRING_GET_NEXT(string)				(((string)+3)->cell)
#define STRING_SET_NEXT(string, next)		(((string)+3)->cell=(next))
#define STRING_GET_STRING(string)			((tCSTRING)((string)+4))
#define STRING_GET_DATA(string)				((tSTRING)((string)+4))

// リテラル文字列 変更不可能
VM_RET string_create(tPVM vm, tCSTRING string, tPCELL* cell)
{
	tINT len;
	tUINT s;
	// 長さの取得
	len=strlen((char*)string)+1;
	// VMからヒープの確保
	s=allocate_cell(vm, sizeof(tCELL)*4+sizeof(tCHAR)*len, cell);
	if (!s) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_STRING);
	// CELLの型の設定
	CELL_SET_TYPE(*cell, CELL_STRING);
	// この生成関数で作成される文字列はリテラル文字列
	CELL_SET_IMMUTABLE(*cell);
	// 各値の初期化
	STRING_SET_SIZE(*cell, s);
	STRING_SET_LENGTH(*cell, len);
	strcpy((char*)STRING_GET_STRING(*cell), string);
	STRING_SET_NEXT(*cell, 0);
	return VM_OK;
}

// create-stringで作成される文字列変更可能
VM_RET string_create_2(tPVM vm, const tINT len, tCHAR c, tPCELL* cell)
{
	tUINT s;
	s=allocate_cell(vm, sizeof(tCELL)*4+sizeof(tCHAR)*(len+1), cell);
	if (!s) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_STRING);
	CELL_SET_TYPE(*cell, CELL_STRING);
	STRING_SET_SIZE(*cell, s);
	STRING_SET_LENGTH(*cell, len+1);
	memset(STRING_GET_DATA(*cell), c, sizeof(tCHAR)*len);
	STRING_GET_DATA(*cell)[len]='\0';
	STRING_SET_NEXT(*cell, 0);
	return VM_OK;
}

VM_RET string_destroy(tPVM vm, tPCELL cell)
{
	return tisl_remove_string(vm_get_tisl(vm), vm, cell);
}

tUINT string_get_size(tPCELL string)
{
	return STRING_GET_SIZE(string);
}

tINT string_get_length(tPCELL string)
{
	return STRING_GET_LENGTH(string);
}

tCSTRING string_get_string(tPCELL string)
{
	return STRING_GET_STRING(string);
}

tSTRING string_get_data(tPCELL string)
{
	return STRING_GET_DATA(string);
}

tPCELL string_get_next(tPCELL string)
{
	return STRING_GET_NEXT(string);
}

void string_set_next(tPCELL string, tPCELL next)
{
	STRING_SET_NEXT(string, next);
}

VM_RET string_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tINT i, n=string_get_length(OBJECT_GET_CELL(obj))-1;
	tCSTRING string=string_get_string(OBJECT_GET_CELL(obj));
	if (vm_get_writer_flag(vm)) {
		if (write_char(vm, stream, '"')) return VM_ERROR;
		for (i=0; i<n; i++) {
			if (is_DBCS_lead_byte(vm, string[i])) {
				if (write_char(vm, stream, string[i])) return VM_ERROR;
				i++;
			} else if ((string[i]=='|')||(string[i]=='\\')||(string[i]=='"')) {
				if (write_char(vm, stream, '\\')) return VM_ERROR;
			}
			if (write_char(vm, stream, string[i])) return VM_ERROR;
		}
		if (write_char(vm, stream, '"')) return VM_ERROR;
	} else {
		for (i=0; i<n; i++) {
			if (is_DBCS_lead_byte(vm, string[i])) {
				if (write_char(vm, stream, string[i])) return VM_ERROR;
				i++;
			}
			if (write_char(vm, stream, string[i])) return VM_ERROR;
		}
	}
	return VM_OK;
}

VM_RET string_get_character(tPVM vm, tPCELL string, const tINT x, tPOBJECT ret)
{
	tINT n=STRING_GET_LENGTH(string);
	if ((x<0)||(x>=n)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
	OBJECT_SET_CHARACTER(ret, STRING_GET_STRING(string)[x]);
	return VM_OK;
}

VM_RET string_set_character(tPVM vm, tPCELL string, const tINT x, tCHAR c)
{
	tINT n=STRING_GET_LENGTH(string);
	if (CELL_GET_IMMUTABLE(string)) return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
	if ((x<0)||(x>=n)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
	STRING_GET_DATA(string)[x]=c;
	return VM_OK;
}

tBOOL string_equal(tPCELL string1, tPCELL string2)
{
	if (string1==string2) return tTRUE;
	if (string_get_length(string1)!=string_get_length(string2)) return tFALSE;
	return (strcmp(string_get_string(string1), string_get_string(string2))==0)
		? tTRUE : tFALSE;
}

VM_RET string_mark(tPVM vm, tPCELL cell)
{
//	return cell_mark(vm, STRING_GET_NEXT(cell));
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_SYMBOL

#define SYMBOL_COMPLETE					0x00000001
#define SYMBOL_COMPLETE_				0xfffffffe
#define SYMBOL_GENSYM					0x00000002
#define SYMBOL_GENSYM_					0xfffffffd

#define SYMBOL_GET_COMPLETE(symbol)		((symbol)->ui&SYMBOL_COMPLETE)
#define SYMBOL_SET_COMPLETE(symbol)		((symbol)->ui|=SYMBOL_COMPLETE)
#define SYMBOL_RESET_COMPLETE(symbol)	((symbol)->ui&=SYMBOL_COMPLETE_)
#define SYMBOL_GET_GENSYM(symbol)		((symbol)->ui&SYMBOL_GENSYM)
#define SYMBOL_SET_GENSYM(symbol)		((symbol)->ui|=SYMBOL_GENSYM)
#define SYMBOL_RESET_GENSYM(symbol)	((symbol)->ui&=SYMBOL_GENSYM_)

#define SYMBOL_GET_SIZE(symbol)			(((symbol)+1)->ui)
#define SYMBOL_SET_SIZE(symbol, size)	(((symbol)+1)->ui=(size))
#define SYMBOL_GET_LENGTH(symbol)		(((symbol)+2)->i)
#define SYMBOL_SET_LENGTH(symbol, len)	(((symbol)+2)->i=(len))
#define SYMBOL_GET_NEXT(symbol)			(((symbol)+3)->cell)
#define SYMBOL_SET_NEXT(symbol, next)	(((symbol)+3)->cell=(next))
#define SYMBOL_GET_KEY(symbol)			(((symbol)+4)->i)
#define SYMBOL_SET_KEY(symbol, key)		(((symbol)+4)->i=(key))
#define SYMBOL_GET_STRING(symbol)		((tPCELL*)((symbol)+5))

static VM_RET symbol_write_(tPVM vm, tPCELL stream, tPCELL string);

void symbol_set_key(tPCELL symbol, tINT key)
{
	SYMBOL_SET_KEY(symbol, key);
}

tINT symbol_get_key(tPCELL symbol)
{
	return SYMBOL_GET_KEY(symbol);
}

VM_RET symbol_create(tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell)
{
	tOBJECT obj;
	VM_RET ret;
	OBJECT_SET_CONS(&obj, list);
	if (vm_push(vm, &obj)) return VM_ERROR;
	ret=symbol_create_(vm, list, complete, cell);
	vm_pop(vm);
	return ret;
}

VM_RET symbol_create_(tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell)
{
	tINT i, len;
	tUINT size;
	tPCELL p;
	// 
	len=cons_get_length(list);
	size=allocate_cell(vm, sizeof(tCELL)*(len+5), cell);
	if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_SYMBOL);
	CELL_SET_TYPE(*cell, CELL_SYMBOL);
	if (complete) SYMBOL_SET_COMPLETE(*cell);
	else SYMBOL_RESET_COMPLETE(*cell);
	SYMBOL_SET_SIZE(*cell, size);
	SYMBOL_SET_LENGTH(*cell, len);
	SYMBOL_SET_NEXT(*cell, 0);
	for (i=0, p=list; p; i++, p=cons_get_cdr_cons(p)) {
		tOBJECT string;
		CONS_GET_CAR(p, &string);
		if (!OBJECT_IS_STRING(&string)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		SYMBOL_GET_STRING(*cell)[i]=OBJECT_GET_CELL(&string);
	}
	SYMBOL_RESET_GENSYM(*cell);
	return VM_OK;
}

VM_RET symbol_create_gensym(tPVM vm, tPCELL string, tPCELL* cell)
{
	tUINT size;
	size=allocate_cell(vm, sizeof(tCELL)*6, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_SYMBOL);
	CELL_SET_TYPE(*cell, CELL_SYMBOL);
	SYMBOL_SET_SIZE(*cell, size);
	SYMBOL_SET_LENGTH(*cell, 1);
	SYMBOL_GET_STRING(*cell)[0]=string;
	SYMBOL_SET_NEXT(*cell, 0);
	SYMBOL_SET_GENSYM(*cell);
	return VM_OK;
}

VM_RET symbol_create_simple(tPVM vm, tPCELL string, tBOOL complete, tPCELL* cell)
{
	tUINT size;
	size=allocate_cell(vm, sizeof(tCELL)*6, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_SYMBOL);
	CELL_SET_TYPE(*cell, CELL_SYMBOL);
	if (complete) SYMBOL_SET_COMPLETE(*cell);
	else SYMBOL_RESET_COMPLETE(*cell);
	SYMBOL_SET_SIZE(*cell, size);
	SYMBOL_SET_LENGTH(*cell, 1);
	SYMBOL_SET_NEXT(*cell, 0);
	SYMBOL_GET_STRING(*cell)[0]=string;
	SYMBOL_RESET_GENSYM(*cell);
	return VM_OK;
}


VM_RET symbol_destroy(tPVM vm, tPCELL symbol)
{
	return tisl_remove_symbol(vm_get_tisl(vm), vm, symbol);
}

tUINT symbol_get_size(tPCELL symbol)
{
	return SYMBOL_GET_SIZE(symbol);
}

tINT symbol_get_length(tPCELL symbol)
{
	return SYMBOL_GET_LENGTH(symbol);
}

tPCELL symbol_get_next(tPCELL symbol)
{
	return SYMBOL_GET_NEXT(symbol);
}

void symbol_set_next(tPCELL symbol, tPCELL next)
{
	SYMBOL_SET_NEXT(symbol, next);
}

tBOOL symbol_is_gensym(tPCELL symbol)
{
	return SYMBOL_GET_GENSYM(symbol) ? tTRUE : tFALSE;
}

tBOOL symbol_is_complete(tPCELL symbol)
{
	return SYMBOL_GET_COMPLETE(symbol) ? tTRUE : tFALSE;
}

tBOOL symbol_is_simple(tPCELL symbol)
{
	return (symbol_get_length(symbol)==1) ? tTRUE : tFALSE;
}

tBOOL symbol_get_string(tPCELL symbol, const tINT i, tPCELL* string)
{
	if ((i<0)||(SYMBOL_GET_LENGTH(symbol)<=i)) return tFALSE;
	*string=SYMBOL_GET_STRING(symbol)[i];
	return tTRUE;
}

VM_RET symbol_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tINT i, len;
	if (symbol_is_complete(OBJECT_GET_CELL(obj))&&
		write_char(vm, stream, ':')) return VM_ERROR;
	len=symbol_get_length(OBJECT_GET_CELL(obj));
	for (i=0; i<len; i++) {
		tPCELL string;
		if (!symbol_get_string(OBJECT_GET_CELL(obj), i, &string)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		if (symbol_write_(vm, stream, string)) return VM_ERROR;
		if ((i!=len-1)&&write_char(vm, stream, ':')) return VM_ERROR;
	}
	return VM_OK;
}

static VM_RET symbol_write_(tPVM vm, tPCELL stream, tPCELL string)
{
	tCSTRING p=string_get_string(string);
	tINT i, len=string_get_length(string)-1;
	if (vm_get_writer_flag(vm)) {
		if (write_char(vm, stream, '|')) return VM_ERROR;
		for (i=0; i<len; i++) {
			if (is_DBCS_lead_byte(vm, p[i])) {
				if (write_char(vm, stream, p[i])) return VM_ERROR;
				i++;
			} else if ((p[i]=='|')||(p[i]=='\\')) {
				if (write_char(vm, stream, '\\')) return VM_ERROR;
			}
			if (write_char(vm, stream, p[i])) return VM_ERROR;
		}
		if (write_char(vm, stream, '|')) return VM_ERROR;
	} else {
		for (i=0; i<len; i++) {
			if (is_DBCS_lead_byte(vm, p[i])) {
				if (write_char(vm, stream, p[i])) return VM_ERROR;
				i++;
			}
			if (write_char(vm, stream, p[i])) return VM_ERROR;
		}
	}
	return VM_OK;
}

VM_RET symbol_mark(tPVM vm, tPCELL cell)
{
	tINT i, n;
	tPCELL* p;
	n=SYMBOL_GET_LENGTH(cell);
	p=SYMBOL_GET_STRING(cell);
	for (i=0; i<n; i++) {
		if (cell_mark(vm, *p++)) return VM_ERROR;
	}
	return VM_OK;
//	return cell_mark(vm, SYMBOL_GET_NEXT(cell));
}

/////////////////////////////////////////////////
// CELL_VECTOR

#define VECTOR_GET_SIZE(vector)			(((vector)+1)->ui)
#define VECTOR_SET_SIZE(vector, size)	(((vector)+1)->ui=(size))
#define VECTOR_GET_LENGTH(vector)		(((vector)+2)->i)
#define VECTOR_SET_LENGTH(vector, len)	(((vector)+2)->i=(len))
#define VECTOR_GET_HEAD(vector)			((tPOBJECT)((vector)+3))

VM_RET vector_create(tPVM vm, tPCELL list, tPCELL* cell)
{
	VM_RET ret;
	if (list) {
		tOBJECT obj;
		if (!CELL_IS_CONS(list)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		OBJECT_SET_CONS(&obj, list);
		if (vm_push(vm, &obj)) return VM_ERROR;
	}
	ret=vector_create_(vm, list, cell);
	if (list) vm_pop(vm);
	return ret;
}

VM_RET vector_create_(tPVM vm, tPCELL list, tPCELL* cell)
{
	tINT len;
	tUINT size;
	tPCELL p;
	tPOBJECT head;

	len=cons_get_length(list);
	size=allocate_cell(vm, sizeof(tCELL)*3+sizeof(tOBJECT)*len, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_VECTOR);
	CELL_SET_TYPE(*cell, CELL_VECTOR);
	CELL_SET_IMMUTABLE(*cell);
	VECTOR_SET_SIZE(*cell, size);
	VECTOR_SET_LENGTH(*cell, len);
	head=VECTOR_GET_HEAD(*cell);
	for (p=list; p; p=cons_get_cdr_cons(p)) {
		CONS_GET_CAR(p, head);
		head++;
	}
	return VM_OK;
}

VM_RET vector_create_2_(tPVM vm, const tINT len, tPOBJECT init, tPCELL* cell)
{
	tINT i;
	tUINT size;
	tPOBJECT head;

	size=allocate_cell(vm, sizeof(tCELL)*3+sizeof(tOBJECT)*len, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_VECTOR);
	CELL_SET_TYPE(*cell, CELL_VECTOR);
	VECTOR_SET_SIZE(*cell, size);
	VECTOR_SET_LENGTH(*cell, len);
	head=VECTOR_GET_HEAD(*cell);
	for (i=0; i<len; i++) {
		*head++=*init;
	}
	return VM_OK;
}

tUINT vector_get_size(tPCELL vector)
{
	return VECTOR_GET_SIZE(vector);
}

tINT vector_get_length(tPCELL vector)
{
	return VECTOR_GET_LENGTH(vector);
}

VM_RET vector_get_object(tPVM vm, tPCELL vector, const tINT i, tPOBJECT obj)
{
	if ((i<0)||(i>=VECTOR_GET_LENGTH(vector)))
		return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
	*obj=VECTOR_GET_HEAD(vector)[i];
	return VM_OK;
}

VM_RET vector_set_object(tPVM vm, tPCELL vector, const tINT i, tPOBJECT obj)
{
	if ((i<0)||(i>=VECTOR_GET_LENGTH(vector)))
		return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
	VECTOR_GET_HEAD(vector)[i]=*obj;
	return VM_OK;
}

VM_RET vector_write(tPVM vm, tPCELL stream, tPOBJECT vec)
{
	tINT i, n;
	tOBJECT obj;

	n=vector_get_length(OBJECT_GET_CELL(vec));
	if (write_string(vm, stream, "#(")) return VM_ERROR;
	for (i=0; i<n; i++) {
		if (vector_get_object(vm, OBJECT_GET_CELL(vec), i, &obj)||
			write_object(vm, stream, &obj)||
			((i!=n-1)&&write_char(vm, stream, ' '))) return VM_ERROR;
	}
	return write_char(vm, stream, ')');
}

tBOOL vector_equal(tPCELL vec1, tPCELL vec2)
{
	if (vec1==vec2) return tTRUE;
	if (vector_get_length(vec1)!=vector_get_length(vec2)) return tFALSE;
	{
		tINT i, n=vector_get_length(vec1);
		for (i=0; i<n; i++) {
			if (!object_equal(VECTOR_GET_HEAD(vec1)+i, VECTOR_GET_HEAD(vec2)+i))
				return tFALSE;
		}
		return tTRUE;
	}
}

VM_RET vector_mark(tPVM vm, tPCELL cell)
{
	tINT i, n;
	n=VECTOR_GET_LENGTH(cell);
	for (i=0; i<n; i++) {
		if (gc_push(vm, VECTOR_GET_HEAD(cell)+i)) return VM_ERROR;
	}
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_ARRAY

#define ARRAY_GET_SIZE(array)			(((array)+1)->ui)
#define ARRAY_SET_SIZE(array, size)		(((array)+1)->ui=(size))
#define ARRAY_GET_DIMENSION(array)		(((array)+2)->i)
#define ARRAY_SET_DIMENSION(array, d)	(((array)+2)->i=(d))
#define ARRAY_GET_DIMENSION_HEAD(array)	((array)+3)
#define ARRAY_GET_OBJECT_HEAD(array)	((tPOBJECT)((array)+3+ARRAY_GET_DIMENSION(array)))

static tINT array_get_object_number(tPCELL list, const int d, const int max);
static VM_RET array_create_set_object(tPVM vm, tPCELL array, tPOBJECT head, tPCELL list, const int d);
static tINT array_get_dimension_number(tPCELL array, const int d);
static VM_RET array_write_(tPVM vm, tPCELL stream, tPCELL array, tPOBJECT head, const tINT c);

VM_RET array_create(tPVM vm, const tINT d, tPOBJECT list, tPCELL* cell)
{
	VM_RET ret;
	if (vm_push(vm, list)) return VM_ERROR;
	ret=array_create_(vm, d, list, cell);
	vm_pop(vm);
	return ret;
}

VM_RET array_create_(tPVM vm, const tINT d, tPOBJECT list, tPCELL* cell)
{
	if (d) {
		tINT i, n;
		tUINT size;
		tPCELL dimension;
		tPCELL p;

		if (!OBJECT_IS_CONS(list)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		n=array_get_object_number(OBJECT_GET_CELL(list), 1, d);
		size=allocate_cell(vm, sizeof(tCELL)*(3+d)+sizeof(tOBJECT)*n, cell);
		if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_ARRAY);
		CELL_SET_TYPE(*cell, CELL_ARRAY);
		CELL_SET_IMMUTABLE(*cell);
		ARRAY_SET_SIZE(*cell, size);
		ARRAY_SET_DIMENSION(*cell, d);
		// 次元要素の計算
		dimension=ARRAY_GET_DIMENSION_HEAD(*cell);
		dimension[0].i=n;
		for (p=OBJECT_GET_CELL(list), i=1; i<d; i++, p=cons_get_car_cons(p)) {
			tINT len;
			if (!p) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
			len=cons_get_length(p);
			if (!len) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
			dimension[i].i=dimension[i-1].i / len;
		}
		// 配列要素の初期化
		if (array_create_set_object(vm, *cell, ARRAY_GET_OBJECT_HEAD(*cell), OBJECT_GET_CELL(list), 1)) return VM_ERROR;
	} else { // d==0
		tUINT size;

		size=allocate_cell(vm, sizeof(tCELL)*3+sizeof(tOBJECT), cell);
		if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_ARRAY);
		CELL_SET_TYPE(*cell, CELL_ARRAY);
		CELL_SET_IMMUTABLE(*cell);
		ARRAY_SET_SIZE(*cell, size);
		ARRAY_SET_DIMENSION(*cell, 0);
		ARRAY_GET_OBJECT_HEAD(*cell)[0]=*list;
	}
	return VM_OK;
}

VM_RET array_create_2_(tPVM vm, tPCELL dimensions, tPOBJECT init, tPCELL* cell)
{
	if (dimensions) {
		tINT d;
		d=cons_get_length(dimensions);
		if (d==1) {// 1次元配列 ベクトル
			tOBJECT obj;
			tINT dd;
			CONS_GET_CAR(dimensions, &obj);
			if (!OBJECT_IS_INTEGER(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &obj);
			dd=OBJECT_GET_INTEGER(&obj);
			if (dd<0) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &obj);
			// ベクトルとして初期化
			return vector_create_2_(vm, dd, init, cell);
		} else {// 配列
			tINT i, j, n, dd;
			tUINT size;
			tPCELL p;
			tOBJECT obj;
			for (p=dimensions, n=1; p; p=cons_get_cdr_cons(p)/*ドットリストは無視*/) {
				CONS_GET_CAR(p, &obj);
				if (!OBJECT_IS_INTEGER(&obj)||
					(OBJECT_GET_INTEGER(&obj)<=0)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &obj);
				n*=OBJECT_GET_INTEGER(&obj);
			}
			size=allocate_cell(vm, sizeof(tCELL)*(3+d)+sizeof(tOBJECT)*n, cell);
			if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_ARRAY);
			CELL_SET_TYPE(*cell, CELL_ARRAY);
			ARRAY_SET_SIZE(*cell, size);
			// 次元データ
			ARRAY_SET_DIMENSION(*cell, d);
			j=n;
			for (p=dimensions, i=0; i<d-1; i++, p=cons_get_cdr_cons(p)) {
				CONS_GET_CAR(p, &obj);
				dd=OBJECT_GET_INTEGER(&obj);
				ARRAY_GET_DIMENSION_HEAD(*cell)[i].i=j;
				j/=dd;
			}
			CONS_GET_CAR(p, &obj);
			ARRAY_GET_DIMENSION_HEAD(*cell)[i].i=OBJECT_GET_INTEGER(&obj);
			// 各要素の初期化
			for (i=0; i<n; i++) {
				ARRAY_GET_OBJECT_HEAD(*cell)[i]=*init;
			}
		}
	} else {// 0次元配列
		tUINT size;
		size=allocate_cell(vm, sizeof(tCELL)*3+sizeof(tOBJECT), cell);
		if (!size) return signal_condition(vm, TISL_ERROR_CANNOT_CREATE_ARRAY);
		CELL_SET_TYPE(*cell, CELL_ARRAY);
		ARRAY_SET_SIZE(*cell, size);
		ARRAY_SET_DIMENSION(*cell, 0);
		ARRAY_GET_OBJECT_HEAD(*cell)[0]=*init;
	}
	return VM_OK;
}

static tINT array_get_object_number(tPCELL list, const int d, const int max)
{
	if (!list) return 0;
	if (d==max) {
		return cons_get_length(list);
	} else {
		int s=array_get_object_number(cons_get_car_cons(list), d+1, max);
		return s*cons_get_length(list);
	}
}

static VM_RET array_create_set_object(tPVM vm, tPCELL array, tPOBJECT head, tPCELL list, const int d)
{
	tINT i, n;
	tPCELL p;
	n=array_get_dimension_number(array, d);
	if (!list) return signal_condition(vm, TISL_ERROR_CONVERT_ERROR_ARRAY);
	if (d==ARRAY_GET_DIMENSION(array)) {
		tOBJECT obj;
		for (i=0, p=list; i<n; i++, p=cons_get_cdr_cons(p)) {
			if (!p) return signal_condition(vm, TISL_ERROR_CONVERT_ERROR_ARRAY);
			CONS_GET_CAR(p, &obj);
			head[i]=obj;
		}
	} else {
		tOBJECT obj;
		for (i=0, p=list; i<n; i++, p=cons_get_cdr_cons(p)) {
			if (!p) return signal_condition(vm, TISL_ERROR_CONVERT_ERROR_ARRAY);
			CONS_GET_CAR(p, &obj);
			if (!OBJECT_IS_CONS(&obj)) return signal_condition(vm, TISL_ERROR_CONVERT_ERROR_ARRAY);
			if (array_create_set_object(vm, array, head+ARRAY_GET_DIMENSION_HEAD(array)[d].i*i, cons_get_car_cons(p), d+1)) return VM_ERROR;
		}
	}
	if (p) return signal_condition(vm, TISL_ERROR_CONVERT_ERROR_ARRAY);
	return VM_OK;
}

static tINT array_get_dimension_number(tPCELL array, const int d)
{
	tINT dimension=ARRAY_GET_DIMENSION(array);
	tPCELL head=ARRAY_GET_DIMENSION_HEAD(array);
	if (dimension==d) {
		return head[d-1].i;
	} else {
		return head[d-1].i / head[d].i;
	}
}

tUINT array_get_size(tPCELL array)
{
	return ARRAY_GET_SIZE(array);
}

tINT array_get_dimension(tPCELL array)
{
	return ARRAY_GET_DIMENSION(array);
}

VM_RET array_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tINT d=array_get_dimension(OBJECT_GET_CELL(obj));
	if (write_char(vm, stream, '#')||
		write_integer(vm, stream, d, 10)||
		write_char(vm, stream, 'a')) return VM_ERROR;
	if (d) {
		return array_write_(vm, stream, OBJECT_GET_CELL(obj), ARRAY_GET_OBJECT_HEAD(OBJECT_GET_CELL(obj)), 0);
	} else {
		return write_object(vm, stream, ARRAY_GET_OBJECT_HEAD(OBJECT_GET_CELL(obj)));
	}
}

static VM_RET array_write_(tPVM vm, tPCELL stream, tPCELL array, tPOBJECT head, const tINT c)
{
	tINT i, d, dd;
	if (write_char(vm, stream, '(')) return VM_ERROR;
	if (c==array_get_dimension(array)-1) {
		dd=ARRAY_GET_DIMENSION_HEAD(array)[c].i;
		for (i=0; i<dd; i++) {
			if (write_object(vm, stream, head+i)||
				((i!=dd-1)&&write_char(vm, stream, ' '))) return VM_ERROR;
		}
	} else {
		d=ARRAY_GET_DIMENSION_HEAD(array)[c+1].i;
		dd=ARRAY_GET_DIMENSION_HEAD(array)[c].i/d;
		for (i=0; i<dd; i++) {
			if (array_write_(vm, stream, array, head+d*i, c+1)) return VM_ERROR;
		}
	}
	return write_char(vm, stream, ')');
}

VM_RET array_get_object(tPVM vm, tPCELL array, const tINT d, tPOBJECT ret)
{
	if (ARRAY_GET_DIMENSION(array)!=d) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (d>1) {
		tINT i, d2, d3, d4, d5;
		d2=1;
		d3=0;
		for (i=d-1; i>=0; i--) {
			if (!OBJECT_IS_INTEGER(vm->SP-d+i+1)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, vm->SP-d+i+1);
			d4=OBJECT_GET_INTEGER(vm->SP-d+i+1);
			if (i==d-1) {
				d5=ARRAY_GET_DIMENSION_HEAD(array)[i].i;
			} else {
				d5=ARRAY_GET_DIMENSION_HEAD(array)[i].i /
				   ARRAY_GET_DIMENSION_HEAD(array)[i+1].i;
			}
			if ((d4<0)||(d4>d5)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
			d3+=d2*d4;
			d2=ARRAY_GET_DIMENSION_HEAD(array)[i].i;
		}
		*ret=ARRAY_GET_OBJECT_HEAD(array)[d3];
	} else if (d==1) {// ベクタはないはず
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	} else {// d==0
		*ret=ARRAY_GET_OBJECT_HEAD(array)[0];
	}
	return VM_OK;
}

VM_RET array_set_object(tPVM vm, tPCELL array, const tINT d, tPOBJECT obj)
{
	if (ARRAY_GET_DIMENSION(array)!=d) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (d>1) {
		tINT i, d2, d3, d4, d5;
		d2=1;
		d3=0;
		for (i=d-1; i>=0; i--) {
			if (!OBJECT_IS_INTEGER(vm->SP-d+i+1)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, vm->SP-d+i+1);
			d4=OBJECT_GET_INTEGER(vm->SP-d+i+1);
			if (i==d-1) {
				d5=ARRAY_GET_DIMENSION_HEAD(array)[i].i;
			} else {
				d5=ARRAY_GET_DIMENSION_HEAD(array)[i].i /
				   ARRAY_GET_DIMENSION_HEAD(array)[i+1].i;
			}
			if ((d4<0)||(d4>d5)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
			d3+=d2*d4;
			d2=ARRAY_GET_DIMENSION_HEAD(array)[i].i;
		}
		ARRAY_GET_OBJECT_HEAD(array)[d3]=*obj;
	} else if (d==1) {// ベクタはないはず
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	} else {// d==0
		ARRAY_GET_OBJECT_HEAD(array)[0]=*obj;
	}
	return VM_OK;
}

VM_RET array_get_dimension_list(tPVM vm, tPCELL array, tPOBJECT list)
{
	tPCELL p;
	tOBJECT obj, obj2;
	tINT i, d=ARRAY_GET_DIMENSION(array);
	OBJECT_SET_INTEGER(&obj, ARRAY_GET_DIMENSION_HEAD(array)[d-1].i);
	if (cons_create_(vm, &p, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, p);
	for (i=d-2; i>=0; i--) {
		OBJECT_SET_INTEGER(&obj2, ARRAY_GET_DIMENSION_HEAD(array)[i].i/ARRAY_GET_DIMENSION_HEAD(array)[i+1].i);
		if (cons_create(vm, &p, &obj2, &obj)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, p);
	}
	*list=obj;
	return VM_OK;
}

tBOOL array_equal(tPCELL array1, tPCELL array2)
{
	if (array1==array2) return tTRUE;
	if (array_get_dimension(array1)!=array_get_dimension(array2)) return tFALSE;
	if (ARRAY_GET_DIMENSION_HEAD(array1)[0].i!=ARRAY_GET_DIMENSION_HEAD(array2)[0].i) return tFALSE;
	{
		tINT i, n=ARRAY_GET_DIMENSION_HEAD(array1)[0].i;
		for (i=0; i<n; i++) {
			if (!object_equal(ARRAY_GET_OBJECT_HEAD(array1)+i, ARRAY_GET_OBJECT_HEAD(array2)+i)) return tFALSE;
		}
		return tTRUE;
	}
}

VM_RET array_mark(tPVM vm, tPCELL cell)
{
	if (array_get_dimension(cell)) {
		tINT i, n=ARRAY_GET_DIMENSION_HEAD(cell)[0].i;
		for (i=0; i<n; i++) {
			if (gc_push(vm, ARRAY_GET_OBJECT_HEAD(cell)+i)) return VM_ERROR;
		}
	} else {
		if (gc_push(vm, ARRAY_GET_OBJECT_HEAD(cell))) return VM_ERROR;
	}
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_QUASIQUOTE

#define QUASIQUOTE_SIZE		CELL_UNIT

#define QUASIQUOTE_GET_TYPE(quote)			(((quote)+1)->ui)
#define QUASIQUOTE_SET_TYPE(quote, type)	(((quote)+1)->ui=(type))
#define QUASIQUOTE_GET_DATA(quote)			((quote)[2])
#define QUASIQUOTE_SET_DATA(quote, data)	((quote)[2]=(data))
#define QUASIQUOTE_GET_FORM(quote, obj)		(OBJECT_SET_TYPE(obj, QUASIQUOTE_GET_TYPE(quote)), (obj)->data=QUASIQUOTE_GET_DATA(quote))
#define QUASIQUOTE_SET_FORM(quote, obj)		(QUASIQUOTE_SET_TYPE(quote, OBJECT_GET_TYPE(obj)), QUASIQUOTE_SET_DATA(quote, (obj)->data))

VM_RET quasiquote_create(tPVM vm, tPOBJECT form, tPCELL* cell)
{
	VM_RET ret;
	if (vm_push(vm, form)) return VM_ERROR;
	ret=quasiquote_create_(vm, form, cell);
	vm_pop(vm);
	return ret;
}

VM_RET quasiquote_create_(tPVM vm, tPOBJECT form, tPCELL* cell)
{
	if (!allocate_cell(vm, sizeof(tCELL)*CELL_UNIT, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_QUASIQUOTE);
	QUASIQUOTE_SET_FORM(*cell, form);
	return VM_OK;
}

tUINT quasiquote_get_size(tPCELL quasiquote)
{
	return QUASIQUOTE_SIZE;
}

void quasiquote_get_form(tPCELL quote, tPOBJECT form)
{
	QUASIQUOTE_GET_FORM(quote, form);
}

VM_RET quasiquote_write(tPVM vm, tPCELL stream, tPOBJECT quote)
{
	tOBJECT form;
	if (write_char(vm, stream, '`')) return VM_ERROR;
	QUASIQUOTE_GET_FORM(OBJECT_GET_CELL(quote), &form);
	return write_object(vm, stream, &form);
}

VM_RET quasiquote_mark(tPVM vm, tPCELL cell)
{
	tOBJECT obj;
	QUASIQUOTE_GET_FORM(cell, &obj);
	return gc_push(vm, &obj);
}

tBOOL quasiquote_equal(tPCELL quote1, tPCELL quote2)
{
	tOBJECT obj1, obj2;
	QUASIQUOTE_GET_FORM(quote1, &obj1);
	QUASIQUOTE_GET_FORM(quote2, &obj2);
	return object_equal(&obj1, &obj2);
}

/////////////////////////////////////////////////
// CELL_UNQUOTE

#define UNQUOTE_SIZE		CELL_UNIT

#define UNQUOTE_GET_TYPE(quote)			(((quote)+1)->ui)
#define UNQUOTE_SET_TYPE(quote, type)	(((quote)+1)->ui=(type))
#define UNQUOTE_GET_DATA(quote)			((quote)[2])
#define UNQUOTE_SET_DATA(quote, data)	((quote)[2]=(data))
#define UNQUOTE_GET_FORM(quote, obj)	(OBJECT_SET_TYPE(obj, UNQUOTE_GET_TYPE(quote)), (obj)->data=UNQUOTE_GET_DATA(quote))
#define UNQUOTE_SET_FORM(quote, obj)	(UNQUOTE_SET_TYPE(quote, OBJECT_GET_TYPE(obj)), UNQUOTE_SET_DATA(quote, (obj)->data))

VM_RET unquote_create(tPVM vm, tPOBJECT form, tPCELL* cell)
{
	VM_RET ret;
	if (vm_push(vm, form)) return VM_ERROR;
	ret=unquote_create_(vm, form, cell);
	vm_pop(vm);
	return ret;
}

VM_RET unquote_create_(tPVM vm, tPOBJECT form, tPCELL* cell)
{
	if (!allocate_cell(vm, sizeof(tCELL)*CELL_UNIT, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_UNQUOTE);
	UNQUOTE_SET_FORM(*cell, form);
	return VM_OK;
}

tUINT unquote_get_size(tPCELL unquote)
{
	return UNQUOTE_SIZE;
}

void unquote_get_form(tPCELL quote, tPOBJECT form)
{
	UNQUOTE_GET_FORM(quote, form);
}

VM_RET unquote_write(tPVM vm, tPCELL stream, tPOBJECT quote)
{
	tOBJECT form;
	if (write_char(vm, stream, ',')) return VM_ERROR;
	UNQUOTE_GET_FORM(OBJECT_GET_CELL(quote), &form);
	return write_object(vm, stream, &form);
}

VM_RET unquote_mark(tPVM vm, tPCELL cell)
{
	tOBJECT obj;
	UNQUOTE_GET_FORM(cell, &obj);
	return gc_push(vm, &obj);
}

tBOOL unquote_equal(tPCELL quote1, tPCELL quote2)
{
	tOBJECT obj1, obj2;
	UNQUOTE_GET_FORM(quote1, &obj1);
	UNQUOTE_GET_FORM(quote2, &obj2);
	return object_equal(&obj1, &obj2);
}

/////////////////////////////////////////////////
// CELL_UNQUOTE_SPLICING

#define UNQUOTE_SPLICING_SIZE		CELL_UNIT

#define UNQUOTE_SPLICING_GET_TYPE(quote)		(((quote)+1)->ui)
#define UNQUOTE_SPLICING_SET_TYPE(quote, type)	(((quote)+1)->ui=(type))
#define UNQUOTE_SPLICING_GET_DATA(quote)		((quote)[2])
#define UNQUOTE_SPLICING_SET_DATA(quote, data)	((quote)[2]=(data))
#define UNQUOTE_SPLICING_GET_FORM(quote, obj)	(OBJECT_SET_TYPE(obj, UNQUOTE_SPLICING_GET_TYPE(quote)), (obj)->data=UNQUOTE_SPLICING_GET_DATA(quote))
#define UNQUOTE_SPLICING_SET_FORM(quote, obj)	(UNQUOTE_SPLICING_SET_TYPE(quote, OBJECT_GET_TYPE(obj)), UNQUOTE_SPLICING_SET_DATA(quote, (obj)->data))

VM_RET unquote_splicing_create(tPVM vm, tPOBJECT form, tPCELL* cell)
{
	VM_RET ret;
	if (vm_push(vm, form)) return VM_ERROR;
	ret=unquote_splicing_create_(vm, form, cell);
	vm_pop(vm);
	return ret;
}

VM_RET unquote_splicing_create_(tPVM vm, tPOBJECT form, tPCELL* cell)
{
	if (!allocate_cell(vm, sizeof(tCELL)*CELL_UNIT, cell)) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_UNQUOTE_SPLICING);
	UNQUOTE_SPLICING_SET_FORM(*cell, form);
	return VM_OK;
}

tUINT unquote_splicing_get_size(tPCELL unquote_splicing)
{
	return UNQUOTE_SPLICING_SIZE;
}

void unquote_splicing_get_form(tPCELL quote, tPOBJECT form)
{
	UNQUOTE_SPLICING_GET_FORM(quote, form);
}

VM_RET unquote_splicing_write(tPVM vm, tPCELL stream, tPOBJECT quote)
{
	tOBJECT form;
	if (write_string(vm, stream, ",@")) return VM_ERROR;
	UNQUOTE_SPLICING_GET_FORM(OBJECT_GET_CELL(quote), &form);
	return write_object(vm, stream, &form);
}

VM_RET unquote_splicing_mark(tPVM vm, tPCELL cell)
{
	tOBJECT form;
	UNQUOTE_SPLICING_GET_FORM(cell, &form);
	return gc_push(vm, &form);
}

tBOOL unquote_splicing_equal(tPCELL quote1, tPCELL quote2)
{
	tOBJECT obj1, obj2;
	UNQUOTE_SPLICING_GET_FORM(quote1, &obj1);
	UNQUOTE_SPLICING_GET_FORM(quote2, &obj2);
	return object_equal(&obj1, &obj2);
}

/////////////////////////////////////////////////
// stream

tBOOL check_eos(tPCELL stream)
{
	if (CELL_IS_STRING_STREAM(stream)) {
		return string_stream_check_eos(stream);
	} else if (CELL_IS_FILE_STREAM(stream)) {
		return file_stream_check_eos(stream);
	} else {
		return tFALSE;
	}
}

tINT stream_get_x(tPCELL stream)
{
	if (CELL_IS_STRING_STREAM(stream)) {
		return string_stream_get_x(stream);
	} else if (CELL_IS_FILE_STREAM(stream)) {
		return file_stream_get_x(stream);
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////
// CELL_STRING_STREAM

/////////////////////////////
// string buffer

struct tBUFFER_ {
	tCHAR*		buffer;
	tINT		size;
	tINT		data_size;

	tINT		position;
	tINT		x;
	tINT		y;

	tPVM		lock[2];

	tPBUFFER	next;
};

#define BUFFER_SIZE_UNIT				(8*1024)

VM_RET string_buffer_create(tPVM vm, tPBUFFER* b);
tPBUFFER string_buffer_get_next(tPBUFFER b);
void string_buffer_set_next(tPBUFFER b, tPBUFFER next);
static tBOOL buffer_check_eos(tPBUFFER b);
static tINT buffer_get_x(tPBUFFER b);
static void buffer_clear(tPBUFFER b);
static tBOOL buffer_read_char(tPVM vm, tPBUFFER b, tPCHAR c);
static tBOOL buffer_preview_char(tPVM vm, tPBUFFER b, tPCHAR c);
static VM_RET buffer_write_char(tPVM vm, tPBUFFER b, tCHAR c);
static void buffer_increment_position(tPBUFFER b, tCHAR c);
static VM_RET buffer_extend(tPVM vm, tPBUFFER b);
static VM_RET buffer_initialize(tPVM vm, tPBUFFER b, tCSTRING string);

///////////////////

VM_RET string_buffer_create(tPVM vm, tPBUFFER* b)
{
	*b=malloc(sizeof(tBUFFER));
	if (!*b) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	(*b)->buffer=0;
	(*b)->size=0;
	(*b)->data_size=0;
	(*b)->position=0;
	(*b)->x=0;
	(*b)->y=0;
	(*b)->next=0;
	(*b)->lock[0]=0;
	(*b)->lock[1]=0;
	return VM_OK;
}

// 自分自身に使用しているメモリの開放
void string_buffer_free(tPBUFFER buffer)
{
	if (buffer) {
		free(buffer->buffer);
		string_buffer_free(buffer->next);
		free(buffer);
	}
}

tPBUFFER string_buffer_get_next(tPBUFFER b)
{
	return b->next;
}

void string_buffer_set_next(tPBUFFER b, tPBUFFER next)
{
	b->next=next;
}

static tBOOL buffer_check_eos(tPBUFFER b)
{
	return (b->data_size<=b->position) ? tTRUE : tFALSE;
}

static tINT buffer_get_x(tPBUFFER b)
{
	return b->x;
}

static void buffer_clear(tPBUFFER b)
{
	b->position=0;
	b->data_size=0;
	b->x=0;
	b->y=0;
}

static tBOOL buffer_read_char(tPVM vm, tPBUFFER b, tPCHAR c)
{
	if (b->data_size<=b->position) {
		return tFALSE;
	} else {
		*c=b->buffer[b->position];
		buffer_increment_position(b, *c);
		return tTRUE;
	}
}

static tBOOL buffer_preview_char(tPVM vm, tPBUFFER b, tPCHAR c)
{
	if (b->data_size<=b->position) {
		return tFALSE;
	} else {
		*c=b->buffer[b->position];
		return tTRUE;
	}
}

static VM_RET buffer_write_char(tPVM vm, tPBUFFER b, tCHAR c)
{
	if ((b->size==b->position)&&
		buffer_extend(vm, b)) return VM_ERROR;
	b->buffer[b->position]=c;
	b->data_size++;
	buffer_increment_position(b, c);
	return VM_OK;
}

static void buffer_increment_position(tPBUFFER b, tCHAR c)
{
	b->position++;
	if (c=='\n') {
		b->x=0;
		b->y++;
	} else {
		b->x++;
	}
}

static VM_RET buffer_extend(tPVM vm, tPBUFFER b)
{
	tINT size=b->size+BUFFER_SIZE_UNIT;
	tPCHAR new_buffer;
	new_buffer=malloc(sizeof(tCHAR)*size);
	if (!new_buffer) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	if (b->size) {
		memcpy(new_buffer, b->buffer, sizeof(tCHAR)*b->size);
		free(b->buffer);
	}
	b->buffer=new_buffer;
	b->size=size;
	return VM_OK;
}

static VM_RET buffer_initialize(tPVM vm, tPBUFFER b, tCSTRING string)
{
	tINT size;
	buffer_clear(b);
	size=strlen(string);
	while (size>b->size) {
		if (buffer_extend(vm, b)) return VM_ERROR;
	}
	memcpy(b->buffer, string, sizeof(tCHAR)*size);
	b->data_size=size;
	return VM_OK;
}


// string buffer
/////////////////////////////
// CELL_STRING_STREAM

#define STRING_STREAM_GET_INPUT(stream)			((stream)->ui&STREAM_INPUT)
#define STRING_STREAM_SET_INPUT(stream)			((stream)->ui|=STREAM_INPUT)
#define STRING_STREAM_RESET_INPUT(stream)		((stream)->ui&=STREAM_INPUT_)
#define STRING_STREAM_GET_OUTPUT(stream)		((stream)->ui&STREAM_OUTPUT)
#define STRING_STREAM_SET_OUTPUT(stream)		((stream)->ui|=STREAM_OUTPUT)
#define STRING_STREAM_RESET_OUTPUT(stream)		((stream)->ui&=STREAM_OUTPUT_)
#define STRING_STREAM_GET_CLOSED(stream)		((stream)->ui&STREAM_CLOSED)
#define STRING_STREAM_SET_CLOSED(stream)		((stream)->ui|=STREAM_CLOSED)
#define STRING_STREAM_RESET_CLOSED(stream)		((stream)->ui&=STREAM_CLOSED_)

#define STRING_STREAM_GET_SIZE(stream)			(((stream)+1)->ui)
#define STRING_STREAM_SET_SIZE(stream, size)	(((stream)+1)->ui=(size))
#define STRING_STREAM_GET_BUFFER(stream)		((tPBUFFER)((stream)+2)->p)
#define STRING_STREAM_SET_BUFFER(stream, b)		(((stream)+2)->p=(b))

VM_RET string_stream_create(tPVM vm, const tINT io_flag, tPCELL* cell);

VM_RET string_stream_create_input(tPVM vm, tCSTRING s, tPCELL* cell)
{
	tINT size=strlen(s);
	tPBUFFER b;
	if (string_stream_create(vm, STREAM_INPUT, cell)) return VM_ERROR;
	b=STRING_STREAM_GET_BUFFER(*cell);
	while (b->size<size) {
		if (buffer_extend(vm, b)) return VM_ERROR;
	}
	memcpy(b->buffer, s, size);
	b->data_size=size;
	return VM_OK;
}

VM_RET string_stream_create_output(tPVM vm, tPCELL* cell)
{
	return string_stream_create(vm, STREAM_OUTPUT, cell);
}

VM_RET string_stream_create(tPVM vm, const tINT io_flag, tPCELL* cell)
{
	tUINT size;
	tPBUFFER b;

	size=allocate_cell(vm, sizeof(tCELL)*3, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_STRING_STREAM);
	STRING_STREAM_SET_SIZE(*cell, size);

	if (allocate_string_buffer(vm, &b)) return VM_ERROR;
	buffer_clear(b);
	STRING_STREAM_SET_BUFFER(*cell, b);

	if (io_flag&STREAM_INPUT)
		STRING_STREAM_SET_INPUT(*cell);
	else
		STRING_STREAM_RESET_INPUT(*cell);
	if (io_flag&STREAM_OUTPUT)
		STRING_STREAM_SET_OUTPUT(*cell);
	else
		STRING_STREAM_RESET_OUTPUT(*cell);
	STRING_STREAM_RESET_CLOSED(*cell);
	return VM_OK;
}

VM_RET string_input_stream_initialize(tPVM vm, tPCELL stream, tCSTRING string)
{
	return buffer_initialize(vm, STRING_STREAM_GET_BUFFER(stream), string);
}

VM_RET string_stream_destroy(tPVM vm, tPCELL string_stream)
{
	return free_string_buffer(vm, STRING_STREAM_GET_BUFFER(string_stream));
}

tUINT string_stream_get_size(tPCELL string_stream)
{
	return STRING_STREAM_GET_SIZE(string_stream);
}

tBOOL string_stream_is_input(tPCELL stream)
{
	return STRING_STREAM_GET_INPUT(stream) ? tTRUE : tFALSE;
}

tBOOL string_stream_is_output(tPCELL stream)
{
	return STRING_STREAM_GET_OUTPUT(stream) ? tTRUE : tFALSE;
}

tBOOL string_stream_is_closed(tPCELL stream)
{
	return STRING_STREAM_GET_CLOSED(stream) ? tTRUE : tFALSE;
}

void string_stream_clear(tPCELL stream)
{
	buffer_clear(STRING_STREAM_GET_BUFFER(stream));
}

VM_RET string_stream_read_char(tPVM vm, tPCELL stream, tPCHAR c)
{
	if (!string_stream_is_input(stream)) {
		tOBJECT obj;
		cell_to_object(stream, &obj);
		return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &obj);
	}
	if (!buffer_read_char(vm, STRING_STREAM_GET_BUFFER(stream), c)) {
		if (vm_get_reader_eos_error(vm)) {
			vm_set_last_condition_eos_error(vm);
			return VM_ERROR;
		} else {
			return signal_stream_error(vm, TISL_ERROR_END_OF_STREAM, stream);
		}
	} else {
		return VM_OK;
	}
}

VM_RET string_stream_preview_char(tPVM vm, tPCELL stream, tPCHAR c)
{
	if (!string_stream_is_input(stream)) {
		tOBJECT obj;
		cell_to_object(stream, &obj);
		return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &obj);
	}
	if (!buffer_preview_char(vm, STRING_STREAM_GET_BUFFER(stream), c)) {
		if (vm_get_reader_eos_error(vm)) {
			vm_set_last_condition_eos_error(vm);
			return VM_ERROR;
		} else {
			return signal_stream_error(vm, TISL_ERROR_END_OF_STREAM, stream);
		}
	} else {
		return VM_OK;
	}
}

VM_RET string_stream_write_char(tPVM vm, tPCELL stream, tCHAR c)
{
	if (!string_stream_is_output(stream)) {
		tOBJECT obj;
		cell_to_object(stream, &obj);
		return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &obj);
	}
	return buffer_write_char(vm, STRING_STREAM_GET_BUFFER(stream), c);
}

tINT string_stream_get_x(tPCELL stream)
{
	return buffer_get_x(STRING_STREAM_GET_BUFFER(stream));
}

tBOOL string_stream_check_eos(tPCELL stream)
{
	return buffer_check_eos(STRING_STREAM_GET_BUFFER(stream));
}

VM_RET string_stream_to_string(tPVM vm, tPCELL stream, tPCELL* string)
{
	if (string_stream_write_char(vm, stream, '\0')) return VM_ERROR;
	if (tisl_get_string(vm_get_tisl(vm), vm, STRING_STREAM_GET_BUFFER(stream)->buffer, string)) return VM_ERROR;
	string_stream_clear(stream);
	return VM_OK;
}

VM_RET string_stream_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_string(vm, stream, "#(<string-stream> ")||
		write_integer(vm, stream, (tINT)OBJECT_GET_CELL(obj), 16)||
		write_char(vm, stream, ')');
}

VM_RET string_stream_lock(tPCELL stream, tPVM vm)
{
	return tisl_lock(STRING_STREAM_GET_BUFFER(stream)->lock, vm);
}

void string_stream_unlock(tPCELL stream)
{
	tisl_unlock(STRING_STREAM_GET_BUFFER(stream)->lock);
}

VM_RET string_stream_close(tPVM vm, tPCELL stream)
{
	STRING_STREAM_SET_CLOSED(stream);
	return VM_OK;
}

VM_RET string_stream_mark(tPVM vm, tPCELL cell)
{
	return VM_OK;
}

tBOOL string_stream_is_locked(tPCELL stream)
{
	return (STRING_STREAM_GET_BUFFER(stream)->lock[0]==0) ? tFALSE : tTRUE;
}

tSTRING string_stream_get_buffer_area(tPCELL stream)
{
	return STRING_STREAM_GET_BUFFER(stream)->buffer;
}

tINT string_stream_get_buffer_size(tPCELL stream)
{
	return STRING_STREAM_GET_BUFFER(stream)->size;
}

/////////////////////////////////////////////////
// CELL_FILE_STREAM

#define FILE_STREAM_GET_INPUT(stream)		((stream)->ui&STREAM_INPUT)
#define FILE_STREAM_SET_INPUT(stream)		((stream)->ui|=STREAM_INPUT)
#define FILE_STREAM_RESET_INPUT(stream)		((stream)->ui&=STREAM_INPUT_)
#define FILE_STREAM_GET_OUTPUT(stream)		((stream)->ui&STREAM_OUTPUT)
#define FILE_STREAM_SET_OUTPUT(stream)		((stream)->ui|=STREAM_OUTPUT)
#define FILE_STREAM_RESET_OUTPUT(stream)	((stream)->ui&=STREAM_OUTPUT_)
#define FILE_STREAM_GET_CLOSED(stream)		((stream)->ui&STREAM_CLOSED)
#define FILE_STREAM_SET_CLOSED(stream)		((stream)->ui|=STREAM_CLOSED)
#define FILE_STREAM_RESET_CLOSED(stream)	((stream)->ui&=STREAM_CLOSED_)

#define FILE_STREAM_GET_SIZE(stream)		(((stream)+1)->ui)
#define FILE_STREAM_SET_SIZE(stream, size)	(((stream)+1)->ui=(size))
#define FILE_STREAM_GET_FILE(stream)		((FILE*)(((stream)+2)->p))
#define FILE_STREAM_SET_FILE(stream, file)	(((stream)+2)->p=(FILE*)(file))
#define FILE_STREAM_GET_NAME(stream)		(((stream)+3)->cell)
#define FILE_STREAM_SET_NAME(stream, name)	(((stream)+3)->cell=(name))
#define FILE_STREAM_GET_X(stream)			(((stream)+4)->i)
#define FILE_STREAM_SET_X(stream, x)		(((stream)+4)->i=(x))
#define FILE_STREAM_GET_Y(stream)			(((stream)+5)->i)
#define FILE_STREAM_SET_Y(stream, y)		(((stream)+5)->i=(y))
#define FILE_STREAM_GET_NEXT(stream)		(((stream)+6)->cell)
#define FILE_STREAM_SET_NEXT(stream, next)	(((stream)+6)->cell=(next))

#define FILE_STREAM_GET_LOCK(stream)		((tPVM*)((stream)+7))

static void file_stream_move_position(tPCELL stream, const tCHAR c);

VM_RET file_stream_create(tPVM vm, const tINT io_flag, tPCELL name, tPCELL* stream)
{
	tUINT size;
	FILE* file;

	if (!name||!CELL_IS_STRING(name)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);

	size=allocate_cell(vm, sizeof(tCELL)*7+sizeof(tPVM)*2, stream);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*stream, CELL_FILE_STREAM);
	FILE_STREAM_SET_SIZE(*stream, size);
	FILE_STREAM_SET_NAME(*stream, name);

	if ((io_flag&STREAM_INPUT)&&(io_flag&STREAM_OUTPUT)) {
		// b t c どうしよう？
		file=fopen(string_get_string(name), "w+");
		FILE_STREAM_SET_INPUT(*stream);
		FILE_STREAM_SET_OUTPUT(*stream);
	} else if (io_flag&STREAM_INPUT) {
		file=fopen(string_get_string(name), "r");
		FILE_STREAM_SET_INPUT(*stream);
	} else if (io_flag&STREAM_OUTPUT) {
		file=fopen(string_get_string(name), "w");
		FILE_STREAM_SET_OUTPUT(*stream);
	} else {
		file=0;
	}
	if (!file) return signal_condition(vm, TISL_ERROR_CANNOT_OPEN_FILE);
	FILE_STREAM_SET_FILE(*stream, file);
	FILE_STREAM_RESET_CLOSED(*stream);
	
	FILE_STREAM_SET_X(*stream, 0);
	FILE_STREAM_SET_Y(*stream, 0);

	FILE_STREAM_GET_LOCK(*stream)[0]=0;
	FILE_STREAM_GET_LOCK(*stream)[1]=0;

	FILE_STREAM_SET_NEXT(*stream, 0);

	return VM_OK;
}

VM_RET file_stream_create_(tPVM vm, const tINT io_flag, tPCELL name, FILE* file, tPCELL* stream)
{
	tUINT size;

	if (!name||!CELL_IS_STRING(name)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);

	size=allocate_cell(vm, sizeof(tCELL)*6+sizeof(tPVM)*2, stream);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*stream, CELL_FILE_STREAM);
	FILE_STREAM_SET_SIZE(*stream, size);
	FILE_STREAM_SET_NAME(*stream, name);

	if ((io_flag&STREAM_INPUT)&&(io_flag&STREAM_OUTPUT)) {
		FILE_STREAM_SET_INPUT(*stream);
		FILE_STREAM_SET_OUTPUT(*stream);
	} else if (io_flag&STREAM_INPUT) {
		FILE_STREAM_SET_INPUT(*stream);
	} else if (io_flag&STREAM_OUTPUT) {
		FILE_STREAM_SET_OUTPUT(*stream);
	} else {
		file=0;
	}
	if (!file) return signal_condition(vm, TISL_ERROR_CANNOT_OPEN_FILE);
	FILE_STREAM_SET_FILE(*stream, file);
	FILE_STREAM_RESET_CLOSED(*stream);
	
	FILE_STREAM_SET_X(*stream, 0);
	FILE_STREAM_SET_Y(*stream, 0);

	FILE_STREAM_GET_LOCK(*stream)[0]=0;
	FILE_STREAM_GET_LOCK(*stream)[1]=0;

	FILE_STREAM_SET_NEXT(*stream, 0);

	return VM_OK;
}

VM_RET file_stream_destroy(tPVM vm, tPCELL stream)
{
	if (!CELL_IS_FILE_STREAM(stream)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (FILE_STREAM_GET_FILE(stream)&&!FILE_STREAM_GET_CLOSED(stream)) {
		FILE* file=FILE_STREAM_GET_FILE(stream);
		if ((file==stdin)||(file==stdout)||(file==stderr)) return VM_OK;
		if (fclose(file)) return signal_condition(vm, TISL_ERROR_CANNOT_CLOSE_FILE);
		FILE_STREAM_SET_CLOSED(stream);
	}
	return VM_OK;
}

tUINT file_stream_get_size(tPCELL stream)
{
	return FILE_STREAM_GET_SIZE(stream);
}

tBOOL file_stream_is_input(tPCELL stream)
{
	return FILE_STREAM_GET_INPUT(stream) ? tTRUE : tFALSE;
}

tBOOL file_stream_is_output(tPCELL stream)
{
	return FILE_STREAM_GET_OUTPUT(stream) ? tTRUE : tFALSE;
}

tBOOL file_stream_is_closed(tPCELL stream)
{
	return FILE_STREAM_GET_CLOSED(stream) ? tTRUE : tFALSE;
}

tINT file_stream_get_x(tPCELL stream)
{
	return FILE_STREAM_GET_X(stream);
}

tINT file_stream_get_y(tPCELL stream)
{
	return FILE_STREAM_GET_Y(stream);
}

VM_RET file_stream_read_char(tPVM vm, tPCELL stream, tPCHAR c)
{//引数の検査は呼び出しもとの方でやること
	*c=(tCHAR)fgetc(FILE_STREAM_GET_FILE(stream));
	if (*c==(tCHAR)EOF) {
		if (vm_get_reader_eos_error(vm)) {
			vm_set_last_condition_eos_error(vm);
			return VM_ERROR;
		} else {
			return signal_stream_error(vm, TISL_ERROR_END_OF_STREAM, stream);
		}
	} else {
		file_stream_move_position(stream, *c);
		return VM_OK;
	}
}

VM_RET file_stream_preview_char(tPVM vm, tPCELL stream, tPCHAR c)
{
	*c=(tCHAR)fgetc(FILE_STREAM_GET_FILE(stream));
	if (*c==(tCHAR)EOF) {
		if (vm_get_reader_eos_error(vm)) {
			vm_set_last_condition_eos_error(vm);
			return VM_ERROR;
		} else {
			return signal_stream_error(vm, TISL_ERROR_END_OF_STREAM, stream);
		}
	} else {
		ungetc(*c, FILE_STREAM_GET_FILE(stream));
		return VM_OK;
	}
}

VM_RET file_stream_write_char(tPVM vm, tPCELL stream, tCHAR c)
{
	if (fputc(c, FILE_STREAM_GET_FILE(stream))!=EOF) {
		file_stream_move_position(stream, c);
		return VM_OK;
	} else {
		return signal_stream_error(vm, TISL_ERROR_STREAM_ERROR, stream);
	}
}

tBOOL file_stream_check_eos(tPCELL stream)
{
	return feof(FILE_STREAM_GET_FILE(stream)) ? tTRUE : tFALSE;
}

static void file_stream_move_position(tPCELL stream, const tCHAR c)
{
	if (c=='\n') {
		FILE_STREAM_SET_X(stream, 0);
		FILE_STREAM_SET_Y(stream, FILE_STREAM_GET_Y(stream)+1);
	} else {
		FILE_STREAM_SET_X(stream, FILE_STREAM_GET_X(stream)+1);
	}
}

VM_RET file_stream_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_string(vm, stream, "#i(<file-stream> ")||
		write_integer(vm, stream, (tINT)OBJECT_GET_CELL(obj), 16)||
		write_char(vm, stream, ')');
}

VM_RET file_stream_lock(tPCELL stream, tPVM vm)
{
	return tisl_lock(FILE_STREAM_GET_LOCK(stream), vm);
}

void file_stream_unlock(tPCELL stream)
{
	tisl_unlock(FILE_STREAM_GET_LOCK(stream));
}

tPCELL file_stream_get_next(tPCELL stream)
{
	return FILE_STREAM_GET_NEXT(stream);
}

void file_stream_set_next(tPCELL stream, tPCELL next)
{
	FILE_STREAM_SET_NEXT(stream, next);
}

tPCELL file_stream_get_name(tPCELL stream)
{
	return FILE_STREAM_GET_NAME(stream);
}

FILE* file_stream_get_file(tPCELL stream)
{
	return FILE_STREAM_GET_FILE(stream);
}

VM_RET file_stream_close(tPVM vm, tPCELL stream)
{
	if (fclose(FILE_STREAM_GET_FILE(stream))) {
		return signal_stream_error(vm, TISL_ERROR_CANNOT_CLOSE_STREAM, stream);
	} else {
		FILE_STREAM_SET_CLOSED(stream);
		return VM_OK;
	}
}

VM_RET file_stream_flush(tPVM vm, tPCELL stream)
{
	if (fflush(FILE_STREAM_GET_FILE(stream))) {
		return signal_stream_error(vm, TISL_ERROR_STREAM_ERROR, stream);
	} else {
		return VM_OK;
	}
}

VM_RET file_stream_mark(tPVM vm, tPCELL cell)
{
	if (cell_mark(vm, FILE_STREAM_GET_NAME(cell))) return VM_ERROR;
	return cell_mark(vm, FILE_STREAM_GET_NEXT(cell));
}

tBOOL file_stream_is_locked(tPCELL stream)
{
	return (FILE_STREAM_GET_LOCK(stream)[0]==0) ? tFALSE : tTRUE;
}

VM_RET file_stream_get_position(tPVM vm, tPCELL stream, tINT* pos)
{
	long fpos;
	fpos=ftell(FILE_STREAM_GET_FILE(stream));
	if (fpos<0) {
		return signal_stream_error(vm, TISL_ERROR_STREAM_ERROR, stream);
	}
	*pos=(tINT)fpos;// 問題ありそう?tINT以上のサイズのファイルで問題　2Gぐらい？/*!!!*/
	return VM_OK;
}

VM_RET file_stream_set_position(tPVM vm, tPCELL stream, tINT pos)
{
	if (fseek(FILE_STREAM_GET_FILE(stream), pos, SEEK_SET)) {
		return VM_OK;
	} else {
		return signal_stream_error(vm, TISL_ERROR_STREAM_ERROR, stream);
	}
}

