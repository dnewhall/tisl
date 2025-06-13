//
// TISL/src/primitive_operation.c
// TISL Ver. 4.x
//

#define TISL_VM_STRUCT
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "built_in_object.h"
#include "reader.h"
#include "operation.h"

VM_RET po_functionp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_functionp(vm);
}

VM_RET po_apply(tPVM vm, const tINT anum)
{
	return op_apply(anum, vm);
}

VM_RET po_funcall(tPVM vm, const tINT anum)
{
	return op_funcall(anum, vm);
}

VM_RET po_eq(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_eq(vm);
}

VM_RET po_eql(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_eql(vm);
}

VM_RET po_equal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_equal(vm);
}

VM_RET po_not(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_not(vm);
}

VM_RET po_generic_function_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_generic_function_p(vm);
}

VM_RET po_class_of(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_class_of(vm);
}

VM_RET po_instancep(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_instancep(vm);
}

VM_RET po_subclassp(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_subclassp(vm);
}

VM_RET po_symbolp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_symbolp(vm);
}

VM_RET po_property(tPVM vm, const tINT anum)
{
	return op_property(anum, vm);
}

VM_RET po_set_property(tPVM vm, const tINT anum)
{
	if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_property(vm);
}

VM_RET po_remove_property(tPVM vm, const tINT anum)
{
	return op_remove_property(anum, vm);
}

VM_RET po_gensym(tPVM vm, const tINT anum)
{
	if (anum!=0) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_gensym(vm);
}

VM_RET po_numberp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_numberp(vm);
}

VM_RET po_parse_number(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_parse_number(vm);
}

VM_RET po_number_equal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_number_equal(vm);
}

VM_RET po_number_not_equal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_number_not_equal(vm);
}

VM_RET po_number_ge(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_number_ge(vm);
}

VM_RET po_number_le(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_number_le(vm);
}

VM_RET po_number_greater(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_number_greater(vm);
}

VM_RET po_number_less(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_number_less(vm);
}

VM_RET po_addition(tPVM vm, const tINT anum)
{
	return op_addition(anum, vm);
}

VM_RET po_multiplication(tPVM vm, const tINT anum)
{
	return op_multiplication(anum, vm);
}

VM_RET po_subtraction(tPVM vm, const tINT anum)
{
	return op_substraction(anum, vm);
}

VM_RET po_quotient(tPVM vm, const tINT anum)
{
	return op_quotient(anum, vm);
}

VM_RET po_reciprocal(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_reciprocal(vm);
}

VM_RET po_max(tPVM vm, const tINT anum)
{
	return op_max(anum, vm);
}

VM_RET po_min(tPVM vm, const tINT anum)
{
	return op_min(anum, vm);
}

VM_RET po_abs(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_abs(vm);
}

VM_RET po_exp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_exp(vm);
}

VM_RET po_log(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_log(vm);
}

VM_RET po_expt(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_expt(vm);
}

VM_RET po_sqrt(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_sqrt(vm);
}

VM_RET po_sin(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_sin(vm);
}

VM_RET po_cos(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_cos(vm);
}

VM_RET po_tan(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_tan(vm);
}

VM_RET po_atan(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_atan(vm);
}

VM_RET po_atan2(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_atan2(vm);
}

VM_RET po_sinh(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_sinh(vm);
}

VM_RET po_cosh(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_cosh(vm);
}

VM_RET po_tanh(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_tanh(vm);
}

VM_RET po_atanh(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_atanh(vm);
}

VM_RET po_floatp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_floatp(vm);
}

VM_RET po_float(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_float(vm);
}

VM_RET po_floor(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_floor(vm);
}

VM_RET po_ceiling(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_ceiling(vm);
}

VM_RET po_truncate(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_truncate(vm);
}

VM_RET po_round(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_round(vm);
}

VM_RET po_integerp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_integerp(vm);
}

VM_RET po_div(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_div(vm);
}

VM_RET po_mod(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_mod(vm);
}

VM_RET po_gcd(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_gcd(vm);
}

VM_RET po_lcm(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_lcm(vm);
}

VM_RET po_isqrt(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_isqrt(vm);
}

VM_RET po_characterp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_characterp(vm);
}

VM_RET po_char_euqal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_char_equal(vm);
}

VM_RET po_char_not_equal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_char_not_equal(vm);
}

VM_RET po_char_less(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_char_less(vm);
}

VM_RET po_char_greater(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_char_greater(vm);
}

VM_RET po_char_le(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_char_le(vm);
}

VM_RET po_char_ge(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_char_ge(vm);
}

VM_RET po_consp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_consp(vm);
}

VM_RET po_cons(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_cons(vm);
}

VM_RET po_car(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_car(vm);
}

VM_RET po_cdr(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_cdr(vm);
}

VM_RET po_set_car(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_car(vm);
}

VM_RET po_set_cdr(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_cdr(vm);
}

VM_RET po_null(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_null(vm);
}

VM_RET po_listp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_listp(vm);
}

VM_RET po_create_list(tPVM vm, const tINT anum)
{
	return op_create_list(anum, vm);
}

