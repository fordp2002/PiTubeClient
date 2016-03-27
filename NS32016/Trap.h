#include <signal.h>


#define TrapTRACE printf

enum TrapTypes
{
   NoIssue                 = 0,
   
   // These are features rather than traps
   NMI                     = BIT(0),
   IRQ                     = BIT(1),
   BreakPointHit           = BIT(2),
   INSTRUCTION_PROFILING   = BIT(3),
   PROFILING               = BIT(4),
   SHOW_INSTRUCTIONS       = BIT(5),
   SHOW_WRITES             = BIT(6),

   // These are geniune traps
   ReservedAddressingMode  = BIT(8),
   UnknownFormat           = BIT(9),
   UnknownInstruction      = BIT(10),
   DivideByZero            = BIT(11),
   IllegalImmediate        = BIT(12),
   IllegalDoubleIndexing   = BIT(13),
   IllegalSpecialReading   = BIT(14),
   IllegalSpecialWriting   = BIT(15),
   IllegalWritingImmediate = BIT(16),
   FlagInstruction         = BIT(17),
   PrivilegedInstruction   = BIT(18),
   BreakPointTrap          = BIT(19),
   TOSNotSupported         = BIT(20)
};

#define TrapCount 21
extern sig_atomic_t TrapFlags;
#define CLEAR_TRAP() TrapFlags &= 0xFF

// Use SET_TRAP when in a function
#define SET_TRAP(in) TrapFlags |= (in)

extern void ShowTraps(void);
extern void HandleTrap(void);
extern void n32016_dumpregs();
