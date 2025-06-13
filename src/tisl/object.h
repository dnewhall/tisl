//
// TISL/src/tisl/a/object.h
// TISL Ver 4.x
//

#ifndef TISL_OBJECT_H
#define TISL_OBJECT_H

#include <stdio.h>
#include "../../include/tisl_type.h"

/////////////////////////////////////////////////

typedef struct tOBJECT_		tOBJECT,		*tPOBJECT;
typedef struct tVM_			tVM,			*tPVM;
typedef struct tTISL_		tTISL,			*tPTISL;
typedef union tCELL_		tCELL,			*tPCELL;
typedef struct tBUFFER_		tBUFFER,		*tPBUFFER;
typedef struct tGC_			tGC,			*tPGC;
typedef struct tTRANSLATOR_	tTRANSALTOR,	*tTRANSLATOR;

/////////////////////////////////////////////////

typedef int					VM_RET;
#define	VM_OK				0
#define VM_ERROR			-1

/////////////////////////////////////////////////

union tCELL_ {
	tINT		i;
	tFLOAT		f;
	tCHAR		c;
	tUINT		ui;
	tPCELL		cell;
	tPOBJECT	obj;
	void*		p;
};

/////////////////////////////////////////////////

struct tOBJECT_ {
	tCELL	type_id;
	tCELL	data;
};

/////////////////////////////////////////////////

struct TISL_OBJECT_	{
	tCELL		dummy;
};

/////////////////////////////////////////////////
// TISL_OBJECT

// CELL_TISL_OBJECT参照

/////////////////////////////////////////////////
// tOJBECT

#define OBJECT_UNBOUND				0x00
#define OBJECT_NIL					0x01
#define OBJECT_INTEGER				0x02
#define OBJECT_FLOAT				0x03
#define OBJECT_CHARACTER			0x04
#define OBJECT_BUILT_IN_CLASS		0x05
#define OBJECT_SPECIAL_OPERATOR		0x06
#define OBJECT_DEFINING_OPERATOR	0x07
#define OBJECT_PRIMITIVE_OPERATOR	0x08

#define OBJECT_FREE					0x10
#define OBJECT_CONS					0x11
#define OBJECT_STRING				0x12
#define OBJECT_SYMBOL				0x13
#define OBJECT_VECTOR				0x14
#define OBJECT_ARRAY				0x15
#define OBJECT_QUASIQUOTE			0x16
#define OBJECT_UNQUOTE				0x17
#define OBJECT_UNQUOTE_SPLICING		0x18
#define OBJECT_STRING_STREAM		0x19
#define OBJECT_FILE_STREAM			0x1a
#define OBJECT_CONDITION			0x1b
#define OBJECT_PACKAGE				0x1c
#define OBJECT_BIND					0x1d
#define OBJECT_BIND_LIST			0x1e
#define OBJECT_FOREIGN_OBJECT		0x1f
#define OBJECT_FUNCTION				0x20
#define OBJECT_ENVIRONMENT			0x21
#define OBJECT_LOCAL_FUNCTION		0x22
#define OBJECT_STANDARD_CLASS		0x23
#define OBJECT_GENERIC_FUNCTION		0x24
#define OBJECT_METHOD				0x25
#define OBJECT_EFFECTIVE_METHOD		0x26
#define OBJECT_APPLICABLE_METHOD	0x27
#define OBJECT_INSTANCE				0x28
#define OBJECT_SLOT					0x29
#define OBJECT_LINKED_FUNCTION		0x2a
#define OBJECT_LINKED_LIBRARY		0x2b
#define OBJECT_TISL_OBJECT			0x2c
#define OBJECT_FOREIGN_CLASS		0x2d

#define OBJECT_GET_TYPE(obj)					((obj)->type_id.ui)
#define OBJECT_SET_TYPE(obj, id)				((obj)->type_id.ui=(id))

#define OBJECT_IS_UNBOUND(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_UNBOUND)
#define OBJECT_IS_NIL(obj)						(OBJECT_GET_TYPE(obj)==OBJECT_NIL)
#define OBJECT_IS_INTEGER(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_INTEGER)
#define OBJECT_IS_FLOAT(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_FLOAT)
#define OBJECT_IS_CHARACTER(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_CHARACTER)
#define OBJECT_IS_BUILT_IN_CLASS(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_BUILT_IN_CLASS)
#define OBJECT_IS_SPECIAL_OPERATOR(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_SPECIAL_OPERATOR)
#define OBJECT_IS_DEFINING_OPERATOR(obj)		(OBJECT_GET_TYPE(obj)==OBJECT_DEFINING_OPERATOR)
#define OBJECT_IS_PRIMITIVE_OPERATOR(obj)		(OBJECT_GET_TYPE(obj)==OBJECT_PRIMITIVE_OPERATOR)

#define OBJECT_IS_CELL(obj)						(OBJECT_GET_TYPE(obj)>=OBJECT_FREE)

#define OBJECT_IS_FREE(obj)						(OBJECT_GET_TYPE(obj)==OBJECT_FREE)
#define OBJECT_IS_CONS(obj)						(OBJECT_GET_TYPE(obj)==OBJECT_CONS)
#define OBJECT_IS_STRING(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_STRING)
#define OBJECT_IS_SYMBOL(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_SYMBOL)
#define OBJECT_IS_VECTOR(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_VECTOR)
#define OBJECT_IS_ARRAY(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_ARRAY)
#define OBJECT_IS_QUASIQUOTE(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_QUASIQUOTE)
#define OBJECT_IS_UNQUOTE(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_UNQUOTE)
#define OBJECT_IS_UNQUOTE_SPLICING(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_UNQUOTE_SPLICING)
#define OBJECT_IS_STRING_STREAM(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_STRING_STREAM)
#define OBJECT_IS_FILE_STREAM(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_FILE_STREAM)
#define OBJECT_IS_CONDITION(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_CONDITION)
#define OBJECT_IS_PACKAGE(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_PACKAGE)
#define OBJECT_IS_BIND(obj)						(OBJECT_GET_TYPE(obj)==OBJECT_BIND)
#define OBJECT_IS_BIND_LIST(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_BIND_LIST)
#define OBJECT_IS_FOREIGN_OBJECT(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_FOREIGN_OBJECT)
#define OBJECT_IS_FUNCTION(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_FUNCTION)
#define OBJECT_IS_ENVIRONMENT(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_ENVIRONMENT)
#define OBJECT_IS_LOCAL_FUNCTION(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_LOCAL_FUNCTION)
#define OBJECT_IS_STANDARD_CLASS(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_STANDARD_CLASS)
#define OBJECT_IS_GENERIC_FUNCTION(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_GENERIC_FUNCTION)
#define OBJECT_IS_METHOD(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_METHOD)
#define OBJECT_IS_EFFECTIVE_METHOD(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_EFFECTIVE_METHOD)
#define OBJECT_IS_APPLICABLE_METHOD(obj)		(OBJECT_GET_TYPE(obj)==OBJECT_APPLICABLE_METHOD)
#define OBJECT_IS_INSTANCE(obj)					(OBJECT_GET_TYPE(obj)==OBJECT_INSTANCE)
#define OBJECT_IS_SLOT(obj)						(OBJECT_GET_TYPE(obj)==OBJECT_SLOT)
#define OBJECT_IS_LINKED_FUNCTION(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_LINKED_FUNCTION)
#define OBJECT_IS_LINKED_LIBRARY(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_LINKED_LIBRARY)
#define OBJECT_IS_TISL_OBJECT(obj)				(OBJECT_GET_TYPE(obj)==OBJECT_TISL_OBJECT)
#define OBJECT_IS_FOREIGN_CLASS(obj)			(OBJECT_GET_TYPE(obj)==OBJECT_FOREIGN_CLASS)
//
#define OBJECT_IS_MACRO(obj)					(OBJECT_IS_FUNCTION(obj)&&function_is_macro(OBJECT_GET_CELL(obj)))

