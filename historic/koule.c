#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#include <sys/soundcard.h>
#include "soundIt.h"
#define VGAMODE G640x480x256
#define back(x) (32 + x)
#define ball(x) (64 + x)
#define rocket(x) (96 + x)
#define PLAY_X1 0
#define PLAY_Y1 0
#define PLAY_X2 600
#define PLAY_Y2 400
#define MAPWIDTH (PLAY_X2-PLAY_X1)
#define MAPHEIGHT (PLAY_Y2-PLAY_Y1)
#define RAD(n)  ((float)(n)/180.0*M_PI)
#define ROTSTEP RAD(20)
#define ROCKET_SPEED 0.8
#define BALL_SPEED 0.3
#define SLOWDOWN 0.9
#define GUMM 5
#define BALLM 3
#define LBALLM 4
#define ROCKETM 5
#define BALL_RADIUS 8
#define ROCKET_RADIUS 10
#define EYE_RADIUS 4
#define EYE_RADIUS1 8

#define ROCKET 1
#define BALL 2
#define LBALL 3
#define CREATOR 4
#define MAXOBJECT 50
#define MAXPOINT 1000
#define MAXROCKETS 4
typedef struct {
        int type;
	int live;
	int time;
        float x;
        float y;
	float fx;	/*forces*/
	float fy;
	float rotation;	/*for rockets*/
	float M;
	int radius;
	float speed;
	float accel;
	char letter;
} Object;
typedef struct {
	int x,y,xp,yp,time,color;
	} Point;

int nobjects=8;
int nrockets=2;
Object object[MAXOBJECT];
Point point[MAXPOINT];


GraphicsContext *physicalscreen;
GraphicsContext *backscreen;
GraphicsContext *background;

char ball_data[BALL_RADIUS*2][BALL_RADIUS*2];
char *ball_bitmap;
char eye_data[MAXROCKETS][EYE_RADIUS*2][EYE_RADIUS*2];
char *eye_bitmap[MAXROCKETS];
char rocket_data[ROCKET_RADIUS*2][ROCKET_RADIUS*2];
char *rocket_bitmap;
int npoint=0;
#define BALL_XCENTER (3*BALL_RADIUS/4)
#define BALL_YCENTER (BALL_RADIUS/4)
#define BALL_MAX_RADIUS (BALL_RADIUS+0.5*BALL_RADIUS)
#define BALL_SIZE_MAX (MAX_RADIUS*MAX_RADIUS)
#define ROCKET_XCENTER (3*ROCKET_RADIUS/4)
#define ROCKET_YCENTER (ROCKET_RADIUS/4)
#define ROCKET_MAX_RADIUS (ROCKET_RADIUS+0.5*ROCKET_RADIUS)
#define ROCKET_SIZE_MAX (ROCKET_MAX_RADIUS*ROCKET_MAX_RADIUS)
#define EYE_XCENTER (3*EYE_RADIUS/4)
#define EYE_YCENTER (EYE_RADIUS/4)
#define EYE_MAX_RADIUS (EYE_RADIUS+0.5*EYE_RADIUS)
#define EYE_SIZE_MAX (EYE_MAX_RADIUS*EYE_MAX_RADIUS)
#define NCOLORS 32


#define L_ACCEL 'A'
#define L_SPEED 'S'
#define L_GUMM 'M'
#define L_MULT 1.2
#define LETTER 1024



#define S_START 0
#define S_END 1
#define S_COLIZE 2
#define S_DESTROY_BALL 3
#define S_DESTROY_ROCKET 4
#define NSAMPLES 5
char *files[NSAMPLES]=
 {
    "start.raw",
    "end.raw",
    "colize.raw",
    "destroy1.raw",
    "destroy2.raw"
 };

