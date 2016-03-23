// B-em v2.2 by Tom Walker
// 32016 parasite processor emulation (not working yet)

// And Simon R. Ellwood
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defs.h"
#include "Decode.h"
#include "32016.h"
#include "mem32016.h"
#include "Trap.h"
#include "NDis.h"
#include "Profile.h"

#ifdef PROFILING
#include "Profile.h"
#endif

//#define OLD_INSTRUCTION_TRACE
#define CXP_UNUSED_WORD 0xAAAA

#ifdef NEW_SP_METHOD
uint32_t old_sp_index;
#endif

int nsoutput = 0;

ProcessorRegisters PR;
uint32_t r[8];
FloatingPointRegisters FR;
uint32_t FSR;
uint32_t sp[2];
Temp64Type Immediate64;
DecodeData Data;
uint32_t genaddr[2];
int gentype[2];

const uint32_t IndexLKUP[8] = { 0x0, 0x1, 0x4, 0x5, 0x8, 0x9, 0xC, 0xD };                    // See Page 2-3 of the manual!


#undef TrapTRACE
#define TrapTRACE PiTRACE

void n32016_ShowRegs(int Option)
{
   if (Option & BIT(0))
   {
      TrapTRACE("R0=%08"PRIX32" R1=%08"PRIX32" R2=%08"PRIX32" R3=%08"PRIX32"\n", r[0], r[1], r[2], r[3]);
      TrapTRACE("R4=%08"PRIX32" R5=%08"PRIX32" R6=%08"PRIX32" R7=%08"PRIX32"\n", r[4], r[5], r[6], r[7]);
   }

   if (Option & BIT(1))
   {
      TrapTRACE("PC=%08"PRIX32" SB=%08"PRIX32" SP=%08"PRIX32" TRAP=%08"PRIX32"\n", Data.CurrentAddress, sb, GET_SP(), TrapFlags);
      TrapTRACE("FP=%08"PRIX32" INTBASE=%08"PRIX32" PSR=%04"PRIX32" MOD=%04"PRIX32"\n", fp, intbase, psr, mod);
   }

   if (nscfg.fpu_flag)
   {
      if (Option & BIT(2))
      {
         TrapTRACE("F0=%f F1=%f F2=%f F3=%f\n", FR.fr32[0], FR.fr32[1], FR.fr32[4], FR.fr32[5]);
         TrapTRACE("F4=%f F5=%f F6=%f F7=%f\n", FR.fr32[8], FR.fr32[9], FR.fr32[12], FR.fr32[13]);
      }

      if (Option & BIT(3))
      {
         TrapTRACE("D0=%lf D1=%lf D2=%lf D3=%lf\n", FR.fr64[0], FR.fr64[1], FR.fr64[2], FR.fr64[3]);
         TrapTRACE("D4=%lf D5=%lf D6=%lf D7=%lf\n", FR.fr64[4], FR.fr64[5], FR.fr64[6], FR.fr64[7]);
      }
   }
}

const uint32_t OpSizeLookup[6] =
{
   (sz8  << 16) | sz8,                // Integer byte
   (sz16 << 16) | sz16,               // Integer word
   0,                                 // Illegal
   (sz32 << 16) | sz32,               // Integer double-word
   (sz32 << 16) | sz32,                // Floating Point Single Precision
   (sz64 << 16) | sz64                 // Floating Point Double Precision
};

void n32016_init()
{
   init_ram();
}

void n32016_close()
{
}

void n32016_reset()
{
   n32016_reset_addr(0xF00000);
}

void n32016_reset_addr(uint32_t StartAddress)
{
   n32016_build_matrix();

   Data.CurrentAddress  = StartAddress;

#ifdef NEW_SP_METHOD
   old_sp_index         = 0;
#endif

   psr                  = 0;

   //PR.BPC = 0x20F; //Example Breakpoint
   PR.BPC = 0xFFFFFFFF;
}

static void pushd(uint32_t val)
{
   DEC_SP(4);
   write_x32(GET_SP(), val);
}

void PushArbitary(uint64_t Value, uint32_t Size)
{
   DEC_SP(Size);
   write_Arbitary(GET_SP(), &Value, Size);
}

static uint16_t popw()
{
   uint16_t temp = read_x16(GET_SP());
   INC_SP(2);

   return temp;
}

static uint32_t popd()
{
   uint32_t temp = read_x32(GET_SP());
   INC_SP(4);

   return temp;
}

uint32_t PopArbitary(uint32_t Size)
{
   uint32_t Result = read_n(GET_SP(), Size);
   INC_SP(Size);

   return Result;
}

int32_t GetDisplacement(DecodeData* This)
{
   // Displacements are in Little Endian and need to be sign extended
   int32_t Value;

   MultiReg Disp;
   Disp.u32 = SWAP32(read_x32_direct(This->CurrentAddress));

   switch (Disp.u32 >> 29)
      // Look at the top 3 bits
   {
      case 0: // 7 Bit Positive
      case 1:
      {
         Value = Disp.u8;
         This->CurrentAddress += sizeof(int8_t);
      }
      break;

      case 2: // 7 Bit Negative
      case 3:
      {
         Value = (Disp.u8 | 0xFFFFFF80);
         This->CurrentAddress += sizeof(int8_t);
      }
      break;

      case 4: // 14 Bit Positive
      {
         Value = (Disp.u16 & 0x3FFF);
         This->CurrentAddress += sizeof(int16_t);
      }
      break;

      case 5: // 14 Bit Negative
      {
         Value = (Disp.u16 | 0xFFFFC000);
         This->CurrentAddress += sizeof(int16_t);
      }
      break;

      case 6: // 30 Bit Positive
      {
         Value = (Disp.u32 & 0x3FFFFFFF);
         This->CurrentAddress += sizeof(int32_t);
      }
      break;

      case 7: // 30 Bit Negative
      default: // Stop it moaning about Value not being set ;)
      {
         Value = Disp.u32;
         This->CurrentAddress += sizeof(int32_t);
      }
      break;
   }

   return Value;
}

uint32_t Truncate(uint32_t Value, uint32_t Size)
{
   switch (Size)
   {
      case sz8:
         return Value & 0x000000FF;
      case sz16:
         return Value & 0x0000FFFF;
   }

   return Value;
}

uint32_t ReadGen(uint32_t c)
{
   uint32_t Temp = 0;

   switch (gentype[c])
   {
      case Memory:
      {
         switch (Data.Info.Op[c].Size)
         {
            case sz8:   return read_x8(genaddr[c]);
            case sz16:  return read_x16(genaddr[c]);
            case sz32:  return read_x32(genaddr[c]);
         }
      }
      break;

      case Register:
      {
         Temp = *(uint32_t*) genaddr[c];
         return Truncate(Temp, Data.Info.Op[c].Size);
      }
      // No break due to return
     
      case TOS:
      {
         return PopArbitary(Data.Info.Op[c].Size);
      }
      // No break due to return

      case OpImmediate:
      {
         return genaddr[c];
      }
      // No break due to return
   }

   return 0;
}

uint64_t ReadGen64(uint32_t c)
{
   uint64_t Temp = 0;

   switch (gentype[c])
   {
      case Memory:
      {
         Temp = read_x64(genaddr[c]);
      }
      break;

      case Register:
      {
         Temp = *(uint64_t*)genaddr[c];
      }
      break;

      case TOS:
      {
         Temp = read_x64(GET_SP());
         INC_SP(sz64);
      }
      break;

      case OpImmediate:
      {
         Temp = Immediate64.u64;
      }
      break;
   }

   return Temp;
}

uint32_t ReadAddress(uint32_t c)
{
   if (gentype[c] == Register)
   {
      return *(uint32_t *) genaddr[c];
   }
 
   return genaddr[c];
}