#define OBJECT_SET_UNBOUND(obj)					(OBJECT_SET_TYPE(obj, OBJECT_UNBOUND), (obj)->data.cell=0)
#define OBJECT_SET_NIL(obj)						(OBJECT_SET_TYPE(obj, OBJECT_NIL), (obj)->data.cell=0)
#define OBJECT_SET_INTEGER(obj, ii)				(OBJECT_SET_TYPE(obj, OBJECT_INTEGER), (obj)->data.i=(ii))
#define OBJECT_SET_FLOAT(obj, ff)				(OBJECT_SET_TYPE(obj, OBJECT_FLOAT), (obj)->data.f=(ff))
#define OBJECT_SET_CHARACTER(obj, cc)			(OBJECT_SET_TYPE(obj, OBJECT_CHARACTER), (obj)->data.c=(cc))
#define OBJECT_SET_BUILT_IN_CLASS(obj, id)		(OBJECT_SET_TYPE(obj, OBJECT_BUILT_IN_CLASS), (obj)->data.i=(id))
#define OBJECT_SET_SPECIAL_OPERATOR(obj, id)	(OBJECT_SET_TYPE(obj, OBJECT_SPECIAL_OPERATOR), (obj)->data.i=(id))
#define OBJECT_SET_DEFINING_OPERATOR(obj, id)	(OBJECT_SET_TYPE(obj, OBJECT_DEFINING_OPERATOR), (obj)->data.i=(id))
#define OBJECT_SET_PRIMITIVE_OPERATOR(obj, id)	(OBJECT_SET_TYPE(obj, OBJECT_PRIMITIVE_OPERATOR), (obj)->data.i=(id))

#define OBJECT_SET_FREE(obj, p)					(OBJECT_SET_TYPE(obj, OBJECT_FREE), (obj)->data.cell=(p))
#define OBJECT_SET_CONS(obj, p)					(OBJECT_SET_TYPE(obj, OBJECT_CONS), (obj)->data.cell=(p))
#define OBJECT_SET_STRING(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_STRING), (obj)->data.cell=(p))
#define OBJECT_SET_SYMBOL(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_SYMBOL), (obj)->data.cell=(p))
#define OBJECT_SET_VECTOR(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_VECTOR), (obj)->data.cell=(p))
#define OBJECT_SET_ARRAY(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_ARRAY), (obj)->data.cell=(p))
#define OBJECT_SET_QUASIQUOTE(obj, p)			(OBJECT_SET_TYPE(obj, OBJECT_QUASIQUOTE), (obj)->data.cell=(p))
#define OBJECT_SET_UNQUOTE(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_UNQUOTE), (obj)->data.cell=(p))
#define OBJECT_SET_UNQUOTE_SPLICING(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_UNQUOTE_SPLICING), (obj)->data.cell=(p))
#define OBJECT_SET_STRING_STREAM(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_STRING_STREAM), (obj)->data.cell=(p))
#define OBJECT_SET_FILE_STREAM(obj, p)			(OBJECT_SET_TYPE(obj, OBJECT_FILE_STREAM), (obj)->data.cell=(p))
#define OBJECT_SET_CONDITION(obj, p)			(OBJECT_SET_TYPE(obj, OBJECT_CONDITION), (obj)->data.cell=(p))
#define OBJECT_SET_PACKAGE(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_PACKAGE), (obj)->data.cell=(p))
#define OBJECT_SET_BIND(obj, p)					(OBJECT_SET_TYPE(obj, OBJECT_BIND), (obj)->data.cell=(p))
#define OBJECT_SET_BIND_LIST(obj, p)			(OBJECT_SET_TYPE(obj, OBJECT_BIND_LIST), (obj)->data.cell=(p))
#define OBJECT_SET_FOREIGN_OBJECT(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_FOREIGN_OBJECT), (obj)->data.cell=(p))
#define OBJECT_SET_FUNCTION(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_FUNCTION), (obj)->data.cell=(p))
#define OBJECT_SET_ENVIRONMENT(obj, p)			(OBJECT_SET_TYPE(obj, OBJECT_ENVIRONMENT), (obj)->data.cell=(p))
#define OBJECT_SET_LOCAL_FUNCTION(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_LOCAL_FUNCTION), (obj)->data.cell=(p))
#define OBJECT_SET_STANDARD_CLASS(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_STANDARD_CLASS), (obj)->data.cell=(p))
#define OBJECT_SET_GENERIC_FUNCTION(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_GENERIC_FUNCTION), (obj)->data.cell=(p))
#define OBJECT_SET_METHOD(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_METHOD), (obj)->data.cell=(p))
#define OBJECT_SET_EFFECTIVE_METHOD(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_EFFECTIVE_METHOD), (obj)->data.cell=(p))
#define OBJECT_SET_APPLICABLE_METHOD(obj, p)	(OBJECT_SET_TYPE(obj, OBJECT_APPLICABLE_METHOD), (obj)->data.cell=(p))
#define OBJECT_SET_INSTANCE(obj, p)				(OBJECT_SET_TYPE(obj, OBJECT_INSTANCE), (obj)->data.cell=(p))
#define OBJECT_SET_SLOT(obj, p)					(OBJECT_SET_TYPE(obj, OBJECT_SLOT), (obj)->data.cell=(p))
#define OBJECT_SET_LINKED_FUNCTION(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_LINKED_FUNCTION), (obj)->data.cell=(p))
#define OBJECT_SET_LINKED_LIBRARY(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_LINKED_LIBRARY), (obj)->data.cell=(p))
#define OBJECT_SET_TISL_OBJECT(obj, p)			(OBJECT_SET_TYPE(obj, OBJECT_TISL_OBJECT), (obj)->data.cell=(p))
#define OBJECT_SET_FOREIGN_CLASS(obj, p)		(OBJECT_SET_TYPE(obj, OBJECT_FOREIGN_CLASS), (obj)->data.cell=(p))

#define OBJECT_GET_INTEGER(obj)					((obj)->data.i)
#define OBJECT_GET_FLOAT(obj)					((obj)->data.f)
#define OBJECT_GET_CHARACTER(obj)				((obj)->data.c)
#define OBJECT_GET_CELL(obj)					((obj)->data.cell)
#define OBJECT_GET_POINTER(obj)					((obj)->data.p)

///////////////////

VM_RET unbound_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET nil_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET integer_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET float_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET character_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET built_in_class_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET special_operator_write(tPVM vm, tPCELL stream, tPOBJECT obj);// built_in_object.cで定義
VM_RET defining_operator_write(tPVM vm, tPCELL stream, tPOBJECT obj);// built_in_object.cで定義
VM_RET primitive_operator_write(tPVM vm, tPCELL stream, tPOBJECT obj);// built_in_object.cで定義

tCSTRING built_in_class_get_name(const tINT id);

///////////////////