static void addpoint(int x,int y,int xp,int yp,int color,int time)
{
  point[npoint].x=x;
  point[npoint].y=y;
  point[npoint].xp=xp;
  point[npoint].yp=yp;
  point[npoint].time=time;
  point[npoint].color=color;
  npoint++;
  if(npoint>=MAXPOINT) npoint=0;
}
int cit=0;
#define NTRACKS 4
#define next ((++cit)>=NTRACKS?cit=0:cit)
static void create_bitmap()
{int x,y,r,size,po;
	printf("creating bitmaps...\n");
	for(x=0;x<BALL_RADIUS*2;x++)
	  for(y=0;y<BALL_RADIUS*2;y++)
		{
		 if((x-BALL_RADIUS)*(x-BALL_RADIUS)+(y-BALL_RADIUS)*(y-BALL_RADIUS)
			<BALL_RADIUS*BALL_RADIUS)
		 {
		 r=(x-BALL_XCENTER)*(x-BALL_XCENTER)+
		   (y-BALL_XCENTER)*(y-BALL_XCENTER);
		 r=r*NCOLORS/ROCKET_SIZE_MAX;
		 ball_data[x][y]=ball(r);
		 } else ball_data[x][y]=0;
		}
	printf("compiling bitmaps...\n");
	size = gl_compiledboxmasksize( BALL_RADIUS*2,BALL_RADIUS*2, ball_data );
	if((ball_bitmap=malloc(size))==NULL) perror("balls"),exit(1);
	gl_compileboxmask( BALL_RADIUS*2,BALL_RADIUS*2, ball_data, ball_bitmap );
	for(x=0;x<ROCKET_RADIUS*2;x++)
	  for(y=0;y<ROCKET_RADIUS*2;y++)
		{
		 if((x-ROCKET_RADIUS)*(x-ROCKET_RADIUS)+(y-ROCKET_RADIUS)*(y-ROCKET_RADIUS)
			<ROCKET_RADIUS*ROCKET_RADIUS)
		 {
		 r=(x-ROCKET_XCENTER)*(x-ROCKET_XCENTER)+
		   (y-ROCKET_XCENTER)*(y-ROCKET_XCENTER);
		 r=r*NCOLORS/ROCKET_SIZE_MAX;
		 rocket_data[x][y]=rocket(r);
		 } else rocket_data[x][y]=0;
		}
	printf("compiling bitmaps...\n");
	size = gl_compiledboxmasksize( ROCKET_RADIUS*2,ROCKET_RADIUS*2, rocket_data );
	if((rocket_bitmap=malloc(size))==NULL) perror("rocket"),exit(1);
	gl_compileboxmask( ROCKET_RADIUS*2,ROCKET_RADIUS*2, rocket_data, rocket_bitmap );
	for(po=0;po<MAXROCKETS;po++) {
	for(x=0;x<EYE_RADIUS*2;x++)
	  for(y=0;y<EYE_RADIUS*2;y++)
		{
		 if((x-EYE_RADIUS)*(x-EYE_RADIUS)+(y-EYE_RADIUS)*(y-EYE_RADIUS)
			<EYE_RADIUS*EYE_RADIUS)
		 {
		 r=(x-EYE_XCENTER)*(x-EYE_XCENTER)+
		   (y-EYE_XCENTER)*(y-EYE_XCENTER);
		 r=r*NCOLORS/EYE_SIZE_MAX;
		 eye_data[po][x][y]=32+32*po+r;
		 } else eye_data[po][x][y]=0;
		}
	printf("compiling bitmaps...\n");
	size = gl_compiledboxmasksize( EYE_RADIUS*2,EYE_RADIUS*2, eye_data[po] );
	if((eye_bitmap[po]=malloc(size))==NULL) perror("eye"),exit(1);
	gl_compileboxmask( EYE_RADIUS*2,EYE_RADIUS*2, eye_data[po], eye_bitmap[po] );
	}
}
static inline void normalize(float *x,float *y,float size)
{
	float length=sqrt((*x)*(*x)+(*y)*(*y));
	if(length==0) length=1;
	*x*=size/length;
	*y*=size/length;
}

