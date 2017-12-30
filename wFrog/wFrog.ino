#include <TFT.h>
#include <SPI.h>
#include <Wire.h>
#include <MsTimer2.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// ----------------- CONFIG --------------------

/* Wireing TFT ST7735 (128x160) -> Arduino 
 *  cs    = PIN 10
 *  dc    = PIN 6
 *  reset = PIN 5
 *  vcc   = 3.3V
 *  GND = GND
 *  SDA = PIN 11 (MOSI Data  SPI)
 *  SCL = PIN 13 (SCLK Clock SPI)
 */
TFT display = TFT(10, 6, 5);

// Wireing Buttons to set clock: PIN 8 and PIN 7
// -> If you press it, 8 or 7 are low (connected to ground)
#define HOUR_BTN   8
#define MINU_BTN   7

// Wireing Bosch Sensor on Arduino I2C: SDA = A4, SCL = A5
Adafruit_BMP280 bmp;

#define TEMPMOD    -3.0 // temperature is a bit wrong ?!
#define STOREAGE   120  // 120 bytes
#define STOREEVERY 12   // store a byte every 12 minutes

// --------------------------------------------

#include "myColors.h"
#include "Pictogram.h"
#include "VCC.h"
#include "myClocks.h"
#include "myHack.h"
#include "PowerBar.h"

int hours    = 0;
int minutes  = 0;
int seconds  = 0;

bool dostore = false;

char stri[21];

int  mode    = 0;
int  altmode = 2;

struct Midways {
  byte _basic;
  byte _val[STOREAGE];
  int  _nxt;

  Midways(float initval) {
    _nxt = 0;
    _basic = flo2by(initval);
    for (int i=0; i<STOREAGE; ++i) { 
      _val[i] = _basic;
    }
  }

  void add(float val) {
    _val[_nxt] = flo2by(val);
    _nxt++;
    if (_nxt == STOREAGE) {
      _nxt = 0;
    }
  }

  float last() {
    int l = _nxt -1;
    if (l < 0) l += STOREAGE;
    return by2flo(_val[l]);
  }

  float oneHour() {
    int l = _nxt -6;
    if (l < 0) l += STOREAGE;
    return by2flo(_val[l]);
  }

  float twoHours() {
    int l = _nxt -11;
    if (l < 0) l += STOREAGE;
    return by2flo(_val[l]);
  }
  
  float treeHours() {
    int l = _nxt -16;
    if (l < 0) l += STOREAGE;
    return by2flo(_val[l]);
  }

  float midget() {
    float mid = 0;
    for (int i=0; i<STOREAGE; ++i) mid += by2flo(_val[i]);
    
    return mid/(float)STOREAGE;
  }

  void draw(int x, float fak, short col, bool isHpa, Adafruit_GFX & dis) {
    int y = dis.height()/2;
    
    int id = _nxt-1;
    float mid = midget();
    byte lastx,lasty;

    byte maxX=0;
    byte maxY=255;
    int  maxI=0;
    byte minX=255;
    byte minY=0;
    int  minI=STOREAGE-1;
    
    byte dx = x + STOREAGE;
    short dy = y - fak*(by2flo(_val[id]) - mid);
    
    for (int i=0; i<STOREAGE; ++i) {
      lastx = dx;
      lasty = dy;
      
      dx = x+STOREAGE-i;
      dy = y - fak*(by2flo(_val[id]) - mid);
      if (dy < 0)   dy = 0;
      if (dy > (dis.height()-2)) dy = dis.height()-2;

      if (dy > minY) {
        minY = dy;
        minI = id;
        minX = dx;
      }
      if (dy < maxY) {
        maxY = dy;
        maxI = id;
        maxX = dx;
      }
      dis.drawLine(lastx,   lasty,   dx,   dy,   col);
      dis.drawLine(lastx+1, lasty+1, dx+1, dy+1, col);
      id--;
      if (id < 0) id += STOREAGE;
    }

    //  print max value at the end of the graph (-30) 
    dis.setCursor(x + STOREAGE - 30, 0);
    dis.drawLine(maxX, maxY-1, x + STOREAGE - 30, 4, BLUE);
    if (isHpa) {
      dis.print(by2flo(_val[maxI]));
    } else {
      dis.print(hpa2temp(by2flo(_val[maxI])));      
    }

    //  print min value at the end of the graph (-30) 
    dis.setCursor(x + STOREAGE - 30, 106);
    dis.drawLine(minX, minY+1, x + STOREAGE - 30, 110, BLUE);
    if (isHpa) {
      dis.print(by2flo(_val[minI]));
    } else {
      dis.print(hpa2temp(by2flo(_val[minI])));      
    }

  }
};

Midways * hpa; // pressure
Midways * cel; // celsius


void tick() {
  seconds++;
  
  // debugging
  //dostore = true;
  
  if (seconds%3 == 0) {
    altmode = mode;
    mode++;
  }
  if (mode > 2) {
    mode = 0;    
  }
  
  if (seconds >= 60) {
    minutes++; seconds = seconds%60;
    if (minutes%STOREEVERY == 0) dostore = true;
  }
  if (minutes == 60) {
    hours++;
    minutes = 0;
  }
  if (hours >= 24) {
    hours = hours%24;
  }
}



