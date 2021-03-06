/*
 * 6502 Co Processor Emulation
 *
 * (c) 2015 David Banks
 * 
 * based on code by
 * - Reuben Scratton.
 * - Tom Walker
 *
 */

#define JTAG_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "startup.h"
#include "spi.h"
#include "tube-lib.h"
#include "tube-isr.h"
#include "rpi-gpio.h"
#include "rpi-aux.h"
#include "rpi-interrupts.h"
#include "tuberom_6502.h"
#include "copro-65tube.h"

volatile unsigned int debug;

unsigned int histogram_memory[0x100];

void copro_65tube_init_histogram() {
  int i;
  for (i = 0; i < 256; i++) {
    histogram_memory[i] = 0;
  }
}

void copro_65tube_dump_histogram() {
  int i;
  for (i = 0; i < 256; i++) {
    printf("%02x %u\r\n", i, histogram_memory[i]);
  }
}

void copro_65tube_init_hardware()
{
#ifdef JTAG_DEBUG
  // See http://sysprogs.com/VisualKernel/tutorials/raspberry/jtagsetup/
  RPI_SetGpioPinFunction(RPI_GPIO4, FS_ALT5);		// TDI
  RPI_SetGpioPinFunction(RPI_GPIO22, FS_ALT4);		// nTRST
  RPI_SetGpioPinFunction(RPI_GPIO23, FS_ALT4);		// RTCK
  RPI_SetGpioPinFunction(RPI_GPIO24, FS_ALT4);		// TDO
  RPI_SetGpioPinFunction(RPI_GPIO25, FS_ALT4);		// TCK
  RPI_SetGpioPinFunction(RPI_GPIO27, FS_ALT4);		// TMS
#endif

  // Write 1 to the LED init nibble in the Function Select GPIO
  // peripheral register to enable LED pin as an output
  RPI_GpioBase->LED_GPFSEL |= LED_GPFBIT;

  // Configure our pins as inputs
  RPI_SetGpioPinFunction(IRQ_PIN, FS_INPUT);
  RPI_SetGpioPinFunction(NMI_PIN, FS_INPUT);
  RPI_SetGpioPinFunction(RST_PIN, FS_INPUT);

  // Configure GPIO to detect a falling edge of the IRQ, NMI, RST pins
  RPI_GpioBase->GPFEN0 |= IRQ_PIN_MASK | NMI_PIN_MASK | RST_PIN_MASK;

  // Make sure there are no pending detections
  RPI_GpioBase->GPEDS0 = IRQ_PIN_MASK | NMI_PIN_MASK | RST_PIN_MASK;

  // Enable gpio_int[0] which is IRQ 49
  RPI_GetIrqController()->Enable_IRQs_2 = (1 << (49 - 32));

  // Initialise the UART
  RPI_AuxMiniUartInit( 57600, 8 );

  // Setup SP
  spi_begin();
}

static void copro_65tube_reset() {
  // Wipe memory
  memset(mpu_memory, 0, 0x10000);
  // Re-instate the Tube ROM on reset
  memcpy(mpu_memory + 0xf800, tuberom_6502_orig, 0x800);
}

int copro_65tube_tube_read(uint16_t addr, uint8_t data) {
  //if (addr >= 0xfef8 && addr < 0xff00) {
  data = tubeRead(addr & 7);
  //} else {
  //  data = mpu_memory[addr];
  //}
  //printf("rd: %04x %02x\r\n", addr, data);
  return data;
}

int copro_65tube_tube_write(uint16_t addr, uint8_t data)	{
  //if (addr >= 0xfef8 && addr < 0xff00) {
  tubeWrite(addr & 7, data);
  if ((addr & 7) == 6) {
    debug = data & 1;
    if (data & 2) {
      copro_65tube_dump_histogram();
      copro_65tube_init_histogram();      
    }
  }
  //} else {
  //  mpu_memory[addr] = data;
  //}
  //printf("wr: %04x %02x\r\n", addr, data);
  return 0;
}

int copro_65tube_trace(unsigned char *addr, unsigned char data) {
  if (debug) {
    printf("%04x %02x\r\n", (addr - mpu_memory), data);
  }
  return 0;
}

void copro_65tube_dump_mem(int start, int end) {
  int i;
  for (i = start; i < end; i++) {
    if (i % 16 == 0) {
      printf("\r\n%04x ", i);
    }
    printf("%02x ", mpu_memory[i]);
  }
  printf("\r\n");
}

void copro_65tube_main() {
  copro_65tube_init_hardware();

  printf("Raspberry Pi 65Tube Client\r\n" );

  enable_MMU_and_IDCaches();
  _enable_unaligned_access();

  printf("Initialise UART console with standard libc\r\n" );

  // This ensures interrupts are not re-enabled in tube-lib.c
  in_isr = 1;

  while (1) {
    debug = 0;
    // Wait for reset to go high
    while ((RPI_GpioBase->GPLEV0 & RST_PIN_MASK) == 0);
    printf("RST!\r\n");
    // Reinitialize the 6502 memory
    copro_65tube_reset();
    // Start executing code, this will return when reset goes low
    exec_65tube(mpu_memory);
    // Dump memory
    // copro_65tube_dump_mem(0x0000, 0x0400);
  }
}
