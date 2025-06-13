//
// TISL/src/tisl/reader.h
// TISL Ver 4.x
//

#ifndef TISL_READER_H
#define TISL_READER_H

// VM_RET read_char(tPVM vm, tPCELL stream, tPCHAR c)
// vm上で，stream入力ストリームから1文字読込みを行う．
// 例外が発生した場合は，VM_ERRORを返す．
VM_RET read_char(tPVM vm, tPCELL stream, tPCHAR c);

// VM_RET preview_char(tPVM vm, tPCELL stream, tPCHAR c)
// vm上で，stream入力ストリームから1文字先読みを行う．
// 例外が発生した場合は，VM_ERRORを返す．
VM_RET preview_char(tPVM vm, tPCELL stream, tPCHAR c);

// VM_RET read_form(tPVM vm, tPCELL stream, tPOBJECT obj)
// vm上で stream入力ストリームから読込みを行い，
// ISLISPオブジェクトを作成し，objに返す．
// 例外等が発生した場合，VM_ERRORを返す．
VM_RET read_form(tPVM vm, tPCELL stream, tPOBJECT obj);
//
VM_RET read_line(tPVM vm, tPCELL stream, tPCELL* string);
// 
VM_RET eat_white(tPVM vm, tPCELL stream);

#endif
