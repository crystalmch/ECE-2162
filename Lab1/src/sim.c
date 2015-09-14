#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "shell.h"

/*
 * 步骤一：读取memory到register里面，parse获得的mem_data, 根据suplement选择操作
 * 步骤二：填充操作
 * 步骤三：编写跳出程序
 */

typedef struct TYPE_Structure {
	uint32_t op;
	uint32_t rs;
	uint32_t rt;
	uint32_t im;
	uint32_t rd;
	uint32_t shamt;
	uint32_t funct;
	uint32_t targ;
	uint32_t se_im;

} TYPE;

TYPE INS;

void parser(uint32_t instruction) {
	INS.op = (instruction >> 26) & 0x3F;
	INS.rs = (instruction >> 21) & 0x1F;
	INS.rt = (instruction >> 16) & 0x1F;
	INS.im = (instruction >> 0) & 0xFFFF;
	INS.rd = (instruction >> 11) & 0x1F;
    INS.shamt = (instruction >> 6) & 0x1F;
    INS.funct = (instruction >> 0) & 0x3F;
    INS.targ = (instruction & ((1UL << 26) - 1)) << 2;
    INS.se_im = INS.im | ((INS.im & 0x8000) ? 0xFFFF8000 : 0);
}

void process_instruction() {
	/* execute one instruction here. You should use CURRENT_STATE and modify
	 * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
	 * access memory. */

	//variables
	uint32_t result;
	uint32_t op;
	uint32_t rs_value;
	uint32_t rt_value;
	//multiply variable
	uint64_t uint_temp;
	int64_t int_temp;
	//mem variables
	uint32_t mem_addr;
	uint32_t mem_val;
	uint32_t val;
	//branch marker
	int is_branch = FALSE;
	//instruction fetch
	uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

	//instruction decode/register fetch
	parser(instruction);

	//memory address computation

	//execution
	rs_value= CURRENT_STATE.REGS[INS.rs];
	rt_value= CURRENT_STATE.REGS[INS.rt];
	//load & store
	mem_addr = rs_value + INS.se_im;
	mem_val = CURRENT_STATE.REGS[INS.rt];
	val = mem_read_32(mem_addr & ~3);

	switch (INS.op) {
	case 0:
		switch (INS.funct){
			case 0: 
				NEXT_STATE.REGS[INS.rd] = rt_value << INS.shamt;
				printf("sll: ");
			 	break;
			case 2:
				NEXT_STATE.REGS[INS.rd] = rt_value >> INS.shamt;
				printf("srl: ");
				break;
			case 3:
				NEXT_STATE.REGS[INS.rd] = (int32_t)rt_value >> INS.shamt;
				printf("sra: ");
				break;
			case 4:
				NEXT_STATE.REGS[INS.rd] = rt_value << rs_value;
				printf("sllv: ");
				break;
			case 6:
				NEXT_STATE.REGS[INS.rd] = rt_value >> rs_value;
				printf("srlv: ");
				break;
			case 7:
				NEXT_STATE.REGS[INS.rd] = (int32_t)rt_value >> rs_value;
				printf("srav: ");
				break;
			case 8:
			case 9:
				is_branch = TRUE;
				NEXT_STATE.REGS[INS.rd] = CURRENT_STATE.PC + 4;
				NEXT_STATE.PC = rs_value;
				printf("jr or jalr to %d: ", NEXT_STATE.PC);
				break;
			case 12:
				if(rs_value == 0xA) {
					RUN_BIT = FALSE;
				}
				printf("syscall: ");
				break;
			case 16:
				NEXT_STATE.REGS[INS.rd] = CURRENT_STATE.HI;
				printf("mfhi: ");
				break;
			case 17:
				NEXT_STATE.HI = rs_value;
				printf("mthi: ");
				break;
			case 18:
				NEXT_STATE.REGS[INS.rd] = CURRENT_STATE.LO;
				printf("mflo: ");
				break;
			case 19:
				NEXT_STATE.LO = rs_value;
				printf("mtlo: ");
				break;
			case 24:
				int_temp = (int64_t)((int32_t)rs_value) * (int64_t)((int32_t)rt_value);
				uint_temp = (uint64_t) int_temp;
				NEXT_STATE.HI = (uint_temp >> 32) & 0xFFFFFFFF;
				NEXT_STATE.LO = (uint_temp >> 0) & 0xFFFFFFFF;
				printf("mult: ");
 				break;
			case 25:
				uint_temp = (uint64_t)rs_value * (uint64_t)rt_value;
				NEXT_STATE.HI = (uint_temp >> 32) & 0xFFFFFFFF;
				NEXT_STATE.LO = (uint_temp >> 0) & 0xFFFFFFFF;
				printf("multu: ");
				break;
			case 26:
				if(rt_value !=0){
					int32_t divd = (int32_t) rs_value / (int32_t) rt_value;
					int32_t mod = (int32_t) rs_value % (int32_t) rt_value;

					NEXT_STATE.HI = (uint32_t)divd;
					NEXT_STATE.LO = (uint32_t)mod;
				}else{
					NEXT_STATE.HI = NEXT_STATE.LO = 0;
				}
				printf("div: ");
				break;
			case 27:
				if(rt_value != 0 ){
					NEXT_STATE.HI = rs_value / rt_value;
					NEXT_STATE.LO = rs_value %rt_value;
				}else{
					NEXT_STATE.HI = NEXT_STATE.LO = 0;
				}
				printf("divu: ");
				break;
			case 32:
			case 33:
				NEXT_STATE.REGS[INS.rd] = rs_value + rt_value;
				printf("add or addu: ");
				break;
			case 34:
			case 35:
				NEXT_STATE.REGS[INS.rd] = rs_value - rt_value;
				printf("sub or subu: ");
				break;
			case 36:
				NEXT_STATE.REGS[INS.rd] = rs_value & rt_value;
				printf("and: ");
				break;
			case 37:
				NEXT_STATE.REGS[INS.rd] = rs_value | rt_value;
				printf("or: ");
				break;
			case 38:
				NEXT_STATE.REGS[INS.rd] = rs_value ^ rt_value;
				printf("xor: ");
				break;
			case 39:
				NEXT_STATE.REGS[INS.rd] = ~(rs_value | rt_value);
				printf("nor: ");
				break;
			case 42:
				NEXT_STATE.REGS[INS.rd] = ((int32_t)rs_value < (int32_t)rt_value) ? 1 : 0;
				printf("slt: ");
				break;
			case 43:
				NEXT_STATE.REGS[INS.rd] = (rs_value < rt_value) ? 1 : 0;
				printf("sltu: ");
				break;
		}
		break;

	case 1:
		switch (INS.rt) {
			case 0: 
			case 16: 
				if ((int32_t)rs_value < 0) {
					is_branch = TRUE;
					NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (INS.se_im << 2);
					printf("bltz or bltzal to %d: ", NEXT_STATE.PC);
				}else {
					printf("bltz fall through: ");
				}
				/* link reg */
				if (INS.rt == 16) {
					NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
				}
				break;
			case 1:
			case 17:
				if((int32_t)rs_value >= 0){
					is_branch = TRUE;
					NEXT_STATE.PC = CURRENT_STATE.PC +4 + (INS.se_im << 2);
					printf("bgez or bgezal to %d: ", NEXT_STATE.PC);
				}else{
					printf("bgez fall through: ");
				}
				/* link reg */
				if (INS.rt == 17) {
					NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
				}
				break;
		}
		break;

	case 2:
		is_branch = TRUE;
		NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | INS.targ;
		printf("j to %d: ", NEXT_STATE.PC);
		break;

	case 3:
		is_branch = TRUE;
		NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | INS.targ;
		NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
		printf("jal to %d: ", NEXT_STATE.PC);
		break;

	case 4:
		if (rs_value == rt_value) {
			is_branch = TRUE;
			NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (INS.se_im << 2);
			printf("beq to %d: ", NEXT_STATE.PC);
		}else{
			printf("beq fall through: ");
		}
		break;

	case 5:
		if (rs_value != rt_value) {
			is_branch = TRUE;
			NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (INS.se_im << 2);
			printf("bne to %d: ", NEXT_STATE.PC);
		}else{
			printf("bne fall through: ");
		}
		break;

	case 6:
		if ((int32_t)rs_value <= 0) {
			is_branch = TRUE;
			NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (INS.se_im << 2);
			printf("blez to %d: ", NEXT_STATE.PC);
		}else{
			printf("blez fall through: ");
		}
		break;

	case 7:
		if ((int32_t)rs_value > 0) {
			is_branch = TRUE;
			NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (INS.se_im << 2);
			printf("bgtz to %d: ", NEXT_STATE.PC);
		}else{
			printf("bgtz fall through: ");
		}
		break;

	case 8:
	case 9:
		NEXT_STATE.REGS[INS.rt] = rs_value + INS.se_im;
		printf("addi or addiu: ");
		break;

	case 10:
		NEXT_STATE.REGS[INS.rt] = (int32_t) rs_value < (int32_t) INS.im ? 1 : 0;
		printf("slti: ");
		break;

	case 11:
		NEXT_STATE.REGS[INS.rt] = (uint32_t) rs_value < (uint32_t) INS.im ? 1 : 0;
		printf("sltiu: ");
		break;

	case 12:
		NEXT_STATE.REGS[INS.rt] = rs_value & INS.im;
		printf("andi: ");
		break;

	case 13:
		NEXT_STATE.REGS[INS.rt] = rs_value | INS.im;
		printf("ori: ");
		break;

	case 14:
		NEXT_STATE.REGS[INS.rt] = rs_value ^ INS.im;
		printf("xori: ");
		break;

	case 15:
		NEXT_STATE.REGS[INS.rt] = INS.im << 16;
		printf("lui: ");
		break;

	case 32:
		switch (mem_addr & 3){
			case 0:
				val = val & 0xFF;
				break;
			case 1:
				val = (val >> 8) & 0xFF;
				break;
			case 2:
				val = (val >> 16) & 0xFF;
				break;
			case 3:
				val = (val >> 24) & 0xFF;
				break;
		}
		val |= (val & 0x80) ? 0xFFFFFF80 : 0;
		NEXT_STATE.REGS[INS.rt] = val;
		printf("lb: ");
		break;

	case 33:
		if (mem_addr & 2) {
			val = (val >> 16) & 0xFFFF;
		}else{
			val = val & 0xFFFF;
		}
		val |= (val & 0x8000) ? 0xFFFF8000 : 0;
		NEXT_STATE.REGS[INS.rt] = val;
		printf("lh: ");
		break;

	case 35:
		NEXT_STATE.REGS[INS.rt] = val;
		printf("lw: ");
		break;

	case 36:
		switch (mem_addr & 3){
			case 0:
				val = val & 0xFF;
				break;
			case 1:
				val = (val >> 8) & 0xFF;
				break;
			case 2:
				val = (val >> 16) & 0xFF;
				break;
			case 3:
				val = (val >> 24) & 0xFF;
				break;
		}
		NEXT_STATE.REGS[INS.rt] = val;
		printf("lbu: ");
		break;

	case 37:
		if (mem_addr & 2) {
			val = (val >> 16) & 0xFFFF;
		}else{
			val = val & 0xFFFF;
		}
		NEXT_STATE.REGS[INS.rt] = val;
		printf("lhu: ");
		break;

	case 40:
		switch (mem_addr & 3){
			case 0:
				val = (val & 0xFFFFFF00) | ((mem_val & 0xFF ) << 0);
				break;
			case 1:
				val = (val & 0xFFFF00FF) | ((mem_val & 0xFF ) << 8);
				break;
			case 2:
				val = (val & 0xFF00FFFF) | ((mem_val & 0xFF ) << 16);
				break;
			case 3:
				val = (val & 0x00FFFFFF) | ((mem_val & 0xFF ) << 24);
				break;
		}
		mem_write_32(mem_addr & ~3, val);
		printf("sb: ");
		break;

	case 41:
		if (mem_addr & 2) {
			val = (val & 0x0000FFFF) | (mem_val << 16);
		}else{
			val = (val & 0xFFFF0000) | (mem_val & 0xFFFF);
		}
		mem_write_32(mem_addr & ~3, val);
		printf("sh: ");
		break;

	case 43:
		val =  mem_val;
		mem_write_32(mem_addr % ~3, val);
		printf("sw: ");
		break;

	default:
		printf("Invalid operation\n");
		break;

	}

	if(!is_branch){
		NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	}

	if(mem_read_32(NEXT_STATE.PC) == 0){
		RUN_BIT = FALSE;
	}

	if (INS.op)
	{
		printf("%d, %d, %d, %d\n", INS.rt, INS.rs, NEXT_STATE.REGS[INS.rt], INS.im);
	}else{
		//special
		printf("%d, %d, %d, %d\n", INS.rd, INS.rs, INS.rt, NEXT_STATE.REGS[INS.rd]);
	}

}
