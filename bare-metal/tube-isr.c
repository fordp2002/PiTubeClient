#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "tube-isr.h"
#include "tube-lib.h"

volatile unsigned char *address;

volatile unsigned char escFlag;
volatile unsigned char errNum;
volatile char errMsg[256];
volatile int in_isr;

jmp_buf errorRestart;

extern void _isr_longjmp(jmp_buf env, int val);

void TubeInterrupt(void) {

  in_isr = 1;

  // Check for R1 interrupt
  if (tubeRead(R1_STATUS) & A_BIT) {
    if (DEBUG) {
      printf("R1 irq\r\n");
    }
    unsigned char flag = tubeRead(R1_DATA);
    if (flag & 0x80) {
      // Update escape flag
      escFlag = flag & 0x40;
      if (DEBUG) {
        printf("Escape flag = %02x\r\n", escFlag);
      }
    } else {
      // Event
      unsigned char y = receiveByte(R1);
      unsigned char x = receiveByte(R1);
      unsigned char a = receiveByte(R1);
      if (DEBUG) {
        printf("Event = %02x %02x %02x\r\n", a, x, y);
      }
    }
  }

  // Check for R4 interrupt
  if (tubeRead(R4_STATUS) & A_BIT) {
    if (DEBUG) {
      printf("R4 irq\r\n");
    }
    unsigned char type = tubeRead(R4_DATA);
    if (DEBUG) {
      printf("R4 type = %02x\r\n",type);
    }
    if (type == 0xff) {
      // Error
      receiveByte(R2); // 0
      errNum = receiveByte(R2);
      receiveString(R2, 0x00, errMsg);
      if (DEBUG) {
        printf("Error = %02x %s\r\n", errNum, errMsg);
      }
	  sendString(R1, errMsg);
	  sendString(R1, "\n\r");
	  // TODO will eventually call a propert SWI
	  in_isr = 0;
	  _isr_longjmp(errorRestart, 1);
    } else {
      unsigned char id = receiveByte(R4);
      if (DEBUG) {
        printf("Transfer = %02x %02x", type, id);
      }
      if (type <= 4 || type == 6 || type == 7) {
        unsigned char a3 = receiveByte(R4);
        unsigned char a2 = receiveByte(R4);
        unsigned char a1 = receiveByte(R4);
        unsigned char a0 = receiveByte(R4);
		address = (unsigned char *)((a3 << 24) + (a2 << 16) + (a1 << 8) + a0);
        if (DEBUG) {
          printf(" %08x", (unsigned int) address);
        }
      }
      if (type < 4 ) {
        // Type 0 .. 3 transfers serviced by NMI
        // TODO setup NMI
      }
      if (type == 6) {
        // Type 6 transfers are 256 byte block p -> h
        // TODO write 256 bytes to R3
        sendByte(R4, 0x00);
      }
      if (type == 7) {
        // Type 7 transfers are 256 byte block h -> p
        // TODO read 256 bytes from R3
      }
      if (type == 5) {
        // Type 5 : tube release
      } else {
        // Every thing else has a sync byte
        unsigned char sync = receiveByte(R4);
        if (DEBUG) {
          printf(" %02x", sync);
        }
      }
      if (DEBUG) {
        printf("\r\n");
      }
    }
  }
  in_isr = 0;
}
