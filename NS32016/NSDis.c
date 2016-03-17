#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "Decode.h"
#include "32016.h"
#include "mem32016.h"
#include "Profile.h"
#include "Trap.h"


#define HEX24 "x'%06" PRIX32
#define HEX32 "x'%" PRIX32

uint32_t OpCount = 0;

const char LPRLookUp[16][20] = 
{
   "UPSR",
   "DCR",
   "BPC",
   "DSR",
   "CAR",
   "{0101}",
   "{0110}",
   "{0111}",
   "FP",
   "SP",
   "SB",
   "USP",
   "CFG",
   "PSR",
   "INTBASE",
   "MOD"
};

#ifdef INSTRUCTION_PROFILING
uint32_t IP[MEG16];
#endif

void AddStringFlags(uint32_t opcode)
{
   if (opcode & (BIT(Backwards) | BIT(UntilMatch) | BIT(WhileMatch)))
   {
      PiTRACE("[");
      if (opcode & BIT(Backwards))
      {
         PiTRACE("B");
      }

      uint32_t Options = (opcode >> 17) & 3;
      if (Options == 1) // While match
      {
         PiTRACE("W");
      }
      else if (Options == 3)
      {
         PiTRACE("U");
      }

      PiTRACE("]");
   }
}

void AddCfgFLags(DecodeData* This)
{
   PiTRACE("[");
   if (This->OpCode & BIT(15))   PiTRACE("I");
   if (This->OpCode & BIT(16))   PiTRACE("F");
   if (This->OpCode & BIT(17))   PiTRACE("M");
   if (This->OpCode & BIT(18))   PiTRACE("C");
   PiTRACE("]");
}

const char InstuctionText[InstructionCount][8] =
{
   // FORMAT 0
   "BEQ", "BNE", "BCS", "BCC", "BH", "BLS", "BGT", "BLE", "BFS", "BFC", "BLO", "BHS", "BLT", "BGE", "BR", "BN",

   // FORMAT 1	
   "BSR", "RET", "CXP", "RXP", "RETT", "RETI", "SAVE", "RESTORE", "ENTER", "EXIT", "NOP", "WAIT", "DIA", "FLAG", "SVC", "BPT",

   // FORMAT 2
   "ADDQ", "CMPQ", "SPR", "Scond", "ACB", "MOVQ", "LPR", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 3
   "CXPD", "TRAP", "BICPSR", "TRAP", "JUMP", "TRAP", "BISPSR", "TRAP", "TRAP", "TRAP", "ADJSP", "TRAP", "JSR", "TRAP", "CASE", "TRAP",

   // FORMAT 4
   "ADD", "CMP", "BIC", "TRAP", "ADDC", "MOV", "OR", "TRAP", "SUB", "ADDR", "AND", "TRAP", "SUBC", "TBIT", "XOR", "TRAP",

   // FORMAT 5
   "MOVS", "CMPS", "SETCFG", "SKPS", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 6
   "ROT", "ASH", "CBIT", "CBITI", "TRAP", "LSH", "SBIT", "SBITI", "NEG", "NOT", "TRAP", "SUBP", "ABS", "COM", "IBIT", "ADDP",

   // FORMAT 7
   "MOVM", "CMPM", "INSS", "EXTS", "MOVX", "MOVZ", "MOVZ", "MOVX", "MUL", "MEI", "Trap", "DEI", "QUO", "REM", "MOD", "DIV", "TRAP"

   // FORMAT 8
   "EXT", "CVTP", "INS", "CHECK", "INDEX", "FFS", "MOVUS", "MOVSU", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 9
   "MOV", "LFSR", "MOVLF", "MOVFL", "ROUND", "TRUNC", "SFSR", "FLOOR", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 10
   "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 11
   "ADD", "MOV", "CMP", "TRAP", "SUB", "NEG", "TRAP", "TRAP", "DIV", "TRAP", "TRAP", "TRAP", "MUL", "ABS", "TRAP", "TRAP",

   // FORMAT 12
   "TRAP", "TRAP", "POLY", "DOT", "SCALB", "LOGB", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 13
   "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // FORMAT 14
   "RDVAL", "WRVAL", "LMR", "SMR", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "CINV", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP", "TRAP",

   // Illegal
   "TRAP"
};


