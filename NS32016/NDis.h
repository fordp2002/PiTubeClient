extern const char InstuctionText[InstructionCount][8];
extern void DisassembleUsingITrace(uint32_t Location, uint32_t End);
extern void ShowInstruction(DecodeData* This);
extern void Disassemble(uint32_t Location, uint32_t End);
extern void ShowRegisterWrite(RegLKU RegIn, uint64_t Value);

extern uint32_t IP[MEG16];