tBOOL object_eql(tPOBJECT obj1, tPOBJECT obj2);
tBOOL object_equal(tPOBJECT obj1, tPOBJECT obj2);
tBOOL object_is_instance(tPVM vm, tPOBJECT obj, tPOBJECT clss);
VM_RET object_mark(tPVM vm, tPOBJECT obj);
void object_get_class(tPOBJECT obj, tPOBJECT clss);
// classがsuperの下位クラスの時tTRUEを返す
tBOOL class_is_subclass(tPVM vm, tPOBJECT clss, tPOBJECT super);
// clss1のクラス優先度におけるclass2の優先度の取得
tINT class_get_precedence(tPVM vm, tPOBJECT clss1, tPOBJECT clss2);

/////////////////////////////////////////////////

#define CELL_SIZE					(sizeof(tCELL))

#define CELL_UNIT	3

#define CELL_TYPE					0x3f000000
#define CELL_TYPE_					0xc0ffffff

#define CELL_GET_TYPE(cell)			((cell)->ui&CELL_TYPE)
#define CELL_SET_TYPE(cell, id)		((cell)->ui&=CELL_TYPE_, (cell)->ui|=(id)&CELL_TYPE)
#define CELL_GET_TYPE_INDEX(cell)	(CELL_GET_TYPE(cell)>>24)

// CELL gc mark

#define CELL_GC_MARK				0x00800000
#define CELL_GC_MARK_				0xff7fffff

#define CELL_GET_GC_MARK(cell)		((cell)->ui&CELL_GC_MARK)
#define CELL_SET_GC_MARK(cell)		((cell)->ui|=CELL_GC_MARK)
#define CELL_RESET_GC_MARK(cell)	((cell)->ui&=CELL_GC_MARK_)

// CELL immutable object flag

#define CELL_IMMUTABLE				0x00400000
#define CELL_IMMUTABLE_				0xffbfffff

#define CELL_GET_IMMUTABLE(cell)	((cell)->ui&CELL_IMMUTABLE)
#define CELL_SET_IMMUTABLE(cell)	((cell)->ui|=CELL_IMMUTABLE)
#define CELL_RESET_IMMUTABLE(cell)	((cell)->ui&=CELL_IMMUTABLE_)

// CELL loop flag

#define CELL_LOOP					0x80000000
#define CELL_LOOP_					0x7fffffff

#define CELL_GET_LOOP(cell)			((cell)->ui&CELL_LOOP)
#define CELL_SET_LOOP(cell)			((cell)->ui|=CELL_LOOP)
#define CELL_RESET_LOOP(cell)		((cell)->ui&=CELL_LOOP_)

// CELL tenured

#define CELL_TENURED				0x40000000
#define CELL_TENURED_				0xbfffffff

#define CELL_GET_TENURED(cell)		((cell)->ui&CELL_TENURED)
#define CELL_SET_TENURED(cell)		((cell)->ui|=CELL_TENURED)
#define CELL_RESET_TENURED(cell)	((cell)->ui&=CELL_TENURED_)

// CELL type id

#define CELL_FREE					0x00000000
#define CELL_CONS					0x01000000
#define CELL_STRING					0x02000000
#define CELL_SYMBOL					0x03000000
#define CELL_VECTOR					0x04000000
#define CELL_ARRAY					0x05000000
#define CELL_QUASIQUOTE				0x06000000
#define CELL_UNQUOTE				0x07000000
#define CELL_UNQUOTE_SPLICING		0x08000000
#define CELL_STRING_STREAM			0x09000000
#define CELL_FILE_STREAM			0x0a000000
#define CELL_CONDITION				0x0b000000
#define CELL_PACKAGE				0x0c000000
#define CELL_BIND					0x0d000000
#define CELL_BIND_LIST				0x0e000000
#define CELL_FOREIGN_OBJECT			0x0f000000
#define CELL_FUNCTION				0x10000000
#define CELL_ENVIRONMENT			0x11000000
#define CELL_LOCAL_FUNCTION			0x12000000
#define CELL_STANDARD_CLASS			0x13000000
#define CELL_GENERIC_FUNCTION		0x14000000
#define CELL_METHOD					0x15000000
#define CELL_EFFECTIVE_METHOD		0x16000000
#define CELL_APPLICABLE_METHOD		0x17000000
#define CELL_INSTANCE				0x18000000
#define CELL_SLOT					0x19000000
#define CELL_LINKED_FUNCTION		0x1a000000
#define CELL_LINKED_LIBRARY			0x1b000000
#define CELL_TISL_OBJECT			0x1c000000
#define CELL_FOREIGN_CLASS			0x1d000000

///////////////////

#define CELL_HEAD_CLEAR(cell)			((cell)->ui=0)

#define CELL_IS_FREE(cell)				(CELL_GET_TYPE(cell)==CELL_FREE)
#define CELL_IS_CONS(cell)				(CELL_GET_TYPE(cell)==CELL_CONS)
#define CELL_IS_STRING(cell)			(CELL_GET_TYPE(cell)==CELL_STRING)
#define CELL_IS_SYMBOL(cell)			(CELL_GET_TYPE(cell)==CELL_SYMBOL)
#define CELL_IS_VECTOR(cell)			(CELL_GET_TYPE(cell)==CELL_VECTOR)
#define CELL_IS_ARRAY(cell)				(CELL_GET_TYPE(cell)==CELL_ARRAY)
#define CELL_IS_QUASIQUOTE(cell)		(CELL_GET_TYPE(cell)==CELL_QUASIQUOTE)
#define CELL_IS_UNQUOTE(cell)			(CELL_GET_TYPE(cell)==CELL_UNQUOTE)
#define CELL_IS_UNQUOTE_SPLICING(cell)	(CELL_GET_TYPE(cell)==CELL_UNQUOTE_SPLICING)
#define CELL_IS_STRING_STREAM(cell)		(CELL_GET_TYPE(cell)==CELL_STRING_STREAM)
#define CELL_IS_FILE_STREAM(cell)		(CELL_GET_TYPE(cell)==CELL_FILE_STREAM)
#define CELL_IS_CONDITION(cell)			(CELL_GET_TYPE(cell)==CELL_CONDITION)
#define CELL_IS_PACKAGE(cell)			(CELL_GET_TYPE(cell)==CELL_PACKAGE)
#define CELL_IS_BIND(cell)				(CELL_GET_TYPE(cell)==CELL_BIND)
#define CELL_IS_BIND_LIST(cell)			(CELL_GET_TYPE(cell)==CELL_BIND_LIST)
#define CELL_IS_FOREIGN_OBJECT(cell)	(CELL_GET_TYPE(cell)==CELL_FOREIGN_OBJECT)
#define CELL_IS_FUNCTION(cell)			(CELL_GET_TYPE(cell)==CELL_FUNCTION)
#define CELL_IS_ENVIRONMENT(cell)		(CELL_GET_TYPE(cell)==CELL_ENVIRONMENT)
#define CELL_IS_LOCAL_FUNCTION(cell)	(CELL_GET_TYPE(cell)==CELL_LOCAL_FUNCTION)
#define CELL_IS_STANDARD_CLASS(cell)	(CELL_GET_TYPE(cell)==CELL_STANDARD_CLASS)
#define CELL_IS_GENERIC_FUNCTION(cell)	(CELL_GET_TYPE(cell)==CELL_GENERIC_FUNCTION)
#define CELL_IS_METHOD(cell)			(CELL_GET_TYPE(cell)==CELL_METHOD)
#define CELL_IS_EFFECTIVE_METHOD(cell)	(CELL_GET_TYPE(cell)==CELL_EFFECTIVE_METHOD)
#define CELL_IS_APPLICABLE_METHOD(cell)	(CELL_GET_TYPE(cell)==CELL_APPLICABLE_METHOD)
#define CELL_IS_INSTANCE(cell)			(CELL_GET_TYPE(cell)==CELL_INSTANCE)
#define CELL_IS_SLOT(cell)				(CELL_GET_TYPE(cell)==CELL_SLOT)
#define CELL_IS_LINKED_FUNCTON(cell)	(CELL_GET_TYPE(cell)==CELL_LINKED_FUNCTION)
#define CELL_IS_LINKED_LIBRARY(cell)	(CELL_GET_TYPE(cell)==CELL_LINKED_LIBRARY)
#define CELL_IS_TISL_OBJECT(cell)		(CELL_GET_TYPE(cell)==CELL_TISL_OBJECT)
#define CELL_IS_FOREIGN_CLASS(cell)		(CELL_GET_TYPE(cell)==CELL_FOREIGN_CLASS)

