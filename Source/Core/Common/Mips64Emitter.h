#pragma once

#include "Common/CodeBlock.h"
#include "Common/CommonTypes.h"

namespace Mips64Gen {

enum Mips64Reg
{
	R0 = 0,
	R1 = 1,
	R2 = 2, R3 = 3,
	R4 = 4, R5 = 5, R6 = 6, R7 = 7,
	R8 = 8, R9 = 9, R10 = 10, R11 = 11, R12 = 12, R13 = 13, R14 = 14, R15 = 15, R24 = 24, R25 = 25,
	R16 = 16, R17 = 17, R18 = 18, R19 = 19, R20 = 20, R21 = 21, R22 = 22, R23 = 23,
	R26 = 26, R27 = 27,
	R28 = 28,
	R29 = 29,
	R30 = 30,
	R31 = 31,

	// Always Zero
	zero = R0,
	// Reserved for assembler
	at = R1,
	// Stores Results
	v0 = R2, v1 = R3,
	// Stores Arguments
	a0 = R4, a1 = R5, a2 = R6, a3 = R7,
	// Temporary
	t0 = R8, t1 = R9, t2 = R10, t3 = R11, t4 = R12, t5 = R13, t6 = R14, t7 = R15, t8 = R24, t9 = R25,
	// Contents saved for use later
	s0 = R16, s1 = R17, s2 = R18, s3 = R19, s4 = R20, s5 = R21, s6 = R22, s7 = R23,
	// Reserved by operating system
	k0 = R26, k1 = R27,
	// Global pointer
	gp = R28,
	// Stack pointer
	sp = R29,
	// Frame pointer
	fp = R30,
	// Return address
	ra = R31
};

class Mips64Emitter
{
private:
	u8* codePtr;

	void Write32(u32 value)
	{
		*(u32*)codePtr = value;
		codePtr += 4;
	}

	void SpecialInstruction(u8 opcode, u32 data, u8 func);
	void RegisterInstruction(u8 opcode, Mips64Reg rs, Mips64Reg rt, Mips64Reg rd, u8 shift, u8 func);
	void ImmediateInstruction(u8 opcode, Mips64Reg rs, Mips64Reg rt, u16 data);
	void JumpInstruction(u8 opcode, u32 offset);
public:
	Mips64Emitter(void* ptr) { setCodePtr(ptr); }
	Mips64Emitter() { setCodePtr(nullptr); }

	void setCodePtr(void* ptr) { codePtr = (u8*)ptr; }

	// Binary
	void ADD(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt);
	void ADDIU(Mips64Reg rt, Mips64Reg rs, u16 immediate);
	void ADDU(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt);
	void AUI(Mips64Reg rt, Mips64Reg rs, u16 immediate);
	void DADD(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt);
	void DADDIU(Mips64Reg rt, Mips64Reg rs, u16 immediate);
	void DADDU(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt);
	void DAHI(Mips64Reg rs, u16 immediate);
	void DATI(Mips64Reg rs, u16 immediate);
	void DAUI(Mips64Reg rt, Mips64Reg rs, u16 immediate);
	void DSLL(Mips64Reg rd, Mips64Reg rt, u8 sa);
	void DSLL32(Mips64Reg rd, Mips64Reg rt, u8 sa);
	void LUI(Mips64Reg rt, u16 immediate);
	void OR(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt);
	void ORI(Mips64Reg rt, Mips64Reg rs, u16 immediate);
	void SLL(Mips64Reg rd, Mips64Reg rt, u8 sa);
	void SRL(Mips64Reg rd, Mips64Reg rt, u8 sa);
	void XOR(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt);

	// Flow Control
	void B(s32 offset);
	void BC(s32 offset);
	void BEQ(Mips64Reg rs, Mips64Reg rt, s32 offset);
	void JALR(Mips64Reg rd, Mips64Reg rs);
	void JALR(Mips64Reg rs);
	void JR(Mips64Reg rs);

	void BREAK(u32 code);
	void BREAK();
	void NOP();

	// Helpers
	void MOVI2R(Mips64Reg rt, u64 immediate);
};

class Mips64CodeBlock : public CodeBlock<Mips64Emitter>
{
private:
	void PoisonMemory() override
	{
		u32* ptr = (u32*)region;
		u32* ptr_end = (u32*)(region + region_size);
		// Mips64 BREAK = 0x0000000D
		while(ptr < ptr_end)
			*ptr = 0x0000000D;
	}
};

} // namespace Mips64Gen

