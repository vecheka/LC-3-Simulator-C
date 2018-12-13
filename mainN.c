#include "lc3N.h"
/**
* @Program Outlines: 
*	This program should implements the LC-3 instructions except LDI, RTI, STI, and mimicks the LC-3 simulator. 
*   This program should also implements TRAP's 20,21,22, and 25 :
*        TRAPx20- GETC: This gets a character from the keyboard 
*        TRAPx21- OUT : This writes [7:0] in register 1 to the monitor 
*        TRAPx22- PUTS: This writes a string to the monitor 
*        TRAPx25- HALT: This prints the message to a monitor and halts the execution.                  
*   
*   The purpose of this program is to ask the user for a hex file 
*   (we used memory.hex to run and test) with a list of hexadecimal 
*   values for a specific LC-3 routine. This user has the choice to step or run through
*   each memory address until a halt instruction is met.          
*                                    
*   The program should step through using a debug monitor program that 
*   keeps track of which register has which values and the PC should 
*   always increment 1 unless it jumps to another PC to perform that value.
*     
*   The user should be able to 1.) load, 2,) run, 3.) step, and 4.) exit the program.
*   The user can also start at 5.) DISP Mem which will allow the user to choose their starting memory address
*   that they want to display instead (i.e Default Mem Address 3000 -> 3555).                 
*    
*   This program utilizes ncurses.c library in C.
*
* @Authors: Vecheka Chhourn, Sally Ho, David Chau, Grant Christopher Schorbach
* @Date: 11/27/2018 version 2.0
* *Note*: Please compile using "gcc mainN.c lc3N.c -lncurses"             
*
*/




/**
* Main class to executes the program.
*/
int main(void) {

	LC *lc = malloc(sizeof(LC));
	initialize(lc);	// initialize registers, and cpu at the start
	
	run(lc);
	
	free(lc);
	
	return 0;
}	