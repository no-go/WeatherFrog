//#include <Adafruit_GFX.h>
#include <TFT.h>

struct Pix {
  byte x,y,lastX,lastY;
  void hopp() {
    lastX = x;
    lastY = y; 
  } 
};

Pix secPos;
Pix minPos;
Pix hourPos;

void analogClock(byte x, byte y, Adafruit_GFX & dis, int hours, int minutes, int seconds) {
  byte radius = 40;
  int hour = hours;
  if (hour>12) hour-=12;
  secPos.x = x + (radius-1)*cos(PI * ((float)seconds-15.0) / 30);
  secPos.y = y + (radius-1)*sin(PI * ((float)seconds-15.0) / 30);
  minPos.x = x + (radius-8)*cos(PI * ((float)minutes-15.0) / 30);
  minPos.y = y + (radius-8)*sin(PI * ((float)minutes-15.0) / 30);
  hourPos.x =x + (radius-14)*cos(PI * ((float)hour-3.0) / 6);
  hourPos.y =y + (radius-14)*sin(PI * ((float)hour-3.0) / 6);

  // remove old
  dis.drawLine(x, y, secPos.lastX, secPos.lastY, GREY);
  dis.drawLine(x, y, minPos.lastX, minPos.lastY, GREY);
  dis.drawLine(x, y, hourPos.lastX, hourPos.lastY, GREY);

  // draw new ones
  dis.drawLine(x, y, secPos.x, secPos.y, YELLOW);
  dis.drawLine(x, y, minPos.x, minPos.y, GREEN);
  dis.drawLine(x, y, hourPos.x, hourPos.y, RED);
  secPos.hopp();
  minPos.hopp();
  hourPos.hopp();

  // dots
  for (byte i=0; i<12; ++i) {
    dis.drawPixel(x + (radius-3)*cos(PI * ((float)i) / 6), y + (radius-3)*sin(PI * ((float)i) / 6), WHITE);  
  }

  dis.setCursor(x-5,y-radius+4);
  dis.print(12);
  dis.setCursor(x-2,y+radius-11);
  dis.print(6);
  dis.setCursor(x+radius-9,y-3);
  dis.print(3);
  dis.setCursor(x-radius+6,y-3);
  dis.print(9);
}

void digiClock(int x, int y, Adafruit_GFX & dis, int hours, int minutes) {
  dis.setCursor(x, y);
  if (hours<10) dis.print('0');
  dis.print(hours);
  dis.print(':');
  
  if (minutes<10) dis.print('0');
  dis.print(minutes);
}

