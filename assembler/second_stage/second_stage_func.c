/*
 * second_stage_func.c - This file handles the second stage of the assembler process.
 * It includes functions for encoding labels in the instruction memory, handling entry directives, 
 * and generating the output files. The file uses linked lists to manage the label, data, and instruction memory images,
 * ensuring proper encoding and output formatting.
 */

#include "../../pre_processing/macros_table.h" 
#include "../assembler.h"
#include "../excess_macro_list.h"


/*
 * addEntry - Encodes the label within the .entry instruction to the label table.
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @label_word: The label to be encoded.
 * 
 * Return: 1 on success, 0 on regular error, 2 on memory error.
 */
int addEntry(char *file_name, int line_num, char *label_word)
{
    /* check if the label was already added in the first stage*/
    if (isAlreadyLabel(label_word) == 0) {
        printf("\nERROR: unkown label word after \".entry\" instruction.\n");
        return 0;
    }

    /* change label status to ".entry" */
    if (changeEntryStatus(file_name, line_num, label_word) == 0) {
        return 2;
    }

    return 1;
}


/*
 * changeEntryStatus - Changes the label status to ".entry".
 * @file_name: The name of the file being processed.
 * @line_num: The current line number being processed.
 * @label_word: The label whose status is to be changed.
 * 
 * Return: 1 on success, 0 on memory error.
 */
int changeEntryStatus(char *file_name, int line_num, char *label_word)
{
	/* traversing the linked list */
    Lptr p = Labelhptr;
    while (p) {	
        if (strcmp(label_word, p -> label_name) == 0) {

            /* label was found */
            free(p -> type); /* reset current status */

            /* create the new type cell */
            p -> type = (char *) malloc(strlen(".entry") + 1);
                if ((p -> type) == NULL) {
                printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label_word);
                free(p -> label_name);        
                free(p); /* Clean up allocated memory */
                return 0;
            }

            /* add type .entry to label status */
            strcpy(p -> type, ".entry");
            return 1; /* success */
        }
        p = p->next;
    }  
}


/*
 * get_third_word - Returns the third word in the given line.
 * @line: The line to be processed.
 * 
 * Return: The third word in the line.
 */
char *get_third_word(char *line)
{
    size_t length;
    static char third_word[LINE_SIZE];

    if (findNumOfWords(line) < 3) {
        return ""; /* no third word */
    }

    /* Skip leading whitespace */
    line = skipFirstWord(line);
    line = skipFirstWord(line); /* start of the 3rd word*/  

    /* Find the end of the first word */
    const char* end = line;
    while (*end && !isspace(*end)) {
        end++;
    }
    
    /* Calculate length of the first word */
    length = end - line;
    
	/* Create a static buffer to hold the first word */
	if (length >= sizeof(third_word)) {
        printf("Word is too long to fit in the buffer\n");
        return NULL;
    }
    
    /* Copy the first word */
    strncpy(third_word, line, length);
    third_word[length] = '\0';  /* Null-terminate the string */

    return third_word;
}


/*
 * get_fourth_word - Returns the fourth word in the given line.
 * @line: The line to be processed.
 * 
 * Return: The fourth word in the line.
 */
char *get_fourth_word(char *line)
{
    const char* end;
    size_t length;
    static char fourth_word[LINE_SIZE]; 
    
    if (findNumOfWords(line) < 4) {
        return ""; /* no fourth word */
    }
    /* Skip leading whitespace */
    line = skipFirstWord(line);
    line = skipFirstWord(line);
    line = skipFirstWord(line); /* start of the 4rd word*/

    /* Find the end of the first word */
    end = line;
    while (*end && !isspace(*end)) {
        end++;
    }
    
    /* Calculate length of the first word */
    length = end - line;
    
	/* Create a static buffer to hold the first word */ 
	if (length >= sizeof(fourth_word)) {
        printf("Word is too long to fit in the buffer\n");
        return NULL;
    }
    
    /* Copy the first word */
    strncpy(fourth_word, line, length);
    fourth_word[length] = '\0';  /* Null-terminate the string */

    return fourth_word;
}


