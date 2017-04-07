/* 
 Name 1: Matthew Tawil
 UTEID 1: mt33924
 */

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
 MEMORY[A][1] stores the most significant byte of word at word address A
 */

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{
    
    int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
    int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3b ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
    
    process_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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
    
    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");
    
    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");
    
    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;
    
    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
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
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }
    printf("LC-3b Simulator\n\n");
    
    initialize(argv[1], argc - 1);
    
    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }
    
    while (1)
        get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
 You are allowed to use the following global variables in your
 code. These are defined above.
 
 MEMORY
 
 CURRENT_LATCHES
 NEXT_LATCHES
 
 You may define your own local/global variables and functions.
 You may use the functions to get at the control bits defined
 above.
 
 Begin your code here 	  			       */

/***************************************************************/



#define DRreg(x) (((x) & 0x0E00) >> 9)
#define SR1reg(x) (((x) & 0x01C0) >> 6)
#define SR2reg(x) ((x) & 0x0007)
#define amt4(x) ((x) & 0x000F)
#define trap8(x) ((x) & 0x00FF)

int getInstruction(){
    return (MEMORY[CURRENT_LATCHES.PC/2][1] << 8) + (MEMORY[CURRENT_LATCHES.PC/2][0]);
    
}

int getOpCode(){
    
    return (MEMORY[CURRENT_LATCHES.PC/2][1] & 0xF0) >> 4;
    
}

int IMM5(int x){
    x = x & 0x001F;
    if(x & 0x0010){
        return (x-32);
    }
    return x;
}

int pcOff9(int x){
    x = x & 0x01FF;
    if(x & 0x0100){
        return (x-512);
    }
    return x;
}

int pcOff11(int x){
    x = x & 0x07FF;
    if(x & 0x0400){
        return (x-2048);
    }
    return x;
}

int bOff6(int x){
    x = x & 0x003F;
    if(x & 0x0020){
        return (x-64);
    }
    return x;
}
int twoComp(int x){
    if((x&0XF000) == 0XF000)
    {x = x^0xFFFF; x++;}
        return x;
}


void setCC(int x){
    if(x > 0){CURRENT_LATCHES.N = 0; CURRENT_LATCHES.Z = 0; CURRENT_LATCHES.P = 1;}
    if(x == 0){CURRENT_LATCHES.N = 0; CURRENT_LATCHES.Z = 1; CURRENT_LATCHES.P = 0;}
    if(x > 0x8000){CURRENT_LATCHES.N = 1; CURRENT_LATCHES.Z = 0; CURRENT_LATCHES.P = 0;}
}

int branch(int instruction){
    
    int n = (instruction&0x0800) >> 11;
    int z = (instruction&0x0400) >> 10;
    int p = (instruction&0x0200) >> 9;
    if((p&CURRENT_LATCHES.P) || (n&CURRENT_LATCHES.N) || (z&CURRENT_LATCHES.Z))
        return TRUE;
    else
        return FALSE;
    
}

