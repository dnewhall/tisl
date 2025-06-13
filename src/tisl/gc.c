//
// TISL/src/tisl/gc.c
// TISL Ver 4.x
//

#include <malloc.h>

#define TISL_VM_STRUCT
#define TISL_GC_MARK_TABLE
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "gc.h"
#include "translator.h"
#include "writer.h"

extern tPGC vm_get_gc(tPVM vm);

///////////////////////////////////////

typedef struct tHIVE_		tHIVE,	*tPHIVE;

///////////////////////////////////////

struct tGC_ {
	tPVM	my_vm;
	//
	tINT	count;
	tBOOL	gc_mark;
	//
	tUINT	heap_unit;
	tUINT	heap_size;
	//
	tPCELL	free_list;
	tUINT	free_list_size;
	tUINT	free_list_number;

	tPHIVE		head;
	tPHIVE		last_hive;

	tPBUFFER	buffer_head;

	tINT		garbage_size;
};

static tPCELL gc_get_free_list(tPGC gc);
static void gc_set_free_list(tPGC gc, tPCELL list);
static tUINT gc_get_free_list_average(tPGC gc);
static void gc_reverse_mark(tPGC gc);
void gc_mark_cell(tPGC gc, tPCELL cell);
static void gc_unmark_cell(tPGC gc, tPCELL cell);
static tPCELL gc_get_next_cell(tPHIVE hive, tPCELL cell);
static tBOOL cell_is_garbage(tPGC gc, tPCELL cell);
static void gc_add_free_list(tPGC gc, tPCELL free_cell, const tUINT size);
static void gc_refresh(tPVM vm);

///////////////////////////////////////

struct tHIVE_ {
	tPCELL		cells;
	tINT		size;
	tPCELL		last;
	tPHIVE		next;
};

static tBOOL hive_create(tPHIVE* hive, const tUINT size, tPGC gc);
static tBOOL add_hive(tPGC gc, const tUINT size);
static void free_hive(tPHIVE hive);
static void hive_refresh(tPHIVE hive);

///////////////////////////////////////

extern void string_buffer_free(tPBUFFER buffer);

///////////////////////////////////////

// garbage collectorの生成と初期化
tBOOL gc_create(tPVM vm, tPGC* gc, tUINT heap_size)
{
	*gc=malloc(sizeof(tGC));
	if (!*gc) return tFALSE;

	(*gc)->my_vm=vm;

	(*gc)->count=0;
	(*gc)->gc_mark=tFALSE;

	(*gc)->free_list=0;
	(*gc)->free_list_number=0;
	(*gc)->free_list_size=0;

	(*gc)->heap_unit=heap_size;
	(*gc)->heap_size=0;

	(*gc)->head=0;
	if (!add_hive(*gc, 0)) return tFALSE;
	(*gc)->last_hive=(*gc)->head;


	(*gc)->buffer_head=0;
	(*gc)->garbage_size=0;

	return tTRUE;
}

static void gc_refresh(tPVM vm)
{
	tPGC gc=vm_get_gc(vm);

	gc->last_hive=gc->head;
	gc->last_hive->last=gc->last_hive->cells;
}

// garbage collectorの使用しているメモリの開放
void free_gc(tPGC gc)
{
	if (gc) {
		string_buffer_free(gc->buffer_head);
		free_hive(gc->head);
		free(gc);
	}
}

static tBOOL hive_create(tPHIVE* hive, const tUINT size, tPGC gc)
{
	*hive=malloc(sizeof(tHIVE));
	if (!*hive) return tFALSE;

	(*hive)->cells=malloc(sizeof(tCELL)*size);
	if (!(*hive)->cells) { free(*hive); return tFALSE; }
	
	(*hive)->size=size;
	(*hive)->last=(*hive)->cells;
	(*hive)->next=0;

	CELL_HEAD_CLEAR((*hive)->cells);
	free_initialize((*hive)->cells, size, 0);
	gc_unmark_cell(gc, (*hive)->cells);

	return tTRUE;
}

static void hive_refresh(tPHIVE hive)
{
	hive->last=hive->cells;
}

static void free_hive(tPHIVE hive)
{
	if (hive) {
		free(hive->cells);
		free(hive->next);
		free(hive);
	}
}

