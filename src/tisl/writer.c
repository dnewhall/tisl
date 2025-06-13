//
// TISL/src/tisl/writer.c
// TISL Ver 4.x
// 

#define TISL_WRITER_C

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "writer.h"
#include "built_in_object.h"

/////////////////////////////

const tCHAR integer_char[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static VM_RET writer_lock_stream(tPCELL stream, tPVM vm);
static void writer_unlock_stream(tPCELL stream);

VM_RET write_char(tPVM vm, tPCELL stream, const tCHAR c);
VM_RET write_string(tPVM vm, tPCELL stream, tCSTRING string);
VM_RET write_float(tPVM vm, tPCELL stream, const tFLOAT f);
static VM_RET write_fresh_line(tPVM vm, tPCELL stream);
VM_RET write_integer(tPVM vm, tPCELL stream, const tINT i, const tINT r);
static VM_RET write_integer_(tPVM vm, tPCELL stream, const tINT i, const tINT t, const tBOOL sign);
VM_RET write_object(tPVM vm, tPCELL stream, tPOBJECT obj);
static VM_RET write_tab(tPVM vm, tPCELL stream, const tINT n);

/////////////////////////////

VM_RET format_char(tPVM vm, tPCELL stream, const tCHAR c)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_char(vm, stream, c);
	writer_unlock_stream(stream);
	return ret;
}

VM_RET format_float(tPVM vm, tPCELL stream, const tFLOAT f)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_float(vm, stream, f);
	writer_unlock_stream(stream);
	return ret;
}

VM_RET format_fresh_line(tPVM vm, tPCELL stream)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_fresh_line(vm, stream);
	writer_unlock_stream(stream);
	return ret;
}

// formatは破棄

VM_RET format_l(tPVM vm, tPCELL stream, tCSTRING string, tPCELL list)
{
	tINT x, len, r;
	tOBJECT obj;

	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	len=strlen(string);
	r=0;
	//
	for (x=0; x<len; x++) {
		if (is_DBCS_lead_byte(vm, string[x])) {
			if (write_char(vm, stream, string[x])) goto ERROR;
			x++;
			if (write_char(vm, stream, string[x])) goto ERROR;
		} else if (string[x]=='~') {
			x++;
			switch (string[x]) {
			case 'A'://ISLISPオブジェクト
			case 'a':
			case 'S':// エスケープ付き
			case 's':
				if ((string[x]=='S')||(string[x]=='s')) vm_set_writer_flag(vm);
				if (list) {
					cons_get_car(list, &obj);
					list=cons_get_cdr_cons(list);
				} else {
					writer_unlock_stream(stream);
					vm_reset_writer_flag(vm);
					return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
				}
				if (write_object(vm, stream, &obj)) { vm_reset_writer_flag(vm); goto ERROR; }
				vm_reset_writer_flag(vm);
				break;
				// 整数
			case 'c':
			case 'C':
				if (list) {
					cons_get_car(list, &obj);
					list=cons_get_cdr_cons(list);
				} else {
					writer_unlock_stream(stream);
					return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
				}
				if (!OBJECT_IS_CHARACTER(&obj)) { writer_unlock_stream(stream); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &obj); }
				if (write_char(vm, stream, OBJECT_GET_CHARACTER(&obj))) goto ERROR;
				break;
			case '~':
				if (write_char(vm, stream, '~')) goto ERROR;
				break;
			case 'B':
			case 'b'://　2進数
				r=2;
			case 'O':
			case 'o': // 8進数
				if (!r) r=8;
			case 'D':
			case 'd': // 10進数
				if (!r) r=10;
			case 'X':
			case 'x': // 16進数
				if (!r) r=16;
				if (list) {
					cons_get_car(list, &obj);
					list=cons_get_cdr_cons(list);
				} else {
					writer_unlock_stream(stream);
					return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
				}
				if (!OBJECT_IS_INTEGER(&obj)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &obj); goto ERROR; }
				if (write_integer(vm, stream, OBJECT_GET_INTEGER(&obj), r)) goto ERROR;
				break;
				// 不動小数点数
			case 'G':
			case 'g':
				if (list) {
					cons_get_car(list, &obj);
					list=cons_get_cdr_cons(list);
				} else {
					writer_unlock_stream(stream);
					return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
				}
				if (!OBJECT_IS_FLOAT(&obj)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FLOAT, &obj); goto ERROR; }
				if (write_float(vm, stream, OBJECT_GET_FLOAT(&obj))) goto ERROR;
				break;
			case '%':// 改行
				if (write_char(vm, stream, '\n')) goto ERROR;
				break;
			case '&':// 条件付改行
				if (write_fresh_line(vm, stream)) goto ERROR;
				break;
			case '0':// タブかn進数整数
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					tINT m=0;
					while (isdigit(string[x])) {
						m*=10;
						m+=string[x]-'0';
						x++;
					}
					if ((string[x]=='R')||(string[x]=='r')) {
						// R進数
						if (list) {
							cons_get_car(list, &obj);
							list=cons_get_cdr_cons(list);
						} else {
							writer_unlock_stream(stream);
							return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
						}
						if (!OBJECT_IS_INTEGER(&obj)) { signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &obj); goto ERROR; }
						if (write_integer(vm, stream, OBJECT_GET_INTEGER(&obj), m)) goto ERROR;
					} else if ((string[x]=='T')||(string[x]=='t')) {
						// タブ
						if (write_tab(vm, stream, m)) goto ERROR;
					} else {
						signal_condition(vm, TISL_ERROR_UNKNOWN_FORMAT_CONTROL);
						goto ERROR;
					}
				}
				break;
			default:
				signal_condition(vm, TISL_ERROR_UNKNOWN_FORMAT_CONTROL);
				goto ERROR;
			}
		} else {
			if (write_char(vm, stream, string[x])) goto ERROR;
		}
	}
	writer_unlock_stream(stream);
	return VM_OK;
	//
