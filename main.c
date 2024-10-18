/*
 * main.c - Main file for the assembler project
 * This file contains the main function which drives the assembler program.
 * The assembler program takes multiple input files, processes them through a pre-assembler, 
 * and then runs two stages of the assembler to produce the final output files.
 */

#include "data.h"
#include "pre_processing/pre_assembler.h"
#include "pre_processing/macros_table.h"
#include "assembler/assembler.h"
#include "assembler/excess_macro_list.h"

/* 
 * main - Main function for the assembler program.
 * @argc: Number of command line arguments.
 * @argv: Array of command line arguments, each representing an input file name.
 * 
 * The function processes each input file through the pre-assembler, first stage, 
 * and second stage of the assembler. If any errors occur during processing, they 
 * are reported, and the program moves on to the next file.
 * 
 * Return: 1 on success, 0 on error.
 */
int main(int argc, char *argv[]) 
{ 
	int i, file_error_count = 0;
	
	/* in case there are no input files */
	if (argc == 1) {
		printf("\nERROR: You must enter input files.\n");
		return 0;;
	} else {
		NUM_OF_FILES = argc-1;
	}


	/* --(scrolling between input files)-- */
	for (i = 1; i < argc; i++) 
	{
		int pre_assemblerErrorType;
		int first_stageErrorType;
		int second_stageErrorType;
		char src_filename[256];	
		int file_name_length = strlen(argv[i]);
		FILE *fd;

		/* check in case the input files are too long */
		if (file_name_length >= 256) { 
			printf("\nERROR: in file \"%s\", the file name is too long.\n", argv[i]);
			return 0;
		}	

		/* open the i'th file, store in a pointer */
		sprintf(src_filename, "%s.as", argv[i]);

		fd = fopen(src_filename, "r"); 
		if (fd == NULL) {
			printf("ERROR: Unable to open file: \"%s\".\n", argv[i]);
			file_error_count++;
			continue;
		}		
		
		/* --(Run the pre-assembler)-- */
		pre_assemblerErrorType = pre_assembler(fd, i, argv[i]);
		
		if (pre_assemblerErrorType == 0) 
		{
			/* in case pre-assembler failed */
			char filename[256];			
			sprintf(filename, "%s.am", argv[i]);

			printf("\nERROR in pre-assembler of file \"%s\". moving to next file.\n", src_filename);
			MAIN_CLEANUP_AND_CONTINUE;
			continue;
		}
		else if (pre_assemblerErrorType == 2) 
		{
			/* in case pre-assembler failed due to memory error */
			printf("\nMEMORY ERROR in pre-assembler of file \"%s\". Exiting program.\n", src_filename);
			MAIN_CLEAN_BEFORE_EXIT;
			return 0;
		}

		/* AT THIS POINT THE .am file has been created succesfully */
		printf("Pre-assembler of file \"%s\" is finished successfully.\n", argv[i]);

		/* --(run the first stage of the assembler)-- */	
		first_stageErrorType = first_stage(argv[i]);	
		if (first_stageErrorType == 0) 
		{
			/* invalid regular error */
			char filename[256];			
			sprintf(filename, "%s.am", argv[i]);
			printf("\nERROR in assembler of file \"%s\". moving to next file.\n", src_filename);
			MAIN_CLEANUP_AND_CONTINUE;
			continue;
		}
		else if (first_stageErrorType == 2) 
		{
			/* memory error */
			printf("\nMEMORY ERROR in assembler of file \"%s\". Exiting program.\n", src_filename);
			MAIN_CLEAN_BEFORE_EXIT;
			return 0;
		}

		/* --(run the second stage of the assembler)--  */ 
		second_stageErrorType = second_stage(argv[i]);
		if (second_stageErrorType == 0) 
		{
			/* invalid regular error */
			char filename[256];			
			sprintf(filename, "%s.am", argv[i]);
			printf("\nERROR in assembler of file \"%s\". moving to next file.\n", src_filename);
			MAIN_CLEANUP_AND_CONTINUE;
			continue;
		}
		else if (second_stageErrorType == 2) 
		{
			/* memory error */ 
			printf("\nMEMORY ERROR in assembler of file \"%s\". Exiting program.\n", src_filename);
			MAIN_CLEAN_BEFORE_EXIT;
			return 0;
		}  
	
		/* AT THIS POINT THE output files have been created succesfully */
		printf("\nAssembler of file \"%s\" is finished successfully.\n", argv[i]);

		printf("\n"); /* new line before the next file*/
		
		/* free linked lists of the current file */
		MAIN_CLEANUP_AND_CONTINUE;
	}

	/* in case all input files are unreadable */
	if (file_error_count == argc-1) {
		printf("\n\nERROR: Notice! ALL of the input files are unreadable.\n");
		printf("Unable to read files, Exiting program...\n");
		return 0;
	}

	return 1;
}
