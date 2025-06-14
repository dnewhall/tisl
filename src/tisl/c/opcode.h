//
// TISL/src/tisl/c/opcode.h
// TISL Ver. 4.0
//

#ifndef TISL_OPCODE_H
#define TISL_OPCODE_H

VM_RET c_discard(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_nil(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_t(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_float(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_character(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_cons(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_string(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_symbol(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_vector(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_array(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_cell_object(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_heap(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_variable(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_rec(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_rec_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_bind(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_bind_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_local_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_local_stack_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_local_heap(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_local_heap_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_next_method_around(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_next_method_around_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_next_method_primary(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_call_next_method_primary_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_next_method_p_around(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_next_method_p_around_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_next_method_p_primary(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_next_method_p_primary_tail(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_ret(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_lambda_in(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_lambda_out(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_lambda_heap_in(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_lambda_heap_out(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_function(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_lfunction(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_lambda(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_labels_in(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_labels_out(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_flet_in(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_flet_out(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_and(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_or(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_heap(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_variable(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_dynamic(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_aref(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_garef(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_elt(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_property(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_car(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_cdr(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_accessor(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_push_dynamic(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_dynamic_let(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_if(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_case(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_case_using(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_while(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_for_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_for_heap(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_block(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_return_from(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_catch(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_throw(tPVM vm, tPCELL functin, tINT* pc);
VM_RET c_tagbody(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_go(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_unwind_protect(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_class(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_the(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_assure(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_convert(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_standard_input(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_standard_output(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_error_output(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_open_input_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_open_output_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_open_io_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_ignore_errors(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_continue_condition(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_with_handler(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_time(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_quasiquote(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_quasiquote2(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_unquote(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_unquote_splicing(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_unquote_splicing2(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_functionp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_apply(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_funcall(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_eq(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_eql(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_not(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_generic_function_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_class_of(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_instancep(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_subclassp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_symbolp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_property(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_remove_property(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_gensym(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_numberp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_parse_number(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_not_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_ge(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_le(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_greater(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_less(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_addition(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_multiplication(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_substraction(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_quotient(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_reciprocal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_max(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_min(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_abs(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_exp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_log(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_expt(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_sqrt(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_sin(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_cos(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_tan(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_atan(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_atan2(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_sinh(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_cosh(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_tanh(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_atanh(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_floatp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_float(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_floor(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_ceiling(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_truncate(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_round(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_integerp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_div(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_mod(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_gcd(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_lcm(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_isqrt(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_characterp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_not_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_less(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_greater(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_le(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_ge(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_consp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_cons(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_car(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_cdr(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_null(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_listp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_create_list(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_list(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_reverse(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_nreverse(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_append(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_member(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_mapcar(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_mapc(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_mapcan(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_maplist(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_mapl(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_mapcon(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_assoc(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_basic_array_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_basic_array_a_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_general_array_a_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_create_array(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_aref(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_garef(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_array_dimensions(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_basic_vector_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_general_vector_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_create_vector(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_vector(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_stringp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_create_string(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_not_equal(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_less(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_greater(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_ge(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_le(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_char_index(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_index(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_string_append(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_length(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_elt(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_subseq(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_map_into(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_streamp(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_open_stream_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_input_stream_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_output_stream_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_standard_input(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_standard_output(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_error_output(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_open_input_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_open_output_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_open_io_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_close(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_finish_output(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_create_string_input_stream(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_create_string_output_stream(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_output_stream_string(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_read(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_read_char(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_preview_char(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_read_line(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_stream_ready_p(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format_char(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format_float(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format_fresh_line(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format_object(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_format_tab(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_read_byte(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_write_byte(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_probe_file(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_file_position(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_set_file_position(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_file_length(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_error(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_cerror(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_signal_condition(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_condition_continuable(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_arithmetic_error_operation(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_arithmetic_error_operand(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_domain_error_object(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_domain_error_expected_class(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_parse_error_string(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_parse_error_expected_class(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_simple_error_format_string(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_simple_error_format_arguments(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_stream_error_stream(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_undefined_entity_name(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_undefined_entity_namespace(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_identity(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_universal_time(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_internal_run_time(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_internal_real_time(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_internal_time_units_per_second(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_system(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_exit(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_strftime(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_argument(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_get_environment(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_arity_error(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_eval(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_equal_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_equal_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_not_equal_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_not_equal_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_less_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_less_integer_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_less_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_le_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_le_integer_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_number_le_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_addition_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_addition_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_substraction_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_substraction_integer_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_substraction_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_eq_stack_integer(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_eq_stack_stack(tPVM vm, tPCELL function, tINT* pc);
VM_RET c_equal_stack_stack(tPVM vm, tPCELL function, tINT* pc);

#endif