static void GetGenPhase2(RegLKU gen, int c)
{
   if (Data.Info.Op[c].Class)
//   if (gen.Whole < 0xFFFF)                                              // Does this Operand exist ?
   {
      if (gen.OpType <= R7)
      {
         if (Data.Info.Op[c].Class & 0x80)
         { 
            if (Data.Info.Op[c].Size == sz32)
            {
               genaddr[c] = (uint32_t) &FR.fr32[IndexLKUP[gen.OpType]];
            }
            else
            {
               genaddr[c] = (uint32_t) &FR.fr64[gen.OpType];
            }
         }
         else
         {
            genaddr[c] = (uint32_t) &r[gen.OpType];
         }

         gentype[c] = Register;
         return;
      }

      if (gen.OpType == Immediate)
      {
         gentype[c] = OpImmediate;

         if (Data.Info.Op[c].Size == sz8)
         {
            genaddr[c] = Consume_x8(&Data);
            return;
         }
                
         MultiReg temp3;
         temp3.u32 = SWAP32(read_x32_direct(Data.CurrentAddress));

         if (Data.Info.Op[c].Size == sz64)
         {
            Immediate64.u64 = (((uint64_t) temp3.u32) << 32) | SWAP32(read_x32_direct(Data.CurrentAddress + 4));
         }
         else
         {
            genaddr[c] = (Data.Info.Op[c].Size == sz32) ? temp3.u32 : temp3.u16;
         }

         Data.CurrentAddress += Data.Info.Op[c].Size;
         return;
      }

      gentype[c] = Memory;

      if (gen.OpType <= R7_Offset)
      {
         genaddr[c] = r[gen.Whole & 7] + GetDisplacement(&Data);
         return;
      }

      uint32_t temp, temp2;

      if (gen.OpType >= EaPlusRn)
      {
         uint32_t Shift = gen.Whole & 3;
         RegLKU NewPattern;
         NewPattern.Whole = gen.IdxType;
         GetGenPhase2(NewPattern, c);

         int32_t Offset = ((int32_t) r[gen.IdxReg]) * (1 << Shift);
         if (gentype[c] != Register)
         {
            genaddr[c] += Offset;
         }
         else
         {
            genaddr[c] = *((uint32_t*) genaddr[c]) + Offset;
         }

         gentype[c] = Memory;                               // Force Memory
         return;
      }

      switch (gen.OpType)
      {
         case FrameRelative:
            temp = GetDisplacement(&Data);
            temp2 = GetDisplacement(&Data);
            genaddr[c] = read_x32(fp + temp);
            genaddr[c] += temp2;
            break;

         case StackRelative:
            temp = GetDisplacement(&Data);
            temp2 = GetDisplacement(&Data);
            genaddr[c] = read_x32(GET_SP() + temp);
            genaddr[c] += temp2;
            break;

         case StaticRelative:
            temp = GetDisplacement(&Data);
            temp2 = GetDisplacement(&Data);
            genaddr[c] = read_x32(sb + temp);
            genaddr[c] += temp2;
            break;

         case Absolute:
            genaddr[c] = GetDisplacement(&Data);
            break;

         case External:
            temp = read_x32(mod + 4);
            temp += ((int32_t) GetDisplacement(&Data)) * 4;
            temp2 = read_x32(temp);
            genaddr[c] = temp2 + GetDisplacement(&Data);
            break;

         case TopOfStack:
            genaddr[c] = GET_SP();
            gentype[c] = TOS;
            break;

         case FpRelative:
            genaddr[c] = GetDisplacement(&Data) + fp;
            break;

         case SpRelative:
            genaddr[c] = GetDisplacement(&Data) + GET_SP();
            break;

         case SbRelative:
            genaddr[c] = GetDisplacement(&Data) + sb;
            break;

         case PcRelative:
            genaddr[c] = GetDisplacement(&Data) + Data.StartAddress;
            break;

         default:
            n32016_dumpregs("Bad NS32016 gen mode");
            break;
      }
   }
}

// From: http://homepage.cs.uiowa.edu/~jones/bcd/bcd.html
static uint32_t bcd_add_16(uint32_t a, uint32_t b, uint32_t *carry)
{
   uint32_t t1, t2; // unsigned 32-bit intermediate values
   //PiTRACE("bcd_add_16: in  %08x %08x %08x\n", a, b, *carry);
   if (*carry)
   {
      b++; // I'm 90% sure its OK to handle carry this way
   } // i.e. its OK if the ls digit of b becomes A
   t1 = a + 0x06666666;
   t2 = t1 ^ b; // sum without carry propagation
   t1 = t1 + b; // provisional sum
   t2 = t1 ^ t2; // all the binary carry bits
   t2 = ~t2 & 0x11111110; // just the BCD carry bits
   t2 = (t2 >> 2) | (t2 >> 3); // correction
   t2 = t1 - t2; // corrected BCD sum
   *carry = (t2 & 0xFFFF0000) ? 1 : 0;
   t2 &= 0xFFFF;
   //PiTRACE("bcd_add_16: out %08x %08x\n", t2, *carry);
   return t2;
}

static uint32_t bcd_sub_16(uint32_t a, uint32_t b, uint32_t *carry)
{
   uint32_t t1, t2; // unsigned 32-bit intermediate values
   //PiTRACE("bcd_sub_16: in  %08x %08x %08x\n", a, b, *carry);
   if (*carry)
   {
      b++;
   }
   *carry = 0;
   t1 = 0x9999 - b;
   t2 = bcd_add_16(t1, 1, carry);
   t2 = bcd_add_16(a, t2, carry);
   *carry = 1 - *carry;
   //PiTRACE("bcd_add_16: out %08x %08x\n", t2, *carry);
   return t2;
}

static uint32_t bcd_add(uint32_t a, uint32_t b, int size, uint32_t *carry)
{
   if (size == sz8)
   {
      uint32_t word = bcd_add_16(a, b, carry);
      // If anything beyond bit 7 is set, then there has been a carry out
      *carry = (word & 0xFF00) ? 1 : 0;
      return word & 0xFF;
   }
   else if (size == sz16)
   {
      return bcd_add_16(a, b, carry);
   }
   else
   {
      uint32_t word0 = bcd_add_16(a & 0xFFFF, b & 0xFFFF, carry);
      uint32_t word1 = bcd_add_16(a >> 16, b >> 16, carry);
      return word0 + (word1 << 16);
   }
}

static uint32_t bcd_sub(uint32_t a, uint32_t b, int size, uint32_t *carry)
{
   if (size == sz8)
   {
      uint32_t word = bcd_sub_16(a, b, carry);
      // If anything beyond bit 7 is set, then there has been a carry out
      *carry = (word & 0xFF00) ? 1 : 0;
      return word & 0xFF;
   }
   else if (size == sz16)
   {
      return bcd_sub_16(a, b, carry);
   }
   else
   {
      uint32_t word0 = bcd_sub_16(a & 0xFFFF, b & 0xFFFF, carry);
      uint32_t word1 = bcd_sub_16(a >> 16, b >> 16, carry);
      return word0 + (word1 << 16);
   }
}

static uint32_t AddCommon(uint32_t a, uint32_t b, uint32_t cin)
{
   uint32_t sum = a + (b + cin);

   if (b == 0xffffffff && cin == 1)
   {
      C_FLAG = 1;
      F_FLAG = 0;
   }
   else
   {
      // Overflow can only happen in the following cases:
      //   A is positive, B is positive, sum is negative
      //   A is negative, B is negative, sum is positive
      // So the test on the sign bits is (A ^ sum) & (B ^ sum)
      // Note: this test implies sign A == sign B
      switch (Data.Info.Op[0].Size)
      {
         case sz8:
            {
               C_FLAG = TEST(sum & 0x100);
               F_FLAG = TEST((a ^ sum) & (b ^ sum) & 0x80);
            }
            break;

         case sz16:
            {
               C_FLAG = TEST(sum & 0x10000);
               F_FLAG = TEST((a ^ sum) & (b ^ sum) & 0x8000);
            }
            break;

         case sz32:
            {
               C_FLAG = TEST(sum < a);
               F_FLAG = TEST((a ^ sum) & (b ^ sum) & 0x80000000);
            }
            break;
      }
   }
   //PiTRACE("ADD FLAGS: C=%d F=%d\n", C_FLAG, F_FLAG);

   return sum;
}

