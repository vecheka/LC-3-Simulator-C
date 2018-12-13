#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h> 
#include <ncurses.h>
#include <signal.h>
#ifndef LC_H
#define LC_H

#define FETCH 0
#define DECODE 1
#define EVAL_ADDR 2
#define FETCH_OP 3
#define EXECUTE 4
#define STORE 5

#define BR 0 
#define ADD 1
#define LD 2
#define ST 3
#define JSR 4
#define AND 5
#define LDR 6
#define STR 7
#define NOT 9
#define JMP 12
#define LEA 14
#define HALT 15

#define LOAD 1
#define RUN 2 
#define STEP 3
#define DISPLAY_MEM 5
#define EXIT 9

#define R7 7
#define CODE_BITS 4
#define JSR_BITS 11 // COULD BE 10
#define HEX_BITS 16
#define MEM_SIZE 16
#define BR_OFFSET 9
#define NO_OF_REGISTERS 8
#define NZP 1

#define STRING_SIZE 50
#define STARTING_ADDRESS 0x3000
#define X_REG 7
#define Y_REG COLS / 4
#define Y_MEM (COLS / 2) + 5
#define REG_SPACE 4
#define MEM_SPACE 7


typedef unsigned short Register;



/* CPU class.*/
typedef struct cpu_s {
  Register IR, PC, SEXT, MDR, MAR, A, B, R, n, z, p;
  Register reg_file[NO_OF_REGISTERS];
} CPU_s;

/* LC_3 class*/
typedef struct lc {
  CPU_s cpus;
  Register start_address;
  Register memory[MEM_SIZE];
} LC;



/** Functions Declarations.*/
int getOpcode(Register);
int getDr(Register);
int getSr1(Register);
int getSr2(Register);
int isBitFiveOne(Register);
int isBitElevenOne(Register);
int getOffset6(Register);
int getOffset9(Register);
int getOffset11(Register);
int getBaseR(Register);
void loadMemory(char *);
void printMenu(LC *);
void handle_winch(int);
void setNewDisplayMem(LC *, char *);
char out(LC *);
void halt();
char * PUTS(char *);
void initialize(LC *);
void debug_monitor(LC *, int);
void run(LC *);



#endif