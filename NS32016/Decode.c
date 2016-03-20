// B-em v2.2 by Tom Walker
//32016 parasite processor emulation (not working yet)

// 32106 Decode
// (c) By Simon R. Ellwood

// Decode is the base module that is common to Soft-Core, Live Trace and Dissassembly

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "Decode.h"
#include "32016.h"
#include "Profile.h"
#include "Trap.h"
#include "mem32016.h"

uint8_t FunctionLookup[256];

const uint8_t FormatSizes[FormatCount + 1] =
{
   1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1
};

#define FUNC(FORMAT, OFFSET) (((FORMAT) << 4) + (OFFSET)) 

uint8_t GetFunction(uint8_t FirstByte)
{
   switch (FirstByte & 0x0F)
   {
      case 0x0A:
         return FUNC(Format0, FirstByte >> 4);
      case 0x02:
         return FUNC(Format1, FirstByte >> 4);

      case 0x0C:
      case 0x0D:
      case 0x0F:
      {
         if ((FirstByte & 0x70) != 0x70)
         {
            return FUNC(Format2, (FirstByte >> 4) & 0x07);
         }

         return FUNC(Format3, 0);
      }
      // No break due to return
   }

   if ((FirstByte & 0x03) != 0x02)
   {
      return FUNC(Format4, (FirstByte >> 2) & 0x0F);
   }

   switch (FirstByte)
   {
      case 0x0E:
         return FUNC(Format5, 0); // String instruction
      case 0x4E:
         return FUNC(Format6, 0);
      case 0xCE:
         return FUNC(Format7, 0);
      CASE4(0x2E):
         return FUNC(Format8, 0);
      case 0x3E:
         return FUNC(Format9, 0);
      case 0x7E:
         return FUNC(Format10, 0);
      case 0xBE:
         return FUNC(Format11, 0);
      case 0xFE:
         return FUNC(Format12, 0);
      case 0x9E:
         return FUNC(Format13, 0);
      case 0x1E:
         return FUNC(Format14, 0);
   }

   return FormatBad;
}

void n32016_build_matrix()
{
   uint32_t Index;

   for (Index = 0; Index < 256; Index++)
   {
      FunctionLookup[Index] = GetFunction(Index);
   }
}

void BreakPoint(DecodeData* This)
{
#if 1
#ifndef TEST_SUITE
   // Exec address of Bas32
   if (This.StartAddress == 0x000200)
   {
      printf("Entering Bas32\n");
      ProfileInit();
   }
   // Exec address of Panos
   if (This.StartAddress == 0x000400)
   {
      printf("Entering Panos\n");
      ProfileInit();
   }
   // Address of SVC &11 (OS_EXIT)
   if (This.StartAddress == 0xF007BB)
   {
      n32016_dumpregs("Retuning to Pandora");
      ProfileInit();
   }
#endif
#endif
}

#if 1
#define OP(o1, o2) ((o2) << 16 | (o1))