static uint32_t SubCommon(uint32_t a, uint32_t b, uint32_t cin)
{
   uint32_t diff = a - (b + cin);

   if (b == 0xffffffff && cin == 1)
   {
      C_FLAG = 1;
      F_FLAG = 0;
   }
   else
   {
      // Overflow can only happen in the following cases:
      //   A is positive, B is negative, diff is negative
      //   A is negative, B is positive, diff is positive
      // So the test on the sign bits is (A ^ B) & (A ^ diff)
      // Note: this test implies sign diff == sign B
      switch (Data.Info.Op[0].Size)
      {
         case sz8:
            {
               C_FLAG = TEST(diff & 0x100);
               F_FLAG = TEST((a ^ b) & (a ^ diff) & 0x80);
            }
            break;

         case sz16:
            {
               C_FLAG = TEST(diff & 0x10000);
               F_FLAG = TEST((a ^ b) & (a ^ diff) & 0x8000);
            }
            break;

         case sz32:
            {
               C_FLAG = TEST(diff > a);
               F_FLAG = TEST((a ^ b) & (a ^ diff) & 0x80000000);
            }
            break;
      }
   }
   //PiTRACE("SUB FLAGS: C=%d F=%d\n", C_FLAG, F_FLAG);

   return diff;
}

// The difference between DIV and QUO occurs when the result (the quotient) is negative
// e.g. -16 QUO 3 ===> -5
// e.g. -16 DIV 3 ===> -6
// i.e. DIV rounds down to the more negative nu,ber
// This case is detected if the sign bits of the two operands differ
static uint32_t div_operator(uint32_t a, uint32_t b)
{
   uint32_t ret = 0;
   int signmask = BIT(((Data.Info.Op[0].Size - 1) << 3) + 7);
   if ((a & signmask) && !(b & signmask))
   {
      // e.g. a = -16; b =  3 ===> a becomes -18
      a -= b - 1;
   }
   else if (!(a & signmask) && (b & signmask))
   {
      // e.g. a =  16; b = -3 ===> a becomes 18
      a -= b + 1;
   }
   switch (Data.Info.Op[0].Size)
   {
      case sz8:
         ret = (int8_t) a / (int8_t) b;
      break;

      case sz16:
         ret = (int16_t) a / (int16_t) b;
      break;

      case sz32:
         ret = (int32_t) a / (int32_t) b;
      break;
   }
   return ret;
}

static uint32_t mod_operator(uint32_t a, uint32_t b)
{
   return a - div_operator(a, b) * b;
}


// OffsetDiv8 needs to work as follows
//
//  9 =>  1
//  8 =>  1
//  7 =>  0
//  6 =>  0
//  5 =>  0
//  4 =>  0
//  3 =>  0
//  2 =>  0
//  1 =>  0
//  0 =>  0
// -1 => -1
// -2 => -1
// -3 => -1
// -4 => -1
// -5 => -1
// -6 => -1
// -7 => -1
// -8 => -1
// -9 => -2

static int32_t OffsetDiv8(int32_t Offset)
{
   if (Offset >= 0)
   {
      return Offset / 8;
   }
   else
   {
      return (Offset - 7) / 8;
   }
}

// Handle the writing to the upper half of mei/dei destination
static void handle_mei_dei_upper_write(uint64_t result)
{
   uint32_t temp;
   // Writing to an odd register is strictly speaking undefined
   // But BBC Basic relies on a particular behaviour that the NS32016 has in this case
   uint32_t reg_addr = genaddr[1] + ((Data.Regs[1].Whole & 1) ? -4 : 4);
   switch (Data.Info.Op[0].Size)
   {
      case sz8:
         temp = (uint8_t) (result >> 8);
         if (gentype[1] == Register)
            *(uint8_t *) (reg_addr) = temp;
         else
            write_x8(genaddr[1] + 4, temp);
      break;

      case sz16:
         temp = (uint16_t) (result >> 16);
         if (gentype[1] == Register)
            *(uint16_t *) (reg_addr) = temp;
         else
            write_x16(genaddr[1] + 4, temp);
      break;

      case sz32:
         temp = (uint32_t) (result >> 32);
         if (gentype[1] == Register)
            *(uint32_t *) (reg_addr) = temp;
         else
            write_x32(genaddr[1] + 4, temp);
      break;
   }
}

uint32_t CompareCommon(uint32_t src1, uint32_t src2)
{
   L_FLAG = TEST(src1 > src2);

   if (Data.Info.Op[0].Size == sz8)
   {
      N_FLAG = TEST(((int8_t) src1) > ((int8_t) src2));
   }
   else if (Data.Info.Op[0].Size == sz16)
   {
      N_FLAG = TEST(((int16_t) src1) > ((int16_t) src2));
   }
   else
   {
      N_FLAG = TEST(((int32_t) src1) > ((int32_t) src2));
   }

   Z_FLAG = TEST(src1 == src2);

   return Z_FLAG;
}

uint32_t StringMatching(uint32_t opcode, uint32_t Value)
{
   uint32_t Options = (opcode >> 17) & 3;

   if (Options)
   {
      uint32_t Compare = Truncate(r[4], Data.Info.Op[0].Size);

      if (Options == 1) // While match
      {
         if (Value != Compare)
         {
            F_FLAG = 1; // Set PSR F Bit
            return 1;
         }
      }
      else if (Options == 3) // Until Match
      {
         if (Value == Compare)
         {
            F_FLAG = 1; // Set PSR F Bit
            return 1;
         }
      }
   }

   return 0;
}

void StringRegisterUpdate(uint32_t opcode)
{
   uint32_t Size = Data.Info.Op[0].Size;

   if (opcode & BIT(Backwards)) // Adjust R1
   {
      r[1] -= Size;
   }
   else
   {
      r[1] += Size;
   }

   if (((opcode >> 10) & 0x0F) != (SKPS & 0x0F))
   {
      if (opcode & BIT(Backwards)) // Adjust R2 for all but SKPS
      {
         r[2] -= Size;
      }
      else
      {
         r[2] += Size;
      }
   }

   r[0]--; // Adjust R0
}

uint32_t CheckCondition(uint32_t Pattern)
{
   uint32_t bResult = 0;

   switch (Pattern & 0xF)
   {
      case 0x0:
         if (Z_FLAG)
            bResult = 1;
         break;
      case 0x1:
         if (!Z_FLAG)
            bResult = 1;
         break;
      case 0x2:
         if (C_FLAG)
            bResult = 1;
         break;
      case 0x3:
         if (!C_FLAG)
            bResult = 1;
         break;
      case 0x4:
         if (L_FLAG)
            bResult = 1;
         break;
      case 0x5:
         if (!L_FLAG)
            bResult = 1;
         break;
      case 0x6:
         if (N_FLAG)
            bResult = 1;
         break;
      case 0x7:
         if (!N_FLAG)
            bResult = 1;
         break;
      case 0x8:
         if (F_FLAG)
            bResult = 1;
         break;
      case 0x9:
         if (!F_FLAG)
            bResult = 1;
         break;
      case 0xA:
         if (!(psr & (0x40 | 0x04)))
         //if (!(Z_FLAG || L_FLAG))
            bResult = 1;
         break;
      case 0xB:
         if (psr & (0x40 | 0x04))
         //if (Z_FLAG || L_FLAG)
            bResult = 1;
         break;
      case 0xC:
         if (!(psr & (0x40 | 0x80)))
         //if (!(Z_FLAG || N_FLAG))
            bResult = 1;
         break;
      case 0xD:
         if (psr & (0x40 | 0x80))
         //if (Z_FLAG || N_FLAG)
            bResult = 1;
         break;
      case 0xE:
         bResult = 1;
         break;
      case 0xF:
         break;
   }

   return bResult;
}

uint32_t BitPrefix(void)
{
   int32_t Offset = ReadGen(0);
   uint32_t bit;
   SIGN_EXTEND(Data.Info.Op[0].Size, Offset);

   if (gentype[1] == Register)
   {
      // operand 0 is a register
      Data.Info.Op[1].Size = sz32;
      bit = ((uint32_t) Offset) & 31;
   }
   else
   {
      // operand0 is memory
      genaddr[1] += OffsetDiv8(Offset);
      Data.Info.Op[1].Size = sz8;
      bit = ((uint32_t) Offset) & 7;
   }

   return BIT(bit);
}

void PopRegisters(void)
{
   int c;
   int32_t temp = Consume_x8(&Data);

   for (c = 0; c < 8; c++)
   {
      if (temp & BIT(c))
      {
         r[c ^ 7] = popd();
      }
   }
}

