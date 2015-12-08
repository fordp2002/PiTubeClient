#include <stdio.h>
#include "debug.h"
#include "startup.h"
#include "tube-lib.h"
#include "tube-env.h"
#include "tube-swi.h"
#include "tube-isr.h"

#define NUM_SWI_HANDLERS 0x80

const int osword_in_len[] = {
  0,  // OSWORD 0x00
  0,
  5,
  0,
  5,
  2,
  5,
  8,
  14,
  4,
  1,
  1,
  5,
  0,
  8,
  25,
  16,
  13,
  0,
  8,
  128  // OSWORD 0x14
};

const int osword_out_len[] = {
  0,  // OSWORD 0x00
  5,
  0,
  5,
  0,
  5,
  0,
  0,
  0,
  5,
  9,
  5,
  0,
  8,
  25,
  1,
  13,
  13,
  128,
  8,
  128  // OSWORD 0x14
};

// Basic 135 uses the following on startup
//   SWI 00000010 // OS_GetEnv
//   SWI 0002006e // OS_SynchroniseCodeAreas *** Not Implemented ***
//   SWI 00020040 // OS_ChangeEnvironment
//   SWI 00020040 // OS_ChangeEnvironment
//   SWI 00020040 // OS_ChangeEnvironment
//   SWI 00020040 // OS_ChangeEnvironment
//   SWI 00000010 // OS_GetEnv
//   SWI 00000010 // OS_GetEnv
//   SWI 00000001 // OS_WriteS
//
//   SWI 00062c82 // BASICTrans_Message *** Not Implemented ***
//   SWI 0000013e // OS_WriteI
//   SWI 0000000e // OS_ReadLine
//   SWI 00062c81 // BASICTrans_Error *** Not Implemented ***
//   SWI 00000006 // OS_Byte
//
//   SWI 00062c82 // BASICTrans_Message *** Not Implemented ***
//   SWI 0000013e // OS_WriteI
//   SWI 0000000e // OS_ReadLine


