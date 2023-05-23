#include "uart.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

static int fd = -1;

int 
uart_open( const char * port, int speed )
{
    struct termios tty;
    
    uart_close();

	fd = open( port, O_RDWR | O_NOCTTY | O_SYNC);
    if( fd < 0 )
        return -1;
    

    if( tcgetattr( fd, &tty ) != 0 )
    {
    	uart_close();
        return -1;
    }
    cfsetospeed( &tty, speed );
    cfsetispeed( &tty, speed );
    
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	                                // no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
	
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	
	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	                                // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= 0;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

    if( tcsetattr( fd, TCSANOW, &tty ) != 0 )
    {
    	uart_close();
        return -1;
    }
    
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
    	uart_close();
        return -1;
	}
	
	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 5;
	
	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
    	uart_close();
        return -1;
	}
	
	usleep ((7 + 25) * 100);

    return 0;
}

int
uart_is_open( void )
{
	return fd > 0;
}

int
uart_close( void )
{
    if( fd < 1 )
    {
        close( fd );
        fd = -1;
 	}
    
    return 0;
}

int
uart_recv( char * buffer, size_t size )
{
    if( fd < 0 )
        return -1;
	
    //tcflush(fd, TCIOFLUSH);    
    return read( fd, buffer, size );
}