const uint32_t OpFlags[InstructionCount] =
{
   // Format 0 Branches
   OP(not_used, not_used),      
   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          
   
   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          

   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          

   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          
   OP(not_used, not_used),          

   // Format 1 Special Instructions
   OP(not_used, not_used),   
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 2  
   OP(rmw,      not_used),    // ADDQ    
   OP(read,     not_used),    // CMPQ
   OP(write,    not_used),    // SPR
   OP(write,    not_used),    // Scond

   OP(rmw,      not_used),    // ACB
   OP(write,    not_used),    // MOVQ
   OP(read,     not_used),    // LPR
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 3  
   OP(addr,     not_used),    // CXPD    
   OP(not_used, not_used),
   OP(read,     not_used),    // BICPSR
   OP(not_used, not_used),

   OP(addr,     not_used),    // JUMP
   OP(not_used, not_used),
   OP(read,     not_used),    // BIPSR
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(read,     not_used),    // ADJSP
   OP(not_used, not_used),

   OP(addr,     not_used),    // JSR
   OP(not_used, not_used),
   OP(read,     not_used),    // CASE
   OP(not_used, not_used),

   // Format 4
   OP(read,     rmw),         // ADD
   OP(read,     read),        // CMP
   OP(read,     rmw),         // BIC
   OP(not_used, not_used),

   OP(read,     rmw),         // ADDC
   OP(read,     write),       // MOV
   OP(read,     rmw),         // OR
   OP(not_used, not_used),

   OP(read,     rmw),         // SUB
   OP(addr,     write),       // ADDR
   OP(read,     rmw),         // AND
   OP(not_used, not_used),

   OP(read,     rmw),         // SUBC
   OP(Regaddr,  read),        // TBIT
   OP(read,     rmw),         // XOR
   OP(not_used, not_used),

   // Format 5
   OP(not_used, not_used),    // MOVS
   OP(not_used, not_used),    // CMPS
   OP(not_used, not_used),    // SETCFG
   OP(not_used, not_used),    // SKPS

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 6
   OP(read | sz8,     rmw),               // ROT (Force 0)
   OP(read | sz8,     rmw),               // ASH (Force 0)
   OP(Regaddr,        rmw),               // CBIT
   OP(Regaddr,        rmw),               // CBITI

   OP(not_used,  not_used),
   OP(read |sz8,      rmw),               // LSH (Force 0)
   OP(Regaddr,        rmw),               // SBIT
   OP(Regaddr,        rmw),               // SBITI

   OP(read,     write),                   // NEG
   OP(read,     write),                   // NOT
   OP(not_used, not_used),
   OP(read,     rmw),                     // SUBP

   OP(read,     write),                   // ABS
   OP(read,     write),                   // COM
   OP(Regaddr,  rmw),                     // IBIT
   OP(read,     rmw),                     // ADDP

   // Format 7
   OP(addr,     addr),                    // MOVM
   OP(addr,     addr),                    // CMPM
   OP(EXTRA_BYTE | read,  rmw | sz32),    // INSS
   OP(EXTRA_BYTE | read,  rmw),           // EXTS

   OP(read,     write | sz16),            // MOVXiW
   OP(read,     write | sz16),            // MOVZiW
   OP(read,     write | sz32),            // MOVZiD
   OP(read,     write | sz32),            // MOVXiD 

   OP(read,     rmw),                     // MUL
   OP(read,     rmw),                     // MEI
   OP(not_used, not_used),
   OP(read,     rmw | sz64),              // DEI

   OP(read,     rmw),                     // QUO
   OP(read,     rmw),                     // REM
   OP(read,     rmw),                     // MOD
   OP(read,     rmw),                     // DIV

   // Format 8
   OP(read | sz32, write),                // EXT
   OP(addr | sz32, write | sz32),         // CVTP
   OP(read,        rmw | sz32),           // INS
   OP(addr,        read),                 // CHECK

   OP(read,        read),                 // INDEX
   OP(read,        rmw | sz8),            // FFS
   OP(not_used, not_used),                // MOVUS
   OP(not_used, not_used),                // MOVSU

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
 
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),


   // Format 9
   OP(read, write | FP),                     // MOVif
   OP(read | sz32, not_used),                // LFSR
   OP(read | FP | sz64, write | FP | sz32),  // MOVLF
   OP(read | FP | sz32, write | FP | sz64),  // MOVFL

   OP(read | FP,  write),                    // ROUND
   OP(read | FP,  write),                    // TRUNC
   OP(not_used,   write | sz32),             // SFSR
   OP(read | FP,  write),                    // FLOOR

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 10
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 11
   OP(read | FP, rmw |FP),       // ADDf
   OP(read | FP, write | FP),    // MOVf
   OP(read | FP, read | FP),     // CMPf
   OP(not_used, not_used),

   OP(read | FP, rmw | FP),      // SUBf
   OP(read | FP, write | FP),    // NEGf
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(read | FP, rmw | FP),      // DIVf
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(read | FP, rmw | FP),      // MULf
   OP(read | FP, write | FP),    // ABSf
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 12
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),       // POLY
   OP(not_used, not_used),       // DOT

   OP(not_used, not_used),       // SCALB
   OP(read | FP, write | FP),       // LOGB
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 13
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // Format 14
   OP(not_used, not_used),    // RDVAL
   OP(not_used, not_used),    // WRVAL
   OP(not_used, not_used),    // LMR
   OP(not_used, not_used),    // SMR

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),    // CINV
   OP(not_used, not_used),
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),

   // TRAP
   OP(not_used, not_used)
};
#endif