/////////////////////////////////////////////////
// CELL_FREE

void free_initialize(tPCELL cell, tUINT size, tPCELL next);
tUINT free_get_size(tPCELL cell);
tPCELL free_get_next(tPCELL cell);
void free_set_next(tPCELL cell, tPCELL next);
VM_RET free_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET free_mark(tPVM vm, tPCELL cell);

/////////////////////////////////////////////////
// CELL_CONS

// car cdr の保護あり
VM_RET cons_create(tPVM vm, tPCELL* cons, tPOBJECT car, tPOBJECT cdr);
// car cdr の保護無し
VM_RET cons_create_(tPVM vm, tPCELL* cons, tPOBJECT car, tPOBJECT cdr);
tUINT cons_get_size(tPCELL cons);
void cons_get_car(tPCELL cons, tPOBJECT obj);
void cons_set_car(tPCELL cons, tPOBJECT obj);
void cons_get_cdr(tPCELL cons, tPOBJECT obj);
void cons_set_cdr(tPCELL cons, tPOBJECT obj);
tPCELL cons_get_car_cons(tPCELL cons);
tPCELL cons_get_cdr_cons(tPCELL cons);
tINT cons_get_length(tPCELL cons);
VM_RET cons_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tBOOL cons_equal(tPCELL cons1, tPCELL cons2);
VM_RET cons_mark(tPVM vm, tPCELL cell);

VM_RET list_get_object(tPVM vm, tPCELL list, const tINT i, tPOBJECT obj);
VM_RET list_set_object(tPVM vm, tPCELL list, const tINT i, tPOBJECT obj);

/////////////////////////////////////////////////
// CELL_STRING

VM_RET string_create(tPVM vm, tCSTRING string, tPCELL* cell);
VM_RET string_create_2(tPVM vm, const tINT len, tCHAR c, tPCELL* cell);
VM_RET string_destroy(tPVM vm, tPCELL cell);
tUINT string_get_size(tPCELL string);
tINT string_get_length(tPCELL string);
tCSTRING string_get_string(tPCELL string);
tSTRING string_get_data(tPCELL string);
tPCELL string_get_next(tPCELL string);
void string_set_next(tPCELL string, tPCELL next);
VM_RET string_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET string_get_character(tPVM vm, tPCELL string, const tINT x, tPOBJECT ret);
VM_RET string_set_character(tPVM vm, tPCELL string, const tINT x, tCHAR c);
tBOOL string_equal(tPCELL string1, tPCELL string2);
VM_RET string_mark(tPVM vm, tPCELL cell);

/////////////////////////////////////////////////
// CELL_SYMBOL

VM_RET symbol_create(tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell);
VM_RET symbol_create_(tPVM vm, tPCELL list, tBOOL complete, tPCELL* cell);
VM_RET symbol_create_gensym(tPVM vm, tPCELL string, tPCELL* cell);
VM_RET symbol_create_simple(tPVM vm, tPCELL string, tBOOL complete, tPCELL* cell);
VM_RET symbol_destroy(tPVM vm, tPCELL cell);
tUINT symbol_get_size(tPCELL symbol);
tINT symbol_get_length(tPCELL symbol);
tPCELL symbol_get_next(tPCELL symbol);
void symbol_set_next(tPCELL symbol, tPCELL next);
void symbol_set_key(tPCELL symbol, tINT key);
tINT symbol_get_key(tPCELL symbol);
tBOOL symbol_is_gensym(tPCELL symbol);
tBOOL symbol_is_complete(tPCELL symbol);
tBOOL symbol_is_simple(tPCELL symbol);
tBOOL symbol_get_string(tPCELL symbol, const tINT i, tPCELL* string);
VM_RET symbol_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tBOOL symbol_is_special_operator(tPCELL symbol);// built_in_object.cで定義
tINT symbol_get_special_operator_id(tPCELL symbol);// built_in_object.cで定義
VM_RET symbol_mark(tPVM vm, tPCELL cell);
VM_RET symbol_is_built_in_function(tPVM vm, tPCELL symbol);// built_in_object.cで定義

/////////////////////////////////////////////////
// CELL_VECTOR

VM_RET vector_create(tPVM vm, tPCELL list, tPCELL* cell);
VM_RET vector_create_(tPVM vm, tPCELL list, tPCELL* cell);
VM_RET vector_create_2_(tPVM vm, const tINT len, tPOBJECT init, tPCELL* cell);
tUINT vector_get_size(tPCELL vector);
tINT vector_get_length(tPCELL vector);
VM_RET vector_get_object(tPVM vm, tPCELL vector, const tINT i, tPOBJECT obj);
VM_RET vector_set_object(tPVM vm, tPCELL vector, const tINT i, tPOBJECT obj);
VM_RET vector_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tBOOL vector_equal(tPCELL vec1, tPCELL vec2);
VM_RET vector_mark(tPVM vm, tPCELL cell);

/////////////////////////////////////////////////
// CELL_ARRAY

VM_RET array_create(tPVM vm, const tINT d, tPOBJECT list, tPCELL* cell);
VM_RET array_create_(tPVM vm, const tINT d, tPOBJECT list, tPCELL* cell);
VM_RET array_create_2_(tPVM vm, tPCELL dimsnions, tPOBJECT init, tPCELL* cell);
tUINT array_get_size(tPCELL array);
tINT array_get_dimension(tPCELL array);
VM_RET array_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET array_get_object(tPVM vm, tPCELL array, const tINT d, tPOBJECT ret);
VM_RET array_set_object(tPVM vm, tPCELL array, const tINT d, tPOBJECT obj);
VM_RET array_get_dimension_list(tPVM vm, tPCELL array, tPOBJECT list);
tBOOL array_equal(tPCELL array1, tPCELL array2);
VM_RET array_mark(tPVM vm, tPCELL cell);

/////////////////////////////////////////////////
// CELL_QUASIQUOTE

VM_RET quasiquote_create(tPVM vm, tPOBJECT form, tPCELL* cell);
VM_RET quasiquote_create_(tPVM vm, tPOBJECT form, tPCELL* cell);
tUINT quasiquote_get_size(tPCELL quasiquote);
void quasiquote_get_form(tPCELL quote, tPOBJECT form);
VM_RET quasiquote_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET quasiquote_mark(tPVM vm, tPCELL cell);
tBOOL quasiquote_equal(tPCELL quote1, tPCELL quote2);

/////////////////////////////////////////////////
// CELL_UNQUOTE

VM_RET unquote_create(tPVM vm, tPOBJECT form, tPCELL* cell);
VM_RET unquote_create_(tPVM vm, tPOBJECT form, tPCELL* cell);
tUINT unquote_get_size(tPCELL unquote);
void unquote_get_form(tPCELL quote, tPOBJECT form);
VM_RET unquote_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET unquote_mark(tPVM vm, tPCELL cell);
tBOOL unquote_equal(tPCELL quote1, tPCELL quote2);

