//
// TISL/src/tisl/writer.h
// TISL Ver 4.x
// 

#ifndef TISL_WRITER_H
#define TISL_WRITER_H

VM_RET format_char(tPVM vm, tPCELL stream, const tCHAR c);
VM_RET format_float(tPVM vm, tPCELL stream, const tFLOAT f);
VM_RET format_fresh_line(tPVM vm, tPCELL stream);
VM_RET format_l(tPVM vm, tPCELL stream, tCSTRING string, tPCELL list);
VM_RET format_integer(tPVM vm, tPCELL stream, const tINT i, const tINT r);
VM_RET format_object(tPVM vm, tPCELL stream, tPOBJECT obj);
VM_RET format_tab(tPVM vm, tPCELL stream, const tINT n);

VM_RET format_elapsed_time(tPVM vm, tPCELL stream, const tFLOAT f);
VM_RET format_current_package(tPVM vm, tPCELL stream);

//

VM_RET write_char(tPVM vm, tPCELL stream, const tCHAR c);
VM_RET write_string(tPVM vm, tPCELL stream, tCSTRING string);
VM_RET write_integer(tPVM vm, tPCELL stream, const tINT i, const tINT r);
VM_RET write_float(tPVM vm, tPCELL stream, const tFLOAT f);
VM_RET write_object(tPVM vm, tPCELL stream, tPOBJECT obj);

#endif
