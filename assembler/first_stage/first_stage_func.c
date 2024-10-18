/*
 * first_stage_func.c - This file contains functions that handle the first stage of the assembler process.
 * It includes functions for handling labels, encoding data and instructions into the memory image,
 * and validating instruction formats. The file uses linked lists to manage the data and instruction memory images,
 * ensuring correct processing and referencing during the assembly process.
 */

#include "../../pre_processing/macros_table.h" 
#include "../assembler.h"
#include "../excess_macro_list.h"


/*
 * clearOfMacro - Checks whether there is a macro name or "macr" definition later in the line.
 * @line: The line to be checked.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function traverses the given line to find any occurrence of a macro name or the "macr" definition.
 * If either is found, it prints an error message indicating the presence of a macro and returns 0.
 * If no macro is found, it returns 1.
 * 
 * Return: 1 if no macro is found, 0 otherwise.
 */
int clearOfMacro(char *line, char* file_name, int line_num)
{
	while (*line) 
	{
		char word[LINE_SIZE];
        int length = 0;
        
		/* Skip leading whitespace */
        while (*line && isspace(*line)) line++;
        if (!*line) 
			break;
        
        /* Find the end of the current word */
        while (length < 80 && *line && !isspace(*line)) {
            word[length++] = *line++;
        }
        word[length] = '\0';  /* Null-terminate the string */

        /* Check if the word matches "macr" or a macro name */
        if (strcmp(word, "macr") == 0) {
            printf("\nERROR: in file \"%s\", line %d, there's a \"macr\" defined later in line.\n", file_name, line_num);
            return 0;
        } else if (isMacro(word)) {
            printf("\nERROR: in file \"%s\", line %d, there's a macro name defined later in line.\n", file_name, line_num);
            return 0;
        }
    }
    return 1;
}


/*
 * findDataOrStringWord - Checks if .data or .string exists as the first or second word in the line.
 * @line: The line to be checked.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function traverses the given line to find the .data or .string directive. If found, it returns
 * the found word. If a memory allocation error occurs, it returns "0". If the word is not found, it returns NULL.
 * 
 * Return: The found word if successful, "0" on memory error, NULL if not found.
 */
char *findDataOrStringWord(char *line, char* file_name, int line_num)
{
    char word[81];
    int length = 0;
    char *foundWord = NULL;

    while (*line && isspace(*line)) line++; // Skip leading whitespace

    while (*line) 
    {
        /* Skip leading whitespace */
        while (*line && isspace(*line)) line++;
        if (!*line) 
            break;
        
        /* Find the end of the current word */
        length = 0;
        while (length < 80 && *line && !isspace(*line)) {
            word[length++] = *line++;
        }
        word[length] = '\0';  /* Null-terminate the string */

        /* Check if the word matches .data or .string */
        if (strcmp(word, ".data") == 0 || strcmp(word, ".string") == 0) {

            foundWord = (char*) malloc((strlen(word) + 1) * sizeof(char));
            if (foundWord == NULL) {
                printf("\nERROR: in file \"%s\", line %d, unable to save word of type \"%s\".\n", file_name, line_num, word);
                return "0";
            }
            strcpy(foundWord, word);
            return foundWord;
        }

    }

    return NULL;
}


/*
 * findEntryOrExternWord - Checks if .entry or .extern exists as the first or second word in the line.
 * @line: The line to be checked.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function traverses the given line to find the .entry or .extern directive.
 * 
 * Return: The found word if successful, "0" on memory error, "1" on regular error, NULL if not found.
 */
char *findEntryOrExternWord(char *line, char *file_name, int line_num)
{
    char *first_word = get_first_word(line);
    char *second_word = get_second_word(line);

    int length = 0;
    char *foundWord = NULL;

	/* Check if both .entry and .extern are found */
    if ((strcmp(first_word, ".entry") == 0 && strcmp(second_word, ".extern") == 0) ||
        (strcmp(first_word, ".extern") == 0 && strcmp(second_word, ".entry") == 0)) {

        printf("\nERROR: in file \"%s\", line %d, both \".entry\" and \".extern\" are found.\n", file_name, line_num);
        return "1";
    }

	/* Check if the same word appears twice */
    if ((strcmp(first_word, ".entry") == 0 && strcmp(second_word, ".entry") == 0) ||
        (strcmp(first_word, ".extern") == 0 && strcmp(second_word, ".extern") == 0)) {

        printf("\nERROR: in file \"%s\", line %d, \".entry\" or \".extern\" appear twice.\n", file_name, line_num);
        return "1";
    }

	/* when found .entry */
    if (strcmp(first_word, ".entry") == 0 || strcmp(second_word, ".entry") == 0) {

        length = strlen(".entry") + 1;
        foundWord = (char *)malloc(length * sizeof(char));
        if (foundWord == NULL) {
            printf("\nERROR: in file \"%s\", line %d, unable to save word of type \".entry\".\n", file_name, line_num);
        	return "0";
        }
        strcpy(foundWord, ".entry");
    }
	/* when found .extern */
    else if (strcmp(first_word, ".extern") == 0 || strcmp(second_word, ".extern") == 0) {

        length = strlen(".extern") + 1;
        foundWord = (char *)malloc(length * sizeof(char));
        if (foundWord == NULL) {
            printf("\nERROR: in file \"%s\", line %d, unable to save word of type \".extern\".\n", file_name, line_num);
        	return "0";
        }
        strcpy(foundWord, ".extern");
    }

    return foundWord; /* or NULL if wasn't found */
}



/*
 * isLabel - Checks whether the line defines a label.
 * @line: The line to be checked.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function checks if the line defines a label.
 * 
 * Return: 1 if it is a label, 0 if not a label, 2 if invalid label, 3 if .extern or .entry.
 */