static void setcustompalette() {
    /* colors 0-31 are a RGB mix (bits 0 and 1 red, 2 green, 3 and 4 blue) */
    /* 32-63    black to red */
	Palette pal;
	int i;
	for (i = 0; i < 64; i++) {
		int r, g, b;
		r = g = b = 0;
		if ((i & 32) > 0) b = (i & 31) << 1;
		if (i < 32) {
			r = (i & 3) << 4;   /* 2 bits */
			g = (i & 4) << 3;   /* 1 bit */
			b = (i & 24) << 1;  /* 2 bits */
		}
		pal.color[i].red = r;
		pal.color[i].green = g;
		pal.color[i].blue = b;
	}
	for (i = 64; i < 64+32; i++) {
		int r, g, b;
		r = g = b = 0;
		r = (32-(i - 63)) << 1;
		if(i<64+8) {
		 b=g=((32-(i - 63))) << 3;
		}
		pal.color[i].red = r;
		pal.color[i].green = g;
		pal.color[i].blue = b;
	}
	for (i = 96; i < 96+32; i++) {
		int r, g, b;
		r = g = b = 0;
		r = g = (32-(i - 95)) << 1;
		if(i<96+8) {
		 b=((32-(i - 95))) << 3;
		}
		pal.color[i].red = r;
		pal.color[i].green = g;
		pal.color[i].blue = b;
	}
	for (i = 128; i < 128+32; i++) {
		int r, g, b;
		r = g = b = 0;
		g = (32-(i - 127)) << 1;
		if(i<128+8) {
		 r = b=((32-(i - 127))) << 3;
		}
		pal.color[i].red = r;
		pal.color[i].green = g;
		pal.color[i].blue = b;
	}
	for (i = 160; i < 160+32; i++) {
		int r, g, b;
		r = g = b = 0;
		b = (32-(i - 159)) << 1;
		if(i<160+8) {
		 r = g=((32-(i - 159))) << 3;
		}
		pal.color[i].red = r;
		pal.color[i].green = g;
		pal.color[i].blue = b;
	}
	gl_setpalette(&pal);
}


Sample sa[NSAMPLES];
void loadsamples()
{
 int index;
    for (index=0; index<NSAMPLES; index++)
	{
		if(Snd_loadRawSample( files[index], &sa[index] ))
			perror(files[index]),exit(-1);
        }
	Snd_init( NSAMPLES, sa,11000, NTRACKS, "/dev/dsp" );
}

static void initialize(){
	void *font;
	loadsamples();
	vga_disabledriverreport();
	vga_init();
	vga_setmode(VGAMODE);
	gl_setcontextvga(VGAMODE);
	physicalscreen = gl_allocatecontext();
	gl_getcontext(physicalscreen);
	gl_setcontextvgavirtual(VGAMODE);
	backscreen = gl_allocatecontext();
	gl_getcontext(backscreen);
	gl_setcontextvgavirtual(VGAMODE);
	setcustompalette();
	background = gl_allocatecontext();
	gl_getcontext(background);
	font = malloc(256 * 8 * 8 );
	gl_expandfont(8, 8, back(3), gl_font8x8, font);
	gl_setfont(8, 8, font);
#ifdef USE_PAGEFLIPPING
	/* Try to enable page flipping. */
	gl_enablepageflipping(&physicalscreen);
#endif
	if (keyboard_init()) {
		printf("Could not initialize keyboard.\n");
		exit(-1);
	}
        /*keyboard_translatekeys(TRANSLATE_CURSORKEYS | TRANSLATE_KEYPADENTER);*/
        keyboard_translatekeys(0);
}
static void createbackground() {
/* Create fancy dark red background */
	int x, y;
	for (y = 0; y < MAPHEIGHT; y++)
		for (x = 0; x < MAPWIDTH; x++) {
			int i = 0;
			int n = 0;
			int c;
			if (x > 0) {
				i += gl_getpixel(x - 1, y) - back(0);
				n++;
			}
			if (y > 0) {
				i += gl_getpixel(x, y - 1) - back(0);
				n++;
			}
			c = (i + (random()%16)) / (n + 1);
			if (c > 9)
				c = 9;
			gl_setpixel(x, y, back(0) + c);
		}
}

static void drawbackground() {
/* Build up background from map data */
	gl_setcontext(background);
	gl_clearscreen(0);
	createbackground();
}