/////////////////////////////////////////////////
// CELL_UNQUOTE_SPLICING

VM_RET unquote_splicing_create(tPVM vm, tPOBJECT form, tPCELL* cell);
VM_RET unquote_splicing_create_(tPVM vm, tPOBJECT form, tPCELL* cell);
tUINT unquote_splicing_get_size(tPCELL unquote_splicing);
void unquote_splicing_get_form(tPCELL quote, tPOBJECT form);
VM_RET unquote_splicing_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET unquote_splicing_mark(tPVM vm, tPCELL cell);
tBOOL unquote_splicing_equal(tPCELL quote1, tPCELL quote2);

/////////////////////////////////////////////////
// stream

tBOOL check_eos(tPCELL stream);
tINT stream_get_x(tPCELL stream);

#define STREAM_INPUT		0x00000001
#define STREAM_INPUT_		0xfffffffe
#define STREAM_OUTPUT		0x00000002
#define STREAM_OUTPUT_		0xfffffffd
#define STREAM_CLOSED		0x00000004
#define STREAM_CLOSED_		0xfffffffb

/////////////////////////////////////////////////
// CELL_STRING_STREAM

VM_RET string_stream_create_input(tPVM vm, tCSTRING s, tPCELL* cell);
VM_RET string_stream_create_output(tPVM vm, tPCELL* cell);
VM_RET string_stream_destroy(tPVM vm, tPCELL string_stream);
VM_RET string_input_stream_initialize(tPVM vm, tPCELL stream, tCSTRING string);
tUINT string_stream_get_size(tPCELL string_stream);
tBOOL string_stream_is_input(tPCELL stream);
tBOOL string_stream_is_output(tPCELL stream);
tBOOL string_stream_is_closed(tPCELL stream);
void string_stream_clear(tPCELL stream);
VM_RET string_stream_read_char(tPVM vm, tPCELL stream, tPCHAR c);
VM_RET string_stream_preview_char(tPVM vm, tPCELL stream, tPCHAR c);
VM_RET string_stream_write_char(tPVM vm, tPCELL stream, tCHAR c);
tINT string_stream_get_x(tPCELL stream);
tBOOL string_stream_check_eos(tPCELL stream);
VM_RET string_stream_to_string(tPVM vm, tPCELL stream, tPCELL* string);
VM_RET string_stream_lock(tPCELL stream, tPVM vm);
void string_stream_unlock(tPCELL stream);
VM_RET string_stream_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET string_stream_close(tPVM vm, tPCELL stream);
VM_RET string_stream_mark(tPVM vm, tPCELL cell);
tBOOL string_stream_is_locked(tPCELL stream);

/////////////////////////////////////////////////
// CELL_FILE_STREAM

VM_RET file_stream_create(tPVM vm, const tINT io_flag, tPCELL name, tPCELL* stream);
VM_RET file_stream_create_(tPVM vm, const tINT io_flag, tPCELL name, FILE* file, tPCELL* stream);
VM_RET file_stream_destroy(tPVM vm, tPCELL file_stream);
tUINT file_stream_get_size(tPCELL file_stream);
tBOOL file_stream_is_input(tPCELL stream);
tBOOL file_stream_is_output(tPCELL stream);
tBOOL file_stream_is_closed(tPCELL stream);
tINT file_stream_get_x(tPCELL stream);
tINT file_stream_get_y(tPCELL stream);
VM_RET file_stream_read_char(tPVM vm, tPCELL stream, tPCHAR c);
VM_RET file_stream_preview_char(tPVM vm, tPCELL stream, tPCHAR c);
VM_RET file_stream_write_char(tPVM vm, tPCELL stream, tCHAR c);
tBOOL file_stream_check_eos(tPCELL stream);
VM_RET file_stream_lock(tPCELL stream, tPVM vm);
void file_stream_unlock(tPCELL stream);
VM_RET file_stream_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tPCELL file_stream_get_next(tPCELL stream);
void file_stream_set_next(tPCELL stream, tPCELL next);
tPCELL file_stream_get_name(tPCELL stream);
FILE* file_stream_get_file(tPCELL stream);
VM_RET file_stream_close(tPVM vm, tPCELL stream);
VM_RET file_stream_flush(tPVM vm, tPCELL stream);
VM_RET file_stream_mark(tPVM vm, tPCELL stream);
tBOOL file_stream_is_locked(tPCELL stream);
VM_RET file_stream_get_position(tPVM vm, tPCELL stream, tINT* pos);
VM_RET file_stream_set_position(tPVM vm, tPCELL stream, tINT pos);

/////////////////////////////////////////////////
// CELL_CONDITION

VM_RET condition_create(tPVM vm, const tINT class_id, tPCELL name, tPOBJECT slot1, tPOBJECT slot2, tPOBJECT continuable, tPOBJECT place, tPCELL* cell);
tUINT condition_get_size(tPCELL condition);
VM_RET condition_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tINT condition_get_class_id(tPCELL condition);
tBOOL condition_is_continuable(tPCELL condition);
tPCELL condition_get_name(tPCELL condition);
void condition_get_slot1(tPCELL condition, tPOBJECT slot1);
void condition_get_slot2(tPCELL condition, tPOBJECT slot2);
VM_RET condition_mark(tPVM vm, tPCELL cell);

// 名前付き例外のスロットアクセス
// arithmetic-error
void arithmetic_error_get_operation(tPCELL condition, tPOBJECT obj);
void arithmetic_error_set_operation(tPCELL condition, tPOBJECT obj);
void arithmetic_error_get_operands(tPCELL condition, tPOBJECT obj);
void arithmetic_error_set_operands(tPCELL condition, tPOBJECT obj);
// domain-error
void domain_error_get_expected_class(tPCELL condition, tPOBJECT obj);
void domain_error_set_expected_class(tPCELL condition, tPOBJECT obj);
void domain_error_get_object(tPCELL condition, tPOBJECT obj);
void domain_error_set_object(tPCELL condition, tPOBJECT obj);
// parse-error
void parse_error_get_string(tPCELL condition, tPOBJECT obj);
void parse_error_set_string(tPCELL condition, tPOBJECT obj);
void parse_error_get_expected_class(tPCELL condition, tPOBJECT obj);
void parse_error_set_expected_class(tPCELL condition, tPOBJECT obj);
// stream-error
void stream_error_get_stream(tPCELL condition, tPOBJECT obj);
void stream_error_set_stream(tPCELL condition, tPOBJECT obj);
// undefined-entity
void undefined_entity_set_name(tPCELL condition, tPOBJECT name);
void undefined_entity_get_name(tPCELL condition, tPOBJECT name);
void undefined_entity_set_namespace(tPCELL condition, const int namespace_id);
void undefined_entity_get_namespace(tPCELL condition, tPOBJECT name);
// simple-error
void simple_error_get_format_string(tPCELL condition, tPOBJECT obj);
void simple_error_set_format_string(tPCELL condition, tPOBJECT obj);
void simple_error_get_format_argument(tPCELL condition, tPOBJECT obj);
void simple_error_set_format_argument(tPCELL condition, tPOBJECT obj);

/////////////////////////////////////////////////
// CELL_PACKAGE