int isLabel(char *line, char* file_name, int line_num) 
{
	/* side cases */
	char *second_word = get_second_word(line);
	if (strcmp(second_word, ".extern") == 0 || strcmp(second_word, ".entry") == 0) {
		return 3;
	}

	/* skip leading whitespaces */
	while (*line && isspace(*line)) {line++; }
	
	/* Find the end of the possible label word */
    const char* end = line;
    while (*end && !isspace(*end)) {end++; }

	/* suspecting a possible label */
	if (*(end-1) == ':') {
		/* Calculate length of the label */
   	 	size_t length = (end - line) - 1;

		/* Create a static buffer to hold the label word */
   		static char label[31];  
		if (length > sizeof(label)) {
      	  printf("\nERROR: in file \"%s\", line %d, the label length exceeds the limit.\n", file_name, line_num);
      	  return 2;
    	}

		strncpy(label, line, length);
    	label[length] = '\0';  /* Null-terminate the string */

		/* Make sure that the label is valid: */
		if (!isalpha(label[0])) {
			printf("\nERROR: in file \"%s\", line %d, the label definition is invalid.\n", file_name, line_num);			
			return 2;
		}
		
		/* check label name is valid */
		char *invalidlabelName[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string", ".entry", ".extern", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

		int i;
		for (i = 0; i < 28; i++) {
			if (strcmp(label, invalidlabelName[i]) == 0) {
				printf("\nERROR: in file \"%s\", line %d, the label definition is invalid.\n", file_name, line_num);				
				return 2;
			}
		}
	
		/* check that the label name isn't a macro name */
		if (isMacro(label) == 1) {
			printf("\nERROR: in file \"%s\", line %d, the label definition is matched to a macro name.\n", file_name, line_num);
			return 2;
		}
		
		return 1; /* successfuly found a new VALID label */
	
	} else {

		/* check if ':' is far from the end of the word */
		while (*end && isspace(*end)) {end++; }

		if (*end == ':') {
			printf("\nERROR: in file \"%s\", line %d, the label is wrongly defined.\n", file_name, line_num);
			return 2;
		}
	}

	return 0; /* not a label at all */
}


/*
 * isAlreadyLabel - Checks whether the given label name has already been defined.
 * @word: The label name to be checked.
 * 
 * This function traverses the label list to check if the given label name has already been defined.
 * 
 * Return: 1 if the label is found, 0 if not found.
 */
int isAlreadyLabel(char *word)
{
	Lptr t = Labelhptr;	
	while (t) {
		if (strcmp(t -> label_name, word) == 0) {
			return 1;
		}
		t = t -> next;
	}	
	return 0;
}


/*
 * getLabelStatus - Returns the status of the given label name.
 * @word: The label name to be checked.
 * 
 * This function traverses the label list to find the given label name and returns its type.
 * 
 * Return: The type of the label if found, NULL if not found.
 */
char *getLabelStatus(char *word)
{
    Lptr t = Labelhptr;	
	while (t) {
		if (strcmp(t -> label_name, word) == 0) {
			return t -> type;
		}
		t = t -> next;
	}	
	return NULL;
}


/*
 * getLabelAddress - Returns the address of the given label name.
 * @word: The label name to be checked.
 * 
 * This function traverses the label list to find the given label name and returns its address.
 * 
 * Return: The address of the label if found, -1 if not found.
 */
int getLabelAddress(char *word)
{
    Lptr t = Labelhptr;	
	while (t) {
		if (strcmp(t -> label_name, word) == 0) {
			return t -> value;
		}
		t = t -> next;
	}	
	return -1;
}


/*
 * validInstructionName - Checks if the given word is a valid instruction name.
 * @first_word: The word to be checked.
 * 
 * This function compares the given word against a list of valid instruction names.
 * 
 * Return: 1 if the word is a valid instruction name, 0 if not.
 */
int validInstructionName(char *first_word) 
{
	char *instructionWords[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"};
	int i;
	for (i = 0; i < 16; i++) {
		if (strcmp(first_word, instructionWords[i]) == 0) {
			return 1;
		}
	}
	return 0;
}


/*
 * validInstructionAddress - Checks if the given instruction's addressing type is valid.
 * @instructionType: The type of the instruction.
 * @addressingType: The addressing type to be checked.
 * 
 * This function compares the given instruction type and addressing type against a predefined table
 * of valid instruction-addressing type combinations. 
 * 
 * Return: 1 if the combination is valid, 0 if not.
 */
int validInstructionAddress(char *instructionType, int addressingType)
{
	#define NONE -1
	int i;

	struct table {
		char *name;
		int type[4];
	};

	struct table list[] = {
		{"clr", {1, 2, 3, NONE}},
		{"not", {1, 2, 3, NONE}},
		{"inc", {1, 2, 3, NONE}},
		{"dec", {1, 2, 3, NONE}},
		{"jmp", {1, 2, NONE, NONE}},
		{"bne", {1, 2, NONE, NONE}},
		{"red", {1, 2, 3, NONE}},
		{"prn", {0, 1, 2, 3}},
		{"jsr", {1, 2, NONE, NONE}}
	};

	for (i = 0; i < 9; i++) {
		if (strcmp(instructionType, list[i].name) == 0) {
			int j;
			for (j = 0; j < 4; j++) {

				if (addressingType == list[i].type[j]) {
					return 1;
				}
			}
		}
	}

    return 0;
}


/*
 * valid2operandsAddress - Checks if the given instruction's two operand addressing types are valid.
 * @instructionType: The type of the instruction.
 * @firstAddressingType: The addressing type of the first operand.
 * @secondAddressingType: The addressing type of the second operand.
 * 
 * This function compares the given instruction type and the addressing types of its two operands
 * against a predefined table of valid combinations.
 * 
 * Return: 1 if the combination is valid, 0 if not.
 */
int valid2operandsAddress(char *instructionType, int firstAddressingType, int secondAddressingType)
{
	#define NONE -1
	int count = 0;
	int i, j, k;

	struct table {
		char *name;
		int first_addressing[4];
		int second_addressing[4];
	};

	struct table list[] = {
		{"mov", {0, 1, 2, 3}, {NONE, 1, 2, 3}},
		{"cmp", {0, 1, 2, 3}, {0, 1, 2, 3}},
		{"add", {0, 1, 2, 3}, {NONE, 1, 2, 3}},
		{"sub", {0, 1, 2, 3}, {NONE, 1, 2, 3}},
		{"lea", {NONE, 1, NONE, NONE}, {NONE, 1, 2, 3}}
	};
	
	for (i = 0; i < 5; i++) {
		/* when finding the instruction type */
		if (strcmp(instructionType, list[i].name) == 0) {
			
			for (j = 0; j < 4; j++) {
				if (firstAddressingType == list[i].first_addressing[j]) {
					count++; /* found correct first addressing type */
				}
			}

			for (k = 0; k < 4; k++) {
				if (secondAddressingType == list[i].second_addressing[k]) {
					count++; /* found a correct second addressing type */
				}
			}
		}
	}

	if (count == 2) {
		return 1;
	}
    return 0;
}


/*
 * validOperandAddress - Checks if the given operand's addressing type is valid for the instruction.
 * @instructionType: The type of the instruction.
 * @AddressingType: The addressing type to be checked.
 * @source_or_target: Whether the operand is a source or target.
 * 
 * This function compares the given instruction type and operand addressing type against a predefined table
 * of valid combinations for source and target operands.
 * 
 * Return: 1 if the combination is valid, 0 if not.
 */
int validOperandAddress(char *instructionType, int AddressingType, char *source_or_target)
{	
	/* in case it may be a future label */
	if (AddressingType == -2) {
		return 1;
	}

	#define NONE -1
	int count = 0;
	
	struct table {
		char *name;
		int source_addressing[4];
		int target_addressing[4];
	};

	struct table list[] = {
		{"mov", {0, 1, 2, 3}, {NONE, 1, 2, 3}},
		{"cmp", {0, 1, 2, 3}, {0, 1, 2, 3}},
		{"add", {0, 1, 2, 3}, {NONE, 1, 2, 3}},
		{"sub", {0, 1, 2, 3}, {NONE, 1, 2, 3}},
		{"lea", {NONE, 1, NONE, NONE}, {NONE, 1, 2, 3}}
	};

	int i, j, k;
	for (i = 0; i < 5; i++) {
		/* when finding the instruction type */
		if (strcmp(instructionType, list[i].name) == 0) {
			
			if (strcmp(source_or_target, "source") == 0) {
				for (j = 0; j < 4; j++) {
					if (AddressingType == list[i].source_addressing[j]) {
						count++; /* found correct addressing type */
					}
				}
			}
			
			else if (strcmp(source_or_target, "target") == 0) {
				for (k = 0; k < 4; k++) {
					if (AddressingType == list[i].target_addressing[k]) {
						count++; /* found correct addressing type */
					}
				}
			}

		}
	}

	if (count == 1) {
		return 1;
	}
    return 0;
}


/*
 * replaceCommas - Replaces commas with whitespaces in the given line.
 * @line: The line to be processed.
 * 
 * This function replaces commas in the given line with whitespaces to ensure proper
 * separation of operands. 
 * 
 * Return: 1 if the commas are managed correctly, 0 if not.
 */
int replaceCommas(char *line)
{
	
	int comma_found = 0;
	int last_comma_found = 0;
	line = skipFirstWord(line);
	
	if (*line == ',') {
		return 0; /* comma at the beginning */
	}

	while (*line) {
		
		comma_found = 0;
		while (*line && !isspace(*line) && *line != ',') {line++; }
		if (*line == ',') {
			*line = ' '; /* replace comma with whitespace */
			comma_found++;
			line++;
		}

		while (*line && isspace(*line)) {line++; }
		
		if (*line != ',' && comma_found == 0 && *line != '\0') {
			return 0; /* no comma seperating at all */
		}

		if (*line == ',' && comma_found == 1) {
			return 0; /* second comma */
		}
	}

	if (comma_found == 1) {
		return 0; /* comma at the end of the last word */
	}
	
    return 1; /* Success */
}


/*
 * addLabel - Adds the given label name to the label table.
 * @label_name: The name of the label to be added.
 * @value_num: The value associated with the label.
 * @instructionWord: The type of the instruction.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function creates a new node for the label linked list with the given label name, value,
 * and instruction type. It inserts the new node at the end of the list.
 * 
 * Return: 1 on success, 0 on memory failure.
 */
int addLabel(char *label_name, int value_num, char *instructionWord, char *file_name, int line_num)
{
	Lptr t = (Lptr) malloc(sizeof(l_item));
	if (!t) {
		printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_name);
		return 0;
	}
	
	/* insert label name */
    t -> label_name = (char *) malloc(strlen(label_name) + 1);
    if ((t -> label_name) == NULL) {
        printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_name);
        free(t); /* Clean up allocated memory */
        return 0;
    }
    strcpy(t -> label_name, label_name);

	/* insert value */
	t -> value = value_num;
	
	/* insert type name */
	t -> type = (char *) malloc(strlen(instructionWord) + 1);
	if ((t -> type) == NULL) {
        printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_name);
		free(t -> label_name);        
		free(t); /* Clean up allocated memory */
        return 0;
    }
    strcpy(t -> type, instructionWord);
		
	t -> next = NULL;
	
	/* assign the node to the list. */
	if (Labelhptr == NULL) {
		Labelhptr = t; /* if the list is empty, make this the first node */
	} else {
		Lptr last = Labelhptr;
		while (last -> next != NULL) {
            last = last -> next;
        }
		last -> next = t;
	}
	return 1;
}


