// based on the youtube tutorial by 3DSage youtube.com/@3DSage

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#include "assets/textures.ppm"
#include "assets/sky.ppm"

#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2

typedef struct
{
  int w,a,d,s;
}ButtonKeys; ButtonKeys Keys;
int depth[120];      //hold wall line depth to compare for sprite depth

int mapX=8,mapY=8,mapS=64;
int mapW[]=          //walls
{
 1,1,1,1,1,3,1,1,
 1,0,0,1,0,0,0,1,
 1,0,0,5,0,2,0,1,
 1,1,5,1,0,0,0,1,
 2,0,0,0,0,0,0,1,
 2,0,0,0,0,6,0,1,
 2,0,0,0,0,0,0,1,
 1,1,3,1,3,1,3,1,	
};

int mapF[]=          //floors
{
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,
 5,5,5,5,5,5,5,5,	
};

int mapC[]=          //ceiling
{
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,	
};

typedef struct
{
 int type; //static, key, enemy, etc.
 int state; //on off
 int map; //texture to show
 int x,y,z;
}sprite; sprite sp[4];

void drawSprite()
{
 float sx=sp[0].x-px; //temp float variables
 float sy=sp[0].y-py;
 float sz=sp[0].z;

 float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotate around origin
 float a=sy*CS+sx*SN;
 float b=sx*CS-sy*SN;
 sx=a; sy=b;

 sx=(sx*108.0/sy)+(120/2); //convert to screen x,y
 sy=(sz*108.0/sy)+( 80/2);

 glPointSize(8); glColor3ub(255,255,0); glBegin(GL_POINTS); glVertex2i(sx*8,sy*8); glEnd();
}

void drawMap2D()
{
  int x,y,xo,yo;
  for(y=0;y<mapY;y++)
  {
    for(x=0;x<mapX;x++)
    {
    if(mapW[y*mapX+x]>0){ glColor3f(1,1,1);} else{ glColor3f(0,0,0);}
      xo=x*mapS; yo=y*mapS;
      glBegin(GL_QUADS);
      glVertex2i(xo     +1, yo     +1);
      glVertex2i(xo     +1, yo+mapS-1);
      glVertex2i(xo+mapS-1, yo+mapS-1);
      glVertex2i(xo+mapS-1, yo     +1);
      glEnd();
    }
  }
}

//--------PLAYER--------
float degToRad(float a) { return a*M_PI/180.0;}
float FixAng(float a) { if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}

float px,py,pdx,pdy,pa; //player position

void drawPlayer()
{
  glColor3f(1,1,0);
  glPointSize(8);
  glLineWidth(4);
  glBegin(GL_POINTS);
  glVertex2i(px,py);
  glEnd();

  glLineWidth(3);
  glBegin(GL_LINES);
  glVertex2i(px,py);
  glVertex2i(px+pdx*20, py+pdy*20);
  glEnd();
}

void drawRays3D()
{
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
 for(r=0;r<120;r++)
 {
  int vmt=0,hmt=0;                                                              //vertical and horizontal map texture number 
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
  else { rx=px; ry=py; dof=8;}                                                  //looking up or down. no hit  

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=8;}                                                   //looking straight left or right
 
  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  
  float shade=1;
  glColor3f(0,0.8,0);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}//horizontal hit first
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH); 
  float ty_step=32.0/(float)lineH; 
  float ty_off=0; 
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
  int lineOff = 320 - (lineH>>1);                                               //line offset

  depth[r]=disH; //save this line's depth
  //---draw walls---
  int y;
  float ty=ty_off*ty_step;//+hmt*32;
  float tx;
  if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
  else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
  for(y=0;y<lineH;y++)
  {
   int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
   int red   =All_Textures[pixel+0]*shade;
   int green =All_Textures[pixel+1]*shade;
   int blue  =All_Textures[pixel+2]*shade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y+lineOff); glEnd();
   ty+=ty_step;
  }
 
  //---draw floors---
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  int red   =All_Textures[pixel+0]*0.7;
  int green =All_Textures[pixel+1]*0.7;
  int blue  =All_Textures[pixel+2]*0.7;
  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y); glEnd();

 //---draw ceiling---
  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
  pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
  red   =All_Textures[pixel+0];
  green =All_Textures[pixel+1];
  blue  =All_Textures[pixel+2];
  if(mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,640-y); glEnd();}
 }
 
 ra=FixAng(ra-0.5);                                                               //go to next ray, 60 total
 }
}

