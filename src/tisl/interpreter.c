//
// TISL/src/tisl/interpreter.c
// TISL Ver 4.x
//

#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "reader.h"
#include "writer.h"
#include "built_in_object.h"

TISL_IMPORT_OR_EXPORT void TISLCALL tisl_interpreter_top_loop(TNI* tni)
{
	tOBJECT obj, ret;
	tPVM vm=tni_get_vm(tni);

LOOP:
	vm_clear(vm);
	if (format_current_package(vm, vm_get_standard_output(vm))) goto ERROR;
	if (format_l(vm, vm_get_standard_output(vm), " > ", 0)) goto ERROR;
	if (read_form(vm, vm_get_standard_input(vm), &obj)) goto ERROR;
	if (vm_evaluate_top_form(vm, &obj, &ret)) goto ERROR;
	if (format_object(vm, vm_get_standard_output(vm), &ret)) goto ERROR;
	if (format_fresh_line(vm, vm_get_standard_output(vm))) goto ERROR;
	goto LOOP;
ERROR:
	if (vm_last_condition_is_user_interrupt(vm)) {
		if (format_l(vm, vm_get_standard_output(vm), "; user-interrupt~%", 0)) return;
		tisl_reset_user_interrupt_flag();
	} else {
		tPCELL cell;
		vm_get_last_condition(vm, &obj);
		if (format_object(vm, vm_get_standard_output(vm), &obj)) return;
		if (format_fresh_line(vm, vm_get_standard_output(vm))) return;
		// 一行捨てる
		if (read_line(vm, vm_get_standard_input(vm), &cell)) return ;
	}
	goto LOOP;
}

TISL_IMPORT_OR_EXPORT void TISLCALL tisl_interpreter_file(TNI* tni, const char* file_name)
{
	tOBJECT obj, ret;
	tPCELL stream, name;
	tPVM vm=tni_get_vm(tni);
	tPTISL tisl=vm_get_tisl(vm);

	if (tisl_get_string(tisl, vm, file_name, &name)) return;
	if (file_stream_create(vm, STREAM_INPUT, name, &stream)) return;
	cell_to_object(stream, &obj);
	if (vm_push_temp(vm, &obj)) return;// streamの保存に注意!!!

LOOP:
	vm_clear(vm);
	if (read_form(vm, stream, &obj)) goto ERROR;
	if (vm_evaluate_top_form(vm, &obj, &ret)) goto ERROR;
	goto LOOP;
ERROR:
	file_stream_close(vm, stream);
	// end-of-streamで終了する？
	vm_pop_temp(vm);
}

