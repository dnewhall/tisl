//
// TISL/src/tisl/gc.h
// TISL Ver 4.x
//

#ifndef TISL_GC_H
#define TISL_GC_H

// garbage collector�������Ƚ����
tBOOL gc_create(tPVM vm, tPGC* gc, tUINT heap_size);
// garbage collector�λ��Ѥ��Ƥ������γ���
void free_gc(tPGC gc);

VM_RET gc_push(tPVM vm, tPOBJECT obj);

//
tUINT allocate_cell(tPVM vm, tUINT size, tPCELL *cell);
//
VM_RET allocate_string_buffer(tPVM vm, tPBUFFER* buffer);
VM_RET free_string_buffer(tPVM vm, tPBUFFER buffer);

#endif
