#include <Arduino.h>
#include <functions.h>

void setup()
{
  Serial.begin(9600);

  set_pinMode_for_relay_OUTPUT();
  set_start_position_for_relay();

  lcd_init();

  set_pinMode_for_sensors_INPUT();
}

void loop()
{
  lcd_view();
  door();
  poliv();
}
