#include <M5StickCPlus2.h>
#include "UNIT_MiniJoyC.h"
#include "tet.h"

#define Disp M5.Lcd
#define BITMAP M5.Lcd.drawBitmap(0,0,135,240,tet)
#define POS_X 0
#define POS_Y 1

#define offsetx 14     //pixel offset X
#define offsety 20     //pixel offset Y
#define blockSize 11  //block size
#define fieldx 10     //how many blocks wide (Standard is 10)
#define fieldy 20     //how many blocks tall (Standard is 20)

int piece[7][4][2][4]={{{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}},{{0,1,0,1},{0,0,1,1}}},
                        {{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}},{{0,-1,0,1},{0,0,1,1}},{{0,1,1,0},{0,0,-1,1}}},
                        {{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}},{{0,-1,0,1},{0,0,-1,-1}},{{0,0,1,1},{0,-1,0,1}}},
                        {{{1,0,-1,1},{0,0,0,-1}},{{0,-1,0,0},{0,0,1,2}},{{0,1,2,0},{0,0,0,1}},{{1,0,0,0},{1,1,0,-1}}},
                        {{{1,0,0,0},{-1,-1,0,1}},{{1,1,0,-1},{1,0,0,0}},{{-1,0,0,0},{1,1,0,-1}},{{0,0,1,2},{-1,0,0,0}}},
                        {{{1,0,-1,0},{0,0,0,1}},{{0,0,0,-1},{-1,0,1,0}},{{1,0,-1,0},{0,0,0,-1}},{{0,0,0,1},{-1,0,1,0}}},
                        {{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}},{{0,0,0,0},{-1,0,1,2}},{{-1,0,1,2},{0,0,0,0}}}};
// piece[block][rot][y/x][4 spots]

int score;
byte multiplier;
byte digit[10][7];
#define dLength 4
const int cLength = dLength*7;
int pc;
int rot;
int block;
int color;
int level;
byte music;
long unsigned speed;
long unsigned timez;
long unsigned downHold,leftHold,rightHold;
boolean collide;
boolean game;
boolean release;
int posX;
int posY;
int a, b, c, f, g, h,i,j,k;
int linePos[4];
  int linez;
    int blockposY;
    int blockposX;
  int lineCount;
  
int stage;

UNIT_JOYC Joystick;

int pins[8];
boolean buttons[8];
boolean hold[8];

byte field[fieldy][fieldx];

void setup() {
  
  M5.begin();
  posX=4;
  posY=4;

  pinMode(35,INPUT_PULLUP);
  pinMode(37,INPUT_PULLUP);
  pinMode(39,INPUT_PULLUP);

  Disp.setRotation(0);
  BITMAP;
  Disp.fillRect(0,0,135,18);
  //M5.Lcd.setTextSize(2);
  //M5.Lcd.setFont(&fonts::FreeSerifBoldItalic9pt7b);
  //M5.Lcd.drawString("C+2",37,77);
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawString("v.1.7.0",4,8,1);
  M5.Lcd.drawString((String)M5.Power.getBatteryLevel()+"%",107,8,1);
  M5.Lcd.drawString("Loading...",39,134,1);
  while (!(Joystick.begin(&Wire, JoyC_ADDR, 0, 26, 100000UL))) {
    delay(100);
    Serial.println("I2C Error!\r\n");
  }
  M5.Lcd.fillRect(39,134,100,10,BLACK);
  M5.Lcd.drawString(" Press M5 ",39,134,1);
  while (digitalRead(37)==1) {
    delay(30);
  }
  delay(300);

  Disp.fillScreen(BLACK);
  M5.Lcd.drawLine(0,16,135,16,WHITE);
}

void loop(){
  speed=1500;
  timez=millis();
  score=0;
  level=0;
  stage=0;
  newPiece(0);
  game=1;
  showField(0,fieldy,1);
  while(game){
    collide=1;
    multiplier=1;
    scoreBoardWrite(score);
      while(collide){
        wait();
        if(collide) moveDown();
      }
    if(game!=0)
    newPiece(1);
  }
  for(a=0;a<fieldy;a++){
    for(b=0;b<fieldx;b++){
      field[a][b]=0;
    }
  }
  delay(1500);
  M5.Lcd.drawString("            ",36,135,1); //Erase Reset msg
  M5.Lcd.drawString("Sco.:00     ",10,4,1);
}

