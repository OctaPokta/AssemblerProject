/*
 * pre_processing.h - Header file for pre-processing functions in the assembler project
 * This file contains the declarations of functions used in the pre-processing stage of the assembler.
 * The pre-processing stage involves handling macros and creating intermediate files for further assembly stages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* declerations: */
int pre_assembler(FILE*, int, char*);