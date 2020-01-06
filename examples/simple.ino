#include <Arduino.h>
#include "ssd1306gfx.h"

SSD1306Gfx screen;

void setup(){
  screen.init();
}

void loop(){
  screen.startVertical();
  do
  {
  
  } while (screen.nextColumn());
}