int64_t GetImmediate(DecodeData* This, uint32_t Index)
{
   int64_t Result;
   MultiReg Temp;
   uint32_t Size = This->Info.Op[Index].Size;

   Temp.u32 = SWAP32(read_x32(This->CurrentAddress));

   if (Size == sz8)
   {
      Result = Temp.u8;
   }
   else if (Size == sz16)
   {
      Result = Temp.u16;
   }
   else if (Size == sz32)
   {
      Result = Temp.u32;
   }
   else
   {
      Result = (((int64_t) Temp.u32) << 32) | SWAP32(read_x32(This->CurrentAddress + 4));
   }

   This->CurrentAddress += Size;
   return Result;
}

void GetOperandText(DecodeData* This, RegLKU Pattern, uint32_t Index)
{
   const char RegLetter[] = "RDF*****";

   if (Pattern.OpType < 8)
   {
      PiTRACE("%c%0" PRId32, RegLetter[Pattern.RegType], Pattern.OpType);
   }
   else if (Pattern.OpType < 16)
   {
      int32_t d = GetDisplacement(&This->CurrentAddress);
      PiTRACE("%0" PRId32 "(R%u)", d, (Pattern.Whole & 7));
   }
   else
   {
      switch (Pattern.Whole & 0x1F)
      {
         case FrameRelative:
         {
            int32_t d1 = GetDisplacement(&This->CurrentAddress);
            int32_t d2 = GetDisplacement(&This->CurrentAddress);
            PiTRACE("%" PRId32 "(%" PRId32 "(FP))", d2, d1);
         }
         break;

         case StackRelative:
         {
            int32_t d1 = GetDisplacement(&This->CurrentAddress);
            int32_t d2 = GetDisplacement(&This->CurrentAddress);
            PiTRACE("%" PRId32 "(%" PRId32 "(SP))", d2, d1);
         }
         break;

         case StaticRelative:
         {
            int32_t d1 = GetDisplacement(&This->CurrentAddress);
            int32_t d2 = GetDisplacement(&This->CurrentAddress);
            PiTRACE("%" PRId32 "(%" PRId32 "(SB))", d2 , d1);
         }
         break;

         case IllegalOperand:
         {
            PiTRACE("(reserved)");
         }
         break;

         case Immediate:
         {
            Temp64Type Value;
            Value.s64 = GetImmediate(This, Index);
            if (This->Info.Op[Index].Class & 0x80)
            {
               if (This->Info.Op[Index].Size == sz32)
               {
                  Temp32Type Value32;
                  Value32.u32 = (uint32_t) Value.u64;
                  PiTRACE("%g", Value32.f32);
               }
               else
               {
                  PiTRACE("%g", Value.f64);
               }
            }
            else
            {
               PiTRACE(HEX32, Value);
            }
         }
         break;

         case Absolute:
         {
            int32_t d = GetDisplacement(&This->CurrentAddress);
            PiTRACE("@" HEX32, d);
         }
         break;

         case External:
         {
            int32_t d1 = GetDisplacement(&This->CurrentAddress);
            int32_t d2 = GetDisplacement(&This->CurrentAddress);
            PiTRACE("EXT(" HEX32 ")+" HEX32, d1, d2);
         }
         break;

         case TopOfStack:
         {
            PiTRACE("TOS");
         }
         break;

         case FpRelative:
         {
            int32_t d = GetDisplacement(&This->CurrentAddress);
            PiTRACE("%" PRId32 "(FP)", d);
         }
         break;

         case SpRelative:
         {
            int32_t d = GetDisplacement(&This->CurrentAddress);
            PiTRACE("%" PRId32 "(SP)", d);
         }
         break;

         case SbRelative:
         {
            int32_t d = GetDisplacement(&This->CurrentAddress);
            PiTRACE("%" PRId32 "(SB)", d);
         }
         break;

         case PcRelative:
         {
            int32_t d = GetDisplacement(&This->CurrentAddress);
#if 1
            PiTRACE("* + %" PRId32, d);
            
#else            
            PiTRACE("&06%" PRIX32 "[PC]", This->StartAddress + d);
#endif
         }
         break;

         case EaPlusRn:
         case EaPlus2Rn:
         case EaPlus4Rn:
         case EaPlus8Rn:
         {
            const char SizeLookup[] = "BWDQ";
            RegLKU NewPattern;
            NewPattern.Whole = Pattern.Whole >> 11;
            GetOperandText(This, NewPattern, Index);   // Recurse
            PiTRACE("[R%" PRId16 ":%c]", ((Pattern.Whole >> 8) & 3), SizeLookup[Pattern.Whole & 3]);
         }
         break;
      }
   }
}

