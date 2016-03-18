#ifdef _MSC_VER
#  define PACKED_STRUCT(name) \
    __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif defined(__GNUC__)
#  define PACKED_STRUCT(name) struct __attribute__((packed)) name
#endif

#define CASE2(in) case (in): case ((in) | 0x80)
#define CASE4(in) case (in): case ((in) | 0x40): case ((in) | 0x80): case ((in) | 0xC0)

enum Formats
{
   Format0,
   Format1,
   Format2,
   Format3,
   Format4,
   Format5,
   Format6,
   Format7,
   Format8,
   Format9,
   Format10,
   Format11,
   Format12,
   Format13,
   Format14,
   FormatCount,
   FormatBad = 0xFF
};

uint8_t FunctionLookup[256];

#define F_BASE(in) ((in) << 4)
enum Functions
{
   BEQ = F_BASE(Format0),
   BNE,
   BCS,
   BCC,
   BH,
   BLS,
   BGT,
   BLE,
   BFS,
   BFC,
   BLO,
   BHS,
   BLT,
   BGE,
   BR,
   BN,

   BSR = F_BASE(Format1),
   RET,
   CXP,
   RXP,
   RETT,
   RETI,
   SAVE,
   RESTORE,
   ENTER,
   EXIT,
   NOP,
   WAIT,
   DIA,
   FLAG,
   SVC,
   BPT,

   ADDQ = F_BASE(Format2),
   CMPQ,
   SPR,
   Scond,
   ACB,
   MOVQ,
   LPR,

   CXPD = F_BASE(Format3),
   TRAP_F3_0001,
   BICPSR,
   TRAP_F3_0011,
   JUMP,
   TRAP_F3_0101,
   BISPSR,
   TRAP_F3_0111,
   TRAP_F3_1000,
   TRAP_F3_1001,
   ADJSP,
   TRAP_F3_1011,
   JSR,
   TRAP_F3_1101,
   CASE,
   TRAP_F3_1111,

   ADD = F_BASE(Format4),
   CMP,
   BIC,
   TRAP_F4_0011,
   ADDC,
   MOV,
   OR,
   TRAP_F4_0111,
   SUB,
   ADDR,
   AND,
   TRAP_F4_1011,
   SUBC,
   TBIT,
   XOR,

   MOVS = F_BASE(Format5),
   CMPS,
   SETCFG,
   SKPS,

   ROT = F_BASE(Format6),
   ASH,
   CBIT,
   CBITI,
   TRAP_F5_0100,
   LSH,
   SBIT,
   SBITI,
   NEG,
   NOT,
   TRAP_F5_1010,
   SUBP,
   ABS,
   COM,
   IBIT,
   ADDP,

   MOVM = F_BASE(Format7),
   CMPM,
   INSS,
   EXTS,
   MOVXiW,
   MOVZiW,
   MOVZiD,
   MOVXiD,
   MUL,
   MEI,
   Trap,
   DEI,
   QUO,
   REM,
   MOD,
   DIV,

   EXT = F_BASE(Format8),
   CVTP,
   INS,
   CHECK,
   INDEX,
   FFS,
   MOVUS,
   MOVSU,

   MOVif = F_BASE(Format9),
   LFSR,
   MOVLF,
   MOVFL,
   ROUND,
   TRUNC,
   SFSR,
   FLOOR,

   ADDf = F_BASE(Format11),
   MOVf,
   CMPf,
   TRAP_F11_0011,
   SUBf,
   NEGf,
   TRAP_F11_0110,
   TRAP_F11_0111,
   DIVf,
   TRAP_F11_1001,
   TRAP_F11_1010,
   TRAP_F11_1011,
   MULf,
   ABSf,

   RDVAL = F_BASE(Format14),
   WRVAL,
   LMR,
   SMR,
   TRAP_F14_0100,
   TRAP_F14_0101,
   TRAP_F14_0110,
   TRAP_F14_0111,
   TRAP_F14_1000,
   CINV,

   TRAP = F_BASE(FormatCount),
   InstructionCount,

   BAD = 0xFF
};

// See Table 4-1 page 4-5 in the manual
enum OperandFlags
{
   not_used          = 0 * 256,   
   read              = 1 * 256,
   write             = 2 * 256,
   rmw               = 3 * 256,
   addr              = 5 * 256,
   Regaddr           = 7 * 256,
   EXTRA_BYTE        = BIT(14),
   FP                = BIT(15)
};

//#define OP(o1, o2) ((o2) << 8 | (o1))

#if 0
typedef struct
{
   unsigned Size : 8;
   unsigned Class : 7;
   unsigned Float : 1;
} OpDetail;
#else
typedef struct
{
   uint8_t Size;
   uint8_t  Class;
} OpDetail;
#endif

typedef union
{
   OpDetail Op[2];
   uint32_t Whole;
} OperandPair;

typedef union
{
   struct
   {
      unsigned OpType   : 8;
      unsigned IdxReg   : 3;
      unsigned IdxType  : 5;
   };

   struct
   {
      uint8_t LowerByte;
      uint8_t UpperByte;
   };

   uint16_t Whole;
} RegLKU;

typedef struct
{
   uint32_t       CurrentAddress;               // This is first with a zero offset as it used the most
   uint32_t       StartAddress;
   uint32_t       Function;
   uint32_t       OpCode;
   RegLKU         Regs[2];
   OperandPair    Info;
} DecodeData;

typedef union
{
   double      f64;
   uint64_t    u64;
   int64_t     s64;
} Temp64Type;

typedef union
{
   float       f32;
   uint32_t    u32;
   int32_t     s32;
} Temp32Type;

typedef union
{
   uint32_t	u32;
   int32_t s32;

   struct
   {
      uint8_t DoNotUse_u8_3;
      uint8_t DoNotUse_u8_2;
      uint8_t DoNotUse_u8_1;
      uint8_t u8;
   };

   struct
   {
      int8_t DoNotUse_s8_3;
      int8_t DoNotUse_s8_2;
      int8_t DoNotUse_s8_1;
      int8_t s8;
   };

   struct
   {
      uint16_t DoNotUse_u16_1;
      uint16_t u16;
   };

   struct
   {
      int16_t DoNotUse_s16_1;
      int16_t s16;
   };
} MultiReg;

typedef union
{
   uint64_t u64;
   int64_t  s64;

   uint32_t	u32;
   int32_t s32;

   struct
   {
      uint8_t DoNotUse_u8_3;
      uint8_t DoNotUse_u8_2;
      uint8_t DoNotUse_u8_1;
      uint8_t u8;
   };

   struct
   {
      int8_t DoNotUse_s8_3;
      int8_t DoNotUse_s8_2;
      int8_t DoNotUse_s8_1;
      int8_t s8;
   };

   struct
   {
      uint16_t DoNotUse_u16_1;
      uint16_t u16;
   };

   struct
   {
      int16_t DoNotUse_s16_1;
      int16_t s16;
   };
} MultiReg64;


extern const uint32_t OpFlags[InstructionCount];
extern uint8_t Consume_x8(DecodeData* This);
extern uint32_t Decode(DecodeData* This);

#define GET_PRECISION(in) ((in) ? SinglePrecision : DoublePrecision)
#define GET_F_SIZE(in) ((in) ? sz32 : sz64)