static void uninitialise()
{
	vga_setmode(TEXT);
}

static void init_objects()
{	int i;
	for(i=0;i<nobjects;i++)
	{	object[i].live=(i<nrockets?5:1);
		object[i].x=rand()%MAPWIDTH;
		object[i].y=rand()%MAPHEIGHT;
		object[i].fx=(rand()%10)/10.0;
		object[i].fy=(rand()%10)/10.0;
		object[i].rotation=0;
		object[i].type=(i<nrockets?ROCKET:BALL);
		object[i].M=(i<nrockets?ROCKETM:BALLM);
		object[i].radius=BALL_RADIUS;
		object[i].speed=2;
		object[i].accel=ROCKET_SPEED;
		object[i].letter=' ';
	}
}
static void move_objects()
{
	int i;
	for(i=0;i<nobjects;i++)
	  if(object[i].type==CREATOR){	
		object[i].time--;
		if(!object[i].time) {
			object[i].live=1,
			object[i].type=BALL;
		}
	  } else
	  if(object[i].live){	
		object[i].x+=object[i].fx;
		object[i].y+=object[i].fy;
	}

}

static void points()
{
	int i;
	for(i=0;i<MAXPOINT;i++)
	{
		if(point[i].time>0) {
			point[i].time--;
			point[i].x+=point[i].xp;
			point[i].y+=point[i].yp;
			if(point[i].x>0&&point[i].x>>8<MAPWIDTH&&
			   point[i].y>0&&point[i].y>>8<MAPHEIGHT) 
				gl_setpixel( point[i].x>>8,point[i].y>>8,point[i].color);
			else point[i].time=0;
		}
	}
}
char str[2];
static void draw_objects()
{
	int i;
        gl_setcontext(background);
        gl_copyscreen(backscreen);

        /* Now draw the objects in backscreen. */
        gl_setcontext(backscreen);

	points();
	for(i=0;i<nobjects;i++)
	  if(object[i].live){	
		if(object[i].type==BALL||object[i].type==LBALL) {
		  gl_putboxmaskcompiled( (int)object[i].x - BALL_RADIUS, (int)object[i].y - BALL_RADIUS, 
		  BALL_RADIUS * 2 ,BALL_RADIUS * 2 , ball_bitmap );
		  gl_setwritemode(WRITEMODE_MASKED);
		  str[0]=object[i].letter;
		  gl_write((int)object[i].x-4,(int)object[i].y-4, str);

		  }
		if(object[i].type==ROCKET)
		  {int x1,y1;
		  gl_putboxmaskcompiled( (int)object[i].x - ROCKET_RADIUS, (int)object[i].y - ROCKET_RADIUS, 
		  ROCKET_RADIUS * 2 ,ROCKET_RADIUS * 2 , rocket_bitmap );
		  x1=object[i].x+sin(object[i].rotation-RAD(30))*EYE_RADIUS1-EYE_RADIUS;
		  y1=object[i].y+cos(object[i].rotation-RAD(30))*EYE_RADIUS1-EYE_RADIUS;
		  gl_putboxmaskcompiled(  x1 , y1 ,
		  EYE_RADIUS * 2  ,EYE_RADIUS * 2 , eye_bitmap[i] );
		  x1=object[i].x+sin(object[i].rotation+RAD(30))*EYE_RADIUS1-EYE_RADIUS;
		  y1=object[i].y+cos(object[i].rotation+RAD(30))*EYE_RADIUS1-EYE_RADIUS;
		  gl_putboxmaskcompiled(  x1 , y1 ,
		  EYE_RADIUS * 2 ,EYE_RADIUS * 2 , eye_bitmap[i] );
		  }
	}

        /* Copy backscreen to physical screen. */
        gl_copyscreen(physicalscreen);
}
void explosion(int x,int y,int type)
{float i;
 int speed;
 int color;
	for(i=0;i<RAD(360);i+=RAD(2))
	{
	speed=rand()%1024;
	switch(type)
	{
		case BALL:color=ball(rand()%32);break;
		case ROCKET:color=rocket(rand()%32);break;
	}
	addpoint(x*256,y*256,
		sin(i)*(speed),
		cos(i)*(speed),
		color,
		rand()%1000);
	}
}
static void check_limit()
{
	int i;
	for(i=0;i<nobjects;i++)
	  if(object[i].live){	
		if(object[i].x-object[i].radius<0||object[i].x+object[i].radius>MAPWIDTH||
		   object[i].y-object[i].radius<0||object[i].y+object[i].radius>MAPHEIGHT)
			{
			switch(object[i].type) {
			case LBALL:
				Snd_effect( S_DESTROY_BALL, next );
				object[i].live=0,explosion(object[i].x,object[i].y,object[i].type);break;
			case BALL:
				Snd_effect( S_DESTROY_BALL, next );
				if(rand()%2==0)
				{
					if(object[i].x-object[i].radius<0||object[i].x+object[i].radius>MAPWIDTH)
						object[i].fx*=-1; else
						object[i].fy*=-1;
				object[i].type=LBALL;
				object[i].M=LBALLM;
		  	   	switch(rand()%2) {
					case 0:object[i].letter=L_ACCEL;break;
					case 1:object[i].letter=L_GUMM;break;
		  		}
				} else
					object[i].live=0,explosion(object[i].x,object[i].y,object[i].type);break;
				break;
			case ROCKET:
				Snd_effect( S_DESTROY_ROCKET, next );
				object[i].live--,explosion(object[i].x,object[i].y,object[i].type);
				if(object[i].live) {
					object[i].x=rand()%MAPWIDTH;
					object[i].y=rand()%MAPHEIGHT;
					object[i].fx=0;
					object[i].fy=0;
					object[i].rotation=0;
					object[i].type=ROCKET;
					object[i].speed=2;
					object[i].accel=ROCKET_SPEED;
				}
				break;
			}
			}
	}

}