/*
 * NOTICE: this is a helper function which has no use in the program.
 * printLabel - Prints the entire Label-Table data line by line.
 * This function traverses the label linked list and prints each label's name, value, and type.
 */
void printLabel()
{
	printf("Label Table:\n");
	/* traversing the linked list */
    Lptr p = Labelhptr;
    while (p) {
		printf("(%d) ", p -> value);
        printf("%s - ", p -> label_name);
		printf("[%s]", p -> type);
        printf("\n");
        p = p->next;
    }   
}


/*
 * updateLabels - Updates all labels with type .data by adding IC+100 to their value.
 * @IC: The instruction counter value to be added.
 * 
 * This function traverses the label linked list and updates the value of labels with type .data
 * or .string by adding IC+100.
 */
void updateLabels(int IC)
{
	/* traversing the linked list */
    Lptr p = Labelhptr;
    while (p) {
		if (strcmp(p -> type, ".data") == 0 || strcmp(p -> type, ".string") == 0) {
			p -> value += IC + 100;
		}
        p = p->next;
    }  
}


/*
 * freeLabel - Frees all the labels in the label table.
 * 
 * This function traverses the label linked list and frees the memory allocated for each node,
 * including the label name and type.
 */
void freeLabel()
{
	Lptr p;
	while (Labelhptr) 
	{
		p = Labelhptr;
		Labelhptr = Labelhptr -> next;
		/* free node */
		free(p -> label_name);	
		free(p -> type);
		free(p);
	}	

	free(Labelhptr);
}


/*
 * encodeData - Encodes the data in the given line to the data image.
 * @line: The line containing the data to be encoded.
 * @DC_address: The current data counter address.
 * @dataType: The type of data (.data or .string).
 * @LABEL_FLAG: Indicates whether a label is present in the line.
 * 
 * This function encodes the data in the given line to the data image, incrementing the data counter
 * accordingly. It handles both .data and .string types, ensuring proper formatting and error checking.
 * 
 * Return: The value by which DC needs to be incremented on success, -1 on regular error, -2 on memory error.
 */
int encodeData(char *line, int DC_address, char *dataType, int LABEL_FLAG)
{
	/* recognize .data OR .string */
	if (strcmp(dataType, ".data") == 0) 
	{
		#define MAX_NUMBER 32767 /* equal to [111111111111111] */
		int numberFound = 0;

		/* found a .data */
		if (LABEL_FLAG == 1) {
			/* skip label word */
			line = skipFirstWord(line);
			while (*line && !isspace(*line)) {line++; } /* end of ".data" word */
			while (*line && isspace(*line)) {line++; }
		}
		else {
			/* skip .data word */
			line = skipFirstWord(line);
		}
				
		/* start of the 3rd word */
		if (*line == ',') {
			return -1; /* Error: comma before the first number */
		}

		while (*line) 
		{
		    /* Extract the number */
		    char numberBuffer[MAX_NUMBER];
		    int i = 0;
			
			/* Skip leading spaces */
		    while (*line && isspace(*line)) { line++; }

			/* exit if there is no number coming next */
			if (*line != '+' && *line != '-' && !isdigit(*line)) {
				return -1; /* Error: non-digit value after .data definition  */
			}
			/* Skip the optional + sign */
			if (*line == '+') {
            	line++;
        	}

			/* Include the - sign if present */
        	if (*line == '-') {
          	  	numberBuffer[i++] = *line++; 
       		}

			/* Read the digits of the number */
		    while (*line && isdigit(*line)) {
            	numberBuffer[i++] = *line++;
        	}
		    numberBuffer[i] = '\0'; /* Null-terminate the string */

		    /* Convert the number to an integer */
		    if (i > 0) {
		        int number = atoi(numberBuffer);
				/* check if numebr exceeds limit */
				if (number > MAX_NUMBER || number < -MAX_NUMBER) {
                    return -1; /* Error: number out of range */
                }
	
				numberFound = 1;
		 
				/* Encode the number to the data memory image: */
				if (addData(number, DC_address) == 0) {
					return -2; /* memory error */
				}
				
				DC_address++; /* increment DC after reading a number */
        	}
		
		    /* Skip trailing spaces */
		    while (*line && isspace(*line)) {line++; }

			/* skip the next comma and handle consecutive commas */
		    if (*line == ',') {
		        line++; /* Skip the comma */
		        while (*line && isspace(*line)) { line++; } /* Skip any spaces after the comma */
		        if (*line == ',' || (!isdigit(*line) && *line != '-' && *line != '+')) {
		            return -1; /* Error: two consecutive commas or comma without a following number */
		        }
		    }
		}

		if (!numberFound) {
        	return -1; /* Error: no numbers found at all */
    	}
    }
	
	/* recognizes a .string */
	else if (strcmp(dataType, ".string") == 0) 
	{
		int quoteCount = 0;
    	char *ptr = line;
		
		/* found a .string */
		if (LABEL_FLAG == 1) {
			/* skip label word */
			line = skipFirstWord(line);
			while (*line && !isspace(*line)) {line++; } /* end of ".data" word */
			while (*line && isspace(*line)) {line++; }
		}
		else {
			/* skip .data word */
			line = skipFirstWord(line);
		}

		/* Check for valid string format */
    	while (*ptr) {
    	    if (*ptr == '"') {
     	       quoteCount++;
      	  	}
      	  ptr++;
   	 	}
		    
    	if (quoteCount != 2) {
      	  return -1; /* Error: invalid number of quotes */
    	}

		/* now pointing at the start of the string */
	    if (*line != '"') {
	        return -1;
	    }

		line++; /* move past the first quote */
		
		/* Check for an empty string */
    	if (*line == '"') {
        	return -1; /* ERROR: empty string */
    	}

		/* load each character into the memory image */
    	while (*line) 
    	{
        	if (*line == '"') {
            	break; /* found the closing quote */
        	}

        	/* Encode the character to the memory-image: */
        	if (addString(*line, DC_address) == 0) {
            	return -2; /* memory error */
      		}

        	DC_address++; /* increment DC after reading a character */
        	line++;
    	}

		/* put a closing '\0' for string */
		if (addString('\0', DC_address) == 0) {
            	return -2; /* memory error */
      	}
		DC_address++;

		line++; /* move past the closing quote */

		/* Check if there are trailing characters after the closing quote */
    	while (*line && isspace(*line)) { line++; }

    	if (*line) {
        	return -1; /* Error: unexpected characters after the closing quote */
    	}
	}

	return DC_address; /* updated DC counter */
}


