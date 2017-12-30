namespace VCC {
  
  const float MAX = 4100.0;
  const float MID = 3300.0;
  const float MIN = 3180.0;

  int get() {
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(10); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring
    int vcc = ADCL; 
    vcc |= ADCH<<8; 
    vcc = 1126400L / vcc;
    return vcc;
  }
};
