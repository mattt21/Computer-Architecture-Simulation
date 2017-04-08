
/*
    Name 1: Matthew Tawil
    UTEID 1: mt33924
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    /* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                                      (x[J3] << 3) + (x[J2] << 2) +
                                      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
 MEMORY[A][1] stores the most significant byte of word at word address A
 There are two write enable signals, one for each byte. WE0 is used for
 the least significant byte of a word. WE1 is used for the most significant
 byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{
    
    int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */
    
    int READY;	/* ready bit */
    /* The ready bit is also latched as you dont want the memory system to assert it
     at a bad point in the cycle*/
    
    int REGS[LC_3b_REGS]; /* register file. */
    
    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */
    
    int STATE_NUMBER; /* Current State Number - Provided for debugging */
    
    /* For lab 4 */
    int INTV; /* Interrupt vector register */
    int EXCV; /* Exception vector register */
    int SSP; /* Initial value of system stack pointer */
    /* MODIFY: You may add system latches that are required by your implementation */
    
    int PSR; /* Program Status Register */
    
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
    
    eval_micro_sequencer();
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();
    
    CURRENT_LATCHES = NEXT_LATCHES;
    
    CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;
    
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    
    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    
    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */
    
    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");
    
    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;
    
    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");
    
    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;
    
    printf("LC-3b-SIM> ");
    
    scanf("%s", buffer);
    printf("\n");
    
    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;
            
        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;
            
        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);
            
        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;
            
        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];
    
    printf("Loading Control Store from file: %s\n", ucode_filename);
    
    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }
    
    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n",
                   i, ucode_filename);
            exit(-1);
        }
        
        /* Put in bits one at a time. */
        index = 0;
        
        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
                       ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
                       ucode_filename, i, j);
                exit(-1);
            }
            
            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
            index++;
        }
        
        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
                   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;
    
    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;
    
    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }
    
    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }
    
    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }
        
        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }
    
    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);
    
    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);
    
    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    
    NEXT_LATCHES = CURRENT_LATCHES;
    
    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;
    
    /* Error Checking */
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }
    
    printf("LC-3b Simulator\n\n");
    
    initialize(argv[1], argv[2], argc - 2);
    
    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }
    
    while (1)
        get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
 with a "MODIFY:" comment.
 
 Do not modify the rdump and mdump functions.
 
 You are allowed to use the following global variables in your
 code. These are defined above.
 
 CONTROL_STORE
 MEMORY
 BUS
 
 CURRENT_LATCHES
 NEXT_LATCHES
 
 You may define your own local/global variables and functions.
 You may use the functions to get at the control bits defined
 above.
 
 Begin your code here 	  			       */
/***************************************************************/
int TIMER_COUNT = 300;
int TIMER_INTV = 0x01;
int initialCycle = 0;
int memoryAccessToggle = 0;
int marmux, pc, alu, shf, mdr;
int addr1, addr2;
int SR, DR;
int SR2value;

int twosComp(int x){
    if(x & 0x8000){ x = x^(-1); x++; return Low16bits(x);}
    return Low16bits(x);
}

int SEXT(int x, int amt, int sign){
    if(sign == 0)
        return Low16bits(x);
    if(amt == 11)
        return Low16bits(x | 0xFFE0);
    else if(amt == 10)
        return Low16bits(x | 0xFFC0);
    else if(amt == 7)
        return Low16bits(x | 0xFE00);
    else if(amt == 5)
        return Low16bits(x | 0xF800);

    return x;
}

int rshf(int SR, int amt, int sign){
    if(sign == 0) return Low16bits(SR >> amt);
    SR = Low16bits(~SR + 1);
    SR = SR >> amt;
    SR = ~SR;
    return Low16bits(SR);
}

void push(int address, int data){
    address -= 0x0001;
    MEMORY[address/2][0] = data & 0x00FF;
    MEMORY[address/2][1] = (data & 0xFF00) >> 8;
    NEXT_LATCHES.REGS[6] = address;
}

int pop(int address){
    int data = 0;
    data += MEMORY[address/2][0];
    data += MEMORY[address/2][1] << 8;
    address += 0x0001;
    NEXT_LATCHES.REGS[6] = address;
    return data;
}