//==============================JoyC==============================
int check_Btn(){
  if (Joystick.getButtonStatus()==0){
    return 0;
  }
  return 1;
}
int check_Up(){
  if (Joystick.getADCValue(POS_Y)>2950){
    return 0;
  }
  return 1;
}
int check_Right(){
  if ((Joystick.getADCValue(POS_X)>2950)){
    return 0;
  }
  return 1;
}
int check_Left(){
  if ((Joystick.getADCValue(POS_X)<1350)){
    return 0;
  }
  return 1;
}
int check_Down() {
  if ((Joystick.getADCValue(POS_Y)<1600)){
    return 0;
  }
  return 1;
}
//================================================================

void blockDraw(int y, int x, int colorz){
  colorz%=96;
  int red=0;
  int gre=0;
  int blu=0;
  if(colorz<32){
    red=31-colorz;
    gre=colorz;
  }
  if(colorz>=32&&colorz<64){
    colorz-=32;
    gre=31-colorz;
    blu=colorz*2;
  }
  if(colorz>=64){
    colorz-=64;
    blu=(31-colorz)*2;
    red=colorz;
  }
  int returnz=(red*2048)+(gre*64)+blu;
  int posx=(x*blockSize)+offsetx;
  int posy=(y*blockSize)+offsety;
  Disp.drawRect(posx,posy,blockSize-1,blockSize-1,returnz);
  red/=2;
  gre/=2;
  blu/=2;
  returnz=(red*2048)+(gre*64)+blu;
  Disp.fillRect(posx+1,posy+1,blockSize-3,blockSize-3,returnz);
  Disp.drawPixel(posx,posy,WHITE);
  Disp.drawPixel(posx+1,posy+1,48599);
  Disp.drawPixel(posx+2,posy+1,48599);
  Disp.drawPixel(posx+1,posy+2,48599);
}

void bloff(int y,int x, int brightz){
  Disp.fillRect((x*blockSize)+offsetx,(y*blockSize)+offsety,blockSize-1,blockSize-1,2113*brightz);
}

void clearLines(){
  linez=0;
  for(a=0;a<fieldy;a++){
    byte counter=0;
    for(b=0;b<fieldx;b++){
      counter+=field[a][b]>0;
    }
    if(counter==fieldx){
      linePos[linez]=a;
      linez++;
    }
  }
  if(linez==1){
    level++;
    score+=multiplier;
    for(a=0;a<fieldx;a++){
      for(b=31;b>=0;b-=1){
        bloff(linePos[0],a,b);
        delay(1);
      }
    }
    delay(200);
  }
  
  if(linez==2){
    level+=2;
    score+=3*multiplier;
    for(a=0;a<fieldx;a++){
      bloff(linePos[0],a,31);
      bloff(linePos[1],a,31);
      delay(50);
    }
    for(a=0;a<50;a++){
      bloff(linePos[random(2)],random(fieldx),0);
      delay(15);
    }
  }
  
  if(linez==3){
    level+=3;
    score+=8*multiplier;
    for(a=0;a<fieldx;a++){
      for(b=0;b<3;b++){
        blockDraw(linePos[b],a,(10*b));
      }
      delay(50);
    }
    for(a=0;a<fieldx;a++){
      for(b=0;b<3;b++){
        bloff(linePos[b],a,0);
      }
      delay(50);
    }
  }
  
  if(linez==4){
    level+=4;
    score+=20*multiplier;
    for(a=0;a<192;a+=1){
      for(b=linePos[0];b<linePos[3]+1;b++){
        for(c=0;c<fieldx;c++){
          blockDraw(b,c,(3*c)+(b*8)+a);
        }
      }
      delay(10);
    }
  }
  for(a=0;a<linez;a++){  //clear
    for(c=0;c<fieldx;c++){
      field[linePos[a]][c]=0;
    }
    for(b=linePos[a];b>0;b--){
      for(c=0;c<fieldx;c++){
        field[b][c]=field[b-1][c];
      }
    }
  }
  lineCount+=linez;
  if(lineCount>9){
    lineCount-=10;
    (speed*4)/5;
  }
  showField(0,fieldy,0);
  if(level>9){
    stage++;
    level-=10;
    speed=((speed*4)/5);
  }
  for(int a=0;a<linez;a++){
    for(int b=0;b<fieldx;b++){
      field[a][b]=0;
    }
  }
  timez=millis();
}
void newPiece(boolean setPiece){
  if(setPiece){
      for(a=0;a<4;a++){
        field[posY+piece[block][rot][0][a]][posX+piece[block][rot][1][a]]=color;
      }
    clearLines();
  }
  posY=1;
  posX=4;
  rot=0;
  block=random(7);
  color=random(96)+1;
  showField(0,20,1);
  if(game!=0){
    game=test(0,0);    
  }
}

