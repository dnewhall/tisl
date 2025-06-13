//
// TISL/src/tisl/object_2.c
// TISL Ver. 4.x
//

#include <memory.h>

#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "writer.h"
#include "gc.h"
#include "translator.h"
#include "built_in_object.h"

/////////////////////////////////////////////////
// CELL_CONDITION

#define CONDITION_SIZE		(sizeof(tCELL)*4+sizeof(tOBJECT)*4)

#define CONDITION_GET_SIZE(condition)				(((condition)+1)->ui)
#define CONDITION_SET_SIZE(condition, size)			(((condition)+1)->ui=(size))
#define CONDITION_GET_CLASS_ID(condition)			(((condition)+2)->i)
#define CONDITION_SET_CLASS_ID(condition, id)		(((condition)+2)->i=(id))
#define CONDITION_GET_NAME(condition)				(((condition)+3)->cell)
#define CONDITION_SET_NAME(condition, name)			(((condition)+3)->cell=(name))
#define CONDITION_GET_SLOT_HEAD(condition)			((tPOBJECT)((condition)+4))
#define CONDITION_GET_SLOT1(condition, obj)			(*(obj)=CONDITION_GET_SLOT_HEAD(condition)[0])
#define CONDITION_SET_SLOT1(condition, obj)			(CONDITION_GET_SLOT_HEAD(condition)[0]=*(obj))
#define CONDITION_GET_SLOT2(condition, obj)			(*(obj)=CONDITION_GET_SLOT_HEAD(condition)[1])
#define CONDITION_SET_SLOT2(condition, obj)			(CONDITION_GET_SLOT_HEAD(condition)[1]=*(obj))
#define CONDITION_GET_CONTINUABLE(condition, obj)	(*(obj)=CONDITION_GET_SLOT_HEAD(condition)[2])
#define CONDITION_SET_CONTINUABLE(condition, obj)	(CONDITION_GET_SLOT_HEAD(condition)[2]=*(obj))
#define CONDITION_GET_PLACE(condition, obj)			(*(obj)=CONDITION_GET_SLOT_HEAD(condition)[3])
#define CONDITION_SET_PLACE(condition, obj)			(CONDITION_GET_SLOT_HEAD(condition)[3]=*(obj))

VM_RET condition_create(tPVM vm, const tINT class_id, tPCELL name, tPOBJECT slot1, tPOBJECT slot2, tPOBJECT continuable, tPOBJECT place, tPCELL* cell)
{
	tUINT size;
	size=allocate_cell(vm, CONDITION_SIZE, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_CONDITION);
	CONDITION_SET_SIZE(*cell, size);
	CONDITION_SET_CLASS_ID(*cell, class_id);
	CONDITION_SET_NAME(*cell, name);
	CONDITION_SET_SLOT1(*cell, slot1);
	CONDITION_SET_SLOT2(*cell, slot2);
	CONDITION_SET_CONTINUABLE(*cell, continuable);
	CONDITION_SET_PLACE(*cell, place);
	return VM_OK;
}

tUINT condition_get_size(tPCELL condition)
{
	return CONDITION_GET_SIZE(condition);
}

static VM_RET domain_error_write_slot(tPVM vm, tPCELL stream, tPCELL condition);
static VM_RET simple_error_write_slot(tPVM vm, tPCELL stream, tPCELL condition);
static VM_RET stream_error_write_slot(tPVM vm, tPCELL stream, tPCELL condition);
static VM_RET undefined_entity_write_slot(tPVM vm, tPCELL stream, tPCELL condition);

VM_RET condition_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT place;
	tPCELL p=OBJECT_GET_CELL(obj);
	if (write_string(vm, stream, "#i(")) return VM_ERROR;
	if (write_string(vm, stream, built_in_class_get_name(CONDITION_GET_CLASS_ID(p)))) return VM_ERROR;
	if (write_string(vm, stream, " condition-name ")) return VM_ERROR;
	if (write_string(vm, stream, string_get_string(condition_get_name(p)))) return VM_ERROR;
	CONDITION_GET_PLACE(p, &place);
	if (!OBJECT_IS_UNBOUND(&place)) {
		if (write_string(vm, stream, " at ")) return VM_ERROR;
		if (write_object(vm, stream, &place)) return VM_ERROR;
	}
	switch (CONDITION_GET_CLASS_ID(p)) {
	case CLASS_DOMAIN_ERROR:
		if (domain_error_write_slot(vm, stream, p)) return VM_ERROR;
		break;
	case CLASS_SIMPLE_ERROR:
		if (simple_error_write_slot(vm, stream, p)) return VM_ERROR;
		break;
	case CLASS_STREAM_ERROR:
		if (stream_error_write_slot(vm, stream, p)) return VM_ERROR;
		break;
	case CLASS_UNDEFINED_ENTITY:
	case CLASS_UNBOUND_VARIABLE:
	case CLASS_UNDEFINED_FUNCTION:
		if (undefined_entity_write_slot(vm, stream, p)) return VM_ERROR;
		break;
	}
	return write_string(vm, stream, ")");
}

static VM_RET domain_error_write_slot(tPVM vm, tPCELL stream, tPCELL condition)
{
	tOBJECT object, expected_class;
	domain_error_get_object(condition, &object);
	domain_error_get_expected_class(condition, &expected_class);
	return write_string(vm, stream, " object ")||
		   write_object(vm, stream, &object)||
		   write_string(vm, stream, " expected-class ")||
		   write_object(vm, stream, &expected_class);

}

static VM_RET simple_error_write_slot(tPVM vm, tPCELL stream, tPCELL condition)
{
	tOBJECT string, arguments;
	simple_error_get_format_string(condition, &string);
	simple_error_get_format_argument(condition, &arguments);
	return write_string(vm, stream, " format-string ")||
		   write_object(vm, stream, &string)||
		   write_string(vm, stream, " format-arguments ")||
		   write_object(vm, stream, &arguments);
}

static VM_RET stream_error_write_slot(tPVM vm, tPCELL stream, tPCELL condition)
{
	tOBJECT obj;
	stream_error_get_stream(condition, &obj);
	return write_string(vm, stream, " stream ")||
		   write_object(vm, stream, &obj);
}

static VM_RET undefined_entity_write_slot(tPVM vm, tPCELL stream, tPCELL condition)
{
	tOBJECT name, space;
	undefined_entity_get_name(condition, &name);
	undefined_entity_get_namespace(condition, &space);
	return write_string(vm, stream, " name ")||
		   write_object(vm, stream, &name)||
		   write_string(vm, stream, " namespace ")||
		   write_object(vm, stream, &space);
}

tINT condition_get_class_id(tPCELL condition)
{
	return CONDITION_GET_CLASS_ID(condition);
}

tBOOL condition_is_continuable(tPCELL condition)
{
	tOBJECT obj;
	CONDITION_GET_CONTINUABLE(condition, &obj);
	return OBJECT_IS_UNBOUND(&obj) ? tFALSE : tTRUE;
}

tPCELL condition_get_name(tPCELL condition)
{
	return CONDITION_GET_NAME(condition);
}

void condition_get_slot1(tPCELL condition, tPOBJECT slot1)
{
	CONDITION_GET_SLOT1(condition, slot1);
}

void condition_get_slot2(tPCELL condition, tPOBJECT slot2)
{
	CONDITION_GET_SLOT2(condition, slot2);
}