void drawSky()
{int x,y;
 for(y=0;y<40;y++)
 {
  for(x=0;x<120;x++)
  {
    int xo=(int)pa*2-x; if(xo<0){ xo+=120;} xo=xo % 120;
    int pixel=(y*120+xo)*3;
    int red   =sky[pixel+0];
    int green =sky[pixel+1];
    int blue  =sky[pixel+2];
    glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
  }
 }
}

int gameState=0, timer=0;
float fade=0;

void screen(int v)
{int x,y;
 int *T;
 if(v==1){ T=title;}
 if(v==2){ T=won;}
 if(v==3){ T=lost;}
 for(y=0;y<80;y++)
 {
  for(x=0;x<120;x++)
  {
    int xo=(int)pa*2-x; if(xo<0){ xo+=120;} xo=xo % 120;
    int pixel=(y*120+xo)*3;
    int red   =T[pixel+0]*fade;
    int green =T[pixel+1]*fade;
    int blue  =T[pixel+2]*fade;
    glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
  }
 }
 if(fade<1){ fade+=0.001*fps;}
 if(fade>1){ fade=1;}
}

void init()
{
  glClearColor(0.3,0.3,0.3,0);

  px=300; py=300; pdx=cos(pa)*5; pdy=sin(pa)*5;
  mapW[19]=4; mapW[26]=4; //close doors

  sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=5*64; sp[0].z=0; //sprite 1
}

float frame1,frame2,fps;

void display()
{
  //frames per second
  frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

  if(gameState==0){ init(); timer=0; gameState=1;}
  if(gameState==1){ timer+=1*fps; if(timer>1000){ timer=0; gameState=2;}} //start screen
  if(gameState==3)
  {

   //buttons
   if(Keys.a==1){ pa+=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
   if(Keys.d==1){ pa-=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

   int xo=0; if(pdx<0){ xo=-20;} else{ xo=20;}                                    //x offset to check map
   int yo=0; if(pdy<0){ yo=-20;} else{ yo=20;}                                    //y offset to check map
   int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;             //x position and offset
   int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;             //y position and offset
   if(Keys.w==1)                                                                  //move forward
   {  
    if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*0.2*fps;}
    if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*0.2*fps;}
   }
   if(Keys.s==1)                                                                  //move backward
   { 
    if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*0.2*fps;}
    if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*0.2*fps;}
   } 

   //drawMap2D();
   //drawPlayer();
   drawSky();
   drawRays3D();
   drawSprite();
   if( (int)px>>6==1 && (int)py>>6==1 ){ fade=0; timer=0; gameState=3;}
  }

  if(gameState==3){ timer+=1*fps; if(timer>1000){ fade=0; timer=0; gameState=0;}} //won screen

  glutPostRedisplay();
  glutSwapBuffers();
}

void ButtonDown(unsigned char key,int x,int y)
{
  if(key=='a'){ Keys.a=1;}
  if(key=='d'){ Keys.d=1;}
  if(key=='w'){ Keys.w=1;}
  if(key=='s'){ Keys.s=1;}
  if(key=='e')
  {
    int xo=0; if(pdx<0){ xo=-25;} else{ xo=25;}
    int yo=0; if(pdy<0){ yo=-25;} else{ yo=25;}
    int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
    int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;
    if(mapW[ipy_add_yo*mapX+ipx_add_xo]==4){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
  }
  glutPostRedisplay();
}

void ButtonUp(unsigned char key,int x,int y)
{
  if(key=='a'){ Keys.a=0;}
  if(key=='d'){ Keys.d=0;}
  if(key=='w'){ Keys.w=0;}
  if(key=='s'){ Keys.s=0;}
  glutPostRedisplay();
}

void resize(int w,int h)
{
  glutReshapeWindow(960,640);
}

int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(960, 640);
  glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH)/2-960/2,glutGet(GLUT_SCREEN_HEIGHT)/2-640/2);
  glutCreateWindow("by wisdom a house is built, and through understanding it is established");
  gluOrtho2D(0,960,640,0);
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutKeyboardFunc(ButtonDown);
  glutKeyboardUpFunc(ButtonUp);
  glutMainLoop();
}
