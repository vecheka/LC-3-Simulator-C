#include "lc3N.h"

LC *tempLc; // use to re-draw the menu when terminal resizes

/**
* Get opcode from the IR
* @param theIR instruction register
* @return opcode
*/
int getOpcode(Register theIR) {
  return (theIR & 0xF000) >> (HEX_BITS - CODE_BITS);
}

/**
* Get Destination Register from the IR
* @param theIR instruction register
* @return Destination Register
*/
int getDr(Register theIR) {
  return (theIR & 0x0F00) >> (HEX_BITS - (CODE_BITS * 2) + 1);
}


/**
* Get source register 1 from the IR
* @param theIR instruction register
* @return source register 1
*/
int getSr1(Register theIR) {
  return (theIR & 0x01C0) >> (CODE_BITS + 2);
}

/**
* Get source register 2 when bit[5] = 1 from the IR
* @param theIR instruction register
* @return source register 1
*/
int getSr2(Register theIR) {
  return (theIR & 0x0007) >> 0;
}

/**
* Check if bit 5 is 1 or 0
* @param theIR instruction register
* @return 1 if bit 5 is 1
*/
int isBitFiveOne(Register theIR) {
  return (theIR & 0x0020) >> (CODE_BITS + 1);
}


/**
* Get offset 6 from the IR
* @param theIR instruction register
* @return immed5
*/
int getOffset6(Register theIR) {
  return isBitFiveOne(theIR) == 1 ? (theIR & 0x003F) | 0xFF00 : (theIR & 0x003F);
}


/**
* Get offset 9 from the IR
* @param theIR instruction register
* @return immed8
*/
int getOffset9(Register theIR) {
  int offset9 = (theIR & 0x01FF);
  if ((offset9 & 0x0100) >> (CODE_BITS * 2) == 1) return offset9 | 0XFF00;
  return offset9; 
}

/**
* Get offset 11 from the IR
* @param theIR instruction register
* @return immed11
*/
int getOffset11(Register theIR) {
	int offset11 = (theIR & 0x07FF);
	if((offset11 & 0x07FF) >> JSR_BITS == 1) {
		return offset11 | 0xF800;
	}
	return offset11;
}

/**
* Get BaseR from the IR
* @param theIR instruction register
* @return BaseR
*/
int getBaseR(Register theIR){
  return (theIR & 0x01C0) >> (CODE_BITS + 2);
}

/**
* Determine if bit 11 is 1.
* @param theIR instruction register
* @return bit 11 in either 1 or 0
*/
int isBitElevenOne(Register theIR) {
	return(theIR & 0x0800) >> (JSR_BITS);
}

/**
* Load memory's data from a text file.
* @param lc LC class object
*/

void loadMemory(char * fileName) {

	FILE *file;
	initscr();
		
	file = fopen(fileName, "r");
	
	if (file == NULL) {
		clear();
		mvprintw(0, 0, "No such File or Directory\nExit(1)");
		refresh();
		sleep(1);
		clear();
		endwin();
		halt();
	}
	char hex[NO_OF_REGISTERS - 1];

	int i = 0;

	while(fscanf(file, "%s", hex) != EOF) {
		tempLc->memory[i++] = (Register) strtol(hex, NULL, HEX_BITS);
	}

	endwin();
	fclose(file);
}

/**
* Get a new display memory address
* @param lc LC class object
*/
void setNewDisplayMem(LC *lc, char * mem) {
	
	lc->start_address = (Register) strtol(mem, NULL, 16);
}

/**
* Handle resizing of the terminal.
* @param terminal's size
*/
void handle_winch(int sig) {
	signal(SIGWINCH, SIG_IGN);
	
	endwin();
	initscr();
	refresh();
	clear();
	
	// re-draw menu
	run(tempLc);
	
	refresh();
	signal(SIGWINCH, handle_winch);
}