VM_RET condition_mark(tPVM vm, tPCELL cell)
{
	tOBJECT obj;
	if (cell_mark(vm, CONDITION_GET_NAME(cell))) return VM_ERROR;
	CONDITION_GET_SLOT1(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	CONDITION_GET_SLOT2(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	CONDITION_GET_PLACE(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	CONDITION_GET_CONTINUABLE(cell, &obj);
	return gc_push(vm, &obj);
}

// 名前付き例外のスロットアクセス

// arithmetic-error
void arithmetic_error_get_operation(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT1(condition, obj);
}

void arithmetic_error_set_operation(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT1(condition, obj);
}

void arithmetic_error_get_operands(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT2(condition, obj);
}

void arithmetic_error_set_operands(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT2(condition, obj);
}

// domain-error
void domain_error_get_expected_class(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT1(condition, obj);
}

void domain_error_set_expected_class(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT1(condition, obj);
}

void domain_error_get_object(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT2(condition, obj);
}

void domain_error_set_object(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT2(condition, obj);
}

// parse-error
void parse_error_get_string(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT1(condition, obj);
}

void parse_error_set_string(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT1(condition, obj);
}

void parse_error_get_expected_class(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT2(condition, obj);
}

void parse_error_set_expected_class(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT2(condition, obj);
}

// stream-error
void stream_error_get_stream(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT1(condition, obj);
}

void stream_error_set_stream(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT1(condition, obj);
}

// undefined-entity
void undefined_entity_set_name(tPCELL condition, tPOBJECT name)
{
	CONDITION_SET_SLOT1(condition, name);
}

void undefined_entity_get_name(tPCELL condition, tPOBJECT name)
{
	CONDITION_GET_SLOT1(condition, name);
}

void undefined_entity_set_namespace(tPCELL condition, const int namespace_id)
{
	tOBJECT obj;
	switch (namespace_id) {
	case NAMESPACE_VARIABLE:	OBJECT_SET_SYMBOL(&obj, global_symbol[sVARIABLE]);			break;
	case NAMESPACE_FUNCTION:	OBJECT_SET_SYMBOL(&obj, global_symbol[sFUNCTION]);			break;
	case NAMESPACE_DYNAMIC:		OBJECT_SET_SYMBOL(&obj, global_symbol[sDYNAMIC_VARIABLE]);	break;
	case NAMESPACE_CLASS:		OBJECT_SET_SYMBOL(&obj, global_symbol[sCLASS]);				break;
	case NAMESPACE_PACKAGE:		OBJECT_SET_SYMBOL(&obj, global_symbol[sPACKAGE]);			break;
	default:OBJECT_SET_UNBOUND(&obj);//エラー？
	}
	CONDITION_SET_SLOT2(condition, &obj);
}

void undefined_entity_get_namespace(tPCELL condition, tPOBJECT name)
{
	CONDITION_GET_SLOT2(condition, name);
}

// simple-error
void simple_error_get_format_string(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT1(condition, obj);
}

void simple_error_set_format_string(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT1(condition, obj);
}

void simple_error_get_format_argument(tPCELL condition, tPOBJECT obj)
{
	CONDITION_GET_SLOT2(condition, obj);
}

void simple_error_set_format_argument(tPCELL condition, tPOBJECT obj)
{
	CONDITION_SET_SLOT2(condition, obj);
}

/////////////////////////////////////////////////
// CELL_PACKAGE

#define PACKAGE_BIND_TABLE_SIZE					128
#define PACKAGE_BIND_LIST_TABLE_SIZE			128

#define PACKAGE_GET_SIZE(package)								(((package)+1)->ui)
#define PACKAGE_SET_SIZE(package, size)							(((package)+1)->ui=(size))
#define PACKAGE_GET_BIND(package)								(((package)+2)->cell)
#define PACKAGE_SET_BIND(package, bind)							(((package)+2)->cell=(bind))
#define PACKAGE_GET_NAME_PRECEDENCE_LIST(package)				(((package)+3)->cell)
#define PACKAGE_SET_NAME_PRECEDENCE_LIST(package, list)			(((package)+3)->cell=(list))
#define PACKAGE_GET_NAME(package)								(((package)+4)->cell)
#define PACKAGE_SET_NAME(package, name)							(((package)+4)->cell=(name))
#define PACKAGE_GET_PARENT(package)								(((package)+5)->cell)
#define PACKAGE_SET_PARENT(package, p)							(((package)+5)->cell=(p))
#define PACKAGE_GET_BIND_TABLE(package)							((tPCELL*)((package)+6))
#define PACKAGE_GET_BIND_LIST_TABLE(package)					((tPCELL*)(PACKAGE_GET_BIND_TABLE(package)+PACKAGE_BIND_TABLE_SIZE))

VM_RET package_create(tPVM vm, tPCELL bind, tPCELL name_precedence_list, tPCELL name, tPCELL parent, tPCELL* cell)
{
	VM_RET ret;
	if (bind) {
		tOBJECT obj;
		cell_to_object(bind, &obj);
		if (vm_push(vm, &obj)) return VM_ERROR;
	}
	if (name_precedence_list) {
		tOBJECT obj;
		cell_to_object(name_precedence_list, &obj);
		if (vm_push(vm, &obj)) return VM_ERROR;
	}
	ret=package_create_(vm, bind, name_precedence_list, name, parent, cell);
	if (bind) vm_pop(vm);
	if (name_precedence_list) vm_pop(vm);
	return ret;
}

VM_RET package_create_(tPVM vm, tPCELL bind, tPCELL name_precedence_list, tPCELL name, tPCELL parent, tPCELL* cell)
{
	tUINT size;

	size=sizeof(tCELL)*6+sizeof(tPCELL)*(PACKAGE_BIND_TABLE_SIZE+PACKAGE_BIND_LIST_TABLE_SIZE);
	size=allocate_cell(vm, size, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_PACKAGE);
	PACKAGE_SET_SIZE(*cell, size);
	PACKAGE_SET_BIND(*cell, bind);
	PACKAGE_SET_NAME_PRECEDENCE_LIST(*cell, name_precedence_list);
	PACKAGE_SET_NAME(*cell, name);
	PACKAGE_SET_PARENT(*cell, parent);

	memset(PACKAGE_GET_BIND_TABLE(*cell), 0, sizeof(tPCELL)*PACKAGE_BIND_TABLE_SIZE);
	memset(PACKAGE_GET_BIND_LIST_TABLE(*cell), 0, sizeof(tPCELL)*PACKAGE_BIND_LIST_TABLE_SIZE);
	return VM_OK;
}

VM_RET package_reset(tPVM vm, tPCELL npl, tPCELL package)
{
	tINT i;
	tPCELL* pp=PACKAGE_GET_BIND_LIST_TABLE(package);
	PACKAGE_SET_NAME_PRECEDENCE_LIST(package, npl);
	for (i=0; i<PACKAGE_BIND_LIST_TABLE_SIZE; i++, pp++) {
		if (bind_list_reset(vm, package, *pp)) return VM_ERROR;
	}
	return VM_OK;
}

tUINT package_get_size(tPCELL package)
{
	return PACKAGE_GET_SIZE(package);
}

tINT package_get_length_of_name_precedence_list(tPCELL package)
{
	return cons_get_length(PACKAGE_GET_NAME_PRECEDENCE_LIST(package));
}

tPCELL package_get_use_package(tPVM vm, tPCELL package, const tINT precedence)
{
	if (precedence) {
		tOBJECT tmp;
		if (list_get_object(vm, PACKAGE_GET_NAME_PRECEDENCE_LIST(package), precedence-1, &tmp)) return 0;
		return OBJECT_GET_CELL(&tmp);
	} else {
		return package;
	}
}

VM_RET package_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{//エラーか？
	return write_string(vm, stream, "#i(<package>)");
}

VM_RET package_write_(tPVM vm, tPCELL stream, tPCELL package)
{
	if (PACKAGE_GET_PARENT(package)) {
		if (package_write_(vm, stream, PACKAGE_GET_PARENT(package))) return VM_ERROR;
	}
	if (PACKAGE_GET_NAME(package)) {
		if (write_string(vm, stream, ":")) return VM_ERROR;
		if (write_string(vm, stream, string_get_string(PACKAGE_GET_NAME(package)))) return VM_ERROR;
	}
	return VM_OK;
}

VM_RET package_mark(tPVM vm, tPCELL cell)
{
	tINT i;
	tPCELL* p;
	if (cell_mark(vm, PACKAGE_GET_NAME(cell))) return VM_ERROR;
	if (cell_mark(vm, PACKAGE_GET_NAME_PRECEDENCE_LIST(cell))) return VM_ERROR;
	p=PACKAGE_GET_BIND_TABLE(cell);
	for (i=0; i<PACKAGE_BIND_TABLE_SIZE; i++) {
		if (cell_mark(vm, *p++)) return VM_ERROR;
	}
	p=PACKAGE_GET_BIND_LIST_TABLE(cell);
	for (i=0; i<PACKAGE_BIND_LIST_TABLE_SIZE; i++) {
		if (cell_mark(vm, *p++)) return VM_ERROR;
	}
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_BIND

#define BIND_PROPERTY_TABLE_SIZE		32

#define BIND_CONSTANT			0x00000001
#define BIND_CONSTANT_			0xfffffffe

#define BIND_VARIABLE_PUBLIC	0x00000002
#define BIND_VARIABLE_PUBLIC_	0xfffffffd
#define BIND_FUNCTION_PUBLIC	0x00000004
#define BIND_FUNCTION_PUBLIC_	0xfffffffb
#define BIND_DYNAMIC_PUBLIC		0x00000008
#define BIND_DYNAMIC_PUBLIC_	0xfffffff7
#define BIND_CLASS_PUBLIC		0x00000010
#define BIND_CLASS_PUBLIC_		0xffffffef
#define BIND_PACKAGE_PUBLIC		0x00000020
#define BIND_PACKAGE_PUBLIC_	0xffffffdf

#define BIND_GET_CONSTANT(bind)				((bind)->ui&BIND_CONSTANT)
#define BIND_SET_CONSTANT(bind)				((bind)->ui|=BIND_CONSTANT)
#define BIND_RESET_CONSTANT(bind)			((bind)->ui&=BIND_CONSTANT_)
#define BIND_GET_VARIABLE_PUBLIC(bind)		((bind)->ui&BIND_VARIABLE_PUBLIC)
#define BIND_SET_VARIABLE_PUBLIC(bind)		((bind)->ui|=BIND_VARIABLE_PUBLIC)
#define BIND_RESET_VARIABLE_PUBLIC(bind)	((bind)->ui&=BIND_VARIABLE_PUBLIC_)
#define BIND_GET_FUNCTION_PUBLIC(bind)		((bind)->ui&BIND_FUNCTION_PUBLIC)
#define BIND_SET_FUNCTION_PUBLIC(bind)		((bind)->ui|=BIND_FUNCTION_PUBLIC)
#define BIND_RESET_FUNCTION_PUBLIC(bind)	((bind)->ui&=BIND_FUNCTION_PUBLIC_)
#define BIND_GET_DYNAMIC_PUBLIC(bind)		((bind)->ui&BIND_DYNAMIC_PUBLIC)
#define BIND_SET_DYNAMIC_PUBLIC(bind)		((bind)->ui|=BIND_DYNAMIC_PUBLIC)
#define BIND_RESET_DYNAMIC_PUBLIC(bind)		((bind)->ui&=BIND_DYNAMIC_PUBLIC_)
#define BIND_GET_CLASS_PUBLIC(bind)			((bind)->ui&BIND_CLASS_PUBLIC)
#define BIND_SET_CLASS_PUBLIC(bind)			((bind)->ui|=BIND_CLASS_PUBLIC)
#define BIND_RESET_CLASS_PUBLIC(bind)		((bind)->ui&=BIND_CLASS_PUBLIC_)
#define BIND_GET_PACKAGE_PUBLIC(bind)		((bind)->ui&BIND_PACKAGE_PUBLIC)
#define BIND_SET_PACKAGE_PUBLIC(bind)		((bind)->ui|=BIND_PACKAGE_PUBLIC)
#define BIND_RESET_PACKAGE_PUBLIC(bind)		((bind)->ui&=BIND_PACKAGE_PUBLIC_)

#define BIND_GET_SIZE(bind)				(((bind)+1)->ui)
#define BIND_SET_SIZE(bind, size)		(((bind)+1)->ui=(size))
#define BIND_GET_NAME(bind)				(((bind)+2)->cell)
#define BIND_SET_NAME(bind, name)		(((bind)+2)->cell=(name))
#define BIND_GET_PARENT(bind)			(((bind)+3)->cell)
#define BIND_SET_PARENT(bind, parent)	(((bind)+3)->cell=(parent))
#define BIND_GET_NEXT(bind)				(((bind)+4)->cell)
#define BIND_SET_NEXT(bind, next)		(((bind)+4)->cell=(next))

#define BIND_GET_BIND_TABLE(bind)		((tPOBJECT)((bind)+5))

#define BIND_GET_VARIABLE(bind, obj)	(*(obj)=BIND_GET_BIND_TABLE(bind)[0])
#define BIND_SET_VARIABLE(bind, obj)	(BIND_GET_BIND_TABLE(bind)[0]=*(obj))
#define BIND_GET_FUNCTION(bind, obj)	(*(obj)=BIND_GET_BIND_TABLE(bind)[1])
#define BIND_SET_FUNCTION(bind, obj)	(BIND_GET_BIND_TABLE(bind)[1]=*(obj))
#define BIND_GET_DYNAMIC(bind, obj)		(*(obj)=BIND_GET_BIND_TABLE(bind)[2])
#define BIND_SET_DYNAMIC(bind, obj)		(BIND_GET_BIND_TABLE(bind)[2]=*(obj))
#define BIND_GET_CLASS(bind, obj)		(*(obj)=BIND_GET_BIND_TABLE(bind)[3])
#define BIND_SET_CLASS(bind, obj)		(BIND_GET_BIND_TABLE(bind)[3]=*(obj))
#define BIND_GET_PACKAGE(bind, obj)		(*(obj)=BIND_GET_BIND_TABLE(bind)[4])
#define BIND_SET_PACKAGE(bind, obj)		(BIND_GET_BIND_TABLE(bind)[4]=*(obj))

#define BIND_GET_PROPERTY_TABLE(bind)	((tPCELL*)(BIND_GET_BIND_TABLE(bind)+5))

VM_RET bind_create(tPVM vm, tPCELL name, tPCELL package, tPCELL* cell)
{
	tOBJECT obj;
	VM_RET ret;
	cell_to_object(name, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	cell_to_object(package, &obj);
	if (vm_push(vm, &obj)) return VM_ERROR;
	ret=bind_create_(vm, name, package, cell);
	vm_pop(vm);
	vm_pop(vm);
	return ret;
}

VM_RET bind_create_(tPVM vm, tPCELL name, tPCELL package, tPCELL* cell)
{
	tUINT size;
	size=sizeof(tCELL)*5+sizeof(tOBJECT)*5+sizeof(tPCELL)*BIND_PROPERTY_TABLE_SIZE;
	size=allocate_cell(vm, size, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_BIND);
	BIND_SET_SIZE(*cell, size);
	BIND_SET_NAME(*cell, name);
	BIND_SET_PARENT(*cell, package);
	BIND_SET_NEXT(*cell, 0);

	BIND_RESET_CONSTANT(*cell);

	BIND_RESET_VARIABLE_PUBLIC(*cell);
	BIND_RESET_FUNCTION_PUBLIC(*cell);
	BIND_RESET_DYNAMIC_PUBLIC(*cell);
	BIND_RESET_CLASS_PUBLIC(*cell);
	BIND_RESET_PACKAGE_PUBLIC(*cell);

	BIND_SET_VARIABLE(*cell, &unbound);
	BIND_SET_FUNCTION(*cell, &unbound);
	BIND_SET_DYNAMIC(*cell, &unbound);
	BIND_SET_CLASS(*cell, &unbound);
	BIND_SET_PACKAGE(*cell, &unbound);

	memset(BIND_GET_PROPERTY_TABLE(*cell), 0, sizeof(tPCELL)*BIND_PROPERTY_TABLE_SIZE);
	return VM_OK;
}

tUINT bind_get_size(tPCELL bind)
{
	return BIND_GET_SIZE(bind);
}

void bind_get_object(tPCELL bind, const tINT namespace_id, tPOBJECT obj)
{
	switch (namespace_id) {
	case NAMESPACE_VARIABLE:	BIND_GET_VARIABLE(bind, obj);	break;
	case NAMESPACE_FUNCTION:	BIND_GET_FUNCTION(bind, obj);	break;
	case NAMESPACE_DYNAMIC:		BIND_GET_DYNAMIC(bind, obj);	break;
	case NAMESPACE_CLASS:		BIND_GET_CLASS(bind, obj);		break;
	case NAMESPACE_PACKAGE:		BIND_GET_PACKAGE(bind, obj);	break;
	default:					OBJECT_SET_UNBOUND(obj);		break;
	}
}

void bind_set_object(tPCELL bind, const tINT namespace_id, tPOBJECT obj)
{
	switch (namespace_id) {
	case NAMESPACE_VARIABLE:	BIND_SET_VARIABLE(bind, obj);	break;
	case NAMESPACE_FUNCTION:	BIND_SET_FUNCTION(bind, obj);	break;
	case NAMESPACE_DYNAMIC:		BIND_SET_DYNAMIC(bind, obj);	break;
	case NAMESPACE_CLASS:		BIND_SET_CLASS(bind, obj);		break;
	case NAMESPACE_PACKAGE:		BIND_SET_PACKAGE(bind, obj);	break;
	}
}

void bind_get_variable(tPCELL bind, tPOBJECT obj)
{
	BIND_GET_VARIABLE(bind, obj);
}

void bind_set_variable(tPCELL bind, tPOBJECT obj)
{
	BIND_SET_VARIABLE(bind, obj);
}

void bind_get_function(tPCELL bind, tPOBJECT obj)
{
	BIND_GET_FUNCTION(bind, obj);
}

void bind_set_function(tPCELL bind, tPOBJECT obj)
{
	BIND_SET_FUNCTION(bind, obj);
}

void bind_get_dynamic(tPCELL bind, tPOBJECT obj)
{
	BIND_GET_DYNAMIC(bind, obj);
}

void bind_set_dynamic(tPCELL bind, tPOBJECT obj)
{
	BIND_SET_DYNAMIC(bind, obj);
}

void bind_get_class(tPCELL bind, tPOBJECT obj)
{
	BIND_GET_CLASS(bind, obj);
}

void bind_set_class(tPCELL bind, tPOBJECT obj)
{
	BIND_SET_CLASS(bind, obj);
}

void bind_get_package(tPCELL bind, tPOBJECT obj)
{
	BIND_GET_PACKAGE(bind, obj);
}

void bind_set_package(tPCELL bind, tPOBJECT obj)
{
	BIND_SET_PACKAGE(bind, obj);
}

tPCELL bind_get_name(tPCELL bind)
{
	return BIND_GET_NAME(bind);
}

VM_RET bind_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{/*!!!*///例外？
	return write_string(vm, stream, "#tisl(bind)");
}

VM_RET bind_mark(tPVM vm, tPCELL cell)
{
	tOBJECT obj;
	tINT i;
	tPCELL* p;

	if (cell_mark(vm, BIND_GET_NAME(cell))) return VM_ERROR;
	if (cell_mark(vm, BIND_GET_NEXT(cell))) return VM_ERROR;
	BIND_GET_VARIABLE(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	BIND_GET_FUNCTION(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	BIND_GET_DYNAMIC(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	BIND_GET_CLASS(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;
	BIND_GET_PACKAGE(cell, &obj);
	if (gc_push(vm, &obj)) return VM_ERROR;

	p=BIND_GET_PROPERTY_TABLE(cell);
	for (i=0; i<BIND_PROPERTY_TABLE_SIZE; i++) {
		if (cell_mark(vm, *p++)) return VM_ERROR;
	}
	return VM_OK;
}

void bind_set_variable_public(tPCELL bind)
{
	BIND_SET_VARIABLE_PUBLIC(bind);
}

void bind_set_variable_private(tPCELL bind)
{
	BIND_RESET_VARIABLE_PUBLIC(bind);
}

tBOOL bind_variable_is_public(tPCELL bind)
{
	return BIND_GET_VARIABLE_PUBLIC(bind) ? tTRUE : tFALSE;
}

void bind_set_function_public(tPCELL bind)
{
	BIND_SET_FUNCTION_PUBLIC(bind);
}

void bind_set_function_private(tPCELL bind)
{
	BIND_RESET_FUNCTION_PUBLIC(bind);
}

tBOOL bind_function_is_public(tPCELL bind)
{
	return BIND_GET_FUNCTION_PUBLIC(bind) ? tTRUE : tFALSE;
}

void bind_set_dynamic_public(tPCELL bind)
{
	BIND_SET_DYNAMIC_PUBLIC(bind);
}

void bind_set_dynamic_private(tPCELL bind)
{
	BIND_RESET_DYNAMIC_PUBLIC(bind);
}

tBOOL bind_dyanmic_is_public(tPCELL bind)
{
	return BIND_GET_DYNAMIC_PUBLIC(bind) ? tTRUE : tFALSE;
}

void bind_set_class_public(tPCELL bind)
{
	BIND_SET_CLASS_PUBLIC(bind);
}

void bind_set_class_private(tPCELL bind)
{
	BIND_RESET_CLASS_PUBLIC(bind);
}

tBOOL bind_class_is_public(tPCELL bind)
{
	return BIND_GET_CLASS_PUBLIC(bind) ? tTRUE : tFALSE;
}

void bind_set_package_public(tPCELL bind)
{
	BIND_SET_PACKAGE_PUBLIC(bind);
}

void bind_set_package_private(tPCELL bind)
{
	BIND_RESET_PACKAGE_PUBLIC(bind);
}

tBOOL bind_package_is_public(tPCELL bind)
{
	return BIND_GET_PACKAGE_PUBLIC(bind) ? tTRUE : tFALSE;
}

// 属性リスト

static tINT bind_get_property_key(tPCELL property_name)
{
	tINT key=((tINT)property_name)/23%BIND_PROPERTY_TABLE_SIZE;
	return (key<0) ? -key : key;
}

// propety_name は CELL_SYMBOL
tBOOL bind_get_property(tPCELL bind, tPCELL property_name, tPOBJECT obj)
{
	tINT key;
	tPCELL p, pair;
	tOBJECT name;

	key=bind_get_property_key(property_name);
	// a-list?
	for (p=BIND_GET_PROPERTY_TABLE(bind)[key]; p; p=cons_get_cdr_cons(p)) {
		pair=cons_get_car_cons(p);
		cons_get_car(pair, &name);
		if (OBJECT_GET_CELL(&name)==property_name) {
			cons_get_cdr(pair, obj);
			return tTRUE;
		}
	}
	return tFALSE;
}

VM_RET bind_set_property(tPVM vm, tPCELL bind, tPCELL property_name, tPOBJECT obj)
{
	tINT key;
	tPCELL p, pair;
	tOBJECT name, tmp;

	key=bind_get_property_key(property_name);
	for (p=BIND_GET_PROPERTY_TABLE(bind)[key]; p; p=cons_get_cdr_cons(p)) {
		pair=cons_get_car_cons(p);
		cons_get_car(pair, &name);
		if (OBJECT_GET_CELL(&name)==property_name) {
			cons_set_cdr(pair, obj);
			return VM_OK;
		}
	}
	// 新規に作成
	if (BIND_GET_PROPERTY_TABLE(bind)[key]) {
		OBJECT_SET_CONS(&tmp, BIND_GET_PROPERTY_TABLE(bind)[key]);
	} else {
		OBJECT_SET_NIL(&tmp);
	}
	if (cons_create_(vm, &pair, &nil, &tmp)) return VM_ERROR;
	BIND_GET_PROPERTY_TABLE(bind)[key]=pair;
	OBJECT_SET_SYMBOL(&tmp, property_name);
	if (cons_create_(vm, &p, &tmp, obj)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, p);
	cons_set_car(pair, &tmp);
	return VM_OK;
}

void bind_remove_property(tPCELL bind, tPCELL property_name, tPOBJECT obj)
{
	tINT key;
	tPCELL p, last, pair;
	tOBJECT name;

	key=bind_get_property_key(property_name);
	for (p=BIND_GET_PROPERTY_TABLE(bind)[key], last=0; p; last=p, p=cons_get_cdr_cons(p)) {
		pair=cons_get_car_cons(p);
		cons_get_car(pair, &name);
		if (OBJECT_GET_CELL(&name)==property_name) {
			if (last) {
				tOBJECT tmp;
				cons_get_cdr(p, &tmp);
				cons_set_cdr(last, &tmp);
			} else {
				BIND_GET_PROPERTY_TABLE(bind)[key]=cons_get_cdr_cons(p);
			}
			cons_get_cdr(pair, obj);
			return;
		} 
	}
	OBJECT_SET_NIL(obj);
}

/////////////////////////////////////////////////
// CELL_BIND_LIST

#define BIND_LIST_SIZE		(sizeof(tCELL)*6)

#define BIND_LIST_GET_SIZE(list)			(((list)+1)->ui)
#define BIND_LIST_SET_SIZE(list, size)		(((list)+1)->ui=(size))
#define BIND_LIST_GET_PACKAGE(list)			(((list)+2)->cell)
#define BIND_LIST_SET_PACKAGE(list, p)		(((list)+2)->cell=(p))
#define BIND_LIST_GET_IDENTIFIER(list)		(((list)+3)->cell)
#define BIND_LIST_SET_IDENTIFIER(list, p)	(((list)+3)->cell=(p))
#define BIND_LIST_GET_NEXT(list)			(((list)+4)->cell)
#define BIND_LIST_SET_NEXT(list, next)		(((list)+4)->cell=(next))
#define BIND_LIST_GET_LIST(list)			(((list)+5)->cell)
#define BIND_LIST_SET_LIST(list, p)			(((list)+5)->cell=(p))

VM_RET bind_list_create(tPVM vm, tPCELL package, tPCELL identifier, tPCELL list, tPCELL* cell)
{
	return bind_list_create(vm, package, identifier, list, cell);
}

VM_RET bind_list_create_(tPVM vm, tPCELL package, tPCELL identifier, tPCELL list, tPCELL* cell)
{
	tUINT size;

	size=allocate_cell(vm, BIND_LIST_SIZE, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);

	CELL_SET_TYPE(*cell, CELL_BIND_LIST);

	BIND_LIST_SET_SIZE(*cell, size);
	BIND_LIST_SET_PACKAGE(*cell, package);
	BIND_LIST_SET_IDENTIFIER(*cell, identifier);
	BIND_LIST_SET_NEXT(*cell, 0);
	BIND_LIST_SET_LIST(*cell, list);
	return VM_OK;
}

tUINT bind_list_get_size(tPCELL bind_list)
{
	return BIND_LIST_GET_SIZE(bind_list);
}

tPCELL bind_list_get_bind(tPCELL bind_list, const tINT namespace_id, tPCELL current)
{
	if (namespace_id==NAMESPACE_DYNAMIC) {
		tOBJECT obj;// 一番優先度の高いものを利用する．
		cons_get_car(BIND_LIST_GET_LIST(bind_list), &obj);
		return OBJECT_GET_CELL(&obj);
	} else {
		tPCELL p, bind;
		tOBJECT obj;
		tBOOL visible;
		for (p=BIND_LIST_GET_LIST(bind_list); p; p=cons_get_cdr_cons(p)) {
			cons_get_car(p, &obj);
			if (!OBJECT_IS_BIND(&obj)) return 0;
			bind=OBJECT_GET_CELL(&obj);
			switch (namespace_id) {
			case NAMESPACE_VARIABLE:	bind_get_variable(bind, &obj); visible=bind_variable_is_public(bind); break;
			case NAMESPACE_FUNCTION:	bind_get_function(bind, &obj); visible=bind_function_is_public(bind); break;
			case NAMESPACE_CLASS:		bind_get_class(bind, &obj); visible=bind_class_is_public(bind); break;
			case NAMESPACE_PACKAGE:		bind_get_package(bind, &obj); visible=bind_package_is_public(bind); break;
			default:
				return 0;
			}
			if ((visible||(BIND_GET_PARENT(bind)==current)) &&
				!OBJECT_IS_UNBOUND(&obj)) return bind;
		}
	}
	return 0;
}

tBOOL bind_list_bind_is_head(tPCELL blist, tPCELL bind)
{
	tPCELL p=BIND_LIST_GET_LIST(blist);
	if (p) {
		tOBJECT tmp;
		cons_get_car(p, &tmp);
		return (OBJECT_GET_CELL(&tmp)==bind) ? tTRUE : tFALSE;
	}
	return tFALSE;
}


VM_RET bind_list_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{/*!!!*///例外？
	return write_string(vm, stream, "#tisl(bind-list)");
}

tPCELL bind_list_get_name(tPCELL bind_list)
{
	return BIND_LIST_GET_IDENTIFIER(bind_list);
}

VM_RET bind_list_mark(tPVM vm, tPCELL cell)
{
	if (cell_mark(vm, BIND_LIST_GET_IDENTIFIER(cell))) return VM_ERROR;
	if (cell_mark(vm, BIND_LIST_GET_NEXT(cell))) return VM_ERROR;
	if (cell_mark(vm, BIND_LIST_GET_LIST(cell))) return VM_ERROR;
	return VM_OK;
}

/////////////////////////////////////////////////
// package bind bind-list関係

static tINT package_get_bind_table_key(tPCELL name);
static tINT package_get_bind_list_table_key(tPCELL identifier);
static VM_RET make_bind_list(tPVM vm, tPCELL package, tPCELL identifier, tPCELL* blist);
static VM_RET make_list_of_bind_list(tPVM vm, tPCELL list, const tPCELL current, tPCELL package, tPCELL identifier, const tINT i);
static VM_RET tail_list_add_object(tPVM vm, tPCELL list, tPOBJECT obj);

tPCELL package_get_bind(tPCELL package, tPCELL name)
{
	tINT key=package_get_bind_table_key(name);
	tPCELL p;

	for (p=PACKAGE_GET_BIND_TABLE(package)[key]; p; p=BIND_GET_NEXT(p)) {
		if (BIND_GET_NAME(p)==name) return p;
	}
	return 0;
}

tPCELL package_get_bind_list(tPCELL package, tPCELL name)
{
	tINT key=package_get_bind_list_table_key(name);
	tPCELL p;

	for (p=PACKAGE_GET_BIND_LIST_TABLE(package)[key]; p; p=BIND_LIST_GET_NEXT(p)) {
		if (BIND_LIST_GET_IDENTIFIER(p)==name) return p;
	}
	return 0;
}

VM_RET package_add_bind(tPVM vm, tPCELL package, tPCELL name, tPCELL* bind)
{
	tINT key;
	// 既に存在するものから検索
	*bind=package_get_bind(package, name);
	if (*bind) return VM_OK;
	// 新規に作成する
	if (bind_create(vm, name, package, bind)) return VM_ERROR;
	// 登録
	key=package_get_bind_table_key(name);
	BIND_SET_NEXT(*bind, PACKAGE_GET_BIND_TABLE(package)[key]);
	PACKAGE_GET_BIND_TABLE(package)[key]=*bind;
	return VM_OK;
}

VM_RET package_add_bind_list(tPVM vm, tPCELL package, tPCELL name, tPCELL* blist)
{
	tINT key;
	// 既にあるものから検索
	*blist=package_get_bind_list(package, name);
	if (*blist) return VM_OK;
	// 新規に作成
	if (make_bind_list(vm, package, name, blist)) return VM_ERROR;
	// 登録
	key=package_get_bind_list_table_key(name);
	BIND_LIST_SET_NEXT(*blist, PACKAGE_GET_BIND_LIST_TABLE(package)[key]);
	PACKAGE_GET_BIND_LIST_TABLE(package)[key]=*blist;
	return VM_OK;
}

VM_RET search_bind(tPVM vm, tPCELL current_package, tPCELL identifier, const tINT namespace_id, tPCELL* bind)
{
	tPCELL blist;
	// blistが既に作成されているばそれを利用する
	blist=package_get_bind_list(current_package, identifier);
	if (!blist) {
		// 存在しなければ作成しておく
		if (package_add_bind_list(vm, current_package, identifier, &blist)) return VM_ERROR;
	}
	*bind=bind_list_get_bind(blist, namespace_id, current_package);
	return VM_OK;
}

static tINT package_get_bind_table_key(tPCELL name)
{/*!!!*/// てきとー 後で統計でもとる？
	tINT key=((tINT)name)/23%PACKAGE_BIND_TABLE_SIZE;
	return (key<0) ? -key : key;
}

static tINT package_get_bind_list_table_key(tPCELL identifier)
{/*!!!*/// てきとー 後で統計でもとる？
	tINT key=((tINT)identifier)/23%PACKAGE_BIND_LIST_TABLE_SIZE;
	return (key<0) ? -key : key;
}

static VM_RET make_bind_list(tPVM vm, tPCELL package, tPCELL identifier, tPCELL* blist)
{
	tPCELL list;
	tOBJECT obj;
	VM_RET ret;
	// 優先度順のリストを作成する
	if (cons_create_(vm, &list, &nil, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, list);
	if (vm_push(vm, &obj)) return VM_ERROR;
	cons_set_car(list, &obj);
	if (make_list_of_bind_list(vm, list, package, symbol_is_complete(identifier) ? vm_get_top_package(vm) : package, identifier, 0)) { vm_pop(vm); return VM_ERROR; }
	cons_get_cdr(list, &obj);
	list=OBJECT_IS_CONS(&obj) ? OBJECT_GET_CELL(&obj) : 0;
	ret=bind_list_create_(vm, package, identifier, list, blist);
	vm_pop(vm);
	return ret;
}

VM_RET bind_list_reset(tPVM vm, tPCELL package, tPCELL blist)
{
	if (blist) {
		tPCELL identifier, list;
		tOBJECT obj;

		identifier=bind_list_get_name(blist);
		if (cons_create_(vm, &list, &nil, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, list);
		if (vm_push(vm, &obj)) return VM_ERROR;
		cons_set_car(list, &obj);
		if (make_list_of_bind_list(vm, list, package, symbol_is_complete(identifier) ? vm_get_top_package(vm) : package, identifier, 0)) { vm_pop(vm); return VM_ERROR; }
		cons_get_cdr(list, &obj);
		list=OBJECT_IS_CONS(&obj) ? OBJECT_GET_CELL(&obj) : 0;
		BIND_LIST_SET_LIST(blist, list);
		vm_pop(vm);
	}
	return VM_OK;
}

static VM_RET make_list_of_bind_list(tPVM vm, tPCELL list, const tPCELL current, tPCELL package, tPCELL identifier, const tINT i)
{
	tPCELL name, bind, use;
	tOBJECT obj;
	tINT j, n;

	if (!symbol_get_string(identifier, i, &name)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	n=package_get_length_of_name_precedence_list(package);
	if (i+1==symbol_get_length(identifier)) {
		// 最後の記号
		for (j=0; j<=n; j++) {
			use=package_get_use_package(vm, package, j);
			if (!use) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
			if (package_add_bind(vm, use, name, &bind)) return VM_ERROR;
			OBJECT_SET_BIND(&obj, bind);
			if (tail_list_add_object(vm, list, &obj)) return VM_ERROR;
		}
	} else {
		// パッケージ修飾子
//		for (j=0; j<=n; j++) {
//			use=package_get_use_package(vm, package, j);
//			if (!use) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
//			bind=package_get_bind(use, name);
//			if (bind&&((current==package)||(bind_package_is_public(bind)))) {
//				bind_get_package(bind, &obj);
//				if (OBJECT_IS_PACKAGE(&obj)) {
//					if (make_list_of_bind_list(vm, list, current, OBJECT_GET_CELL(&obj), identifier, i+1)) return VM_ERROR;
//				}
//			}
//		}
		bind=package_get_bind(package, name);
		if (bind&&((current==package)||bind_package_is_public(bind))) {
			bind_get_package(bind, &obj);
			if (OBJECT_IS_PACKAGE(&obj)) {
				if (make_list_of_bind_list(vm, list, current, OBJECT_GET_CELL(&obj), identifier, i+1)) return VM_ERROR;
			}
		}
	}
	return VM_OK;
}	

// tail list 
// (tail . ( ... ( obj . (obj . nil)) ... ))
//                       ^tail
static VM_RET tail_list_add_object(tPVM vm, tPCELL list, tPOBJECT obj)
{
	tOBJECT tmp;
	tPCELL p;

	if (cons_create(vm, &p, obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, p);
	cons_set_cdr(cons_get_car_cons(list), &tmp);
	cons_set_car(list, &tmp);

	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_FOREIGN_OBJECT

typedef void (*FOBJ_RELEASE)(TNI* , void*);

#define FOREIGN_OBJECT_SIZE		

#define FOREIGN_OBJECT_GET_OBJECT(obj)			(((obj)+1)->p)
#define FOREIGN_OBJECT_SET_OBJECT(obj, fobj)	(((obj)+1)->p=(fobj))
#define FOREIGN_OBJECT_GET_DESTRUCTOR(obj)		((FOBJ_RELEASE)((obj)+2)->p)
#define FOREIGN_OBJECT_SET_DESTRUCTOR(obj, f)	(((obj)+2)->p=(f))
#define FOREIGN_OBJECT_GET_CLASS(obj)			(((obj)+3)->cell)
#define FOREIGN_OBJECT_SET_CLASS(obj, c)		(((obj)+3)->cell=(c))
#define FOREIGN_OBJECT_GET_SIZE(obj)			(((obj)+4)->ui)
#define FOREIGN_OBJECT_SET_SIZE(obj, size)		(((obj)+4)->ui=(size))

VM_RET foreign_object_create(tPVM vm, void* fobj, void* release, tPCELL my_class, tPCELL* cell)
{
	tUINT size;
	size=allocate_cell(vm, sizeof(tCELL)*5, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_FOREIGN_OBJECT);
	FOREIGN_OBJECT_SET_OBJECT(*cell, fobj);
	FOREIGN_OBJECT_SET_DESTRUCTOR(*cell, release);
	FOREIGN_OBJECT_SET_CLASS(*cell, my_class);
	FOREIGN_OBJECT_SET_SIZE(*cell, size);
	return VM_OK;
}

tUINT foreign_object_get_size(tPCELL fobj)
{
	return FOREIGN_OBJECT_GET_SIZE(fobj);
}

VM_RET foreign_object_destroy(tPVM vm, tPCELL fobj)
{
	if (FOREIGN_OBJECT_GET_DESTRUCTOR(fobj)) {
		(*FOREIGN_OBJECT_GET_DESTRUCTOR(fobj))(vm_get_tni(vm), FOREIGN_OBJECT_GET_OBJECT(fobj));
	}
	return VM_OK;
}

VM_RET foreign_object_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (FOREIGN_OBJECT_GET_CLASS(OBJECT_GET_CELL(obj))) {
		tOBJECT tmp;
		if (write_string(vm, stream, "#i(")) return VM_ERROR;
		OBJECT_SET_SYMBOL(&tmp, foreign_class_get_name(FOREIGN_OBJECT_GET_CLASS(OBJECT_GET_CELL(obj))));
		if (write_object(vm, stream, &tmp)) return VM_ERROR;
		if (write_string(vm, stream, " ")) return VM_ERROR;
		if (write_integer(vm, stream, (tINT)FOREIGN_OBJECT_GET_OBJECT(OBJECT_GET_CELL(obj)), 16)) return VM_ERROR;
		return write_char(vm, stream, ')');
	} else {
		return write_string(vm, stream, "#i(<foreign-object> ")||
			write_integer(vm, stream, (tINT)FOREIGN_OBJECT_GET_OBJECT(OBJECT_GET_CELL(obj)), 16)||
			write_char(vm, stream, ')');
	}
}

VM_RET foreign_object_mark(tPVM vm, tPCELL cell)
{
	return cell_mark(vm, FOREIGN_OBJECT_GET_CLASS(cell));
}

void* foreign_object_get_object(tPCELL fobj)
{
	return FOREIGN_OBJECT_GET_OBJECT(fobj);
}

void foreign_object_get_class(tPCELL fobj, tPOBJECT clss)
{
	if (FOREIGN_OBJECT_GET_CLASS(fobj)) {
		OBJECT_SET_FOREIGN_CLASS(clss, FOREIGN_OBJECT_GET_CLASS(fobj));
	} else {
		OBJECT_SET_BUILT_IN_CLASS(clss, CLASS_FOREIGN_OBJECT);
	}
}

///////////////////////////////////////
// CELL_MACRO

VM_RET macro_expand(tPVM vm, tPCELL macro, tPOBJECT ret)
{
	return function_call(vm, macro, 0, ret);
}

tINT macro_get_parameter_number(tPCELL macro)
{
	return function_get_parameter_number(macro);
}

tBOOL macro_get_rest(tPCELL macro)
{
	return function_is_rest(macro);
}

///////////////////////////////////////
// CELL_ENVIRONMENT

#define ENVIRONMENT_LENGTH		0x000000ff
#define ENVIRONMENT_LENGTH_		0xffffff00

#define ENVIRONMENT_TYPE		0x0000ff00
#define ENVIRONMENT_TYPE_		0xffff00ff

#define ENVIRONMENT_GET_LENGTH(env)			((env)->ui&ENVIRONMENT_LENGTH)
#define ENVIRONMENT_SET_LENGTH(env, len)	((env)->ui|=(len))
#define ENVIRONMENT_GET_ENVIRONMENT(env)	(((env)+1)->cell)
#define ENVIRONMENT_SET_ENVIRONMENT(env, p)	(((env)+1)->cell=(p))
// size==1のとき
#define ENVIRONMENT_GET_TYPE(env)				(((env)->ui&ENVIRONMENT_TYPE)>>8)
#define ENVIRONMENT_SET_TYPE(env, type)			((env)->ui&=ENVIRONMENT_TYPE_, (env)->ui|=((type)<<8)&ENVIRONMENT_TYPE)
#define ENVIRONMENT_GET_DATA(env)				((env)[2])
#define ENVIRONMENT_SET_DATA(env, data)			((env)[2]=(data))
#define ENVIRONMENT_GET_OBJECT(env, obj)		(OBJECT_SET_TYPE(obj, ENVIRONMENT_GET_TYPE(env)), (obj)->data=ENVIRONMENT_GET_DATA(env))
#define ENVIRONMENT_SET_OBJECT(env, obj)		(ENVIRONMENT_SET_TYPE(env, OBJECT_GET_TYPE(obj)), ENVIRONMENT_SET_DATA(env, (obj)->data))
// size>1のとき
#define ENVIRONMENT_GET_SIZE(env)				(((env)+2)->ui)
#define ENVIRONMENT_SET_SIZE(env, size)			(((env)+2)->ui=(size))
#define ENVIRONMENT_GET_HEAD(env)				((tPOBJECT)((env)+3))
//

VM_RET environment_create_(tPVM vm, const tINT n, tPCELL environment, tPCELL* cell)
{
	tUINT size;
	if (n>1) {
		size=allocate_cell(vm, sizeof(tCELL)*3+sizeof(tOBJECT)*n, cell);
	} else {
		size=allocate_cell(vm, sizeof(tCELL)*3, cell);
	}
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_ENVIRONMENT);
	ENVIRONMENT_SET_LENGTH(*cell, n);
	ENVIRONMENT_SET_ENVIRONMENT(*cell, environment);
	if (n>1) {
		tINT i;
		ENVIRONMENT_SET_SIZE(*cell, size);
		for (i=0; i<n; i++) {
			OBJECT_SET_UNBOUND(ENVIRONMENT_GET_HEAD(*cell)+i);
		}
	} else {
		ENVIRONMENT_SET_OBJECT(*cell, &unbound);
	}
	return VM_OK;
}

tUINT environment_get_size(tPCELL env)
{
	if (ENVIRONMENT_GET_LENGTH(env)>1) {
		// 保持している値の数が2以上
		return ENVIRONMENT_GET_SIZE(env);
	} else {
		// 保持している値の数は1
		return 3;
	}
}

tPCELL environment_get_environment(tPCELL env)
{
	return ENVIRONMENT_GET_ENVIRONMENT(env);
}

VM_RET environment_get_value(tPVM vm, tPCELL env, const tINT offset, tPOBJECT obj)
{
	tINT len=ENVIRONMENT_GET_LENGTH(env);

	if (!env) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (offset<len) {// この環境が保持
		if (len==1) {// 環境が一つの値を保持している場合
			ENVIRONMENT_GET_OBJECT(env, obj);
		} else {// 環境が複数の値を保持している場合
			*obj=ENVIRONMENT_GET_HEAD(env)[offset];
		}
		return VM_OK;
	} else {// ここより大域側で保持
		return environment_get_value(vm, ENVIRONMENT_GET_ENVIRONMENT(env), offset-len, obj);
	}
}

VM_RET environment_set_value(tPVM vm, tPCELL env, const tINT offset, tPOBJECT obj)
{
	tINT len=ENVIRONMENT_GET_LENGTH(env);

	if (!env) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (offset<len) {// この環境が保持
		if (len==1) {// 環境が一つの値を保持している場合
			ENVIRONMENT_SET_OBJECT(env, obj);
		} else {// 環境が複数の値を保持している場合
			ENVIRONMENT_GET_HEAD(env)[offset]=*obj;
		}
		return VM_OK;
	} else {// ここより大域側の環境で保持
		return environment_set_value(vm, ENVIRONMENT_GET_ENVIRONMENT(env), offset-len, obj);
	}
}

VM_RET environment_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tINT i, n;
	tPCELL env=OBJECT_GET_CELL(obj);
	if (write_string(vm, stream, "#env(")) return VM_ERROR;
	n=ENVIRONMENT_GET_LENGTH(env);
	for (i=0; i<n; i++) {
		tOBJECT obj;
		// 表示の順番これでいいのかな？
		// 後でチェック
		if (environment_get_value(vm, env, i, &obj)) return VM_ERROR;
		if (write_object(vm, stream, &obj)) return VM_ERROR;
		if ((i!=n-1)&&write_char(vm, stream, ' ')) return VM_ERROR;
	}
	return write_char(vm, stream, ')');
}

VM_RET environment_mark(tPVM vm, tPCELL cell)
{
	if (cell_mark(vm, ENVIRONMENT_GET_ENVIRONMENT(cell))) return VM_ERROR;
	if (ENVIRONMENT_GET_LENGTH(cell)==1) {
		tOBJECT obj;
		ENVIRONMENT_GET_OBJECT(cell, &obj);
		if (gc_push(vm, &obj)) return VM_ERROR;
	} else {
		tINT i, n=ENVIRONMENT_GET_LENGTH(cell);
		tPOBJECT p=ENVIRONMENT_GET_HEAD(cell);
		for (i=0; i<n; i++) {
			if (object_mark(vm, p++)) return VM_ERROR;
		}
	}
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_LOCAL_FUNCTION

#define LOCAL_FUNCTION_SIZE			CELL_UNIT

#define LFUNCTION_GET_FUNCTION(lfunc)			(((lfunc)+1)->cell)
#define LFUNCTION_SET_FUNCTION(lfunc, func)		(((lfunc)+1)->cell=(func))
#define LFUNCTION_GET_ENVIRONMENT(lfunc)		(((lfunc)+2)->cell)
#define LFUNCTION_SET_ENVIRONMENT(lfunc, env)	(((lfunc)+2)->cell=(env))

VM_RET local_function_create_(tPVM vm, tPCELL function, tPCELL environment, tPCELL* cell)
{
	if (!allocate_cell(vm, sizeof(tCELL)*3, cell)) return VM_ERROR;
	CELL_SET_TYPE(*cell, CELL_LOCAL_FUNCTION);
	LFUNCTION_SET_FUNCTION(*cell, function);
	LFUNCTION_SET_ENVIRONMENT(*cell, environment);
	return VM_OK;
}

tUINT local_function_get_size(tPCELL lfunction)
{
	return LOCAL_FUNCTION_SIZE;
}

tPCELL local_function_get_function(tPCELL lfunction)
{
	return LFUNCTION_GET_FUNCTION(lfunction);
}

tPCELL local_function_get_environment(tPCELL lfunction)
{
	return LFUNCTION_GET_ENVIRONMENT(lfunction);
}

VM_RET local_function_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	if (write_string(vm, stream, "#i(<local-function> ")) return VM_ERROR;
	if (write_integer(vm, stream, (tINT)OBJECT_GET_CELL(obj), 16)) return VM_ERROR;
	return write_string(vm, stream, " )");
}

VM_RET local_function_mark(tPVM vm, tPCELL cell)
{
	return cell_mark(vm, LFUNCTION_GET_FUNCTION(cell))||
		   cell_mark(vm, LFUNCTION_GET_ENVIRONMENT(cell));
}
/////////////////////////////////////////////////
// CELL_STANDARD_CLASS

#define STANDARD_CLASS_ABSTRACT									0x00000001
#define STANDARD_CLASS_ABSTRACT_								0xfffffffe
#define STANDARD_CLASS_INVALID									0x00000002
#define STANDARD_CLASS_INVALID_									0xfffffffd

#define STANDARD_CLASS_GET_ABSTRACT(sclass)						((sclass)->ui&STANDARD_CLASS_ABSTRACT)
#define STANDARD_CLASS_SET_ABSTRACT(sclass)						((sclass)->ui|=STANDARD_CLASS_ABSTRACT)
#define STANDARD_CLASS_RESET_ABSTRACT(sclass)					((sclass)->ui&=STANDARD_CLASS_ABSTRACT_)
#define STANDARD_CLASS_GET_INVALID(sclass)						((sclass)->ui&STANDARD_CLASS_INVALID)
#define STANDARD_CLASS_SET_INVALID(sclass)						((sclass)->ui|=STANDARD_CLASS_INVALID)
#define STANDARD_CLASS_RESET_INVALID(sclass)					((sclass)->ui&=STANDARD_CLASS_INVALID_)
#define STANDARD_CLASS_GET_SIZE(sclass)							(((sclass)+1)->ui)
#define STANDARD_CLASS_SET_SIZE(sclass, size)					(((sclass)+1)->ui=(size))
#define STANDARD_CLASS_GET_SUPER_CLASS_LIST(sclass)				(((sclass)+2)->cell)
#define STANDARD_CLASS_SET_SUPER_CLASS_LIST(sclass, list)		(((sclass)+2)->cell=(list))
#define STANDARD_CLASS_GET_SLOT_NUMBER(sclass)					(((sclass)+3)->i)
#define STANDARD_CLASS_SET_SLOT_NUMBER(sclass, n)				(((sclass)+3)->i=(n))
#define STANDARD_CLASS_GET_NAME(sclass)							(((sclass)+4)->cell)
#define STANDARD_CLASS_SET_NAME(sclass, name)					(((sclass)+4)->cell=(name))
#define STANDARD_CLASS_GET_SLOT(sclass)							((tPCELL*)((sclass)+6))

static VM_RET sclass_make_super_class_list(tPVM vm, tPCELL sc_name_list, tPCELL* sclist);
static VM_RET sclass_count_slot_spec(tPVM vm, tPCELL slot_spec_list, tINT* n);
static VM_RET sclass_initialize_slot_spec(tPVM vm, tPCELL sclass, tPCELL slot_spec_list);
static VM_RET sclass_initialize_slot_spec_(tPVM vm, tPCELL sclass, tPCELL slot_spec_list, tPCELL* slot);

VM_RET standard_class_create_(tPVM vm, tPCELL class_name, tPCELL sc_name_list, tPCELL slot_spec_list, const tBOOL abstractp, tPCELL* cell)
{
	tPCELL sclist;
	tINT n;
	tUINT size;
	tOBJECT tmp;
	// データ初期化用のリストの作成
	//　上位クラスのリスト
	if (sclass_make_super_class_list(vm, sc_name_list, &sclist)) return VM_ERROR;
	if (sclist)
		OBJECT_SET_CONS(&tmp, sclist);
	else
		OBJECT_SET_NIL(&tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// slot-specの数の計算
	n=0;
	if (sclass_count_slot_spec(vm, slot_spec_list, &n)) { vm_pop(vm); return VM_ERROR; }

	size=allocate_cell(vm, sizeof(tPCELL)*(6+n), cell);
	if (!size) { vm_pop(vm); return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED); }
	CELL_SET_TYPE(*cell, CELL_STANDARD_CLASS);
	STANDARD_CLASS_SET_SIZE(*cell, size);
	STANDARD_CLASS_SET_SUPER_CLASS_LIST(*cell, sclist);
	STANDARD_CLASS_SET_SLOT_NUMBER(*cell, n);
	STANDARD_CLASS_SET_NAME(*cell, class_name);
	memset(STANDARD_CLASS_GET_SLOT(*cell), 0, sizeof(tPCELL)*n);
	if (abstractp)
		STANDARD_CLASS_SET_ABSTRACT(*cell);
	else
		STANDARD_CLASS_RESET_ABSTRACT(*cell);
	vm_pop(vm);
	cell_to_object(*cell, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	// slot-specの設定
	if (sclass_initialize_slot_spec(vm, *cell, slot_spec_list)) return VM_ERROR;

	vm_pop(vm);
	return VM_OK;
}

static VM_RET sclass_make_super_class_list(tPVM vm, tPCELL sc_name_list, tPCELL* sclist)
{
	tPCELL p, last;
	tOBJECT tmp;

	*sclist=0;
	last=0;
	for (p=sc_name_list; p; p=cons_get_cdr_cons(p)) {
		tPCELL pp, blist;
		cons_get_car(p, &tmp);
		if (tisl_get_bind_list(vm_get_tisl(vm), vm, vm_get_current_package(vm), OBJECT_GET_CELL(&tmp), &blist)) return VM_ERROR;
		cell_to_object(blist, &tmp);
		if (cons_create_(vm, &pp, &tmp, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&tmp, pp);
		if (last) {
			cons_set_cdr(last, &tmp);
		} else {
			*sclist=pp;
			if (vm_push(vm, &tmp)) return VM_ERROR;
		}
		last=pp;
	}
	if (last) vm_pop(vm);
	return VM_OK;
}

static VM_RET sclass_count_slot_spec(tPVM vm, tPCELL slot_spec_list, tINT* n)
{
	tOBJECT tmp;
	tPCELL p;
	// slot-name のみのものは無視する
	for (p=slot_spec_list; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		// (slot-name slot-opt*)
		if (OBJECT_IS_CONS(&tmp)) {
			tPCELL pp=OBJECT_GET_CELL(&tmp);
			cons_get_cdr(pp, &tmp);
			// slot-nameのほかに何かある場合
			if (!OBJECT_IS_NIL(&tmp)) ++*n;
		}
	}
	return VM_OK;
}

static VM_RET sclass_initialize_slot_spec(tPVM vm, tPCELL sclass, tPCELL slot_spec_list)
{
	tOBJECT tmp;
	tPCELL p, slot;
	tINT i;
	i=0;
	for (p=slot_spec_list; p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		if (OBJECT_IS_CONS(&tmp)) {
			tPCELL pp=OBJECT_GET_CELL(&tmp);
			cons_get_cdr(pp, &tmp);
			if (!OBJECT_IS_NIL(&tmp)) {
				if (sclass_initialize_slot_spec_(vm, sclass, pp, &slot)) return VM_ERROR;
				STANDARD_CLASS_GET_SLOT(sclass)[i]=slot;
				i++;
			}
		}
	}
	return VM_OK;
}

static VM_RET sclass_initialize_slot_spec_(tPVM vm, tPCELL sclass, tPCELL slot_spec_list, tPCELL* slot)
{
	tOBJECT name, tmp;
	tPCELL pp;
	cons_get_car(slot_spec_list, &name);
	if (slot_create_(vm, OBJECT_GET_CELL(&name), slot)) return VM_ERROR;
	cell_to_object(*slot, &tmp);
	if (vm_push(vm, &tmp)) return VM_ERROR;
	for (pp=cons_get_cdr_cons(slot_spec_list); pp; pp=cons_get_cdr_cons(pp)) {
		tOBJECT keyword;
		cons_get_car(pp, &keyword);
		pp=cons_get_cdr_cons(pp);
		cons_get_car(pp, &tmp);
		if (OBJECT_GET_CELL(&keyword)==KEYWORD_READER) {
			if (gfunction_create_reader(vm, OBJECT_GET_CELL(&tmp), OBJECT_GET_CELL(&name), sclass)) goto ERROR;
		} else if (OBJECT_GET_CELL(&keyword)==KEYWORD_WRITER) {
			if (gfunction_create_writer(vm, OBJECT_GET_CELL(&tmp), OBJECT_GET_CELL(&name), sclass)) goto ERROR;
		} else if (OBJECT_GET_CELL(&keyword)==KEYWORD_BOUNDP) {
			if (gfunction_create_boundp(vm, OBJECT_GET_CELL(&tmp), OBJECT_GET_CELL(&name), sclass)) goto ERROR;
		} else if (OBJECT_GET_CELL(&keyword)==KEYWORD_ACCESSOR) {
			if (gfunction_create_reader(vm, OBJECT_GET_CELL(&tmp), OBJECT_GET_CELL(&name), sclass)) goto ERROR;
			if (slot_add_accessor(vm, *slot, OBJECT_GET_CELL(&tmp))) goto ERROR;
		} else if (OBJECT_GET_CELL(&keyword)==KEYWORD_INITARG) {
			if (slot_add_initarg(vm, *slot, OBJECT_GET_CELL(&tmp))) goto ERROR;
		} else if (OBJECT_GET_CELL(&keyword)==KEYWORD_INITFORM) {
			if (slot_set_initform(vm, *slot, &tmp)) goto ERROR;
		} else {
			return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
		}
	}
	vm_pop(vm);
	return VM_OK;
ERROR:
	vm_pop(vm);
	return VM_ERROR;
}

tUINT standard_class_get_size(tPCELL sclass)
{
	return STANDARD_CLASS_GET_SIZE(sclass);
}

VM_RET standard_class_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tOBJECT tmp;
	tPCELL sclass=OBJECT_GET_CELL(obj);
	if (standard_class_is_invalid(sclass)) {
		return write_string(vm, stream, "#invalid-class");
	} else {
		if (write_string(vm, stream, "#i(<standard-class> name ")) return VM_ERROR;
		cell_to_object(STANDARD_CLASS_GET_NAME(sclass), &tmp);
		if (write_object(vm, stream, &tmp)) return VM_ERROR;
		if (write_string(vm, stream, ")")) return VM_ERROR;
		return VM_OK;
	}
}

VM_RET standard_class_mark(tPVM vm, tPCELL cell)
{
	tINT i, n;
	tPCELL *p;
	if (cell_mark(vm, STANDARD_CLASS_GET_NAME(cell))) return VM_ERROR;
	if (cell_mark(vm, STANDARD_CLASS_GET_SUPER_CLASS_LIST(cell))) return VM_ERROR;
	p=STANDARD_CLASS_GET_SLOT(cell);
	n=STANDARD_CLASS_GET_SLOT_NUMBER(cell);
	for (i=0; i<n; i++) {
		if (cell_mark(vm, *p++)) return VM_ERROR;
	}
	return VM_OK;
}

tPCELL standard_class_get_name(tPCELL sclass)
{
	return STANDARD_CLASS_GET_NAME(sclass);
}

tBOOL standard_class_is_invalid(tPCELL sclass)
{
	return STANDARD_CLASS_GET_INVALID(sclass) ? tTRUE : tFALSE;
}

void standard_class_set_invalid(tPCELL sclass)
{
	STANDARD_CLASS_SET_INVALID(sclass);
}

tBOOL standard_class_is_subclass(tPVM vm, tPCELL sclass, tPCELL super)
{
	tPCELL p, bind, blist;
	tOBJECT tmp;
	if (super==sclass_standard_object) return tTRUE;
	for (p=STANDARD_CLASS_GET_SUPER_CLASS_LIST(sclass); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		blist=OBJECT_GET_CELL(&tmp);
		bind=bind_list_get_bind(blist, NAMESPACE_CLASS, vm_get_current_package(vm));
		if (bind) {
			bind_get_class(bind, &tmp);
			if (OBJECT_GET_CELL(&tmp)==super) return tTRUE;
			if (standard_class_is_subclass(vm, OBJECT_GET_CELL(&tmp), super)) return tTRUE;
		}
	}
	return tFALSE;
}

static tBOOL sclass_get_precedence_(tPVM vm, tPCELL sclass, tPCELL clss, tINT* i);

tINT standard_class_get_precedence(tPVM vm, tPCELL sclass, tPOBJECT clss)
{
	if (OBJECT_IS_BUILT_IN_CLASS(clss)) {
		if (OBJECT_GET_INTEGER(clss)==CLASS_OBJECT) return 0;
		else return -1;
	} else if (OBJECT_IS_STANDARD_CLASS(clss)) {
		tINT i;
		i=1;
		if (OBJECT_GET_CELL(clss)==sclass_standard_object) return 1;
		if (sclass_get_precedence_(vm, sclass, OBJECT_GET_CELL(clss), &i)) return i;
		return -1;
	} else {
		return -1;
	}
}

static tBOOL sclass_get_precedence_(tPVM vm, tPCELL sclass, tPCELL clss, tINT* i)
{
	tPCELL p, bind;
	tOBJECT tmp;
	for (p=STANDARD_CLASS_GET_SUPER_CLASS_LIST(sclass); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		bind=bind_list_get_bind(OBJECT_GET_CELL(&tmp), NAMESPACE_CLASS, vm_get_current_package(vm));
		if (bind) {
			bind_get_class(bind, &tmp);
			if (OBJECT_IS_STANDARD_CLASS(&tmp)&&
				sclass_get_precedence_(vm, OBJECT_GET_CELL(&tmp), clss, i)) return tTRUE;
		}
	}
	++*i;
	return (sclass==clss) ? tTRUE : tFALSE;
}

VM_RET sclass_initialize_instance(tPVM vm, tPCELL sclass, tPCELL instance, tPCELL list)
{
	tINT i, n;
	tPCELL p;
	// 自分で定義しているスロットの初期化
	n=STANDARD_CLASS_GET_SLOT_NUMBER(sclass);
	for (i=0; i<n; i++) {
		if (slot_initialize_instance(vm, STANDARD_CLASS_GET_SLOT(sclass)[i], instance, list)) return VM_ERROR;
	}
	// 上位クラスから継承したスロットの初期化
	for (p=STANDARD_CLASS_GET_SUPER_CLASS_LIST(sclass); p; p=cons_get_cdr_cons(p)) {
		tPCELL bind, c;
		tOBJECT tmp;
		cons_get_car(p, &tmp);
		bind=bind_list_get_bind(OBJECT_GET_CELL(&tmp), NAMESPACE_CLASS, vm_get_current_package(vm));
		if (!bind) return signal_condition(vm, TISL_ERROR_INVALID_CLASS);
		bind_get_class(bind, &tmp);
		if (!OBJECT_IS_STANDARD_CLASS(&tmp)) return signal_condition(vm, TISL_ERROR_INVALID_CLASS);
		c=OBJECT_GET_CELL(&tmp);
		if (standard_class_is_invalid(c)) return signal_condition(vm, TISL_ERROR_INVALID_CLASS);
		if (sclass_initialize_instance(vm, c, instance, list)) return VM_ERROR;
	}
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_SLOT

#define SLOT_GET_SIZE(slot)				(((slot)+1)->ui)
#define SLOT_SET_SIZE(slot, size)		(((slot)+1)->ui=(size))
#define SLOT_GET_NAME(slot)				(((slot)+2)->cell)
#define SLOT_SET_NAME(slot, name)		(((slot)+2)->cell=(name))
#define SLOT_GET_INITARG(slot)			(((slot)+3)->cell)
#define SLOT_SET_INITARG(slot, list)	(((slot)+3)->cell=(list))
#define SLOT_GET_INITFORM(slot)			(((slot)+4)->cell)
#define SLOT_SET_INITFORM(slot, func)	(((slot)+4)->cell=(func))
#define SLOT_GET_ACCESSOR(slot)			(((slot)+5)->cell)
#define SLOT_SET_ACCESSOR(slot, list)	(((slot)+5)->cell=(list))

VM_RET slot_create_(tPVM vm, tPCELL name, tPCELL* cell)
{
	tUINT size;
	size=allocate_cell(vm, sizeof(tCELL)*6, cell);
	if (!size) return signal_condition(vm, TISL_ERROR_STORAGE_EXHAUSTED);
	CELL_SET_TYPE(*cell, CELL_SLOT);
	SLOT_SET_SIZE(*cell, size);
	SLOT_SET_NAME(*cell, name);
	SLOT_SET_INITARG(*cell, 0);
	SLOT_SET_ACCESSOR(*cell, 0);
	SLOT_SET_INITFORM(*cell, 0);
	return VM_OK;
}

tUINT slot_get_size(tPCELL slot)
{
	return SLOT_GET_SIZE(slot);
}

VM_RET slot_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return write_string(vm, stream, "#tisl(slot)");
}

VM_RET slot_mark(tPVM vm, tPCELL cell)
{
	if (cell_mark(vm, SLOT_GET_NAME(cell))) return VM_ERROR;
	if (cell_mark(vm, SLOT_GET_INITARG(cell))) return VM_ERROR;
	if (cell_mark(vm, SLOT_GET_INITFORM(cell))) return VM_ERROR;
	if (cell_mark(vm, SLOT_GET_ACCESSOR(cell))) return VM_ERROR;
	return VM_OK;
}

VM_RET slot_add_initarg(tPVM vm, tPCELL slot, tPCELL initarg)
{
	tPCELL p;
	tOBJECT tmp;

	cell_to_object(initarg, &tmp);
	if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;

	if (SLOT_GET_INITARG(slot)) {
		OBJECT_SET_CONS(&tmp, SLOT_GET_INITARG(slot));
		cons_set_cdr(p, &tmp);
	}
	SLOT_SET_INITARG(slot, p);
	return VM_OK;
}

VM_RET slot_add_accessor(tPVM vm, tPCELL slot, tPCELL accessor)
{
	tPCELL p;
	tOBJECT tmp;

	cell_to_object(accessor, &tmp);
	if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;

	if (SLOT_GET_ACCESSOR(slot)) {
		OBJECT_SET_CONS(&tmp, SLOT_GET_ACCESSOR(slot));
		cons_set_cdr(p, &tmp);
	}
	SLOT_SET_ACCESSOR(slot, p);
	return VM_OK;
}

VM_RET slot_set_initform(tPVM vm, tPCELL slot, tPOBJECT form)
{
	tPCELL func;

	if (SLOT_GET_INITFORM(slot)) return signal_condition(vm, TISL_ERROR_INITFORMS);
	if (translate(vm, form, &func)) return VM_ERROR;
	SLOT_SET_INITFORM(slot, func);
	return VM_OK;
}

VM_RET slot_initialize_instance(tPVM vm, tPCELL slot, tPCELL instance, tPCELL list)
{
	// スロットが既に値を持っている場合は，その値は変更しない
	if (instance_has_slot(instance, SLOT_GET_NAME(slot))) return VM_OK;
	// スロットに対する初期化引数と初期値の組がある場合はその値で初期化する
	if (SLOT_GET_INITARG(slot)) {
		tPCELL p, pp, initarg;
		tOBJECT tmp;
		for (p=SLOT_GET_INITARG(slot); p; p=cons_get_cdr_cons(p)) {
			cons_get_car(p, &tmp);
			initarg=OBJECT_GET_CELL(&tmp);
			for (pp=list; pp; pp=cons_get_cdr_cons(pp)) {
				cons_get_car(pp, &tmp);
				pp=cons_get_cdr_cons(pp);
				if (OBJECT_GET_CELL(&tmp)==initarg) {
					cons_get_car(pp, &tmp);
					return instance_add_slot(vm, instance, SLOT_GET_NAME(slot), &tmp);
				}
			}
		}
	}
	// スロットが省略時初期値形式をもっている場合は，その評価結果
	if (SLOT_GET_INITFORM(slot)) {
		tOBJECT value;
		if (function_call(vm, SLOT_GET_INITFORM(slot), 0, &value)) return VM_ERROR;
		if (instance_add_slot(vm, instance, SLOT_GET_NAME(slot), &value)) return VM_ERROR;
		return VM_OK;
	}
	// スロットは初期化されない
	return VM_OK;
}

/////////////////////////////////////////////////
// CELL_INSTANCE

#define INSTANCE_GET_CLASS(instance)			(((instance)+1)->cell)
#define INSTANCE_SET_CLASS(instance, c)			(((instance)+1)->cell=(c))
#define INSTANCE_GET_SLOT_LIST(instance)		(((instance)+2)->cell)
#define INSTANCE_SET_SLOT_LIST(instance, list)	(((instance)+2)->cell=(list))

// 効率わるすぎ　後でハッシュテーブルにでもすること/*!!!*/

VM_RET instance_create_(tPVM vm, tPCELL sclass, tPCELL* cell)
{
	if (STANDARD_CLASS_GET_ABSTRACT(sclass)) return signal_condition(vm, TISL_ERROR_ABSTRACT_CLASS);
	if (!allocate_cell(vm, sizeof(tCELL)*3, cell)) return VM_ERROR;
	CELL_SET_TYPE(*cell, CELL_INSTANCE);
	INSTANCE_SET_CLASS(*cell, sclass);
	INSTANCE_SET_SLOT_LIST(*cell, 0);
	return VM_OK;
}

VM_RET instance_initialize(tPVM vm, tPCELL instance, tPCELL list)
{
	return sclass_initialize_instance(vm, INSTANCE_GET_CLASS(instance), instance, list);
}

tUINT instance_get_size(tPCELL instance)
{
	return 3;
}

VM_RET instance_write(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tPCELL instance=OBJECT_GET_CELL(obj);
	tOBJECT tmp;
	if (write_string(vm, stream, "#i(")) return VM_ERROR;
	cell_to_object(INSTANCE_GET_CLASS(instance), &tmp);
	if (write_object(vm, stream, &tmp)) return VM_ERROR;
	if (INSTANCE_GET_SLOT_LIST(instance)) {
		if (write_string(vm, stream, " ")) return VM_ERROR;
		cell_to_object(INSTANCE_GET_SLOT_LIST(instance), &tmp);
		if (write_object(vm, stream, &tmp)) return VM_ERROR;
	}
	if (write_string(vm, stream, ")")) return VM_ERROR;
	return VM_OK;
}

VM_RET instance_mark(tPVM vm, tPCELL cell)
{
	if (cell_mark(vm, INSTANCE_GET_CLASS(cell))) return VM_ERROR;
	if (cell_mark(vm, INSTANCE_GET_SLOT_LIST(cell))) return VM_ERROR;
	return VM_OK;
}

tPCELL instance_get_class(tPCELL instance)
{
	return INSTANCE_GET_CLASS(instance);
}

VM_RET instance_get_slot(tPVM vm, tPCELL instance, tPCELL slot_name, tPCELL sclass, tPOBJECT obj)
{
	tPCELL p;
	tOBJECT tmp;
	for (p=INSTANCE_GET_SLOT_LIST(instance); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		p=cons_get_cdr_cons(p);
		if (OBJECT_GET_CELL(&tmp)==slot_name) {
			cons_get_car(p, obj);
			return VM_OK;
		}
	}
	return signal_condition(vm, TISL_ERROR_SLOT_UNBOUND);
}

VM_RET instance_set_slot(tPVM vm, tPCELL instance, tPCELL slot_name, tPCELL sclass, tPOBJECT obj)
{
	tPCELL p;
	tOBJECT tmp;
	for (p=INSTANCE_GET_SLOT_LIST(instance); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		p=cons_get_cdr_cons(p);
		if (OBJECT_GET_CELL(&tmp)==slot_name) {
			cons_set_car(p, obj);
			return VM_OK;
		}
	}
	return instance_add_slot(vm, instance, slot_name, obj);
}

VM_RET instance_add_slot(tPVM vm, tPCELL instance, tPCELL name, tPOBJECT value)
{
	tPCELL p;
	tOBJECT tmp, tmp2;
	if (INSTANCE_GET_SLOT_LIST(instance)) {
		OBJECT_SET_CONS(&tmp2, INSTANCE_GET_SLOT_LIST(instance));
	} else {
		OBJECT_SET_NIL(&tmp2);
	}
	if (cons_create_(vm, &p, value, &tmp2)) return VM_ERROR;
	OBJECT_SET_CONS(&tmp, p);
	cell_to_object(name, &tmp2);
	if (cons_create(vm, &p, &tmp2, &tmp)) return VM_ERROR;
	INSTANCE_SET_SLOT_LIST(instance, p);
	return VM_OK;
}

VM_RET instance_check_slot(tPVM vm, tPCELL instance, tPCELL slot_name, tPCELL sclass, tPOBJECT ret)
{
	if (instance_has_slot(instance, slot_name)) {
		OBJECT_SET_SYMBOL(ret, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(ret);
	}
	return VM_OK;
}

tBOOL instance_has_slot(tPCELL instance, tPCELL name)
{
	tPCELL p;
	tOBJECT tmp;
	for (p=INSTANCE_GET_SLOT_LIST(instance); p; p=cons_get_cdr_cons(p)) {
		cons_get_car(p, &tmp);
		p=cons_get_cdr_cons(p);
		if (OBJECT_GET_CELL(&tmp)==name) {
			return tTRUE;
		}
	}
	return tFALSE;
}