void TakeInterrupt(uint32_t IntBase)
{
   uint32_t temp = psr;
   uint32_t temp2, temp3;

   psr &= ~0xF00;
   UPDATE_SP();
   pushd((temp << 16) | mod);
   
   while (read_x8(Data.CurrentAddress) == 0xB2)                                    // Do not stack the address of a WAIT instruction!
   {
      Data.CurrentAddress++;
   }
   
   pushd(Data.CurrentAddress);
   temp = read_x32(IntBase);
   mod = temp & 0xFFFF;
   temp3 = temp >> 16;
   sb = read_x32(mod);
   temp2 = read_x32(mod + 8);
   Data.CurrentAddress = temp2 + temp3;
}

void WarnIfShiftInvalid(uint32_t shift, uint8_t size)
{
   size *= 8;    // 8, 16, 32
   if (((shift >= size) && (shift <= (0xFF - size))) || (shift > 0xFF))
   {
      PiWARN("Invalid shift of %08"PRIX32" for size %"PRId8"\n", shift, size);
   }
}

uint32_t ReturnCommon(void)
{
   if (U_FLAG)
   {
      return 1;                  // Trap
   }

   Data.CurrentAddress = popd();
   uint16_t unstack = popw();

   if (nscfg.de_flag == 0)
   {
      mod = unstack;
   }

   psr = popw();
   UPDATE_SP();

   if (nscfg.de_flag == 0)
   {
      sb = read_x32(mod);
   }

   return 0;                     // OK
}