static tBOOL add_hive(tPGC gc, const tUINT size)
{
	tPHIVE hive;
	tUINT s;

	s=(size > gc->heap_unit) ? size : gc->heap_unit;
	if (!hive_create(&hive, s*CELL_UNIT, gc)) return tFALSE;
	if (gc->head) {
		tPHIVE p;
		for (p=gc->head; p->next; p=p->next);
		p->next=hive;
	} else {
		gc->head=hive;
	}
	gc->last_hive=hive;
	gc->heap_size+=s*CELL_UNIT;

#ifdef _DEBUG
	if (vm_get_standard_output(gc->my_vm)) {
		tPCELL stream;
		tPVM vm;
		vm=gc->my_vm;
		stream=vm_get_standard_output(vm);
		if (format_l(vm, stream, "add hive : total ", 0))return tFALSE;
		if (format_integer(vm, stream, (tINT)gc->heap_size*sizeof(tCELL), 10)) return tFALSE;
		if (format_l(vm, stream, "Byte : free-list (", 0)) return tFALSE;
		if (format_integer(vm, stream, gc->free_list_number, 10)) return tFALSE;
		if (format_l(vm, stream, " / ", 0)) return tFALSE;
		if (format_integer(vm, stream, gc->free_list_size*sizeof(tCELL), 10)) return tFALSE;
		if (format_l(vm, stream, "Byte )~%", 0)) return tFALSE;
	}
#endif

	return tTRUE;
}

///////////////////////////////////////

static tPCELL gc_get_free_list(tPGC gc)
{
	return gc->free_list;
}

static void gc_set_free_list(tPGC gc, tPCELL list)
{
	gc->free_list=list;
}

static tUINT gc_get_free_list_average(tPGC gc)
{
	return gc->free_list_number ? gc->free_list_size / gc->free_list_number : 0;
}

static void gc_mark_reverse(tPGC gc)
{
	gc->gc_mark = gc->gc_mark ? tFALSE : tTRUE;
}

void gc_mark_cell(tPGC gc, tPCELL cell)
{
	if (gc->gc_mark) {
		CELL_SET_GC_MARK(cell);
	} else {
		CELL_RESET_GC_MARK(cell);
	}
}

static void gc_unmark_cell(tPGC gc, tPCELL cell)
{
	if (gc->gc_mark) {
		CELL_RESET_GC_MARK(cell);
	} else {
		CELL_SET_GC_MARK(cell);
	}
}

tBOOL cell_is_marked(tPVM vm, tPCELL cell)
{
	if (vm_get_gc(vm)->gc_mark) {
		return CELL_GET_GC_MARK(cell) ? tTRUE : tFALSE;
	} else {
		return CELL_GET_GC_MARK(cell) ? tFALSE : tTRUE;
	}
}

static tBOOL cell_is_garbage(tPGC gc, tPCELL cell)
{
	if(gc->gc_mark) {
		return CELL_GET_GC_MARK(cell) ? tFALSE : tTRUE;
	} else {
		return CELL_GET_GC_MARK(cell) ? tTRUE : tFALSE;
	}
}

void vm_set_gc_mark(tPVM vm, const tBOOL mark)
{
	vm_get_gc(vm)->gc_mark=mark;
}

VM_RET cell_mark(tPVM vm, tPCELL cell)
{
	if (!cell) return VM_OK;
	if (cell_is_marked(vm, cell)) return VM_OK;
	{
		tOBJECT obj;
		gc_mark_cell(vm_get_gc(vm), cell);
		cell_to_object(cell, &obj);
		return vm_push(vm, &obj);
	}
}

VM_RET cell_mark_(tPVM vm, tPCELL cell)
{
	return (*cell_mark_table[CELL_GET_TYPE_INDEX(cell)])(vm, cell);
}

static tPCELL gc_get_next_cell(tPHIVE hive, tPCELL cell)
{
	cell+=cell_get_size(cell);
	if (cell-hive->cells==hive->size) {
		// 終わりまできた
		return 0;
	} else {
		return cell;
	}
}

///////////////////////////////////////

static tUINT allocate_cell_(tPVM vm, tUINT size, tPCELL* cell);
static tUINT allocate_cell_free_list(tPVM vm, tUINT size, tPCELL* cell);
static tUINT allocate_cell_sweep(tPVM vm, tUINT size, tPCELL* cell);
tBOOL garbage_collection(tPVM vm, tUINT size);

tUINT allocate_cell(tPVM vm, tUINT size, tPCELL *cell)
{
	tUINT s, r;
	tPGC gc=vm_get_gc(vm);
	tisl_gc_wait_2(vm_get_tisl(vm), vm);
	r=size%(sizeof(tCELL)*CELL_UNIT);
	s=size-r;
	s/=sizeof(tCELL);
	if (r) s+=CELL_UNIT;
	// 割当て
	if (allocate_cell_(vm, s, cell)) { gc->garbage_size+=s; return s; }
	// GCをしてみる
	if (gc->garbage_size>1000) {
		if (!garbage_collection(vm, s)) return 0;
		gc->garbage_size=0;
		gc_refresh(vm);
		// 割当て
		if (allocate_cell_(vm, s, cell)) { gc->garbage_size+=s; return s; }
	}
	// HEAPの拡張
	if (!add_hive(vm_get_gc(vm), s)) return 0;
	if (allocate_cell_(vm, s, cell)) { gc->garbage_size+=s; return s; }
	return 0;
}