void RegLookUp(DecodeData* This)
{
   // printf("RegLookUp(%06" PRIX32 ", %06" PRIX32 ")\n", pc, (*pPC));
   uint32_t Index;

   for (Index = 0; Index < 2; Index++)
   {
      if (This->Regs[Index].Whole < 0xFFFF)
      {
         if (Index == 1)
         {
            if (This->Regs[0].Whole < 0xFFFF)
            {
               PiTRACE(",");
            }
         }

         GetOperandText(This, This->Regs[Index], Index);
      }
   }
}

void ShowRegs(uint8_t Pattern, uint8_t Reverse)
{
   uint32_t Count;
   uint32_t First = 1;

   PiTRACE("[");

   for (Count = 0; Count < 8; Count++)
   {
      if (Pattern & BIT(Count))
      {
         if (First == 0)
         {
            PiTRACE(",");
         }

         if (Reverse)
         {
            PiTRACE("R%" PRIu32, Count ^ 7);
         }
         else
         {
            PiTRACE("R%" PRIu32, Count);
         }
         First = 0;
      }
   }

   PiTRACE("]");
}

const char PostFixLk[] = "BWTD????";
const char PostFltLk[] = "123F567L";
const char TenSpaces[] = "          ";

char GetText(OpDetail Info)
{
   uint32_t Size = Info.Size;

   if (Size--)
   {
      if (Size < 8)
      {
         if (Info.Class & 0x80)
         {
            return PostFltLk[Size];
         }
      }

      return PostFixLk[Size];
   }

   return '?';
}

void AddInstructionText(DecodeData* This)
{
   if (This->Function < InstructionCount)
   {
      uint32_t OperandSize = This->Info.Op[1].Size;
      char Str[80];

      if (This->Function == Scond)
      {
         uint32_t Condition = ((This->OpCode >> 7) & 0x0F);
         sprintf(Str, "S%s", &InstuctionText[Condition][1]);             // Offset by 1 to lose the 'B'
      }
      else
      {
         sprintf(Str, "%s", InstuctionText[This->Function]);

         switch (This->Function)
         {
            case MOVif:
            case ROUND:
            case TRUNC:
            case FLOOR:
            {
               sprintf(Str + strlen(Str), "%c%c", GetText(This->Info.Op[0]), GetText(This->Info.Op[1]));
            }
            // Fall Through

            case SFSR:
            case LFSR:
            {
               OperandSize = 0;
            }
            break;
         }

         if ((This->OpCode & 0x80FF) == 0x800E)
         {
            sprintf(Str + strlen(Str), "T");                 // Offset by 1 to loose the 'B'
         }
         else
         {
            if (OperandSize)
            {
               sprintf(Str + strlen(Str), "%c", GetText(This->Info.Op[0]));                 // Offset by 1 to loose the 'B'
            }
         }

         switch (This->Function)
         {
            case MOVXiW:
            case MOVZiW:
            {
               sprintf(Str + strlen(Str), "W");
            }
            break;

            case MOVZiD:
            case MOVXiD:
            {
               sprintf(Str + strlen(Str), "D");
            }
            break;
         }
      }

      size_t Len = strlen(Str);
      if (Len < (sizeof(TenSpaces) - 1))
      {
         PiTRACE("%s%s", Str, &TenSpaces[Len]);
      }
   }
}