/**
* Print Menu Options and all the registers and memory state of the machine
* to the console.
* @param lc LC class pointer
* @return user's selection
*/
void printMenu(LC * lc) {
    tempLc = lc;
	initscr();	
	cbreak();
	signal(SIGWINCH, handle_winch); // handle resizing of the terminal win
	noecho();
	
	char *reg = malloc(sizeof(char) * STRING_SIZE); // initialize char ptr for string 
	int xR = X_REG, yR = Y_REG, yM = Y_MEM; // register & mem coordinates
	
	
	// print title
	mvaddstr(5, COLS / 3, "Welcome To LC-3 Simulator\n");
	mvaddstr(xR, yR, "Registers\n");
	mvaddstr(xR, yM, "Memory\n");
	
	// print first 7 register and mem address
	int i;
	for (i = 0; i < NO_OF_REGISTERS; i++) {
		snprintf(reg, sizeof(reg), "R%d: ", i);
		mvaddstr(xR + 1, yR, reg);
		snprintf(reg, sizeof(reg), "x%04X", lc->cpus.reg_file[i]);
		mvaddstr(xR + 1, yR + REG_SPACE, reg);
		snprintf(reg, sizeof(reg), "x%04X: ", lc->start_address + i);
		mvaddstr(xR + 1, yM, reg);
		snprintf(reg, sizeof(reg), "x%04X", lc->memory[i]);
		mvaddstr(xR + 1, yM + MEM_SPACE, reg);
		xR += 1;
	}
	
	// print the next 2 mem address
	yM = Y_MEM;
    for (i = NO_OF_REGISTERS; i < MEM_SIZE - 5; i++) {
		snprintf(reg, sizeof(reg), "x%04X: ", lc->start_address + i);
		mvaddstr(xR + 1, yM, reg);
		snprintf(reg, sizeof(reg), "x%04X",lc->memory[i]);
		mvaddstr(xR + 1, yM + MEM_SPACE, reg);
		xR += 1;
	}
	
	
	// print CPU's members and the remaining mem address
	int memA = MEM_SIZE - 5;
	yR = Y_REG;
	yM= Y_MEM;
	int space1 = 5, space2 = 12, space3 = 17;
	mvaddstr(xR + 1, yR, "PC:");
	snprintf(reg, sizeof(reg), "x%04X", lc->cpus.PC | lc->start_address);
	mvaddstr(xR + 1, yR + space1, reg);
	mvaddstr(xR + 1, yR + space2, "IR:");
	snprintf(reg, sizeof(reg), "x%04X", lc->cpus.IR);
	mvaddstr(xR + 1, yR + space3, reg);
	snprintf(reg, sizeof(reg), "x%04X: ", lc->start_address + memA);
	mvaddstr(xR + 1, yM, reg);
	snprintf(reg, sizeof(reg), "x%04X\n", lc->memory[memA++]);
	mvaddstr(xR + 1, yM + MEM_SPACE, reg);
	
	xR += 1;
	yR = Y_REG;
	yM= Y_MEM;
	mvaddstr(xR + 1, yR , "A:");
	snprintf(reg, sizeof(reg), "x%04X", lc->cpus.A);
	mvaddstr(xR + 1, yR + space1, reg);
	mvaddstr(xR + 1, yR + space2, "B:");
	snprintf(reg, sizeof(reg), "x%04X", lc->cpus.B);
	mvaddstr(xR + 1, yR + space3, reg);
	snprintf(reg, sizeof(reg), "x%04X: ", lc->start_address + memA);
	mvaddstr(xR + 1, yM, reg);
	snprintf(reg, sizeof(reg), "x%04X\n", lc->memory[memA++]);
	mvaddstr(xR + 1, yM + MEM_SPACE, reg);
	
	xR += 1;
	yR = Y_REG;
	yM= Y_MEM;
	mvaddstr(xR + 1, yR, "MAR:");
	snprintf(reg, sizeof(reg), "x%04X", lc->cpus.MAR);
	mvaddstr(xR + 1, yR + space1, reg);
	mvaddstr(xR + 1, yR + space2, "MDR:");
	snprintf(reg, sizeof(reg), "x%04X", lc->cpus.MDR);
	mvaddstr(xR + 1, yR + space3, reg);
	snprintf(reg, sizeof(reg), "x%04X: ", lc->start_address + memA);
	mvaddstr(xR + 1, yM, reg);
	snprintf(reg, sizeof(reg), "x%04X\n", lc->memory[memA++]);
	mvaddstr(xR + 1, yM + MEM_SPACE, reg);
	
	xR += 1;
	yR = Y_REG;
	yM= Y_MEM;
	space1 = 5; space2 = 8;
	mvaddstr(xR + 1, yR + 1, "CC:");
	mvaddstr(xR + 1, yR + space1, "N:");
	snprintf(reg, sizeof(reg), "%d  ", lc->cpus.n);
	mvaddstr(xR + 1, yR + space2, reg);
	mvaddstr(xR + 1, yR + space2 + 2, "Z: ");
	snprintf(reg, sizeof(reg), "%d  ", lc->cpus.z);
	space2 += space1;
	mvaddstr(xR + 1, yR + space2, reg);
	mvaddstr(xR + 1, yR + space2 + 2, "P: ");
	space2 += space1;
	snprintf(reg, sizeof(reg), "%d", lc->cpus.p);
	mvaddstr(xR + 1, yR + space2, reg);
	snprintf(reg, sizeof(reg), "x%04X: ", lc->start_address + memA);
	mvaddstr(xR + 1, yM, reg);
	snprintf(reg, sizeof(reg), "x%04X", lc->memory[memA++]);
	mvaddstr(xR + 1, yM + MEM_SPACE, reg);
	xR += 1;
	snprintf(reg, sizeof(reg), "x%04X:", lc->start_address + memA);
	mvaddstr(xR + 1, yM, reg);
	snprintf(reg, sizeof(reg), "x%04X", lc->memory[memA++]);
	mvaddstr(xR + 1, yM + MEM_SPACE, reg);
	
		
	
	xR += 1;
	yR = Y_REG;
	
	
	
	mvaddstr(xR + 2, yR, "Select: 1) Load, 2) Run, 3) Step, 5) Display Mem, 9) Exit\n");
	xR += 1;
	mvaddstr(xR + 3, yR - 2, "> ");
	int selection = getch();
	xR += 1;
	mvaddstr(xR + 3, yR, "----------------------------------------------------------");

	lc->cpus.reg_file[0] = selection;
	
	selection = out(lc) - 0x30;
	xR += 1;
	mvaddstr(xR + 5, yR, "Input:");
	snprintf(reg, sizeof(reg), "%c", out(lc));
	mvaddstr(xR + 5, yR + MEM_SPACE + 1, reg);
	
	xR += 1;
	mvaddstr(xR + 5, yR, "Output:");
	
	if (selection == LOAD) {
		char * fileName = malloc(sizeof(char) * STRING_SIZE);
		
		mvaddstr(xR + 5, yR + MEM_SPACE + 1, PUTS("Enter a file name: "));
		echo();
		getstr(fileName);
		loadMemory(fileName);
		refresh();
		free(fileName);
	} else if (selection == DISPLAY_MEM) {
		char * mem = malloc(sizeof(char) * STRING_SIZE);
		
		mvaddstr(xR + 5, yR + MEM_SPACE + 1, PUTS("Enter a memory address: "));
		echo();
		getstr(mem);
		setNewDisplayMem(lc, mem);
		refresh();
		sleep(1);
		free(mem);
	} else if (selection == EXIT) {
		
		mvaddstr(xR + 5, yR + MEM_SPACE + 1, PUTS("Halting..."));
		refresh();
		sleep(1);
		clear();
		halt();
	} else if (selection == STEP || selection == RUN) {
		
		if (selection == STEP) mvaddstr(xR + 5, yR + MEM_SPACE + 1, PUTS("Stepping..."));
		if (selection == RUN) mvaddstr(xR + 5, yR + MEM_SPACE + 1, PUTS("Running..."));
		debug_monitor(lc, selection);
		refresh();
		sleep(1);
	} 
	
	clear();
	endwin();
	free(reg);

}

