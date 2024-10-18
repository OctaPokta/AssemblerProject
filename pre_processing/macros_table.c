/*
 * macro_labels.c - This file deals with the macro table using a linked list 
 * and provides various helper functions for handling macros during the pre-processing stage.
 */

#include "macros_table.h"
#include "pre_assembler.h"
#include "../assembler/assembler.h"


/*
 * isOnlyWord - Checks whether there is only one word in the given line.
 * @line: The line to be checked.
 * 
 * This function skips leading and trailing whitespace and checks if the line contains 
 * only one word. It returns 1 if there is only one word, and 0 otherwise.
 * 
 * Return: 1 if the line contains only one word, 0 otherwise.
 */
int isOnlyWord(char *line) 
{
    /* Skip leading whitespace */
    line = skipFirstWord(line);
    
    /* If there are more characters after the first word, it's not a single word */
    if (*line != '\0') {
        return 0;
    }
    
    return 1;  /* If only one word is found */
}
 

/*
 * onlyTwoWords - Checks whether there are only two words in the given line.
 * @line: The line to be checked.
 * 
 * This function skips leading and trailing whitespace and checks if the line contains 
 * exactly two words. If the line contains more or less than two words, it returns 0.
 * 
 * Return: 1 if the line contains exactly two words, 0 otherwise.
 */
int onlyTwoWords(char *line)
{
	/* skip leading spaces */
	line = skipFirstWord(line);
	
	/* If there is no macro definition */
    if (*line == '\0') {
		printf("\nERROR: Notice! there's no defined name following the macro (\"macr\") definition. \n");
        return 0;
    }

	/* find the end of the second word */
	while (*line && !isspace(*line)) {line++; }
	
	/* skip trailing whitespace after the second word */
	while (*line && isspace(*line)) {line++; }

	/* If there is a third word */
    if (*line != '\0') {
        return 0;
    }

	return 1;
}


/*
 * validMacroName - Checks if the given name is a valid macro name.
 * @name: The name to be checked.
 * 
 * This function compares the given name against a list of invalid macro names 
 * (instructions and directives) and returns 1 if the name is valid, and 0 otherwise.
 * 
 * Return: 1 if the name is valid, 0 otherwise.
 */