/*
 * addSpacesAfterCommas - Adds spaces after commas in the given line.
 * @line: The line to be processed.
 * 
 * This function adds spaces after commas in the given line to ensure proper separation of operands.
 * It returns the modified line as a static buffer.
 * 
 * Return: The modified line with spaces after commas.
 */
char *addSpacesAfterCommas(char *line)
{
	static char buffer[LINE_SIZE];  /* Static buffer to hold the modified string */
    int j = 0;

    /* Iterate through the input string */
    for (int i = 0; line[i] != '\0'; i++) {
        if (j >= LINE_SIZE - 1) {  /* Check if the buffer size is exceeded */
            fprintf(stderr, "Error: Buffer size exceeded\n");
            buffer[LINE_SIZE - 1] = '\0';  /* Ensure the buffer is null-terminated */
            return buffer;
        }

        buffer[j++] = line[i];  /* Copy the current character */

        /* If the current character is a comma, add a space after it */
        if (line[i] == ',') {
            if (j >= LINE_SIZE - 1) {  /* Check if the buffer size is exceeded */
                fprintf(stderr, "Error: Buffer size exceeded\n");
                buffer[LINE_SIZE - 1] = '\0';  /* Ensure the buffer is null-terminated */
                return buffer;
            }
            buffer[j++] = ' ';
        }
    }
    buffer[j] = '\0';  /* Null-terminate the buffer */

    return buffer;
}


/*
 * encodeInstruction - Encodes the instruction in the given line to the instruction image.
 * @line: The line containing the instruction to be encoded.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @LABEL_FLAG: Indicates whether a label is present in the line.
 * @IC: The current instruction counter address.
 * 
 * This function encodes the instruction in the given line to the instruction image, incrementing the instruction counter
 * accordingly. It handles various types of instructions, ensuring proper formatting and error checking.
 * 
 * Return: The value by which L needs to be incremented on success, -1 on regular error, -2 on memory error.
 */
int encodeInstruction(char *line, char *file_name, int line_num, int LABEL_FLAG, int IC)
{
	int L = 0; /* reset word counter */
	int instruction_length = 0;
	int opcode;
	int i;
	char *instruction_word;
	char *temp;
	int err_type;

	/* Define data about every instruction. */
	ins_length instructionType[] = {
        {"mov",  2}, /* the opcode of the instruction is sorted by index */
        {"cmp",  2},
        {"add",  2},
        {"sub",  2},
        {"lea",  2},
        {"clr",  1},
        {"not",  1},
        {"inc",  1},
        {"dec",  1},
        {"jmp",  1},
        {"bne",  1},
        {"red",  1},
        {"prn",  1},
        {"jsr",  1},
        {"rts",  0},
        {"stop", 0}
	};

	/* add spaces after detecting commas */
	line = addSpacesAfterCommas(line);
	
	/* in case there is a label */
	if (LABEL_FLAG == 1) {
		line = skipFirstWord(line); /* skip label word */
	}
	
	/* replace all commas with whitespaces */
	if (replaceCommas(line) == 0) {

		printf("\nERROR: in file \"%s\", line %d, the commas aren't managed accordingly.\n", file_name, line_num);
		return -1; /* commas aren't managed accordingly */
	}

	instruction_word = get_first_word(line); /* get instruction type */
	
	/* calculate word-length of the instrution sentence. */
	temp = line;
	temp = skipFirstWord(temp); /* skip instruction word */
	L += 1;

	/* Now temp points to the first operand or end of line */
	while (*temp && *temp != '\n') {
		temp = skipFirstWord(temp);  
		instruction_length++;
        L++; /* counting the num of words */
    }
	
	/* check if the num of operands of the instruction are valid */
	for (i = 0; i < 16; i++) {
		if (strcmp(instruction_word, instructionType[i].instruction) == 0) {
			
			
			if (instruction_length != instructionType[i].operand_num) {
				printf("\nERROR: in file \"%s\", line %d, the instruction operand length is invalid.\n", file_name, line_num);
				return -1;
			}

			opcode = i;
			break;
		}
	}

	/* recognize the type of instruction and encode it to the instruction-image */
	err_type = addInstruction(file_name, line_num, line, instruction_word, instruction_length, IC, opcode);
	
	if (err_type == 0) {
		printf("\nERROR: in file \"%s\", line %d, unable to allocate memory for instruction.\n", file_name, line_num);
		return -2;
	}
	else if (err_type == 2) {
		return -1;
	}

	return L;
}


/*
 * encodeMila - Encodes a mila (word) for the given operand.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @first_adressing_type: The addressing type of the operand.
 * @operand: The operand to be encoded.
 * @line: The line containing the operand.
 * @IC: The current instruction counter address.
 * @operandType: Indicates whether the operand is a source or target.
 * 
 * This function encodes a mila (word) for the given operand based on its addressing type. It handles various
 * addressing types and updates the instruction memory image accordingly. 
 * 
 * Return: 1 on success, 2 on error while encoding.
 */
int encodeMila(char *file_name, int line_num, int first_adressing_type, char *operand, char *line, int IC, char *operandType)
{
	/* addressing type 0 */
	if (first_adressing_type == 0) 
	{
		int num;
		operand++; /* skip the # sign */
		num = atoi(operand);
		if (num > 4095) {
			printf("\nERROR: in file \"%s\", line %d, the operand numebr is too big.\n", file_name, line_num);
			return 2; /* number is too big */
		}
		/* create the second mila */
		ALLOCATE_NODE; /* returns 0 when memory-error */
		CREATE_AND_RESET_MILA; 
		space.MILA |= (1 << 2); /* set A in ARE to 1 */
		space.MILA |= (num << 3); /* add num between 3-14 bits */

		/* complete node */
		insertDataToNode(t, space, IC); 
		assignNodeToList(t);	
		return 1;			
	}	
	/* addressing type 1 */
	else if (first_adressing_type == 1) 
	{
		char *labelStatus = getLabelStatus(operand); /* get the label type ie. ".external" etc... */
		int Labeladdress = getLabelAddress(operand);
		if (Labeladdress == -1) {
			return 1; /* label wasn't found */
		}
	
		/* create the second mila */
		ALLOCATE_NODE; /* returns 0 when memory-error */		
		CREATE_AND_RESET_MILA;
		if (strcmp(labelStatus, ".external") == 0) {
			space.MILA |= 1; /* set E in ARE to 1 */
			/* complete node */
			insertDataToNode(t, space, IC); 
			assignNodeToList(t);
			return 1;
		}
		else {
			space.MILA |= (1 << 1);; /* set R in ARE to 1 */
			space.MILA |= ((Labeladdress) << 3); /* add label addrress between 3-14 bits */
			/* complete node */
			insertDataToNode(t, space, IC); 
			assignNodeToList(t);
			return 1;
		}
	}

	/* addressing type 2 */
	else if (first_adressing_type == 2) 
	{
		int registerNum;
		operand += 2; /* skip the * and the letter of the register */

		registerNum = atoi(operand);
		if (registerNum > 7) {
			printf("\nERROR: in file \"%s\", line %d, the register numebr is too big.\n", file_name, line_num);
			return 2; /* if num is bigger than 111 or 7 */
		}
				
		/* create the second mila */
		ALLOCATE_NODE;
		CREATE_AND_RESET_MILA; /* returns 0 when memory-error */
		space.MILA |= (1 << 2); /* set A in ARE to 1 */
				
		if (strcmp(operandType, "target") == 0) {
			space.MILA |= (registerNum << 3); /* add the register num between 3-5 bits */
		}	
		else if (strcmp(operandType, "source") == 0) {
			space.MILA |= (registerNum << 6); /* add the register num between 3-5 bits */
		}

		/* complete node */
		insertDataToNode(t, space, IC); 
		assignNodeToList(t);
		return 1;
	}

	/* addressing type 3 */
	else if (first_adressing_type == 3) 
	{
		int registerNum;
		operand++; /* skip the letter of the register */

		registerNum = atoi(operand);
		if (registerNum > 7) {
			printf("\nERROR: in file \"%s\", line %d, the register numebr is too big.\n", file_name, line_num);
			return 2; /* if num is bigger than 111 or 7 */
		}

		/* create the second mila */
		ALLOCATE_NODE;
		CREATE_AND_RESET_MILA; /* returns 0 when memory-error */
		space.MILA |= (1 << 2); /* set A in ARE to 1 */
				
		if (strcmp(operandType, "target") == 0) {
			space.MILA |= (registerNum << 3); /* add the register num between 3-5 bits */
		}	
		else if (strcmp(operandType, "source") == 0) {
			space.MILA |= (registerNum << 6); /* add the register num between 3-5 bits */
		}
		
		/* complete node */
		insertDataToNode(t, space, IC); 
		assignNodeToList(t);
		return 1;
	}
}


