//*****************************************************************************
//--ALUSimulator.c
//
//		Author: 		Gary J. Minden
//		Organization:	KU/EECS/EECS 645
//		Date:			2017-04-22 (B70422)
//		Version:		1.0
//		Description:	This is the Test File for ALU Sim
//		Notes:			Modified by Patrick Canny, Andrew Growney, and Liam Ormiston
//						for EECS645 Project #1
//						Due Date: 2018-05-3
//
//*****************************************************************************
//

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <stdio.h>

#include "RegisterFile_01.h"
#include "ALUSimulator.h"

//Definition of HI and LO Register Indicies. This may change in future iterations
#define LO 30
#define HI 31

extern void ALUSimulator( RegisterFile theRegisterFile,
				uint32_t OpCode,
				uint32_t Rs, uint32_t Rt, uint32_t Rd,
				uint32_t ShiftAmt,
				uint32_t FunctionCode,
				uint32_t ImmediateValue,
				uint32_t* Status) {

	/********************************************************************
	 * LOCAL VARIABLES
	 ********************************************************************/

	// updated_value - used to store the calculation to be written to RegisterFile
	uint32_t updated_value = 0;

	// remainder_value - used in cases of overflow
	uint32_t remainder_value = 0;

	// Rs_Value & Rt_Value - These Variables will store the value in the register
	// when this subroutine is called
	uint32_t Rs_Value;
	uint32_t Rt_Value;

	/********************************************************************
	 * PROGRAM BODY
	 ********************************************************************/
	 
    //NOTE: Removed printf statements previously used to validate input values

	//Load in Rs_Value and Rt_Value from the Register file
	RegisterFile_Read(theRegisterFile, Rs, &Rs_Value, Rt, &Rt_Value);

	// We will Need to Look at a variety of different Opcodes in this exercise,
	// but many instructions fall under opcode 000000. These are stored in the
	// FunctionCode argument, so a switch will be defined on this value.
	if (OpCode == 0x00){
		switch(FunctionCode){
			//R-Type Instruction Implementations

			// Use updated_value to store the desired operations result
			// before writing to the register

			// NOOP & SLL - Following is the implementation for SLL operation.
			case 0x0:
			updated_value = (unsigned)Rt_Value << ShiftAmt;
			printf("Writing Value: %d\n",updated_value);
			RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SRL - Following is the implementation for SRL operation, which uses unsigned Rt_Value
			// and ShiftAmt
			case 0x2:
				updated_value = (unsigned)Rt_Value >> ShiftAmt;
				printf("Writing Value: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SRA - Following is the implementation for SRA operation, which uses Rt_Value and ShiftAmt
			case 0x3:
				updated_value = Rt_Value >> ShiftAmt;
				printf("Writing Value: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SLLV - Following is the SLLV which utilizes $t and $s
			case 0x4:
				updated_value = Rt_Value << Rs_Value;
				printf("Writing Value: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			//SRLV - Following is the implementation for SRLV, which is similar to the above
			case 0x6:
				updated_value = Rt_Value >> Rs_Value;
				printf("Writing Value: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			//MFHI - Write Value currently in HI to $d (via MIPS reference)
			case 0x10:
				RegisterFile_Write(theRegisterFile,true,Rd,HI);
				break;

			// MFLO - Write Value from LO to $d
			case 0x12:
				RegisterFile_Write(theRegisterFile,true,Rd,LO);
				break;

			// MULT - Multiply s by t and store in LO
			case 0x18:
				updated_value = (Rs_Value * Rt_Value);
				printf("Updated Value MULT: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,LO, updated_value);
				break;

			// MULTU - Multiply s by t and store in LO
			case 0x19:
				updated_value = (((unsigned)Rs_Value) / ((unsigned)Rt_Value));
				printf("Updated Value MULTU: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,LO,updated_value);
				break;

			// DIV - Implemented via MIPS reference guide
			case 0x1A:
				/* get quotient value
				 * initialize value to get remainder value then get the remainder value
				 * store quotient value in LO register (R30)
				 * store remainder value in HI register (R31)
				 * This is the method I found on the MIPS reference
				 */
				updated_value = (Rs_Value / Rt_Value);
				remainder_value = (Rs_Value % Rt_Value);
				printf("Updated Value DIV: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,LO,updated_value);
				RegisterFile_Write(theRegisterFile,true,HI,remainder_value);
				break;

			// DIVU - Implemented via MIPS reference guide
			case 0x1B:
				/* get quotient value
				 * initialize value to get remainder value then get the remainder value
				 * store quotient value in LO register (R30)
				 * store remainder value in HI register (R31)
				 * This is the method I found on the MIPS reference
				 */
				updated_value = (((unsigned)Rs_Value) / ((unsigned)Rt_Value));
				remainder_value = ((unsigned)Rs_Value % (unsigned)Rt_Value);
				printf("Updated Value DIVU: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,LO,updated_value);
				RegisterFile_Write(theRegisterFile,true,HI,remainder_value);
				break;

			// ADD - Implemention of the ADD operation
			case 0x20:
				updated_value = (Rs_Value + Rt_Value);
				printf("Updated Value ADD: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// ADDU - Implementation of the ADDU operation
			case 0x21:
				updated_value = ((unsigned)Rs_Value + (unsigned)Rt_Value);
				printf("Updated Value ADDU: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SUB - Implementation of the SUB operation
			case 0x22:
				updated_value = Rs_Value - Rt_Value;
				printf("Updated Value SUB: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SUBU - Implementation of SUBU operation
			case 0x23:
				updated_value = (unsigned)Rs_Value - (unsigned)Rt_Value;
				printf("Updated Value SUBU: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// AND - Implementation of AND Operation (bitwise)
			case 0x24:
				updated_value = (Rs_Value & Rt_Value);
				printf("Updated Value AND: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// OR - Implementation of the OR operation (bitwise)
			case 0x25:
				updated_value = (Rs_Value | Rt_Value);
				printf("Updated Value OR: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// XOR - Implemention of the XOR operation
			case 0x26:
				updated_value = (Rs_Value ^ Rt_Value);
				printf("Updated Value XOR: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SLT - Implemention of the SLT operation
			case 0x2A:
				updated_value = (Rs_Value < Rt_Value);
				printf("Updated Value SLT: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// SLTU - Implemention of the SLTU operation
			case 0x2B:
				updated_value = ((unsigned)Rs_Value < (unsigned)Rt_Value);
				printf("Updated Value SLTU: %d\n",updated_value);
				RegisterFile_Write(theRegisterFile,true,Rd,updated_value);
				break;

			// Default case is used as a failsafe in case the code is invalid
			// for some reason
			default:
				printf("DEFAULTED ON FUNCTION CODE\n");
				break;
		}
	}
	/********************************************************************
	 * If the OpCode is not 000000, the instruction must be I-Type
	 ********************************************************************/
	else {
		// I-Type Instruction Implementations

		// Use updated_value to store the desired operations
		// result before writing to the register
		uint32_t updated_value = 0;

		switch(OpCode){
			// ADD - Add Rs_Value with the given immediate value
			case 0x08:
				printf("Add immediate function\n");
				updated_value = Rs_Value + ImmediateValue;
				printf("Writing Value: %d\n",updated_value);

				RegisterFile_Write(theRegisterFile,true,Rt,updated_value);

				break;
			// ADDU - Add unsigned Rs_Value with the given immediate value
			case 0x09:
				printf("Add immediate unsigned function\n");
				updated_value = (unsigned)Rs_Value + ImmediateValue;
				printf("Writing Value: %d\n",updated_value);

				RegisterFile_Write(theRegisterFile,true,Rt,updated_value);
				break;

			// SLTI - Left shift Rs_Value by the given immediate value
			case 10:
				updated_value = (Rs_Value < ImmediateValue);
				printf("Writing Value: %d\n",updated_value);

				RegisterFile_Write(theRegisterFile,true,Rt,updated_value);
				break;

			// SLTIU - Left shift unsigned Rs_Value by the given immediate value
			case 11:
				updated_value = ((unsigned)Rs_Value < (unsigned)ImmediateValue);
				printf("Writing Value: %d\n",updated_value);

				RegisterFile_Write(theRegisterFile,true,Rt,updated_value);
				break;

			// Default case is used as a failsafe in case the code is invalid
			// for some reason
			default:
				printf("DEFAULTED ON OPCODE: %d\n",OpCode);
				break;
		}
	}
};
