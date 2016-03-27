#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "Decode.h"
#include "32016.h"
#include "mem32016.h"
#include "Trap.h"
#include "Profile.h"
#include "NDis.h"

#ifdef PC_SIMULATION
//uint32_t TrapFlags = INSTRUCTION_PROFILING | PROFILING | SHOW_WRITES | SHOW_INSTRUCTIONS;
uint32_t TrapFlags = SHOW_WRITES | SHOW_INSTRUCTIONS;
#else
uint32_t TrapFlags = 0;
#endif

const char TrapText[TrapCount][40] =
{
   "FIQ",
   "IRQ",
   "Break Point Hit",
   "INSTRUCTION_PROFILING",
   "PROFILING",
   "SHOW_INSTRUCTIONS",
   "SHOW_WRITES",
   "",
   "Reserved Addressing Mode",
   "Unknown Format",
   "Unknown Instruction",
   "Divide By Zero",
   "Illegal Immediate",
   "Illegal DoubleIndexing",
   "Illegal SpecialReading",
   "Illegal SpecialWriting",
   "Illegal Writing Immediate",
   "Flag Instuction",
   "Privileged Instruction",
   "BreakPointTrap",
   "TOS not supported"
};

void ShowTraps(void)
{
   if (TrapFlags)
   {
      uint32_t Count, Pattern = BIT(0);
      for (Count = 0; Count < TrapCount; Count++, Pattern <<= 1)
      {
         if (TrapFlags & Pattern)
         {
            TrapTRACE("%s\n", TrapText[Count]);
         }
      }
   }
}

void Dump(void)
{
   n32016_ShowRegs(0xFF);
   ShowTraps();
   TrapTRACE("\n");

   if (TrapFlags & PROFILING)
   {
      ProfileDump();
   }

   if (TrapFlags & INSTRUCTION_PROFILING)
   {
      DisassembleUsingITrace(0, 0x1000);
   }
}

void n32016_dumpregs(char* pMessage)
{
   TrapTRACE("%s\n", pMessage);
   Dump();

#ifdef PC_SIMULATION
#ifdef TRACE_TO_FILE
   printf("\n");
#endif

#ifdef WIN32
   system("pause");
#endif
   CloseTrace();
   exit(1);
#endif
}

void HandleTrap(void)
{
   n32016_dumpregs("HandleTrap() called");
}