void eval_micro_sequencer() {
    
    /*
     * Sets PSR[15] = 0
     * Sets Interrupt Vector to 0x01
     * Sets R6 to Supervisor Stack Pointer
     */
    if(CYCLE_COUNT == TIMER_COUNT){
        NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR & 0x7FFF;
        NEXT_LATCHES.INTV = TIMER_INTV;
        NEXT_LATCHES.REGS[6] = CURRENT_LATCHES.SSP;
        push(NEXT_LATCHES.REGS[6], CURRENT_LATCHES.PSR);
        push(NEXT_LATCHES.REGS[6], CURRENT_LATCHES.PC);
        NEXT_LATCHES.SSP = NEXT_LATCHES.REGS[6];
    }
    
    if(GetIRD(CURRENT_LATCHES.MICROINSTRUCTION)){
        memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[(CURRENT_LATCHES.IR & 0xF000) >> 12], sizeof(int)*CONTROL_STORE_BITS); NEXT_LATCHES.STATE_NUMBER = ((CURRENT_LATCHES.IR & 0xF000) >> 12); return;}
    
    
    int Cond0 = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) & 0x0001;
    int Cond1 = (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) & 0x0002) >> 1;
    
    int IR11 = (CURRENT_LATCHES.IR & 0x0800) >> 11;
    
    int gate1 = (Cond0 & Cond1 & IR11);
    int gate2 = ((~Cond1) & (Cond0) & CURRENT_LATCHES.READY);
    int gate3 = (Cond1 & (~Cond0) & CURRENT_LATCHES.BEN);
    
    int j = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    int j0 = j & 0x0001;
    int j1 = (j & 0x0002) >> 1;
    int j2 = (j & 0x0004) >> 2;
    int j3 = j & 0x0008;
    int j4 = j & 0x0010;
    int j5 = j & 0x0020;
    
    int gate11 = j0 | gate1;
    int gate22 = (j1 | gate2) << 1;
    int gate33 = (j2 | gate3) << 2;
    
    
    int nextAddr = gate11 + gate22 + gate33 + j3 + j4 + j5;
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextAddr], sizeof(int)*CONTROL_STORE_BITS);
    NEXT_LATCHES.STATE_NUMBER = nextAddr;
    return;
    
    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */
    
    
    
    
}


void cycle_memory() {
    
    
    if(CURRENT_LATCHES.READY){
        int we0 = ~(CURRENT_LATCHES.MAR & 0x0001) & GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
        int we1 = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) & ((CURRENT_LATCHES.MAR & 0x0001) ^ GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION));
        
        if(we0 && we1){
            MEMORY[CURRENT_LATCHES.MAR/2][0] = CURRENT_LATCHES.MDR & 0x00FF;
            MEMORY[CURRENT_LATCHES.MAR/2][1] = (CURRENT_LATCHES.MDR & 0xFF00) >> 8;
        }
        
        else if(we0)
            MEMORY[CURRENT_LATCHES.MAR/2][0] = CURRENT_LATCHES.MDR & 0x00FF;
        else if(we1)
            MEMORY[CURRENT_LATCHES.MAR/2][1] = (CURRENT_LATCHES.MDR & 0x00FF);
    }
    
    
    
    
    if(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) == 1 && memoryAccessToggle == 0)
    {initialCycle = CYCLE_COUNT; memoryAccessToggle = 1; NEXT_LATCHES.READY = 0;}
    
    if(((CYCLE_COUNT - initialCycle) == 3 )&& (memoryAccessToggle == 1))
        NEXT_LATCHES.READY = 1;
    
    if(((CYCLE_COUNT - initialCycle) == 4) && (memoryAccessToggle == 1)){
        memoryAccessToggle = 0; NEXT_LATCHES.READY = 0;
    }
    
    
    /*
     * This function emulates memory and the WE logic.
     * Keep track of which cycle of MEMEN we are dealing with.
     * If fourth, we need to latch Ready bit at the end of
     * cycle to prepare microsequencer for the fifth cycle.
     */
    
}

void eval_bus_drivers() {
    
    if(GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION))
        SR = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
    else
        SR = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    
    if(GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION))
        DR = 0x0007;
    else
        DR = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    
    
    if(CURRENT_LATCHES.IR & 0x0020)
        SR2value = SEXT((CURRENT_LATCHES.IR & 0x001F), 11, (CURRENT_LATCHES.IR & 0x0010) >> 4);
    else
        SR2value = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x0007)];
    
    if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION))
        addr1 = CURRENT_LATCHES.REGS[SR];
    else
        addr1 = CURRENT_LATCHES.PC;
    
    /* 0 == ZERO, 1 == offset6[5:0], 2 == PCoffset9[8:0], 3 == PCoffset11[10:0]*/
    switch (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
        case 0:
            addr2 = 0;
            break;
        case 1:
            addr2 = SEXT((CURRENT_LATCHES.IR & 0x003F), 10, ((CURRENT_LATCHES.IR & 0x0020) >> 5));
            break;
        case 2:
            addr2 = SEXT((CURRENT_LATCHES.IR & 0x01FF), 7, ((CURRENT_LATCHES.IR & 0x0100) >> 8));
            break;
        case 3:
            addr2 = SEXT((CURRENT_LATCHES.IR & 0x07FF), 5, ((CURRENT_LATCHES.IR & 0x0400) >> 10));
            break;
        default:
            
            break;
    }
    addr2 = addr2 << GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
    
    /**************************************************************/
    if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION))
        marmux = addr1 + addr2;
    else
        marmux = (CURRENT_LATCHES.IR & 0x00FF) << 1;
    
    pc = CURRENT_LATCHES.PC;
    
    /* 0 == ADD, 1 == AND, 2 == XOR, 3 == PASSA*/
    switch (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)) {
        case 0:
            alu = Low16bits((twosComp(SR2value + CURRENT_LATCHES.REGS[SR])));
            break;
        case 1:
            alu = SR2value & CURRENT_LATCHES.REGS[SR];
            break;
        case 2:
            alu = Low16bits(SR2value ^ CURRENT_LATCHES.REGS[SR]);
            break;
        case 3:
            alu = CURRENT_LATCHES.REGS[SR];
            break;
        default:
            
            break;
    }
    
    switch ((CURRENT_LATCHES.IR & 0x0030) >> 4) {
        case 0:
            shf = (CURRENT_LATCHES.REGS[SR]) << (CURRENT_LATCHES.IR & 0x000F);
            break;
        case 1:
            shf = (CURRENT_LATCHES.REGS[SR]) >> (CURRENT_LATCHES.IR & 0x000F);
            break;
        case 3:
            shf = rshf((CURRENT_LATCHES.REGS[SR]), (CURRENT_LATCHES.IR & 0x000F), ((CURRENT_LATCHES.REGS[SR]) & 0x8000));
            break;
            
        default:
            
            break;
    }
    
    
    if(GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION))
        mdr = CURRENT_LATCHES.MDR;
    else
        if(CURRENT_LATCHES.MAR & 0x0001)
            mdr = SEXT(((CURRENT_LATCHES.MDR & 0xFF00) >> 8), 8, (CURRENT_LATCHES.MDR & 0x8000) >> 15);
        else
            mdr = SEXT((CURRENT_LATCHES.MDR & 0x00FF), 8,  (CURRENT_LATCHES.MDR & 0x0080) >> 7);
    
    
    /**************************************************************/
    
    /*
     * Datapath routine emulating operations before driving the bus.
     * Evaluate the input of tristate drivers
     *         Gate_MARMUX,
     *		 Gate_PC,
     *		 Gate_ALU,
     *		 Gate_SHF,
     *		 Gate_MDR.
     */
    
}


