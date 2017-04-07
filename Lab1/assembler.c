/*
	Name 1: Matthew Tawil
	UTEID 1: MT33924
 */

#include <stdio.h>
#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255

typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];	/* Question for the reader: Why do we need to add 1? */
} TableEntry;

TableEntry symbolTable[MAX_SYMBOLS];
int amtOfSymbols = 0;
int startAddress = 0;


enum
{
	   DONE, OK, EMPTY_LINE
};

FILE *inFile = NULL;
FILE *outFile = NULL;


int isEven(int x){
    if(x%2)
        return 0;
    return 1;
    
}
int checkOpOrderA(char *lArg1, char *lArg2, char *lArg3){
    
    if((lArg1[0] == 'r') && (lArg2[0] == 'r') && (lArg3[0] == 'r'))
        return 1;
    else if((lArg1[0] == 'r') && (lArg2[0] == 'r') && (lArg3[0] == '#'))
        return 1;
    else if((lArg1[0] == 'r') && (lArg2[0] == 'r') && (lArg3[0] == 'x'))
        return 1;
    
    printf("OpOrderA error");
    exit(4);
    
}

/* type 0 = imm5 && type 1 = amount4 && type2 = offset6 && type3 = trapvector */
void checkIMM(int imm, int type){
    if(type == 0) if(!(imm <= 15) || !(imm >= -16)){printf("checkIMM error");
        exit(3);}
    if(type == 1) if(!(imm <= 15) || !(imm >= 0)){printf("checkIMM error");
        exit(3);}
    if(type == 2) if(!(imm <= 31) || !(imm >= -32)){printf("checkIMM error");
        exit(3);}
    if(type == 3) if(!(imm <= 255) || !(imm >= 0)){printf("checkIMM error");
        exit(3);}
}

int pCOffset9(int lineNumber, char *symbol){
    /*needs to be between -256 and 255*/
    int offset = 0;
    int i = 0;
    for(i = 0; i < amtOfSymbols; i++){
        if(strcmp(symbolTable[i].label,symbol) == 0){
           offset = symbolTable[i].address - lineNumber;
            if(offset < 256 && offset >= -256)
                return offset;
            else{
                printf("offset is too large ");
                exit(4);
            }
        }
        
    }
    /*If symbol does not match*/
    printf("Undefined Label");
    exit(1);
    
}

int pCOffset11(int lineNumber, char *symbol){
    /*needs to be between -256 and 255*/
    int offset = 0;
    int i = 0;
    for(i = 0; i < amtOfSymbols; i++){
        if(strcmp(symbolTable[i].label,symbol) == 0){
            offset =  symbolTable[i].address - lineNumber;
            if(offset < 1024 && offset >= -1024)
                return offset;
            else{
                printf("offset is too large");
                exit(4);
            }
        }
        
    }
    /*If symbol does not exist*/
    printf("Undefined Label");
    exit(1);
    
}

int isOpcode(char *lptr){
    
    /* ADD, AND, BR(all 8 variations), HALT, JMP, JSR, JSRR, LDB, LDW, LEA, NOP, NOT, RET, LSHF, RSHFL, RSHFA, RTI, STB, STW, TRAP, XOR*/
    
    if(((strcmp(lptr, "add") == 0) || (strcmp(lptr, "and") == 0) || (strcmp(lptr, "br") == 0) || (strcmp(lptr, "brz") == 0) || (strcmp(lptr, "brp") == 0) || (strcmp(lptr, "brn") == 0) || (strcmp(lptr, "brzp") == 0) || (strcmp(lptr, "brzn") == 0) || (strcmp(lptr, "brpn") == 0) || (strcmp(lptr, "brnzp") == 0) || (strcmp(lptr, "halt") == 0) || (strcmp(lptr, "jmp") == 0) || (strcmp(lptr, "jsr") == 0) || (strcmp(lptr, "jsrr") == 0) || (strcmp(lptr, "ldb") == 0) || (strcmp(lptr, "ldw") == 0) || (strcmp(lptr, "lea") == 0) || (strcmp(lptr, "nop") == 0) || (strcmp(lptr, "not") == 0) || (strcmp(lptr, "ret") == 0) || (strcmp(lptr, "lshf") == 0) || (strcmp(lptr, "rshfl") == 0) || (strcmp(lptr, "rshfa") == 0) || (strcmp(lptr, "rti") == 0) || (strcmp(lptr, "stb") == 0) || (strcmp(lptr, "stw") == 0) || (strcmp(lptr, "trap") == 0) || (strcmp(lptr, "xor")) == 0))
        return 1;

    
    return -1;
}