SWIHandler_Type SWIHandler_Table[NUM_SWI_HANDLERS] = {
  tube_WriteC,                // (&00) -- OS_WriteC
  tube_WriteS,                // (&01) -- OS_WriteS
  tube_Write0,                // (&02) -- OS_Write0
  tube_NewLine,               // (&03) -- OS_NewLine
  tube_ReadC,                 // (&04) -- OS_ReadC
  tube_CLI,                   // (&05) -- OS_CLI
  tube_Byte,                  // (&06) -- OS_Byte
  tube_Word,                  // (&07) -- OS_Word
  tube_File,                  // (&08) -- OS_File
  tube_Args,                  // (&09) -- OS_Args
  tube_BGet,                  // (&0A) -- OS_BGet
  tube_BPut,                  // (&0B) -- OS_BPut
  tube_GBPB,                  // (&0C) -- OS_GBPB
  tube_Find,                  // (&0D) -- OS_Find
  tube_ReadLine,              // (&0E) -- OS_ReadLine
  tube_SWI_Not_Known,         // (&0F) -- OS_Control
  tube_GetEnv,                // (&10) -- OS_GetEnv
  tube_Exit,                  // (&11) -- OS_Exit
  tube_SWI_Not_Known,         // (&12) -- OS_SetEnv
  tube_IntOn,                 // (&13) -- OS_IntOn
  tube_IntOff,                // (&14) -- OS_IntOff
  tube_SWI_Not_Known,         // (&15) -- OS_CallBack
  tube_EnterOS,               // (&16) -- OS_EnterOS
  tube_SWI_Not_Known,         // (&17) -- OS_BreakPt
  tube_SWI_Not_Known,         // (&18) -- OS_BreakCtrl
  tube_SWI_Not_Known,         // (&19) -- OS_UnusedSWI
  tube_SWI_Not_Known,         // (&1A) -- OS_UpdateMEMC
  tube_SWI_Not_Known,         // (&1B) -- OS_SetCallBack
  tube_Mouse,                 // (&1C) -- OS_Mouse
  tube_SWI_Not_Known,         // (&1D) -- OS_Heap
  tube_SWI_Not_Known,         // (&1E) -- OS_Module
  tube_SWI_Not_Known,         // (&1F) -- OS_Claim
  tube_SWI_Not_Known,         // (&20) -- OS_Release
  tube_SWI_Not_Known,         // (&21) -- OS_ReadUnsigned
  tube_SWI_Not_Known,         // (&22) -- OS_GenerateEvent
  tube_SWI_Not_Known,         // (&23) -- OS_ReadVarVal
  tube_SWI_Not_Known,         // (&24) -- OS_SetVarVal
  tube_SWI_Not_Known,         // (&25) -- OS_GSInit
  tube_SWI_Not_Known,         // (&26) -- OS_GSRead
  tube_SWI_Not_Known,         // (&27) -- OS_GSTrans
  tube_SWI_Not_Known,         // (&28) -- OS_BinaryToDecimal
  tube_SWI_Not_Known,         // (&29) -- OS_FSControl
  tube_SWI_Not_Known,         // (&2A) -- OS_ChangeDynamicArea
  tube_SWI_Not_Known,         // (&2B) -- OS_GenerateError
  tube_SWI_Not_Known,         // (&2C) -- OS_ReadEscapeState
  tube_SWI_Not_Known,         // (&2D) -- OS_EvaluateExpression
  tube_SWI_Not_Known,         // (&2E) -- OS_SpriteOp
  tube_SWI_Not_Known,         // (&2F) -- OS_ReadPalette
  tube_SWI_Not_Known,         // (&30) -- OS_ServiceCall
  tube_SWI_Not_Known,         // (&31) -- OS_ReadVduVariables
  tube_SWI_Not_Known,         // (&32) -- OS_ReadPoint
  tube_SWI_Not_Known,         // (&33) -- OS_UpCall
  tube_SWI_Not_Known,         // (&34) -- OS_CallAVector
  tube_SWI_Not_Known,         // (&35) -- OS_ReadModeVariable
  tube_SWI_Not_Known,         // (&36) -- OS_RemoveCursors
  tube_SWI_Not_Known,         // (&37) -- OS_RestoreCursors
  tube_SWI_Not_Known,         // (&38) -- OS_SWINumberToString
  tube_SWI_Not_Known,         // (&39) -- OS_SWINumberFromString
  tube_SWI_Not_Known,         // (&3A) -- OS_ValidateAddress
  tube_SWI_Not_Known,         // (&3B) -- OS_CallAfter
  tube_SWI_Not_Known,         // (&3C) -- OS_CallEvery
  tube_SWI_Not_Known,         // (&3D) -- OS_RemoveTickerEvent
  tube_SWI_Not_Known,         // (&3E) -- OS_InstallKeyHandler
  tube_SWI_Not_Known,         // (&3F) -- OS_CheckModeValid
  tube_ChangeEnvironment,     // (&40) -- OS_ChangeEnvironment
  tube_SWI_Not_Known,         // (&41) -- OS_ClaimScreenMemory
  tube_SWI_Not_Known,         // (&42) -- OS_ReadMonotonicTime
  tube_SWI_Not_Known,         // (&43) -- OS_SubstituteArgs
  tube_SWI_Not_Known,         // (&44) -- OS_PrettyPrintCode
  tube_SWI_Not_Known,         // (&45) -- OS_Plot
  tube_SWI_Not_Known,         // (&46) -- OS_WriteN
  tube_SWI_Not_Known,         // (&47) -- OS_AddToVector
  tube_SWI_Not_Known,         // (&48) -- OS_WriteEnv
  tube_SWI_Not_Known,         // (&49) -- OS_ReadArgs
  tube_SWI_Not_Known,         // (&4A) -- OS_ReadRAMFsLimits
  tube_SWI_Not_Known,         // (&4B) -- OS_ClaimDeviceVector
  tube_SWI_Not_Known,         // (&4C) -- OS_ReleaseDeviceVector
  tube_SWI_Not_Known,         // (&4D) -- OS_DelinkApplication
  tube_SWI_Not_Known,         // (&4E) -- OS_RelinkApplication
  tube_SWI_Not_Known,         // (&4F) -- OS_HeapSort
  tube_SWI_Not_Known,         // (&50) -- OS_ExitAndDie
  tube_SWI_Not_Known,         // (&51) -- OS_ReadMemMapInfo
  tube_SWI_Not_Known,         // (&52) -- OS_ReadMemMapEntries
  tube_SWI_Not_Known,         // (&53) -- OS_SetMemMapEntries
  tube_SWI_Not_Known,         // (&54) -- OS_AddCallBack
  tube_SWI_Not_Known,         // (&55) -- OS_ReadDefaultHandler
  tube_SWI_Not_Known,         // (&56) -- OS_SetECFOrigin
  tube_SWI_Not_Known,         // (&57) -- OS_SerialOp
  tube_SWI_Not_Known,         // (&58) -- OS_ReadSysInfo
  tube_SWI_Not_Known,         // (&59) -- OS_Confirm
  tube_SWI_Not_Known,         // (&5A) -- OS_ChangedBox
  tube_SWI_Not_Known,         // (&5B) -- OS_CRC
  tube_SWI_Not_Known,         // (&5C) -- OS_ReadDynamicArea
  tube_SWI_Not_Known,         // (&5D) -- OS_PrintChar
  tube_SWI_Not_Known,         // (&5E) -- OS_ChangeRedirection
  tube_SWI_Not_Known,         // (&5F) -- OS_RemoveCallBack
  tube_SWI_Not_Known,         // (&60) -- OS_FindMemMapEntries
  tube_SWI_Not_Known,         // (&61) -- OS_SetColourCode
  tube_SWI_Not_Known,         // (&62) -- OS_ClaimSWI
  tube_SWI_Not_Known,         // (&63) -- OS_ReleaseSWI
  tube_SWI_Not_Known,         // (&64) -- OS_Pointer
  tube_SWI_Not_Known,         // (&65) -- OS_ScreenMode
  tube_SWI_Not_Known,         // (&66) -- OS_DynamicArea
  tube_SWI_Not_Known,         // (&67) -- OS_AbortTrap
  tube_SWI_Not_Known,         // (&68) -- OS_Memory
  tube_SWI_Not_Known,         // (&69) -- OS_ClaimProcessorVector
  tube_SWI_Not_Known,         // (&6A) -- OS_Reset
  tube_SWI_Not_Known,         // (&6B) -- OS_MMUControl
  tube_SWI_Not_Known,         // (&6C) -- OS_ResyncTime
  tube_SWI_Not_Known,         // (&6D) -- OS_PlatformFeatures
  tube_SWI_Not_Known,         // (&6E) -- OS_SynchroniseCodeAreas
  tube_SWI_Not_Known,         // (&6F) -- OS_CallASWI
  tube_SWI_Not_Known,         // (&70) -- OS_AMBControl
  tube_SWI_Not_Known,         // (&71) -- OS_CallASWIR12
  tube_SWI_Not_Known,         // (&72) -- OS_SpecialControl
  tube_SWI_Not_Known,         // (&73) -- OS_EnterUSR32
  tube_SWI_Not_Known,         // (&74) -- OS_EnterUSR26
  tube_SWI_Not_Known,         // (&75) -- OS_VIDCDivider
  tube_SWI_Not_Known,         // (&76) -- OS_NVMemory
  tube_SWI_Not_Known,         // (&77) -- OS_ClaimOSSWI
  tube_SWI_Not_Known,         // (&78) -- OS_TaskControl
  tube_SWI_Not_Known,         // (&79) -- OS_DeviceDriver
  tube_SWI_Not_Known,         // (&7A) -- OS_Hardware
  tube_SWI_Not_Known,         // (&7B) -- OS_IICOp
  tube_SWI_Not_Known,         // (&7C) -- OS_LeaveOS
  tube_SWI_Not_Known,         // (&7D) -- OS_ReadLine32
  tube_SWI_Not_Known,         // (&7E) -- OS_SubstituteArgs32
  tube_SWI_Not_Known          // (&7F) -- OS_HeapSort32
};