static tUINT allocate_cell_(tPVM vm, tUINT size, tPCELL* cell)
{
	tUINT s;
	tINT old;
	old=vm_get_state(vm);
	vm_set_state(vm, VM_STATE_ALLOCATE);
	s=allocate_cell_free_list(vm, size, cell);
	if (s) goto RET;
	s=allocate_cell_sweep(vm, size, cell);
RET:
	vm_set_state(vm, old);
	return s;
}

static tUINT allocate_cell_free_list(tPVM vm, tUINT size, tPCELL* cell)
{
	tPCELL p, last;
	tPGC gc=vm_get_gc(vm);

	for (p=gc_get_free_list(gc), last=0; p; last=p, p=free_get_next(p)) {
		tUINT s=free_get_size(p);
		// sizeの比較
		if (s>=size) {
			if (s==size) {
				// サイズが等しい そのままメモリ割当てに使用する
				// フリーリストから取除く
				if (last) {
					free_set_next(last, free_get_next(p));
				} else {
					gc_set_free_list(gc, free_get_next(p));
				}
				gc->free_list_number--;
			} else {
				// フリーリストの方がサイズが大きい
				// 小さく分割して割当てを行う
				tPCELL free;
				free=p+size;
				free_initialize(free, s-size, free_get_next(p));
				gc_mark_cell(gc, free);
				// フリーリストに登録
				if (last) {
					free_set_next(last, free);
				} else {
					gc_set_free_list(gc, free);
				}
			}
			// 割当て
			CELL_HEAD_CLEAR(p);
			gc_mark_cell(gc, p);
			gc->free_list_size-=size;
			*cell=p;
			return size;
		}
	}
	return 0;
}

static tUINT allocate_cell_sweep(tPVM vm, tUINT size, tPCELL* cell)
{
	tPGC gc=vm_get_gc(vm);
	tPHIVE hive=gc->last_hive;
	do {
		tPCELL p;
		for (p=hive->last; p; p=gc_get_next_cell(hive, p), hive->last=p) {
			tUINT s=cell_get_size(p);
			if (cell_is_garbage(gc, p)) {
				// ごみオブジェクト発見
				// ごみオブジェクトの後処理
				if (cell_destroy(vm, p)) return 0;/*!!!*/
				// サイズの比較
				if (s==size) {
					// 大きさが等しい
					// hiveポインタの移動
					hive->last=gc_get_next_cell(hive, p);
					*cell=p;
					CELL_HEAD_CLEAR(p);
					gc_mark_cell(gc, p);
					return size;
				} else if (s>size) {
					tPCELL free;
					// ごみの方が大きい
					free=p+size;
					free_initialize(free, s-size, 0);
					gc_unmark_cell(gc, free);
					// hiveのポインタの移動
					hive->last=free;
					// 
					*cell=p;
					CELL_HEAD_CLEAR(p);
					gc_mark_cell(gc, p);
					return size;
				} else {
					// ごみの方が小さい
					// ごみをフリーリストへ
					gc_add_free_list(gc, p, s);
				}
			}
		}
		// 次のhiveへ
		hive=hive->next;
		if (hive) {
			hive->last=hive->cells;
			gc->last_hive=hive;
		}
	} while (hive);
	return 0;
}

static void gc_add_free_list(tPGC gc, tPCELL cell, const tUINT size)
{
	tPCELL free=gc->free_list;
	if (free&&(free+free_get_size(free)==cell)) {
		free_initialize(free, size+free_get_size(free), free_get_next(free));
	} else {
		free_initialize(cell, size, free);
		gc_mark_cell(gc, cell);
		gc->free_list=cell;
		gc->free_list_number++;
	}
	gc->free_list_size+=size;
}

///////////////////////////////////////