/* given a line, this function checks whether one of its operands is a label that wasn't encoded in the first-stage,
 * then, the function encodes the specific operand as a mila cell in the correct place. it returns:
 (L) - the value which L needs to be incremented by
 (-1) - regular error
 (-2) - memory error */
int encodeMissingOperand(char *file_name, int line_num, char *line, int LABEL_FLAG, int IC)
{
    
    int L = 0;
    int instruction_length = 0; /* represents the num of operands */
    static char *instructionType;
    int encodeErr;
    char instructionTypeBuffer[LINE_SIZE];
    
    /* skip label word if exists */
    if (LABEL_FLAG == 1) {
        line = skipFirstWord(line);
    }
    
    replaceCommas(line); /* replace commas with spaces */
    
    instructionType = get_first_word(line); /* get the instruction word */
    line = skipFirstWord(line); /* skip instruction word */

    IC += 1;
    L += 1;
    L += findNumOfWords(line); /* get num of words excluding the label */
    
    /* save instruction-type word as a buffer */ 
	strncpy(instructionTypeBuffer, instructionType, strlen(instructionType));
	instructionTypeBuffer[sizeof(instructionType)-1] = '\0'; 

    /* in case there are 0 operands */
    if (*line == '\0' || *line == '\n') {
        return L;
    }

    /* find how many operands there are in the line */
    instruction_length = findNumOfWords(line);

    switch (instruction_length) 
    {
        /* one operand */
        case 1:
        {
            /* check addressing type of the operand */
            char *operand = get_first_word(line); /* get the operand word */
            int addressing_type = getAddressingType(file_name, line_num, operand, instructionTypeBuffer);

            if (addressing_type == 1) {

                /* check if the label was already encoded in the first-stage */
                if (LabelWasAlreadyEncoded(IC) == 1) {
                    return L; /* success */
                }

                /* encode the mila in the correct place */
                if (encodeLabelMila(operand, IC) == 0) {
                    return -2; /* memory error */
                }
            }
            else if (addressing_type == -2) {
                printf("\nERROR: in file \"%s\", line %d, operand after instruction of type \"%s\" is invalid.\n", file_name, line_num, instructionTypeBuffer);
                return -1;
            }

            return L; /* success*/
        }

        /* two operands */
        case 2:
        {
            /* get first and second operands: */
            char *first_operand = get_first_word(line); 
            char *second_operand = get_second_word(line); 
            
            /* get addressing type of the operands: */
            int first_addressing_type = getAddressingType(file_name, line_num, first_operand, instructionTypeBuffer);
            int second_addressing_type = getAddressingType(file_name, line_num, second_operand, instructionTypeBuffer);
            
            /* SPECIAL CASE - when two of the adressing types are either 2 or 3 */
            if ((first_addressing_type == 2 || first_addressing_type == 3) && (second_addressing_type == 2 || second_addressing_type == 3)) {
                return L; /* this was already handled in first-stage */
            }

            /* encode the first operand (second mila)*/

            /* First operand may be a label definition */
            if (first_addressing_type == 1)  {
                
                /* encode the first operand as long as the label was not encoded in the first-stage */
                if (encodeLabelMila(first_operand, IC) == 0 && LabelWasAlreadyEncoded(IC) == 0) {
                    return -2; /* memory error */
                }    
            }
            /* Second operand may be a future label */
            if (second_addressing_type == 1) {
                /* encode the second operand as long as the label was not encoded in the first-stage */
                IC++;;
                if (encodeLabelMila(second_operand, IC) == 0 && LabelWasAlreadyEncoded(IC) == 0) {
                    return -2; /* memory error */
                }    
            }

            /* Deal in case the operand may not be a label */
            if (first_addressing_type == -2) {
                /* unknown word */
                printf("\nERROR: in file \"%s\", line %d, first operand after instruction of type \"%s\" is invalid.\n", file_name, line_num, instructionTypeBuffer);
                return -1;
            }
            if (second_addressing_type == -2) {
                /* unknown word */
                printf("\nERROR: in file \"%s\", line %d, second operand after instruction of type \"%s\" is invalid.\n", file_name, line_num, instructionTypeBuffer);
                return -1;
            }

            return L; /* success*/
        }
    }
}


