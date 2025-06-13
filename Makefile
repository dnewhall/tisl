CC	= gcc
CFLAGS	= -O2 -DTISL_C -DTISL_TEST
OBJ	= src/tisl/built_in_object.o\
	  src/tisl/evaluator.o\
	  src/tisl/gc.o\
	  src/tisl/object.o src/tisl/object_2.o src/tisl/object_3.o\
	  src/tisl/operation.o src/tisl/operation_2.c\
	  src/tisl/primitive_operation.o\
	  src/tisl/reader.o src/tisl/tisl.o src/tisl/tni.o\
	  src/tisl/translator.o src/tisl/translator_1.o\
	  src/tisl/translator_2.o src/tisl/vm.o src/tisl/writer.o\
	  src/tisl/c/function.o src/tisl/c/opcode.o\
	  src/tisl/c/translator_3.o\
	  src/tisl/interpreter.o

tisl : tisl_obj tisl_c_obj tisl_main
	$(CC) $(CFLAGS) -lm -o tisl_4_08 $(OBJ) src/console_app/main.o

tisl_obj :
	cd src/tisl; make tisl_obj

tisl_c_obj :
	cd src/tisl/c ; make tisl_c_obj

tisl_main :
	$(CC) $(CFLAGS) -c src/console_app/main.c -o src/console_app/main.o

clean :
	rm -f *.o
	rm -f *~
	cd src/tisl ; rm -f *.o ; rm -f *~
	cd src/console_app ; rm -f *.o ; rm -f *~
	cd src/tisl/c ; rm -f *.o ; rm -f *~