tBOOL garbage_collection(tPVM vm, tUINT size)
{
#if defined(_DEBUG)
	tPGC gc=vm_get_gc(vm);
	if (vm_get_standard_output(vm)) {
		tPCELL stream=vm_get_standard_output(vm);
		if (format_l(vm, stream, "gc : free-list (", 0)) return tFALSE;
		if (format_integer(vm, stream, gc->free_list_number, 10)) return tFALSE;
		if (format_l(vm, stream, " / ", 0)) return tFALSE;
		if (format_integer(vm, stream, gc->free_list_size*sizeof(tCELL), 10)) return tFALSE;
		if (format_l(vm, stream, "Byte )~%", 0)) return tFALSE;
	}
#endif
	// GC
	return (tisl_signal_gc_start(vm_get_tisl(vm), vm)==VM_OK) ? tTRUE : tFALSE;
}

tBOOL vm_mark(tPVM vm, const tINT sp)
{
	// 評価スタック
	{
		tINT i;
		for (i=1; i<=sp; i++) {
			if (object_mark(vm, vm->stack+i)) return tFALSE;
		}
		if (sp&&object_mark(vm, vm->SP)) return tFALSE;
	}
	//
	if (cell_mark(vm, vm->temp_stack)) return tFALSE;
	// 関数
	if (cell_mark(vm, vm_get_function(vm))) return tFALSE;
	// 環境
	if (cell_mark(vm, vm_get_environment(vm))) return tFALSE;
	// 例外ハンドラリスト
	if (cell_mark(vm, vm->handler_list)) return tFALSE;
	// タグリスト
	if (cell_mark(vm, vm->tag_list)) return tFALSE;
	if (object_mark(vm, &vm->last_condition)) return tFALSE;
	if (object_mark(vm, &vm->throw_object)) return tFALSE;
	// 標準ストリーム
	if (cell_mark(vm, vm->standard_input)) return tFALSE;
	if (cell_mark(vm, vm->standard_output)) return tFALSE;
	if (cell_mark(vm, vm->error_output)) return tFALSE;
	// 内部ストリーム
	if (cell_mark(vm, vm->private_stream)) return tFALSE;
	if (cell_mark(vm, vm->private_input_stream)) return tFALSE;
	// 変換器
	if (translator_mark(vm)) return tFALSE;
	// 局所参照
	if (cell_mark(vm, vm->local_ref_head)) return tFALSE;
	// 大域参照
	if (cell_mark(vm, vm->global_ref_head)) return tFALSE;
	//
	if (cell_mark(vm, vm->gc->free_list)) return tFALSE;
	// GCスタック(評価スタックを利用))へのマーキング
	while (vm->SP-vm->stack>sp) {
		vm->SP--;
//		if (OBJECT_IS_CELL(vm->SP+1)) {
			if (cell_mark_(vm, OBJECT_GET_CELL(vm->SP+1))) return tFALSE;
//		}
	}
	return tTRUE;
}

tBOOL vm_sweep(tPVM vm, const tINT sp)
{
	tPGC gc=vm_get_gc(vm);
	tPHIVE hive=gc->last_hive;
	do {
		tPCELL p;
		for (p=hive->last; p; p=gc_get_next_cell(hive, p)) {
			tUINT s=cell_get_size(p);
			if (cell_is_garbage(gc, p)) {
				// ごみオブジェクト発見
				// ごみオブジェクトの後処理
				if (cell_destroy(vm, p)) return tFALSE;/*!!!*/
				// サイズの比較
				gc_add_free_list(gc, p, s);
			}
		}
		// 次のhiveへ
		hive=hive->next;
		if (hive) {
			hive->last=hive->cells;
			gc->last_hive=hive;
		}
	} while (hive);
	return tTRUE;
}

VM_RET gc_push(tPVM vm, tPOBJECT obj)
{	// CELLを使用していて
	if (OBJECT_IS_CELL(obj)) {
		return cell_mark(vm, OBJECT_GET_CELL(obj));
	} else {
		return VM_OK;
	}
}

///////////////////////////////////////
//

extern VM_RET string_buffer_create(tPVM vm, tPBUFFER* b);
extern tPBUFFER string_buffer_get_next(tPBUFFER b);
extern void string_buffer_set_next(tPBUFFER b, tPBUFFER next);

VM_RET allocate_string_buffer(tPVM vm, tPBUFFER* buffer)
{
	tPGC gc=vm_get_gc(vm);
	// ごみになっているバッファがあるときはそれを利用する
	if (gc->buffer_head) {
		*buffer=gc->buffer_head;
		gc->buffer_head=string_buffer_get_next(*buffer);
	}
	return string_buffer_create(vm, buffer);
}

VM_RET free_string_buffer(tPVM vm, tPBUFFER buffer)
{
	tPGC gc=vm_get_gc(vm);
	string_buffer_set_next(buffer, gc->buffer_head);
	gc->buffer_head=buffer;
	return VM_OK;
}