/*
 * addInstruction - Encodes the given instruction into the instruction memory image.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @line: The line containing the instruction to be encoded.
 * @instructionType: The type of the instruction.
 * @InstructionLength: The length of the instruction.
 * @IC: The current instruction counter address.
 * @opcode: The opcode of the instruction.
 * 
 * This function encodes the given instruction into the instruction memory image, handling different
 * types of instructions based on their operand lengths. It performs error checking and updates the instruction
 * memory image accordingly.
 * 
 * Return: 1 on success, 0 on memory error, 2 on error while loading instruction.
 */
int addInstruction(char *file_name, int line_num, char *line, char *instructionType, int InstructionLength, int IC, int opcode)
{
	/* recognize the instructions by their operand length: */
	switch (InstructionLength)
	{
		/* instructions with opcode of 14-15 (0 operands) */
		case 0: 
		{   
			ALLOCATE_NODE; /* return 0 when unable to allocate mem */
			CREATE_AND_RESET_MILA; /* create memory cell and reset it */
			space.MILA |= (1 << 2); /* set A in ARE to 1 */
			opcode <<= 11; /* add the OPCODE to the correct position */
			space.MILA |= opcode;
			insertDataToNode(t, space, IC); /* add address and set next/prev to null */
			assignNodeToList(t);; /* assign the node to the instruction list */
			break;	
		}
		
		/* instructions with opcode of 5-13 (1 operands) */
		case 1:
		{	
			int first_adressing_type;
			char *operand;
			int AddressErrType;
			int invalid_instr_err;
			int addressing_place;
			int encode_err_type;

			/* create the first "mila" */
			ALLOCATE_NODE;
			CREATE_AND_RESET_MILA; /* returns 0 when memory-error */
			space.MILA |= (1 << 2); /* set A in ARE to 1 */
			opcode <<= 11; /* add the OPCODE to the correct position */
			space.MILA |= opcode; 

			/* recognize the addressing type and load it, then check for errors */
			operand = get_second_word(line);
			first_adressing_type = getAddressingType(file_name, line_num, operand, instructionType);
			
			/* if func "getAddressingType" returns (-1): Completes node processing and returns 2 (error).
			   if func "getAddressingType" returns (-2): Updates MILA with label-addressing, completes node processing, and returns 1 (invalid operand or future label). */
			AddressErrType = checkERRloadLabelADRRtype(first_adressing_type, space, IC, t); 
			if (AddressErrType != 0) {
				return AddressErrType;
			}
			
			/* now check if the addressing type of the instruction is valid, instructions that are calling for later defined labels will be dealt with on the second stage */
			invalid_instr_err = validInstructionAddress(instructionType, first_adressing_type);
			if (invalid_instr_err == 0) { 
				insertDataToNode(t, space, IC); 
				assignNodeToList(t); 
				printf("\nERROR: in file \"%s\", line %d, the adressing type of the instruction %s is invalid.\n", file_name, line_num, instructionType);
				return 2; /* return 2 if invalid addressing type */
			}  
			
			/* assign the addressing type to target operand at the "info mila" */
			addressing_place = (1 << (first_adressing_type + 3));
			space.MILA |= addressing_place; 
			/* complete node */
			insertDataToNode(t, space, IC); 
			assignNodeToList(t);

			/* encode the second mila according to the addressing type */
			IC++;
			encode_err_type = encodeMila(file_name, line_num, first_adressing_type, operand, line, IC, "target");
			if (encode_err_type == 2) { 
				return 2; /* loading error */ 
			} 
			else if (encode_err_type == 1) { 
				return 1; /* success or may be a future label  */ 
			}	
		}
			
		/* instructions with opcode of 0-4 (2 operands) */
		case 2:
		{
			int FIRST_IS_FUTURE_LABEL = 0;
			int SECOND_IS_FUTURE_LABEL = 0;
			char instructionTypeBuffer[strlen(instructionType)]; 
			char *first_operand;
			int first_adressing_type;
			char *second_operand;
			int second_addressing_type;
			int checkValidOperand;
			int encode_err_type1;
			int encode_err_type2;

			/* create the first mila */
			ALLOCATE_NODE;
			CREATE_AND_RESET_MILA; /* returns 0 when memory-error */
			space.MILA |= (1 << 2); /* set A in ARE to 1 */
			opcode <<= 11;
			space.MILA |= opcode; /* add the OPCODE */
			
			/* replace commas was here */

			/* skip the instruction word */
			line = skipFirstWord(line);

			/* save instruction-type word as a buffer */
			strncpy(instructionTypeBuffer, instructionType, strlen(instructionType));
			instructionTypeBuffer[sizeof(instructionTypeBuffer)] = '\0'; 

			/* get the FIRST OPERAND addressing type */
			first_operand = get_first_word(line);

			first_adressing_type = getAddressingType(file_name, line_num, first_operand, instructionType);
			if (first_adressing_type == -1) {

		        insertDataToNode(t, space, IC); /* complete node */
				assignNodeToList(t);
				printf("\nERROR: in file \"%s\", line %d, the operand of type \"%s\" has no matching adressing type.\n", file_name, line_num, first_operand);
				return 2; 
			}
			else if (first_adressing_type == -2) {

				/* either invalid operand or a future label:
				in case it's a future label, the SECOND addressing type can still be encoded, and then leave. */
				FIRST_IS_FUTURE_LABEL = 1;
			}
			
			/* get the SECOND OPERAND addressing type */
			second_operand = get_second_word(line);
			second_addressing_type = getAddressingType(file_name, line_num, second_operand, instructionType);
			if (second_addressing_type == -1) {
		        insertDataToNode(t, space, IC); /* complete node */
				assignNodeToList(t);
				printf("\nERROR: in file \"%s\", line %d, the operand of type \"%s\" has no matching adressing type.\n", file_name, line_num, second_operand);
				return 2; /* error while loading instruction */
			}
			else if (second_addressing_type == -2) {
				
				/* either invalid operand or a future label
				in case it's a future label, the FIRST addressing type can still be encoded, and then leave. */
				SECOND_IS_FUTURE_LABEL = 1;
			}
			
			/* Check that each of the operands addressing type match the instructions, and add addressing type occordingly on the info mila: */
			checkValidOperand = checkValidOperands(FIRST_IS_FUTURE_LABEL, SECOND_IS_FUTURE_LABEL, instructionTypeBuffer, first_adressing_type, second_addressing_type);
			if (checkValidOperand == 2) {
				insertDataToNode(t, space, IC); /* complete node */
				assignNodeToList(t);
				printf("\nERROR: in file \"%s\", line %d, invalid operands make wrong addressing type for this instruction.\n", file_name, line_num);
				return 2; /* ERROR: invalid addressing types for the instructions */
			}
			/* two of the operands are either invalid or a future label, exit and take care of the rest milas in the second stage */
			else if (checkValidOperand == 1) {

				int source_addressing_place;
				int target_addressing_place;

				/* Add addressing of labels to MILA */				
				source_addressing_place = (1 << (1 + 7));
				space.MILA |= source_addressing_place; // Insert source
								
				target_addressing_place = (1 << (1 + 3));
				space.MILA |= target_addressing_place; // Insert target
				
				/* complete node */
				insertDataToNode(t, space, IC);
				assignNodeToList(t);
				return 1; /* two of the operands are either invalid or a future label, exit and take care of the rest milas in the second stage */
			}
			
			/* deal with inserting the addressing types in their correct position */
			if (FIRST_IS_FUTURE_LABEL == 0) { /* making sure to not enter a false value */				
				int source_addressing_place = (1 << (first_adressing_type + 7));
				space.MILA |= source_addressing_place; /* insert source */
			}
			if (SECOND_IS_FUTURE_LABEL == 0) {
				int target_addressing_place = (1 << (second_addressing_type + 3));
				space.MILA |= target_addressing_place; /* insert target */
			}
	
			/* insert source or target adressing types just in case it's a future label */
			if (FIRST_IS_FUTURE_LABEL) {
				int source_addressing_place = (1 << (1 + 7));
				space.MILA |= source_addressing_place; /* insert source */
			} 
			if (SECOND_IS_FUTURE_LABEL) {
				int target_addressing_place = (1 << (1 + 3));
				space.MILA |= target_addressing_place; /* insert target */
			}

			/* complete info mila node */
		    insertDataToNode(t, space, IC);
			assignNodeToList(t); /* close the info mila */	
			
			/* SPECIAL CASE: encoding the second operand only (third mila), when the first operand may be unknown label, but the second operand is okay */
			if (FIRST_IS_FUTURE_LABEL && SECOND_IS_FUTURE_LABEL == 0) 
			{
				int encode_err_type;
				IC += 2;
				encode_err_type = encodeMila(file_name, line_num, second_addressing_type, second_operand, line, IC, "target");
				if (encode_err_type == 2) { 
					return 2; /* loading error */ 
				} 
				return 1; /* success or future label  */ 
			}

			/* After making the info mila, now creating the second mila: */
			IC++; /* incrementing the IC for the next mila. */
			
			/* SPECIAL CASE - when two of the adressing types are either 2 or 3, make just one more mila and then exit */
			if ((first_adressing_type == 2 || first_adressing_type == 3) && (second_addressing_type == 2 || second_addressing_type == 3)) 
			{	
				if (encodeRegisterMilaOnly(file_name, line_num, line, IC) == 0) { /* encoding the mila to instruction-memory */
					return 2; /* error while loading instruction */
				}
				return 1;
			}

			/* Encoding the FIRST OPERAND, second mila to memory: */
			encode_err_type1 = encodeMila(file_name, line_num, first_adressing_type, first_operand, line, IC, "source");
			if (encode_err_type1 == 2) { 
				return 2; /* error while loading instruction */ 
			} 

			/* check in case the second operand may be a future label, then skip the encoding and leave it for the second stage: */
			if (SECOND_IS_FUTURE_LABEL) 
				return 1;
			
			/* Encoding the SECOND OPERAND, third mila to memory: */
			IC++;
			encode_err_type2 = encodeMila(file_name, line_num, second_addressing_type, second_operand, line, IC, "target");
			if (encode_err_type2 == 2) { 
				return 2; /* error while loading instruction */ 
			} 

			/* end of case 2 where there are two operands */
		}
		/* end of the switch segmeant */
	}
	/* successfully encoded the instruction line. */
	return 1;
}


