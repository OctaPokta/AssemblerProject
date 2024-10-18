/*
 * first_stage.c - First stage function for the assembler project
 * This file contains the first stage function which processes a given input file (.am),
 * handling label definitions, .data, .string, .entry, and .extern directives, and generating
 * the intermediate code and data images for further assembly stages.
 */

#include "../assembler.h"
#include "../excess_macro_list.h"
#include "../../pre_processing/macros_table.h" 


/*
 * first_stage - Handles the first stage of the assembler process.
 * @file_name: Name of the input file to be processed.
 * 
 * This function processes the .am file line by line, handling label definitions, 
 * .data, .string, .entry, and .extern directives. It updates the instruction counter (IC) 
 * and data counter (DC) and generates the intermediate code and data images.
 * The function returns different values based on the type of error encountered or success.
 * 
 * Return: 0 - regular error
 *         1 - success
 *         2 - memory error
 */ 
int first_stage(char *file_name)
{
	int err_count = 0;
	int LABEL_FLAG = 0;
	int IC = 0; /* instruction counter */
	int DC = 0; /* data counter */
	int L = 0; /* words counter */
	int line_num = 0;

	/* open .am file */
    char src_filename[256];	
	sprintf(src_filename, "pre_processing/%s.am", file_name);

	FILE *fp = fopen(src_filename, "r"); 
	if (fp == NULL) {
		printf("ERROR: Unable to open file: \"%s.am\".\n", file_name);
		return 0;
	}	

	/* Reading line after line */
	char line[LINE_SIZE]; 
	while (fgets(line, sizeof(line), fp)) 
	{
		/* check if we surpassed the memory size limit */
		if (IC + DC > MEMORY_SIZE) {
			printf("\nERROR: in file %s, file size is too big, surpassing memory limit of 4096.\n", file_name);
			err_count++;
			break;
		}

		line_num++; /* First line is 1 */	
		int label_err = isLabel(line, file_name, line_num);	
		char *dataOrString = findDataOrStringWord(line, file_name, line_num);
		char *entryOrExtern = findEntryOrExternWord(line, file_name, line_num);
	
		/* Reading the first word in a line. */
		char *first_word = get_first_word(line); 
		if (first_word == NULL || strcmp(first_word, "") == 0 || *first_word == '\n') {
            continue; /* Skip blank lines */
        }
		
		/* --(PRE-ERROR Checking)-- */
		if (line[0] == ';') { /* ignore comments*/
			*line = '\0';
			continue;
		}
		/* check that "macro" or macro name or a label is not later in line */
		if (clearOfMacro(line, file_name, line_num) == 0) {  
			CLEANUP_AND_CONTINUE;
			continue; /* regular error */
		}
	
		/* --(check if the first word is a label definintion)-- */
		if (label_err == 1) {
			LABEL_FLAG = 1;
		}
		else if (label_err == 2) {
			CLEANUP_AND_CONTINUE;
			continue; /* error - invalid label definition */ 
		}

		/* --(check if it's .data or .string)-- */
		if (dataOrString != NULL) {

			if (*dataOrString == '0') {
				CLEAN_BEFORE_EXIT; 
				return 2; /* memory error */
			}
			
			/* add label to table if exists */		
			if (LABEL_FLAG == 1) {
				/* put in the Label table with .data, value will be DC. */ 
				char *label = get_first_word(line);
				label[strlen(label)-1] = '\0';

				/* check that the label isn't defined twice */
				if (isAlreadyLabel(label) == 1) {
					printf("\nERROR: in file %s, line %d, the label \"%s\" is defined more than once.\n", file_name, line_num, label);
					CLEANUP_AND_CONTINUE;
					continue; /* regular error */
				}
	
				/* add the label to the table ;) */
				if (addLabel(label, DC,  dataOrString, file_name, line_num) == 0) {
					printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label);
					CLEAN_BEFORE_EXIT;
					return 2; /* memory error */
				}
			}

			/* encode .data instruction to the Data-Image. */
			if (strcmp(dataOrString, ".data") == 0) { 
				int newDC = encodeData(line, DC, dataOrString, LABEL_FLAG);
				
				/* error checking: */
				if (newDC == -1) {
					printf("\nERROR: in file %s, line %d, while encoding .data.\n", file_name, line_num);
					CLEANUP_AND_CONTINUE;
					continue; /* regular error */
				} 
				else if (newDC == -2) {
					printf("\nERROR: in file %s, line %d, memory error while encoding .data.\n", file_name, line_num);
					CLEAN_BEFORE_EXIT;
					return 2; /* memory error */
				}
				
				DC = newDC; /* update DC counter occurdingly. */
				SUCCESS_AND_CONTINUE;
				continue;
			}

			/* encode .string instruction to the Data-Image */
			else if (strcmp(dataOrString, ".string") == 0) {
				int newDC = encodeData(line, DC, dataOrString, LABEL_FLAG);
				/* error checking: */
				if (newDC == -1) {
					printf("\nERROR: in file %s, line %d, while encoding .string\n", file_name, line_num);		
					CLEANUP_AND_CONTINUE;
					continue; /* regular error */
				} 
				else if (newDC == -2) {
					printf("\nERROR: in file %s, line %d, memory error while encoding .string\n", file_name, line_num);
					CLEAN_BEFORE_EXIT;
					return 2; /* memory error */
				}

				DC = newDC; /* update DC counter accordingly. */
				SUCCESS_AND_CONTINUE;
				continue;
			}						
		}
			
		/* --(check if it's .entry or .extern)-- */
		if (entryOrExtern != NULL) {

			if (*entryOrExtern == '0') {
				CLEAN_BEFORE_EXIT; /* memory error */
				return 2; /* memory error */
			} 
			else if (*entryOrExtern == '1') {
				CLEANUP_AND_CONTINUE;
				continue; /* regular error */
			}

			/* add .extern instruction to the Label Table. */
			if (strcmp(entryOrExtern, ".extern") == 0) {
				
				/* put one or more labels into the table without a value, with type .external */
				int extern_err = addExtern(line, file_name, line_num, label_err); 
				if (extern_err == 0) {
					CLEANUP_AND_CONTINUE;
					continue; /* regular error */
				}
				else if (extern_err == 2) {
					CLEAN_BEFORE_EXIT;
					return 2; /* memory error */
				}

				SUCCESS_AND_CONTINUE;
				continue;
			}
			
			SUCCESS_AND_CONTINUE;
			continue;			
		}
		
		/* if LABEL_FLAG turned on, then put it in the label table with type .code with a value of IC+100 */
		if (LABEL_FLAG == 1) {

			/* load the label to the table */
			char *second_word = get_second_word(line);
			char *label = get_first_word(line);
			label[strlen(label)-1] = '\0';

			/* check that the label isn't defined twice */
			if (isAlreadyLabel(label) == 1) {
				printf("\nERROR: in file %s, line %d, the label \"%s\" is defined more than once.\n", file_name, line_num, label);
				CLEANUP_AND_CONTINUE;
				continue; /* regular error */
			}
	
			/* add the label to the table */
			if (addLabel(label, IC + 100, ".code", file_name, line_num) == 0) {
				printf("\nERROR: in file %s, line %d, unable to allocate memory for label \"%s\".\n", file_name, line_num, label);
				CLEAN_BEFORE_EXIT;
				return 2; /* memory error */
			}

			/* check that the instruction type is valid */
			if (validInstructionName(second_word) == 0) {
				printf("\nERROR: in file %s, line %d, instruction word of type \"%s\" that comes after the label is unknown.\n", file_name, line_num, second_word);
				CLEANUP_AND_CONTINUE;
				continue; /* regular error */
			}

			/* encode the instruction (with a label) */
			L = encodeInstruction(line, file_name, line_num, LABEL_FLAG, IC);
			if (L == -1) {
				CLEANUP_AND_CONTINUE;
				continue; /* regular error */
			}
			else if (L == -2) {
				CLEAN_BEFORE_EXIT;
				return 2; /* memory error */
			}

			IC += L; 
			/* in case there are two registers on the same line, decrease IC as they will be encoded both in the same "mila" */
			if (isAddressType2Or3(line, file_name, line_num) == 1) {
				IC--;
			}

			SUCCESS_AND_CONTINUE;
			continue;
		}

		/* check that the instruction type is valid */
		if (validInstructionName(first_word) == 0 && !LABEL_FLAG) {
			printf("\nERROR: in file %s, line %d, instruction word of type \"%s\" is unknown.\n", file_name, line_num, first_word);
			CLEANUP_AND_CONTINUE;
			continue; /* regular error */
		}

		/* encode the instruction (without a label) */
		L = encodeInstruction(line, file_name, line_num, LABEL_FLAG, IC);
		if (L == -1) {
			CLEANUP_AND_CONTINUE;
			continue; /* regular error */
		}
		else if (L == -2) {
			CLEAN_BEFORE_EXIT;
			return 2; /* memory error */
		}
		
		IC += L;
		/* in case there are two registers on the same line, decrease IC as they will be encoded both in the same "mila" */
		if (isAddressType2Or3(line, file_name, line_num) == 1) {
			IC--;
		} 
		
		SUCCESS_AND_CONTINUE;
	}	

	/* at this point we read all the lines */
	fclose(fp);
	if (err_count > 0) {
		return 0;
	}

	/* Update all ".data" & ".string" labels with IC+100, and update all .code labels with +100 */
	updateLabels(IC);
	
	return 1; /* success */
} 