VM_RET po_list(tPVM vm, const tINT anum)
{
	return op_list(anum, vm);
}

VM_RET po_reverse(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_reverse(vm);
}

VM_RET po_nreverse(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_nreverse(vm);
}

VM_RET po_append(tPVM vm, const tINT anum)
{
	return op_append(anum, vm);
}

VM_RET po_member(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_member(vm);
}

VM_RET po_mapcar(tPVM vm, const tINT anum)
{
	return op_mapcar(anum, vm);
}

VM_RET po_mapc(tPVM vm, const tINT anum)
{
	return op_mapc(anum, vm);
}

VM_RET po_maplist(tPVM vm, const tINT anum)
{
	return op_maplist(anum, vm);
}

VM_RET po_mapl(tPVM vm, const tINT anum)
{
	return op_mapl(anum, vm);
}

VM_RET po_mapcan(tPVM vm, const tINT anum)
{
	return op_mapcan(anum, vm);
}

VM_RET po_mapcon(tPVM vm, const tINT anum)
{
	return op_mapcon(anum, vm);
}

VM_RET po_assoc(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_assoc(vm);
}

VM_RET po_basic_array_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_basic_array_p(vm);
}

VM_RET po_basic_array_a_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_basic_array_a_p(vm);
}

VM_RET po_general_array_a_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_general_array_a_p(vm);
}

VM_RET po_create_array(tPVM vm, const tINT anum)
{
	return op_create_array(anum, vm);
}

VM_RET po_aref(tPVM vm, const tINT anum)
{
	return op_aref(anum, vm);
}

VM_RET po_garef(tPVM vm, const tINT anum)
{
	return op_garef(anum, vm);
}

VM_RET po_set_aref(tPVM vm, const tINT anum)
{
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_aref(anum, vm);
}

VM_RET po_set_garef(tPVM vm, const tINT anum)
{
	if (anum<2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_garef(anum, vm);
}

VM_RET po_array_dimensions(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_array_dimensions(vm);
}

VM_RET po_basic_vector_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_basic_vector_p(vm);
}

VM_RET po_general_vector_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_general_vector_p(vm);
}

VM_RET po_create_vector(tPVM vm, const tINT anum)
{
	return op_create_vector(anum, vm);
}

VM_RET po_vector(tPVM vm, const tINT anum)
{
	return op_vector(anum, vm);
}

VM_RET po_stringp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_stringp(vm);
}

VM_RET po_create_string(tPVM vm, const tINT anum)
{
	return op_create_string(anum, vm);
}

VM_RET po_string_equal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_string_equal(vm);
}

VM_RET po_string_not_equal(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_string_not_equal(vm);
}

VM_RET po_string_less(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_string_less(vm);
}

VM_RET po_string_greater(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_string_greater(vm);
}

VM_RET po_string_ge(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_string_ge(vm);
}

VM_RET po_string_le(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_string_le(vm);
}

VM_RET po_char_index(tPVM vm, const tINT anum)
{
	return op_char_index(anum, vm);
}

VM_RET po_string_index(tPVM vm, const tINT anum)
{
	return op_string_index(anum, vm);
}

VM_RET po_string_append(tPVM vm, const tINT anum)
{
	return op_string_append(anum, vm);
}

VM_RET po_length(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_length(vm);
}

VM_RET po_elt(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_elt(vm);
}

VM_RET po_set_elt(tPVM vm, const tINT anum)
{
	if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_elt(vm);
}

VM_RET po_subseq(tPVM vm, const tINT anum)
{
	if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_subseq(vm);
}

VM_RET po_map_into(tPVM vm, const tINT anum)
{
	return op_map_into(anum, vm);
}

VM_RET po_streamp(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_streamp(vm);
}

VM_RET po_open_stream_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_open_stream_p(vm);
}

VM_RET po_input_stream_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_input_stream_p(vm);
}

VM_RET po_output_stream_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_output_stream_p(vm);
}

VM_RET po_standard_input(tPVM vm, const tINT anum)
{
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_standard_input(vm);
}

VM_RET po_standard_output(tPVM vm, const tINT anum)
{
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_standard_output(vm);
}

VM_RET po_error_output(tPVM vm, const tINT anum)
{
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_error_output(vm);
}

VM_RET po_open_input_file(tPVM vm, const tINT anum)
{
	return op_open_input_file(anum, vm);
}

VM_RET po_open_output_file(tPVM vm, const tINT anum)
{
	return op_open_output_file(anum, vm);
}

VM_RET po_open_io_file(tPVM vm, const tINT anum)
{
	return op_open_io_file(anum, vm);
}

VM_RET po_close(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_close(vm);
}

VM_RET po_finish_output(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_finish_output(vm);
}

VM_RET po_create_string_input_stream(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_create_string_input_stream(vm);
}

VM_RET po_create_string_output_stream(tPVM vm, const tINT anum)
{
	if (anum) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_create_string_output_stream(vm);
}

VM_RET po_get_output_stream_string(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_get_output_stream_string(vm);
}