/*
 * getAddressingType - Returns the addressing type of a given operand.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @operand: The operand to be checked.
 * @instructionType: The type of the instruction.
 * 
 * This function determines the addressing type of a given operand based on its format and the instruction type.
 * 
 * Return: The addressing type (0-3) on success, -1 on error, -2 for future label or invalid operand.
 */
int getAddressingType(char *file_name, int line_num, char *operand, char *instructionType) 
{
	
	int addressingType;
	char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
	int i;

	/* addressing type 0 */
	if (operand[0] == '#') {
		if (operand[1] == '-' || isdigit(operand[1])) {
	
			int count = 0;
			operand++;
			while (*operand && !isspace(*operand) && *operand != ',') {
				operand++;
				count++;
			}

			if (count == 0) {
				printf("\nERROR: in file %s, line %d, no number after # sign of \"%s\" instruction word.\n", file_name, line_num, instructionType);
				return -1; /* no number at all */
			}
			return 0; /* addressing type found - number is valid */
		}
		else {
			printf("\nERROR: in file %s, line %d, invalid text after # sign of \"%s\" instruction word.\n", file_name, line_num, instructionType);
			return -1; /* invalid text */
		}
	}

	/* addressing type 1 */
	else if (isAlreadyLabel(operand) == 1) {
		return 1; /* addressing type found - label found*/
	}

	/* addressing type 2 */
	else if (operand[0] == '*') {
		int i;
		operand++; /* skip * sign */
		for (i = 0; i < 8; i++) {
			if (strcmp(operand, registers[i]) == 0) {
				return 2;  /* addresing type found - register found */
			}
		}
		printf("\nERROR: in file %s, line %d, invalid register name.\n", file_name, line_num);
		return -1; /* invalid register name */
	}

	/* addressing type 3 */	
	for (i = 0; i < 8; i++) {
		if (strcmp(operand, registers[i]) == 0) {
			return 3; /* addresing type found - register found */
		}
	}

	/* at this point, it's either invalid operand or a future label */
	return -2; /* only in the second stage, will we be able to conclude whether it's a future label or an error */
}


/*
 * NOTICE: this is a helper function and there's no use in that function at all in the assembler.
 * printInstructionImage - Prints the entire instruction memory image line by line.
 * This function traverses the instruction memory image linked list and prints each instruction's address and value
 * in binary format.
 */
void printInstructionImage()
{
	printf("Instructions-Memory-Image:\n");
    /* Helper function to print an integer in binary */
    void printBinary(int n) {
		int i;
        unsigned int mask = 1 << 14; /* This sets the mask to the highest bit (15th bit)*/
        for (i = 0; i < 15; i++) {
            if(n & mask) {
                printf("1");
            } else {
                printf("0");
            }
            mask >>= 1; // Shift the mask one position to the right
        }
    }
	/* traversing the linked list */
    Iptr p = Instruction_hptr;
    while (p) {
        printf("%d:\t", p -> address);
        printBinary(p->cell.MILA);
        printf("\n");
        p = p -> next;
    }   
}


/*
 * freeInstructionImage - Frees the instruction memory image linked list.
 * This function traverses the instruction memory image linked list and frees the memory allocated for each node.
 */
void freeInstructionImage()
{
	Iptr p;
	while (Instruction_hptr) 
	{
		p = Instruction_hptr;
		Instruction_hptr = Instruction_hptr -> next;
		/* free node */
		free(p);
	}	

	free(Instruction_hptr);
}


/*
 * addExtern - Encodes the .extern directive and its parameters into the label table.
 * @line: The line containing the .extern directive.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @label_err: Indicates whether a label error occurred.
 * 
 * This function encodes the .extern directive and its parameters into the label table, performing error checking
 * and handling memory allocation as needed.
 * 
 * Return: 1 on success, 0 on regular error, 2 on memory error.
 */
