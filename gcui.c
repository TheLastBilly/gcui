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

#define SWAP(x, y, t)						({t _s = x; x = y; y = _s;})
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
extern const struct {
  int  	 			width;
  int  	 			height;
  int  	 			bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  
  unsigned char 	pixel_data[140 * 197 * 4 + 1];
} btlogo;

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
    Camera2D camera = {0};
    
   	Image bt = {0};
   	
    time_t tt = {0};
    struct tm * ti = NULL;
    
	long l = 0;
	float f = 0, framerate = 30.0, maxtemp = 100.0, mintemp = 0.0,
		tm = 0.0;
	const char * smetric = "mph", * tmetric = "°";
	int i = 0, wwidth = 640,wheight = 360, fontsize = 0,
		w = 0, mspeed = 80, fullscreen = 0, flipscreen = 0;
	
	const float sangle = 30.0, eangle = 360.0 - sangle;
	
	sdir = "./";
   	while((i = getopt(argc, argv, "s:x:y:ft:n:er:")) != -1)
   	{
   		switch(i)
   		{
   		case 'f':
   			flipscreen = 1;
   			break;
   		case 'e':
   			fullscreen = 1;
   			break;
   		case 's':
   			if(!isdir(optarg))
   			{
   				printf("\"%s\" is not a directory\n", optarg);
   				return -ENOENT;
  			}
   			sdir = optarg;
   			break;
   		case 'n':
   			mintemp = strtof(optarg, NULL);
   			break;
   		case 't':
   			maxtemp = strtof(optarg, NULL);
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
   		case 'r':
   			framerate = MAX(0.0, strtof(optarg, NULL));
   		default:
   			break;
   		}
   	}
   	
   	if(maxtemp < mintemp)
   		SWAP(maxtemp, mintemp, float);
   	tm = maxtemp - mintemp;
   	
   	InitWindow(wwidth, wheight, "gcui");
   	SetTargetFPS(framerate);
   	bt.width = btlogo.width;
   	bt.height = btlogo.height;
   	bt.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    bt.mipmaps = 1;
    bt.data = RL_MALLOC(sizeof(btlogo.pixel_data));
    memcpy(bt.data, btlogo.pixel_data, sizeof(btlogo.pixel_data));
    
    Texture2D bttexture = LoadTextureFromImage(bt);
    
    if(flipscreen)
    {
	    camera.offset = (Vector2){wwidth, wheight};
	    camera.rotation = 180;
	    camera.zoom = 1;
	}
    
    if(fullscreen)
    {
    	ToggleFullscreen();
    }
   	
   	while(!WindowShouldClose() && !IsKeyDown(KEY_Q))
   	{
        BeginDrawing();
        
        ClearBackground(bg);
	        
	    if(flipscreen)
	    {
        	BeginMode2D(camera);
       	}
        
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
        f = MAX(MIN(strtof(buf, NULL), maxtemp), mintemp);
        fontsize = wwidth/20;
        snprintf(buf, sizeof(buf), "%.1f%s", f, tmetric);
        DrawText(buf, fontsize/2, wheight - fontsize - (wheight * .40) * .2, fontsize, fg);
        DrawRing(((Vector2){.x = 0, .y = wheight + fontsize}), wheight * .45, wheight * .40,
        	100.0, 100.0 + 80.0*((f - mintemp)/tm), 100, fg);
        
        // battery
        sread("battery");
        f = MIN(MAX(0.0, strtof(buf, NULL)), 1.0);
        fontsize = wwidth/20;
        snprintf(buf, sizeof(buf), "%d%%", (int)(f*100));
        w = MeasureText(buf, fontsize);
        DrawText(buf, wwidth - w - fontsize/2, wheight - fontsize - (wheight * .40) * .2, fontsize, fg);
        DrawRing(((Vector2){.x = wwidth, .y = wheight + fontsize}), wheight * .45, wheight * .40,
        	-100.0, -100.0 - 80.0*f, 100, fg);
        	
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
        DrawText(buf, wwidth/2 - w/2, wheight/2 + fontsize * 2.5, fontsize, fg);
        
        // bt
        sread("bt");
        if(strtol(buf, NULL, 10) > 0)
        {
        	DrawTexturePro(bttexture, (Rectangle){.y = 0, .x = 0, .width = bt.width,
        		.height = bt.height}, (Rectangle){.y = wheight * 0.02, .x = wwidth - wwidth * 0.1, .height = wwidth * 0.1,
        		.width = wheight * 0.15}, (Vector2){0}, 0.0,WHITE);
       	}
	    
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