int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4) {
    
    char * lRet, *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))		/*gets one line of the file, if it is empty the method exits with code done*/
        return (DONE);
    
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);		/*changes upper case to lowercase*/
    
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine
    + strlen(pLine);
    
    /* ignore the comments */
    lPtr = pLine;
    
    while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n')
        lPtr++;
    
    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);
    
    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok( NULL, "\t\n ,")))
            return (OK);
    }
    
    *pOpcode = lPtr;
    
    if (!(lPtr = strtok( NULL, "\t\n ,")))
        return (OK);
    
    *pArg1 = lPtr;
    
    if (!(lPtr = strtok( NULL, "\t\n ,")))
        return (OK);
    
    *pArg2 = lPtr;
    if (!(lPtr = strtok( NULL, "\t\n ,")))
        return (OK);
    
    *pArg3 = lPtr;
    
    if (!(lPtr = strtok( NULL, "\t\n ,")))
        return (OK);
    
    *pArg4 = lPtr;
    
    return (OK);
}


int toNum(char * pStr) {
    char * t_ptr;
    char * orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;
    
    orig_pStr = pStr;
    if (*pStr == '#') /* decimal */
    {
        pStr++;
        if (*pStr == '-') /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isdigit(*t_ptr)) {
                printf("Error: invalid decimal operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;
        
        return lNum;
    }
    
    else if (*pStr == 'x') /* hex     */
    {
        pStr++;
        if (*pStr == '-') /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isxdigit(*t_ptr)) {
                printf("Error: invalid hex operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16); /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if (lNeg)
            lNum = -lNum;
        return lNum;
    }
    
    else {
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4); /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

int main(int argc, char *argv[]) {
    
    
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
    *lArg2, *lArg3, *lArg4;
    

    int lineNumber = -1;
    int value = 0;
    int lRet = 5;
    int DR = 0, SR1 = 0, SR2 = 0;
    
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    
   /* inFile = fopen("/Users/matttawil/Desktop/College/Junior Year/Spring 2017/EE 460N Comp Architecture/Lab1/Lab1/Test.asm", "r");
    outFile = fopen("/Users/matttawil/Desktop/College/Junior Year/Spring 2017/EE 460N Comp Architecture/Lab1/Lab1/Export.txt", "w");*/
    
    inFile = fopen(argv[1], "r");
    outFile = fopen(argv[2], "w");
    

    
    if (!inFile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outFile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }
    
    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];
    
    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);
    
    while( lRet != DONE){
        int i = 0;
        lRet = readAndParse( inFile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE && strcmp(lLabel, "in") != 0 && strcmp(lLabel, "out") != 0 && strcmp(lLabel, "getc") != 0 && strcmp(lLabel, "puts") != 0 && lLabel[0] != 'x' && strcmp(lLabel, "") != 0)
        {
            for(i = 0; (i < 256 && lLabel[i] != '\0'); i++)
                if(isalnum(lLabel[i]) == 0){printf("1"); exit(4);}
            for(i = 0; i < amtOfSymbols; i++)
                if (strcmp(lLabel, symbolTable[i].label) == 0){printf("2"); exit(4);}
            
            strcpy(symbolTable[amtOfSymbols].label, lLabel);
            symbolTable[amtOfSymbols].address = lineNumber;
            amtOfSymbols++;
            
        }
        if(lRet == OK)
            lineNumber++;
        
    }
    printf("passed symbols");
    lineNumber = -1;
    rewind(inFile);
    lRet = 5;
    
    while( lRet != DONE){
        int opValue = 0;
        lRet = readAndParse( inFile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            lineNumber ++;
            
            
            if(strcmp(lOpcode, ".orig") == 0){
                value = toNum(lArg1);
                if(value % 2 != 0){printf("3");
                    exit(3);}
                startAddress = value;
                fprintf( outFile, "0x%.4X\n", value );
                
            }
            else if(strcmp(lOpcode, ".fill") == 0){
                value = toNum(lArg1);
                if((value > 0x7FFF) || (value <= -0x8000))
                    exit(3);
                value = value&0xFFFF;
                fprintf( outFile, "0x%.4X\n", value );
                
            }
            else if(strcmp(lOpcode, ".end") == 0){
                lRet = DONE;
            }
             /*beggining of opcodes*/
            else if(strcmp(lOpcode, "add") == 0){
                opValue = 0x1000;
                checkOpOrderA(lArg1, lArg2, lArg3);
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("4");
                    exit(4);}
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("5");
                    exit(4);}
                DR = DR << 9;
                SR1 = SR1 << 6;
                if(lArg3[0] == 'r'){
                    SR2 = atoi(++lArg3);
                    if((SR2 < 0) || (SR2 > 8)){printf("6");
                        exit(4);}
                }
                else{
                    SR2 = toNum(lArg3);
                    checkIMM(SR2, 0);
                    SR2 = SR2&(31);
                    SR2 = SR2|(32);
                }
                value =  opValue + DR + SR1 + abs(SR2);
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "and") == 0){
                opValue = 0x5000;
                checkOpOrderA(lArg1, lArg2, lArg3);
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("7");
                    exit(4);}
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("8");
                    exit(4);}
                DR = DR << 9;
                SR1 = SR1 << 6;
                if(lArg3[0] == 'r'){
                    SR2 = atoi(++lArg3);
                    if((SR2 < 0) || (SR2 > 8)){printf("9");
                        exit(4);}
                }
                else
                {
                        SR2 = toNum(lArg3);
                        checkIMM(SR2, 0);
                        SR2 = SR2&(31);
                        SR2 = SR2|(32);
                }
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );
            }
            /*br is assembled as brnzp*/
            else if((strcmp(lOpcode, "brnzp") == 0) || (strcmp(lOpcode, "br") == 0)){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 3584;              /*3584 is setting the nzp bits*/
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "brnz") == 0){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 3072;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "brnp") == 0){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 2560;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "brn") == 0){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 2048;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "brzp") == 0){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 1536;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "brz") == 0){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 1024;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "brp") == 0){
                opValue = 0;
                SR1 = pCOffset9(lineNumber, lArg1);
                SR1 = SR1&511;
                value = opValue + SR1 + 512;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "halt") == 0){
                value = 0xf025;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "jmp") == 0){
                opValue = 0xC000;
                if(lArg1[0] != 'r'){printf("10");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("11");
                    exit(4);}
                DR = DR << 6;
                value = opValue + DR;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "jsr") == 0){
                opValue = 0x4800;
                DR = pCOffset11(lineNumber, lArg1);
                DR = DR&0x7FF;
                value = opValue + DR;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "jsrr") == 0){
                opValue = 0x4000;
                if(lArg1[0] != 'r'){printf("12");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("13");
                    exit(4);}
                DR = DR << 6;
                value = opValue + DR;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "ldb") == 0){
                opValue = 0x2000;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("14");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("15");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("16");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 2);
                SR2 = SR2&(63);
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "ldw") == 0){
                opValue = 0x6000;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("17");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("18");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("19");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 2);
                SR2 = SR2&63;
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "lea") == 0){
                opValue = 0xE000;
                if(lArg1[0] != 'r'){printf("20");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("21");
                    exit(4);}
                DR = DR << 9;
                SR1 = pCOffset9(lineNumber, lArg2);
                SR1 = SR1&0x1FF;
                value = DR + opValue + SR1;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "nop") == 0){
                value = 0;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "not") == 0){
                opValue = 0x903F;
                if(lArg1[0] != 'r'){printf("22");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("23");
                    exit(4);}
                DR = DR << 9;
                if(lArg2[0] != 'r'){printf("24");
                    exit(4);}
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("25");
                    exit(4);}
                SR1 = SR1 << 6;
                value = DR + opValue + SR1;
                fprintf( outFile, "0x%.4X\n", value );            }
            else if(strcmp(lOpcode, "ret") == 0){
                value = 0xC1C0;
                fprintf( outFile, "0x%.4X\n", value );	/* where lInstr is declared as an int */
            }
            else if(strcmp(lOpcode, "lshf") == 0){
                opValue = 0xD000;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("26");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("27");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("28");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 1);
                SR2 = SR2&(15);
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );	/* where lInstr is declared as an int */
            }
            else if(strcmp(lOpcode, "rshfl") == 0){
                opValue = 0xD010;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("29");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("30");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("31");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 1);
                SR2 = SR2&(15);
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );	/* where lInstr is declared as an int */

            }
            else if(strcmp(lOpcode, "rshfa") == 0){
                opValue = 0xD030;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("32");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("33");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("34");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 1);
                SR2 = SR2&(15);
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );	/* where lInstr is declared as an int */
            }
            else if(strcmp(lOpcode, "rti") == 0){
                value = 0x8000;
                fprintf( outFile, "0x%.4X\n", value );	/* where lInstr is declared as an int */
            }
            else if(strcmp(lOpcode, "stb") == 0){
                opValue = 0x3000;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("35");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("36");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("37");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 2);
                SR2 = SR2&(63);
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );

            }
            else if(strcmp(lOpcode, "stw") == 0){
                opValue = 0x7000;
                if((lArg1[0] != 'r') || (lArg2[0] != 'r')){printf("38");
                    exit(4);}
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("39");
                    exit(4);}
                DR = DR << 9;
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("40");
                    exit(4);}
                SR1 = SR1 << 6;
                SR2 = toNum(lArg3);
                checkIMM(SR2, 2);
                SR2 = SR2&(63);
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "trap") == 0){
                opValue = 0xF000;
                DR = toNum(lArg1);
                checkIMM(DR, 3);
                value = opValue + DR;
                fprintf( outFile, "0x%.4X\n", value );
            }
            else if(strcmp(lOpcode, "xor") == 0){
                opValue = 0x9000;
                checkOpOrderA(lArg1, lArg2, lArg3);
                DR = atoi(++lArg1);
                if((DR < 0) || (DR > 8)){printf("41");
                    exit(4);}
                SR1 = atoi(++lArg2);
                if((SR1 < 0) || (SR1 > 8)){printf("42");
                    exit(4);}
                DR = DR << 9;
                SR1 = SR1 << 6;
                if(lArg3[0] == 'r'){
                    SR2 = atoi(++lArg3);
                    if((SR2 < 0) || (SR2 > 8)){printf("43");
                        exit(4);}
                }
                else{
                    SR2 = toNum(lArg3);
                    checkIMM(SR2, 0);
                    SR2 = SR2&(31);
                    SR2 = SR2|(32);
                }
                value = opValue + DR + SR1 + SR2;
                fprintf( outFile, "0x%.4X\n", value );

            }
            else{
                printf("Invalid Op!");
                exit(2);
            }
            
            
            
        }
        
        
    }
    
    
    fclose(inFile);
    fclose(outFile);
    printf("No Errors\n");
    return 1;
    
}