/* this function encodes a mila as an adressing type of 1 (ie label) to the instruction-memory-image,
1 - success
0 - memmory error */
int encodeLabelMila(char *operand, int IC)
{
	char *labelStatus = getLabelStatus(operand); /* get the label type ie. ".external" etc... */
	int Labeladdress = getLabelAddress(operand); /* get the label address */
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


/* Given a line, this function returns the number of words in the line */
int findNumOfWords(char *line) {
    int length_num = 0;
    
    /* Skip leading spaces */
    while (*line && isspace(*line)) {
        line++;
    }
    
    while (*line && *line != '\n') {
        /* Ensure the current position is the start of a word */
        if (!isspace(*line)) {
            length_num++;
            line = skipFirstWord(line); /* Move to the next word */
        }
    }
    return length_num;
}


/*
 * LabelWasAlreadyEncoded - Checks if a given address exists in the instruction memory image.
 * @IC: The instruction counter address to be checked.
 * 
 * Return: 1 if address found, 0 if not found.
 */
int LabelWasAlreadyEncoded(int IC)
{
	/* traversing the linked list */
    Iptr p = Instruction_hptr;
    while (p) {
		if (p -> address == IC) {
            return 1; /* found */
        }
        p = p->next;
    }   

    return 0;
}


/*
 * countInstructionCell - Counts and returns the number of memory cells in the instruction image.
 * Return: The number of memory cells in the instruction image.
 */
int countInstructionCell()
{
    int total_num = 0;
    Iptr p = Instruction_hptr;

    while (p) {
        total_num++;
        p = p->next;
    }

    return total_num;
}


/*
 * countDataCell - Counts and returns the number of memory cells in the data image.
 * Return: The number of memory cells in the data image.
 */
int countDataCell()
{
    int total_num = 0;
    Dptr t = Data_hptr;
    while (t) {
        
        total_num++;
        t = t->next;
    }
    
    return total_num;
}


/*
 * write2Object - Writes the output object file data.
 * @obj: The file pointer to the object file.
 */
void write2Object(FILE *obj)
{
    /* Helper function to print a binary number with leading zeros */
    void print_binary(FILE *obj, int num, int bits) {
        for (int i = bits - 1; i >= 0; i--) {
            fprintf(obj, "%d", (num >> i) & 1);
        }
    }

    int sum_instruction_cell = countInstructionCell(); /* represents the sum of instruction cells */
    int sum_data_cell = countDataCell(); /* represents the sum of data cells */
    int total_cells = sum_instruction_cell + sum_data_cell;
    int i;

    /* write the sum data at the top */
    fprintf(obj, "%d %d", sum_instruction_cell, sum_data_cell); 
    fprintf(obj, "\n");

    /* write memory-address + cell */
    for (i = 0; i < total_cells; i++) {
        
        int address = i + 100; /* Calculate the address */

        fprintf(obj, "%04d ", address); /* Write address in decimal format with 4 digits */

        /* Write the memory cell content in octal format with 5 digits */
        fprintf(obj, "%05o\n", memory_image[i + 100].MILA & 077777); /* 077777 is the octal representation of a 15-bit mask */
    }
}


/* This function writes the output entry file data with a given pointer to file */
void write2Ent(FILE *ent)
{
    Lptr p = Labelhptr;
    while (p) {

        /* found an .entry label*/
        if (strcmp(p->type, ".entry") == 0) {
            /* write label to file */
            fprintf(ent, "%s %d\n", p->label_name, p->value);
        }
        p = p->next;
    }
}


/*
 * entryLabelsExists - Checks if there is at least one label defined as ".entry".
 * Return: 1 if found, 0 if not found.
 */
int entryLabelsExists()
{
    Lptr p = Labelhptr;
    while (p) {

        if (strcmp(p->type, ".entry") == 0) {
            return 1;
        }
        p = p->next;
    }

    return 0;
}


/*
 * externLabelExists - Checks if there is at least one label defined as ".extern".
 * Return: 1 if found, 0 if not found.
 */
int externLabelExists()
{
    Lptr p = Labelhptr;
    while (p) {

        if (strcmp(p -> type, ".external") == 0) {
            return 1;
        }
        p = p->next;
    }

    return 0;
}


/* This function sorts and load the instruction-image and the data-image to the PC memory in order, then returns 1 */
int loadPCMemory()
{
    int AC = 0; /* address counter */

    /* Load instruction-memory image to the main PC memory-image: */
    Iptr p = Instruction_hptr;
    Dptr t = Data_hptr;
    while (p) {
        
        /* check if the address doesn't match */
        if (p->address != AC) {
            
            Iptr t = Instruction_hptr;
            int foundNode = 0;
            /* find the missing cell */
            while (t) {
                if (t->address == AC) {
                    /* missing cell found */
                    memory_image[AC+100] = t->cell;
                    foundNode = 1;
                    break;
                }

                t = t->next;
            }
            /* if didn't find cell, exit */
            if (foundNode = 0) {
                break;
            }
        }
        else {
            /* address does match, add the current cell */
            memory_image[AC+100] = p->cell;
        }

        AC++;
        p = p->next;
    }

    /* Load data-memory image to the main PC memory-image: */
    while (t) {
        memory_image[AC+100] = t->cell;
        AC++;
        t = t->next;
    }


    return 1;
}


/* NOTICE: This is a helper function and it does not have a use in the assembler at all.
 * This function prints the PC memory until it reaches a given n limit */
void printPCmemory(int n)
{
    printf("PC-Memory-Image:\n");
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

    int i;
    printf("\n");
    for (i = 100; i < n; i++) {
        printf("%d:    ", i);
        printBinary(memory_image[i].MILA);
        printf("\n");
    }
}


/*
 * createOutput - Creates the output files.
 * @file_name: The name of the file being processed.
 * @fp: The file pointer to the source file.
 * 
 * Return: 1 on success, 0 on memory error.
 */
int createOutput(char *file_name, int line_num, FILE *fp)
{
    #define EXTRA_OBJ_NAME_SPACE 11
    #define EXTRA_ENT_NAME_SPACE 12
    #define EXTRA_EXT_NAME_SPACE 12
    FILE *obj = NULL;
    FILE *ent = NULL;
    FILE *ext = NULL;

    loadPCMemory(); /* sort the data into the PC memory */
    
    /* create object file name */
    int obj_length = strlen(file_name) + EXTRA_OBJ_NAME_SPACE;
    char obj_name[obj_length];
    memset(obj_name, 0, obj_length); // Initialize to zero
    snprintf(obj_name, obj_length, "output/%s.ob", file_name);

    /* open file */
    obj = fopen(obj_name, "w");
    if (obj == NULL) {
        printf("ERROR: Unable to create object file: \"%s\".\n", obj_name);
        return 0; /* moving to the next file */
    }	

    write2Object(obj); /* write the object file */
    fclose(obj);
    
    /* check in case there is at least one ".entry" label */
    if (entryLabelsExists() == 1) {

        /* create the .entry file */
        int ent_length = strlen(file_name) + EXTRA_ENT_NAME_SPACE;
        char ent_name[ent_length];
        memset(ent_name, 0, ent_length); // Initialize to zero
        snprintf(ent_name, ent_length, "output/%s.ent", file_name);

        /* open file */
        ent = fopen(ent_name, "w");
        if (ent == NULL) {
            printf("ERROR: Unable to create entry file: \"%s\".\n", ent_name);
            return 0; /* moving to the next file */
        }	
        
        write2Ent(ent); /* write to the entry file */
        fclose(ent);
    }

    /* check in case there is at least one ".extern" label */
    if (externLabelExists() == 1) {
        
        /* create the .extern file */
        int ext_length = strlen(file_name) + EXTRA_EXT_NAME_SPACE;
        char ext_name[ext_length];
        memset(ext_name, 0, ext_length); // Initialize to zero
        snprintf(ext_name, ext_length, "output/%s.ext", file_name);

        /* open file */
        ext = fopen(ext_name, "w");
        if (ext == NULL) {
            printf("ERROR: Unable to create extern file: \"%s\".\n", ext_name);
            return 0; /* moving to the next file */
        }	

        write2Extern(file_name, line_num, ext, fp); /* write to the extern file */
        fclose(ext);
    }
    
    return 1;
}


/*
 * write2Extern - Writes the external labels data to the .ext file.
 * @ext: The file pointer to the .ext file.
 * @fp: The file pointer to the source file.
 * 
 * Return: 1 on success.
 */
int write2Extern(char *file_name, int line_num, FILE *ext, FILE *fp)
{
    int IC = 0; /* instruction word counter (used to find the location of the extern labels) */
    char line[LINE_SIZE]; 
    rewind(fp); /* reset source file pointer */
    
    /* Reading line after line */
	while (fgets(line, sizeof(line), fp)) 
	{
        char *first_word = get_first_word(line); /* get first word */
        int first_word_length = strlen(first_word);
        int L;
        char possible_label[first_word_length]; 
        char *second_word;
        char *third_word;
        char *fourth_word;

        replaceCommas(line); /* replace commas with whitespaces*/
         
        L = findNumOfWords(line); /* assign num of words */

        /* Copy first word to another buffer as a possible label and remove the last character ':' */
        if (first_word[first_word_length - 1] == ':') {
            strncpy(possible_label, first_word, first_word_length - 1);
            possible_label[first_word_length - 1] = '\0'; /* Null-terminate the string */
        } else {
            strncpy(possible_label, first_word, first_word_length);
            possible_label[first_word_length] = '\0'; /* Null-terminate the string */
        }

        /* replace all commas with whitespaces */
        if (isAlreadyLabel(possible_label) == 1 || isLabel(line, file_name, line_num) == 3) {
            replaceCommas(line + first_word_length); /* handles commas differently when label is declared */
            L -= 1; /* exclude label word */
        }
        
        /* get words */
        second_word = get_second_word(line);
        third_word = get_third_word(line);
        fourth_word = get_fourth_word(line); 

        /* skip if there's ".extern" OR ".entry" as a next word or if there's .data OR .string */
        if (isExternORentry(first_word, second_word) == 1 || isDataORstring(first_word, second_word) == 1) {
            continue;
        }
        
        /* decrease IC by one if two of the operands are registers */
        if (checkRegisters(second_word, third_word, fourth_word) == 1) {
            L--;
        }

        print2ExternFile(possible_label, second_word, third_word, fourth_word, IC, ext); /* print the data to the file */
        
        IC += L; /* update IC */
    }

    return 1;
}


/*
 * print2ExternFile - Prints the data to the .ext file.
 * @label_name: The name of the label.
 * @second_word: The second word in the line.
 * @third_word: The third word in the line.
 * @fourth_word: The fourth word in the line.
 * @IC: The instruction counter address.
 * @ext: The file pointer to the .ext file.
 */
void print2ExternFile(char *label_name, char *second_word, char *third_word, char *fourth_word, int IC, FILE *ext)
{
    /* search for labels */
    if (isAlreadyLabel(label_name) == 1) {

        /* skip past label (special case)*/
        if (isExternLabel(third_word) == 1) {
            /* write to file */
            fprintf(ext, "%s %04d\n", third_word, IC+100 + 1);
        }

        if (isExternLabel(fourth_word) == 1) {
            /* write to file */
            fprintf(ext, "%s %04d\n", fourth_word, IC+100 + 2);
        }
    }
    else {

        if (isExternLabel(second_word) == 1) {
            /* write to file */
            fprintf(ext, "%s %04d\n", second_word, IC+100 + 1);

        }

        if (isExternLabel(third_word) == 1) {
            /* write to file */
            fprintf(ext, "%s %04d\n", third_word, IC+100 + 2);
        }
    }
}


/*
 * checkRegisters - Checks if at least two of the given words are registers.
 * @second_word: The second word to be checked.
 * @third_word: The third word to be checked.
 * @fourth_word: The fourth word to be checked.
 * 
 * Return: 1 if at least two are registers, 0 otherwise.
 */
int checkRegisters(char *second_word, char *third_word, char *fourth_word)
{
    int i;
    if ((isRegister(second_word) == 1 && isRegister(third_word)) == 1 || (isRegister(third_word) == 1 && isRegister(fourth_word) == 1)) {
        return 1;
    }
    return 0;
}


/*
 * isRegister - Checks if a given word is a register.
 * @word: The word to be checked.
 * 
 * Return: 1 if the word is a register, 0 otherwise.
 */
int isRegister(char *word) 
{
    char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "*r0", "*r1", "*r2", "*r3", "*r4", "*r5", "*r6", "*r7"};
    int i;
    for (i = 0; i < 16; i++) {
        if (strcmp(registers[i], word) == 0) {
            return 1; /* reguster found */
        }
    }
    return 0; /* register wasn't found */
}