void wait(){
  timez+=speed;
  while(millis()<(timez+speed)){
    if(millis()>(leftHold+150)){hold[0]=0;}
    if(millis()>(downHold+150)){hold[1]=0;}
    if(millis()>(rightHold+150)){hold[2]=0;}
    for(int d=0;d<8;d++){
      buttons[d]=0;
      ButtonLoop(d);
      if(!pins[d]&&!hold[d]){
        buttons[d]=1;
        hold[d]=1;
        delay(25);
      }
      if(pins[d]&&hold[d]){
        hold[d]=0;
        delay(25);
      }
    }
    if(buttons[0]){moveLeft();leftHold=millis();}
    if(buttons[1]){moveDown();downHold=millis();}
    if(buttons[2]){moveRight();rightHold=millis();}
    if(buttons[3]){plummet();}
    if(buttons[4]){rotateLeft();}
    if(buttons[5]){rotateRight();}
    if(buttons[6]){pausegame();}
    if(buttons[7]){reset();}
  }
}

void moveLeft(){
  posX-=test(0,-1);
  showField(posY-2,posY+3,1);
}

void moveDown(){
  if(test(1,0)){
    posY++;
    showField(posY-3,posY+3,1);
  } else{
    collide=0;
  }
  timez=millis();
}

void moveRight(){
  posX+=(test(0,1));
  showField(posY-2,posY+3,1);
}

void plummet(){
  multiplier=2;posY;
  while(collide){
    moveDown();
    delay(12);
  }
  timez-=speed;
}

void rotateLeft(){
  rot+=4;
  rot--;
  rot%=4;
  if(!test(0,0)){
    rot++;
    rot%=4;
  }
  showField(posY-2,posY+3,1);
}

void rotateRight(){
  rot++;
  rot%=4;
  if(!test(0,0)){
    rot+=4;
    rot--;
    rot%=4;
  }
  showField(posY-2,posY+3,1);
}

void pausegame(){
  delay(200);
  M5.Lcd.drawString("(P)",61,4,1);
  while (digitalRead(37)==1) {
  Serial.println("Game paused :'v");
  delay(100);
  }
  M5.Lcd.drawString("   ",61,4,1);
  M5.Lcd.drawString("Sco.:0"+String(score),10,4,1);
}

void reset(){
  game = 0;
  collide = 0;
  M5.Lcd.drawString("Resetting...",36,135,1);
}

void showField(int startz, int finish,boolean showpiece){
  startz=max(0,startz);
  finish=min(fieldy,finish);
  for(f=startz;f<finish;f++){
    for(g=0;g<fieldx;g++){
      int colorz=field[f][g];
      if(colorz){blockDraw(f,g,colorz);}
      if(!colorz){bloff(f,g,0);}
    }
  }
  if(showpiece){
    for(f=0;f<4;f++){ // grid max
      blockposY=posY+piece[block][rot][0][f];
      blockposX=posX+piece[block][rot][1][f];

      blockDraw(blockposY,blockposX,color);
    }
  }
}

boolean test(int y, int x){
  boolean hitz=0;
  int blockposY[4];
  int blockposX[4];
  
  for(a=0;a<4;a++){ // grid max
    blockposY[a]=posY+y+piece[block][rot][0][a];
    if(blockposY[a]<0){hitz=1;}
    if(blockposY[a]>(fieldy-1)){hitz=1;}
    blockposX[a]=posX+x+piece[block][rot][1][a];
    if(blockposX[a]<0){hitz=1;}
    if(blockposX[a]>(fieldx-1)){hitz=1;}
  }
  
  for(a=0;a<4;a++){ // collision
    int collide=field[blockposY[a]][blockposX[a]];
    if(collide>0){hitz=1;}
  }
  return(!hitz);
}

void ButtonLoop(int _btn){
  switch (_btn) {
  case 0:
    pins[0]=check_Left();
  break; 
  case 1:
    pins[1]=check_Down();
  break;
  case 2:
    pins[2]=check_Right();
  break;
  case 3:
    pins[3]=check_Btn();
  break;
  case 4:
    pins[4]=digitalRead(35);
  break;
  case 5:
    pins[5]=check_Up();
  break;
  case 6:
    pins[6]=digitalRead(37);
  break;
  case 7:
    pins[7]=digitalRead(39);
  break;
  default:
  break;
  }
}

void scoreBoardWrite(int scorez){
  //M5.Lcd.drawLine(0,16,135,16,WHITE);
  M5.Lcd.drawString("Sco.:0"+String(score),10,4,1);
  M5.Lcd.drawString("Level:"+String(level),84,4,1);
}