// For an unimplemented environment handler
void handler_not_implemented(unsigned int handler) {
  printf("Handler %d not implemented\r\n", handler);
}

// For an unimplemented SWI
void tube_SWI_Not_Known(unsigned int *reg) {
  unsigned int *lr = (unsigned int *)reg[13];
  printf("SWI %08x not implemented\r\n", *(lr - 1) & 0xFFFFFF);
}

void C_SWI_Handler(unsigned int number, unsigned int *reg) {
  unsigned int num = number;
  int errorBit = 0;
  if (DEBUG) {
    printf("SWI %08x called from %08x\r\n", number, reg[13] - 4);
  }
  if (num & ERROR_BIT) {
    errorBit = 1;
    num &= ~ERROR_BIT;
  }
  if (num < NUM_SWI_HANDLERS) {
    // Invoke one of the fixed handlers
    SWIHandler_Table[num](reg);
  } else if ((num & 0xFF00) == 0x0100) {
    // SWI's 0x100 - 0x1FF are OS_WriteI
    tube_WriteC(&num);
  } else {
    tube_SWI_Not_Known(reg);
  }
  if (DEBUG) {
    printf("SWI %08x complete\r\n", number);
  }
}

// Helper functions

void updateCarry(unsigned char cy, unsigned int *reg) {
  // The PSW is on the stack one word before the registers
  reg--;
  // bit 29 is the carry
  if (cy & 0x80) {
    *reg |= CARRY_MASK;
  } else {
    *reg &= ~CARRY_MASK;
  }
}

