/*  ----------------
   | (MACRO LIST) |
   -----------------  
 ~(This list contains assisting macros to help reduce the function lengths and make it more readable)~ */


/* (used in "main" )*/
#define MAIN_CLEAN_BEFORE_EXIT \
    do { \
        freeMacro(); \
      	freeLabel(); \
		freeDataImage(); \
		freeInstructionImage(); \
		fclose(fd); \
    } while (0)

#define MAIN_CLEANUP_AND_CONTINUE \
    do { \
        freeLabel(); \
		freeMacro();  \
		freeDataImage(); \
		freeInstructionImage(); \
		fclose(fd); \
    } while (0)


/* (used in "first_stage.c") */

/* Free memory, reset state, increment error count, and clear line. */
#define CLEANUP_AND_CONTINUE \
    do { \
        free(dataOrString); \
		free(entryOrExtern); \
        LABEL_FLAG = 0; \
		L = 0; \
        err_count++; \
        *line = '\0'; \
    } while (0)


/* Free memory, reset state, and clear line on success. */
#define SUCCESS_AND_CONTINUE \
    do { \
        free(dataOrString); \
		free(entryOrExtern); \
		LABEL_FLAG = 0; \
		L = 0; \
		*line = '\0'; \
    } while (0)


/* Free memory and close file before exiting. */
#define CLEAN_BEFORE_EXIT \
    do { \
        free(dataOrString); \
		free(entryOrExtern); \
		fclose(fp); \
    } while (0)


/* allocate node and check for memory fault. */
#define ALLOCATE_NODE \
	Iptr t = (Iptr) malloc(sizeof(I_item)); \
	if (!t) { \
		return 0; \
	} 


/* (used in "first_stage_func.c") */

/* create memory cell and reset it. */
#define CREATE_AND_RESET_MILA \
	mila space; \
	space.MILA = 0; /* Initially set to all zeros */ 


/* returns error OR success accordiong to the result of the encodeSecondMila function */
#define CHECK_ERRORS_ENCODE_2ND_MILA \
	/* check for errors: */ \
	if (err_type == 2) { \
		return 2; /* loading error */ \
	} \
	else if (err_type == 1) { \
		return 1; /* success */ \
	}










