//
// TISL/src/tisl/reader.c
// TISL Ver 4.x
//

#include <ctype.h>
#include <string.h>
#include "../../include/tni.h"
#include "object.h"
#include "vm.h"
#include "tisl.h"
#include "reader.h"
#include "built_in_object.h"

///////////////////////////////////////

static VM_RET read_char_(tPVM vm, tPCELL stream, tPCHAR c);
static VM_RET preview_char_(tPVM vm, tPCELL stream, tPCHAR c);

static VM_RET eat_comment(tPVM vm, tPCELL stream);
static VM_RET eat_wchar(tPVM vm, tPCELL stream, tCHAR c);
static VM_RET eat_line(tPVM vm, tPCELL stream);

static tBOOL is_return_char(tPVM vm, const tCHAR c);
static tBOOL is_symbol_head(tPVM vm, const tCHAR c);
static tBOOL is_symbol(tPVM vm, const tCHAR c);
static tBOOL is_delimiter(const tCHAR c);
static tBOOL is_comment(const tPOBJECT obj);
static void set_comment(tPOBJECT obj);

static VM_RET read_evaluation_form(tPVM vm, tPCELL stream, tPOBJECT obj);

static VM_RET read_line_(tPVM vm, tPCELL stream, tPCELL* string);

///////////////////