void AddASCII(opcode, Format)
{
   if (Format < sizeof(FormatSizes))
   {
      uint32_t Len = FormatSizes[Format];
      uint32_t Count;

      for (Count = 0; Count < 4; Count++)
      {
         if (Count < Len)
         { 
            uint8_t Data = opcode & 0xFF;
            PiTRACE("%c", (Data < 0x20) ? '.' : Data);
            opcode >>= 8;
         }
         else
         {
            PiTRACE(" ");
         }
      }
   }
}

#ifdef SHOW_INSTRUCTIONS
void ShowInstruction(DecodeData* This)
{
   static uint32_t old_pc = 0xFFFFFFFF;

   if (This->StartAddress < (IO_BASE - 64))                     // The code will not work near the IO Space as it will have side effects
   {
      if (This->StartAddress == old_pc)
      {
         switch (This->Function)
         {
            case MOVS:
            case WAIT:                                             // Wait for interrupt then continue execution
            case DIA:                                              // Wait for interrupt and in theory never resume execution (stack manipulation would get round This)
            case CMPS:
            case SKPS:
            {
               return;                                            // This is just another iteration of an interruptable instructions
            }
            // No break due to return
         }
      }
      
      old_pc = This->StartAddress;
         
#ifdef WIN32
      if (OpCount > 25000)
      {
         PiTRACE("25000 Traces done!\n");
         exit(1);
      }
      OpCount++;
      //PiTRACE("#%08"PRIu32" ", OpCount);
#endif
        
      PiTRACE("&%06" PRIX32 " ", This->StartAddress);
      PiTRACE("[%08" PRIX32 "] ", This->OpCode);
      uint32_t Format = This->Function >> 4;
      PiTRACE("F%02" PRIu32 " ", Format);
      //AddASCII(opcode, Format);

      if (This->Function < InstructionCount)
      {
         AddInstructionText(This);

         switch (This->Function)
         {
            case ADDQ:
            case CMPQ:
            case ACB:
            case MOVQ:
            {
               int32_t Value = (This->OpCode >> 7) & 0xF;
               NIBBLE_EXTEND(Value);
               PiTRACE("%" PRId32 ",", Value);
            }
            break;

            case LPR:
            case SPR:
            {
               int32_t Value = (This->OpCode >> 7) & 0xF;
               PiTRACE("%s", LPRLookUp[Value]);
               if (This->Function == LPR)
               {
                  PiTRACE(",");
               }
            }
            break;
         }


         RegLookUp(This);

         if ((This->Function <= BN) || (This->Function == BSR))
         {
            int32_t d = GetDisplacement(&This->CurrentAddress);
            PiTRACE("&%06"PRIX32" ", This->StartAddress + d);
         }

         switch (This->Function)
         {
            case SAVE:
            {
               ShowRegs(read_x8(This->CurrentAddress), 0);    //Access directly we do not want tube reads!
               This->CurrentAddress++;
            }
            break;

            case RESTORE:
            {
               ShowRegs(read_x8(This->CurrentAddress), 1);    //Access directly we do not want tube reads!
               This->CurrentAddress++;
            }
            break;

            case EXIT:
            {
               ShowRegs(read_x8(This->CurrentAddress), 1);    //Access directly we do not want tube reads!
               This->CurrentAddress++;
            }
            break;
  
            case ENTER:
            {
               ShowRegs(read_x8(This->CurrentAddress), 0);    //Access directly we do not want tube reads!
               This->CurrentAddress;
               int32_t d = GetDisplacement(&This->CurrentAddress);
               PiTRACE(" " HEX32 "", d);
            }
            break;

            case RET:               
            case CXP:
            case RXP:
            {
               int32_t d = GetDisplacement(&This->CurrentAddress);
               PiTRACE(" " HEX32 "", d);
            }
            break;

            case ACB:
            {
               int32_t d = GetDisplacement(&This->CurrentAddress);
               PiTRACE("PC x+'%" PRId32 "", d);
            }
            break;

            case MOVM:
            case CMPM:
            {
               int32_t d = GetDisplacement(&This->CurrentAddress);
               PiTRACE(",%" PRId32, (d / This->Info.Op[1].Size + 1));
            }
            break;

            case EXT:
            case INS:
            {
               int32_t d = GetDisplacement(&This->CurrentAddress);
               PiTRACE(",%" PRId32, d);
            }
            break;

            case MOVS:
            case CMPS:
            case SKPS:
            {
               AddStringFlags(This->OpCode);
            }
            break;

            case SETCFG:
            {
               AddCfgFLags(This);
            }
            break;

            case INSS:
            case EXTS:
            {
               uint8_t Value = read_x8(This->CurrentAddress);
               This->CurrentAddress++;
               PiTRACE(",%" PRIu32 ",%" PRIu32,  Value >> 5, ((Value & 0x1F) + 1));
            }
            break;
         }


         PiTRACE("\n");

#ifdef TEST_SUITE

#if TEST_SUITE == 0
         if ((This->CurrentAddress == 0x1CA9) || (This->CurrentAddress == 0x1CB2))
#else
         if ((This->CurrentAddress == 0x1CA8) || (This->CurrentAddress == 0x1CBD))
#endif
         {

#ifdef INSTRUCTION_PROFILING
            DisassembleUsingITrace(0, 0x10000);
#endif

            n32016_dumpregs("Test Suite Complete!\n");
            exit(1);
         }
#endif

#ifndef TEST_SUITE
         if (OpCount >= 10000)
         {
            n32016_dumpregs("Lots of trace data here!");
         }
#endif
      }

      return;
   }

   //PiTRACE("PC is :%08"PRIX32" ?????\n", *pPC);
}
#endif