int addExtern(char *line, char *file_name, int line_num, int label_err)
{
	int label_num = 0;
	const char* end;
	size_t length;
	char* label;
	int i;
	char *invalidlabelName[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", 
								"jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string", 
								".entry", ".extern", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
		
	/* in case there is a label defined as a first word */
	if (label_err == 3) {
		/* skip label */
		line = skipFirstWord(line);
	}
	line = skipFirstWord(line); /* start of the third/second word */

	if (line == '\0') {
		printf("\nERROR: in file \"%s\", line %d, there are no labels defined after .extern.\n", file_name, line_num);
		return 0; /* not a label at all */
	}

	while (*line && isspace(*line)) {line++; } /* skip leading whitespaces */
	
	/* Find the end of the possible label word */
    end = line;
    while (*end && !isspace(*end)) {end++; }
    length = (end - line); /* Calculate length of the label */
			
    /* Allocate memory to hold the label word */
    label = (char*)malloc((length + 1) * sizeof(char));
    if (!label) {
        printf("\nERROR: in file \"%s\", line %d, memory allocation failed.\n", file_name, line_num);
        return 2; /* memory error */
    }

	if (length > 31) {
        printf("\nERROR: in file \"%s\", line %d, the label length exceeds the limit.\n", file_name, line_num);
        free(label);  /* Free the allocated memory */
        return 0;
    }

	strncpy(label, line, length);
    label[length] = '\0';  /* Null-terminate the string */

	/* check if there's another operand after the .extern expression */
	line = skipFirstWord(line);
	if (*line != '\0') {
		printf("\nERROR: in file \"%s\", line %d, Invalid num of operands after the \".extern\" definition.\n", file_name, line_num);
		free(label);
		return 0;
	}

	/* at this point the label has been created. Now error-checking: */
		
	/* check label name is valid */
	for (i = 0; i < 28; i++) {
		if (strcmp(label, invalidlabelName[i]) == 0) {
			printf("\nERROR: in file \"%s\", line %d, the label definition is invalid.\n", file_name, line_num);	
			free(label);			
			return 0;
		}
	}
	
	/* check that the label name isn't a macro name */
	if (isMacro(label) == 1) {
		printf("\nERROR: in file \"%s\", line %d, the label definition is matched to a macro name.\n", file_name, line_num);
		free(label);
		return 0;
	}
		
	/* make sure that the label wasn't already defined */
	if (isAlreadyLabel(label) == 1) {
		printf("\nERROR: in file \"%s\", line %d, the label is already defined.\n", file_name, line_num);
		free(label);
		return 0;
	}

	/* load the label to the Label Table: */
	if (loadLabelExtern(label, file_name, line_num) == 0) {
		printf("\nERROR: in file \"%s\", line %d, memory allocation failed.\n", file_name, line_num);
		free(label);
        return 2; /* memory error */
	} else {
		label_num++;
	}
	
	free(label);
	
	if (label_num == 0) {
		printf("\nERROR: in file \"%s\", line %d, no labels after .extern definition.\n", file_name, line_num);
		return 0;
	}
	return 1; /* passed all the checks, quit with success, loaded all labels */
}


/*
 * addData - Stores a number in the memory data image.
 * @number: The number to be stored.
 * @DC_address: The current data counter address.
 * 
 * This function stores the given number in the memory data image at the specified data counter address.
 * It handles memory allocation and updates the data image linked list accordingly.
 * 
 * Return: 1 on success, 0 on failure.
 */
int addData(int number, int DC_address)
{
	Dptr t = (Dptr) malloc(sizeof(d_item));
	if (!t) {
		return 0;
	}
	
	/* insert number to the memory cell: */
	mila space;
	space.MILA = 0; /* Initially set to all zeros */
	space.MILA = (unsigned short)number; /* Directly assign the number */
    t -> cell.MILA = space.MILA; 
    
	/* insert address */
	t -> address = DC_address;
	
	/* set next to null */
	t -> next = NULL;
	
	/* assign the node to the list. */
	if (Data_hptr == NULL) {
		Data_hptr = t; /* if the list is empty, make this the first node */
	} else {
		Dptr last = Data_hptr;
		while (last -> next != NULL) {
            last = last -> next;
        }
		last -> next = t;
	}
	return 1;
}


/*
 * addString - Stores a character in the memory data image.
 * @c: The character to be stored.
 * @DC_address: The current data counter address.
 * 
 * This function stores the given character in the memory data image at the specified data counter address.
 * It handles memory allocation and updates the data image linked list accordingly.
 * 
 * Return: 1 on success, 0 on failure.
 */
int addString(char c, int DC_address)
{
	Dptr t = (Dptr) malloc(sizeof(d_item));
	if (!t) {
		return 0;
	}
	
	/* insert character to the memory cell: */
	mila space;
	space.MILA = 0; /* Initially set to all zeros */
	space.MILA = (unsigned short)c; /* Directly assign the character ASCII code */
    t -> cell.MILA = space.MILA; 
    
	/* insert address */
	t -> address = DC_address;
	
	/* set next to null */
	t -> next = NULL;
	
	/* assign the node to the list. */
	if (Data_hptr == NULL) {
		Data_hptr = t; /* if the list is empty, make this the first node */
	} else {
		Dptr last = Data_hptr;
		while (last -> next != NULL) {
            last = last -> next;
        }
		last -> next = t;
	}
	return 1;
}


/*
 * loadLabelExtern - Loads the given label name as an extern label into the label table.
 * @label_name: The name of the label to be loaded.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function loads the given label name as an extern label into the label table, performing error checking
 * and handling memory allocation as needed.
 * 
 * Return: 1 on success, 0 on failure.
 */
int loadLabelExtern(char *label_name, char *file_name, int line_num)
{
    Lptr t = (Lptr) malloc(sizeof(l_item));
	if (!t) {
		printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_name);
		return 0;
	}
	
	/* insert label name */
    t -> label_name = (char *) malloc(strlen(label_name) + 1);
    if ((t -> label_name) == NULL) {
        printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_name);
        free(t); /* Clean up allocated memory */
        return 0;
    }
    strcpy(t -> label_name, label_name);

	/* insert redundant value */
	t -> value = 0;
	
	/* insert type name */
	t -> type = (char *) malloc(strlen(".external") + 1);
	if ((t -> type) == NULL) {
        printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_name);
		free(t -> label_name);        
		free(t); /* Clean up allocated memory */
        return 0;
    }
    strcpy(t -> type, ".external");
		
	t -> next = NULL;
	
	/* assign the node to the list. */
	if (Labelhptr == NULL) {
		Labelhptr = t; /* if the list is empty, make this the first node */
	} else {
		Lptr last = Labelhptr;
		while (last -> next != NULL) {
            last = last -> next;
        }
		last -> next = t;
	}
	return 1;
}


/*
 * NOTICE: this is a helper function and it does not take place in the assembler at all.
 * printDataImage - Prints the entire data memory image line by line.
 * This function traverses the data memory image linked list and prints each data's address and value in binary format.
 */
void printDataImage()
{
	Dptr p;

	printf("Data-Memory-Image:\n");
    /* Helper function to print an integer in binary */
    void printBinary(int n) {
		int i;
        unsigned int mask = 1 << 14; /* This sets the mask to the highest bit (15th bit)*/
        for (i = 0; i < 15; i++) {
            if(n & mask) {
                printf("1");
            } else {
                printf("0");
            }
            mask >>= 1; // Shift the mask one position to the right
        }
    }

	/* traversing the linked list */
    p = Data_hptr;
    while (p) {
        printf("%d:\t", p->address);
        printBinary(p->cell.MILA);
        printf("\n");
        p = p->next;
    }   
}


/*
 * freeDataImage - Frees the data memory image linked list.
 * This function traverses the data memory image linked list and frees the memory allocated for each node.
 */
void freeDataImage()
{
	Dptr p;
	while (Data_hptr) 
	{
		p = Data_hptr;
		Data_hptr = Data_hptr -> next;
		/* free node */
		free(p);
	}	

	free(Data_hptr);
}


/* (EXCESS FUNCTIONS:) */


/*
 * insertDataToNode - Inserts data into a node and sets its address and links.
 * @t: The node to be updated.
 * @space: The data to be inserted.
 * @IC: The address to be set for the node.
 * 
 * This function inserts the given data into the specified node, sets its address, and initializes its next and
 * previous links to NULL.
 */
void insertDataToNode(Iptr t, mila space, int IC)
{
	t->cell.MILA = space.MILA; 
	t->address = IC; 
	t->next = NULL; 
	t->prev = NULL;
}


/*
 * assignNodeToList - Assigns the node to the next location in the instruction list.
 * @t: The node to be assigned.
 * 
 * This function inserts the given node at the end of the instruction linked list, updating the next and previous
 * links as needed.
 */
void assignNodeToList(Iptr t)
{
	if (Instruction_hptr == NULL) { 
		Instruction_hptr = t; /* if the list is empty, make this the first node */ 
	} else { 
		Iptr last = Instruction_hptr; 
		while (last->next != NULL) { 
			last = last->next; 
		} 
		last->next = t; 
		t->prev = last; 
	}
}


