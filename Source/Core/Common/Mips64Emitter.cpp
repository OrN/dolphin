#include "Common/Mips64Emitter.h"

namespace Mips64Gen {

void Mips64Emitter::SpecialInstruction(u8 opcode, u32 data, u8 func)
{
	Write32((opcode << 26) | (data << 6) | func);
}

void Mips64Emitter::RegisterInstruction(u8 opcode, Mips64Reg rs, Mips64Reg rt, Mips64Reg rd, u8 shift, u8 func)
{
	SpecialInstruction(opcode, (rs << 15) | (rt << 10) | (rd << 5) | shift, func);
}

void Mips64Emitter::ImmediateInstruction(u8 opcode, Mips64Reg rs, Mips64Reg rt, u16 data)
{
	Write32((opcode << 26) | (rs << 21) | (rt << 16) | data);
}

void Mips64Emitter::JumpInstruction(u8 opcode, u32 offset)
{
	Write32((opcode << 26) | (offset & 0x3FFFFFF));
}

void Mips64Emitter::ADD(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt)
{
	RegisterInstruction(0b000000, rs, rt, rd, 0, 0b100000);
}

void Mips64Emitter::ADDIU(Mips64Reg rt, Mips64Reg rs, u16 immediate)
{
	ImmediateInstruction(0b001001, rs, rt, immediate);
}

void Mips64Emitter::ADDU(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt)
{
	RegisterInstruction(0b000000, rs, rt, rd, 0, 0b100001);
}

void Mips64Emitter::AUI(Mips64Reg rt, Mips64Reg rs, u16 immediate)
{
	ImmediateInstruction(0b001111, rs, rt, immediate);
}

void Mips64Emitter::DADD(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt)
{
	RegisterInstruction(0b000000, rs, rt, rd, 0, 0b101100);
}

void Mips64Emitter::DADDIU(Mips64Reg rt, Mips64Reg rs, u16 immediate)
{
	ImmediateInstruction(0b011001, rs, rt, immediate);
}

void Mips64Emitter::DADDU(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt)
{
	RegisterInstruction(0b000000, rs, rt, rd, 0, 0b101101);
}

void Mips64Emitter::DAHI(Mips64Reg rs, u16 immediate)
{
	ImmediateInstruction(0b000001, rs, (Mips64Reg)0b00110, immediate);
}

void Mips64Emitter::DATI(Mips64Reg rs, u16 immediate)
{
	ImmediateInstruction(0b000001, rs, (Mips64Reg)0b11110, immediate);
}

void Mips64Emitter::DAUI(Mips64Reg rt, Mips64Reg rs, u16 immediate)
{
	// TODO: rs can't be R0
	ImmediateInstruction(0b011101, rs, rt, immediate);
}

void Mips64Emitter::DSLL(Mips64Reg rd, Mips64Reg rt, u8 sa)
{
	RegisterInstruction(0b000000, (Mips64Reg)0b00000, rt, rd, sa, 0b111000);
}

void Mips64Emitter::DSLL32(Mips64Reg rd, Mips64Reg rt, u8 sa)
{
	RegisterInstruction(0b000000, (Mips64Reg)0b00000, rt, rd, sa, 0b111100);
}

void Mips64Emitter::LUI(Mips64Reg rt, u16 immediate)
{
	AUI(rt, R0, immediate);
}

void Mips64Emitter::OR(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt)
{
	RegisterInstruction(0b000000, rs, rt, rd, 0, 0b100101);
}

void Mips64Emitter::ORI(Mips64Reg rt, Mips64Reg rs, u16 immediate)
{
	ImmediateInstruction(0b001101, rs, rt, immediate);
}

void Mips64Emitter::SLL(Mips64Reg rd, Mips64Reg rt, u8 sa)
{
	RegisterInstruction(0b000000, (Mips64Reg)0b00000, rt, rd, sa, 0b000000);
}

void Mips64Emitter::SRL(Mips64Reg rd, Mips64Reg rt, u8 sa)
{
	// TODO: 5th bit of arg2?
	RegisterInstruction(0b000000, (Mips64Reg)0b00000, rt, rd, sa, 0b000010);
}

void Mips64Emitter::XOR(Mips64Reg rd, Mips64Reg rs, Mips64Reg rt)
{
	RegisterInstruction(0b000000, rs, rt, rd, 0, 0b100110);
}

void Mips64Emitter::B(s32 offset)
{
	BEQ(R0, R0, offset);
}

void Mips64Emitter::BC(s32 offset)
{
	// TODO: Assert greater than 26 bits
	offset -= 4;
	JumpInstruction(0b110010, offset >> 2);
}

void Mips64Emitter::BEQ(Mips64Reg rs, Mips64Reg rt, s32 offset)
{
	// TODO: Assert greater than 16 bits
	offset -= 4;
	ImmediateInstruction(0b000100, rs, rt, offset >> 2);
}

void Mips64Emitter::JALR(Mips64Reg rd, Mips64Reg rs)
{
	RegisterInstruction(0b000000, rs, (Mips64Reg)0b00000, rd, 0, 0b001001);
}

void Mips64Emitter::JALR(Mips64Reg rs)
{
	JALR(R31, rs);
}

void Mips64Emitter::JR(Mips64Reg rs)
{
	JALR(R0, rs);
}

void Mips64Emitter::BREAK(u32 code)
{
	// TODO: Check if code to high, assert
	SpecialInstruction(0b000000, code, 0b001101);
}

void Mips64Emitter::BREAK()
{
	BREAK(0);
}

void Mips64Emitter::NOP()
{
	SLL(R0, R0, 0);
}

void Mips64Emitter::MOVI2R(Mips64Reg rt, u64 immediate)
{
	// 64 Bit
	u16 imm_top = (u16)(immediate >> 48);
	u16 imm_high = (u16)((immediate >> 32) & 0xFFFF);
	u16 imm_upper = (u16)((immediate >> 16) & 0xFFFF);
	u16 imm_low = (u16)(immediate & 0xFFFF);
	if(imm_upper & 0x8000)
	{
		imm_high += 1;
		imm_top += 1;
	}
	else if(imm_high & 0x8000)
	{
		imm_top += 1;
	}
	LUI(rt, imm_upper);
	DAHI(rt, imm_high);
	DATI(rt, imm_top);
	ORI(rt, rt, imm_low);
}

} // namespace Mips64Gen

