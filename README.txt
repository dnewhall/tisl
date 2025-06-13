This is a mirror of the code for version 4.08C of TISL, with the requisite changes to get this to compile on modern systems.

TISL stands for "Tohoku university ISLisp system" and was one of the first ISLisp interpreters (after OpenLisp). Takayasu Ito appears to have been the primary developer. The first references to it were published in 1998 (the first official ISLIsp standard was released in 1997). The last official build was released Mar. 2, 2002.

The Computer History Museum's Software Preservation Project posted the code at https://www.softwarepreservation.org/projects/LISP/ISLISP/ saying they had the permission of Prof. Takayasu Ito to release it, but there is no licensing information attached. Thus, this code is right now only provided with the minimum amount of changes needed to get it to compile again. It has been tested on Linux on x64 with GCC and MacOS on Arm64 with clang.

The following changes have been made:
- Converted comments from EUC-JP to UTF-8
- Use <stdlib.h> instead of <malloc.h>
- Fixed multiple definitions of built-in objects
- Added -lm flag to Makefile to get math routines

The only files that were not in the original distribution are this file (README.txt) and BUGS.md.

See BUGS.md for the many known issues in this code.