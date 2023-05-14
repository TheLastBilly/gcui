#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#define SUPPORT_BUSY_WAIT_LOOP
#include "raylib.h"

#define uchar								unsigned char

#define MIN(x, y)      						(((x) <= (y) ? (x) : (y)))
#define MAX(x, y)      						(((x) > (y) ? (x) : (y)))
#define COLOR(_r, _g, _b)					(Color){(_r), (_g), (_b), 255}
#define HEX2RGBA(hex)     					\
(Color){                                  	\
    (uchar)(0xff & ((hex) >> 24)),   		\
    (uchar)(0xff & ((hex) >> 16)),   		\
    (uchar)(0xff & ((hex) >> 8)),    		\
    (uchar)(0xff & ((hex)))          		\
}

#define LIGHT_DARK_COLOR					HEX2RGBA(0x222222ff)
#define DARK_COLOR							BLACK
#define LIGHT_COLOR							WHITE

extern int errno;

static const char * sdir = NULL;
char buf[100] = {0};
Color fg = LIGHT_COLOR, bg = LIGHT_DARK_COLOR;
static int theme = 0;

static int
isdir( const char * p )
{
	struct stat ps = {0};
	if(stat(p, &ps) != 0)
		return 0;
	return S_ISDIR(ps.st_mode);
}

static int
sread( const char * s )
{
    long fsize = 0, r = 0;
    FILE * fp = NULL;

	snprintf(buf, sizeof(buf) - 1, "%s/%s",
		sdir, s);
	
    fp = fopen(buf, "r");
    if(fp == NULL)
        return errno;
    
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    fsize = MIN(fsize, sizeof(buf));
    while(r < fsize)
        r += fread(&buf[r], sizeof(buf[0]), fsize, fp);

    fclose(fp);

    return 0;
}

static inline void
switchdark( void )
{
	fg = LIGHT_COLOR;
	bg = LIGHT_DARK_COLOR;
}

static inline void
switchlight( void )
{
	fg = DARK_COLOR;
	bg = LIGHT_COLOR;
}

int
main( int argc, char *argv[] )
{
    time_t tt = {0};
    struct tm * ti = NULL;
    
	float f = 0, framerate = 30.0;
	long l = 0;
	const char * smetric = "mph", * tmetric = "°";
	int i = 0, wwidth = 640,wheight = 480, fontsize = 0, 
		w = 0, mspeed = 80;
	
	const float sangle = 30.0, eangle = 360.0 - sangle;
	
	sdir = "./";
   	while((i = getopt(argc, argv, "s:x:y:f:")) != -1)
   	{
   		switch(i)
   		{
   		case 's':
   			if(!isdir(optarg))
   			{
   				printf("\"%s\" is not a directory\n", optarg);
   				return -ENOENT;
  			}
   			sdir = optarg;
   			break;
   		case 'x':
   			wwidth = strtol(optarg, NULL, 10);
   			break;
   		case 'y':
   			wheight = strtol(optarg, NULL, 10);
   			break;
   		case 'm':
   			mspeed = strtol(optarg, NULL, 10);
   			break;
   		case 'a':
   			smetric = optarg;
   			break;
   		case 'f':
   			framerate = MAX(0.0, strtof(optarg, NULL));
   		default:
   			break;
   		}
   	}
   	
   	InitWindow(wwidth, wheight, "gcui");
   	SetTargetFPS(framerate);
   	
   	while(!WindowShouldClose())
   	{
        BeginDrawing();
        
        ClearBackground(bg);
        
        // speedometer
        sread("speed");
        l = MIN(strtol(buf, NULL, 10), mspeed);
        snprintf(buf, sizeof(buf), "%ld", l);
        fontsize = wwidth/6;
        w = MeasureText(buf, fontsize);
        DrawText(buf, wwidth/2 - w/2, wheight/2 - fontsize/2, fontsize, fg);			// speed
        fontsize = fontsize/4;
        w = MeasureText(smetric, fontsize);
        DrawText(smetric, wwidth/2 - w/2, wheight/2 + fontsize*4/3, fontsize, fg);		// metric
        DrawRing((Vector2){.x = wwidth/2, .y = wheight/2}, wheight/2, 					// ring
        	wheight/2 - wheight/20, -sangle, -(sangle + (eangle - sangle)*((float)l/(float)mspeed)), 100, fg);
        	
        // temperature
        sread("temperature");
        f = strtof(buf, NULL);
        fontsize = wwidth/20;
        snprintf(buf, sizeof(buf), "%.1f%s", f, tmetric);
        DrawText(buf, fontsize/2, wheight - fontsize - (wheight * .40) * .2, fontsize, fg);
        DrawRing(((Vector2){.x = 0, .y = wheight + fontsize}), wheight * .45, wheight * .40,
        	0.0, 360.0, 100, fg);
        
        // battery
        sread("battery");
        f = MIN(MAX(0.0, strtof(buf, NULL)), 1.0);
        fontsize = wwidth/20;
        snprintf(buf, sizeof(buf), "%d%%", (int)(f*100));
        w = MeasureText(buf, fontsize);
        DrawText(buf, wwidth - w - fontsize/2, wheight - fontsize - (wheight * .40) * .2, fontsize, fg);
        DrawRing(((Vector2){.x = wwidth, .y = wheight + fontsize}), wheight * .45, wheight * .40,
        	0.0, 360.0, 100, fg);
        	
        // clock
        time(&tt);
	    ti = localtime(&tt);
	
	    snprintf(buf, sizeof(buf), 
	        "%02d:%02d %s",	
	        ti->tm_hour%12,
	        ti->tm_min,
	        ti->tm_hour >= 12 ? "PM" : "AM"
	    );
        fontsize = wheight/10;
        w = MeasureText(buf, fontsize);
        //DrawRectangleV((Vector2){.x =  wwidth/2 - w*1.25/2, .y = wheight/2 + fontsize * 2.3},
        //	(Vector2){.x =  w*1.25, .y = fontsize * 1.25}, fg);
        //DrawText(buf, wwidth/2 - w/2, wheight/2 + fontsize * 2.5, fontsize, bg);
        DrawText(buf, wwidth/2 - w/2, wheight/2 + fontsize * 2.5, fontsize, fg);
	    
        // switch dark/light mode
        sread("theme");
        i = strtol(buf, NULL, 10) == 1;
        if(theme != i)
        {
        	theme = i;
        	if(theme)
        		switchlight();
        	else
        		switchdark();
        }
		
        EndDrawing();
   	}
   	
   	CloseWindow();
   	
	return 0;
}