static void process_keys()
{
	
                keyboard_update();

                /* Move. */
                if (keyboard_keypressed(SCANCODE_CURSORLEFT))
                        {object[0].rotation+=ROTSTEP;}
                if (keyboard_keypressed(SCANCODE_CURSORRIGHT))
                        object[0].rotation-=ROTSTEP;
                if (keyboard_keypressed(SCANCODE_CURSORUP)){
			float p;
			int i;
                        object[0].fx+=sin(object[0].rotation)*object[0].accel,
                        object[0].fy+=cos(object[0].rotation)*object[0].accel;
			for(i=0;i<5;i++) {
			p=RAD(rand()%45-22);
			addpoint(object[0].x*256,
			         object[0].y*256,
			         (object[0].fx-sin(object[0].rotation+p)*object[0].accel*10)*(rand()%512),
			         (object[0].fy-cos(object[0].rotation+p)*object[0].accel*10)*(rand()%512),
				 rocket(rand()%16),10);
			}
			}


                /* Move. */
                if (keyboard_keypressed(SCANCODE_CURSORBLOCKLEFT))
                        {object[1].rotation+=ROTSTEP;}
                if (keyboard_keypressed(SCANCODE_CURSORBLOCKRIGHT))
                        object[1].rotation-=ROTSTEP;
                if (keyboard_keypressed(SCANCODE_CURSORBLOCKUP)){
			float p;
			int i;
                        object[1].fx+=sin(object[1].rotation)*object[1].accel,
                        object[1].fy+=cos(object[1].rotation)*object[1].accel;
			for(i=0;i<5;i++) {
			p=RAD(rand()%45-22);
			addpoint(object[1].x*256,
			         object[1].y*256,
			         (object[1].fx-sin(object[1].rotation+p)*object[1].accel*10)*(rand()%512),
			         (object[1].fy-cos(object[1].rotation+p)*object[1].accel*10)*(rand()%512),
				 rocket(rand()%16),10);
			}
			}

}

