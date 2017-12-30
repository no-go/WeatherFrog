//#include <Adafruit_GFX.h>
#include <TFT.h>

namespace PowerBar {

  const byte LENGTH = 42;
  
  short green2red(int val, int maxi) {
    // hint: blue green red in 16 bits = 5+6+5 (sometimes red green blue)
    short result = 0x0000;
    int redPart   = 0;
    int greenPart = 0;
    if (val-1 == (maxi/2)) {
      greenPart = 63;
      redPart   = 31;
    } else if (val > (maxi/2)) {
      greenPart = 63;
      redPart = 31 - 62 * ((float) val)/((float) maxi); // 31 = 0b11111
    } else {
      redPart = 31;
      greenPart = 127 * ((float) val)/((float) maxi); // 63 = 0b111111
    }
    //result += redPart  <<11; // (red green blue)
    result = redPart;          // (blue green red)
    result += greenPart<<5;
    return result;
  }
  
  void print(int x, int y, int vcc, float mini, float maxi, Adafruit_GFX & dis) {

    vcc = vcc/10;
    mini = mini/10.0;
    maxi = maxi/10.0;
    
    dis.drawPixel(x+3, y, WHITE);
    dis.drawPixel(x+4, y, WHITE);
    dis.drawRect(x, y+1, 8, LENGTH-1, WHITE); 
    
    byte vccVal = (LENGTH-3) * (float)((vcc-mini)/(maxi-mini));
    if (vcc < mini) vccVal=0;
    if (vcc > maxi) vccVal=LENGTH-3;
    
    dis.fillRect(x+1, y+2, 6, LENGTH-3-vccVal, BLACK);
    for (int v=vccVal; v>0; --v) {
      dis.drawLine(
        x+1, y+LENGTH -v-1,
        x+6, y+LENGTH -v-1,
        green2red(v, LENGTH)
      );
    }

    dis.setTextSize(1);
    dis.stroke(65, 65, 65);
    dis.setCursor(x-25, y-8);
    dis.print((float)vcc/100.0);
    dis.print('V');
    
  }
  
};

