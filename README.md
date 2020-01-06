# SSD1306GFX
This library is developed for making games and other low latency interactives on the Atmel ATTiny85 Microcontroller and 128x64px SSD1306 OLED screen. 


## The problem
This chip has only 512 bytes of ram, not enough for a full screen buffer. My solution is to employ a single pixel column buffer ( 64bits ), the draw loop then iterates over all draw instructions for each column of the screen. This is computationaly more constly than using a buffer, but when clocked at 8Mhz this leaves plenty of clock cycles for game code.