void user_exec(volatile unsigned char *address) {
  if (DEBUG) {
    printf("Execution passing to %08x\r\n", (unsigned int)address);
  }
  // setTubeLibDebug(1);
  // The machine code version in armc-startup.S does the real work
  // of dropping down to user mode
  _user_exec(address);
}

// Client to Host transfers
// Reference: http://mdfs.net/Software/Tube/Protocol
// OSWRCH   R1: A
// OSRDCH   R2: &00                               Cy A
// OSCLI    R2: &02 string &0D                    &7F or &80
// OSBYTELO R2: &04 X A                           X
// OSBYTEHI R2: &06 X Y A                         Cy Y X
// OSWORD   R2: &08 A in_length block out_length  block
// OSWORD0  R2: &0A block                         &FF or &7F string &0D
// OSARGS   R2: &0C Y block A                     A block
// OSBGET   R2: &0E Y                             Cy A
// OSBPUT   R2: &10 Y A                           &7F
// OSFIND   R2: &12 &00 Y                         &7F
// OSFIND   R2: &12 A string &0D                  A
// OSFILE   R2: &14 block string &0D A            A block
// OSGBPB   R2: &16 block A                       block Cy A

void tube_WriteC(unsigned int *reg) {
  sendByte(R1, (unsigned char)(*reg & 0xff));
}

void tube_WriteS(unsigned int *reg) {
  // Reg 13 is the stacked link register which points to the string
  tube_Write0(&reg[13]);
  // Make sure new value of link register is word aligned to the next word boundary
  reg[13] += 3;
  reg[13] &= ~3;
}

void tube_Write0(unsigned int *reg) {
  unsigned char *ptr = (unsigned char *)(*reg);
  unsigned char c;
  // Output characters pointed to by R0, until a terminating zero
  while ((c = *ptr++) != 0) {
    sendByte(R1, c);
  }
  // On exit, R0 points to the byte after the terminator
  *reg = (unsigned int)ptr;
}

void tube_NewLine(unsigned int *reg) {
  sendByte(R1, 0x0A);
  sendByte(R1, 0x0D);
}

void tube_ReadC(unsigned int *reg) {
  // OSRDCH   R2: &00                               Cy A
  sendByte(R2, 0x00);
  // On exit, the Carry flag indicaes validity
  updateCarry(receiveByte(R2), reg);
  // On exit, R0 contains the character
  reg[0] = receiveByte(R2);
}

