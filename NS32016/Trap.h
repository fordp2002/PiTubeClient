#define TrapTRACE printf

enum TrapTypes
{
   NoIssue                 = 0,
   
   // These are features rather than traps
   BreakPointHit           = BIT(0),
   INSTRUCTION_PROFILING   = BIT(1),
   PROFILING               = BIT(2),
   SHOW_INSTRUCTIONS       = BIT(3),
   SHOW_WRITES             = BIT(4),

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
   BreakPointTrap          = BIT(19)
};

#define TrapCount 20
extern uint32_t TrapFlags;
#define CLEAR_TRAP() TrapFlags &= 0xFF

// Use SET_TRAP when in a function
#define SET_TRAP(in) TrapFlags |= (in)

extern void ShowTraps(void);
extern void HandleTrap(void);
extern void n32016_dumpregs();
