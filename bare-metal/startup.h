// startup.h

#ifndef STARTUP_H
#define STARTUP_H

#include <setjmp.h>
#include "tube-env.h"

/* Found in the *start.S file, implemented in assembler */

extern void _start( void );

extern void _enable_interrupts( void );

extern void _disable_interrupts( void );

extern void _user_exec(volatile unsigned char *address);

extern void _error_handler_wrapper(ErrorBuffer_type *eb, EnvironmentHandler_type errorHandler);

extern void _escape_handler_wrapper(unsigned int escapeFlag, EnvironmentHandler_type escapeHandler);

extern void _exit_handler_wrapper(unsigned int r12, EnvironmentHandler_type exitHandler);

extern unsigned int _get_cpsr();

#endif