/*
 * isExternLabel - Checks if a given label is defined as external.
 * @label_name: The name of the label to be checked.
 * 
 * Return: 1 if the label is external, 0 if not found.
 */
int isExternLabel(char *label_name)
{
    Lptr p = Labelhptr;
    while (p) {

        if ((strcmp(label_name, p->label_name) == 0) && (strcmp(p->type, ".external") == 0)) {

            /* a label that matches the input label was found, tagged as "external" */
            return 1; /* found matching label */
        }
        p = p->next;
    }
    return 0; /* label wasn't found */
}


/*
 * isDataORstring - Checks if the given words are ".data" or ".string".
 * @first_word: The first word to be checked.
 * @second_word: The second word to be checked.
 * 
 * Return: 1 if one of them is ".data" or ".string", 0 otherwise.
 */
int isDataORstring(char *first_word, char *second_word) 
{
    if (strcmp(first_word, ".string") == 0 || strcmp(first_word, ".data") == 0 ||
        strcmp(second_word, ".string") == 0 || strcmp(second_word, ".data") == 0) {
        return 1;
    }
    return 0;
}


/*
 * isExternORentry - Checks if the given words are ".entry" or ".extern".
 * @first_word: The first word to be checked.
 * @second_word: The second word to be checked.
 * 
 * Return: 1 if one of them is ".entry" or ".extern", 0 otherwise.
 */
int isExternORentry(char *first_word, char *second_word)
{
    if (strcmp(first_word, ".entry") == 0 || strcmp(first_word, ".extern") == 0 ||
        strcmp(second_word, ".entry") == 0 || strcmp(second_word, ".extern") == 0) {
        return 1;
    }
    return 0;
}


/* This function checks whether there are excess operands after the entry definition, if it found excess operands it returns 1, otherwise, 0. */
int check4OperandEntry(char *file_name, int line_num, char *line, int LABEL_FLAG)
{
    if (LABEL_FLAG == 1) {
        char *fourth_word = get_fourth_word(line);        
        if (fourth_word != "") {   
            printf("\nERROR: in file \"%s\", line %d, Invalid num of operands after the \".entry\" definition.\n", file_name, line_num);
            return 1;
        }
        return 0;
    }
    char *third_word = get_third_word(line);
    if (third_word != "") {       
        printf("\nERROR: in file \"%s\", line %d, Invalid num of operands after the \".entry\" definition.\n", file_name, line_num);
        return 1;
    }

    return 0;
}