static void creator()
{
 int time=rand()%50+1;
 int i;
 int z;
 int x,y;
	for(i=nobjects;i<MAXOBJECT&&(object[i].live||
		object[i].type==CREATOR);
		i++);
	if(i==MAXOBJECT) return;
	if(i>=nobjects) nobjects=i+1;
	object[i].live=0;
	object[i].x=rand()%MAPWIDTH;
	object[i].y=rand()%MAPHEIGHT;
	object[i].fx=0.0;
	object[i].fy=0.0;
	object[i].time=time;
	object[i].rotation=0;
	object[i].type=CREATOR;
	object[i].M=(i<nrockets?ROCKETM:BALLM);
	object[i].radius=BALL_RADIUS;
	object[i].speed=2;
	object[i].accel=ROCKET_SPEED;
	object[i].letter=' ';
	for(z=0;z<360;z++)
	{
	x=rand()%MAPWIDTH;
	y=rand()%MAPHEIGHT;
	addpoint(x*256,y*256,
		(object[i].x-x)*256/(time),
		(object[i].y-y)*256/(time),
		ball(rand()%32),
		time);
	}
}

static void update_forces()
{
	int i;
	int rocket=0;
	int r;
	float d;
	float xp,yp;
	if(!(rand()%120)) creator();
	for(i=0;i<nobjects;i++)
	{
	  if(object[i].live){	
	  	if(object[i].type==BALL||object[i].type==LBALL){	
			d=640*640;
			if(object[i].type==BALL&&!rand()%2048) {
				object[i].type=LBALL;
		  	   	switch(rand()%2) {
					case 0:object[i].letter=L_ACCEL;break;
					case 1:object[i].letter=L_GUMM;break;
		  		}
			}
			for(r=0;r<nrockets;r++)
			{
				if(object[r].live) {
				xp=object[r].x-object[i].x;
                        	yp=object[r].y-object[i].y;
				if(xp*xp+yp*yp<d) d=xp*xp+yp*yp,rocket=r;}

			}
			xp=object[rocket].x-object[i].x;
			yp=object[rocket].y-object[i].y;
			normalize(&xp,&yp,BALL_SPEED);
			object[i].fx+=xp;
			object[i].fy+=yp;
		}
	  /*if(object[i].fx*object[i].fx+
	     object[i].fy*object[i].fy>object[i].speed*object[i].speed)*/
		object[i].fx*=SLOWDOWN,
		object[i].fy*=SLOWDOWN;
	}
	}
}
static void colisions()
{	int i,y;
	int colize=0;
	float xp,yp;
	for(i=0;i<nobjects;i++)
	  if(object[i].live)
	     for(y=i+1;y<nobjects;y++)
	       if(object[y].live)
		{
		  xp=object[y].x-object[i].x;
		  yp=object[y].y-object[i].y;
		  if(xp*xp+yp*yp<(object[y].radius+object[i].radius)*
				 (object[y].radius+object[i].radius))
			{colize=1;
			if(object[i].type==ROCKET) {
				if(object[y].letter==L_ACCEL)
					object[i].accel*=L_MULT;
				if(object[y].letter==L_GUMM)
					object[i].M*=L_MULT;
				object[y].letter=' ';
				if(object[y].type==LBALL) object[y].type=BALL;
				}
			normalize(&xp,&yp,object[i].M/object[y].M*GUMM);
			object[y].fx+=xp;
			object[y].fy+=yp;
			normalize(&xp,&yp,object[y].M/object[i].M*GUMM);
			object[i].fx-=xp;
			object[i].fy-=yp;
			}
		}
	if(colize)
	Snd_effect( S_COLIZE, next );
}

static void game()
{
	Snd_effect( S_START, next );
	while(1) {
		process_keys();
		update_forces();
		colisions();
		move_objects();
		check_limit();
		draw_objects();
		usleep(5000);
	}
	Snd_effect( S_END, next );
}
int main()
{	
	create_bitmap();
	printf("initializing vgalib\n");
	initialize();
	printf("initializing objects\n");
	init_objects();
	printf("creating bitmaps\n");
	drawbackground();
	printf("playing game\n");
	game();
	printf("uninitializing\n");
	uninitialise();
	return 0;
}
