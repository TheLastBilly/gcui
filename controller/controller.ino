#include <SoftwareSerial.h>

#define BAUDRATE        9600

#define BATTERY_NOMINAL 58.8
#define BATTERY_DEAD    (BATTERY_NOMINAL*0.2)
#define BATTERY_DIV_R1  1000
#define BATTERY_DIV_R2  100
#define BATTERY_REAL(v) ((v*(BATTERY_DIV_R1+BATTERY_DIV_R2))/BATTERY_DIV_R2)
#define BATTERY_PIN     A2

#define SPEEDOMETER_MAX 4.275
#define SPEEDOMETER_MIN 0.82
#define SPEEDOMETER_PIN A2

#define THERMISTOR_R1   1000
#define THERMISTOR_C1   1.009249522e-03
#define THERMISTOR_C2   2.378405444e-04
#define THERMISTOR_C3   2.019202697e-07
#define THERMISTOR_PIN  A3

#ifndef ADC_RESOLUTION
#define RESOLUTION(x)   (0x01 << ((x)))
#define ADC_RESOLUTION  RESOLUTION(10)
#endif

#define REFERENCE_VOLTAGE 5.0

static float battery_level = 0.f, speedometer = 0.f, temperature = 0.f;

float
mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  float r = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  return min(max(r, out_min), out_max);
}


float
read_voltage(int pin)
{
  return ((float)analogRead(pin))/1024.0 * REFERENCE_VOLTAGE;
}

float 
read_temperature( void )
{
  int rT = analogRead(THERMISTOR_PIN);
  float r2 = THERMISTOR_R1 * (ADC_RESOLUTION / (float)rT - 1.0);
  float logR2 = log(r2);
  float t = (1.0 / (THERMISTOR_C1 + THERMISTOR_C2*logR2 + THERMISTOR_C3*logR2*logR2*logR2));
  float tc = t - 273.15;
  float tf = (tc * 9.0)/ 5.0 + 32.0; 
  return tf;
}

SoftwareSerial gps(5, 6);

void setup() {
  gps.begin(9600);
  Serial.begin(BAUDRATE);
}

void loop() {
  battery_level = BATTERY_REAL(read_voltage(BATTERY_PIN));
  battery_level = mapfloat(battery_level, BATTERY_DEAD, BATTERY_NOMINAL,
    0.f, 1.0f);
  speedometer = mapfloat(read_voltage(SPEEDOMETER_PIN), SPEEDOMETER_MIN, SPEEDOMETER_MAX,
    0.f, 1.f);
  temperature = read_temperature();
  Serial.print("[");
  Serial.print(battery_level);
  Serial.print(",");
  Serial.print(speedometer);
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",");
  Serial.print("]");
  Serial.println("");
  delay(1);
}