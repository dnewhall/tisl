# Known bugs and implementation behavior for TISL 4.08C

These are all the implementation details and known bugs in TISL 4.8C.

## Implementation-specific behavior

This are the specifics for some of the behavior that can vary between standard ISLisp implementations.

- The neutral alphabetic case is lower-case.
- All conditions are `<built-in-class>`es and thus new ones cannot be created or defined.
- Gensyms are printed as `#g1`.
- TISL treats a bare keyword as a variable reference.
- TISL allows redefinition of top-level forms.
- TISL does not allow definition of top-level forms in any other forms except `progn`.
(This restriction is standard, but is a common extension.)
- Symbols cannot begin with "-" or "+"
(This restriction is standard, but is a common extension.)
- TISL has separate `<file-stream>` and `<string-stream>` classes.
- Only character names allowed are `#\newline` and `#\space`.
- TISL seems to check arguments from right to left
  - `(elt 5 '(1 2))` complains about the `'(1 2)` first
  - `(= nil t)` complains about the `t` first
- The default initial value of `create-list` elements is `nil`.
- The default initial value of `create-vector` elements is `nil`.
- The default initial value of `create-string` elements is `#\space`.
- The initial value of all undefined dynamic variables is `#unbound`.

## Implementation-specific errors

There are a number of places where the standard says that something is implementation defined, not allowed, or an error should be signaled, but doesn't specify what kind of error should be signaled or what types it should check for.

- The expected class of all sequence `<domain-error>`s is `<list>`.
- For `(open-input-file "/tmp" 'foo)`, TISL signals a domain-error that `'foo` isn't an integer.
- When creating a built-in class, TISL signals a `<program-error>` with the condition name `no-applicable-method` (presumably from the implicit call to `initialize-object`).
- When creating an abstract class, TISL signals a `<program-error>` with the condition name `abstract-class`.
- When no applicable method applies, TISL signals a `<program-error>` with the condition name `no-applicable-method`.
- When continuing a non-continuable condition, TISL signals a `<control-error>`.
- TISL signals a `<contol-error>` upon going off the end of a condition handler.

## Specification-omitted behaviors

These are behaviors that the standard doesn't address.

- TISL allows zero-rank arrays: `#0(...)`
- For out of bounds arguments ex. `(char-index #\a "foobar" 15)`, TISL returns `nil`.
- For underspecified arguments to `format` ex: `(format (standard-output) "~A")`, TISL signals a `<program-error>`.
- For too many arguments to `format` ex:`(format (standard-output) "~A" 1 2)`, TISL ignores any additional unused arguments.
- `file-position (standard-error))` signals a `stream-error`.
- Identifiers can be reused in a `dynamic-let`:
```
;; TISL prints `#unbound` and returns `nil`.
(dynamic-let ((d 2)
              (d (format (standard-output) "~A~&" (dynamic d))))
  (dynamic d))
```

## Bugs

- TISL allows `t` and `*pi*` to be shadowed and assigned to locally. The spec. disallows this on pg. 2.
- TISL signals a `<domain-error>` on `(convert nil <string>)`. The spec. implies this should work because `nil` is a symbol.
- For `(subclassp (class <integer>) (class <integer>))`, TISL returns `t`. The spec. seems to indicate that it should be strict subclassing only.
- `set-dynamic` doesn't exist.
- `:accessor`s used in `defclass` cannot by used to assign.
- `read-line` never consumes the newline it is reading to, so you will always receive an empty string until you move past the newline in some other manner (`read-char`, `set-file-position`, etc.).
- `set-file-position` will signal an error, but still sets the file position.
- Given the function:
```
  (defun read-from-string (string &rest arguments)
    (let ((stream (create-string-input-stream string)))
      (apply #'read stream arguments)))
```
TISL fails on `(read-from-string "2")` with an `<end-of-stream>` error. Calling `(read-from-string "2 ")`or `(read-from-string "12")` works, however.