void drive_bus() {
    
    
    
    if(GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION))
        BUS = marmux;
    else if(GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION))
        BUS = pc;
    else if(GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION))
        BUS = alu;
    else if(GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION))
        BUS = shf;
    else if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION))
        BUS = mdr;
    else
        BUS = 0;
    
    /*
     * Datapath routine for driving the bus from one of the 5 possible
     * tristate drivers.
     */
    
}


void latch_datapath_values() {
    
    if(GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) ){
        if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) && CURRENT_LATCHES.READY)
            NEXT_LATCHES.MDR = (MEMORY[CURRENT_LATCHES.MAR/2][1] << 8) + MEMORY[CURRENT_LATCHES.MAR/2][0];
        else{
            if(GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION))
                NEXT_LATCHES.MDR = BUS;
            else
                if(CURRENT_LATCHES.MAR & 0x0001)
                    NEXT_LATCHES.MDR = BUS;
                else
                    NEXT_LATCHES.MDR = BUS;
        }
    }
    else
        NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR;
    
    if(GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION))
        NEXT_LATCHES.MAR = Low16bits(BUS);
    else
        NEXT_LATCHES.MAR = CURRENT_LATCHES.MAR;
    
    
    if(GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION))
        NEXT_LATCHES.IR = Low16bits(BUS);
    else
        NEXT_LATCHES.IR = CURRENT_LATCHES.IR;
    
    if(GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION)){
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.P = 0;
        NEXT_LATCHES.Z = 0;
        if(BUS == 0)
            NEXT_LATCHES.Z = 1;
        else if(BUS < 0x7FFF)
            NEXT_LATCHES.P = 1;
        else
            NEXT_LATCHES.N = 1;
        
    }
    else{
        NEXT_LATCHES.N = CURRENT_LATCHES.N;
        NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
        NEXT_LATCHES.P = CURRENT_LATCHES.P;
    }
    
    if(GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION))
        NEXT_LATCHES.BEN = (((CURRENT_LATCHES.IR & 0x0800) >> 11) & CURRENT_LATCHES.N) || (((CURRENT_LATCHES.IR & 0x0400) >> 10) & CURRENT_LATCHES.Z) || (((CURRENT_LATCHES.IR & 0x0200) >> 9) & CURRENT_LATCHES.P);
    else
        NEXT_LATCHES.BEN = CURRENT_LATCHES.BEN;
    
    if(GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION)){
        switch (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
            case 0:
                NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
                break;
            case 1:
                NEXT_LATCHES.PC = Low16bits(BUS);
                break;
            case 2:
                NEXT_LATCHES.PC = Low16bits(addr1 + addr2);
                break;
            default:
                
                break;
        }
    }
    else
        NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    
    
    memcpy(NEXT_LATCHES.REGS, CURRENT_LATCHES.REGS, sizeof(int)*8);
    
    if(GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION))
        NEXT_LATCHES.REGS[DR] = Low16bits(BUS);
    
    
    
    /*
     * Datapath routine for computing all functions that need to latch
     * values in the data path at the end of this cycle.  Some values
     * require sourcing the bus; therefore, this routine has to come 
     * after drive_bus.
     */       
    
}
