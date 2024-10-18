/*
 * ASSEMBLER DATA STRUCTURES AND MEMORY MANAGEMENT
 *
 * This header file defines the data structures and functions
 * used for managing labels, data memory image, and instructions
 * memory image in an assembler program.
 *
 * Sections:
 * - Label Table: Defines structures and functions for label management.
 * - Memory Image: Defines structures and functions for the computer general memory management.
 * - Declarations: Provides function declarations for various assembler operations.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 81 /* max line size */


/*  -----------------
   | (Label Table) |
   -----------------  */


typedef struct Lnode *Lptr;
/* struct of a label node */
typedef struct Lnode {
	char *label_name;
	int value;
	char *type;
	struct Lnode *next;
} l_item;
/* Global head pointer for the label linked list */
struct Lnode *Labelhptr;


/*  -----------------
   | (MEMORY IMAGE) |
   -----------------  */


#define MEMORY_SIZE 4096

/* this struct defines a cell of 16-bits in memory */
typedef struct {
	unsigned short MILA; /* last bit is ignored, only supports positive */
} mila;

/* --(Entire Memory Image)-- */

mila memory_image[MEMORY_SIZE]; 

/* --(Data Memory Image)-- */

typedef struct dataNode *Dptr; /* declare pointer */
/* this struct defines a cell in the data image */
typedef struct dataNode {
	mila cell;
	int address; 
	struct dataNode *next;
} d_item;
struct dataNode *Data_hptr; /* head pointer */

int encodeData(char*, int, char*, int);
int addExtern(char*, char*, int, int);
int addData(int, int);
int addString(char, int);
int loadLabelExtern(char*, char*, int); 
void printDataImage(); /* this function is used for test purposes only */
void freeDataImage();

/* --(Instructions Memory Image)-- */

typedef struct InstructionNode *Iptr; /* declare pointer */
/* this struct defines a cell in the instruction image */
typedef struct InstructionNode {
	mila cell;
	int address; 
	struct InstructionNode *next;
	struct InstructionNode *prev;
} I_item;
struct InstructionNode *Instruction_hptr; /* head pointer */


/* this struct is used on an array when defining specifics about every instruction */
typedef struct ins_length {
    char *instruction; 
    int operand_num;
} ins_length;

int encodeInstruction(char*, char*, int, int, int);
int addInstruction(char*, int, char*, char*, int, int, int);
int getAddressingType(char*, int, char*, char*);
void printInstructionImage(); /* this function is used for test purposes only */
void freeInstructionImage();


/*  -----------------
   | (DECLERATIONS) |
   -----------------  */


int first_stage(char*);
int second_stage(char*);

int clearOfMacro(char*, char*, int);
char *findDataOrStringWord(char*, char*, int);
char *findEntryOrExternWord(char*, char*, int);
int isLabel(char*, char*, int);
int isAlreadyLabel(char*);
char *getLabelStatus(char*);
int getLabelAddress(char*);
int validInstructionName(char*);
int validInstructionAddress(char*, int);
int valid2operandsAddress(char*, int, int);
int validOperandAddress(char*, int , char*);
int replaceCommas(char*);
int addLabel(char*, int, char*, char*, int);
void printLabel(); /* this function is used for test purposes only */
void updateLabels(int);
void freeLabel();
char *addSpacesAfterCommas(char*);
char *replaceCommasWithSpaces(char*);

int addEntry(char*, int, char*);
int changeEntryStatus(char*, int, char*);
char *get_third_word(char*);
char *get_fourth_word(char*);
int encodeMissingOperand(char*, int, char*, int, int);
int encodeLabelMila(char*, int);
int findNumOfWords(char*);
int LabelWasAlreadyEncoded(int);
int loadPCMemory();
int createOutput(char*, int, FILE*);
int countInstructionCell();
int countDataCell();
int entryLabelsExists();
int externLabelExists();
int write2Extern(char*, int, FILE*, FILE*);
int isExternLabel(char*);
int isDataORstring(char*, char*);
int isExternORentry(char*, char*);
int checkRegisters(char*, char*, char*);
int isRegister( char*);
void print2ExternFile(char*, char*, char*, char*, int, FILE*);
int check4OperandEntry(char*, int, char*, int);

/* excess functions */
void insertDataToNode(Iptr, mila, int);
void assignNodeToList(Iptr);
char *skipFirstWord(char*);
int checkERRloadLabelADRRtype(int, mila, int, Iptr);
int checkValidOperands(int, int, char*, int, int);
int encodeMila(char*, int, int, char*, char*, int, char*);
int encodeRegisterMilaOnly(char*, int, char*, int);
int isAddressType2Or3(char*, char*, int);
char *skip_word(char*);
void printPCmemory(); /* this function is used for test purposes only */



