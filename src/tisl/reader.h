//
// TISL/src/tisl/reader.h
// TISL Ver 4.x
//

#ifndef TISL_READER_H
#define TISL_READER_H

// VM_RET read_char(tPVM vm, tPCELL stream, tPCHAR c)
// vm��ǡ�stream���ϥ��ȥ꡼�फ��1ʸ���ɹ��ߤ�Ԥ���
// �㳰��ȯ���������ϡ�VM_ERROR���֤���
VM_RET read_char(tPVM vm, tPCELL stream, tPCHAR c);

// VM_RET preview_char(tPVM vm, tPCELL stream, tPCHAR c)
// vm��ǡ�stream���ϥ��ȥ꡼�फ��1ʸ�����ɤߤ�Ԥ���
// �㳰��ȯ���������ϡ�VM_ERROR���֤���
VM_RET preview_char(tPVM vm, tPCELL stream, tPCHAR c);

// VM_RET read_form(tPVM vm, tPCELL stream, tPOBJECT obj)
// vm��� stream���ϥ��ȥ꡼�फ���ɹ��ߤ�Ԥ���
// ISLISP���֥������Ȥ��������obj���֤���
// �㳰����ȯ��������硤VM_ERROR���֤���
VM_RET read_form(tPVM vm, tPCELL stream, tPOBJECT obj);
//
VM_RET read_line(tPVM vm, tPCELL stream, tPCELL* string);
// 
VM_RET eat_white(tPVM vm, tPCELL stream);

#endif