int validMacroName(char* name)
{
	/* instructions & directives list */
	char *invalidMacroName[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string", ".entry", ".extern"};

	int i;
	for (i = 0; i < 20; i++) {
		if (strcmp(name, invalidMacroName[i]) == 0) {
			return 0;
		}
	}
	return 1;
}


/*
 * isMacro - Checks if the given word matches a defined macro name.
 * @word: The word to be checked.
 * 
 * This function traverses the macro linked list to check if the given word matches 
 * any macro name. It returns 1 if the word is a defined macro, and 0 otherwise.
 * 
 * Return: 1 if the word is a macro, 0 otherwise.
 */
int isMacro(char *word) 
{
	ptr t = hptr;	
	while (t) {
		if (strcmp(t -> macro_name, word) == 0) {
			return 1;
		}
		t = t -> next;
	}	
	return 0;
}


/*
 * getMacro - Retrieves the content of the given macro name.
 * @macro_name: The name of the macro.
 * 
 * This function traverses the macro linked list to find the node with the given 
 * macro name and returns the macro content. If the macro name is not found, 
 * it returns NULL.
 * 
 * Return: The content of the macro, or NULL if not found.
 */
char *getMacro(char *macro_name) 
{
	ptr t = hptr;	
	while (t) 
	{
		if (strcmp(t -> macro_name, macro_name) == 0) {
			return (t -> macro_content);
		}
		t = t-> next;	
	}		
}


/*
 * get_first_word - Retrieves the first word from the given line.
 * @line: The line to be processed.
 * 
 * This function skips leading whitespace and finds the first word in the line.
 * It copies the first word into a static buffer and returns a pointer to it.
 * 
 * Return: The first word in the line.
 */
char* get_first_word(const char* line) 
{
    /* Skip leading whitespace */
    while (*line && isspace(*line)) {line++; }

    /* Find the end of the first word */
    const char* end = line;
    while (*end && !isspace(*end)) {end++; }
    
    /* Calculate length of the first word */
    size_t length = end - line;
    
	/* Create a static buffer to hold the first word */
    static char first_word[81];  
	if (length >= sizeof(first_word)) {
        printf("Word is too long to fit in the buffer\n");
        return NULL;
    }
    
    /* Copy the first word */
    strncpy(first_word, line, length);
    first_word[length] = '\0';  /* Null-terminate the string */

    return first_word;
}


/*
 * get_second_word - Retrieves the second word from the given line.
 * @line: The line to be processed.
 * 
 * This function skips the first word and leading whitespace, then finds the second 
 * word in the line. It copies the second word into a static buffer and returns a 
 * pointer to it. If there is no second word, it returns an empty string.
 * 
 * Return: The second word in the line, or an empty string if not found.
 */
char* get_second_word(char* line) {
    
	if (findNumOfWords(line) < 2) {
        return ""; /* no second word */
    }

	/* Skip leading whitespace */
    line = skipFirstWord(line);
    
    /* Find the end of the second word */
    const char* end = line;
    while (*end && !isspace(*end)) {end++; }

    /* Calculate length of the second word */
    size_t length = end - line;
    
    /* Create a static buffer to hold the second word */
    static char second_word[80];  
	if (length >= sizeof(second_word)) {
        printf("Word is too long to fit in the buffer\n");
        return NULL;
    }
    
    /* Copy the second word */
    strncpy(second_word, line, length);
    second_word[length] = '\0';  /* Null-terminate the string */
    
    return second_word;
}


/*
 * addMacro - Adds a new macro name to the macros table.
 * @macro_name: The name of the macro to be added.
 * 
 * This function creates a new node for the macro linked list with the given 
 * macro name and initializes its content to an empty string. It inserts the 
 * new node at the end of the list.
 * 
 * Return: 1 on success, 0 on failure.
 */
int addMacro(char *macro_name)
{	
	ptr t = (ptr) malloc(sizeof(m_item));
	if (!t) {
		printf("\nERROR: unable to allocate memory for macro \"%s\".\n", macro_name);
		return 0;
	}
	
	/* inserting the data to each node */
    t -> macro_name = (char *) malloc(strlen(macro_name) + 1);
    if ((t -> macro_name) == NULL) {
        printf("\nERROR: unable to allocate memory for macro name \"%s\".\n", macro_name);
        free(t); /* Clean up allocated memory */
        return 0;
    }
    strcpy(t -> macro_name, macro_name);

	t -> macro_content = (char *) malloc(1); /* Allocate for an empty string */
    if ((t -> macro_content) == NULL) {
        printf("\nERROR: unable to allocate memory for macro content \"%s\".\n", macro_name);
        free(t -> macro_name); /* Clean up allocated memory */
        free(t);
        return 0;
    }
	t -> macro_content[0] = '\0';

	t -> next = NULL;
	
	/* assign the node to the list. */
	if (hptr == NULL) {
		hptr = t; /* if the list is empty, make this the first node */
	} else {
		ptr last = hptr;
		while (last -> next != NULL) {
            last = last -> next;
        }
		last -> next = t;
	}
	return 1;
}


/*
 * addMacroContent - Adds content to the macro with the given name.
 * @line: The content to be added.
 * @macro_name: The name of the macro.
 * 
 * This function finds the node with the given macro name in the macro linked list 
 * and appends the provided content to the macro's existing content. If the macro 
 * content is initially empty, it allocates memory for the new content.
 * 
 * Return: 1 on success, 0 on failure.
 */
int addMacroContent(char *line, char *macro_name) {	
	ptr t = hptr;	
	while (t) 
	{
		if (strcmp(t -> macro_name, macro_name) == 0) 
		{
			/* found the node. */
			size_t new_size = strlen(line) + 1;
			if (t -> macro_content == NULL) 
			{
				t -> macro_content = (char *)malloc(new_size); /* resize the buffer */
				if (t -> macro_content == NULL) {
					printf("\nUnable to allocate memory for macro content.\n");
					return 0;
				}
				strcpy(t->macro_content, line);

			} else {
				new_size += strlen(t -> macro_content); /* resize the buffer, adding the new content*/
				char *new_content = (char *)realloc(t -> macro_content, new_size);
				if (new_content == NULL) {
					printf("\nUnable to reallocate memory for macro content.\n");
					return 0;
				}
				t -> macro_content = new_content;
				strcat(t -> macro_content, line);
			}
			return 1;
		}
		t = t->next;
	}
	return 0;
}


/*
 * freeMacro - Frees all the macros in the macros table.
 * 
 * This function traverses the macro linked list and frees the memory allocated 
 * for each node, including the macro name and content.
 */
void freeMacro() 
{
	ptr p;
	while (hptr) 
	{
		p = hptr;
		hptr = hptr -> next;
		/* free node */
		free(p -> macro_content);
		free(p -> macro_name);	
		free(p);
	}	

	free(hptr);
}