void ShowRegisterWrite(RegLKU RegIn, uint32_t Value)
{
   if (RegIn.OpType < 8)
   {
#ifdef SHOW_REG_WRITES
      if (RegIn.RegType == Integer)
      {
         PiTRACE(" R%u = %"PRIX32"\n", RegIn.OpType, Value);
      }
#endif

#ifdef TEST_SUITE
      if (RegIn.OpType == 7)
      {
         PiTRACE("*** TEST = %u\n", Value);

#if 0
         if (Value == 137)
         {
            PiTRACE("*** BREAKPOINT\n");
         }
#endif
      }
#endif
   }
}

static void getgen(DecodeData* This, int gen, int c)
{
   gen &= 0x1F;
   This->Regs[c].Whole = gen;

   if (gen >= EaPlusRn)
   {
      This->Regs[c].Whole |= read_x8(This->CurrentAddress) << 8;
      This->CurrentAddress++;

      if ((This->Regs[c].Whole & 0xF800) == (Immediate << 11))
      {
         SET_TRAP(IllegalImmediate);
      }

      if ((This->Regs[c].Whole & 0xF800) >= (EaPlusRn << 11))
      {
         SET_TRAP(IllegalDoubleIndexing);
      }
   }
}

void SetSize(DecodeData* This, uint32_t Size)
{
   Size &= 3;
   Size++;

   if (This->Info.Op[0].Size == 0)                                      // Only set size if it has not already been set
   {
      This->Info.Op[0].Size = Size;
   }

   if (This->Info.Op[1].Size == 0)
   {
      This->Info.Op[1].Size = Size;
   }
}

