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

void BreakPoint(uint32_t pc, uint32_t opcode)
{
#if 1
#ifndef TEST_SUITE
   // Exec address of Bas32
   if (pc == 0x000200)
   {
      printf("Entering Bas32\n");
      ProfileInit();
   }
   // Exec address of Panos
   if (pc == 0x000400)
   {
      printf("Entering Panos\n");
      ProfileInit();
   }
   // Address of SVC &11 (OS_EXIT)
   if (pc == 0xF007BB)
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
   OP(not_used, not_used),    // ADDQ    
   OP(not_used, not_used),    // CMPQ
   OP(not_used, not_used),    // SPR
   OP(not_used, not_used),    // Scond

   OP(not_used, not_used),    // ACB
   OP(not_used, not_used),    // LPR
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

   // Format 3  
   OP(not_used, not_used),    // CXPD    
   OP(not_used, not_used),
   OP(not_used, not_used),    // BICPSR
   OP(not_used, not_used),

   OP(not_used, not_used),    // JUMP
   OP(not_used, not_used),
   OP(not_used, not_used),    // BIPSR
   OP(not_used, not_used),

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),    // ADJSP
   OP(not_used, not_used),

   OP(not_used, not_used),    // JSR
   OP(not_used, not_used),
   OP(not_used, not_used),    // CASE
   OP(not_used, not_used),

   // Format 4
   OP(not_used, not_used),    // ADD
   OP(not_used, not_used),    // CMP
   OP(not_used, not_used),    // BIC
   OP(not_used, not_used),

   OP(not_used, not_used),    // ADDC
   OP(not_used, not_used),    // MOV
   OP(not_used, not_used),    // OR
   OP(not_used, not_used),

   OP(not_used, not_used),    // SYB
   OP(not_used, not_used),    // ADDR
   OP(not_used, not_used),    // AND
   OP(not_used, not_used),

   OP(not_used, not_used),    // SUBC
   OP(not_used, not_used),    // TBIT
   OP(not_used, not_used),    // XOR
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
   OP(not_used, not_used),    // ROT
   OP(not_used, not_used),    // ASH
   OP(not_used, not_used),    // CBIT
   OP(not_used, not_used),    // CBITI

   OP(not_used, not_used),
   OP(not_used, not_used),    // LSH
   OP(not_used, not_used),    // SBIT
   OP(not_used, not_used),    // SBITI

   OP(not_used, not_used),    // NEG
   OP(not_used, not_used),    // NOT
   OP(not_used, not_used),
   OP(not_used, not_used),    // SUBP

   OP(not_used, not_used),    // ABS
   OP(not_used, not_used),    // COM
   OP(not_used, not_used),    // IBIT
   OP(not_used, not_used),    // ADDP

   // Format 7
   OP(not_used, not_used),    // MOVM
   OP(not_used, not_used),    // CMPM
   OP(not_used, not_used),    // INSS
   OP(not_used, not_used),    // EXTS

   OP(not_used, not_used),    // MOVXiW
   OP(not_used, not_used),    // MOVZiW
   OP(not_used, not_used),    // MOVZiD
   OP(not_used, not_used),    // MOVXiD 

   OP(not_used, not_used),    // MUL
   OP(not_used, not_used),    // MEI
   OP(not_used, not_used),
   OP(not_used, not_used),    // DEI

   OP(not_used, not_used),    // QUO
   OP(not_used, not_used),    // REM
   OP(not_used, not_used),    // MOD
   OP(not_used, not_used),    // DIV

   // Format 8
   OP(not_used, not_used),    // EXT
   OP(not_used, not_used),    // CVTP
   OP(not_used, not_used),    // INS
   OP(not_used, not_used),    // CHECK

   OP(not_used, not_used),    // INDEX
   OP(not_used, not_used),    // FFS
   OP(not_used, not_used),    // MOVUS
   OP(not_used, not_used),    // MOVSU

   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
 
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),
   OP(not_used, not_used),


   // Format 9
   OP(read, write | FP),      // MOVif
   OP(read, not_used),        // LFSR
   OP(read | FP, write | FP), // MOVLF
   OP(read | FP, write | FP), // MOVFL

   OP(read | FP,  write),     // ROUND
   OP(read | FP,  write),     // TRUNC
   OP(not_used,   write),     // SFSR
   OP(read | FP,  write),     // FLOOR

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