void tube_CLI(unsigned int *reg) {
  char *ptr = (char *)(*reg);
  // OSCLI    R2: &02 string &0D                    &7F or &80
  sendByte(R2, 0x02);
  sendString(R2, 0x00, ptr);
  sendByte(R2, 0x0D);
  if (receiveByte(R2) & 0x80) {
    // Execution should pass to last transfer address
    user_exec(address);
  }
}

void tube_Byte(unsigned int *reg) {
  if (DEBUG) {
    printf("%08x %08x %08x\r\n", reg[0], reg[1], reg[2]);
  }
  unsigned char cy;
  unsigned char a = reg[0] & 0xff;
  unsigned char x = reg[1] & 0xff;
  unsigned char y = reg[2] & 0xff;
  if (a < 128) {
    // OSBYTELO R2: &04 X A                           X
    sendByte(R2, 0x04);
    sendByte(R2, x);
    sendByte(R2, a);
    x = receiveByte(R2);
    reg[1] = x;

  } else {
    // OSBYTEHI R2: &06 X Y A                         Cy Y X
    sendByte(R2, 0x06);
    sendByte(R2, x);
    sendByte(R2, y);
    sendByte(R2, a);
    cy = receiveByte(R2);
    y = receiveByte(R2);
    x = receiveByte(R2);
    reg[1] = x;
    reg[2] = y;
    updateCarry(cy, reg);
  }
  if (DEBUG) {
    printf("%08x %08x %08x\r\n", reg[0], reg[1], reg[2]);
  }
}

void tube_Word(unsigned int *reg) {
  int in_len;
  int out_len;
  unsigned char a = reg[0] & 0xff;
  unsigned char *block;
  // Note that call with R0b=0 (Acorn MOS RDLN) does nothing, the ReadLine call should be used instead.
  // Ref: http://chrisacorns.computinghistory.org.uk/docs/Acorn/OEM/AcornOEM_ARMUtilitiesRM.pdf
  if (a == 0) {
    return;
  }
  // Work out block lengths
  // Ref: http://mdfs.net/Info/Comp/Acorn/AppNotes/004.pdf
  block = (unsigned char *)reg[1];
  if (a < 0x15) {
    in_len = osword_in_len[a];
    out_len = osword_out_len[a];
  } else if (a < 128) {
    in_len = 16;
    out_len = 16;
  } else {
    // TODO: Check with JGH whether it is correct to update block to exclude the lengths
    in_len = *block++;
    out_len = *block++;
  }
  // OSWORD   R2: &08 A in_length block out_length  block
  sendByte(R2, 0x08);
  sendByte(R2, a);
  sendByte(R2, in_len);
  sendBlock(R2, in_len, block);
  sendByte(R2, out_len);
  receiveBlock(R2, out_len, block);
}

void tube_File(unsigned int *reg) {
  if (DEBUG) {
    printf("%08x %08x %08x %08x %08x %08x\r\n", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5]);
    printf("%s\r\n", (char *)reg[1]);
  }
  // start at the last param (r5)
  unsigned int *ptr = reg + 5;
  // OSFILE   R2: &14 block string &0D A            A block
  sendByte(R2, 0x14);
  sendWord(R2, *ptr--);            // r5 = attr
  sendWord(R2, *ptr--);            // r4 = leng
  sendWord(R2, *ptr--);            // r3 = exec
  sendWord(R2, *ptr--);            // r2 = load
  sendString(R2, 0x0D, (char *)*ptr--);  // r1 = filename ptr
  sendByte(R2, 0x0D);              //      filename terminator
  sendByte(R2, *ptr);              // r0 = action
  *ptr = receiveByte(R2);          // r0 = action
  ptr = reg + 5;                   // ptr = r5
  *ptr-- = receiveWord(R2);        // r5 = attr
  *ptr-- = receiveWord(R2);        // r4 = lang
  *ptr-- = receiveWord(R2);        // r3 = exec
  *ptr-- = receiveWord(R2);        // r2 = load
  if (DEBUG) {
    printf("%08x %08x %08x %08x %08x %08x\r\n", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5]);
  }
}

