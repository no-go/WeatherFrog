#define MODHPA   9400.0
#define FAKTOR   4


// some hacks, to store tempetature and hpa values more efficient in a single byte

/* 
 * Hack to convert a float from 940.0 to 1042.0 (+/- 0.25) in a byte 
 */
inline byte flo2by(float val) {
  byte res = 0;
  val = val * 10;
  val = val - MODHPA;
  res = val / FAKTOR;
  return res;
}

/*
 * Hack to convert hpa byte back to float
 */
inline float by2flo(byte val) {
  float res = val;
  res = res * FAKTOR;
  res = res + MODHPA;
  res = res / 10.0;
  return res;
}

/* 
 * Hack to convert a float from -25.0 to + 70.0 as "hpa" float value ;-D
 */
inline float temp2hpa(float val) {
  int q = val * 10;
  float res = map(q, -250, 700, 9400, 10420);
  return res/10.0;
}

/* 
 * Hack to convert a "hpa" float back to a value -25.0 to + 70.0 (tempetature)
 */
inline float hpa2temp(float val) {
  int q = val * 10;
  float res = map(q, 9400, 10420, -250, 700);
  return res/10.0;
}

