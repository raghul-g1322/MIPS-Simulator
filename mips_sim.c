#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

/*--------------------------------------------------------------------------*/

/* R - TYPE */
/* OP-Code */
#define SPLOP 0x00
/* FUNCT */
#define SLL 0x00
#define SRL 0x02
#define SRA 0x03
#define SLLV 0x04
#define SRLV 0x06
#define SRAV 0x07
#define JR 0x08
#define JALR 0x09
#define ADD 0x20
#define ADDU 0x21
#define SUB 0x22
#define SUBU 0x23
#define AND 0x024
#define OR 0x25
#define XOR 0x26
#define NOR 0x27
#define SLT 0x2A
#define SLTU 0x2B
#define MULT 0x18
#define MFHI 0x10
#define MFLO 0x12
#define MTHI 0x11
#define MTLO 0x13
#define MULTU 0x19
#define DIV 0x1A
#define DIVU 0x1B
#define SYSCALL 0x0C

/*--------------------------------------------------------------------------*/

/* I - TYPE */
#define BEQ 0x04
#define BNE 0x05
#define BLEZ 0x06
#define BGTZ 0x07
#define ADDI 0x08
#define ADDIU 0x09
#define SLTI 0x0A
#define SLTIU 0x0B
#define ANDI 0x0C
#define ORI 0x0D
#define XORI 0x0E
#define LUI 0x0F
#define LB 0x20
#define LH 0x21
#define LW 0x23
#define LBU 0x24
#define LHU 0x25
#define SB 0x28
#define SH 0x29
#define SW 0x2B
/* OP-Code */
#define REGIMM 0x01
/* RT */
#define BLTZ 0x00
#define BGEZ 0x01
#define BLTZAL 0x10
#define BGEZAL 0x11

/*--------------------------------------------------------------------------*/

/* J - TYPE */
#define J 0x02
#define JAL 0x03

/*--------------------------------------------------------------------------*/

#define NEXT 4

int JUMP_FLAG = 0; // For keeping track of JUMPS

/*--------------------------------------------------------------------------*/

//FUNCTION DECLARATION
void r_type(uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t funct);
void i_type(uint8_t op, uint8_t rs, uint8_t rt, int16_t imm);
void regimm(uint8_t rs, uint8_t rt, int16_t imm);

/*--------------------------------------------------------------------------*/

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */

     uint32_t INST = mem_read_32(CURRENT_STATE.PC);
     printf("PC = %8x\t||\t%08x\n", CURRENT_STATE.PC, INST);
     uint8_t OPCODE = (INST >> 26) & 0x3F;
     uint8_t RS = (INST >> 21) & 0x1F;
     uint8_t RT = (INST >> 16) & 0x1F;
     uint8_t RD = (INST >> 11) & 0x1F;
     uint8_t SHAMT = (INST >> 6) & 0x1F;
     uint8_t FUNCT = (INST) & 0x3F;
     int16_t IMM  = (INST) & 0xFFFF;
     uint32_t J_TARGET = (CURRENT_STATE.PC & 0xF0000000) | ((INST & 0x03FFFFFF) << 2);

     switch(OPCODE){
        case SPLOP :
            r_type(RS, RT, RD, SHAMT, FUNCT);
            break;

        case J :        //J-TYPE                                                        // 1
            NEXT_STATE.PC = J_TARGET;
            JUMP_FLAG = 1;
            break;

        case JAL :      //J-TYPE                                                        //2
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            NEXT_STATE.PC = J_TARGET;
            JUMP_FLAG = 1;
            break;

        default :
            i_type(OPCODE, RS, RT, IMM);
            break;
     }
     if(JUMP_FLAG != 1)
         NEXT_STATE.PC = CURRENT_STATE.PC + NEXT;
     JUMP_FLAG = 0;
}

/*------------------------------------I-TYPE--------------------------------------*/