static VM_RET read_object(tPVM vm, tPCELL stream, tCHAR c, tPOBJECT ret);
static VM_RET read_cons(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_cdr(tPVM vm, tPCELL stream, tPOBJECT car, tPOBJECT ret);
static VM_RET read_cdr_object(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_dot_list(tPVM vm, tPCELL stream, tPOBJECT ret);

///////////////////

static VM_RET read_function(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_vector(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_binary(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_octal(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_hexadecimal(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_array(tPVM vm, tPCELL stream, const tINT d, tPOBJECT ret);
static VM_RET read_quote(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_string(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_quasiquote(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_unquote(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_unquote_splicing(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_number(tPVM vm, tPCELL stream, tCHAR c, const tBOOL sign, tPOBJECT ret);
static VM_RET read_character(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_plus(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_plus_(tPVM vm, tPCELL stream, tPCELL* string);
static VM_RET read_minus(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_minus_(tPVM vm, tPCELL stream, tPCELL* string);
static VM_RET read_symbol_all(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_symbol_all_(tPVM vm, tPCELL stream, tPCELL* string);
static VM_RET read_symbol_bar(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_symbol_bar_(tPVM vm, tPCELL stream, tPCELL* string);
static VM_RET read_symbol_complete(tPVM vm, tPCELL stream, tPOBJECT ret);
static VM_RET read_symbol(tPVM vm, tPCELL stream, tCHAR c, tPOBJECT ret);
static VM_RET read_symbol_(tPVM vm, tPCELL stream, tCHAR c, tPCELL* string);
static VM_RET read_next_symbol(tPVM vm, tPCELL stream, tPCELL list);

///////////////////

static VM_RET list_to_vector(tPVM vm, tPOBJECT list, tPOBJECT vec);
static VM_RET list_to_array(tPVM vm, tPOBJECT list, tPOBJECT array, const tINT d);
static VM_RET list_to_symbol(tPVM vm, tPOBJECT list, tPOBJECT symbol, const tBOOL complete);

///////////////////////////////////////

// VM_RET read_char(tPVM vm, tPCELL stream, tPCHAR c)
// vm上で，stream入力ストリームから1文字読込みを行う．
// 例外が発生した場合は，VM_ERRORを返す．
VM_RET read_char(tPVM vm, tPCELL stream, tPCHAR c)
{
	VM_RET ret;
	if (CELL_IS_STRING_STREAM(stream)&&
		string_stream_is_input(stream)) {
		if (string_stream_lock(stream, vm)) return VM_ERROR;
		ret=string_stream_read_char(vm, stream, c);
		string_stream_unlock(stream);
		return ret;
	} else if (CELL_IS_FILE_STREAM(stream)&&
		file_stream_is_input(stream)) {
		if (file_stream_lock(stream, vm)) return VM_ERROR;
		ret=file_stream_read_char(vm, stream, c);
		file_stream_unlock(stream);
		return ret;
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

// VM_RET preview_char(tPVM vm, tPCELL stream, tPCHAR c)
// vm上で，stream入力ストリームから1文字先読みを行う．
// 例外が発生した場合は，VM_ERRORを返す．
VM_RET preview_char(tPVM vm, tPCELL stream, tPCHAR c)
{
	VM_RET ret;
	if (CELL_IS_STRING_STREAM(stream)&&
		string_stream_is_input(stream)) {
		if (string_stream_lock(stream, vm)) return VM_ERROR;
		ret=string_stream_preview_char(vm, stream, c);
		string_stream_unlock(stream);
		return ret;
	} else if (CELL_IS_FILE_STREAM(stream)&&
		file_stream_is_input(stream)) {
		if (file_stream_lock(stream, vm)) return VM_ERROR;
		ret=file_stream_preview_char(vm, stream, c);
		file_stream_unlock(stream);
		return ret;
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

// VM_RET read_form(tPVM vm, tPCELL stream, tPOBJECT obj)
// vm上で stream入力ストリームから読込みを行い，
// ISLISPオブジェクトを作成し，objに返す．
// 例外等が発生した場合，VM_ERRORを返す．
VM_RET read_form(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	VM_RET ret;
	if (CELL_IS_STRING_STREAM(stream)&&
		string_stream_is_input(stream)) {
		if (string_stream_lock(stream, vm)) return VM_ERROR;
		ret=read_evaluation_form(vm, stream, obj);
		string_stream_unlock(stream);
		return ret;
	} else if (CELL_IS_FILE_STREAM(stream)&&
		file_stream_is_input(stream)) {
		if (file_stream_lock(stream, vm)) return VM_ERROR;
		ret=read_evaluation_form(vm, stream, obj);
		file_stream_unlock(stream);
		return ret;
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

//
VM_RET read_line(tPVM vm, tPCELL stream, tPCELL* string)
{
	VM_RET ret;
	if (CELL_IS_STRING_STREAM(stream)&&
		string_stream_is_input(stream)) {
		if (string_stream_lock(stream, vm)) return VM_ERROR;
		ret=read_line_(vm, stream, string);
		string_stream_unlock(stream);
		return ret;
	} else if (CELL_IS_FILE_STREAM(stream)&&
		file_stream_is_input(stream)) {
		if (file_stream_lock(stream, vm)) return VM_ERROR;
		ret=read_line_(vm, stream, string);
		file_stream_unlock(stream);
		return ret;
	}
	return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
}

///////////////////////////////////////

// 一文字読み込み
// 内部で使用するため 検査項目が少ない
static VM_RET read_char_(tPVM vm, tPCELL stream, tPCHAR c)
{
	if (CELL_IS_STRING_STREAM(stream)) {
		return string_stream_read_char(vm, stream, c);
	} else {
		return file_stream_read_char(vm, stream, c);
	}
}

// 一文字先読み
// 内部で使用するため 検査項目が少ない
static VM_RET preview_char_(tPVM vm, tPCELL stream, tPCHAR c)
{
	if (CELL_IS_STRING_STREAM(stream)) {
		return string_stream_preview_char(vm, stream, c);
	} else {
		return file_stream_preview_char(vm, stream, c);
	}
}

// 空白文字と1行コメントの読み飛ばし
// VM_RET eat_white(tPVM vm, tPCELL stream)
// vm上でstreamから空白文字を読み飛ばす
VM_RET eat_white(tPVM vm, tPCELL stream)
{
	tCHAR c;
	while (1) {
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
		while (isspace(c)) {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (preview_char_(vm, stream, &c)) return VM_ERROR;
		}
		if (c==';') {
			if (eat_line(vm, stream)) return VM_ERROR;
		} else {
			return VM_OK;
		}
	}
}

// #| |# で囲まれたコメントの読み飛ばし
// VM_RET eat_comment(isPVM vm, tPCELL stream)
// vm上でstreamからコメントを読み飛ばす
static VM_RET eat_comment(tPVM vm, tPCELL stream)
{
	tCHAR c;
	while (1) {
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (c=='|') {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (c=='#') return VM_OK;
		} else if (c=='#') {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if ((c=='|')&&eat_comment(vm, stream)) return VM_ERROR;
		}
		if (eat_wchar(vm, stream, c)) return VM_ERROR;
	}
}

// 2バイト文字の読み飛ばし
// VM_RET eat_wchar(tPVM vm, tPCELL stream, const tCHAR c)
// vm上でstreamから2バイト文字を読み飛ばす
// cが2バイト文字の1バイト目であった場合にもう一文字読み飛ばす．
static VM_RET eat_wchar(tPVM vm, tPCELL stream, tCHAR c)
{
	if (is_DBCS_lead_byte(vm, c)) {
		return read_char_(vm, stream, &c);
	}
	return VM_OK;
}

// 1行読み飛ばし
// tBOOL eat_line(tPVM vm, tPCELL stream)
// vm上でstreamから改行文字まで読み飛ばす
static VM_RET eat_line(tPVM vm, tPCELL stream)
{
	tCHAR c;
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	while (!is_return_char(vm, c)) {
		if (eat_wchar(vm, stream, c)) return VM_ERROR;
		if (read_char_(vm, stream, &c)) return VM_ERROR;
	}
	return VM_OK;
}

static tBOOL is_return_char(tPVM vm, const tCHAR c)
{
	return (c=='\n') ? tTRUE : tFALSE;
}

static tBOOL is_symbol_head(tPVM vm, const tCHAR c)
{
	return isalpha(c)||
		(c=='<')||(c=='>')||(c=='/')||(c=='*')||(c=='=')||(c=='?')||
		(c=='_')||(c=='!')||(c=='$')||(c=='%')||(c=='[')||(c==']')||
		(c=='^')||(c=='{')||(c=='}')||(c=='~')||is_DBCS_lead_byte(vm, c);
}

static tBOOL is_symbol(tPVM vm, const tCHAR c)
{
	return (isdigit(c)||(c=='+')||(c=='-')||is_symbol_head(vm, c)) ? tTRUE : tFALSE;
}

static tBOOL is_delimiter(const tCHAR c)
{
	/* ( ) ' , ` 区切り文字 */
	return (isspace(c)||(c=='(')||(c==')')||(c=='\'')||(c==',')||
		(c=='`')||(c=='\0')||(c==';')||(c==(tCHAR)EOF)) ? tTRUE : tFALSE;
}

static tBOOL is_comment(const tPOBJECT obj)
{
	return OBJECT_IS_UNBOUND(obj) ? tTRUE : tFALSE;
}

static void set_comment(tPOBJECT obj)
{
	OBJECT_SET_UNBOUND(obj);
}

///////////////////////////////////////

// 評価形式の読込み (リスト含む)
static VM_RET read_evaluation_form(tPVM vm, tPCELL stream, tPOBJECT obj)
{
	tCHAR c;
	while (1) {
		// 余計な空白文字の読み込み
		if (eat_white(vm, stream)) return VM_ERROR;
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		switch (c) {
		case '(':// コンス
			return read_cons(vm, stream, obj);
		case ')':
			return signal_parse_error(vm, TISL_ERROR_RIGHT_ARC, stream);
		case '\0':// end of stream
		case EOF:
			OBJECT_SET_NIL(obj);
			if (vm_get_reader_eos_error(vm)) {
				vm_set_last_condition_eos_error(vm);
				return VM_ERROR;
			} else {
				return signal_stream_error(vm, TISL_ERROR_END_OF_STREAM, stream);
			}
		default:// リスト以外のISLISPオブジェクト
			if (read_object(vm, stream, c, obj)==VM_OK) return VM_OK;
			// 失敗　コメントか他の例外
			if (!is_comment(obj)) return VM_ERROR;
			// コメントの場合読み込みなおす
		}
	}
}

///////////////////

static VM_RET read_line_(tPVM vm, tPCELL stream, tPCELL* string)
{
	tCHAR c;

	vm_output_stream_clear(vm);
	if (check_eos(stream)) c='\n';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	while (c!='\n') {
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		vm_output_stream_write_char(vm, c);
		if (check_eos(stream)) c='\n';
		else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (vm_output_stream_to_string(vm, string)) return VM_ERROR;
	vm_output_stream_clear(vm);
	return VM_OK;
}

///////////////////

static VM_RET read_cons(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	tOBJECT car;
COMMENT_LOOP:
	// car部の読込み
	if (eat_white(vm, stream)) return VM_ERROR;
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	switch (c) {
	case ')':// () 空括弧
		OBJECT_SET_NIL(ret);
		return VM_OK;
	case '(':// car部がcons
		if (read_cons(vm, stream, &car)) return VM_ERROR;
		break;
	default:// car部はコンス以外のオブジェクト
		if (read_object(vm, stream, c, &car)) {
			// コメントの場合は読込み直し
			if (is_comment(&car)) goto COMMENT_LOOP;
			// 何か例外が発生している
			return VM_ERROR;
		}
	}
	// cdr部の読込み
	return read_cdr(vm, stream, &car, ret);
}

// コンスの読込み(cdrbu)
static VM_RET read_cdr(tPVM vm, tPCELL stream, tPOBJECT car, tPOBJECT ret)
{
	tOBJECT cdr;
	tPCELL cons;
	// car部を保存
	if (vm_push(vm, car)) return VM_ERROR;
	// cdr部の読込み
	if (read_cdr_object(vm, stream, &cdr)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	// コンスの作成
	if (cons_create(vm, &cons, car, &cdr)) return VM_ERROR;
	CELL_SET_IMMUTABLE(cons);
	OBJECT_SET_CONS(ret, cons);
	return VM_OK;
}

// cdr部のオブジェクトの読込み
static VM_RET read_cdr_object(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	tOBJECT car;
COMMENT_LOOP:
	if (eat_white(vm, stream)) return VM_ERROR;
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	switch (c) {
	case '.':// ドットリスト
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
		if (!is_delimiter(c)) 
			return signal_parse_error(vm, TISL_ERROR_NOT_DELIMITER_AROUND_DOT, stream);
		return read_dot_list(vm, stream, ret);
	case ')':// nilで終わるリスト
		OBJECT_SET_NIL(ret);
		return VM_OK;
	case '(':// cdr部はコンスでそのcar部はリスト
		if (read_cons(vm, stream, &car)) return VM_ERROR;
		break;
	default:// 次のコンスのcar部
		if (read_object(vm, stream, c, &car)) {
			if (is_comment(&car)) {
				goto COMMENT_LOOP;
			} else {
				return VM_ERROR;
			}
		}
	}
	return read_cdr(vm, stream, &car, ret);
}

// ドットリストの読込み
static VM_RET read_dot_list(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	if (eat_white(vm, stream)) return VM_ERROR;
	if (read_evaluation_form(vm, stream, ret)) return VM_ERROR;
	if (eat_white(vm, stream)) return VM_ERROR;
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	// 括弧が閉じるまで読み進む
	while (c!=')') {
		if (c=='#') {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			// ドットの後ろに複数のオブジェクト
			if (c!='|') return signal_parse_error(vm, TISL_ERROR_OBJECTS_BEHIND_DOT, stream);
			// コメントの読込み
			if (eat_white(vm, stream)) return VM_ERROR;
		} else {
			return signal_parse_error(vm, TISL_ERROR_OBJECTS_BEHIND_DOT, stream);
		}
		if (eat_white(vm, stream)) return VM_ERROR;
		if (read_char_(vm, stream, &c)) return VM_ERROR;
	}
	return VM_OK;
}

// 評価形式の読込み
static VM_RET read_object(tPVM vm, tPCELL stream, tCHAR c, tPOBJECT ret)
{
	tINT n=0;
	OBJECT_SET_NIL(ret);
	switch (c) {
	case '#':// #' #( #B #b #O #o #X #x #nA #na #\ #| comment |#
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		switch (c) {
		case '\'':// #'name -> (function name)
			return read_function(vm, stream, ret);
		case '(':
			return read_vector(vm, stream, ret);
		case 'B':
		case 'b':// 2進数
			return read_binary(vm, stream, ret);
		case 'O':
		case 'o':// 8進数
			return read_octal(vm, stream, ret);
		case 'X':
		case 'x':// 16進数
			return read_hexadecimal(vm, stream, ret);
		case '|':// コメント
			if (eat_comment(vm, stream)) return VM_ERROR;
			set_comment(ret);
			return VM_ERROR;
		case '\\':// 文字
			return read_character(vm, stream, ret);
		case '!':// line comment
			if (eat_line(vm, stream)) return VM_ERROR;
			set_comment(ret);
			return VM_ERROR;
		}
		while (isdigit(c)) {
			n*=10;
			n+=c-'0';
			if (read_char_(vm, stream, &c)) return VM_ERROR;
		}
		if ((c=='A')||(c=='a'))
			return read_array(vm, stream, n, ret);
		else
			return signal_parse_error(vm, TISL_ERROR_SHARP_NUMBER, stream);
	case '\'':// 'obj (quote obj)
		return read_quote(vm, stream, ret);
	case '"':// "string"
		return read_string(vm, stream, ret);
	case '`'://	quasiquote
		return read_quasiquote(vm, stream, ret);
	case ',':// unquote unquote-splicing
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
		if (c=='@') {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			return read_unquote_splicing(vm, stream, ret);
		} else {
			return read_unquote(vm, stream, ret);
		}
	case '|':// |symbol|
		return read_symbol_bar(vm, stream, ret);
	case '&':// &rest
	case ':':// keyword完全限定名
		return read_symbol_complete(vm, stream, ret);
	}
	// 10進数字
	if (isdigit(c)) return read_number(vm, stream, c, tTRUE, ret);
	// シンボル
	if (is_symbol_head(vm, c)) return read_symbol(vm, stream, c, ret);
	// +, - または 符号付10進数字
	if (c=='+') return read_plus(vm, stream, ret);
	if (c=='-') return read_minus(vm, stream, ret);
	// 未知の文字
	return signal_parse_error(vm, TISL_ERROR_UNKNOWN_CHARACTER, stream);
}

///////////////////////////////////////

// #'name (function name)
static VM_RET read_function(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tOBJECT name, obj2, obj3;
	tPCELL cell1, cell2;
	// nameの読込み
	if (read_symbol_all(vm, stream, &name)) return VM_ERROR;
	if (!OBJECT_IS_SYMBOL(&name)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_FUNCTION, stream);
	// (name . nil)
	if (cons_create(vm, &cell2, &name, &nil)) return VM_ERROR;
	CELL_SET_IMMUTABLE(cell2);
	OBJECT_SET_CONS(&obj3, cell2);
	// function
	OBJECT_SET_SYMBOL(&obj2, SYMBOL_FUNCTION);
	// (function . (name . nil))
	if (cons_create(vm, &cell1, &obj2, &obj3)) return VM_ERROR;
	CELL_SET_IMMUTABLE(cell1);
	OBJECT_SET_CONS(ret, cell1);
	return VM_OK;
}

// ベクタ
// #(obj1 obj2 ... objn)
static VM_RET read_vector(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tOBJECT list;
	if (read_cons(vm, stream, &list)) return VM_ERROR;
	if (list_to_vector(vm, &list, ret)) return VM_ERROR;
	return VM_OK;
}

static tBOOL read_sign(tPVM vm, tPCELL stream, tINT* sign, tPCHAR c)
{
	if (preview_char_(vm, stream, c)) return VM_ERROR;
	if ((*c=='+')||(*c=='-')) {
		if (*c=='-') *sign=-1;
		if (read_char_(vm, stream, c)) return VM_ERROR;
		if (preview_char_(vm, stream, c)) return VM_ERROR;
	}
	return VM_OK;
}

// 2進数整数
// #B[s]bb...b
static VM_RET read_binary(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	tINT s=1, b=0;
	// 符号の読込み
	if (read_sign(vm, stream, &s, &c)) return VM_ERROR;
	// bb...b
	while ((c=='0')||(c=='1')) {
		b<<=1;
		b+=c-'0';
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (check_eos(stream)) goto RET;
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (!is_delimiter(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_INTEGER, stream);
RET:
	OBJECT_SET_INTEGER(ret, s*b);
	return VM_OK;
}

// 8進整数
// #O[s]oo...o
static VM_RET read_octal(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	tINT s=1, o=0;
	// 符号の読込み
	if (read_sign(vm, stream, &s, &c)) return VM_ERROR;
	// oo...o
	while ((c>='0')&&(c<='7')) {
		o<<=3;
		o+=c-'0';
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (check_eos(stream)) goto RET;
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (!is_delimiter(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_INTEGER, stream);
RET:
	OBJECT_SET_INTEGER(ret, s*o);
	return VM_OK;
}

// 16進整数
// #X[s]xx...x
static VM_RET read_hexadecimal(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	tINT s=1, x=0;
	// 符号の読込み
	if (read_sign(vm, stream, &s, &c)) return VM_ERROR;
	// xx...x
	while (isxdigit(c)) {
		x<<=4;
		if (isdigit(c))
			x+=c-'0';
		else if (isupper(c))
			x+=c-'A'+10;
		else
			x+=c-'a'+10;
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (check_eos(stream)) goto RET;
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (!is_delimiter(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_INTEGER, stream);
RET:
	OBJECT_SET_INTEGER(ret, s*x);
	return VM_OK;
}

// 配列
// #da(...)
static VM_RET read_array(tPVM vm, tPCELL stream, const tINT d, tPOBJECT ret)
{
	tCHAR c;
	tOBJECT list;
	if (check_eos(stream)) goto RET;
	if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (c=='(') {
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (read_cons(vm, stream, &list)) return VM_ERROR;
	} else {
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (is_delimiter(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_ARRAY, stream);
		if (read_object(vm, stream, c, &list)) return VM_ERROR;
	}
RET:
	if (list_to_array(vm, &list, ret, d)) return VM_ERROR;
	CELL_SET_IMMUTABLE(OBJECT_GET_CELL(ret));
	return VM_OK;
}

// quote
// 'obj -> (quote obj)
static VM_RET read_quote(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tOBJECT obj1, obj2;
	tPCELL p;

	if (read_evaluation_form(vm, stream, &obj1)) return VM_ERROR;
	if (cons_create(vm, &p, &obj1, &nil)) return VM_ERROR;
	CELL_SET_IMMUTABLE(p);
	OBJECT_SET_CONS(&obj2, p);
	OBJECT_SET_SYMBOL(&obj1, SYMBOL_QUOTE);
	if (cons_create(vm, &p, &obj1, &obj2)) return VM_ERROR;
	CELL_SET_IMMUTABLE(p);
	OBJECT_SET_CONS(ret, p);
	return VM_OK;
}

// string
static VM_RET read_string(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	tPCELL p;
	vm_output_stream_clear(vm);
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	while (c!='"') {
		if (c=='\\') {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if ((c!='\\')&&(c!='"')) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_STRING, stream);
		}
		if (is_DBCS_lead_byte(vm, c)) {
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
			if (read_char_(vm, stream, &c)) return VM_ERROR;
		}
		if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
		if (read_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (check_eos(stream)) goto RET;
	if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (!is_delimiter(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_STRING, stream);
RET:
	if (vm_output_stream_to_string(vm, &p)) return VM_ERROR;
	CELL_SET_IMMUTABLE(p);
	OBJECT_SET_STRING(ret, p);
	return VM_OK;
}

// quasiquote
// `form
static VM_RET read_quasiquote(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tPCELL p;
	tOBJECT form;
	if (read_evaluation_form(vm, stream, &form)) return VM_ERROR;
	if (quasiquote_create(vm, &form, &p)) return VM_ERROR;
	CELL_SET_IMMUTABLE(p);
	OBJECT_SET_QUASIQUOTE(ret, p);
	return VM_OK;
}

// unquote
// ,form
static VM_RET read_unquote(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tPCELL p;
	tOBJECT form;
	if (read_evaluation_form(vm, stream, &form)) return VM_ERROR;
	if (unquote_create(vm, &form, &p)) return VM_ERROR;
	CELL_SET_IMMUTABLE(p);
	OBJECT_SET_UNQUOTE(ret, p);
	return VM_OK;
}

// unquote-splicing
// ,@form
static VM_RET read_unquote_splicing(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tPCELL p;
	tOBJECT form;
	if (read_evaluation_form(vm, stream, &form)) return VM_ERROR;
	if (unquote_splicing_create(vm, &form, &p)) return VM_ERROR;
	CELL_SET_IMMUTABLE(p);
	OBJECT_SET_UNQUOTE_SPLICING(ret, p);
	return VM_OK;
}

///////////////////

static VM_RET read_float(tPVM vm, tPCELL stream, tFLOAT f, const tBOOL sign, tPOBJECT ret);
static VM_RET read_float_(tPVM vm, tPCELL stream, tFLOAT f, const tBOOL sign, tPOBJECT ret);

static VM_RET read_number(tPVM vm, tPCELL stream, tCHAR c, const tBOOL sign, tPOBJECT ret)
{
	tINT i;
	// 整数
	i=c-'0';
	if (preview_char_(vm, stream, &c)) return VM_ERROR;
	while (isdigit(c)) {
		if (i>TISL_MOST_POSITIVE_INTEGER/10) return read_float(vm, stream, (tFLOAT)i, sign, ret);
		i*=10;
		i+=c-'0';
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (check_eos(stream)) {
			// 10進整数
			if (!sign) i=-1;
			OBJECT_SET_INTEGER(ret, i);
			return VM_OK;
		}
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (is_delimiter(c)) {
		// 10進整数
		if (!sign) i=-i;
		OBJECT_SET_INTEGER(ret, i);
		return VM_OK;
	} else if ((i==1)&&((c=='+')||(c=='-'))) {
		// 1+, 1-の可能性
		tCHAR cc;
		if (read_char_(vm, stream, &cc)) return VM_ERROR;
		if (!check_eos(stream)) {
			if (preview_char_(vm, stream, &cc)) return VM_ERROR;
			if (!is_delimiter(cc)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_INTEGER, stream);
		}
		if (c=='+') {
			OBJECT_SET_SYMBOL(ret, SYMBOL_ONE_PLUS);
		} else {
			OBJECT_SET_SYMBOL(ret, SYMBOL_ONE_MINUS);
		}
		return VM_OK;
	}
	// 不動小数点数
	return read_float_(vm, stream, (tFLOAT)i, sign, ret);
}

static VM_RET read_float(tPVM vm, tPCELL stream, tFLOAT f, const tBOOL sign, tPOBJECT ret)
{
	tCHAR c;
	if (preview_char_(vm, stream, &c)) return VM_ERROR;
	while (isdigit(c)) {
		f*=10;
		f+=c-'0';
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (check_eos(stream)) c='\0';
		else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (is_delimiter(c)) {
		if (!sign) f=-f;
		OBJECT_SET_FLOAT(ret, f);
		return VM_OK;
	} else {
		return read_float_(vm, stream, f, sign, ret);
	}
}

static VM_RET read_float_(tPVM vm, tPCELL stream, tFLOAT f, const tBOOL sign, tPOBJECT ret)
{
	tINT i, e;
	tCHAR c;
	e=0;
	if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (c=='.') {
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
		while (isdigit(c)) {
			f*=10;
			f+=c-'0';
			e--;
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (check_eos(stream)) c='\0';
			else if (preview_char_(vm, stream, &c)) return VM_ERROR;
		}
		// e==0 小数点数以下に記述がないものは許されない
		if (!e) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_FLOAT, stream);
		if (is_delimiter(c)) goto RET;
	}
	if ((c!='E')&&(c!='e')) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_FLOAT, stream);
	{// 指数部分の読込み
		tINT ee=0, s=1;
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (preview_char_(vm, stream, &c)) return VM_ERROR;
		if ((c=='+')||(c=='-')) {
			if (c=='-') s=-1;
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (preview_char_(vm, stream, &c)) return VM_ERROR;
		}
		while (isdigit(c)) {
			ee*=10;
			ee+=c-'0';
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (check_eos(stream)) c='\0';
			else if (preview_char_(vm, stream, &c)) return VM_ERROR;
		}
		if (!is_delimiter(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_FLOAT, stream);
		e+=s*ee;
	}
	if (e>0) {
		for (i=0; i<e; i++) f*=10;
	} else {
RET:
		for (i=0; i>e; i--) f/=10;
	}
	if (!sign) f=-f;
	OBJECT_SET_FLOAT(ret, f);
	return VM_OK;

}

static VM_RET read_character(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c, cc;
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	if ((c=='n')||(c=='N')||(c=='s')||(c=='S')) {
		if (check_eos(stream)) cc=EOF;
		else if (preview_char_(vm, stream, &cc)) return VM_ERROR;
		if (!is_delimiter(cc)) {
			tCHAR string[8];
			int i=0;
			if (isupper(c)) c=tolower(c);
			string[i]=c;
			while (!is_delimiter(cc)&&(i<7)) {
				if (isupper(cc)) cc=tolower(cc);
				string[++i]=cc;
				if (read_char_(vm, stream, &cc)) return VM_ERROR;
				if (check_eos(stream)) cc=EOF;
				else if (preview_char_(vm, stream, &cc)) return VM_ERROR;
			}
			if (i==7) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_CHARACTER, stream);
			string[++i]='\0';
			if (!strcmp((char*)string, "space")) c=' ';
			else if (!strcmp((char*)string, "newline")) c='\n';
			else return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_CHARACTER, stream);
		}
	} else {
		if (!isgraph(c)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_CHARACTER, stream);
		if (check_eos(stream)) cc=EOF;
		else if (preview_char_(vm, stream, &cc)) return VM_ERROR;
		if (!is_delimiter(cc)) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_CHARACTER, stream);
	}
	OBJECT_SET_CHARACTER(ret, c);
	return VM_OK;
}

// + または 符号付10進数字
static VM_RET read_plus(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	if (check_eos(stream)) c='\0';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (is_delimiter(c)) {
		// 記号 +
		OBJECT_SET_SYMBOL(ret, SYMBOL_PLUS);
		return VM_OK;
	} else if (isdigit(c)) {
		// 符号付数字
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		return read_number(vm, stream, c, tTRUE, ret);
	} else if (c==':') {
		tOBJECT obj2;
		tPCELL list;
		OBJECT_SET_STRING(&obj2, string_plus);
		if (cons_create(vm, &list, &obj2, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&obj2, list);
		if (vm_push(vm, &obj2)) return VM_ERROR;
		if (read_next_symbol(vm, stream, list)) { vm_pop(vm); return VM_ERROR; }
		vm_pop(vm);
		return list_to_symbol(vm, &obj2, ret, tFALSE);
	} else {
		return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
	}
}

static VM_RET read_plus_(tPVM vm, tPCELL stream, tPCELL* string)
{
	tCHAR c;
	if (check_eos(stream)) c='\0';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (is_delimiter(c)||(c==':')) {
		*string=string_plus;
		return VM_OK;
	} else {
		return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
	}
}

static VM_RET read_minus(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	if (check_eos(stream)) c='\0';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (is_delimiter(c)) {
		//記号 -
		OBJECT_SET_SYMBOL(ret, SYMBOL_MINUS);
		return VM_OK;
	} else if (isdigit(c)) {
		// 符号付数字
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		return read_number(vm, stream, c, tFALSE, ret);
	} else if (c==':') {
		tOBJECT obj2;
		tPCELL list;
		OBJECT_SET_STRING(&obj2, string_minus);
		if (cons_create(vm, &list, &obj2, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&obj2, list);
		if (vm_push(vm, &obj2)) return VM_ERROR;
		if (read_next_symbol(vm, stream, list)) { vm_pop(vm); return VM_ERROR; }
		vm_pop(vm);
		return list_to_symbol(vm, &obj2, ret, tFALSE);
	} else {
		return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
	}
}

static VM_RET read_minus_(tPVM vm, tPCELL stream, tPCELL* string)
{
	tCHAR c;
	if (check_eos(stream)) c='\0';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (is_delimiter(c)||(c==':')) {
		*string=string_minus;
		return VM_OK;
	} else {
		return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
	}
}

static VM_RET read_symbol_all(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tCHAR c;
	// 先頭の文字で場合わけ
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	if (c=='|') {
		return read_symbol_bar(vm, stream, ret);
	} else if (c==':') {
		return read_symbol_complete(vm, stream, ret);
	} else if (c=='+') {
		return read_plus(vm, stream, ret);
	} else if (c=='-') {
		return read_minus(vm, stream, ret);
	} else if (is_symbol_head(vm, c)) {
		return read_symbol(vm, stream, c, ret);
	}
	return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
}

static VM_RET read_symbol_all_(tPVM vm, tPCELL stream, tPCELL* string)
{
	tCHAR c;
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	if (c=='|') {
		return read_symbol_bar_(vm, stream, string);
	} else if (c=='+') {
		return read_plus_(vm, stream, string);
	} else if (c=='-') {
		return read_minus_(vm, stream, string);
	} else if (is_symbol_head(vm, c)) {
		return read_symbol_(vm, stream, c, string);
	}
	return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
}

static VM_RET read_symbol_bar(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tOBJECT obj;
	tPCELL string, list;
	if (read_symbol_bar_(vm, stream, &string)) return VM_ERROR;
	OBJECT_SET_STRING(&obj, string);
	if (cons_create(vm, &list, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, list);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (read_next_symbol(vm, stream, list)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return list_to_symbol(vm, &obj, ret, tFALSE);
}

static VM_RET read_symbol_bar_(tPVM vm, tPCELL stream, tPCELL* string)
{
	tCHAR c;
	// 記号の読込み
	vm_output_stream_clear(vm);
	if (read_char_(vm, stream, &c)) return VM_ERROR;
	while (c!='|') {
		if (c=='\\') {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if ((c!='\\')&&(c!='|')) return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
		} else if (is_DBCS_lead_byte(vm, c)) {
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
		} else {
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
		}
		if (read_char_(vm, stream, &c)) return VM_ERROR;
	}
	// 文字列のリストに変換しておく
	return vm_output_stream_to_string(vm, string);
}

static VM_RET read_symbol_complete(tPVM vm, tPCELL stream, tPOBJECT ret)
{
	tPCELL string, list;
	tOBJECT obj;
	if (read_symbol_all_(vm, stream, &string)) return VM_ERROR;
	OBJECT_SET_STRING(&obj, string);
	if (cons_create(vm, &list, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, list);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (read_next_symbol(vm, stream, list)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return list_to_symbol(vm, &obj, ret, tTRUE);
}

static VM_RET read_symbol(tPVM vm, tPCELL stream, tCHAR c, tPOBJECT ret)
{
	tPCELL string, list;
	tOBJECT obj;
	if (read_symbol_(vm, stream, c, &string)) return VM_ERROR;
	OBJECT_SET_STRING(&obj, string);
	if (cons_create(vm, &list, &obj, &nil)) return VM_ERROR;
	OBJECT_SET_CONS(&obj, list);
	if (vm_push(vm, &obj)) return VM_ERROR;
	if (read_next_symbol(vm, stream, list)) { vm_pop(vm); return VM_ERROR; }
	vm_pop(vm);
	return list_to_symbol(vm, &obj, ret, tFALSE);
}

static VM_RET read_symbol_(tPVM vm, tPCELL stream, tCHAR c, tPCELL* string)
{
	vm_output_stream_clear(vm);
	c=tolower(c);
	if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
	if (check_eos(stream)) c='\0';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	while (is_symbol(vm, c)) {
		if (is_DBCS_lead_byte(vm, c)) {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
		} else {
			if (read_char_(vm, stream, &c)) return VM_ERROR;
			c=tolower(c);
			if (vm_output_stream_write_char(vm, c)) return VM_ERROR;
		}
		if (check_eos(stream)) c='\0';
		else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	}
	if (is_delimiter(c)||(c==':')) {
		return vm_output_stream_to_string(vm, string);
	} else {
		return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
	}
}

static VM_RET read_next_symbol(tPVM vm, tPCELL stream, tPCELL list)
{
	tCHAR c;
	if (check_eos(stream)) c='\0';
	else if (preview_char_(vm, stream, &c)) return VM_ERROR;
	if (c==':') {
		tPCELL string, p;
		tOBJECT obj;
		// 一つの記号を文字列として読込む
		if (read_char_(vm, stream, &c)) return VM_ERROR;
		if (read_symbol_all_(vm, stream, &string)) return VM_ERROR;
		// 文字列をリストに追加
		OBJECT_SET_STRING(&obj, string);
		if (cons_create(vm, &p, &obj, &nil)) return VM_ERROR;
		OBJECT_SET_CONS(&obj, p);
		cons_set_cdr(list, &obj);
		return read_next_symbol(vm, stream, p);
	} else if (is_delimiter(c)) {
		return VM_OK;
	} else {
		return signal_parse_error(vm, TISL_ERROR_PARSE_ERROR_SYMBOL, stream);
	}
}

///////////////////

static VM_RET list_to_symbol(tPVM vm, tPOBJECT list, tPOBJECT symbol, const tBOOL complete)
{
	tPCELL cell;
	if (!OBJECT_IS_CONS(list)) return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (tisl_get_symbol(vm_get_tisl(vm), vm, OBJECT_GET_CELL(list), complete, &cell)) return VM_ERROR;
	if (cell==SYMBOL_NIL) {
		OBJECT_SET_NIL(symbol);
	} else {
		OBJECT_SET_SYMBOL(symbol, cell);
	}
	return VM_OK;
}

static VM_RET list_to_vector(tPVM vm, tPOBJECT list, tPOBJECT vec)
{
	tPCELL vector, p;
	if (OBJECT_IS_CONS(list))
		p=OBJECT_GET_CELL(list);
	else if (OBJECT_IS_NIL(list))
		p=0;
	else
		return signal_condition(vm, TISL_ERROR_SYSTEM_ERROR);
	if (vector_create(vm, p, &vector)) return VM_ERROR;
	OBJECT_SET_VECTOR(vec, vector);
	return VM_OK;
}

static VM_RET list_to_array(tPVM vm, tPOBJECT list, tPOBJECT array, const tINT d)
{
	tPCELL p;
	if (d==1) {
		// d==1 ベクタ
		return list_to_vector(vm, list, array);
	} else {
		if (array_create(vm, d, list, &p)) return VM_ERROR;
	}
	OBJECT_SET_ARRAY(array, p);
	return VM_OK;
}