VM_RET package_create(tPVM vm, tPCELL bind, tPCELL name_precedence_list, tPCELL name, tPCELL parent, tPCELL* cell);
VM_RET package_create_(tPVM vm, tPCELL bind, tPCELL name_precedence_list, tPCELL name, tPCELL parent, tPCELL* cell);
VM_RET package_reset(tPVM vm, tPCELL npl, tPCELL package);
tUINT package_get_size(tPCELL package);
tINT package_get_length_of_name_precedence_list(tPCELL package);
tPCELL package_get_use_package(tPVM vm, tPCELL package, const tINT precedence);
VM_RET package_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET package_mark(tPVM vm, tPCELL cell);
VM_RET package_write_(tPVM vm, tPCELL stream, tPCELL package);

/////////////////////////////////////////////////
// CELL_BIND

VM_RET bind_create(tPVM vm, tPCELL name, tPCELL package, tPCELL* cell);
VM_RET bind_create_(tPVM vm, tPCELL name, tPCELL package, tPCELL* cell);
tUINT bind_get_size(tPCELL bind);
void bind_get_object(tPCELL bind, const tINT namespace_id, tPOBJECT obj);
void bind_set_object(tPCELL bind, const tINT namespace_id, tPOBJECT obj);
void bind_get_variable(tPCELL bind, tPOBJECT obj);
void bind_set_variable(tPCELL bind, tPOBJECT obj);
void bind_get_function(tPCELL bind, tPOBJECT obj);
void bind_set_function(tPCELL bind, tPOBJECT obj);
void bind_get_dynamic(tPCELL bind, tPOBJECT obj);
void bind_set_dynamic(tPCELL bind, tPOBJECT obj);
void bind_get_class(tPCELL bind, tPOBJECT obj);
void bind_set_class(tPCELL bind, tPOBJECT obj);
void bind_get_package(tPCELL bind, tPOBJECT obj);
void bind_set_package(tPCELL bind, tPOBJECT obj);
VM_RET bind_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET bind_mark(tPVM vm, tPCELL cell);
tPCELL bind_get_name(tPCELL bind);

void bind_set_variable_public(tPCELL bind);
void bind_set_variable_private(tPCELL bind);
tBOOL bind_variable_is_public(tPCELL bind);
void bind_set_function_public(tPCELL bind);
void bind_set_function_private(tPCELL bind);
tBOOL bind_function_is_public(tPCELL bind);
void bind_set_dynamic_public(tPCELL bind);
void bind_set_dynamic_private(tPCELL bind);
tBOOL bind_dyanmic_is_public(tPCELL bind);
void bind_set_class_public(tPCELL bind);
void bind_set_class_private(tPCELL bind);
tBOOL bind_class_is_public(tPCELL bind);
void bind_set_package_public(tPCELL bind);
void bind_set_package_private(tPCELL bind);
tBOOL bind_package_is_public(tPCELL bind);

// 属性リスト
// propety_name は CELL_SYMBOL
tBOOL bind_get_property(tPCELL bind, tPCELL property_name, tPOBJECT obj);
VM_RET bind_set_property(tPVM vm, tPCELL bind, tPCELL property_name, tPOBJECT obj);
void bind_remove_property(tPCELL bind, tPCELL property_name, tPOBJECT obj);

/////////////////////////////////////////////////
// CELL_BIND_LIST

VM_RET bind_list_create(tPVM vm, tPCELL package, tPCELL identifier, tPCELL list, tPCELL* cell);
VM_RET bind_list_create_(tPVM vm, tPCELL package, tPCELL identifier, tPCELL list, tPCELL* cell);
VM_RET bind_list_reset(tPVM vm, tPCELL package, tPCELL blist);
tUINT bind_list_get_size(tPCELL bind_list);
tPCELL bind_list_get_bind(tPCELL bind_list, const tINT namespace_id, tPCELL current);
tBOOL bind_list_bind_is_head(tPCELL blist, tPCELL bind);
VM_RET bind_list_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tPCELL bind_list_get_name(tPCELL bind_list);
VM_RET bind_list_mark(tPVM vm, tPCELL blist);

/////////////////////////////////////////////////
// package bind bind-list関係

tPCELL package_get_bind(tPCELL package, tPCELL name);
tPCELL package_get_bind_list(tPCELL package, tPCELL name);
VM_RET package_add_bind(tPVM vm, tPCELL package, tPCELL name, tPCELL* bind);
VM_RET package_add_bind_list(tPVM vm, tPCELL package, tPCELL name, tPCELL* blist);
VM_RET search_bind(tPVM vm, tPCELL current_package, tPCELL identifier, const tINT namespace_id, tPCELL* bind);

/////////////////////////////////////////////////
// CELL_FOREIGN_OBJECT

VM_RET foreign_object_create(tPVM vm, void* fobj, void* release, tPCELL my_class, tPCELL* cell);
tUINT foreign_object_get_size(tPCELL fobj);
void* foreign_object_get_object(tPCELL fobj);
VM_RET foreign_object_destroy(tPVM vm, tPCELL fobj);
VM_RET foreign_object_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET foreign_object_mark(tPVM vm, tPCELL cell);
void foreign_object_get_class(tPCELL fobj, tPOBJECT clss);

/////////////////////////////////////////////////
// CELL_FUNCTION

VM_RET function_create(tPVM vm, tPCELL plist, const tINT size, const tINT max, tPCELL package, tPCELL* cell);
VM_RET function_create_(tPVM vm, tPCELL plist, const tINT size, const tINT max, tPCELL package, tPCELL* cell);
tUINT function_get_size(tPCELL function);
VM_RET function_write(tPVM vm, tPCELL stream, tPOBJECT obj);
unsigned char* function_get_code_head(tPCELL function);
VM_RET function_call(tPVM vm, tPCELL function, tPCELL environment, tPOBJECT ret);
VM_RET function_call_(tPVM vm, tPCELL function, tPOBJECT ret);
tPCELL function_get_package(tPCELL function);
tINT function_get_parameter_number(tPCELL function);
tINT function_get_code_size(tPCELL function);
tBOOL function_is_rest(tPCELL function);
tBOOL function_is_heap(tPCELL function);
VM_RET function_add_use_object(tPVM vm, tPCELL function, tPOBJECT obj);
tBOOL function_is_macro(tPCELL function);
void function_set_macro(tPCELL function);
void function_set_body(tPCELL function, tPOBJECT body);
void function_get_body(tPCELL function, tPOBJECT body);
VM_RET function_mark(tPVM vm, tPCELL cell);

///////////////////
// CELL_MACRO

VM_RET macro_expand(tPVM vm, tPCELL macro, tPOBJECT ret);
tINT macro_get_parameter_number(tPCELL macro);
tBOOL macro_get_rest(tPCELL macro);

/////////////////////////////////////////////////
// CELL_ENVIRONMENT

VM_RET environment_create_(tPVM vm, const tINT n, tPCELL environment, tPCELL* cell);
tUINT environment_get_size(tPCELL environment);
tPCELL environment_get_environment(tPCELL env);
VM_RET environment_get_value(tPVM vm, tPCELL envnrionment, const tINT offset, tPOBJECT value);
VM_RET environment_set_value(tPVM vm, tPCELL envnrionment, const tINT offset, tPOBJECT value);
VM_RET environment_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET environment_mark(tPVM vm, tPCELL cell);
VM_RET environment_set_argument(tPVM vm, tPCELL environment, const tINT n);

/////////////////////////////////////////////////
// CELL_LOCAL_FUNCTION

VM_RET local_function_create_(tPVM vm, tPCELL function, tPCELL environment, tPCELL* cell);
tUINT local_function_get_size(tPCELL lfunction);
tPCELL local_function_get_function(tPCELL lfunction);
tPCELL local_function_get_environment(tPCELL lfunction);
VM_RET local_function_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET local_function_mark(tPVM vm, tPCELL cell);