void process_instruction(){
    
    int currentInstruction = getInstruction();
    int opCode = getOpCode();
    CURRENT_LATCHES.PC+=2;
    NEXT_LATCHES = CURRENT_LATCHES;
    
    switch (opCode) {
        case 0x0:
            /*printf("BR");*/
            if (branch(currentInstruction))
                CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + (pcOff9(currentInstruction) << 1));
            break;
        case 0x1:
            /*printf("ADD");*/
            if ((currentInstruction&0x20) == 0){
                CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + CURRENT_LATCHES.REGS[SR2reg(currentInstruction)]);}
            else{
                CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + IMM5(currentInstruction));}
            setCC(CURRENT_LATCHES.REGS[DRreg(currentInstruction)]);
            break;
        case 0x2:
            /*printf("LDB");*/
            CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = MEMORY[(Low16bits((CURRENT_LATCHES.REGS[SR1reg(currentInstruction)]) + bOff6(currentInstruction)))/2][(Low16bits((CURRENT_LATCHES.REGS[SR1reg(currentInstruction)]) + bOff6(currentInstruction))) % 2] & 0x00FF;
            setCC(CURRENT_LATCHES.REGS[DRreg(currentInstruction)]);
            break;
        case 0x3:
            /*printf("STB");*/
            MEMORY[(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + bOff6(currentInstruction))/2][(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + bOff6(currentInstruction))%2] = CURRENT_LATCHES.REGS[DRreg(currentInstruction)] & 0x00FF;

            break;
        case 0x4:{
            /*printf("JSR");*/
            int temp = CURRENT_LATCHES.PC;
            if ((currentInstruction&0x0800) == 0)
                CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)]);
            else
                CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + (pcOff11(currentInstruction) << 1));
            CURRENT_LATCHES.REGS[7] = temp;
            break;}
        case 0x5:
            /*printf("AND");*/
            if ((currentInstruction&0x20) == 0){CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)]& CURRENT_LATCHES.REGS[SR2reg(currentInstruction)]);}
            else{CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] & IMM5(currentInstruction));}
            setCC(CURRENT_LATCHES.REGS[DRreg(currentInstruction)]);
            break;
        case 0x6:
            /*printf("LDW");*/
            CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(((MEMORY[(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + bOff6(currentInstruction)*2)/2][1] << 8)&0xFF00) + (MEMORY[(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + (bOff6(currentInstruction)*2))/2][0] & 0x00FF));
            setCC(CURRENT_LATCHES.REGS[DRreg(currentInstruction)]);
            break;
        case 0x7:
            /*printf("STW");*/
            MEMORY[(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + (bOff6(currentInstruction)*2))/2][0] = Low16bits(CURRENT_LATCHES.REGS[DRreg(currentInstruction)] & 0x00FF);
            MEMORY[(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] + (bOff6(currentInstruction)*2))/2][1] = Low16bits((CURRENT_LATCHES.REGS[DRreg(currentInstruction)] & 0xFF00) >> 8);
            break;
        case 0x8:
            /*printf("RTI");*/
            break;
        case 0x9:
            /*printf("XOR");*/
            if ((currentInstruction&0x20) == 0){CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)]^CURRENT_LATCHES.REGS[SR2reg(currentInstruction)]);}
            else{CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] ^ IMM5(currentInstruction));}
            setCC(CURRENT_LATCHES.REGS[DRreg(currentInstruction)]);
            break;
        case 0xA:
            /*printf("UNUSED");*/
            break;
        case 0xB:
            /*printf("UNUSED");*/
            break;
        case 0xC:
            /*printf("JMP");*/
            CURRENT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)]);
            break;
        case 0xD:
            /*printf("SHF");*/
            if ((currentInstruction&0x10) == 0){CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] << amt4(currentInstruction));}
            else if ((currentInstruction&0x20) == 0){Low16bits(CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] >> amt4(currentInstruction));}
            else {
                if((CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] & 0X8000) == 0X8000){
                    int temp = CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] - 65536;
                    temp = (temp >> amt4(currentInstruction));
                    CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = (Low16bits(temp));
                }
                else
                    CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.REGS[SR1reg(currentInstruction)] >> amt4(currentInstruction));
            }
            setCC(CURRENT_LATCHES.REGS[DRreg(currentInstruction)]);
            break;
        case 0xE:
            /*printf("LEA");*/
            CURRENT_LATCHES.REGS[DRreg(currentInstruction)] = Low16bits(CURRENT_LATCHES.PC + (pcOff9(currentInstruction)*2));
            break;
        case 0xF:
            /*printf("TRAP");*/
            CURRENT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC);
            CURRENT_LATCHES.PC = Low16bits(MEMORY[trap8(currentInstruction)*2][0]);
            break;
            
        default:
            /*printf("NO OP");*/
            break;
            
            
            
    }
    
    
    NEXT_LATCHES = CURRENT_LATCHES;
    
    
    
}