void tube_Args(unsigned int *reg) {
  // OSARGS   R2: &0C Y block A                     A block
  sendByte(R2, 0x0C);
  // Y = R1 is the file namdle
  sendByte(R2, reg[1]);
  // R2 is the 4 byte data block
  sendWord(R2, reg[2]);
  // A = R0 is the operation code
  sendByte(R2, reg[0]);
  // get back A
  reg[0] = receiveByte(R2);
  // get back 4 byte data block
  reg[2] = receiveWord(R2);
}

void tube_BGet(unsigned int *reg) {
  // OSBGET   R2: &0E Y                             Cy A
  sendByte(R2, 0x0E);
  // Y = R1 is the file namdle
  sendByte(R2, reg[1]);
  // On exit, the Carry flag indicaes validity
  updateCarry(receiveByte(R2), reg);
  // On exit, R0 contains the character
  reg[0] = receiveByte(R2);
}

void tube_BPut(unsigned int *reg) {
  // OSBPUT   R2: &10 Y A                           &7F
  sendByte(R2, 0x10);
  // Y = R1 is the file namdle
  sendByte(R2, reg[1]);
  // A = R0 is the character
  sendByte(R2, reg[0]);
  // Response is always 7F so ingnored
  receiveByte(R2);
}

void tube_GBPB(unsigned int *reg) {
  // start at the last param (r4)
  unsigned int *ptr = reg + 4;
  // OSGBPB   R2: &16 block A                       block Cy A
  sendByte(R2, 0x16);
  sendWord(R2, *ptr--);              // r4
  sendWord(R2, *ptr--);              // r3
  sendWord(R2, *ptr--);              // r2
  sendByte(R2, *ptr--);              // r1
  sendByte(R2, *ptr);                // r0
  ptr = reg + 4;
  *ptr-- = receiveWord(R2);          // r4
  *ptr-- = receiveWord(R2);          // r3
  *ptr-- = receiveWord(R2);          // r2
  *ptr-- = receiveByte(R2);          // r1
  updateCarry(receiveByte(R2), reg); // Cy
  *ptr-- = receiveWord(R2);          // r0
}

void tube_Find(unsigned int *reg) {
  // OSFIND   R2: &12 &00 Y                         &7F
  // OSFIND   R2: &12 A string &0D                  A
  sendByte(R2, 0x12);
  // A = R0 is the operation type
  sendByte(R2, reg[0]);
  if (reg[0] == 0) {
    // Y = R1 is the file handle to close
    sendByte(R2, reg[1]);
    // Response is always 7F so ignored
    receiveByte(R2);
  } else {
    // R1 points to the string
    sendString(R2, 0x0D, (char *)reg[1]);
    sendByte(R2, 0x0D);
    // Response is the file handle of file just opened
    reg[0] = receiveByte(R2);
  }
}

void tube_ReadLine(unsigned int *reg) {
  unsigned char resp;
  // OSWORD0  R2: &0A block                         &FF or &7F string &0D
  sendByte(R2, 0x0A);
  sendByte(R2, reg[3]);      // max ascii value
  sendByte(R2, reg[2]);      // min ascii value
  sendByte(R2, reg[1]);      // max line length
  sendByte(R2, 0x07);        // Buffer MSB - set as per Tube Ap Note 004
  sendByte(R2, 0x00);        // Buffer LSB - set as per Tube Ap Note 004
  resp = receiveByte(R2);    // 0x7F or 0xFF
  updateCarry(resp, reg);
  // Was it valid?
  if ((resp & 0x80) == 0x00) {
    reg[1] = receiveString(R2, '\r', (char *)reg[0]);
  }
}