/////////////////////////////////////////////////
// CELL_STANDARD_CLASS

VM_RET standard_class_create_(tPVM vm, tPCELL class_name, tPCELL sc_name_list, tPCELL slot_spec_list, const tBOOL abstractp, tPCELL* cell);
tUINT standard_class_get_size(tPCELL sclass);
VM_RET standard_class_write(tPVM vm, tPCELL sclass, tPOBJECT obj);
VM_RET standard_class_mark(tPVM vm, tPCELL cell);
tBOOL standard_class_is_subclass(tPVM vm, tPCELL sclass, tPCELL super);
tINT standard_class_get_precedence(tPVM vm, tPCELL sclass, tPOBJECT clss);
VM_RET sclass_initialize_instance(tPVM vm, tPCELL sclass, tPCELL instance, tPCELL list);
tBOOL standard_class_is_invalid(tPCELL sclass);
tPCELL standard_class_get_name(tPCELL sclass);
void standard_class_set_invalid(tPCELL sclass);

/////////////////////////////////////////////////
// CELL_SLOT

VM_RET slot_create_(tPVM vm, tPCELL name, tPCELL* cell);
tUINT slot_get_size(tPCELL slot);
VM_RET slot_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET slot_mark(tPVM vm, tPCELL cell);
VM_RET slot_add_initarg(tPVM vm, tPCELL slot, tPCELL initarg);
VM_RET slot_add_accessor(tPVM vm, tPCELL slot, tPCELL accessor);
VM_RET slot_set_initform(tPVM vm, tPCELL slot, tPOBJECT form);
VM_RET slot_initialize_instance(tPVM vm, tPCELL slot, tPCELL instance, tPCELL list);

/////////////////////////////////////////////////
// CELL_INSTANCE

VM_RET instance_create_(tPVM vm, tPCELL sclass, tPCELL* cell);
VM_RET instance_initialize(tPVM vm, tPCELL instance, tPCELL list);
tUINT instance_get_size(tPCELL instance);
VM_RET instance_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET instance_mark(tPVM vm, tPCELL cell);
tPCELL instance_get_class(tPCELL instance);
VM_RET instance_get_slot(tPVM vm, tPCELL instance, tPCELL slot_name, tPCELL sclass, tPOBJECT obj);
VM_RET instance_set_slot(tPVM vm, tPCELL instance, tPCELL slot_name, tPCELL sclass, tPOBJECT obj);
VM_RET instance_check_slot(tPVM vm, tPCELL instance, tPCELL slot_name, tPCELL sclass, tPOBJECT ret);
tBOOL instance_has_slot(tPCELL instance, tPCELL name);
VM_RET instance_add_slot(tPVM vm, tPCELL instance, tPCELL name, tPOBJECT value);

/////////////////////////////////////////////////
// CELL_GENERIC_FUNCTION

tUINT generic_function_get_size(tPCELL gfunction);
VM_RET generic_function_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET generic_function_mark(tPVM vm, tPCELL cell);
VM_RET gfunction_create_(tPVM vm, const tINT pnum, const tBOOL rest, const tBOOL standard, tPCELL lambda_list, tPCELL* cell);
VM_RET gfunction_add_method(tPVM vm, tPCELL gfunction, tPCELL method);
tINT gfunction_get_parameter_number(tPCELL gfunction);
tBOOL gfunction_is_rest(tPCELL gfunction);
VM_RET gfunction_call(tPVM vm, tPCELL gfunction, tPOBJECT valule);
tPCELL gfunction_get_around_method_list(tPCELL gfunction);
tPCELL gfunction_get_before_method_list(tPCELL gfunction);
tPCELL gfunction_get_primary_method_list(tPCELL gfunction);
tPCELL gfunction_get_after_method_list(tPCELL gfunction);
VM_RET gfunction_add_emethod(tPVM vm, tPCELL gfunction, tPCELL emethod);
void gfunction_clear_emethod(tPCELL gfunction);
VM_RET gfunction_create_reader(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass);
VM_RET gfunction_create_writer(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass);
VM_RET gfunction_create_boundp(tPVM vm, tPCELL name, tPCELL slot_name, tPCELL sclass);

/////////////////////////////////////////////////
// CELL_METHOD

#define METHOD_AROUND		0
#define METHOD_BEFORE		1
#define METHOD_PRIMARY		2
#define METHOD_AFTER		3

#define SYSTEM_METHOD_READER			0
#define SYSTEM_METHOD_WRITER			1
#define SYSTEM_METHOD_BOUNDP			2
#define SYSTEM_METHOD_CREATE			3
#define SYSTEM_METHOD_INITIALIZE_OBJECT	4
#define SYSTEM_METHOD_REPORT_CONDITION	5

VM_RET method_create_(tPVM vm, tPCELL function, tPCELL pplist, const tBOOL next, const tINT qualifier, tPOBJECT form, tPCELL* method);
VM_RET method_create_system(tPVM vm, tPCELL slot_name, tPCELL sclass, const tINT kind, tPCELL* cell);
tUINT method_get_size(tPCELL method);
VM_RET method_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET method_mark(tPVM vm, tPCELL cell);
tINT method_get_qualifier(tPCELL method);
tINT method_get_parameter_number(tPCELL method);
tBOOL method_is_rest(tPCELL method);
tBOOL method_is_next(tPCELL method);
tBOOL method_is_stack(tPCELL method);
tBOOL method_agreement_on_parameter_specializer(tPVM vm, tPCELL old_method, tPCELL new_method);
VM_RET method_call(tPVM vm, tPCELL method, tPCELL env, tPOBJECT ret);
tPCELL method_get_function(tPCELL method);

/////////////////////////////////////////////////
// CELL_EFFECTIVE_METHOD

VM_RET effective_method_create_(tPVM vm, tPCELL gfunction, tPCELL* cell);
tUINT effective_method_get_size(tPCELL emethod);
VM_RET effective_method_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET effective_method_mark(tPVM vm, tPCELL cell);
tPCELL emethod_get_around_method_list(tPCELL emethod);
tPCELL emethod_get_before_method_list(tPCELL emethod);
tPCELL emethod_get_primary_method_list(tPCELL emethod);
tPCELL emethod_get_after_method_list(tPCELL emethod);
VM_RET emethod_call_around(tPVM vm, tPCELL emethod, tPOBJECT value);
VM_RET emethod_call_no_around(tPVM vm, tPCELL emethod, tPOBJECT value);
VM_RET emethod_call_primary(tPVM vm, tPCELL emethod, tPOBJECT value);
tPCELL emethod_get_next(tPCELL emethod);
void emethod_set_next(tPCELL emethod, tPCELL next);
tBOOL emethod_is_applicable(tPCELL emethod, tPVM vm);

/////////////////////////////////////////////////
// CELL_APPLICABLE_METHOD

VM_RET applicable_method_create(tPVM vm, tPCELL emethod, tPCELL env, tPCELL* cell);
tUINT applicable_method_get_size(tPCELL amethod);
VM_RET applicable_method_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET applicable_method_mark(tPVM vm, tPCELL cell);
VM_RET amethod_call_around(tPVM vm, tPCELL amethod, tPCELL list);
VM_RET amethod_call_primary(tPVM vm, tPCELL amethod, tPCELL list);

/////////////////////////////////////////////////
// CELL_LINKED_FUNCTION

