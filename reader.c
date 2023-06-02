#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

static int verbose = 0;
static int fd = 0;
static char buf[100] = {0};
static char parsed[sizeof(100)] = {0};
static const char * sdir = "./";
static const char * spath = "/dev/ttyACM0";
static volatile int shouldrun = 1;

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
    		if(verbose)
    			printf("parse(%d): %s\n", i, parsed);
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

int main(int argc, char *argv[])
{
	int i = 0, r = 0, t = 0;
	
   	while((i = getopt(argc, argv, "s:p:v")) != -1)
   	{
   		switch(i)
   		{
   		case 'v':
   			verbose = 1;
   			break;
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

    fd = open (spath, O_RDONLY);
    if (fd < 0)
    {
    	printf("failed to open socket: %s\n\n", strerror(errno));
        return 1;
    }
    printf("connected to \"%s\"\n", spath);
 
    while(shouldrun)
    {
    	r = read(fd, &buf[t], sizeof(buf) -1);
    	if(r < 0)
    	{
            continue;
    	}
    	else
    	{
    		t += r;
    		if(t >= sizeof(buf))
    		{
    			t = 0;
    			continue;
    		}
    	}
    	
    	if(t < 1)
    		continue;
    	if(buf[0] == '[' && buf[t-2] == ']')
    	{
    		buf[t-3] = 0;
    		memmove(buf, &buf[1], t-2);
    		
    		if(verbose)
    			printf("recevied: %s\n", buf);
    		
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
    
    if(fd > 0)
        close(fd);
    return 0;
}