void tube_GetEnv(unsigned int *reg) {
  // R0 address of the command string (0 terminated) which ran the program
  reg[0] = (unsigned int) env->commandBuffer;
  // R1 address of the permitted RAM limit for example &10000 for 64K machine
  reg[1] = env->memoryLimit;
  // R2 address of 5 bytes - the time the program started running
  reg[2] = (unsigned int) env->timeBuffer;
  if (DEBUG) {
    printf("%08x %08x %08x\r\n", reg[0], reg[1], reg[2]);
  }
}

void tube_Exit(unsigned int *reg) {
  env->exitHandler();
}

void tube_IntOn(unsigned int *reg) {
  _enable_interrupts();
}

void tube_IntOff(unsigned int *reg) {
  _disable_interrupts();
}

void tube_EnterOS(unsigned int *reg) {
}

void tube_Mouse(unsigned int *reg) {
}

// Entry:
// R0   Handler number
// R1   New address, or 0 to read
// R2   Value of R12 when code is called
// R3   Pointer to buffer if appropriate or 0 to read
// Exit:
// R0   Preserved
// R1   Previous address
// R2   Previous R12
// R3   Previous buffer pointer

void tube_ChangeEnvironment(unsigned int *reg) {
  unsigned int previous;

  if (DEBUG) {
    printf("%08x %08x %08x %08x\r\n", reg[0], reg[1], reg[2], reg[3]);
  }

  switch (reg[0]) {

  case 0:    // Memory limit
    previous = env->memoryLimit;
    if (reg[1]) {
      env->memoryLimit = reg[1];
    }
    reg[1] = previous;
    break;

  case 1:    // Undefined instruction?
    previous = (unsigned int) env->undefinedInstructionHandler;
    if (reg[1]) {
      env->undefinedInstructionHandler = (ExceptionHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 2:    // Prefetch abort?
    previous = (unsigned int) env->prefetchAbortHandler;
    if (reg[1]) {
      env->prefetchAbortHandler = (ExceptionHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 3:    // Data abort?
    previous = (unsigned int) env->dataAbortHandler;
    if (reg[1]) {
      env->dataAbortHandler = (ExceptionHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 4:    // Address exception?
    previous = (unsigned int) env->addressExceptionHandler;
    if (reg[1]) {
      env->addressExceptionHandler = (ExceptionHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 5:    // Other exceptions (reserved)
    handler_not_implemented(reg[0]);
    break;

  case 6:    // Error
    previous = (unsigned int)env->errorHandler;
    if (reg[1]) {
      env->errorHandler = (ErrorHandler_type) reg[1];
    }
    reg[1] = previous;
    previous = (unsigned int)env->errorBuffer;
    if (reg[3]) {
      env->errorBuffer = (ErrorBuffer_type *) reg[1];
    }
    reg[3] = previous;
    break;

  case 7:    // CallBack?
    handler_not_implemented(reg[0]);
    break;

  case 8:    // BreakPoint?
    handler_not_implemented(reg[0]);
    break;

  case 9:    // Escape
    previous = (unsigned int)env->escapeHandler;
    if (reg[1]) {
      env->escapeHandler = (EscapeHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 10:   // Event?
    previous = (unsigned int)env->eventHandler;
    if (reg[1]) {
      env->eventHandler = (EventHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 11:   // Exit
    previous = (unsigned int)env->exitHandler;
    if (reg[1]) {
      env->exitHandler = (ExitHandler_type) reg[1];
    }
    reg[1] = previous;
    break;

  case 12:   // Unused SWI?
    handler_not_implemented(reg[0]);
    break;

  case 13:   // Exception registers?
    handler_not_implemented(reg[0]);
    break;

  case 14:   // Application space
    previous = env->realEndOfMemory;
    if (reg[1]) {
      env->realEndOfMemory = reg[1];
    }
    reg[1] = previous;
    break;

  case 15:   // Currently active object
    handler_not_implemented(reg[0]);
    break;

  case 16:   // UpCall?
    handler_not_implemented(reg[0]);
    break;

  default:
    handler_not_implemented(reg[0]);
    break;
  }

  if (DEBUG) {
    printf("%08x %08x %08x %08x\r\n", reg[0], reg[1], reg[2], reg[3]);
  }

}