/*
 * skipFirstWord - Skips the first word and the whitespaces after it.
 * @line: The line to be processed.
 * 
 * This function skips the first word in the given line and the whitespaces following it.
 * 
 * Return: The remaining part of the line.
 */
char *skipFirstWord(char *line)
{
	while (*line && isspace(*line)) {line++;} 
	while (*line && !isspace(*line)) {line++;} 
	while (*line && isspace(*line)) {line++;}

	return line;
}


/*
 * checkERRloadLabelADRRtype - Checks if an error occurred in the addressing type for the instruction.
 * @first_adressing_type: The addressing type to be checked.
 * @space: The memory space to be updated.
 * @IC: The current instruction counter address.
 * @t: The node to be updated.
 * 
 * This function checks if an error occurred in the addressing type for the instruction. If an error is found, it updates
 * the memory space and instruction node accordingly and returns an appropriate error code.
 * 
 * Return: 2 if an error occurred, 1 if it is a future label, 0 if no error.
 */
int checkERRloadLabelADRRtype(int first_adressing_type, mila space, int IC, Iptr t)
{
	if (first_adressing_type == -1) { 
		insertDataToNode(t, space, IC); 
		assignNodeToList(t); 
		return 2; /* error while loading instruction */ 
	} 
	else if (first_adressing_type == -2) { 
		/* insert a possible label adressing */ 
		int addressing_place = (1 << (1 + 3)); 
		space.MILA |= addressing_place; 
		insertDataToNode(t, space, IC); 
		assignNodeToList(t); 
		return 1; /* either invalid operand or a future label */ 
	}

	return 0;
}


/*
 * checkValidOperands - Checks if the operands' addressing types are valid for the instruction.
 * @FIRST_IS_FUTURE_LABEL: Indicates if the first operand may be a future label.
 * @SECOND_IS_FUTURE_LABEL: Indicates if the second operand may be a future label.
 * @instructionType: The type of the instruction.
 * @first_adressing_type: The addressing type of the first operand.
 * @second_addressing_type: The addressing type of the second operand.
 * 
 * This function checks if the operands' addressing types are valid for the given instruction. 
 * 
 * Return: 2 if invalid, 1 if future label, 3 if one valid, 0 if both valid.
 */
int checkValidOperands(int FIRST_IS_FUTURE_LABEL, int SECOND_IS_FUTURE_LABEL, char *instructionType, int first_adressing_type, int second_addressing_type)
{
	if (FIRST_IS_FUTURE_LABEL == 1 && SECOND_IS_FUTURE_LABEL == 1) {
		/* two of the operands are either invalid or a future label, exit and take care of the rest milas in the second stage */
		return 1;
	}
	if (SECOND_IS_FUTURE_LABEL == 1) {
		/* check if the source adressing type is valid to the instruction: */
		if (validOperandAddress(instructionType, second_addressing_type, "target") == 0) {
			return 2; /* ERROR: invalid addressing types for the instructions */
		}
		return 3;
	}
	if (FIRST_IS_FUTURE_LABEL == 1) {	
		/* check if the target adressing type is valid to the instruction: */
		if (validOperandAddress(instructionType, first_adressing_type, "source") == 0) {
			return 2; /* ERROR: invalid addressing types for the instructions */
		}
		return 3;		
	}
	if (valid2operandsAddress(instructionType, first_adressing_type, second_addressing_type) == 0) {
		return 2; /* ERROR: invalid addressing types for the instructions */
	}
	

	return 0;
}


/*
 * encodeRegisterMilaOnly - Encodes the register operands into the same mila.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @line: The line containing the operands.
 * @IC: The current instruction counter address.
 * 
 * This function handles the special case where both operands are registers and encodes their values into the same mila.
 * 
 * Return: 1 on success, 0 on error.
 */
int encodeRegisterMilaOnly(char *file_name, int line_num, char *line, int IC)
{
	char *word1;
	char *word2;
	int first_registerNum;
	int second_registerNum;

	/* creating a second mila */
	ALLOCATE_NODE;
	CREATE_AND_RESET_MILA; /* returns 0 when memory-error */
	space.MILA |= (1 << 2); /* set A in ARE to 1 */

	/* get the two register numbers */
	word1 = get_first_word(line);

	while (*word1 && !isspace(*word1) && !isdigit(*word1)) {word1++; } /* skip letters or signs until reaching a digit */
	first_registerNum = atoi(get_first_word(word1));

	if (first_registerNum > 7) {
		printf("\nERROR: in file \"%s\", line %d, the register number is too big.\n", file_name, line_num);
		return 0; /* if num is bigger than 111 or 7 */
	}

	word2 = get_second_word(line);

	while (*word2 && !isspace(*word2) && !isdigit(*word2)) {word2++; }
	second_registerNum = atoi(get_first_word(word2));

	if (second_registerNum > 7) {
		printf("\nERROR: in file \"%s\", line %d, the register number is too big.\n", file_name, line_num);
		return 0; /* if num is bigger than 111 or 7 */
	}

	space.MILA |= (first_registerNum << 6); /* add the source register num between 6-8 bits */
	space.MILA |= (second_registerNum << 3); /* add the target register num between 3-5 bits */

	/* complete node */
	insertDataToNode(t, space, IC);
	assignNodeToList(t);
	return 1;
}			


/*
 * isAddressType2Or3 - Checks if both operands in the line are of addressing type 2 or 3.
 * @line: The line to be checked.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * 
 * This function checks if both operands in the given line are of addressing type 2 or 3. 
 * 
 * Return: 1 if both operands are of addressing type 2 or 3, 0 otherwise.
 */
int isAddressType2Or3(char *line, char *file_name, int line_num) 
{
	/* add spaces adter commas */
	line = addSpacesAfterCommas(line);

	char *first_word = get_first_word(line);
	char *first_operand;
	char *second_operand;
	int first_adressing_type;
	int second_adressing_type;
	char *instruction_word;

	if (isLabel(first_word, file_name, line_num) == 1) {
		line = skipFirstWord(line);
	}
	while (*line && isspace(*line)) {line++; }

	/* now pointing at the start of the instruction word */
	instruction_word = get_first_word(line);

	/*skip instruction word: */
	line = skip_word(line);

	/* replace commas with spaces */
	line = replaceCommasWithSpaces(line);
	
	/* get operand names */
	first_operand = get_first_word(line);
	second_operand = get_second_word(line);

	/* get operands adressing types */
	first_adressing_type = getAddressingType(file_name, line_num, first_operand,instruction_word);
	second_adressing_type = getAddressingType(file_name, line_num, second_operand, instruction_word);

	if ((first_adressing_type == 2 || first_adressing_type == 3) && (second_adressing_type == 2 || second_adressing_type == 3)) {
		return 1; /* special case found. */
	}
	return 0;
}


/*
 * replaceCommasWithSpaces - Replaces commas with spaces in the given line.
 * @line: The line to be processed.
 * 
 * This function replaces commas in the given line with spaces to ensure proper separation of operands.
 * 
 * Return: The modified line with spaces instead of commas.
 */
char *replaceCommasWithSpaces(char *line)
{
	static char buffer[LINE_SIZE];  /* Static buffer to hold the modified string */
    int i = 0;

    /* Iterate through the input string */
    while (*line && i < LINE_SIZE - 1) {
        if (*line == ',') {
            buffer[i++] = ' ';  /* Replace comma with space */
        } else {
            buffer[i++] = *line;
        }
        line++;
    }
    buffer[i] = '\0';  /* Null-terminate the buffer */

    return buffer;
}


/*
 * skip_word - Skips one word from the current position in the line.
 * @line: The line to be processed.
 * 
 * This function skips one word from the current position in the given line.
 * 
 * Return: The remaining part of the line.
 */
char *skip_word(char *line)
{
	while (*line && !isspace(*line)) {line++; }
	while (*line && isspace(*line)) {line++; }
	return line;
}