void n32016_exec()
{
   uint32_t temp2, temp3;
   Temp32Type Src, Dst;
   Temp64Type Src64, Dst64;
   int32_t Disp;

   if (tube_irq & 2)
   {
      // NMI is edge sensitive, so it should be cleared here
      tube_irq &= ~2;
      TakeInterrupt(intbase + (1 * 4));
   }
   else if ((tube_irq & 1) && (psr & 0x800))
   {
      // IRQ is level sensitive, so the called should maintain the state
      TakeInterrupt(intbase);
   }

   while (tubecycles > 0)
   {
      tubecycles -= 8;

      //if (Data.CurrentAddress == 0xF000B5)
      //{
      //   printf("Brk\n");
      //}

      TrapFlags |= Decode(&Data);
      
#ifdef OLD_INSTRUCTION_TRACE
      DecodeData This;
      This.CurrentAddress = Data.StartAddress;
      Decode(&This);
      ShowInstruction(&This);
#endif

      if (Data.Info.Whole)
      {
         GetGenPhase2(Data.Regs[0], 0);
         GetGenPhase2(Data.Regs[1], 1);

         switch (Data.Info.Op[0].Class & 0x0F)
         {
            case read >> 8:
            case rmw >> 8:
            {
               if (Data.Info.Op[0].Size == sz64)
               {
                  Src64.u64 = ReadGen64(0);
               }
               else
               {
                  Src.u32 = ReadGen(0);
               }
            }
            break;

            case addr >> 8:
            {
               Src.u32 = ReadAddress(0);
            }
            break;

            case Regaddr >> 8:
            {
               Src.u32 = BitPrefix();
            }
         }

         switch (Data.Info.Op[1].Class & 0x0F)
         {
            case read >> 8:
            case rmw >> 8:
            {
               if (Data.Info.Op[1].Size == sz64)
               {
                  Dst64.u64 = ReadGen64(1);
               }
               else
               {
                  Dst.u32 = ReadGen(1);
               }
            }
            break;

            case addr >> 8:
            {
               Dst.u32 = ReadAddress(1);
            }
            break;
         }
      }
      else if (Data.Function <= RETT)
      {
         Disp = GetDisplacement(&Data);
      }

      switch (Data.Function)
      {
         // Format 0 : Branches

         case BEQ:
         case BNE:
         case BCS:
         case BCC:
         case BH:
         case BLS:
         case BGT:
         case BLE:
         case BFS:
         case BFC:
         case BLO:
         case BHS:
         case BLT:
         case BGE:
         {
            if (CheckCondition(Data.Function) == 0)
            {
               goto skip_write;
            }
         }
         // Fall Through

         case BR:
         {
            Data.CurrentAddress = Data.StartAddress + Disp;
            goto skip_write;
         }
         // No break due to goto

         case BN:
         {
            goto skip_write;
         }
         // No break due to goto

         // Format 1

         case BSR:
         {
            pushd(Data.CurrentAddress);
            Data.CurrentAddress = Data.StartAddress + Disp;
            goto skip_write;
         }
         // No break due to goto

         case RET:
         {
            Data.CurrentAddress = popd();
            INC_SP(Disp);
            goto skip_write;
         }
         // No break due to goto

         case CXP:
         {
            temp2 = read_x32(mod + 4) + Disp * 4;

            uint32_t temp = read_x32(temp2);   // Matching Tail with CXPD, complier do your stuff
            pushd((CXP_UNUSED_WORD << 16) | mod);
            pushd(Data.CurrentAddress);
            mod = temp & 0xFFFF;
            temp3 = temp >> 16;
            sb = read_x32(mod);
            temp2 = read_x32(mod + 8);
            Data.CurrentAddress = temp2 + temp3;
            goto skip_write;
         }
         // No break due to goto

         case RXP:
         {
            Data.CurrentAddress = popd();
            temp2 = popd();
            mod = temp2 & 0xFFFF;
            INC_SP(Disp);
            sb = read_x32(mod);
            goto skip_write;
         }
         // No break due to goto

         case RETT:
         {
            if (ReturnCommon())
            {
               SET_TRAP(PrivilegedInstruction);
               goto skip_write;
            }

            INC_SP(Disp);
            goto skip_write;
         }
         // No break due to goto

         case RETI:
         {
            // No "End of Interrupt" bus cycles here!
            if (ReturnCommon())
            {
               SET_TRAP(PrivilegedInstruction);
            }

            goto skip_write;
         }
         // No break due to goto

         case SAVE:
         {
            int c;
            uint32_t temp = Consume_x8(&Data);

            for (c = 0; c < 8; c++)                             // Matching tail with ENTER
            {
               if (temp & BIT(c))
               {
                  pushd(r[c]);
               }
            }
            goto skip_write;
         }
         // No break due to goto

         case RESTORE:
         {
            PopRegisters();
            goto skip_write;
         }
         // No break due to goto

         case ENTER:
         {
            int c;
            uint32_t temp = Consume_x8(&Data);
            Disp = GetDisplacement(&Data);
            pushd(fp);
            fp = GET_SP();
            DEC_SP(Disp);

            for (c = 0; c < 8; c++)                              // Matching tail with SAVE
            {
               if (temp & BIT(c))
               {
                  pushd(r[c]);
               }
            }
            goto skip_write;
         }
         // No break due to goto

         case EXIT:
         {
            PopRegisters();
            SET_SP(fp);
            fp = popd();
            goto skip_write;
         }
         // No break due to goto

         case NOP:
         {
            goto skip_write;
         }
         // No break due to goto

         case WAIT:                                             // Wait for interrupt then goto skip_write execution
         case DIA:                                              // Wait for interrupt and in theory never resume execution (stack manipulation would get round this)
         {
            tubecycles = 0;                                    // Exit promptly as we are waiting for an interrupt
            Data.CurrentAddress = Data.StartAddress;
            goto skip_write;
         }
         // No break due to goto

         case FLAG:
         {
            SET_TRAP(FlagInstruction);
            goto skip_write;
         }
         // No break due to goto

         case SVC:
         {
            uint32_t temp = psr;
            psr &= ~0x700;
            UPDATE_SP();
            // In SVC, the address pushed is the address of the SVC opcode
            pushd((temp << 16) | mod);
            pushd(Data.StartAddress);
            temp = read_x32(intbase + (5 * 4));
            mod = temp & 0xFFFF;
            temp3 = temp >> 16;
            sb = read_x32(mod);
            temp2 = read_x32(mod + 8);
            Data.CurrentAddress = temp2 + temp3;
            goto skip_write;
         }
         // No break due to goto

         case BPT:
         {
            SET_TRAP(BreakPointTrap);
            goto skip_write;
         }
         // No break due to goto

         // Format 2
         
         case ADDQ:
         {
            temp2 = (Data.OpCode >> 7) & 0xF;
            NIBBLE_EXTEND(temp2);
            Dst.u32 = AddCommon(Src.u32, temp2, 0);
         }
         break;

         case CMPQ:
         {
            temp2 = (Data.OpCode >> 7) & 0xF;
            NIBBLE_EXTEND(temp2);
            SIGN_EXTEND(Data.Info.Op[0].Size, Src.u32);
            CompareCommon(temp2, Src.u32);
            goto skip_write;
         }
         // No break due to goto

         case SPR:
         {
            temp2 = (Data.OpCode >> 7) & 0xF;

            if (U_FLAG)
            {
               if (PrivilegedPSR(temp2))
               {
                  SET_TRAP(PrivilegedInstruction);
                  goto skip_write;
               }
            }

            switch (temp2)
            {
               case 0x0:
                  Dst.u32 = psr_lsb;
               break;
               case 0x8:
                  Dst.u32 = fp;
               break;
               case 0x9:
                  Dst.u32 = GET_SP();    // returned the currently selected stack pointer
               break;
               case 0xA:
                  Dst.u32 = sb;
               break;
               case 0xB:
                  Dst.u32 = GET_USER_SP();       // returns the user stack pointer
               break;
               case 0xD:
                  Dst.u32 = psr;
               break;
               case 0xE:
                  Dst.u32 = intbase;
               break;
               case 0xF:
                  Dst.u32 = mod;
               break;

               default:
               {
                  SET_TRAP(IllegalSpecialReading);
                  goto skip_write;
               }
               // No break due to goto
            }
         }
         break;

         case Scond:
         {
            Dst.u32 = CheckCondition(Data.OpCode >> 7);
         }
         break;

         case ACB:
         {
            temp2 = (Data.OpCode >> 7) & 0xF;
            NIBBLE_EXTEND(temp2);
            Dst.u32 = Src.u32 + temp2;
            Disp = GetDisplacement(&Data);
            if (Truncate(Dst.u32, Data.Info.Op[1].Size))
               Data.CurrentAddress = Data.StartAddress + Disp;
         }
         break;

         case MOVQ:
         {
            Dst.u32 = (Data.OpCode >> 7) & 0xF;
            NIBBLE_EXTEND(Dst.u32);
         }
         break;

         case LPR:
         {
            temp2 = (Data.OpCode >> 7) & 0xF;

            if (U_FLAG)
            {
               if (PrivilegedPSR(temp2))
               {
                  SET_TRAP(PrivilegedInstruction);
                  goto skip_write;
               }
            }

            switch (temp2)
            {
               case 0:
               {
                  psr_lsb = Src.u32;
               }
               break;

               case 5:
               case 6:
               case 7:
               {
                  SET_TRAP(IllegalSpecialWriting);
               }
               break;

               case 9:
               {
                  SET_SP(Src.u32);   // Sets the currently selected stack pointer
               }
               break;

               case 11:
               {
                  SET_USER_SP(Src.u32);   // Sets the user stack pointer
               }
               break;

               default:
               {
                  PR.Direct[temp2] = Src.u32;
               }
               break;
            }

            goto skip_write;
         }
         // No break due to goto
     
         // Format 3

         case CXPD:
         {
            uint32_t temp = read_x32(Src.u32);   // Matching Tail with CXPD, complier do your stuff
            pushd((CXP_UNUSED_WORD << 16) | mod);
            pushd(Data.CurrentAddress);
            mod = temp & 0xFFFF;
            temp3 = temp >> 16;
            sb = read_x32(mod);
            Src.u32 = read_x32(mod + 8);
            Data.CurrentAddress = Src.u32 + temp3;
            goto skip_write;
         }
         // No break due to goto

         case BICPSR:
         {
            if (U_FLAG)
            {
               if (Data.Info.Op[0].Size > sz8)
               {
                  SET_TRAP(PrivilegedInstruction);
                  goto skip_write;
               }
            }
 
            psr &= ~Src.u32;
            UPDATE_SP();
            goto skip_write;
         }
         // No break due to goto

         case JUMP:
         {
            Data.CurrentAddress = Src.u32;
            goto skip_write;
         }
         // No break due to goto

         case BISPSR:
         {
            if (U_FLAG)
            {
               if (Data.Info.Op[0].Size > sz8)
               {
                  SET_TRAP(PrivilegedInstruction);
                  goto skip_write;
               }
            }
            
            psr |= Src.u32;
            UPDATE_SP();
            goto skip_write;
         }
         // No break due to goto

         case ADJSP:
         {
            SIGN_EXTEND(Data.Info.Op[0].Size, Src.u32);
            DEC_SP(Src.u32);
            goto skip_write;
         }
         // No break due to goto

         case JSR:
         {
            pushd(Data.CurrentAddress);
            Data.CurrentAddress = Src.u32;
            goto skip_write;
         }
         // No break due to goto

         case CASE:
         {
            SIGN_EXTEND(Data.Info.Op[0].Size, Src.u32);
            Data.CurrentAddress = Data.StartAddress + Src.u32;
            goto skip_write;
         }
         // No break due to goto

         // Format 4

         case ADD:
         {
            Dst.u32 = AddCommon(Dst.u32, Src.u32, 0);
         }
         break;

         case CMP:
         {
            CompareCommon(Src.u32, Dst.u32);
            goto skip_write;
         }
         // No break due to goto

         case BIC:
         {
            Dst.u32 &= ~Src.u32;
         }
         break;

         case ADDC:
         {
            temp3 = C_FLAG;
            Dst.u32 = AddCommon(Dst.u32, Src.u32, temp3);
         }
         break;

         case MOV:
         {
            Dst.u32 = Src.u32;
         }
         break;

         case OR:
         {
            Dst.u32 |= Src.u32;
         }
         break;

         case SUB:
         {
            Dst.u32 = SubCommon(Dst.u32, Src.u32, 0);
         }
         break;

         case ADDR:
         {
            Dst.u32 = Src.u32;
         }
         break;

         case AND:
         {
             Dst.u32 &= Src.u32;
         }
         break;

         case SUBC:
         {
            temp3 = C_FLAG;
            Dst.u32 = SubCommon(Dst.u32, Src.u32, temp3);
         }
         break;

         case TBIT:
         {
            //Src.u32 = BitPrefix();
            if (gentype[1] == TOS)
            {
               PiWARN("TBIT with base==TOS is not yet implemented\n");
               goto skip_write; // with next instruction
            }

            F_FLAG = TEST(Dst.u32 & Src.u32);
            goto skip_write;
         }
         // No break due to goto

         case XOR:
         {
            Dst.u32 ^= Src.u32;
         }
         break;

         // Format 5

         case MOVS:
         {
            if (r[0] == 0)
            {
               F_FLAG = 0;
               goto skip_write;
            }

            uint32_t temp = read_n(r[1], Data.Info.Op[0].Size);

            if (Data.OpCode & BIT(Translation))
            {
               temp = read_x8(r[3] + temp); // Lookup the translation
            }

            if (StringMatching(Data.OpCode, temp))
            {
               goto skip_write;
            }

            write_Arbitary(r[2], &temp, Data.Info.Op[0].Size);

            StringRegisterUpdate(Data.OpCode);
            Data.CurrentAddress = Data.StartAddress; // Not finsihed so come back again!
            goto skip_write;
         }
         // No break due to goto

         case CMPS:
         {
            if (r[0] == 0)
            {
               F_FLAG = 0;
               goto skip_write;
            }

            uint32_t temp = read_n(r[1], Data.Info.Op[0].Size);

            if (Data.OpCode & BIT(Translation))
            {
               temp = read_x8(r[3] + temp);                               // Lookup the translation
            }

            if (StringMatching(Data.OpCode, temp))
            {
               goto skip_write;
            }

            temp2 = read_n(r[2], Data.Info.Op[0].Size);

            if (CompareCommon(temp, temp2) == 0)
            {
               goto skip_write;
            }

            StringRegisterUpdate(Data.OpCode);
            Data.CurrentAddress = Data.StartAddress;                                               // Not finsihed so come back again!
            goto skip_write;
         }
         // No break due to goto

         case SETCFG:
         {
            if (U_FLAG)
            {
               SET_TRAP(PrivilegedInstruction);
               goto skip_write;
            }

            nscfg.lsb = (Data.OpCode >> 15);                                  // Only sets the bottom 8 bits of which the lower 4 are used!
            goto skip_write;
         }
         // No break due to goto

         case SKPS:
         {
            if (r[0] == 0)
            {
               F_FLAG = 0;
               goto skip_write;
            }

            uint32_t temp = read_n(r[1], Data.Info.Op[0].Size);

            if (Data.OpCode & BIT(Translation))
            {
               temp = read_x8(r[3] + temp); // Lookup the translation
               write_x8(r[1], temp); // Write back
            }

            if (StringMatching(Data.OpCode, temp))
            {
               goto skip_write;
            }

            StringRegisterUpdate(Data.OpCode);
            Data.CurrentAddress = Data.StartAddress; // Not finsihed so come back again!
            goto skip_write;
         }
         // No break due to goto

         // Format 6

         case ROT:
         {
            WarnIfShiftInvalid(Src.u32, Data.Info.Op[1].Size);
 
            temp3 = Data.Info.Op[1].Size * 8;                             // Bit size, compiler will switch to a shift all by itself ;)

            if (Src.u32 & 0xE0)
            {
               Src.u32 |= 0xE0;
               Src.u32 = ((Src.u32 ^ 0xFF) + 1);
               Src.u32 = temp3 - Src.u32;
            }
            Dst.u32 = (Dst.u32 << Src.u32) | (Dst.u32 >> (temp3 - Src.u32));
         }
         break;

         case ASH:
         {
            WarnIfShiftInvalid(Src.u32, Data.Info.Op[1].Size);

            // Test if the shift is negative (i.e. a right shift)
            if (Src.u32 & 0xE0)
            {
               Src.u32 |= 0xE0;
               Src.u32 = ((Src.u32 ^ 0xFF) + 1);
               if (Data.Info.Op[1].Size == sz8)
               {
                  // Test if the operand is also negative
                  if (Dst.u32 & 0x80)
                  {
                     // Sign extend in a portable way
                     Dst.u32 = (Dst.u32 >> Src.u32) | ((0xFF >> Src.u32) ^ 0xFF);
                  }
                  else
                  {
                     Dst.u32 = (Dst.u32 >> Src.u32);
                  }
               }
               else if (Data.Info.Op[1].Size == sz16)
               {
                  if (Dst.u32 & 0x8000)
                  {
                     Dst.u32 = (Dst.u32 >> Src.u32) | ((0xFFFF >> Src.u32) ^ 0xFFFF);
                  }
                  else
                  {
                     Dst.u32 = (Dst.u32 >> Src.u32);
                  }
               }
               else
               {
                  if (Dst.u32 & 0x80000000)
                  {
                     Dst.u32 = (Dst.u32 >> Src.u32) | ((0xFFFFFFFF >> Src.u32) ^ 0xFFFFFFFF);
                  }
                  else
                  {
                     Dst.u32 = (Dst.u32 >> Src.u32);
                  }
               }
            }
            else
               Dst.u32 <<= Src.u32;
         }
         break;

         case CBIT:
         case CBITI:
         {
            // The CBITI instructions, in addition, activate the Interlocked
            // Operation output pin on the CPU, which may be used in multiprocessor systems to
            // interlock accesses to semaphore bits. This aspect is not implemented here.
            F_FLAG = TEST(Dst.u32 & Src.u32);
            Dst.u32 &= ~(Src.u32);
         }
         break;

         case LSH:
         {
            WarnIfShiftInvalid(Src.u32, Data.Info.Op[1].Size);

            if (Src.u32 & 0xE0)
            {
               Src.u32 |= 0xE0;
               Dst.u32 >>= ((Src.u32 ^ 0xFF) + 1);
            }
            else
               Dst.u32 <<= Src.u32;
         }
         break;

         case SBIT:
         case SBITI:
         {
            // The SBITI instructions, in addition, activate the Interlocked
            // Operation output pin on the CPU, which may be used in multiprocessor systems to
            // interlock accesses to semaphore bits. This aspect is not implemented here.
            F_FLAG = TEST(Dst.u32 & Src.u32);
            Dst.u32 |= Src.u32;
         }
         break;

         case NEG:
         {
            Dst.u32 = SubCommon(0, Src.u32, 0);
         }
         break;

         case NOT:
         {
            Dst.u32 = Src.u32 ^ 1;
         }
         break;

         case SUBP:
         {
            uint32_t carry = C_FLAG;
            Dst.u32 = bcd_sub(Dst.u32, Src.u32, Data.Info.Op[0].Size, &carry);
            C_FLAG = TEST(carry);
            F_FLAG = 0;
         }
         break;

         case ABS:
         {
            Dst.u32 = Src.u32;
            switch (Data.Info.Op[0].Size)
            {
               case sz8:
               {
                  if (Dst.u32 == 0x80)
                  {
                     F_FLAG = 1;
                  }
                  if (Dst.u32 & 0x80)
                  {
                     Dst.u32 = (Dst.u32 ^ 0xFF) + 1;
                  }
               }
               break;

               case sz16:
               {
                  if (Dst.u32 == 0x8000)
                  {
                     F_FLAG = 1;
                  }
                  if (Dst.u32 & 0x8000)
                  {
                     Dst.u32 = (Dst.u32 ^ 0xFFFF) + 1;
                  }
               }
               break;

               case sz32:
               {
                  if (Dst.u32 == 0x80000000)
                  {
                     F_FLAG = 1;
                  }
                  if (Dst.u32 & 0x80000000)
                  {
                     Dst.u32 = (Dst.u32 ^ 0xFFFFFFFF) + 1;
                  }
               }
               break;
            }
         }
         break;

         case COM:
         {
            Dst.u32 = ~Src.u32;
         }
         break;

         case IBIT:
         {
            F_FLAG = TEST(Dst.u32 & Src.u32);
            Dst.u32 ^= Src.u32;
         }
         break;

         case ADDP:
         {
            uint32_t carry = C_FLAG;
            Dst.u32 = bcd_add(Dst.u32, Src.u32, Data.Info.Op[0].Size, &carry);
            C_FLAG = TEST(carry);
            F_FLAG = 0;
         }
         break;
 
         // FORMAT 7

         case MOVM:
         {
            //temp = GetDisplacement(&Data) + Data.Info.Op[0].Size;                      // disp of 0 means move 1 byte
            uint32_t temp = (GetDisplacement(&Data) & ~(Data.Info.Op[0].Size - 1)) + Data.Info.Op[0].Size;
            while (temp)
            {
               temp2 = read_x8(Src.u32);
               Src.u32++;
               write_x8(Dst.u32, temp2);
               Dst.u32++;
               temp--;
            }

            goto skip_write;
         }
         // No break due to goto

         case CMPM:
         {
            uint32_t temp4    = Data.Info.Op[0].Size;                                 // disp of 0 means move 1 byte/word/dword
            temp3 = (GetDisplacement(&Data) / temp4) + 1;

            //PiTRACE("CMP Size = %u Count = %u\n", temp4, temp3);
            while (temp3--)
            {
               uint32_t temp = read_n(Src.u32, temp4);
               temp2 = read_n(Dst.u32, temp4);
 
               if (CompareCommon(temp, temp2) == 0)
               {
                  break;
               }

               Src.u32 += temp4;
               Dst.u32 += temp4;
            }

            goto skip_write;
         }
         // No break due to goto

         case INSS:
         {
            uint32_t c;

            temp3 = Consume_x8(&Data);            // Read the immediate offset (3 bits) / length - 1 (5 bits) from the instruction

            // The field can be upto 32 bits, and is independent of the opcode i bits
            for (c = 0; c <= (temp3 & 0x1F); c++)
            {
               Dst.u32 &= ~(BIT((c + (temp3 >> 5)) & 31));
               if (Src.u32 & BIT(c))
               {
                  Dst.u32 |= BIT((c + (temp3 >> 5)) & 31);
               }
            }
         }
         break;

         case EXTS:
         {
            uint32_t c;
            uint32_t temp4 = 1;

            if (gentype[0] == TOS)
            {
               PiWARN("EXTS with base==TOS is not yet implemented\n");
               goto skip_write; // with next instruction
            }

            // Read the immediate offset (3 bits) / length - 1 (5 bits) from the instruction
            temp3 = Consume_x8(&Data);
            temp2 = 0;
            Src.u32 >>= (temp3 >> 5); // Shift by offset
            temp3 &= 0x1F; // Mask off the lower 5 Bits which are number of bits to extract

            for (c = 0; c <= temp3; c++)
            {
               if (Src.u32 & temp4) // Copy the ones
               {
                  temp2 |= temp4;
               }

               temp4 <<= 1;
            }
            Dst.u32 = temp2;
         }
         break;

         case MOVXiW:
         {
            SIGN_EXTEND(Data.Info.Op[0].Size, Src.u32); // Editor need the useless semicolon
            Dst.u32 = Src.u32;
         }
         break;

         case MOVZiW:
         {
            Dst.u32 = Src.u32;
         }
         break;

         case MOVZiD:
         {
            Dst.u32 = Src.u32;
         }
         break;

         case MOVXiD:
         {
            SIGN_EXTEND(Data.Info.Op[0].Size, Src.u32); // Editor need the useless semicolon
            Dst.u32 = Src.u32;
         }
         break;

         case MUL:
         {
            Dst.u32 *= Src.u32;
         }
         break;

         case MEI:
         {
            Dst64.u64 = Dst.u32;
            Dst64.u64 *= Src.u32;
            // Handle the writing to the upper half of dst locally here
            handle_mei_dei_upper_write(Dst64.u64);
            // Allow fall through write logic to write the lower half of dst
            Dst.u32 = (uint32_t) Dst64.u64;
         }
         break;

         case DEI:
         {
            int size = Data.Info.Op[0].Size << 3;                      // 8, 16  or 32 
            if (Src.u32 == 0)
            {
               SET_TRAP(DivideByZero);
               goto skip_write;
            }

            switch (Data.Info.Op[0].Size)
            {
               case sz8:
                  Dst64.u64 = ((Dst64.u64 >> 24) & 0xFF00) | (Dst64.u64 & 0xFF);
                  break;

               case sz16:
                  Dst64.u64 = ((Dst64.u64 >> 16) & 0xFFFF0000) | (Dst64.u64 & 0xFFFF);
                  break;
            }
            // PiTRACE("Dst.u32 = %08x\n", Dst.u32);
            // PiTRACE("temp64.u64 = %016" PRIu64 "\n", temp64.u64);
            Dst64.u64 = ((Dst64.u64 / Src.u32) << size) | (Dst64.u64 % Src.u32);
            //PiTRACE("result = %016" PRIu64 "\n", temp64.u64);
            // Handle the writing to the upper half of dst locally here
            handle_mei_dei_upper_write(Dst64.u64);
            // Allow fallthrough write logic to write the lower half of dst
            Dst.u32 = (uint32_t) Dst64.u64;
            Data.Info.Op[1].Size = Data.Info.Op[0].Size;
         }
         break;

         case QUO:
         {
            if (Src.u32 == 0)
            {
               SET_TRAP(DivideByZero);
               goto skip_write;
            }

            switch (Data.Info.Op[0].Size)
            {
               case sz8:
                  Dst.u32 = (int8_t) Dst.u32 / (int8_t) Src.u32;
               break;

               case sz16:
                  Dst.u32 = (int16_t) Dst.u32 / (int16_t) Src.u32;
               break;

               case sz32:
                  Dst.u32 = (int32_t) Dst.u32 / (int32_t) Src.u32;
               break;
            }
         }
         break;

         case REM:
         {
            if (Src.u32 == 0)
            {
               SET_TRAP(DivideByZero);
               goto skip_write;
            }

            switch (Data.Info.Op[0].Size)
            {
               case sz8:
                  Dst.u32 = (int8_t) Dst.u32 % (int8_t) Src.u32;
               break;

               case sz16:
                  Dst.u32 = (int16_t) Dst.u32 % (int16_t) Src.u32;
               break;

               case sz32:
                  Dst.u32 = (int32_t) Dst.u32 % (int32_t) Src.u32;
               break;
            }
         }
         break;

         case MOD:
         {
            if (Src.u32 == 0)
            {
               SET_TRAP(DivideByZero);
               goto skip_write;
            }

            Dst.u32 = mod_operator(Dst.u32, Src.u32);
         }
         break;

         case DIV:
         {
            if (Src.u32 == 0)
            {
               SET_TRAP(DivideByZero);
               goto skip_write;
            }

            Dst.u32 = div_operator(Dst.u32, Src.u32);
         }
         break;

         // Format 8
 
         case EXT:
         {
            int32_t c;
            int32_t  Offset = r[(Data.OpCode >> 11) & 7];
            Disp = GetDisplacement(&Data);
            uint32_t StartBit;

            if (Disp < 1 || Disp > 32)
            {
               PiWARN("EXT with length %08"PRIx32" is undefined\n", Disp);
               goto skip_write; // with next instruction
            }

            if (gentype[0] == TOS)
            {
               // base is TOS
               //
               // This case is complicated because:
               //
               // 1. We need to avoid modifying the stack pointer.
               //
               // 2. We potentially need to take account of an offset.
               //
               PiWARN("EXT with base==TOS is not yet implemented; offset = %"PRId32"\n", Offset);
               goto skip_write; // with next instruction
            }
            else if (gentype[0] == Register)
            {
               // base is a register
               StartBit = ((uint32_t) Offset) & 31;
            }
            else
            {
               // base is memory
               genaddr[0] += OffsetDiv8(Offset);
               StartBit = ((uint32_t) Offset) & 7;
            }

            Data.Info.Op[0].Size = sz32;

            Dst.u32 = 0;
            for (c = 0; (c < Disp) && (c + StartBit < 32); c++)
            {
               if (Src.s32 & BIT(c + StartBit))
               {
                  Dst.u32 |= BIT(c);
               }
            }
         }
         break;

         case CVTP:
         {
            int32_t Offset = r[(Data.OpCode >> 11) & 7];
            Dst.u32 = (Src.u32 * 8) + Offset;
            Data.Info.Op[1].Size = sz32;
         }
         break;

         case INS:
         {
            int32_t c;
            int32_t  Offset = r[(Data.OpCode >> 11) & 7];
            Disp = GetDisplacement(&Data);
            uint32_t StartBit;

            if (Disp < 1 || Disp > 32)
            {
               PiWARN("INS with length %08"PRIx32" is undefined\n", Disp);
               goto skip_write; // with next instruction
            }

            if (gentype[1] == TOS)
            {
               // base is TOS
               //
               // This case is complicated because:
               //
               // 1. We need to avoid modifying the stack pointer,
               // which might not be an issue as we read then write.
               //
               // 2. We potentially need to take account of an offset. This
               // is harder as our current TOS read/write doesn't allow
               // for an offset. It's also not clear what this means.
               //
               PiWARN("INS with base==TOS is not yet implemented; offset = %"PRId32"\n", Offset);
               goto skip_write; // with next instruction
            }
            else if (gentype[1] == Register)
            {
               // base is a register
               StartBit = ((uint32_t) Offset) & 31;
            }
            else
            {
               // base is memory
               genaddr[1] += OffsetDiv8(Offset);
               StartBit = ((uint32_t) Offset) & 7;
            }

            // The field can be upto 32 bits, and is independent of the opcode i bits
            Data.Info.Op[1].Size = sz32;
            for (c = 0; (c < Disp) && (c + StartBit < 32); c++)
            {
               if (Src.u32 & BIT(c))
               {
                  Dst.u32 |= BIT(c + StartBit);
               }
               else
               {
                  Dst.u32 &= ~(BIT(c + StartBit));
               }
            }
         }
         break;

         case CHECK:
         {
            uint32_t temp;

            switch (Data.Info.Op[0].Size)
            {
               case sz8:
               {
                  temp = read_x8(Src.u32);
                  temp2 = read_x8(Src.u32 + sz8);
               }
               break;

               case sz16:
               {
                  temp = read_x16(Src.u32);
                  temp2 = read_x16(Src.u32 + sz16);
               }
               break;

               default:
               case sz32:
               {
                  temp = read_x32(Src.u32);
                  temp2 = read_x32(Src.u32 + sz32);
               }
               break;
            }

            //PiTRACE("Reg = %u Bounds [%u - %u] Index = %u", 0, temp, temp2, temp3);

            if ((temp >= Dst.u32) && (Dst.u32 >= temp2))
            {
               r[(Data.OpCode >> 11) & 7] = Dst.u32 - temp2;
               F_FLAG = 0;
            }
            else
            {
               F_FLAG = 1;
            }

            goto skip_write;
         }
         // No break due to goto

         case INDEX:
         {
            // r0, r1, r2
            // 5, 7, 0x13 (19)
            // accum = accum * (length+1) + index

            uint32_t temp = r[(Data.OpCode >> 11) & 7];  // Accum
            Src.u32 += 1;                       // (length+1)

            r[(Data.OpCode >> 11) & 7] = (temp * Src.u32) + Dst.u32;
            goto skip_write;
         }
         // No break due to goto

         case FFS:
         {
            uint32_t numbits = Data.Info.Op[0].Size << 3;          // number of bits: 8, 16 or 32
            // Src.u32 is the variable size operand being scanned
            // Dst.u32 is offset and is always 8 bits (also the result)

            // find the first set bit, starting at offset
            for (; Dst.u32 < numbits && !(Src.u32 & BIT(Dst.u32)); Dst.u32++)
            {
               continue;                  // No Body!
            }

            if (Dst.u32 < numbits)
            {
               // a set bit was found, return it in the offset operand
               F_FLAG = 0;
            }
            else
            {
               // no set bit was found, return 0 in the offset operand
               F_FLAG = 1;
               Dst.u32 = 0;
            }
         }
         break;

         // Format 9
         case MOVif:
         {
            if (Data.Info.Op[1].Size == sz64)
            {
               Dst64.f64 = (double) Src.s32;
            }
            else
            {
               Dst.f32 = (float) Src.s32;
            }
         }
         break;

         case LFSR:
         {
            FSR = Src.u32;
            goto skip_write;
         }
         // No break due to goto

         case MOVLF:
         {
            Dst.f32 = (float) Src64.f64;
         }
         break;

         case MOVFL:
         {
            Dst64.f64 = (double) Src.f32;
         }
         break;

         case ROUND:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst.u32 = (int32_t) round(Src64.f64);
            }
            else
            {
               Dst.u32 = (int32_t) roundf(Src.f32);
            }
         }
         break;

         case TRUNC:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst.s32 = (int32_t) Src64.f64;
            }
            else
            {
               Dst.s32 = (int32_t) Src.f32;
            }
         }
         break;

         case SFSR:
         {
            Dst.u32 = FSR;
         }
         break;

         case FLOOR:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst.s32 = (int32_t) floor(Src64.f64);
            }
            else
            {
               Dst.s32 = (int32_t) floorf(Src.f32);
            }
         }
         break;
 
         // Format 11
         case ADDf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 += Src64.f64;
            }
            else
            {
               Dst.f32 += Src.f32;
            }
         }
         break;

         case MOVf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.u64 = Src64.u64;
            }
            else
            {
               Dst.u32 = Src.u32;
            }
         }
         break;

         case CMPf:
         {
            L_FLAG = 0;

            if (Data.Info.Op[0].Size == sz64)
            {
               Z_FLAG = TEST(Src64.f64 == Dst64.f64);
               N_FLAG = TEST(Src64.f64 >  Dst64.f64);
            }
            else
            {
               Z_FLAG = TEST(Src.f32 == Dst.f32);
               N_FLAG = TEST(Src.f32 >  Dst.f32);
            }
            goto skip_write;
         }
         // No break due to goto
 
         case SUBf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 -= Src64.f64;
            }
            else
            {
               Dst.f32 -= Src.f32;
            }
         }
         break;

         case NEGf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 = -Src64.f64;
            }
            else
            {
               Dst.f32 = -Src.f32;
            }
         }
         break;

         case DIVf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 /= Src64.f64;
            }
            else
            {
               Dst.f32 /= Src.f32;
            }
         }
         break;

         case MULf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 *= Src64.f64;
            }
            else
            {
               Dst.f32 *= Src.f32;
            }
         }
         break;

         case ABSf:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 = fabs(Src64.f64);
            }
            else
            {
               Dst.f32  = fabsf(Src.f32);
            }
         }
         break;
  
         case POLY:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               FR.fr64[0] = (FR.fr64[0] * Src64.f64) + Dst64.f64;
            }
            else
            {
               FR.fr32[0] = (FR.fr32[0] * Src.f32) + Dst.f32;
            }

            goto skip_write;
         }
         // No break due to goto

         case DOT:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               FR.fr64[0] += (Src64.f64 * Dst64.f64);
            }
            else
            {
               FR.fr32[0] += (Src.f32 * Dst.f32);
            }

            goto skip_write;
         }
         // No break due to goto
         
         case SCALB:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 *= pow(2, trunc(Src64.f64));
            }
            else
            {
               Dst.f32 *= powf(2, truncf(Src.f32));
            }
         }
         break;
         
         case LOGB:
         {
            if (Data.Info.Op[0].Size == sz64)
            {
               Dst64.f64 = log2(Src64.f64);
            }
            else
            {
               Dst.f32 = log2f(Src.f32);
            }
         }
         break;
        
         default:
         {
            if (Data.Function < TRAP)
            {
               SET_TRAP(UnknownInstruction);
            }
            else
            { 
               SET_TRAP(UnknownFormat);         // Probably already set but belt and braces here!
            }

            goto skip_write;
         }
         // No break due to goto
      }

      {
         uint32_t WriteIndex = ((Data.Function >> 4) == Format2) ? 0 : 1;
         uint32_t WriteSize = Data.Info.Op[WriteIndex].Size;
         if (WriteSize && (WriteSize <= sz64))
         {
            switch (gentype[WriteIndex])
            {
               case Memory:
               {
                  switch (WriteSize)
                  {
                     case sz8:   write_x8( genaddr[WriteIndex], Dst.u32);  break;
                     case sz16:  write_x16(genaddr[WriteIndex], Dst.u32);  break;
                     case sz32:  write_x32(genaddr[WriteIndex], Dst.u32);  break;
                     case sz64:  write_x64(genaddr[WriteIndex], Dst64.u64);  break;
                  }
               }
               break;
            
               case Register:
               {
                  switch (WriteSize)
                  {
                     case sz8:   *((uint8_t*)   genaddr[WriteIndex]) = Dst.u32;  break;
                     case sz16:  *((uint16_t*)  genaddr[WriteIndex]) = Dst.u32;  break;
                     case sz32:  *((uint32_t*)  genaddr[WriteIndex]) = Dst.u32;  break;
                     case sz64:  *((uint64_t*)  genaddr[WriteIndex]) = Dst64.u64;  break;
                  }
               }
               break;

               case TOS:
               {
                  if (WriteSize == sz64)
                  {
                     PushArbitary(Dst64.u64, WriteSize);
                  }
                  else
                  {
                     PushArbitary(Dst.u32, WriteSize);
                  }
               }
               break;

               case OpImmediate:
               {
                  SET_TRAP(IllegalWritingImmediate);
               }
               break;
            }
         }
      }

      skip_write:

      if (TrapFlags)
      {
         uint32_t Index = 1;
         uint32_t Pattern = TrapFlags;

         while (Pattern)
         {
            if (Pattern & 1)
            {
               switch (Index)
               {
                  case BreakPointHit:
                  {
                     BreakPoint(&Data);

                     if (Data.CurrentAddress == PR.BPC)
                     {
                        SET_TRAP(BreakPointTrap);
                     }
                  }
                  break;

                  case INSTRUCTION_PROFILING:
                  {
                     IP[Data.StartAddress]++;
                  }
                  break;

                  case PROFILING:
                  {
                     ProfileAdd(&Data);
                  }
                  break;

                  case SHOW_INSTRUCTIONS:
                  {
#ifndef OLD_INSTRUCTION_TRACE
                     DecodeData This;
                     This.CurrentAddress = Data.StartAddress;
                     Decode(&This);
                     ShowInstruction(&This);
#endif
                  }
                  break;

                  case SHOW_WRITES:
                  {
                     uint32_t Index = ((Data.Function >> 4) == Format2) ? 0 : 1;
                                   
                     if ((Data.Info.Op[Index].Class == write >> 8) || (Data.Info.Op[Index].Class == (rmw >> 8)))
                     {
                        uint32_t Size = Data.Info.Op[Index].Size;
                        ShowRegisterWrite(Data.Regs[Index], (Size <= sz32) ? Truncate(Dst.u32, Size) : Dst64.u64);
                     }
                  }
                  break;

                  default:
                  {
                     HandleTrap();
                     //CLEAR_TRAP();
                  }
                  break;
               }
            }  

            Pattern >>= 1;
            Index <<= 1;
         }

         CLEAR_TRAP();
      }
   }
}
