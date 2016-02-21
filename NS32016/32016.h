#define K640  0x00A0000
#define MEG1  0x0100000
#define MEG4  0x0400000
#define MEG16 0x1000000

#define MEM_MASK 0xFFFFFF
#define RAM_SIZE 0x100000
//#define RAM_SIZE 0x400000

#define CASE2(in) case (in): case ((in) | 0x80)
#define CASE4(in) case (in): case ((in) | 0x40): case ((in) | 0x80): case ((in) | 0xC0)

#define C_FLAG 0x01
#define T_FLAG 0x02
#define L_FLAG 0x04
#define F_FLAG 0x20
#define V_FLAG 0x20
#define Z_FLAG 0x40
#define N_FLAG 0x80

#define U_FLAG 0x100
#define S_FLAG 0x200
#define P_FLAG 0x400
#define I_FLAG 0x800

enum Functions
{
	StrI,
	CMPQ,
	MOVQ,
	ADDQ,
	Scond,
	ACB,
	ADD,
	CMP,
	BIC,
	MOV,
	OR,
	XOR,
	SUB,
	ADDR,
	AND,
	TBIT,
	TYPE6,
	TYPE3,
	SPR,
	LPR,
	FORMAT7,
	TYPE8,
	BSR,
	RET,
	CXP,
	RXP,
	RETT,
	SAVE,
	RESTORE,
	ENTER,
	EXIT,
	SVC,
	BEQ,
	BNE,
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
	BAD
};

enum DataSize
{
	sz8,
	sz16,
	sz32,
	szVaries
};

typedef union
{
	struct
	{
		uint8_t Format;
		uint8_t Size;
		uint8_t Function;
	} p;

	uint32_t Whole;
} DecodeMatrix;

extern void n32016_init();
extern void n32016_reset();
extern void n32016_exec(uint32_t tubecycles);
extern void ShowInstruction(uint32_t startpc);

extern uint8_t ns32016ram[MEG16];
extern uint32_t tube_irq;
extern DecodeMatrix mat[256];
