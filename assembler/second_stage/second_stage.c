/*
 * second_stage.c - Second stage function for the assembler project
 * This file contains the second stage function which processes a given input file (.am),
 * handling the encoding of labels and instructions that were not fully processed in the first stage.
 * It ensures all labels are correctly referenced and encoded in the instruction memory image.
 */

#include "../assembler.h"
#include "../../pre_processing/macros_table.h" 
#include "../excess_macro_list.h"


/*
 * second_stage - Handles the second stage of the assembler process.
 * @file_name: Name of the input file to be processed.
 * 
 * This function processes the .am file line by line, handling the encoding of labels 
 * and instructions that were not fully processed in the first stage. It ensures all labels 
 * are correctly referenced and encoded in the instruction memory image. Additionally, it creates
 * the final output files necessary for the assembler.
 * 
 * Return: 0 - regular error
 *         1 - success
 *         2 - memory error
 */
int second_stage(char *file_name)
{
	int err_count = 0;
	int IC = 0; /* instruction counter */
	int L = 0; /* words counter */
    int line_num = 0;
    
	/* open .am file */
	char filename[256];
	sprintf(filename, "pre_processing/%s.am", file_name);
	
	FILE *fp = fopen(filename, "r"); 
	if (fp == NULL) {
		printf("ERROR: Unable to open file: \"%s.am\".\n", file_name);
		return 0;
	}

	/* Reading line after line */
    char line[LINE_SIZE]; 
	while (fgets(line, sizeof(line), fp)) 
	{
        int LABEL_FLAG = 0;
        char *word_check;
		int encodeErr;

        /* Reading the first word in a line. */
		char *word = get_first_word(line); 
		if (word == NULL || strcmp(word, "") == 0 || *word == '\n') {
            continue; /* Skip blank lines */
        }	
	
		line_num++; /* First line is 1 */

		/* --(PRE-ERROR Checking)-- */
		if (line[0] == ';') { /* ignore comments*/
			*line = '\0';
			continue;
		}
	
        /* START THE SECOND STAGE: */
        
        /* check if a label is defined and add an error message if there's ".entry" defined later */
        word[strlen(word)-1] = '\0';
        
        if (isAlreadyLabel(word) == 1 || isLabel(line, file_name, line_num) == 3) {
            
            /* check .entry and show error message */
            char *possible_entryORextern = get_second_word(line);
            /* check .extern or .entry and show error message */
            if (strcmp(possible_entryORextern, ".extern") == 0 || strcmp(possible_entryORextern, ".entry") == 0) {
                printf("\nNOTICE: in file \"%s\", line %d, the (possible) label that's defined as a first word will not be considered as label in the label table.\n", file_name, line_num);
            }
            LABEL_FLAG = 1; /* label exists in this line */
        }
        
        /* get the instruction-type word */ 
        if (LABEL_FLAG == 1) {
            word_check = get_second_word(line);
            
        }
        else {
            word_check = get_first_word(line);
        }
        
        if (strcmp(word_check, ".data") == 0 || strcmp(word_check, ".string") == 0 || strcmp(word_check, ".extern") == 0) {
            continue; /* skip line when it's ".data", ".string" or ".extern" */
        }

        /* check if it's ".entry" instruction */
        if (strcmp(word_check, ".entry") == 0) {

            /* get label word */
            char *label_word = (LABEL_FLAG == 1) ? get_third_word(line) : get_second_word(line);

            /* change entry label status */
            int entryErr = addEntry(file_name, line_num, label_word);
            if (entryErr == 0) {
                err_count++;
                continue; /* regular error */
            }
            else if (entryErr == 2) {
                return 2; /* memory error */
            }
            
            /* check for a second operand */
            if (check4OperandEntry(file_name, line_num, line, LABEL_FLAG) == 1) {
                err_count++;
                continue; /* regular error */
            }

            continue;
        }

        /* encode the first/second operands to the instruction-memory-image */
        L = encodeErr = encodeMissingOperand(file_name, line_num, line, LABEL_FLAG, IC);
        
        if (encodeErr == -2) {
            return 2; /* memory error */
        }
        else if (encodeErr == -1) {
            err_count++;
            continue; /* regular error */
        }
        
        IC = IC + L; 
        
		/* in case there are two registers on the same line, decrease IC as they will be encoded both in the same "mila" */
		if (isAddressType2Or3(line, file_name, line_num) == 1) {
			IC--;
		}
    }

    /* at this point we read all the lines */

    if (err_count > 0) {
        return 0;
    }

    /* --(run the final stage of the assembler)--  */ 
	if (createOutput(file_name, line_num, fp) == 0) {	
        return 2; /* memory error */
	}	

	fclose(fp);
	

	return 1;
} 








