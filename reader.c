#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

int fd = 0;
static char buf[100] = {0};
static char parsed[sizeof(100)] = {0};
const char * sdir = "./";
const char * spath = "/dev/ttyACM0";
volatile int shouldrun = 1;

extern int errno;

static int
parse( int i )
{
	int s = 0, e = 0;
	
	while(buf[e] > 0 && e < sizeof(buf))
	{
		if(buf[e] != ',')
		{
			e++;
			continue;
		}
		
		if(i <= 0)
		{
			memmove(parsed, &buf[s], e - s + 1);
			parsed[e-s] = 0;
			return 0;
		}
		else
		{
			i--;
			s = e + 1;	
		}
		
		e++;
	}
	
	return 0;
}


static int
swrite( const char * s, const char * str )
{
    FILE * fp = NULL;
    static char path[100] = {0};

	snprintf(path, sizeof(path) - 1, "%s/%s",
		sdir, s);
	
    fp = fopen(path, "w");
    if(fp == NULL)
        return errno;
    
    fprintf(fp, str);

    fclose(fp);

    return 0;
}

int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                exit(1);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

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
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                exit(1);
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                exit(1);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
            exit(1);
}

static inline void
sigint(int n)
{
	shouldrun = 0;
}

int main(int argc, char *argv[])
{
	int i = 0, r = 0, t = 0;
	
	signal(SIGINT, sigint);
   	while((i = getopt(argc, argv, "s:p:")) != -1)
   	{
   		switch(i)
   		{
   		case 's':
   			spath = optarg;
   			break;
   		case 'p':
   			sdir = optarg;
   			break;
   		default:
   			break;
   		}
   	}

recon:
    fd = open (spath, O_RDWR | O_SYNC | O_NONBLOCK);
    if (fd < 0)
    {
    	printf("failed to open socket: %s\n", strerror(errno));
        return 1;
    }

    set_interface_attribs(fd, B9600, 0);
    set_blocking(fd, 0);

    usleep ((7 + 25) * 100);
    
    while(shouldrun)
    {
    	r = read(fd, &buf[t], sizeof(buf) - t);
    	if(r < 0)
    	{
    		r = 0;
    		//printf("failed to read from socket: %s\n", strerror(errno));
    		close(fd);
    		goto recon;
    	}
    	else
    	{
    		t += r;
    		if(t >= sizeof(buf))
    		{
    			t = 0;
    			tcflush(fd, TCIOFLUSH);
    			continue;
    		}
    	}
    	
    	if(t < 1)
    		continue;
    	
    	if(buf[0] == '[' && buf[t-3] == ']')
    	{
    		buf[t-3] = 0;
    		memmove(buf, &buf[1], t-2);
    		parse(0);
    		swrite("battery", parsed);
    		parse(1);
    		swrite("speed", parsed);
    		parse(2);
    		swrite("temperature", parsed);
    	}
    	else
    	{
    		tcflush(fd, TCIOFLUSH);
    	}
    	t = 0;
    }
	
    return 0;
}
