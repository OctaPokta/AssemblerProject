/*
 * macros_table.h - Header file for macros table functions in the assembler project
 * This file contains the declarations of functions and structures used for handling 
 * macros in the pre-processing stage of the assembler.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/* pointer node */
typedef struct node *ptr;

/* struct of a macro node */
typedef struct node {
	char *macro_name;
	char *macro_content;
	struct node *next;
} m_item;

/* Global head pointer for the linked list */
struct node *hptr;

/* Declerations: */
int isOnlyWord(char*);
int onlyTwoWords(char*);
int validMacroName(char*);
int isMacro(char*);
char *getMacro(char*);
char* get_first_word(const char*);
char* get_second_word(char*);
int addMacro(char*);
int addMacroContent(char*, char*);
void freeMacro();