/**
* Execute the program.
* @param lc LC class object
*/
void run(LC * lc) {
	while (1) {
		printMenu(lc);
	}
}


/**
* Initialize LC simulator.
* @param lc LC class object
*/
void initialize(LC *lc) {
	lc->start_address = STARTING_ADDRESS; //intialize default starting address
	lc->cpus.PC = 0;
	lc->cpus.A = 0;
	lc->cpus.B = 0;
	lc->cpus.MDR = 0;
	lc->cpus.MAR = 0;
	lc->cpus.IR = 0;
	lc->cpus.n = 0;
    lc->cpus.z = 0;
    lc->cpus.p = 0;
	int i;
	for(i = 0; i < NO_OF_REGISTERS; i++) {
		lc->cpus.reg_file[i] = 0;
	}
}

/**
* Trap 0x21 implementation for printing a single character.
* @param lc class object
* @return character in R0 register
*/
char out(LC * lc) {
  return ((lc->cpus.reg_file[0] & 0x00FF));
}

/**
* Trap 0x25 implementation for halting.
*/
void halt() {
  initscr();
  mvaddstr(0, 0, "Thank you for using!\nSee ya later!");
  refresh();
  sleep(1);
  endwin();
  exit(0);
}

/**
* Trap 0x22 implementation for printing an array of characters.
* @param theString an array of characters
*/
char * PUTS(char * theString) {
  return theString;
}