VM_RET po_read(tPVM vm, const tINT anum)
{
	return op_read(anum, vm);
}

VM_RET po_read_char(tPVM vm, const tINT anum)
{
	return op_read_char(anum, vm);
}

VM_RET po_preview_char(tPVM vm, const tINT anum)
{
	return op_preview_char(anum, vm);
}

VM_RET po_read_line(tPVM vm, const tINT anum)
{
	return op_read_line(anum, vm);
}

VM_RET po_stream_ready_p(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_stream_ready_p(vm);
}

VM_RET po_format(tPVM vm, const tINT anum)
{
	return op_format(anum, vm);
}

VM_RET po_format_char(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_format_char(vm);
}

VM_RET po_format_float(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_format_float(vm);
}

VM_RET po_format_fresh_line(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_format_fresh_line(vm);
}

VM_RET po_format_integer(tPVM vm, const tINT anum)
{
	if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_format_integer(vm);
}

VM_RET po_format_object(tPVM vm, const tINT anum)
{
	if (anum!=3) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_format_object(vm);
}

VM_RET po_format_tab(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_format_tab(vm);
}

VM_RET po_read_byte(tPVM vm, const tINT anum)
{
	return op_read_byte(anum, vm);
}

VM_RET po_write_byte(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_write_byte(vm);
}

VM_RET po_probe_file(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_probe_file(vm);
}

VM_RET po_file_position(tPVM vm, const tINT anum)
{
	if (anum!=1) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_file_position(vm);
}

VM_RET po_set_file_position(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_set_file_position(vm);
}

VM_RET po_file_length(tPVM vm, const tINT anum)
{
	if (anum!=2) return signal_condition(vm, TISL_ERROR_ARITY_ERROR);
	return op_file_length(vm);
}

VM_RET po_error(tPVM vm, const tINT anum)
{
	return op_error(anum, vm);
}

VM_RET po_cerror(tPVM vm, const tINT anum)
{
	return op_cerror(anum, vm);
}

VM_RET po_signal_condition(tPVM vm, const tINT anum)
{
	return op_signal_condition(anum, vm);
}

VM_RET po_condition_continuable(tPVM vm, const tINT anum)
{
	return op_condition_continuable(anum, vm);
}

VM_RET po_arithmetic_error_operation(tPVM vm, const tINT anum)
{
	return op_arithmetic_error_operation(anum, vm);
}

VM_RET po_arithmetic_error_operand(tPVM vm, const tINT anum)
{
	return op_arithmetic_error_operand(anum, vm);
}

VM_RET po_domain_error_object(tPVM vm, const tINT anum)
{
	return op_domain_error_object(anum, vm);
}

VM_RET po_domain_error_expected_class(tPVM vm, const tINT anum)
{
	return op_domain_error_expected_class(anum, vm);
}

VM_RET po_parse_error_string(tPVM vm, const tINT anum)
{
	return op_parse_error_string(anum, vm);
}

VM_RET po_parse_error_expected_class(tPVM vm, const tINT anum)
{
	return op_parse_error_expected_class(anum, vm);
}

VM_RET po_simple_error_format_string(tPVM vm, const tINT anum)
{
	return op_simple_error_format_string(anum, vm);
}

VM_RET po_simple_error_format_arguments(tPVM vm, const tINT anum)
{
	return op_simple_error_format_arguments(anum, vm);
}

VM_RET po_stream_error_stream(tPVM vm, const tINT anum)
{
	return op_stream_error_stream(anum, vm);
}

VM_RET po_undefined_entity_name(tPVM vm, const tINT anum)
{
	return op_undefined_entity_name(anum, vm);
}

VM_RET po_undefined_entity_namespace(tPVM vm, const tINT anum)
{
	return op_undefined_entity_namespace(anum, vm);
}

VM_RET po_identity(tPVM vm, const tINT anum)
{
	return op_identity(anum, vm);
}

VM_RET po_get_universal_time(tPVM vm, const tINT anum)
{
	return op_get_universal_time(anum, vm);
}

VM_RET po_get_internal_run_time(tPVM vm, const tINT anum)
{
	return op_get_internal_run_time(anum, vm);
}

VM_RET po_get_internal_real_time(tPVM vm, const tINT anum)
{
	return op_get_internal_real_time(anum, vm);
}

VM_RET po_get_internal_time_units_per_second(tPVM vm, const tINT anum)
{
	return op_get_internal_time_units_per_second(anum, vm);
}

VM_RET po_system(tPVM vm, const tINT anum)
{
	return op_system(anum, vm);
}

VM_RET po_exit(tPVM vm, const tINT anum)
{
	return op_exit(anum, vm);
}

VM_RET po_strftime(tPVM vm, const tINT anum)
{
	return op_strftime(anum, vm);
}

VM_RET po_get_argument(tPVM vm, const tINT anum)
{
	return op_get_argument(anum, vm);
}

VM_RET po_get_environment(tPVM vm, const tINT anum)
{
	return op_get_environment(anum, vm);
}

VM_RET po_eval(tPVM vm, const tINT anum)
{
	return op_eval(anum, vm);
}