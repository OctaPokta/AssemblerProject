/*
 * pre_assembler.c - Pre-assembler function for the assembler project
 * This file contains the pre-assembler function which processes a given input file,
 * handling macro definitions and generating an intermediate file (.am) for further assembly stages.
 */

#include "pre_assembler.h"
#include "macros_table.h"
#define LINE_SIZE 81


/*
 * pre_assembler - Handles the pre-assembling of a given file.
 * @fp: File pointer to the input file to be pre-assembled.
 * @num_of_file: Index of the current file being processed.
 * @name_of_file: Name of the input file being processed.
 * 
 * This function processes the input file line by line, handling macro definitions and 
 * replacing macro calls with their corresponding content. It creates an intermediate 
 * file (.am) that is used in subsequent stages of the assembler. The function returns 
 * different values based on the type of error encountered or success.
 * 
 * Return: 0 - regular error (skip to next file)
 *         1 - success (move to next file)
 *         2 - memory allocation error (shutdown program)
 */
int pre_assembler(FILE *fp, int num_of_file, char *name_of_file)
{	
	#define EXTRA_NAME_SPACE 19
	int MACRO_FLAG = 0;
	char *MACRO_NAME;
	int line_num = 0;
	int error0Count = 0;

	/* creating the .am file for the pre-assembler */
    size_t newNameLen = strlen(name_of_file) + EXTRA_NAME_SPACE; 
    char *newName = malloc(newNameLen);
    if (newName == NULL) {
        printf("ERROR: Memory allocation for file \"%s\" name failed.\n", name_of_file);
        return 2;
    }
    snprintf(newName, newNameLen, "pre_processing/%s.am", name_of_file);
	
	FILE *fd = fopen(newName, "w");
	if (fd == NULL) {
		printf("ERROR: Unable to create file: \"%s\".\n", newName);
		return 0; /* moving to the next file */
	}	

	free(newName);

	/* read line by line */
	char line[LINE_SIZE + 2] = "";
	line[81] = '\0';
	while (fgets(line, (LINE_SIZE + 2), fp)) 
	{
		line_num++; /* first line is 1 */
		/* make sure the line size is valid */
		if (line[81] != '\0') {
			printf("\nERROR: in file \"%s\": line %d exceeds the limit.\n", name_of_file, line_num);
			error0Count++;
			char c;
			while ((c = fgetc(fp)) != '\n' && (c != EOF));
			line[81] = '\0';
			continue; /* pick the error and move to next line */
		}
	
		/* ignore comments*/
		if (line[0] == ';') {
			continue;
		}

		/* Reading the first word in a line. */
		char *word = get_first_word(line); 
		if (word == NULL) {
            continue; /* Skip blank lines */
        }

		/* check if we found an existing macro. */
		if (isMacro(word) == 1) 
		{ 
			/* check that there are no excess words/letters */
			if (isOnlyWord(line) == 0) {
				printf("\nERROR: in file \"%s\": line %d there are excess letters after calling a macro.\n", name_of_file, line_num);
				error0Count++;
				continue; /* pick the error and move to next line */
			}

			char *content = getMacro(word);
			fputs(content, fd);	
			continue;
		} 
		
		/* finding a definition of a macro */
		else if (strcmp(word, "macr") == 0) 
		{
			/* check that there are no excess words/letters */
			if (onlyTwoWords(line) == 0) {
				printf("\nERROR: in file \"%s\": line %d there are excess letters after a macro definition.\n", name_of_file, line_num);
				error0Count++;
				continue; /* pick the error and move to next line */
			}
							
			MACRO_NAME = get_second_word(line);
			/* check that the macro name is valid. */
			if (strlen(MACRO_NAME) > 31) {
				printf("\nERROR: in file \"%s\", line %d: the macro length exceeds the limit.\n", name_of_file, line_num);
				error0Count++;
				continue; /* pick the error and move to next line */
			}
			if (validMacroName(MACRO_NAME) == 0) {
				printf("\nERROR: in file \"%s\", line %d: there's an invalid macro name called \"%s\".\n", name_of_file, line_num, MACRO_NAME);
				error0Count++;
				continue; /* pick the error and move to next line */
			}
			/* check that the name isn't already an existing macro name */
			else if (isMacro(MACRO_NAME)) {
				printf("\nERROR: in file \"%s\", line %d: there is another macro definition with the same name of \"%s\".\n", name_of_file, line_num, MACRO_NAME);
				error0Count++;
				continue; /* pick the error and move to next line */
			}

			MACRO_FLAG = 1;
			/* put macro name in the macro table */
			if (addMacro(MACRO_NAME) == 0) {
				printf("\nERROR: in file \"%s\", line %d: Unable to create a macro node for macro: \"%s\".", name_of_file, line_num, MACRO_NAME);
				fclose(fd);
				return 2;
			}

			continue;
		} 
		
		/* reading through a macro definition */
		else if ((MACRO_FLAG == 1) && (strcmp(word, "endmacr") != 0))
		{
			/* put this line in the macro table */
			if (addMacroContent(line, MACRO_NAME) == 0) {
				printf("\nERROR: ERROR: in file \"%s\": Unable to store macro: \"%s\".\n", name_of_file, MACRO_NAME);
				fclose(fd);
				return 2;
			}
			continue;
		} 
		
		/* reached the end of a macro definition */
		else if ((strcmp(word, "endmacr") == 0) && (isOnlyWord(line) == 1)) 
		{
			MACRO_FLAG = 0;
			*MACRO_NAME = '\0'; 
			continue;
		}
		else
		{
			/* if reached here -- It's just a random text unrelated to a macro stuff */
			fputs(line, fd);
		}
	
		*line = '\0';		
	}	

	if (error0Count > 0) {
		fclose(fd);
		return 0;
	}
	/* save .am file */
	fclose(fd);

	return 1;
}