/**
* Execute the program
* @param lc LC class object
*/
void debug_monitor(LC *lc, int option) {

	unsigned int opcode, Dr, Sr1, Sr2, currentPC;
	Register ir, tempPC;


	int state = FETCH;
	int hasStore = 0;
  for (;;) {
    switch(state) {

      case FETCH:
        
        currentPC = lc->cpus.PC;
        lc->cpus.IR = lc->memory[lc->cpus.PC++];
        ir = lc->cpus.IR;
        state = DECODE;
        break;

      case DECODE:
        
        // get opcode
        opcode = getOpcode(lc->cpus.IR);
        state = EVAL_ADDR;
        break;

      case EVAL_ADDR:

        switch(opcode) {
          case ADD:
            // Bit[5] = 0 -> DR <= SR1 + SR2
            // Bit[5] = 1 -> DR <= SR1 + immed5 
            Dr = getDr(ir);
            Sr1 = getSr1(ir);
            if (isBitFiveOne(ir)) lc->cpus.SEXT = getOffset6(ir);
            else Sr2 = getSr2(ir);
            break;
          case AND:
            // Bit[5] = 0 -> DR <= SR1 & SR2
            // Bit[5] = 1 -> DR <= SR1 & immed5
            Dr = getDr(ir);
            Sr1 = getSr1(ir);
            if (isBitFiveOne(ir)) lc->cpus.SEXT = getOffset6(ir);
            else Sr2 = getSr2(ir);
            break;
          case JMP:
            // PC <= BaseR (SR1)
            //Sr1 = getSr1(ir);
            if(getBaseR(ir) == R7){
                Sr1 = R7;
            } else {
                Sr1 = getSr1(ir);
            }
            break;
          case LD:
            // DR <= Mem[PC + offset9]
            Dr = getDr(ir);
            lc->cpus.SEXT = getOffset9(ir);
            break;
          case LDR:
            // DR <= BaseR (SR1) + immed5
            Dr = getDr(ir);
            Sr1 = getSr1(ir);
            lc->cpus.SEXT = getOffset6(ir);
            break;
          case LEA:
            // DR <= PC + offset9
            Dr = getDr(ir);
            lc->cpus.SEXT = getOffset9(ir);
            break;
          case NOT:
            // DR <= NOT(SR1)
            Dr = getDr(ir);
            Sr1 = getSr1(ir);
            break;
          case ST:
            // Mem[offset9] <= SR(DR)
            Dr = getDr(ir);
            lc->cpus.SEXT = getOffset9(ir);
            break;
          case STR:
            Dr = getDr(ir);
            break;
          case JSR:		
			// Bit[11] = 1 <= PC = BaseR
			// Bit[11] = 0 <= PC += offset11		  //added -sally
            tempPC = lc->cpus.PC;
			if (isBitElevenOne(ir)) lc->cpus.SEXT = getOffset11(ir); 
			else lc->cpus.SEXT = getBaseR(ir);
			lc->cpus.reg_file[R7] = tempPC;
        	break;								//added -sally
         
          case BR:
            lc->cpus.SEXT = getOffset9(ir);
            lc->cpus.p = (ir >> BR_OFFSET) & 0x0001;
            lc->cpus.z = (ir >> BR_OFFSET + 1) & 0x0001;
            lc->cpus.n = (ir >> BR_OFFSET + 2) & 0x0001;
            break;
		  case HALT:
            return;
            break;
        }

        state = FETCH_OP;

        break;

      case FETCH_OP:

        switch(opcode) {
          case ADD:
            // Bit[5] = 0 -> DR <= SR1 + SR2
            // Bit[5] = 1 -> DR <= SR1 + immed5 
            if (isBitFiveOne(ir)) {
              lc->cpus.A = lc->cpus.reg_file[Sr1];
              // immed5 = lc->cpus.SEXT;
              lc->cpus.B = lc->cpus.SEXT;
            } else {
              lc->cpus.A = lc->cpus.reg_file[Sr1];
              lc->cpus.B = lc->cpus.reg_file[Sr2];
            }
            break;
          case AND:
            // Bit[5] = 0 -> DR <= SR1 & SR2
            // Bit[5] = 1 -> DR <= SR1 & immed5
            if (isBitFiveOne(ir)) {
              lc->cpus.A = lc->cpus.reg_file[Sr1];
              // immed5 = lc->cpus.SEXT;
              lc->cpus.B = lc->cpus.SEXT;
            } else {
              lc->cpus.A = lc->cpus.reg_file[Sr1];
              lc->cpus.B = lc->cpus.reg_file[Sr2];
            }
            break;
          case JMP:
            // PC <= BaseR (SR1)
            lc->cpus.A = lc->cpus.reg_file[Sr1];
            break;
          case LD:
            // DR <= Mem[PC + offset9]
            // immed9 = lc->cpus.SEXT;
            lc->cpus.A = lc->cpus.SEXT;
            break;
          case LDR:
            // DR <= BaseR (SR1) + immed5
            lc->cpus.A = Sr1;
            // immed5 = lc->cpus.SEXT;
            lc->cpus.B = lc->cpus.SEXT;
            break; 
          case LEA:
            // DR <= PC + offset9
            // immed9 = lc->cpus.SEXT;
            lc->cpus.A = lc->cpus.SEXT;
            break;
          case NOT:
            // DR <= NOT(SR1)
            lc->cpus.A = lc->cpus.reg_file[Sr1];
            break;
          case ST:
            // Mem[offset9] <= SR(DR)
            // immed9 = lc->cpus.SEXT;
            lc->cpus.B = lc->cpus.SEXT;
            lc->cpus.A = lc->cpus.reg_file[Dr];
            break;
          case STR:
            lc->cpus.A = getBaseR(ir);//baseR
            lc->cpus.B = getOffset6(ir);
            break;
          case JSR:	
			// Bit[11] = 1 <= PC = BaseR
			// Bit[11] = 0 <= PC += BaseR		  //added -sally
        	lc->cpus.A = tempPC;
			lc->cpus.B = lc->cpus.SEXT; 		//added -sally
        	break;
          case BR:
			lc->cpus.A = lc->cpus.PC;
			lc->cpus.B = lc->cpus.SEXT;
			break;
		  case HALT:
            return;
            break;
        }

        state = EXECUTE;

        break;

      case EXECUTE:
        switch(opcode) {
        case ADD:
          // Bit[5] = 0 -> DR <= SR1 + SR2
          // Bit[5] = 1 -> DR <= SR1 + immed5 
          lc->cpus.R = lc->cpus.A + lc->cpus.B;
          break;
        case AND:
          // Bit[5] = 0 -> DR <= SR1 & SR2
          // Bit[5] = 1 -> DR <= SR1 & immed5
          lc->cpus.R = lc->cpus.A & lc->cpus.B;
          break;
        case JMP:
          // PC <= BaseR (SR1)
          lc->cpus.R = lc->cpus.A;
          break;
        case LD:
          // DR <= Mem[PC + offset9]
          lc->cpus.MAR = currentPC + lc->cpus.A;
          lc->cpus.R = lc->memory[currentPC + lc->cpus.A];
          break;
        case LDR:
          // DR <= Mem[BaseR (SR1) + immed5]
          lc->cpus.MAR = lc->cpus.A + lc->cpus.B;
          lc->cpus.R = lc->memory[lc->cpus.A + lc->cpus.B];
          break;
        case LEA:
          // DR <= PC + offset9
          lc->cpus.R = currentPC + lc->cpus.A;
          break;
        case NOT:
          // DR <= NOT(SR1)
          lc->cpus.R = ~(lc->cpus.A);
          break;
        case ST:
          // Mem[offset9] <= SR(DR)
          lc->cpus.R = lc->cpus.A;
          break;
        case STR:
          lc->cpus.R = lc->cpus.A + lc->cpus.B;
          break;
        case JSR: 						
          // Bit[11] = 1 <= PC = BaseR
		  // Bit[11] = 0 <= PC += BaseR
          if(isBitElevenOne(ir)) {
        	  lc->cpus.PC = lc->cpus.A + lc->cpus.B;
          } else lc->cpus.PC = lc->cpus.B;
          break;						
        case BR:
          lc->cpus.R = lc->cpus.A + lc->cpus.B;
           break;
		case HALT:
          return;
          break;
      }

      state = STORE;
      break;

      case STORE:

        hasStore = 1;
        switch(opcode) {
          case ADD:
            // Bit[5] = 0 -> DR <= SR1 + SR2
            // Bit[5] = 1 -> DR <= SR1 + immed5 
            lc->cpus.reg_file[Dr] = lc->cpus.R;
			lc->cpus.MAR = Dr;
			lc->cpus.MDR = lc->cpus.R;
            break;
          case AND:
            // Bit[5] = 0 -> DR <= SR1 & SR2
            // Bit[5] = 1 -> DR <= SR1 & immed5
            lc->cpus.reg_file[Dr] = lc->cpus.R;
			lc->cpus.MAR = Dr;
			lc->cpus.MDR = lc->cpus.R;
            break;
          case JMP:
            // PC <= BaseR (SR1)
            lc->cpus.PC = lc->cpus.R;
			lc->cpus.MAR = lc->cpus.R;
			lc->cpus.MDR = lc->cpus.R;
            break;
          case LD:
            // DR <= Mem[PC + offset9]
			lc->cpus.MAR = Dr;
			lc->cpus.MDR = lc->cpus.R;
            lc->cpus.reg_file[Dr] = lc->cpus.R;
            break;
          case LDR:
            // DR <= BaseR (SR1) + immed5
            lc->cpus.MDR = lc->cpus.R;
			lc->cpus.MAR = Dr;
            lc->cpus.reg_file[Dr] = lc->cpus.R;
            break;
          case LEA:
            // DR <= PC + offset9
			lc->cpus.MAR = Dr;
			lc->cpus.MDR = lc->cpus.R;
            lc->cpus.reg_file[Dr] = lc->cpus.R;
            break;
          case NOT:
            // DR <= NOT(SR1)
            lc->cpus.reg_file[Dr] = lc->cpus.R;
			lc->cpus.MAR = Dr;
			lc->cpus.MDR = lc->cpus.R;
            break;
          case ST:
            // Mem[offset9] <= SR(DR)
            lc->memory[lc->cpus.B] = lc->cpus.R;
			lc->cpus.MAR = lc->cpus.B;
			lc->cpus.MDR = lc->cpus.R;
            break;
        	case STR:
			lc->cpus.MAR = lc->cpus.R;
			lc->cpus.MDR = lc->cpus.reg_file[Dr];
            lc->memory[lc->cpus.R] = lc->cpus.reg_file[Dr];
            break;
         case JSR:									
        	//R7 = TEMP*
        	// lc->cpus.reg_file[R7] = lc->cpus.PC;
			lc->cpus.MDR = lc->cpus.reg_file[R7];
			lc->cpus.MAR = R7;
			lc->cpus.PC = lc->cpus.reg_file[R7];
        	break;								
        
         case BR:
            if((lc->cpus.n && NZP) || (lc->cpus.z && NZP) || (lc->cpus.p && NZP)){
                lc->cpus.PC = lc->cpus.R;
            }
			break;
		 case HALT:
            return;
            break;
        }
        state = FETCH;
        break;
    }
    if (option == STEP && hasStore) break;
  }
}
