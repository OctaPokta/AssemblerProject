runfile: main.o pre_processing/pre_assembler.o pre_processing/macros_table.o assembler/first_stage/first_stage.o assembler/second_stage/second_stage.o assembler/first_stage/first_stage_func.o assembler/second_stage/second_stage_func.o 
	gcc -ansi -Wall -pedantic main.o pre_processing/pre_assembler.o pre_processing/macros_table.o assembler/first_stage/first_stage.o assembler/second_stage/second_stage.o assembler/first_stage/first_stage_func.o assembler/second_stage/second_stage_func.o -o runfile

# main folder and the main function
main.o: main.c pre_processing/pre_assembler.o assembler/first_stage/first_stage.o assembler/second_stage/second_stage.o data.h pre_processing/pre_assembler.h assembler/excess_macro_list.h
	gcc -c -ansi -Wall -pedantic main.c


# (-----The Assembler-----)

# First stage
first_stage.o: assembler/first_stage/first_stage.c assembler/first_stage/first_stage_func.o pre_processing/macros_table.o pre_processing/macros_table.h assembler/assembler.h assembler/excess_macro_list.h
	gcc -c -ansi -Wall -pedantic assembler/first_stage/first_stage.c

first_stage_func.o: assembler/first_stage/first_stage_func.c pre_processing/macros_table.o pre_processing/macros_table.h assembler/assembler.h assembler/excess_macro_list.h
	gcc -c -ansi -Wall -pedantic assembler/first_stage/first_stage_func.c

# Second Stage
second_stage.o: assembler/second_stage/second_stage.c assembler/second_stage/second_stage.o pre_processing/macros_table.o pre_processing/macros_table.h assembler/assembler.h assembler/excess_macro_list.h
	gcc -c -ansi -Wall -pedantic assembler/second_stage/second_stage.c

second_stage_func.o: assembler/second_stage/second_stage_func.c pre_processing/macros_table.o assembler/first_stage/first_stage_func.o pre_processing/macros_table.h assembler/assembler.h assembler/excess_macro_list.h
	gcc -c -ansi -Wall -pedantic assembler/second_stage/second_stage_func.c


# (-----The Pre-Assembler-----)

# pre-assembler function.
pre_assembler.o: pre_processing/pre_assembler.c pre_processing/macros_table.o pre_processing/pre_assembler.h pre_processing/macros_table.h
	gcc -c -ansi -Wall -pedantic pre_processing/pre_assembler.c

# macro-table.
macros_table.o: pre_processing/macros_table.c pre_processing/macros_table.h assembler/assembler.h
	gcc -c -ansi -Wall -pedantic pre_processing/macros_table.c


# Clean
clean:
	rm -f *.o runfile
	rm -f pre_processing/*.o
	rm -f assembler/first_stage/*.o
	rm -f assembler/second_stage/*.o
