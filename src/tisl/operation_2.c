//
// TISL/src/tisl/operation_2.c
// TISL Ver. 4.x
//

#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <string.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>

#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "operation.h"
#include "built_in_object.h"
#include "translator.h"
#include "reader.h"
#include "writer.h"


VM_RET OPERATION_CALL op_floatp(tPVM vm)
{
	if (OBJECT_IS_FLOAT(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_float(tPVM vm)
{
	if (OBJECT_IS_INTEGER(vm->SP))
		OBJECT_SET_FLOAT(vm->SP, (tFLOAT)OBJECT_GET_INTEGER(vm->SP));
	else if (!OBJECT_IS_FLOAT(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_floor(tPVM vm)
{
	if (OBJECT_IS_FLOAT(vm->SP))
		OBJECT_SET_INTEGER(vm->SP, (tINT)floor(OBJECT_GET_FLOAT(vm->SP)));
	else if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_ceiling(tPVM vm)
{
	if (OBJECT_IS_FLOAT(vm->SP))
		OBJECT_SET_INTEGER(vm->SP, (tINT)ceil(OBJECT_GET_FLOAT(vm->SP)));
	else if (OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_truncate(tPVM vm)
{
	if (OBJECT_IS_FLOAT(vm->SP)) {
		if (OBJECT_GET_FLOAT(vm->SP)<0)
			OBJECT_SET_INTEGER(vm->SP, (tINT)ceil(OBJECT_GET_FLOAT(vm->SP)));
		else
			OBJECT_SET_INTEGER(vm->SP, (tINT)floor(OBJECT_GET_FLOAT(vm->SP)));
	} else if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_round(tPVM vm)
{
	if (OBJECT_IS_FLOAT(vm->SP)) {
		tINT i;
		tFLOAT f;
		if (OBJECT_GET_FLOAT(vm->SP)<0) {
			i=(tINT)ceil(OBJECT_GET_FLOAT(vm->SP));
			f=i-OBJECT_GET_FLOAT(vm->SP);
			if ((f>0.5)||((f==0.5)&&(i%2))) i--;
		} else {
			i=(tINT)floor(OBJECT_GET_FLOAT(vm->SP));
			f=OBJECT_GET_FLOAT(vm->SP)-i;
			if ((f>0.5)||((f==0.5)&&(i%2))) i++;
		}
		OBJECT_SET_INTEGER(vm->SP, i);
	} else if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_integerp(tPVM vm)
{
	if (OBJECT_IS_INTEGER(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_div(tPVM vm)
{
	double f;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	f=OBJECT_GET_INTEGER(vm->SP--);
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (f==0) return signal_condition(vm, TISL_ERROR_DIVISION_BY_ZERO);
	f=OBJECT_GET_INTEGER(vm->SP)/f;
	OBJECT_SET_INTEGER(vm->SP, (tINT)floor(f));
	return VM_OK;
}

VM_RET OPERATION_CALL op_mod(tPVM vm)
{
	double f1, f2;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	if (OBJECT_GET_INTEGER(vm->SP)==0) return signal_condition(vm, TISL_ERROR_DIVISION_BY_ZERO);
	f2=OBJECT_GET_INTEGER(vm->SP--);
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
	}
	f1=OBJECT_GET_INTEGER(vm->SP)/f2;
	OBJECT_SET_INTEGER(vm->SP, OBJECT_GET_INTEGER(vm->SP)-(tINT)(floor(f1)*f2));
	return VM_OK;
}

static tINT po_gcd_(tINT i1, tINT i2)
{
	if (!i1) return i2;
	if (!i2) return i1;
	while (i1!=i2) {
		if (i1>i2) i1-=i2;
		else i2-=i1;
	}
	return i1;
}

VM_RET OPERATION_CALL op_gcd(tPVM vm)
{
	tINT i1, i2;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	i1=OBJECT_GET_INTEGER(vm->SP--);
	if (i1<0) i1=-i1;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	i2=OBJECT_GET_INTEGER(vm->SP);
	if (i2<0) i2=-i2;
	OBJECT_SET_INTEGER(vm->SP, po_gcd_(i1, i2));
	return VM_OK;
}

VM_RET OPERATION_CALL op_lcm(tPVM vm)
{
	tINT i1, i2, i3;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	i1=OBJECT_GET_INTEGER(vm->SP--);
	if (i1<0) i1=-i1;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	i2=OBJECT_GET_INTEGER(vm->SP);
	if (i2<0) i2=-i2;
	i3=po_gcd_(i1, i2);
	if (i3)
		OBJECT_SET_INTEGER(vm->SP, i1/i3*i2);
	else
		OBJECT_SET_INTEGER(vm->SP, 0);
	return VM_OK;
}

VM_RET OPERATION_CALL op_isqrt(tPVM vm)
{
	if (!OBJECT_IS_INTEGER(vm->SP)||
		(OBJECT_GET_INTEGER(vm->SP)<0)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	OBJECT_SET_INTEGER(vm->SP, (tINT)sqrt(OBJECT_GET_INTEGER(vm->SP)));
	return VM_OK;
}

VM_RET OPERATION_CALL op_characterp(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_char_equal(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP)) {
		vm->SP--;
		if (OBJECT_IS_CHARACTER(vm->SP)) {
			if (OBJECT_GET_CHARACTER(vm->SP) == OBJECT_GET_CHARACTER(vm->SP+1))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			return VM_OK;
		}
	}
	{
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
}

VM_RET OPERATION_CALL op_char_not_equal(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP)) {
		vm->SP--;
		if (OBJECT_IS_CHARACTER(vm->SP)) {
			if (OBJECT_GET_CHARACTER(vm->SP) != OBJECT_GET_CHARACTER(vm->SP+1))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			return VM_OK;
		}
	}
	{
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
}

VM_RET OPERATION_CALL op_char_less(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP)) {
		vm->SP--;
		if (OBJECT_IS_CHARACTER(vm->SP)) {
			if (OBJECT_GET_CHARACTER(vm->SP) < OBJECT_GET_CHARACTER(vm->SP+1))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			return VM_OK;
		}
	}
	{
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
}

VM_RET OPERATION_CALL op_char_greater(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP)) {
		vm->SP--;
		if (OBJECT_IS_CHARACTER(vm->SP)) {
			if (OBJECT_GET_CHARACTER(vm->SP) > OBJECT_GET_CHARACTER(vm->SP+1))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			return VM_OK;
		}
	}
	{
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
}

VM_RET OPERATION_CALL op_char_le(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP)) {
		vm->SP--;
		if (OBJECT_IS_CHARACTER(vm->SP)) {
			if (OBJECT_GET_CHARACTER(vm->SP) <= OBJECT_GET_CHARACTER(vm->SP+1))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			return VM_OK;
		}
	}
	{
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
}

VM_RET OPERATION_CALL op_char_ge(tPVM vm)
{
	if (OBJECT_IS_CHARACTER(vm->SP)) {
		vm->SP--;
		if (OBJECT_IS_CHARACTER(vm->SP)) {
			if (OBJECT_GET_CHARACTER(vm->SP) >= OBJECT_GET_CHARACTER(vm->SP+1))
				OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
			else
				OBJECT_SET_NIL(vm->SP);
			return VM_OK;
		}
	}
	{
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
}

VM_RET OPERATION_CALL op_consp(tPVM vm)
{
	if (OBJECT_IS_CONS(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_cons(tPVM vm)
{
	tPCELL p;
	tOBJECT car, cdr;
	car=*(vm->SP-1);
	cdr=*(vm->SP);
	if (cons_create_(vm, &p, &car, &cdr)) return VM_ERROR;
	vm->SP--;
	OBJECT_SET_CONS(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_car(tPVM vm)
{
	if (!OBJECT_IS_CONS(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CONS, &tmp);
	}
	cons_get_car(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_cdr(tPVM vm)
{
	if (!OBJECT_IS_CONS(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CONS, &tmp);
	}
	cons_get_cdr(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_null(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_listp(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)||OBJECT_IS_CONS(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_create_list(tINT anum, tPVM vm)
{
	// (create-list i [intial-element])
	if (anum==1) {
		if (vm_push(vm, &nil)) return VM_ERROR;
	} else if (anum!=2) 
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (!OBJECT_IS_INTEGER(vm->SP-1)||
		(OBJECT_GET_INTEGER(vm->SP-1)<0)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	{
		tPCELL p;
		tOBJECT car, cdr;
		tINT i, n=OBJECT_GET_INTEGER(vm->SP-1);
		*(vm->SP-1)=nil;
		for (i=0; i<n; i++) {
			car=*(vm->SP);
			cdr=*(vm->SP-1);
			if (cons_create_(vm, &p, &car, &cdr)) return VM_ERROR;
			OBJECT_SET_CONS(vm->SP-1, p);
		}
		vm->SP--;
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_list(tINT anum, tPVM vm)
{
	tINT i;
	tPCELL p;
	tOBJECT car, cdr;
	if (vm_push(vm, &nil)) return VM_ERROR;
	for (i=0; i<anum; i++) {
		car=*(vm->SP-1);
		cdr=*(vm->SP);
		if (cons_create_(vm, &p, &car, &cdr)) return VM_ERROR;
		vm->SP--;
		OBJECT_SET_CONS(vm->SP, p);
	}

	return VM_OK;
}

VM_RET OPERATION_CALL op_reverse(tPVM vm)
{
	tPCELL p, pp;
	tOBJECT obj, tmp;
	if (!OBJECT_IS_NIL(vm->SP)) {
		if (!OBJECT_IS_CONS(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
		p=OBJECT_GET_CELL(vm->SP);
		if (vm_push(vm, &nil)) return VM_ERROR;
		while (p) {
			cons_get_car(p, &obj);
			tmp=*vm->SP;
			if (cons_create_(vm, &pp, &obj, &tmp)) return VM_ERROR;
			OBJECT_SET_CONS(vm->SP, pp);
			// 次
			cons_get_cdr(p, &obj);
			if (!OBJECT_IS_NIL(&obj)) {
				if (!OBJECT_IS_CONS(&obj)) {
					tOBJECT tmp=*(vm->SP-1);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
				}
				p=cons_get_cdr_cons(p);
			} else {
				p=0;
			}
		}
		vm_pop(vm);
		*vm->SP=*(vm->SP+1);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_nreverse(tPVM vm)
{
	return op_reverse(vm);
}

VM_RET OPERATION_CALL op_append(tINT anum, tPVM vm)
{
	if (anum) {
		tOBJECT obj, obj2;
		tPCELL p, pp, p3;
		tINT i;

		if (cons_create_(vm, &p, &nil, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, p);
		cons_set_car(p, &obj);
		if (vm_push(vm, &obj)) return VM_ERROR;
		for (i=anum-1; i>0; i--) {
			if (!OBJECT_IS_NIL(vm->SP-i-1)&&!OBJECT_IS_CONS(vm->SP-i-1)) {
				tOBJECT tmp=*(vm->SP-i-1);
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
			}
			else if (OBJECT_IS_CONS(vm->SP-i-1)) {
				pp=OBJECT_GET_CELL(vm->SP-i-1);
				while (pp) {
					cons_get_car(pp, &obj2);
					if (cons_create(vm, &p3, &obj2, &nil)) { vm_pop(vm); return VM_ERROR; }
					OBJECT_SET_CONS(&obj2, p3);
					cons_set_cdr(cons_get_car_cons(p), &obj2);
					cons_set_car(p, &obj2);
					cons_get_cdr(pp, &obj2);
					if (OBJECT_IS_NIL(&obj2)) pp=0;
					else if (OBJECT_IS_CONS(&obj2)) pp=cons_get_cdr_cons(pp);
					else { vm_pop(vm); return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj2); }
				}
			}
		}
		vm_pop(vm);
		// 最後の要素
		if (OBJECT_IS_CONS(vm->SP)) {
			cons_set_cdr(cons_get_car_cons(p), vm->SP);
		} else if (!OBJECT_IS_NIL(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
		vm->SP-=anum-1;
		cons_get_cdr(p, vm->SP);
	} else {
		vm->SP++;
		*vm->SP=nil;
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_member(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)) {
		*--(vm->SP)=nil;
	} else if (OBJECT_IS_CONS(vm->SP)) {
		tPCELL p;
		tOBJECT obj;
		p=OBJECT_GET_CELL(vm->SP);
		while (p) {
			cons_get_car(p, &obj);
			if (object_eql(vm->SP-1, &obj)) {
				vm->SP--;
				OBJECT_SET_CONS(vm->SP, p);
				p=0;
			} else {
				cons_get_cdr(p, &obj);
				if (OBJECT_IS_CONS(&obj)) {
					p=cons_get_cdr_cons(p);
				} else {
					*--(vm->SP)=nil;
					p=0;
				}
			}
		}
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_mapcar(tINT anum, tPVM vm)
{
	tINT i, c, m=anum-1, sp=vm->SP-vm->stack;
	tOBJECT obj;
	tPCELL p;

	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	c=0;
	while (1) {
		if (vm_push(vm, vm->stack+sp-m)) return VM_ERROR;
		for (i=m-1; i>=0; i--) {
			if (OBJECT_IS_CONS(vm->stack+sp-i)) {
				p=OBJECT_GET_CELL(vm->stack+sp-i);
				cons_get_car(p, &obj);
				if (vm_push(vm, &obj)) return VM_ERROR;
				cons_get_cdr(p, &obj);
				*(vm->stack+sp-i)=obj;
			} else {
				obj=*(vm->stack+sp-i);
				vm->SP-=m-i;
				if (c==0) {
					if (!OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, CLASS_DOMAIN_ERROR, CLASS_LIST, &obj);
					vm->SP-=m;
					OBJECT_SET_NIL(vm->SP);
				} else {
					tOBJECT tmp, tmp2;
					tmp=*vm->SP;
					if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;
					OBJECT_SET_CONS(vm->SP, p);
					for (i=1; i<c; i++) {
						tmp=*(vm->SP-1);
						tmp2=*(vm->SP);
						if (cons_create_(vm, &p, &tmp, &tmp2)) return VM_ERROR;
						vm->SP--;
						OBJECT_SET_CONS(vm->SP, p);
					}
					vm->SP-=anum;
					*vm->SP=*(vm->SP+anum);
				}
				return VM_OK;
			}
		}
		c++;
		if (op_funcall(anum, vm)) return VM_ERROR;
	}
}

VM_RET OPERATION_CALL op_mapc(tINT anum, tPVM vm)
{
	tINT i, c, n=anum-1, sp=vm->SP-vm->stack;
	tOBJECT obj, ret;
	tPCELL p;

	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	ret=*(vm->SP-n+1);
	if (vm_push(vm, &ret)) return VM_ERROR;
	c=0;
	while (1) {
		if (vm_push(vm, vm->stack+sp-n)) return VM_ERROR;
		for (i=n-1; i>=0; i--) {
			if (OBJECT_IS_CONS(vm->stack+sp-i)) {
				p=OBJECT_GET_CELL(vm->stack+sp-i);
				cons_get_car(p, &obj);
				if (vm_push(vm, &obj)) return VM_ERROR;
				cons_get_cdr(p, &obj);
				*(vm->stack+sp-i)=obj;
			} else {
				obj=*(vm->stack+sp-i);
				vm_pop(vm);
				if (c==0) {
					if (!OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
					vm->SP=vm->stack+sp-n;
					OBJECT_SET_NIL(vm->SP);
				} else {
					vm->SP-=n*2-i;
					*vm->SP=ret;
				}
				return VM_OK;
			}
		}
		c++;
		if (op_funcall(anum, vm)) return VM_ERROR;
		vm->SP--;
	}
}

VM_RET OPERATION_CALL op_maplist(tINT anum, tPVM vm)
{
	tINT i, c, n=anum-1, sp=vm->SP-vm->stack;
	tOBJECT obj;
	tPCELL p;

	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	c=0;
	while (1) {
		if (vm_push(vm, vm->stack+sp-n)) return VM_ERROR;
		for (i=n-1; i>=0; i--) {
			if (OBJECT_IS_CONS(vm->stack+sp-i)) {
				if (vm_push(vm, vm->stack+sp-i)) return VM_ERROR;
				cons_get_cdr(OBJECT_GET_CELL(vm->SP), vm->stack+sp-i);
			} else {
				obj=*(vm->stack+sp-i);
				// mapの終了戻り値の作成
				if (c==0) {
					if (!OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
					vm->SP=vm->stack+sp-n;
					OBJECT_SET_NIL(vm->SP);
				} else {
					tOBJECT tmp, tmp2;
					vm->SP-=n-i;
					tmp=*vm->SP;
					if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;
					OBJECT_SET_CONS(vm->SP, p);
					for (i=1; i<c; i++) {
						tmp=*(vm->SP-1);
						tmp2=*(vm->SP);
						if (cons_create_(vm, &p, &tmp, &tmp2)) return VM_ERROR;
						vm->SP--;
						OBJECT_SET_CONS(vm->SP, p);
					}
					vm->SP-=anum;
					*vm->SP=*(vm->SP+anum);
				}
				return VM_OK;
			}
		}
		c++;
		if (op_funcall(anum, vm)) return VM_ERROR;
	}
}

VM_RET OPERATION_CALL op_mapl(tINT anum, tPVM vm)
{
	tINT i, c, n=anum-1, sp=vm->SP-vm->stack;
	tOBJECT obj, ret;
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	ret=*(vm->SP-n+1);
	if (vm_push(vm, &ret)) return VM_ERROR;
	c=0;
	while (1) {
		if (vm_push(vm, vm->stack+sp-n)) return VM_ERROR;
		for (i=n-1; i>=0; i--) {
			if (OBJECT_IS_CONS(vm->stack+sp-i)) {
				if (vm_push(vm, vm->stack+sp-i)) return VM_ERROR;
				cons_get_cdr(OBJECT_GET_CELL(vm->SP), vm->stack+sp-i);
			} else {
				obj=*(vm->stack+sp-i);
				vm_pop(vm);
				if (c==0) {
					if (!OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
					vm->SP=vm->stack+sp-n;
					OBJECT_SET_NIL(vm->SP);
				} else {
					vm->SP-=n*2-i;
					*vm->SP=ret;
				}
				return VM_OK;
			}
		}
		c++;
		if (op_funcall(anum, vm)) return VM_ERROR;
		vm->SP--;
	}
}

VM_RET OPERATION_CALL op_mapcan(tINT anum, tPVM vm)
{
	tINT i, c, n=anum-1, sp=vm->SP-vm->stack;
	tOBJECT obj;
	tPCELL pp, p;
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	c=0; p=0;
	while (1) {
		if (vm_push(vm, vm->stack+sp-n)) return VM_ERROR;
		for (i=n-1; i>=0; i--) {
			if (OBJECT_IS_CONS(vm->stack+sp-i)) {
				pp=OBJECT_GET_CELL(vm->stack+sp-i);
				cons_get_car(pp, &obj);
				if(vm_push(vm, &obj)) return VM_ERROR;
				cons_get_cdr(pp, &obj);
				*(vm->stack+sp-i)=obj;
			} else {
				obj=*(vm->stack+sp-i);
				if (c==0) {
					if (!OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
					vm->SP=vm->stack+sp-n;
					OBJECT_SET_NIL(vm->SP);
				} else {
					vm->SP-=n*2-i+1;
					*vm->SP=*(vm->SP+n+1);
				}
				return VM_OK;
			}
		}
		c++;
		if (op_funcall(anum, vm)) return VM_ERROR;
		if (c!=1) {
			vm->SP--;
			if (OBJECT_IS_NIL(vm->SP)) {
				*vm->SP=*(vm->SP+1);
			} else if (OBJECT_IS_CONS(vm->SP)) {
				if (OBJECT_IS_CONS(vm->SP+1)) {
					if (p) {
						cons_set_cdr(p, vm->SP+1);
						p=cons_get_cdr_cons(p);
					} else {
						cons_set_cdr(OBJECT_GET_CELL(vm->SP), vm->SP+1);
						p=OBJECT_GET_CELL(vm->SP+1);
					}
				} else if (!OBJECT_IS_NIL(vm->SP+1)) {
					tOBJECT tmp=*(vm->SP+1);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
				}
			} else {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
			}
		}
	}
}

VM_RET OPERATION_CALL op_mapcon(tINT anum, tPVM vm)
{
	tINT i, c, n=anum-1, sp=vm->SP-vm->stack;
	tOBJECT obj;
	tPCELL p;
	c=0; p=0;
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	while (1) {
		if (vm_push(vm, vm->stack+sp-n)) return VM_ERROR;
		for (i=n-1; i>=0; i--) {
			if (OBJECT_IS_CONS(vm->stack+sp-i)) {
				if (vm_push(vm, vm->stack+sp-i)) return VM_ERROR;
				cons_get_cdr(OBJECT_GET_CELL(vm->SP), vm->stack+sp-i);
			} else {
				obj=*(vm->stack+sp-i);
				if (c==0) {
					if (!OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
					vm->SP=vm->stack+sp-n;
					OBJECT_SET_NIL(vm->SP);
				} else {
					vm->SP-=n*2-i+1;
					*vm->SP=*(vm->SP+n+1);
				}
				return VM_OK;
			}
		}
		c++;
		if (op_funcall(anum, vm)) return VM_ERROR;
		if (c!=1) {
			vm->SP--;
			if (OBJECT_IS_NIL(vm->SP)) {
				*vm->SP=*(vm->SP+1);
			} else if (OBJECT_IS_CONS(vm->SP)) {
				if (OBJECT_IS_CONS(vm->SP+1)) {
					if (p) {
						cons_set_cdr(p, vm->SP+1);
						p=cons_get_cdr_cons(p);
					} else {
						cons_set_cdr(OBJECT_GET_CELL(vm->SP), vm->SP+1);
						p=OBJECT_GET_CELL(vm->SP+1);
					}
				} else if (!OBJECT_IS_NIL(vm->SP+1)) {
					tOBJECT tmp=*(vm->SP+1);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
				}
			} else {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
			}
		}
	}
}

VM_RET OPERATION_CALL op_assoc(tPVM vm)
{
	if (OBJECT_IS_NIL(vm->SP)) {
		*--(vm->SP)=nil;
	} else if (OBJECT_IS_CONS(vm->SP)) {
		tPCELL p;
		tOBJECT obj;
		p=OBJECT_GET_CELL(vm->SP);
		while (p) {
			cons_get_car(p, &obj);
			if (!OBJECT_IS_CONS(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
			cons_get_car(cons_get_car_cons(p), &obj);
			if (object_eql(vm->SP-1, &obj)) {
				vm->SP--;
				cons_get_car(p, vm->SP);
				p=0;
			} else {
				cons_get_cdr(p, &obj);
				if (OBJECT_IS_CONS(&obj))
					p=OBJECT_GET_CELL(&obj);
				else if (OBJECT_IS_NIL(&obj)) {
					*--(vm->SP)=nil;
					p=0;
				} else {// ドットリスト
					tOBJECT tmp=*vm->SP;
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
				}
			}
		}
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_basic_array_p(tPVM vm)
{
	if (OBJECT_IS_ARRAY(vm->SP)||OBJECT_IS_VECTOR(vm->SP)||OBJECT_IS_STRING(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_basic_array_a_p(tPVM vm)
{
	if (OBJECT_IS_ARRAY(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_general_array_a_p(tPVM vm)
{
	if (OBJECT_IS_ARRAY(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_create_array(tINT anum, tPVM vm)
{
	if (anum==1) {
		if (vm_push(vm, &nil)) return VM_ERROR;
	} else if (anum!=2) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	if (OBJECT_IS_NIL(vm->SP-1)) {
		tPCELL cell;
		tOBJECT tmp=*vm->SP;
		if (array_create_2_(vm, 0, &tmp, &cell)) return VM_ERROR;
		vm->SP--;
		cell_to_object(cell, vm->SP);
	} else {
		tPCELL cell;
		tOBJECT tmp=*vm->SP;
		if (!OBJECT_IS_CONS(vm->SP-1)) {
			return signal_domain_error(vm,TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
		if (array_create_2_(vm, OBJECT_GET_CELL(vm->SP-1), &tmp, &cell)) return VM_ERROR;
		vm->SP--;
		cell_to_object(cell, vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_aref(tINT anum, tPVM vm)
{
	if (anum<1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tOBJECT obj=*(vm->SP-anum+1);
		if (OBJECT_IS_ARRAY(&obj)) {
			if (array_get_object(vm, OBJECT_GET_CELL(&obj), anum-1, &obj)) return VM_ERROR;
			vm->SP-=anum-1;
			*vm->SP=obj;
			return VM_OK;
		} else if (OBJECT_IS_VECTOR(&obj)) {
			// ベクタ
			if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
			if (!OBJECT_IS_INTEGER(vm->SP)) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
			}
			vm->SP--;
			return vector_get_object(vm, OBJECT_GET_CELL(vm->SP), OBJECT_GET_INTEGER(vm->SP+1), vm->SP);
		} else if (OBJECT_IS_STRING(&obj)) {
			// 文字列
			if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
			if (!OBJECT_IS_INTEGER(vm->SP)) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
			}
			vm->SP--;
			return string_get_character(vm, OBJECT_GET_CELL(vm->SP), OBJECT_GET_INTEGER(vm->SP+1), vm->SP);
		} else {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_BASIC_ARRAY, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_garef(tINT anum, tPVM vm)
{
	if (anum==0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tOBJECT obj=*(vm->SP-anum+1);
		if (!OBJECT_IS_ARRAY(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_GENERAL_ARRAY_A, &obj);
		if (array_get_object(vm, OBJECT_GET_CELL(&obj), anum-1, &obj)) return VM_ERROR;
		vm->SP-=anum-1;
		*vm->SP=obj;
		return VM_OK;
	}

	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

VM_RET OPERATION_CALL op_array_dimensions(tPVM vm)
{
	if (OBJECT_IS_ARRAY(vm->SP)) {
		tOBJECT tmp;
		if (array_get_dimension_list(vm, OBJECT_GET_CELL(vm->SP), &tmp)) return VM_ERROR;
		*vm->SP=tmp;
		return VM_OK;
	} else if (OBJECT_IS_VECTOR(vm->SP)) {
		tPCELL p;
		tOBJECT obj;
		OBJECT_SET_INTEGER(&obj, vector_get_length(OBJECT_GET_CELL(vm->SP)));
		if (cons_create_(vm, &p, &obj, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(vm->SP, p);
		return VM_OK;
	} else if (OBJECT_IS_STRING(vm->SP)) {
		tPCELL p;
		tOBJECT obj;
		OBJECT_SET_INTEGER(&obj, string_get_length(OBJECT_GET_CELL(vm->SP))-1);
		if (cons_create_(vm, &p, &obj, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(vm->SP, p);
		return VM_OK;
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_BASIC_ARRAY, &tmp);
	}
}

VM_RET OPERATION_CALL op_basic_vector_p(tPVM vm)
{
	if (OBJECT_IS_STRING(vm->SP)||OBJECT_IS_VECTOR(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_general_vector_p(tPVM vm)
{
	if (OBJECT_IS_VECTOR(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_create_vector(tINT anum, tPVM vm)
{
	tPCELL p;
	if (anum==1) {
		if (vm_push(vm, &nil)) return VM_ERROR;
	} else if (anum!=2) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	if (!OBJECT_IS_INTEGER(vm->SP-1)||
		(OBJECT_GET_INTEGER(vm->SP-1)<0)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	{
		tOBJECT tmp=*vm->SP;
		if (vector_create_2_(vm, OBJECT_GET_INTEGER(vm->SP-1), &tmp, &p)) return VM_ERROR;
	}
	vm->SP--;
	cell_to_object(p, vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_vector(tINT anum, tPVM vm)
{
	tINT i;
	tPCELL p;
	if (vector_create_2_(vm, anum, &nil, &p)) return VM_ERROR;
	for (i=anum-1; i>=0; i--) {
		if (vector_set_object(vm, p, i, vm->SP--)) return VM_ERROR;
	}
	vm->SP++;
	OBJECT_SET_VECTOR(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_stringp(tPVM vm)
{
	if (OBJECT_IS_STRING(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_create_string(tINT anum, tPVM vm)
{
	tPCELL p;
	if (anum==1) {
		tOBJECT obj;
		OBJECT_SET_CHARACTER(&obj, ' ');
		if (vm_push(vm, &obj)) return VM_ERROR;
	} else if (anum!=2) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	if (!OBJECT_IS_INTEGER(vm->SP-1)||
		(OBJECT_GET_INTEGER(vm->SP-1)<0)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (!OBJECT_IS_CHARACTER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
	if (string_create_2(vm, OBJECT_GET_INTEGER(vm->SP-1), OBJECT_GET_CHARACTER(vm->SP), &p)) return VM_ERROR;
	vm->SP--;
	OBJECT_SET_STRING(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_string_equal(tPVM vm)
{
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if ((string_get_length(OBJECT_GET_CELL(vm->SP+1))==string_get_length(OBJECT_GET_CELL(vm->SP)))&&
		(strcmp(string_get_string(OBJECT_GET_CELL(vm->SP+1)), string_get_string(OBJECT_GET_CELL(vm->SP)))==0))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_string_not_equal(tPVM vm)
{
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if ((string_get_length(OBJECT_GET_CELL(vm->SP+1))==string_get_length(OBJECT_GET_CELL(vm->SP)))&&
		(strcmp(string_get_string(OBJECT_GET_CELL(vm->SP+1)), string_get_string(OBJECT_GET_CELL(vm->SP)))==0))
		OBJECT_SET_NIL(vm->SP);
	else
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	return VM_OK;
}

VM_RET OPERATION_CALL op_string_less(tPVM vm)
{
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if ((string_get_length(OBJECT_GET_CELL(vm->SP))<=string_get_length(OBJECT_GET_CELL(vm->SP+1)))&&
		(strcmp(string_get_string(OBJECT_GET_CELL(vm->SP)), string_get_string(OBJECT_GET_CELL(vm->SP+1)))<0))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_string_greater(tPVM vm)
{
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if ((string_get_length(OBJECT_GET_CELL(vm->SP))<=string_get_length(OBJECT_GET_CELL(vm->SP+1)))&&
		(strcmp(string_get_string(OBJECT_GET_CELL(vm->SP)), string_get_string(OBJECT_GET_CELL(vm->SP+1)))<=0))
		OBJECT_SET_NIL(vm->SP);
	else
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	return VM_OK;
}

VM_RET OPERATION_CALL op_string_ge(tPVM vm)
{
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if ((string_get_length(OBJECT_GET_CELL(vm->SP))<=string_get_length(OBJECT_GET_CELL(vm->SP+1)))&&
		(strcmp(string_get_string(OBJECT_GET_CELL(vm->SP)), string_get_string(OBJECT_GET_CELL(vm->SP+1)))<0))
		OBJECT_SET_NIL(vm->SP);
	else
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	return VM_OK;
}

VM_RET OPERATION_CALL op_string_le(tPVM vm)
{
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	vm->SP--;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if ((string_get_length(OBJECT_GET_CELL(vm->SP))<=string_get_length(OBJECT_GET_CELL(vm->SP+1)))&&
		(strcmp(string_get_string(OBJECT_GET_CELL(vm->SP)), string_get_string(OBJECT_GET_CELL(vm->SP+1)))<=0))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_char_index(tINT anum, tPVM vm)
{
	if (anum==2) {
		tOBJECT obj;
		OBJECT_SET_INTEGER(&obj, 0);
		if (vm_push(vm, &obj)) return VM_ERROR;
	} else if (anum!=3) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	{
		tINT i;
		tCHAR cc, *p;
		if (!OBJECT_IS_INTEGER(vm->SP)||
			(OBJECT_GET_INTEGER(vm->SP)<0)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
		}
		i=OBJECT_GET_INTEGER(vm->SP);
		vm->SP--;
		if (!OBJECT_IS_STRING(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
		}
		if (!OBJECT_IS_CHARACTER(vm->SP-1)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
		}
		cc=OBJECT_GET_CHARACTER(vm->SP-1);
		if (i<string_get_length(OBJECT_GET_CELL(vm->SP)))
			p=memchr(string_get_string(OBJECT_GET_CELL(vm->SP))+i, cc, string_get_length(OBJECT_GET_CELL(vm->SP)));
		else
			p=0;
		vm->SP--;
		if (p)
			OBJECT_SET_INTEGER(vm->SP, p-string_get_string(OBJECT_GET_CELL(vm->SP+1)));
		else
			*vm->SP=nil;
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_string_index(tINT anum, tPVM vm)
{
	if (anum==2) {
		tOBJECT obj;
		OBJECT_SET_INTEGER(&obj, 0);
		if (vm_push(vm, &obj)) return VM_ERROR;
	} else if (anum!=3) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	{
		tINT i, f;
		tPCELL string1, string2;
		tCHAR *p;
		if (!OBJECT_IS_INTEGER(vm->SP)||
			(OBJECT_GET_INTEGER(vm->SP)<0)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
		}
		i=OBJECT_GET_INTEGER(vm->SP);
		vm->SP--;
		if (!OBJECT_IS_STRING(vm->SP)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
		}
		string2=OBJECT_GET_CELL(vm->SP);
		if (!OBJECT_IS_STRING(vm->SP-1)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
		}
		string1=OBJECT_GET_CELL(vm->SP-1);
		f=1;
		if (string_get_string(string1)[0]=='\0') {
			vm->SP--;
			OBJECT_SET_INTEGER(vm->SP, 0);
		} else {
			while (f) {
				p=memchr(string_get_string(string2)+i, string_get_string(string1)[0], string_get_length(string2)-i);
				if (p&&memcmp(p, string_get_string(string1), string_get_length(string1)-1))
					i=p-string_get_string(string2)+1;
				else
					f=0;
			}
			vm->SP--;
			if (p)
				OBJECT_SET_INTEGER(vm->SP, p-string_get_string(string2));
			else
				OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_string_append(tINT anum, tPVM vm)
{
	tINT i, length;
	tPCELL string;
	// 引数の型検査と長さの計算
	length=0;
	for (i=1; i<=anum; i++) {
		if (!OBJECT_IS_STRING(vm->SP-anum+i)) {
			tOBJECT tmp=*(vm->SP-anum+i);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
		}
		length+=string_get_length(OBJECT_GET_CELL(vm->SP-anum+i))-1;// NULL文字除く
	}
	//length++;create_string_2のほうで +1 してるのでいらない
	// データ文字列作成
	if (string_create_2(vm, length, ' ', &string)) return VM_ERROR;
	// 文字のコピー
	length=0;
	for (i=1; i<=anum; i++) {
		tPCELL p=OBJECT_GET_CELL(vm->SP-anum+i);
		memcpy(string_get_data(string)+length, string_get_string(p), string_get_length(p)-1);
		length+=string_get_length(p)-1;
	}
	// 後処理
	vm->SP-=anum-1;
	OBJECT_SET_STRING(vm->SP, string);
	return VM_OK;
}

VM_RET OPERATION_CALL op_length(tPVM vm)
{
	tINT len;
	switch (OBJECT_GET_TYPE(vm->SP)) {
	case OBJECT_NIL:
		len=0;
		break;
	case OBJECT_CONS:
		len=cons_get_length(OBJECT_GET_CELL(vm->SP));
		break;
	case OBJECT_VECTOR:
		len=vector_get_length(OBJECT_GET_CELL(vm->SP));
		break;
	case OBJECT_STRING:
		len=string_get_length(OBJECT_GET_CELL(vm->SP))-1;// NULL文字のぶん
		break;
	default:// expected classは？<sequence>??
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
	}
	OBJECT_SET_INTEGER(vm->SP, len);
	return VM_OK;
}

VM_RET OPERATION_CALL op_elt(tPVM vm)
{
	tINT i;
	tOBJECT obj;
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	i=OBJECT_GET_INTEGER(vm->SP);
	switch (OBJECT_GET_TYPE(vm->SP-1)) {
	case OBJECT_NIL:
		if (i) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
		obj=nil;
		break;
	case OBJECT_CONS:
		if (i<0) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
		if (list_get_object(vm, OBJECT_GET_CELL(vm->SP-1), i, &obj)) return VM_ERROR;
		break;
	case OBJECT_VECTOR:
		if (vector_get_object(vm, OBJECT_GET_CELL(vm->SP-1), i, &obj)) return VM_ERROR;
		break;
	case OBJECT_STRING:
		if (string_get_character(vm, OBJECT_GET_CELL(vm->SP-1), i, &obj)) return VM_ERROR;
		break;
	default:// expected classは？<sequence>??
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
	}
	*--(vm->SP)=obj;
	return VM_OK;
}

VM_RET OPERATION_CALL op_subseq(tPVM vm)
{
	tINT z1, z2;
	tOBJECT obj;

	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	z2=OBJECT_GET_INTEGER(vm->SP);
	if (!OBJECT_IS_INTEGER(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	z1=OBJECT_GET_INTEGER(vm->SP-1);
	switch (OBJECT_GET_TYPE(vm->SP-2)) {
	case OBJECT_NIL:
		if ((z1!=0)||(z2!=0)) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
		OBJECT_SET_NIL(&obj);
		break;
	case OBJECT_CONS:
		if (z1==z2) {
			OBJECT_SET_NIL(&obj);
		} else {
			tPCELL p, p2, p3, p4;
			tINT i;
			p=OBJECT_GET_CELL(vm->SP-2);
			i=0;
			while (i<z1) {
				cons_get_cdr(p, &obj);
				if (!OBJECT_IS_CONS(&obj)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
				p=OBJECT_GET_CELL(&obj);
				i++;
			}
			cons_get_car(p, &obj);
			if (cons_create_(vm, &p2, &obj, &nil)) return VM_ERROR;
			OBJECT_SET_CONS(&obj, p2);
			if (vm_push(vm, &obj)) return VM_ERROR;
			p4=p2;
			while (i<z2-1) {
				cons_get_cdr(p, &obj);
				if (!OBJECT_IS_CONS(&obj)) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
				p=OBJECT_GET_CELL(&obj);
				i++;
				cons_get_car(p, &obj);
				if (cons_create(vm, &p3, &obj, &nil)) return VM_ERROR;
				OBJECT_SET_CONS(&obj, p3);
				cons_set_cdr(p4, &obj);
				p4=p3;
			}
			OBJECT_SET_CONS(&obj, p2);
			vm_pop(vm);
		}
		break;
	case OBJECT_VECTOR:
		{
			tPCELL p, pp;
			tINT i, n;
			pp=OBJECT_GET_CELL(vm->SP-2);
			if ((z1<0)||(z2>=vector_get_length(pp))) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
			n=z2-z1;
			if (vector_create_2_(vm, n, &nil, &p)) return VM_ERROR;
			for (i=0; i<n; i++) {
				if (vector_get_object(vm, pp, z1+i, &obj)||
					vector_set_object(vm, p, i, &obj)) return VM_ERROR;
			}
			OBJECT_SET_VECTOR(&obj, p);
		}
		break;
	case OBJECT_STRING:
		{
			tPCELL p, pp;
			pp=OBJECT_GET_CELL(vm->SP-2);
			if ((z1<0)||(z2>=string_get_length(pp))) return signal_condition(vm, TISL_ERROR_INDEX_OUT_OF_RANGE);
			if (string_create_2(vm, z2-z1, ' ', &p)) return VM_ERROR;
			memcpy(string_get_data(p), string_get_string(pp)+z1, z2-z1);
			OBJECT_SET_STRING(&obj, p);
		}
		break;
	default:// expected classは？<sequence>??
		{
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &tmp);
		}
	}
	vm->SP-=2;
	*vm->SP=obj;
	return VM_OK;
}

VM_RET OPERATION_CALL op_map_into(tINT anum, tPVM vm)
{
	tINT i, c, f, m, n=anum-2, sp=vm->SP-vm->stack;
	tOBJECT obj;
	tPCELL pa;
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	obj=*(vm->SP-anum+1);
	if (!OBJECT_IS_CONS(&obj)&&!OBJECT_IS_VECTOR(&obj)&&!OBJECT_IS_STRING(&obj)) {
		if (OBJECT_IS_NIL(&obj)) {
			vm->SP-=anum-1;
			return VM_OK;
		}
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
	}
	pa=OBJECT_GET_CELL(&obj);
	if (CELL_GET_IMMUTABLE(pa))
		return signal_condition(vm, TISL_ERROR_IMMUTABLE_OBJECT);
	if (OBJECT_IS_CONS(&obj)) {
		f=1;
		m=cons_get_length(pa);
	} else if (OBJECT_IS_VECTOR(&obj)) {
		f=0;
		m=vector_get_length(pa);
	} else {
		f=2;
		m=string_get_length(pa)-1;
	}
	c=0;
	while (c<m) {
		if (vm_push(vm, vm->stack+sp-n)) return VM_ERROR;
		for (i=n-1; i>=0; i--) {
			obj=*(vm->stack+sp-i);
			if (OBJECT_IS_CONS(&obj)) {
				tPCELL p=OBJECT_GET_CELL(&obj);
				cons_get_car(p, &obj);
				if (vm_push(vm, &obj)) return VM_ERROR;
				cons_get_cdr(p, vm->stack+sp-i);
			} else if (OBJECT_IS_VECTOR(&obj)) {
				if (vector_get_object(vm, OBJECT_GET_CELL(&obj), c, &obj)) return VM_ERROR;
				if (vm_push(vm, &obj)) return VM_ERROR;
			} else if (OBJECT_IS_STRING(&obj)) {
				if (string_get_character(vm, OBJECT_GET_CELL(&obj), c, &obj)) return VM_ERROR;
				if (vm_push(vm, &obj)) return VM_ERROR;
			} else {
					goto LOOPOUT;
			}
		}
		c++;
		if (po_funcall(vm, n+1)) return VM_ERROR;
		if (f==1) {
			cons_set_car(pa, vm->SP);
			pa=cons_get_cdr_cons(pa);
		} else if (f==2) {
			if (!OBJECT_IS_CHARACTER(vm->SP)) {
				tOBJECT tmp=*vm->SP;
				return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
			}
			if (string_set_character(vm, pa, c-1, OBJECT_GET_CHARACTER(vm->SP))) return VM_ERROR;
		} else {
			if (vector_set_object(vm, pa, c-1, vm->SP)) return VM_ERROR;
		}
		vm->SP--;
	}
LOOPOUT:
	if (c==0) {
		if (OBJECT_IS_NIL(&obj)) return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_LIST, &obj);
	}
	vm->SP=vm->stack+sp-anum+1;
	return VM_OK;
}

VM_RET OPERATION_CALL op_streamp(tPVM vm)
{
	if (OBJECT_IS_STRING_STREAM(vm->SP)||
		OBJECT_IS_FILE_STREAM(vm->SP))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_open_stream_p(tPVM vm)
{
	if ((OBJECT_IS_FILE_STREAM(vm->SP)&&
		!file_stream_is_closed(OBJECT_GET_CELL(vm->SP)))||
		(OBJECT_IS_STRING_STREAM(vm->SP)&&
		!string_stream_is_closed(OBJECT_GET_CELL(vm->SP))))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_input_stream_p(tPVM vm)
{
	if ((OBJECT_IS_FILE_STREAM(vm->SP)&&
		file_stream_is_input(OBJECT_GET_CELL(vm->SP)))||
		(OBJECT_IS_STRING_STREAM(vm->SP)&&
		string_stream_is_input(OBJECT_GET_CELL(vm->SP))))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_output_stream_p(tPVM vm)
{
	if ((OBJECT_IS_FILE_STREAM(vm->SP)&&
		file_stream_is_output(OBJECT_GET_CELL(vm->SP)))||
		(OBJECT_IS_STRING_STREAM(vm->SP)&&
		string_stream_is_output(OBJECT_GET_CELL(vm->SP))))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_standard_input(tPVM vm)
{
	vm->SP++;
	cell_to_object(vm_get_standard_input(vm), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_standard_output(tPVM vm)
{
	vm->SP++;
	cell_to_object(vm_get_standard_output(vm), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_error_output(tPVM vm)
{
	vm->SP++;
	cell_to_object(vm_get_error_output(vm), vm->SP);
	return VM_OK;
}

static VM_RET po_open_file(tPVM vm, const tINT anum, const tINT flag)
{
	if (anum==1) {
		tOBJECT obj;
		OBJECT_SET_INTEGER(&obj, 8);
		if (vm_push(vm, &obj)) return VM_ERROR;
	} else if (anum!=2) {
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
	{
		tPCELL stream;
		if (!OBJECT_IS_STRING(vm->SP-1)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
		}
		if (!OBJECT_IS_INTEGER(vm->SP)||
			(OBJECT_GET_INTEGER(vm->SP)!=8)) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
		}
		if (file_stream_create(vm, flag, OBJECT_GET_CELL(vm->SP-1), &stream)) return VM_ERROR;
		vm->SP--;
		cell_to_object(stream, vm->SP);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_open_input_file(tINT anum, tPVM vm)
{
	return po_open_file(vm, anum, STREAM_INPUT);
}

VM_RET OPERATION_CALL op_open_output_file(tINT anum, tPVM vm)
{
	return po_open_file(vm, anum, STREAM_OUTPUT);
}

VM_RET OPERATION_CALL op_open_io_file(tINT anum, tPVM vm)
{
	return po_open_file(vm, anum, STREAM_INPUT | STREAM_OUTPUT);
}

VM_RET OPERATION_CALL op_close(tPVM vm)
{
	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		if (string_stream_close(vm, OBJECT_GET_CELL(vm->SP))) return VM_ERROR;
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		if (file_stream_flush(vm, OBJECT_GET_CELL(vm->SP))) return VM_ERROR;
		if (file_stream_close(vm, OBJECT_GET_CELL(vm->SP))) return VM_ERROR;
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_finish_output(tPVM vm)
{
	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		if (!file_stream_is_output(OBJECT_GET_CELL(vm->SP))) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		if (!file_stream_is_output(OBJECT_GET_CELL(vm->SP))) {
			tOBJECT tmp=*vm->SP;
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (!file_stream_flush(vm, OBJECT_GET_CELL(vm->SP))) return VM_ERROR;
	} else {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_create_string_input_stream(tPVM vm)
{
	tPCELL p;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if (string_stream_create_input(vm, string_get_string(OBJECT_GET_CELL(vm->SP)), &p)) return VM_ERROR;
	OBJECT_SET_STRING_STREAM(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_create_string_output_stream(tPVM vm)
{
	tPCELL p;
	if (string_stream_create_output(vm, &p)) return VM_ERROR;
	vm->SP++;
	OBJECT_SET_STRING_STREAM(vm->SP, p);
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_output_stream_string(tPVM vm)
{
	tPCELL p;
	if (!OBJECT_IS_STRING_STREAM(vm->SP)) {
		tOBJECT tmp=*vm->SP;
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING_STREAM, &tmp);
	}
	if (string_stream_to_string(vm, OBJECT_GET_CELL(vm->SP), &p)) return VM_ERROR;
	OBJECT_SET_STRING(vm->SP, p);
	return VM_OK;
}

static VM_RET po_read_(tPVM vm, const tINT anum)
{
	tOBJECT obj;
	switch (anum) {
	case 0:// input-stream 標準入力ストリーム
		cell_to_object(vm_get_standard_input(vm), &obj);
		if (vm_push(vm, &obj)) return VM_ERROR;
	case 1:// eos-error-p t
		OBJECT_SET_SYMBOL(&obj, SYMBOL_T);
		if (vm_push(vm, &obj)) return VM_ERROR;
	case 2:// eos-value nil
		OBJECT_SET_NIL(&obj);
		if (vm_push(vm, &obj)) return VM_ERROR;
	case 3:
		return VM_OK;
	default:
		return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	}
}

VM_RET OPERATION_CALL op_read(tINT anum, tPVM vm)
{
// (read [input-stream [eos-error-p [eos-value]]])
	if (po_read_(vm, anum)) return VM_ERROR;
	{
		tPCELL stream;
		tOBJECT obj;
		tINT sp;
		tBOOL old;
		old=vm_get_reader_eos_error(vm);
		if (OBJECT_IS_NIL(vm->SP-1)) {
			vm_set_reader_eos_error(vm);
		} else {
			vm_reset_reader_eos_error(vm);
		}
		if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!file_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (file_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!string_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (string_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
		}
		sp=vm->SP-vm->stack;
		if (read_form(vm, stream, &obj)) {
			// 正常終了しなかった
			if (vm_last_condition_is_eos_error(vm)) {
				vm->SP=vm->stack+sp-2;
				*vm->SP=*(vm->stack+sp);
				vm_set_last_condition_ok(vm);
			} else {// 別の例外
				vm_reset_reader_eos_error(vm);
				return VM_ERROR;
			}
		} else {// 正常にreadが終了した
			vm->SP=vm->stack+sp-2;// いらないのか？
			*vm->SP=obj;
		}
		if (old)
			vm_set_reader_eos_error(vm);
		else
			vm_reset_reader_eos_error(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_read_char(tINT anum, tPVM vm)
{
	if (po_read_(vm, anum)) return VM_ERROR;
	{
		tPCELL stream;
		tINT sp;
		tCHAR c;
		tBOOL old;
		old=vm_get_reader_eos_error(vm);
		if (OBJECT_IS_NIL(vm->SP-1)) {
			vm_set_reader_eos_error(vm);
		} else {
			vm_reset_reader_eos_error(vm);
		}
		if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!file_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (file_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!string_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (string_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
		}
		sp=vm->SP-vm->stack;
		if (read_char(vm, stream, &c)) {
			// 正常終了しなかった
			if (vm_last_condition_is_eos_error(vm)) {
				vm->SP=vm->stack+sp-2;
				*vm->SP=*(vm->stack+sp);
				vm_set_last_condition_ok(vm);
			} else {// 別の例外
				vm_reset_reader_eos_error(vm);
				return VM_ERROR;
			}
		} else {// 正常にreadが終了した
			vm->SP=vm->stack+sp-2;// いらないのか？
			OBJECT_SET_CHARACTER(vm->SP, c);
		}
		if (old)
			vm_set_reader_eos_error(vm);
		else
			vm_reset_reader_eos_error(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_preview_char(tINT anum, tPVM vm)
{
	if (po_read_(vm, anum)) return VM_ERROR;
	{
		tPCELL stream;
		tINT sp;
		tCHAR c;
		tBOOL old;
		old=vm_get_reader_eos_error(vm);
		if (OBJECT_IS_NIL(vm->SP-1)) {
			vm_set_reader_eos_error(vm);
		} else {
			vm_reset_reader_eos_error(vm);
		}
		if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!file_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (file_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!string_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (string_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
		}
		sp=vm->SP-vm->stack;
		if (preview_char(vm, stream, &c)) {
			// 正常終了しなかった
			if (vm_last_condition_is_eos_error(vm)) {
				vm->SP=vm->stack+sp-2;
				*vm->SP=*(vm->stack+sp);
				vm_set_last_condition_ok(vm);
			} else {// 別の例外
				vm_reset_reader_eos_error(vm);
				return VM_ERROR;
			}
		} else {// 正常にreadが終了した
			vm->SP=vm->stack+sp-2;// いらないのか？
			OBJECT_SET_CHARACTER(vm->SP, c);
		}
		if (old)
			vm_set_reader_eos_error(vm);
		else
			vm_reset_reader_eos_error(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_read_line(tINT anum, tPVM vm)
{
	if (po_read_(vm, anum)) return VM_ERROR;
	{
		tPCELL stream, string;
		tINT sp;
		tBOOL old;
		old=vm_get_reader_eos_error(vm);
		if (OBJECT_IS_NIL(vm->SP-1)) {
			vm_set_reader_eos_error(vm);
		} else {
			vm_reset_reader_eos_error(vm);
		}
		if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!file_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (file_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!string_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (string_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
		}
		sp=vm->SP-vm->stack;
		if (read_line(vm, stream, &string)) {
			// 正常終了しなかった
			if (vm_last_condition_is_eos_error(vm)) {
				vm->SP=vm->stack+sp-2;
				*vm->SP=*(vm->stack+sp);
				vm_set_last_condition_ok(vm);
			} else {// 別の例外
				vm_reset_reader_eos_error(vm);
				return VM_ERROR;
			}
		} else {// 正常にreadが終了した
			vm->SP=vm->stack+sp-2;// いらないのか？
			OBJECT_SET_STRING(vm->SP, string);
		}
		if (old)
			vm_set_reader_eos_error(vm);
		else
			vm_reset_reader_eos_error(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_stream_ready_p(tPVM vm)
{
	tPCELL stream;

	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!string_stream_is_input(stream)) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_locked(stream))
			OBJECT_SET_NIL(vm->SP);
		else
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!file_stream_is_input(stream)) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_locked(stream))
			OBJECT_SET_NIL(vm->SP);
		else
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_format(tINT anum, tPVM vm)
{
	tPCELL p, stream;
	if (anum!=2) {// 引数のリストを作成
		tINT i;
		tOBJECT tmp, tmp2;
		tmp=*vm->SP;
		if (cons_create_(vm, &p, &tmp, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(vm->SP, p);
		for (i=3; i<anum; i++) {
			tmp=*(vm->SP-1);
			tmp2=*(vm->SP);
			if (cons_create(vm, &p, &tmp, &tmp2)) return VM_ERROR;
			vm->SP--;
			OBJECT_SET_CONS(vm->SP, p);
		}
	} else {
		if (vm_push(vm, &nil)) return VM_ERROR;
		p=0;
	}
	// ストリームの検査
	if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
		stream=OBJECT_GET_CELL(vm->SP-2);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
		stream=OBJECT_GET_CELL(vm->SP-2);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP-2);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (!OBJECT_IS_STRING(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if (format_l(vm, stream, string_get_string(OBJECT_GET_CELL(vm->SP-1)), p)) return VM_ERROR;
	vm->SP-=2;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_format_char(tPVM vm)
{
	tPCELL stream;
	if (OBJECT_IS_STRING_STREAM(vm->SP-1)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP-1)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (!OBJECT_IS_CHARACTER(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_CHARACTER, &tmp);
	}
	if (format_char(vm, stream, OBJECT_GET_CHARACTER(vm->SP))) return VM_ERROR;
	vm->SP--;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_format_float(tPVM vm)
{
	tPCELL stream;
	if (OBJECT_IS_STRING_STREAM(vm->SP-1)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP-1)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (!OBJECT_IS_FLOAT(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FLOAT, &tmp);
	}
	if (format_float(vm, stream, OBJECT_GET_FLOAT(vm->SP))) return VM_ERROR;
	vm->SP--;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_format_fresh_line(tPVM vm)
{
	tPCELL stream;
	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (format_fresh_line(vm, stream)) return VM_ERROR;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}


VM_RET OPERATION_CALL op_format_integer(tPVM vm)
{
	tPCELL stream;
	tINT r;
	if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
		stream=OBJECT_GET_CELL(vm->SP-2);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
		stream=OBJECT_GET_CELL(vm->SP-2);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP-2);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (!OBJECT_IS_INTEGER(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	r=OBJECT_GET_INTEGER(vm->SP);
	if ((r<2)||(r>36)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (format_integer(vm, stream, OBJECT_GET_INTEGER(vm->SP-1), r)) return VM_ERROR;
	vm->SP-=2;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_format_object(tPVM vm)
{
	tPCELL stream;
	tBOOL old;
	if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
		stream=OBJECT_GET_CELL(vm->SP-2);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
		stream=OBJECT_GET_CELL(vm->SP-2);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP-2);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	old=vm_get_writer_flag(vm);
	if (OBJECT_IS_NIL(vm->SP)) {
		vm_reset_writer_flag(vm);
	} else {
		vm_set_writer_flag(vm);
	}
	if (format_object(vm, stream, vm->SP-1)) { vm_reset_writer_flag(vm); return VM_ERROR; }
	vm->SP-=2;
	OBJECT_SET_NIL(vm->SP);
	if (old)
		vm_set_writer_flag(vm);
	else
		vm_reset_writer_flag(vm);
	return VM_OK;
}

VM_RET OPERATION_CALL op_format_tab(tPVM vm)
{
	tPCELL stream;
	if (OBJECT_IS_STRING_STREAM(vm->SP-1)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP-1)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (format_tab(vm, stream, OBJECT_GET_INTEGER(vm->SP))) return VM_ERROR;
	vm->SP--;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_read_byte(tINT anum, tPVM vm)
{
	// fileを開くときにtext-modeになっているので改行文字などで補正がはいっている
	// fileを開くときにbinary-modeでひらいて自分で改行文字等の補正をおこなうべき
	// あとで!!!/*!!!*/
	if (po_read_(vm, anum)) return VM_ERROR;
	{
		tPCELL stream;
		tINT sp;
		tCHAR c;
		if (OBJECT_IS_NIL(vm->SP-1)) {
			vm_set_reader_eos_error(vm);
		} else {
			vm_reset_reader_eos_error(vm);
		}
		if (OBJECT_IS_FILE_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!file_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP-2);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (file_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else if (OBJECT_IS_STRING_STREAM(vm->SP-2)) {
			stream=OBJECT_GET_CELL(vm->SP-2);
			if (!string_stream_is_input(stream)) {
				tOBJECT tmp=*(vm->SP);
				return signal_domain_error(vm, TISL_ERROR_NOT_AN_INPUT_STREAM, CLASS_STREAM, &tmp);
			}
			if (string_stream_is_closed(stream))
				return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
		} else {
			tOBJECT tmp=*(vm->SP-2);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
		}
		sp=vm->SP-vm->stack;
		if (read_char(vm, stream, &c)) {
			// 正常終了しなかった
			if (vm_last_condition_is_eos_error(vm)) {
				vm->SP=vm->stack+sp-2;
				*vm->SP=*(vm->stack+sp);
			} else {// 別の例外
				vm_reset_reader_eos_error(vm);
				return VM_ERROR;
			}
		} else {// 正常にreadが終了した
			vm->SP=vm->stack+sp-2;// いらないのか？
			OBJECT_SET_INTEGER(vm->SP, (tINT)c);
		}
		vm_reset_reader_eos_error(vm);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_write_byte(tPVM vm)
{
	tPCELL stream;
	if (OBJECT_IS_STRING_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP);
		if (!string_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (string_stream_is_closed(stream))
			return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		stream=OBJECT_GET_CELL(vm->SP-1);
		if (!file_stream_is_output(stream)) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_NOT_AN_OUTPUT_STREAM, CLASS_STREAM, &tmp);
		}
		if (file_stream_is_closed(stream))
		return signal_stream_error(vm, TISL_ERROR_STREAM_IS_CLOSED, stream);
	} else {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
	if (!OBJECT_IS_INTEGER(vm->SP-1)||
		((OBJECT_GET_INTEGER(vm->SP-1)<0)||(OBJECT_GET_INTEGER(vm->SP-1)>255))) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (format_char(vm, stream, (tCHAR)OBJECT_GET_INTEGER(vm->SP))) return VM_ERROR;
	vm->SP--;
	OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_probe_file(tPVM vm)
{
	FILE* file;
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	file=fopen(string_get_string(OBJECT_GET_CELL(vm->SP)), "r");
	if (file) {
		fclose(file);
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_file_position(tPVM vm)
{
	if (OBJECT_IS_FILE_STREAM(vm->SP)) {
		tINT i;
		tPCELL stream=OBJECT_GET_CELL(vm->SP);
		if (file_stream_is_closed(stream)) return signal_stream_error(vm, TISL_ERROR_STREAM_ERROR, stream);
		if (file_stream_get_position(vm, stream, &i)) return VM_ERROR;
		OBJECT_SET_INTEGER(vm->SP, i);
		return VM_OK;
	} else {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM, &tmp);
	}
}

VM_RET OPERATION_CALL op_set_file_position(tPVM vm)
{
	if (!OBJECT_IS_INTEGER(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (OBJECT_IS_FILE_STREAM(vm->SP-1)) {
		tINT i=OBJECT_GET_INTEGER(vm->SP);
		tPCELL stream=OBJECT_GET_CELL(vm->SP-1);
		if (file_stream_is_closed(stream)) return signal_stream_error(vm, TISL_ERROR_STREAM_ERROR, stream);
		if (file_stream_set_position(vm, stream, i)) return VM_ERROR;
		OBJECT_SET_INTEGER(vm->SP, i);
		return VM_OK;
	} else {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_FILE_STREAM, &tmp);
	}
}

VM_RET OPERATION_CALL op_file_length(tPVM vm)
{
#ifdef _WIN32
	struct _stat buf;
	if (!OBJECT_IS_STRING(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if (!OBJECT_IS_INTEGER(vm->SP)||
		(OBJECT_GET_INTEGER(vm->SP)!=8)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_INTEGER, &tmp);
	}
	if (_stat(string_get_string(OBJECT_GET_CELL(vm->SP-1)), &buf)) {
		vm->SP--;
		OBJECT_SET_NIL(vm->SP);
	} else {
		vm->SP--;
		OBJECT_SET_INTEGER(vm->SP, buf.st_size);
	}
#else// nilを返す
	vm->SP--;
	OBJECT_SET_NIL(vm->SP);
#endif
	return VM_OK;
}

VM_RET OPERATION_CALL op_error(tINT anum, tPVM vm)
{
	tPCELL list;
	tOBJECT continuable;
	if (anum<1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (anum==1) {
		if (vm_push(vm, &nil)) return VM_ERROR;
		list=0;
	} else {
		tINT i;
		tOBJECT tmp, tmp2;
		tmp=*vm->SP;
		if (cons_create_(vm, &list, &tmp, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(vm->SP, list);
		for (i=2; i<anum; i++) {
			tmp=*(vm->SP-1);
			tmp2=*(vm->SP);
			if (cons_create_(vm, &list, &tmp, &tmp2)) return VM_ERROR;
			vm->SP--;
			OBJECT_SET_CONS(vm->SP, list);
		}
	}
	if (!OBJECT_IS_STRING(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	OBJECT_SET_NIL(&continuable);
	return signal_simple_error_(vm, OBJECT_GET_CELL(vm->SP-1), list, &continuable);
}

VM_RET OPERATION_CALL op_cerror(tINT anum, tPVM vm)
{
	tPCELL list;
	tPOBJECT sp=vm->SP;
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (anum==2) {
		if (vm_push(vm, &nil)) return VM_ERROR;
		list=0;
	} else {
		tINT i;
		tOBJECT tmp, tmp2;
		tmp=*vm->SP;
		if (cons_create_(vm, &list, &tmp, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(vm->SP, list);
		for (i=3; i<anum; i++) {
			tmp=*(vm->SP-1);
			tmp2=*(vm->SP);
			if (cons_create_(vm, &list, &tmp, &tmp2)) return VM_ERROR;
			vm->SP--;
			OBJECT_SET_CONS(vm->SP, list);
		}
	}
	if (!OBJECT_IS_STRING(vm->SP-2)) {
		tOBJECT tmp=*(vm->SP-2);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if (!OBJECT_IS_STRING(vm->SP-1)) {
		tOBJECT tmp=*(vm->SP-1);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	{
		tOBJECT tmp=*(vm->SP-1);
		if (signal_simple_error_(vm, OBJECT_GET_CELL(vm->SP-2), list, &tmp)) {
			// 例外のまま
			return VM_ERROR;
		} else {
			// continue-conditionで戻ってきた
			vm->SP=sp-anum+1;
			vm_get_throw_object(vm, vm->SP);
			vm_clear_throw_object(vm);
			return VM_OK;
		}
	}
}

VM_RET OPERATION_CALL op_signal_condition(tINT anum, tPVM vm)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR);
	{
		tPOBJECT sp=vm->SP;
		tOBJECT tmp;
		if (!OBJECT_IS_CONDITION(vm->SP-1)) {
			tmp=*(vm->SP-1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SERIOUS_CONDITION, &tmp);
		}
		if (OBJECT_IS_SYMBOL(vm->SP)&&
			(OBJECT_GET_CELL(vm->SP)==SYMBOL_T))
			OBJECT_SET_STRING(vm->SP, string_continue_condition);
		tmp=*vm->SP;
		if (signal_condition_(vm, OBJECT_GET_CELL(vm->SP-1), &tmp)) {
			// 異常終了した
			return VM_ERROR;
		} else {
			// 正常終了した
			vm->SP=sp-1;
			*vm->SP=*sp;
			return VM_OK;
		}
	}
}

VM_RET OPERATION_CALL op_condition_continuable(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (OBJECT_IS_CONDITION(vm->SP)&&
		condition_is_continuable(OBJECT_GET_CELL(vm->SP)))
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	else
		OBJECT_SET_NIL(vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_arithmetic_error_operation(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tINT id=condition_get_class_id(OBJECT_GET_CELL(vm->SP));
		if ((id&CLASS_ARITHMETIC_ERROR)!=CLASS_ARITHMETIC_ERROR) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_ARITHMETIC_ERROR, &tmp);
		}
		arithmetic_error_get_operation(OBJECT_GET_CELL(vm->SP), vm->SP);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_arithmetic_error_operand(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tINT id=condition_get_class_id(OBJECT_GET_CELL(vm->SP));
		if ((id&CLASS_ARITHMETIC_ERROR)!=CLASS_ARITHMETIC_ERROR) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_ARITHMETIC_ERROR, &tmp);
		}
		arithmetic_error_get_operands(OBJECT_GET_CELL(vm->SP), vm->SP);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_domain_error_object(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_DOMAIN_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_DOMAIN_ERROR, &tmp);
	}
	domain_error_get_object(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_domain_error_expected_class(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_DOMAIN_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_DOMAIN_ERROR, &tmp);
	}
	domain_error_get_expected_class(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_parse_error_string(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_PARSE_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_PARSE_ERROR, &tmp);
	}
	parse_error_get_string(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_parse_error_expected_class(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_PARSE_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_PARSE_ERROR, &tmp);
	}
	parse_error_get_expected_class(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_simple_error_format_string(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_SIMPLE_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SIMPLE_ERROR, &tmp);
	}
	simple_error_get_format_string(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_simple_error_format_arguments(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_SIMPLE_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_SIMPLE_ERROR, &tmp);
	}
	simple_error_get_format_argument(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_stream_error_stream(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (condition_get_class_id(OBJECT_GET_CELL(vm->SP))!=CLASS_STREAM_ERROR) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STREAM_ERROR, &tmp);
	}
	stream_error_get_stream(OBJECT_GET_CELL(vm->SP), vm->SP);
	return VM_OK;
}

VM_RET OPERATION_CALL op_undefined_entity_name(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tINT id;
		id=condition_get_class_id(OBJECT_GET_CELL(vm->SP));
		if ((id&CLASS_UNDEFINED_ENTITY)!=CLASS_UNDEFINED_ENTITY) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_UNDEFINED_ENTITY, &tmp);
		}
		undefined_entity_get_name(OBJECT_GET_CELL(vm->SP), vm->SP);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_undefined_entity_namespace(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	{
		tINT id;
		id=condition_get_class_id(OBJECT_GET_CELL(vm->SP));
		if ((id&CLASS_UNDEFINED_ENTITY)!=CLASS_UNDEFINED_ENTITY) {
			tOBJECT tmp=*(vm->SP);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_UNDEFINED_ENTITY, &tmp);
		}
		undefined_entity_get_namespace(OBJECT_GET_CELL(vm->SP), vm->SP);
		return VM_OK;
	}
}

VM_RET OPERATION_CALL op_identity(tINT anum, tPVM vm)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_universal_time(tINT anum, tPVM vm)
{
	time_t t;
	const tINT d90_97 = 0;// 70*365*24*6*6ぐらい？ 32bit符号付き整数だとオーバーフロー
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	vm->SP++;
	OBJECT_SET_INTEGER(vm->SP, (tINT)time(&t)+d90_97);
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_internal_run_time(tINT anum, tPVM vm)
{
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	vm->SP++;
	OBJECT_SET_INTEGER(vm->SP, (tINT)clock());
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_internal_real_time(tINT anum, tPVM vm)
{
	time_t t;
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	vm->SP++;
	OBJECT_SET_INTEGER(vm->SP, (tINT)time(&t));
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_internal_time_units_per_second(tINT anum, tPVM vm)
{
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	vm->SP++;
	OBJECT_SET_INTEGER(vm->SP, CLOCKS_PER_SEC);
	return VM_OK;
}

VM_RET OPERATION_CALL op_system(tINT anum, tPVM vm)
{
	tINT i;
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	i=system(string_get_string(OBJECT_GET_CELL(vm->SP)));
	OBJECT_SET_INTEGER(vm->SP, i);
	return VM_OK;
}

VM_RET OPERATION_CALL op_exit(tINT anum, tPVM vm)
{
	exit(1);
	return VM_OK;
}

VM_RET OPERATION_CALL op_strftime(tINT anum, tPVM vm)
{
	time_t lt;
	struct tm* now;
	tPCELL string;
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	now=localtime(&lt);
	if (!OBJECT_IS_STRING(vm->SP)) {
		tOBJECT tmp=*(vm->SP);
		return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_STRING, &tmp);
	}
	if (vm_strftime_to_string(vm, string_get_string(OBJECT_GET_CELL(vm->SP)), now, &string)) return VM_ERROR;
	OBJECT_SET_STRING(vm->SP, string);
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_argument(tINT anum, tPVM vm)
{
	tPTISL tisl;
	if (anum!=0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	vm->SP++;
	tisl=vm_get_tisl(vm);
	if (tisl_get_argument(tisl)) {
		OBJECT_SET_CONS(vm->SP, tisl_get_argument(tisl));
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_get_environment(tINT anum, tPVM vm)
{
	tPTISL tisl;
	if (anum!=0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	vm->SP++;
	tisl=vm_get_tisl(vm);
	if (tisl_get_environment(tisl)) {
		OBJECT_SET_CONS(vm->SP, tisl_get_environment(tisl));
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_arity_error(tPVM vm)
{
	return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
}

VM_RET OPERATION_CALL op_eval(tINT anum, tPVM vm)
{
	tOBJECT form, ret;
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	form=*vm->SP;
	if (vm_evaluate_top_form(vm, &form, &ret)) return VM_ERROR;
	*vm->SP=ret;
	return VM_OK;
}

VM_RET OPERATION_CALL op_number_equal_stack_integer(tINT i, tINT offset, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		if (OBJECT_GET_INTEGER(vm->SP+offset)==i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP+offset)==i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_equal_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset1)) {
	case OBJECT_INTEGER:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)==OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)==OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	case OBJECT_FLOAT:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)==OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)==OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_not_equal_stack_integer(tINT i, tINT offset, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		if (OBJECT_GET_INTEGER(vm->SP+offset)!=i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP+offset)!=i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_not_equal_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset1)) {
	case OBJECT_INTEGER:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)!=OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)!=OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	case OBJECT_FLOAT:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)!=OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)!=OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_less_stack_integer(tINT i, tINT offset, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		if (OBJECT_GET_INTEGER(vm->SP+offset)<i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP+offset)<i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_less_integer_stack(tINT offset, tINT i, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		if (i<OBJECT_GET_INTEGER(vm->SP+offset)) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (i<OBJECT_GET_FLOAT(vm->SP+offset)) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_less_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset1)) {
	case OBJECT_INTEGER:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)<OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)<OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	case OBJECT_FLOAT:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)<OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)<OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_le_stack_integer(tINT i, tINT offset, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		if (OBJECT_GET_INTEGER(vm->SP+offset)<=i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (OBJECT_GET_FLOAT(vm->SP+offset)<=i) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_le_integer_stack(tINT offset, tINT i, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		if (i<=OBJECT_GET_INTEGER(vm->SP+offset)) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	case OBJECT_FLOAT:
		if (i<=OBJECT_GET_FLOAT(vm->SP+offset)) {
			vm->SP++;
			OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
		} else {
			vm->SP++;
			OBJECT_SET_NIL(vm->SP);
		}
		return VM_OK;
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_number_le_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset1)) {
	case OBJECT_INTEGER:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)<=OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_INTEGER(vm->SP+offset1)<=OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	case OBJECT_FLOAT:
		{
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)<=OBJECT_GET_INTEGER(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			case OBJECT_FLOAT:
				if (OBJECT_GET_FLOAT(vm->SP+offset1)<=OBJECT_GET_FLOAT(vm->SP+offset2)) {
					vm->SP++;
					OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
				} else {
					vm->SP++;
					OBJECT_SET_NIL(vm->SP);
				}
				return VM_OK;
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_addition_stack_integer(tINT i, tINT offset, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		{
			tINT d=OBJECT_GET_INTEGER(vm->SP+offset);
			tFLOAT f=(tFLOAT)d+i;
			vm->SP++;
			if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
				OBJECT_SET_FLOAT(vm->SP, f);
			} else {
				OBJECT_SET_INTEGER(vm->SP, d+i);
			}
			return VM_OK;
		}
	case OBJECT_FLOAT:
		{
			tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset);
			vm->SP++;
			OBJECT_SET_FLOAT(vm->SP, f+i);
			return VM_OK;
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_addition_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset1)) {
	case OBJECT_INTEGER:
		{
			tINT i=OBJECT_GET_INTEGER(vm->SP+offset1);
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				{
					tINT d=OBJECT_GET_INTEGER(vm->SP+offset2);
					tFLOAT f=(tFLOAT)i+d;
					vm->SP++;
					if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
						OBJECT_SET_FLOAT(vm->SP, f);
					} else {
						OBJECT_SET_INTEGER(vm->SP, i+d);
					}
					return VM_OK;
				}
			case OBJECT_FLOAT:
				{
					tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset2);
					vm->SP++;
					OBJECT_SET_FLOAT(vm->SP, i+f);
					return VM_OK;
				}
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	case OBJECT_FLOAT:
		{
			tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset1);
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				{
					tINT i=OBJECT_GET_INTEGER(vm->SP+offset2);
					vm->SP++;
					OBJECT_SET_FLOAT(vm->SP, f+i);
					return VM_OK;
				}
			case OBJECT_FLOAT:
				{
					tFLOAT f2=OBJECT_GET_FLOAT(vm->SP+offset2);
					vm->SP++;
					OBJECT_SET_FLOAT(vm->SP, f+f2);
					return VM_OK;
				}
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_substraction_stack_integer(tINT i, tINT offset, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		{
			tINT d=OBJECT_GET_INTEGER(vm->SP+offset);
			tFLOAT f=(tFLOAT)d-i;
			vm->SP++;
			if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
				OBJECT_SET_FLOAT(vm->SP, f);
			} else {
				OBJECT_SET_INTEGER(vm->SP, d-i);
			}
			return VM_OK;
		}
	case OBJECT_FLOAT:
		{
			tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset);
			vm->SP++;
			OBJECT_SET_FLOAT(vm->SP, f-i);
			return VM_OK;
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_substraction_integer_stack(tINT offset, tINT i, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset)) {
	case OBJECT_INTEGER:
		{
			tINT d=OBJECT_GET_INTEGER(vm->SP+offset);
			tFLOAT f=(tFLOAT)i-d;
			vm->SP++;
			if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
				OBJECT_SET_FLOAT(vm->SP, f);
			} else {
				OBJECT_SET_INTEGER(vm->SP, i-d);
			}
			return VM_OK;
		}
	case OBJECT_FLOAT:
		{
			tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset);
			vm->SP++;
			OBJECT_SET_FLOAT(vm->SP, i-f);
			return VM_OK;
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_substraction_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	switch (OBJECT_GET_TYPE(vm->SP+offset1)) {
	case OBJECT_INTEGER:
		{
			tINT i=OBJECT_GET_INTEGER(vm->SP+offset1);
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				{
					tINT d=OBJECT_GET_INTEGER(vm->SP+offset2);
					tFLOAT f=(tFLOAT)i-d;
					vm->SP++;
					if ((f>TISL_MOST_POSITIVE_INTEGER)||(f<TISL_MOST_NEGATIVE_INTEGER)) {
						OBJECT_SET_FLOAT(vm->SP, f);
					} else {
						OBJECT_SET_INTEGER(vm->SP, i-d);
					}
					return VM_OK;
				}
			case OBJECT_FLOAT:
				{
					tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset2);
					vm->SP++;
					OBJECT_SET_FLOAT(vm->SP, i-f);
					return VM_OK;
				}
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	case OBJECT_FLOAT:
		{
			tFLOAT f=OBJECT_GET_FLOAT(vm->SP+offset1);
			switch (OBJECT_GET_TYPE(vm->SP+offset2)) {
			case OBJECT_INTEGER:
				{
					tINT i=OBJECT_GET_INTEGER(vm->SP+offset2);
					vm->SP++;
					OBJECT_SET_FLOAT(vm->SP, f-i);
					return VM_OK;
				}
			case OBJECT_FLOAT:
				{
					tFLOAT f2=OBJECT_GET_FLOAT(vm->SP+offset2);
					vm->SP++;
					OBJECT_SET_FLOAT(vm->SP, f-f2);
					return VM_OK;
				}
			default:
				{
					tOBJECT tmp=*(vm->SP+offset2);
					return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
				}
			}
		}
	default:
		{
			tOBJECT tmp=*(vm->SP+offset1);
			return signal_domain_error(vm, TISL_ERROR_DOMAIN_ERROR, CLASS_NUMBER, &tmp);
		}
	}
}

VM_RET OPERATION_CALL op_eq_stack_integer(tINT i, tINT offset, tPVM vm)
{
	tPOBJECT tmp=vm->SP+offset;
	vm->SP++;
	if (OBJECT_IS_INTEGER(tmp)&&(OBJECT_GET_INTEGER(tmp)==i)) {
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_eq_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	if (object_eql(vm->SP+offset1, vm->SP+offset2)) {
		vm->SP++;
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		vm->SP++;
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}

VM_RET OPERATION_CALL op_equal_stack_stack(tINT offset2, tINT offset1, tPVM vm)
{
	if (object_equal(vm->SP+offset1, vm->SP+offset2)) {
		vm->SP++;
		OBJECT_SET_SYMBOL(vm->SP, SYMBOL_T);
	} else {
		vm->SP++;
		OBJECT_SET_NIL(vm->SP);
	}
	return VM_OK;
}