ERROR:
	writer_unlock_stream(stream);
	return VM_ERROR;
}

VM_RET format_integer(tPVM vm, tPCELL stream, const tINT i, const tINT r)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_integer(vm, stream, i, r);
	writer_unlock_stream(stream);
	return ret;
}

VM_RET format_object(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_object(vm, stream, obj);
	writer_unlock_stream(stream);
	return ret;
}

VM_RET format_tab(tPVM vm, tPCELL stream, const tINT n)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_tab(vm, stream, n);
	writer_unlock_stream(stream);
	return ret;
}

/////////////////////////////

static VM_RET writer_lock_stream(tPCELL stream, tPVM vm)
{
	if (CELL_IS_STRING_STREAM(stream)&&string_stream_is_output(stream)) {
		return string_stream_lock(stream, vm);
	} else if (CELL_IS_FILE_STREAM(stream)&&file_stream_is_output(stream)) {
		return file_stream_lock(stream, vm);
	} else {
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	}
}

static void writer_unlock_stream(tPCELL stream)
{
	if (CELL_IS_STRING_STREAM(stream)) {
		string_stream_unlock(stream);
	} else {
		file_stream_unlock(stream);
	}
}

VM_RET write_char(tPVM vm, tPCELL stream, const tCHAR c)
{
	if (CELL_IS_STRING_STREAM(stream)) {
		return string_stream_write_char(vm, stream, c);
	} else {
		return file_stream_write_char(vm, stream, c);
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

VM_RET write_string(tPVM vm, tPCELL stream, tCSTRING string)
{
	int i, n;
	n=strlen(string);
	for (i=0; i<n; i++) {
		if (is_DBCS_lead_byte(vm, string[i])) {
			if (write_char(vm, stream, string[i])) return VM_ERROR;
			i++;
		}
		if (write_char(vm, stream, string[i])) return VM_ERROR;
	}
	return VM_OK;
}

VM_RET write_float(tPVM vm, tPCELL stream, const tFLOAT f)
{
	tCHAR buffer[256];
	int i, x;
	x=sprintf((char*)buffer, "%g", f);
	if (x>255) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	for (i=0; i<x; i++) {
		char c=buffer[i];
		if ((c=='.')||(c=='e')||(c=='E')) goto OUT;
	}
	buffer[x++]='.';
	buffer[x++]='0';
OUT:
	buffer[x]='\0';
	return write_string(vm, stream, buffer);
}

static VM_RET write_fresh_line(tPVM vm, tPCELL stream)
{
	if ((stream_get_x(stream)!=0)&&
		write_char(vm, stream, '\n')) return VM_ERROR;
	return VM_OK;
}

VM_RET write_integer(tPVM vm, tPCELL stream, const tINT i, const tINT r)
{
	tBOOL sign;
	tINT ii;
	if (i<0) {
		sign=tTRUE;
		ii=-i;
	} else {
		sign=tFALSE;
		ii=i;
	}
	if ((r<2)||(r>36)) {
		tOBJECT obj;
		OBJECT_SET_INTEGER(&obj, r);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &obj);
	}
	return write_integer_(vm, stream, ii, r, sign);
}

static VM_RET write_integer_(tPVM vm, tPCELL stream, const tINT i, const tINT r, const tBOOL sign)
{
	if (i<r) {
		if (sign&&write_char(vm, stream, '-')) return VM_ERROR;
	} else {
		if (write_integer_(vm, stream, i/r, r, sign)) return VM_ERROR;
	}
	return write_char(vm, stream, integer_char[i%r]);
}

VM_RET write_object(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	return (*object_write_table[OBJECT_GET_TYPE(obj)])(vm, stream, obj);
}

static VM_RET write_tab(tPVM vm, tPCELL stream, const tINT n)
{
	tINT i;
	i=stream_get_x(stream);
	if (i>=0) {
		for (; i<n; i++) {
			if (write_char(vm, stream, ' ')) return VM_ERROR;
		}
	} else {
		if (write_char(vm, stream, ' ')) return VM_ERROR;
	}
	return VM_OK;
}

VM_RET format_elapsed_time(tPVM vm, tPCELL stream, const tFLOAT f)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=write_fresh_line(vm, stream)||
		write_string(vm, stream, "; elapsed time : ")||
		write_float(vm, stream, f)||
		write_fresh_line(vm, stream);
	writer_unlock_stream(stream);
	return ret;
}

VM_RET format_current_package(tPVM vm, tPCELL stream)
{
	VM_RET ret;
	if (writer_lock_stream(stream, vm)) return VM_ERROR;
	ret=package_write_(vm, stream, vm_get_current_package(vm));
	writer_unlock_stream(stream);
	return ret;
}