void Decode(DecodeData* This)
{
   This->StartAddress    = This->CurrentAddress;
   uint32_t   opcode    =
   This->OpCode          = read_x32(This->StartAddress);
   This->Function        = FunctionLookup[This->OpCode & 0xFF];
   This->Info.Whole      = OpFlags[This->Function];
   uint32_t Format      = This->Function >> 4;

   if (Format < (FormatCount + 1))
   {
      This->StartAddress += FormatSizes[Format];                                        // Add the basic number of bytes for a particular instruction
   }

   switch (Format)
   {
      case Format0:
      case Format1:
      {
         // Nothing here
      }
      break;

      case Format2:
      {
         SetSize(This, opcode);
         getgen(This, opcode >> 11, 0);
      }
      break;

      case Format3:
      {
         This->Function += ((opcode >> 7) & 0x0F);
         SetSize(This, opcode);
         getgen(This, opcode >> 11, 0);
      }
      break;

      case Format4:
      {
         SetSize(This, opcode);
         getgen(This, opcode >> 11, 0);
         getgen(This, opcode >> 6, 1);
      }
      break;

      case Format5:
      {
         This->Function += ((opcode >> 10) & 0x0F);
         SetSize(This, opcode >> 8);

         if (opcode & BIT(Translation))
         {
            This->Info.Op[0].Size = sz8;
            This->Info.Op[1].Size = sz8;
         }
      }
      break;

      case Format6:
      {
         This->Function += ((opcode >> 10) & 0x0F);
         SetSize(This, opcode >> 8);

         // Ordering important here, as getgen uses Operand Size
         switch (This->Function)
         {
            case ROT:
            case ASH:
            case LSH:
            {
               This->Info.Op[0].Size = sz8;
            }
            break;
         }

         getgen(This, opcode >> 19, 0);
         getgen(This, opcode >> 14, 1);
      }
      break;

      case Format7:
      {
         This->Function += ((opcode >> 10) & 0x0F);
         SetSize(This, opcode >> 8);
         getgen(This, opcode >> 19, 0);
         getgen(This, opcode >> 14, 1);
      }
      break;

      case Format8:
      {
         if (opcode & 0x400)
         {
            if (opcode & 0x80)
            {
               switch (opcode & 0x3CC0)
               {
                  case 0x0C80:
                  {
                     This->Function = MOVUS;
                  }
                  break;

                  case 0x1C80:
                  {
                     This->Function = MOVSU;
                  }
                  break;

                  default:
                  {
                     This->Function = TRAP;
                  }
                  break;
               }
            }
            else
            {
               This->Function = (opcode & 0x40) ? FFS : INDEX;
            }
         }
         else
         {
            This->Function += ((opcode >> 6) & 3);
         }

         SetSize(This, opcode >> 8);

         if (This->Function == CVTP)
         {
            This->Info.Op[0].Size = sz32;
            This->Info.Op[1].Size = sz32;
         }

         getgen(This, opcode >> 19, 0);
         getgen(This, opcode >> 14, 1);
      }
      break;

      case Format9:
      {
         This->Function += ((opcode >> 11) & 0x07);
      }
      break;

      case Format11:
      {
         This->Function += ((opcode >> 10) & 0x0F);
      }
      break;

      case Format14:
      {
         This->Function += ((opcode >> 10) & 0x0F);
      }
      break;

      default:
      {
         SET_TRAP(UnknownFormat);
      }
      break;
   }

   ShowInstruction(This);
}

#ifdef INSTRUCTION_PROFILING
#define BYTE_COUNT 10

void DisassembleUsingITrace(uint32_t Location, uint32_t End)
{
   uint32_t Index;
   uint32_t Address = Location;

   PiTRACE("DisassembleUsingITrace(%06" PRIX32 ", %06" PRIX32 ")\n", Location, End);
   for (Index = Location; Index < End; Index++)
   {
      if (IP[Index])
      {
         if (Address < Index)
         {
            uint32_t Temp;
            uint32_t Break = 0;

            for (Temp = Address; Temp < Index; Temp++, Break++)
            {
               if ((Break % BYTE_COUNT) == 0)
               {
                  PiTRACE("\n.byte %u", read_x8(Temp));
                  continue;
               }

               PiTRACE(",%u", read_x8(Temp));
            }

            PiTRACE("\n");
         }

         PiTRACE("#%06" PRId32 ": ", IP[Index]);
         Address = Index;
         Decode(&Address);
         ShowTraps();
         CLEAR_TRAP();
      }
   }
}
#endif

void Disassemble(uint32_t Location, uint32_t End)
{
   DecodeData This;

   This.CurrentAddress = Location;

   do
   {
      Decode(&This);
      ShowTraps();
      CLEAR_TRAP();
   }
   while (This.CurrentAddress < End);

#ifdef WIN32
   system("pause");
#endif
}