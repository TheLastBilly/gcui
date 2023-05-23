#ifndef __UART_H__
#define __UART_H__

#include <stdlib.h>

int uart_open( const char * port, int speed );
int uart_is_open();
int uart_close( void );
int uart_recv( char * buffer, size_t size );

#endif