VM_RET linked_function_create_(tPVM vm, const tINT pnum, const tBOOL rest, tPCELL dll_name, tPCELL procedure_name, const tBOOL voidp, tPCELL lambda_list, tPCELL profile_list, tPCELL* cell);
tUINT linked_function_get_size(tPCELL lf);
VM_RET linked_function_mark(tPVM vm, tPCELL cell);
VM_RET linked_function_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET linked_function_call(tPVM vm, tPCELL lf, tPOBJECT ret);
tBOOL linked_function_is_rest(tPCELL lf);
tINT linked_function_get_parameter_number(tPCELL lf);

/////////////////////////////////////////////////
// CELL_LINKED_LIBRARY

VM_RET linked_library_create_(tPVM vm, tPCELL name, tPCELL* cell);
tUINT linked_library_get_size(tPCELL ll);
VM_RET linked_library_mark(tPVM vm, tPCELL cell);
VM_RET linked_library_write(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET linked_library_destroy(tPVM vm, tPCELL ll);
void* linked_library_get_handle(tPCELL ll);

/////////////////////////////////////////////////
// CELL_TISL_OBJECT

VM_RET tisl_object_create(tPVM vm, tPOBJECT obj, tPCELL* cell);
tUINT tisl_object_get_size(tPCELL tobj);
VM_RET tisl_object_mark(tPVM vm, tPCELL cell);
VM_RET tisl_object_write(tPVM vm, tPCELL stream, tPOBJECT obj);
void tisl_object_get_object(TISL_OBJECT tobj, tPOBJECT obj);
void tisl_object_set_object(TISL_OBJECT tobj, tPOBJECT obj);
tPCELL tisl_object_get_next(tPCELL tobj);
void tisl_object_set_next(tPCELL tobj, tPCELL next);

/////////////////////////////////////////////////
// CELL_FOREIGN_CLASS

VM_RET foreign_class_create_(tPVM vm, tPCELL name, tPCELL super_list, tPCELL* cell);
VM_RET foreign_class_create2_(tPVM vm, tPCELL name, tPCELL super, tPCELL* cell);
void foreign_class_set_super(tPCELL flcass, tPCELL super);
tPCELL foreign_class_get_name(tPCELL fclass);
tPCELL foreign_class_get_super(tPCELL flcass);
tUINT foreign_class_get_size(tPCELL fclass);
VM_RET foreign_class_mark(tPVM vm, tPCELL cell);
VM_RET foreign_class_write(tPVM vm, tPCELL stream, tPOBJECT obj);
tBOOL foreign_class_is_subclass(tPCELL fclass, tPCELL super);

/////////////////////////////////////////////////

tUINT cell_get_size(tPCELL cell);
VM_RET cell_destroy(tPVM vm, tPCELL cell);
void cell_to_object(tPCELL cell, tPOBJECT obj);
VM_RET cell_destroy_dummy(tPVM vm, tPCELL cell);
VM_RET cell_mark(tPVM vm, tPCELL cell);
VM_RET cell_mark_(tPVM vm, tPCELL cell);

void mark_cell(tPVM vm, tPCELL cell);
VM_RET mark_cell_(tPVM vm, tPCELL cell);

tBOOL cell_is_marked(tPVM vm, tPCELL cell);

typedef tUINT (*CELL_GET_SIZE)(tPCELL);
typedef VM_RET (*CELL_DESTROY)(tPVM, tPCELL);
typedef VM_RET (*CELL_MARK)(tPVM, tPCELL);

#ifdef TISL_OBJECT_C
const CELL_GET_SIZE cell_get_size_table[]={
	free_get_size,
	cons_get_size,
	string_get_size,
	symbol_get_size,
	vector_get_size,
	array_get_size,
	quasiquote_get_size,
	unquote_get_size,
	unquote_splicing_get_size,
	string_stream_get_size,
	file_stream_get_size,
	condition_get_size,
	package_get_size,
	bind_get_size,
	bind_list_get_size,
	foreign_object_get_size,
	function_get_size,
	environment_get_size,
	local_function_get_size,
	standard_class_get_size,
	generic_function_get_size,
	method_get_size,
	effective_method_get_size,
	applicable_method_get_size,
	instance_get_size,
	slot_get_size,
	linked_function_get_size,
	linked_library_get_size,
	tisl_object_get_size,
	foreign_class_get_size,
};

const CELL_DESTROY cell_destroy_table[]={
	cell_destroy_dummy,// free
	cell_destroy_dummy,// cons
	cell_destroy_dummy,// string
	cell_destroy_dummy,// symbol
	cell_destroy_dummy,// vector
	cell_destroy_dummy,// array
	cell_destroy_dummy,// quasiquote
	cell_destroy_dummy,// unquote
	cell_destroy_dummy,// unquote_splicing
	string_stream_destroy,
	file_stream_destroy,
	cell_destroy_dummy,// condition
	cell_destroy_dummy,// package
	cell_destroy_dummy,// bind
	cell_destroy_dummy,// bind-list
	foreign_object_destroy,// foreign-object
	cell_destroy_dummy,// function
	cell_destroy_dummy,// environment
	cell_destroy_dummy,// local-function
	cell_destroy_dummy,// standard-class
	cell_destroy_dummy,// generic-function
	cell_destroy_dummy,// method
	cell_destroy_dummy,// effective-method
	cell_destroy_dummy,// applicable-method
	cell_destroy_dummy,// instance
	cell_destroy_dummy,// slot
	cell_destroy_dummy,// linked-function
	linked_library_destroy,// linked-library
	cell_destroy_dummy,// tisl-object
	cell_destroy_dummy,// foreign-class
};

#endif // #ifdef TISL_OBJECT_C

#ifdef TISL_GC_MARK_TABLE
const CELL_MARK cell_mark_table[]={
	free_mark,
	cons_mark,
	string_mark,
	symbol_mark,
	vector_mark,
	array_mark,
	quasiquote_mark,
	unquote_mark,
	unquote_splicing_mark,
	string_stream_mark,
	file_stream_mark,
	condition_mark,
	package_mark,
	bind_mark,
	bind_list_mark,
	foreign_object_mark,
	function_mark,
	environment_mark,
	local_function_mark,
	standard_class_mark,
	generic_function_mark,
	method_mark,
	effective_method_mark,
	applicable_method_mark,
	instance_mark,
	slot_mark,
	linked_function_mark,
	linked_library_mark,
	tisl_object_mark,
	foreign_class_mark,
};
#endif

typedef VM_RET (*WRITE_OBJECT)(tPVM, tPCELL, tPOBJECT);

#ifdef TISL_WRITER_C
const WRITE_OBJECT object_write_table[]={
		unbound_write,
		nil_write,
		integer_write,
		float_write,
		character_write,
		built_in_class_write,
		special_operator_write,
		defining_operator_write,
		primitive_operator_write,
		unbound_write,//0x09
		unbound_write,//0x0a
		unbound_write,//0x0b
		unbound_write,//0x0c
		unbound_write,//0x0d
		unbound_write,//0x0e
		unbound_write,//0x0f
		free_write,
		cons_write,
		string_write,
		symbol_write,
		vector_write,
		array_write,
		quasiquote_write,
		unquote_write,
		unquote_splicing_write,
		string_stream_write,
		file_stream_write,
		condition_write,
		package_write,
		bind_write,
		bind_list_write,
		foreign_object_write,
		function_write,
		environment_write,
		local_function_write,
		standard_class_write,
		generic_function_write,
		method_write,
		effective_method_write,
		applicable_method_write,
		instance_write,
		slot_write,
		linked_function_write,
		linked_library_write,
		tisl_object_write,
		foreign_class_write,
};
#endif // #ifdef TISL_WRITRE_C

#endif // #ifndef TISL_OBJECT_H