uint8_t Consume_x8(DecodeData* This)
{
   return read_x8(This->CurrentAddress++);
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

static void getgen(DecodeData* This, int gen, int c)
{
   gen &= 0x1F;
   This->Regs[c].Whole = gen;

   if (gen >= EaPlusRn)
   {
      This->Regs[c].UpperByte = Consume_x8(This);

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


uint32_t Decode(DecodeData* This)
{
   uint32_t Trap           = NoIssue;
   This->StartAddress      = This->CurrentAddress;
   uint32_t OpCode         =
   This->OpCode            = read_x32(This->StartAddress);
   This->Function          = FunctionLookup[This->OpCode & 0xFF];
   uint32_t Format         = This->Function >> 4;

   if (Format < (FormatCount + 1))
   {
      This->CurrentAddress += FormatSizes[Format];                                        // Add the basic number of bytes for a particular instruction
   }

   switch (Format)
   {
      case Format0:
      case Format1:
      {
         This->Info.Whole = OpFlags[This->Function];
         // Nothing here!
      }
      break;

      case Format2:
      {
         This->Info.Whole = OpFlags[This->Function];
         SetSize(This, OpCode);
      }
      break;

      case Format3:
      {
         This->Function += ((OpCode >> 7) & 0x0F);
         This->Info.Whole = OpFlags[This->Function];
         SetSize(This, OpCode);
      }
      break;

      case Format4:
      {
         This->Info.Whole = OpFlags[This->Function];
         SetSize(This, OpCode);
      }
      break;

      case Format5:
      {
         This->Function += ((OpCode >> 10) & 0x0F);
         This->Info.Whole = OpFlags[This->Function];
         SetSize(This, OpCode >> 8);

         if (This->Function != SETCFG)
         { 
            if (OpCode & BIT(Translation))
            {
               This->Info.Op[0].Size = sz8;
               This->Info.Op[1].Size = sz8;
            }
         }
      }
      break;

      case Format6:
      case Format7:
      {
         This->Function += ((OpCode >> 10) & 0x0F);
         This->Info.Whole = OpFlags[This->Function];
         SetSize(This, OpCode >> 8);
      }
      break;

      case Format8:
      {
         if (OpCode & 0x400)
         {
            if (OpCode & 0x80)
            {
               switch (OpCode & 0x3CC0)
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
               This->Function = (OpCode & 0x40) ? FFS : INDEX;
            }
         }
         else
         {
            This->Function += ((OpCode >> 6) & 3);
         }

         This->Info.Whole = OpFlags[This->Function];
         SetSize(This, OpCode >> 8);
      }
      break;

      case Format9:
      {
         This->Function += ((OpCode >> 11) & 0x07);
         This->Info.Whole = OpFlags[This->Function];

         switch (This->Function)
         {
            case MOVif:
            {
               This->Info.Op[0].Size = ((OpCode >> 8) & 3) + 1;                        // Source Size (Integer)
               This->Info.Op[1].Size = GET_F_SIZE(This->OpCode & BIT(10));             // Destination Size (Float/ Double)
            }
            break;

            case ROUND:
            case TRUNC:
            case FLOOR:
            {
               This->Info.Op[0].Size = GET_F_SIZE(OpCode & BIT(10));                   // Source Size (Float/ Double)
               This->Info.Op[1].Size = ((OpCode >> 8) & 3) + 1;                        // Destination Size (Integer)
            }
            break;
         }

         if (nscfg.fpu_flag == 0)
         {
            Trap |= UnknownInstruction;
         }
      }
      break;

      case Format11:
      case Format12:
      {
         This->Function += ((OpCode >> 10) & 0x0F);
         This->Info.Whole = OpFlags[This->Function];
         This->Info.Op[0].Size =
         This->Info.Op[1].Size = GET_F_SIZE(OpCode & BIT(8));

         if (nscfg.fpu_flag == 0)
         {
            Trap |= UnknownInstruction;
         }
      }
      break;

      case Format14:
      {
         This->Function += ((OpCode >> 10) & 0x0F);
         This->Info.Whole = OpFlags[This->Function];
      }
      break;

      default:
      {
         Trap = UnknownFormat;
         This->Info.Whole = 0;
      }
      break;
   }

   if (Format >= Format6)
   {
      OpCode >>= 8;
   }

   if (This->Info.Op[0].Class)
   {
      getgen(This, OpCode >> 11, 0);
   }

   if (This->Info.Op[1].Class)
   {
      getgen(This, OpCode >> 6, 1);
   }

   return Trap;
}