byte weatherForcast(
  int x, int y,
  float hpa0,
  float hpa1,
  float hpa2,
  float hpa3,
  bool withText=true
) {
  if ( (hpa0 - hpa1) > 1.0 ) {
    if ( (hpa0 - hpa3) > 3.3 ) {
      display.drawBitmap(x, y, pictogram[5], 16,10, CYAN);
      if(withText) display.print(F("sun+breezy"));
      return 5;
    }
    display.drawBitmap(x, y, pictogram[3], 16,10, CYAN);
    if(withText) display.print(F("+ sunny"));
    return 3;
  }
  
  if ( (hpa0 - hpa1) < -1.0 ) {
    if ( (hpa0 - hpa3) < -3.3 ) {
      display.drawBitmap(x, y, pictogram[0], 16,10, GREYBLUE);
      if(withText) display.print(F("rain+storm"));
      return 0;
    }
    display.drawBitmap(x, y, pictogram[1], 16,10, LIGHTBLUE);
    if(withText) display.print(F("+ clouds"));
    return 1;
  }

  if (hpa0 < 1009) {
    display.drawBitmap(x, y, pictogram[0], 16,10, BLUE);
    if(withText) display.print(F("= rainy"));
    return 0;
    
  } else if (hpa0 < 1013) {
    display.drawBitmap(x, y, pictogram[1], 16,10, CYAN);
    if(withText) display.print(F("= cloudy"));
    return 1;
    
  } else if (hpa0 < 1014.5) {
    display.drawBitmap(x, y, pictogram[3], 16,10, WHITE);
    if(withText) display.print(F("unsettled"));
    return 3;
    
  } else if (hpa0 < 1016) {
    display.drawBitmap(x, y, pictogram[4], 16,10, YELLOW);
    if(withText) display.print(F("= sunny"));
    return 4;
    
  } else {
    display.drawBitmap(x, y, pictogram[5], 16,10, YELLOW);
    if(withText) display.print(F("= sun!"));
    return 5;  
  }
}



inline void clearD() {
  display.background(0, 0, 0);
  display.stroke(255, 255, 255);
  display.setTextSize(1);
}

void setup() {
  pinMode(HOUR_BTN, INPUT_PULLUP);
  pinMode(MINU_BTN, INPUT_PULLUP);
  
  display.begin();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (!bmp.begin()) {  
    // Could not find a valid BMP280 sensor, check wiring!
    while (1);
  }
  
  for (byte i=0; i<6; ++i) {
    clearD();
    display.drawBitmap(72, 60, pictogram[i], 16,10, WHITE);
    delay(500);
  }
  
  hpa = new Midways(bmp.readPressure()/100.0);
  cel = new Midways(temp2hpa(bmp.readTemperature() + TEMPMOD));

  MsTimer2::set(1000, tick);
  MsTimer2::start();
}


void loop() {
  
  if (dostore == true) {
    hpa->add( bmp.readPressure()/100.0 );
    cel->add( temp2hpa(bmp.readTemperature() + TEMPMOD) );
    dostore = false;
  }

  if (altmode != mode) {
    clearD();
    altmode=mode;
    if (mode == 0) {
      display.fillCircle(115, 50, 40, GREY);
      display.drawCircle(115, 50, 40, WHITE);
    }
  }
  display.setCursor(0, 0);
  
  if (mode == 1) { // -----------------------------------hPa
    display.stroke(0, 255, 0);
    dtostrf(bmp.readPressure()/100, 6, 1, stri);
    display.print(F("act: "));
    display.println(stri);
    display.print(F("mid: "));
    display.print(hpa->midget());
    display.setCursor(50, 119);
    display.print(F("Pressure"));

    hpa->draw(2, 2.25, GREEN, true, display);
    
    display.stroke(65, 65, 65);
    digiClock(0, 119, display, hours, minutes);
    
    PowerBar::print(
      display.width()-8,
      display.height() - PowerBar::LENGTH,
      VCC::get(),
      VCC::MIN,
      VCC::MAX,
      display
    );
    
    weatherForcast(
      144, 0,
      hpa->last(),
      hpa->oneHour(),
      hpa->twoHours(),
      hpa->treeHours(),
      false
    );
                  
  } else if (mode == 2) { // ----------------------------------- temperature
    display.stroke(0, 125, 255);
    dtostrf(bmp.readTemperature() + TEMPMOD, 5, 1, stri);
    display.print(F("act: "));
    display.print(stri);
    display.println(F(" C"));
    display.print(F("mid: "));
    display.print(hpa2temp(cel->midget()));
    display.println(F(" C"));
    display.setCursor(45, 119);
    display.print(F("Temperature"));

    cel->draw(2, 4.0, ORANGE, false, display);
    
    display.stroke(65, 65, 65);
    digiClock(0, 119, display, hours, minutes);
    
    PowerBar::print(
      display.width()-8,
      display.height() - PowerBar::LENGTH,
      VCC::get(),
      VCC::MIN,
      VCC::MAX,
      display
    );
    
    weatherForcast(
      144, 0,
      hpa->last(),
      hpa->oneHour(),
      hpa->twoHours(),
      hpa->treeHours(),
      false
    );
    
  } else {
    dtostrf(bmp.readPressure()/100, 6, 1, stri);
    display.print(stri);
    display.println(F(" hPa"));

    dtostrf(bmp.readTemperature() + TEMPMOD, 5, 1, stri);
    display.print(stri);
    display.print(F(" C"));

    display.setCursor(0, 30);
    weatherForcast(
      144, 0,
      hpa->last(),
      hpa->oneHour(),
      hpa->twoHours(),
      hpa->treeHours()
    );
    
    display.stroke(255, 255, 255);
    display.setTextSize(4);
    digiClock(20, 97, display, hours, minutes);
    
    display.setTextSize(1);
    analogClock(115, 50, display, hours, minutes, seconds);
  }

  if (digitalRead(HOUR_BTN) == LOW) {
    hours = (hours+1)%24;
    seconds=0;
    delay(300);
    clearD();
  }
  if (digitalRead(MINU_BTN) == LOW) {
    minutes = (minutes+1)%60;
    seconds=0;
    delay(300);
    clearD();
  }

}