void i_type(uint8_t op, uint8_t rs, uint8_t rt, int16_t imm){
    uint32_t address = CURRENT_STATE.REGS[rs] + (int32_t)imm;
    uint32_t group_add = (address & 0xFFFFFFFC);
    int32_t value = mem_read_32(group_add);
    
    switch(op){

        case REGIMM : 
            regimm(rs, rt, imm);
            break;

        case BEQ :                                                                      //3
            {
                if(CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }

        case BNE :                                                                      //4
            {
                if(CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }

        case BLEZ :                                                                     //5
            {
                if((int32_t)CURRENT_STATE.REGS[rs] <= 0){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }

        case BGTZ :                                                                     //6
            {
                if((int32_t)CURRENT_STATE.REGS[rs] > 0){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }

        case ADDI :                                                                     //7
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + (int32_t)imm;
            break;

        case ADDIU :                                                                    //8
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + (int32_t)imm;
            break;
        
        case SLTI :                                                                     //9
            {
                if((int32_t)CURRENT_STATE.REGS[rs] < (int32_t)imm)
                    NEXT_STATE.REGS[rt] = 1;
                else
                    NEXT_STATE.REGS[rt] = 0;
                break;
            }

        case SLTIU :                                                                    //10
            {
                if(CURRENT_STATE.REGS[rs] < (uint32_t)(uint16_t)imm)
                    NEXT_STATE.REGS[rt] = 1;
                else
                    NEXT_STATE.REGS[rt] = 0;
                break;
            }

        case ANDI :                                                                     //11
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & (imm & 0xFFFF);
            break;

        case ORI :                                                                      //12
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | (imm & 0xFFFF);
            break;

        case XORI :                                                                     //13
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ (imm & 0xFFFF);
            break;

        case LUI :                                                                      //14
            {
                int32_t temp = (int32_t)imm << 16;
                NEXT_STATE.REGS[rt] = temp;
                break;
            }

        case LW :                                                                       //15
            {
                NEXT_STATE.REGS[rt] = mem_read_32(address);
                break;
            }

        case SW :                                                                       //16
            {
                mem_write_32(address, CURRENT_STATE.REGS[rt]);
                break;
            }

        case LH :                                                                       //17
            {
                int16_t hw;
                if(address & 0x00000002)
                    hw = value >> 16;
                else
                    hw = value & 0xFFFF;
                NEXT_STATE.REGS[rt] = hw;
                break;
            }
            
        case LHU :                                                                      //18
            {
                uint16_t hw;
                if(address & 0x00000002)
                    hw = value >> 16;
                else
                    hw = value & 0xFFFF;
                NEXT_STATE.REGS[rt] = (uint32_t)hw;
                break;
            }

        case SH :                                                                       //19
            {
                if(address & 0x00000002){
                    int32_t temp = (value & 0x0000FFFF) | (CURRENT_STATE.REGS[rt] << 16);
                    mem_write_32(group_add, temp);
                }
                else{
                    int32_t temp = (value & 0xFFFF0000) | (CURRENT_STATE.REGS[rt] & 0x0000FFFF);
                    mem_write_32(group_add, temp);
                }
                break;
            }

        case LB :                                                                       //20
            {
                int8_t byte;
                if((address & 0x00000003) == 0) byte = value & 0xFF;
                else if((address & 0x00000003) == 1) byte = value >> 8;
                else if((address & 0x00000003) == 2) byte = value >> 16;
                else byte = value >> 24;
                NEXT_STATE.REGS[rt] = (int32_t)byte;
                break;
            }
        
        case LBU :                                                                      //21
            {
                uint8_t byte;
                if((address & 0x00000003) == 0) byte = value & 0xFF;
                else if((address & 0x00000003) == 1) byte = value >> 8;
                else if((address & 0x00000003) == 2) byte = value >> 16;
                else byte = value >> 24;
                NEXT_STATE.REGS[rt] = (uint32_t)byte;
                break;
            }

        case SB :                                                                       //22
            { 
                uint8_t byte = CURRENT_STATE.REGS[rt] & 0xFF;

                switch (address & 0x3) {
                    case 0:
                        value = (value & 0xFFFFFF00) | byte;
                        break;
                    case 1:
                        value = (value & 0xFFFF00FF) | (byte << 8);
                        break;
                    case 2:
                        value = (value & 0xFF00FFFF) | (byte << 16);
                        break;
                    case 3:
                        value = (value & 0x00FFFFFF) | (byte << 24);
                        break;
                }

                mem_write_32(group_add, value);
                break;
            }
        default :
                printf("There is NO valid instruction with the OPCODE - %d\n", op);
                exit(1);
    }
}

/*------------------------------------R-TYPE--------------------------------------*/

void r_type(uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t funct){
    switch(funct) {
        case SLL :                                                                      //23
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
            break;

        case SRL :                                                                      //4
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
            break;

        case SRA :                                                                      //25
            NEXT_STATE.REGS[rd] = ((int32_t)CURRENT_STATE.REGS[rt]) >> shamt;
            break;

        case SLLV :                                                                     //26
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << CURRENT_STATE.REGS[rs];
            break;

        case SRLV :                                                                     //27
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> CURRENT_STATE.REGS[rs];
            break;

        case SRAV :                                                                     //28
            NEXT_STATE.REGS[rd] = ((int32_t)CURRENT_STATE.REGS[rt]) >> CURRENT_STATE.REGS[rs];
            break;

        case JR :                                                                       //29
            {
                NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
                JUMP_FLAG = 1;
                break;
            }

        case JALR :                                                                     //30
            {
                if(rd == 0)
                    rd = 31;
                NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
                NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
                JUMP_FLAG = 1;
                break;
            }
        case ADD :                                                                      //31
            NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs]) + (CURRENT_STATE.REGS[rt]);
            break;

        case ADDU :                                                                     //32
            NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs]) + (CURRENT_STATE.REGS[rt]);
            break;

        case SUB :                                                                      //33
            NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs]) - (CURRENT_STATE.REGS[rt]);
            break;

        case SUBU :                                                                     //34
            NEXT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs]) - (CURRENT_STATE.REGS[rt]);
            break;

        case AND :                                                                      //35
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
            break;

        case OR :                                                                       //36
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
            break;

        case XOR :                                                                      //37
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
            break;

        case NOR :                                                                      //38
            NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
            break;

        case SLT :                                                                      //39
            {
                if((int32_t)CURRENT_STATE.REGS[rs] < (int32_t)CURRENT_STATE.REGS[rt])
                    NEXT_STATE.REGS[rd] = 0x01;
                else
                    NEXT_STATE.REGS[rd] = 0x00;
                break;
            }

        case SLTU :                                                                     //40
            {
                if(CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt])
                    NEXT_STATE.REGS[rd] = 0x01;
                else
                    NEXT_STATE.REGS[rd] = 0x00;
                break;
            }
        

        case MULT :                                                                     //41
            {
                int64_t temp = ((int64_t)(int32_t)CURRENT_STATE.REGS[rs]) * ((int64_t)(int32_t)CURRENT_STATE.REGS[rt]);
                NEXT_STATE.LO = (temp & 0xFFFFFFFF);
                NEXT_STATE.HI = ((temp >> 32) & 0xFFFFFFFF);
                break;  
            }
        

        case MFHI :                                                                     //42
            NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
            break;

        case MFLO :                                                                     //43
            NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
            break;

        case MTHI :                                                                     //44
            NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
            break;

        case MTLO :                                                                     //45
            NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
            break;

        case MULTU :                                                                    //46
            {
                uint64_t temp = ((uint64_t)CURRENT_STATE.REGS[rs]) * ((uint64_t)CURRENT_STATE.REGS[rt]);
                NEXT_STATE.LO = (temp & 0xFFFFFFFF);
                NEXT_STATE.HI = ((temp >> 32) & 0xFFFFFFFF);
                break;   
            }          

        case DIV :                                                                      //47
            {
                if((int32_t)CURRENT_STATE.REGS[rt] == 0){
                    printf("Division By ZERO is not Possible\n");
                    exit(1);
                }
                else{
                    NEXT_STATE.LO = (int32_t)CURRENT_STATE.REGS[rs] / (int32_t)CURRENT_STATE.REGS[rt];
                    NEXT_STATE.HI = (int32_t)CURRENT_STATE.REGS[rs] % (int32_t)CURRENT_STATE.REGS[rt];
                }
                    break;
            }
            
        case DIVU :                                                                     //48
            {
                if(CURRENT_STATE.REGS[rt] == 0){
                    printf("Division by ZERO is not Possible\n");
                    exit(1);
                }else{
                    NEXT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
                    NEXT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
                }
                break;
            }
        

        case SYSCALL :                                                                  //SYSCALL
            {
                if(CURRENT_STATE.REGS[2] == 10){
                    RUN_BIT = 0;
                    return;
                }
                break;
            }

        default : 
            printf("No R - TYPE Instruction is found with [ %d ] as FUNCTION CODE\n",funct );
            exit(1);
            break;
    }
}

/*------------------------------------I-TYPE (REGIMM)--------------------------------------*/

void regimm(uint8_t rs, uint8_t rt, int16_t imm){
    switch(rt){
        case BLTZ :                                                                     //50
            {
                if((int32_t)CURRENT_STATE.REGS[rs] < 0){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }
        case BGEZ :                                                                     //51
            {
                if((int32_t)CURRENT_STATE.REGS[rs] >= 0){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }
        case BLTZAL :                                                                   //52
            {
                NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                if((int32_t)CURRENT_STATE.REGS[rs] < 0){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }
        case BGEZAL :                                                                   //53
            {
                NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                if((int32_t)CURRENT_STATE.REGS[rs] >= 0){
                    NEXT_STATE.PC = CURRENT_STATE.PC + ((int32_t)(imm) << 2);
                    JUMP_FLAG = 1;
                }
                break;
            }

        default :
            printf("There is NO vaild REGIMM instructio found with RT = %d\n",rt);
            exit(1);
    }

}