//~ DEFINES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Some defines for the SSD1306 controller driving a 128x64 resolution OLED display
// PART 		- http://www.simplelabs.co.in/content/96-blue-i2c-oled-module
// DATASHEET 	- https://www.adafruit.com/datasheets/SSD1306.pdf

// The Slave Address (SLA) of the OLED controller - SSD1306 - is 0x3C 
// The LSB is supposed to be the mode. Since we are only going to WRITE to the OLED, the LSB is going to be 0
// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS   0x3C

// The SSD1306 datasheet (pg.20) says that a control byte has to be sent before sending a command
// Control byte consists of 
// bit 7		: Co   : Continuation bit - If 0, then it assumes all the next bytes are data (no more control bytes).
//				:		 You can send a stream of data, ie: gRAM dump - if Co=0
//				:        For Command, you'd prolly wanna set this - one at a time. Hence, Co=1 for commands
//				:		 For Data stream, Co=0 :)
// bit 6    	: D/C# : Data/Command Selection bit, Data=1/Command=0
// bit [5-0] 	: lower 6 bits have to be 0
#define OLED_CONTROL_BYTE_CMD_SINGLE	0x80
#define OLED_CONTROL_BYTE_CMD_STREAM	0x00
#define OLED_CONTROL_BYTE_DATA_STREAM	0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST			0x81	// follow with 0x7F
#define OLED_CMD_DISPLAY_RAM			0xA4
#define OLED_CMD_DISPLAY_ALLON			0xA5
#define OLED_CMD_DISPLAY_NORMAL			0xA6
#define OLED_CMD_DISPLAY_INVERTED 		0xA7
#define OLED_CMD_DISPLAY_OFF			0xAE
#define OLED_CMD_DISPLAY_ON				0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE	0x20	// follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE		0x21	// can be used only in HORZ/VERT mode - follow with 0x00 + 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE			0x22	// can be used only in HORZ/VERT mode - follow with 0x00 + 0x07 = PAGE7

#define OLED_CMD_ADDR_MODE_ROW 0b00
#define OLED_CMD_ADDR_MODE_COL 0b01
#define OLED_CMD_ADDR_MODE_PAGE 0b10

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE	0x40
#define OLED_CMD_SET_SEGMENT_REMAP		0xA1	
#define OLED_CMD_SET_MUX_RATIO			0xA8	// follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE		0xC8	
#define OLED_CMD_SET_DISPLAY_OFFSET		0xD3	// follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP		0xDA	// follow with 0x12

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV	0xD5	// follow with 0x80
#define OLED_CMD_SET_PRECHARGE			0xD9	// follow with 0x22
#define OLED_CMD_SET_VCOMH_DESELCT		0xDB	// follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP		0x8D	// follow with 0x14

// NOP
#define OLED_CMD_NOP 					0xE3

  enum SSD1306Gfx_BitwiseOp {
    Or, And, Xor
  };

class SSD1306Gfx {
  public: 
  static const uint8_t BlendModeAdd = 1<<3;
  static const uint8_t BlendModeXor = 1<<4;
  static const uint8_t BlendModeSubtract = 1<<5;
  static const uint8_t AllBlends = BlendModeAdd | BlendModeXor | BlendModeSubtract;

  static const uint8_t SpriteFlagFlipH = 1<<0;
  static const uint8_t SpriteFlagFlipV = 1<<1;
  static const uint8_t SpriteFlagUseProgmem = 1<<2;
  

  void init();
  void startVertical(uint8_t pageStart=0, uint8_t pageEnd=7, uint8_t colStart=0, uint8_t colEnd=0x7f);
  bool nextColumn();
  void drawBackground(uint8_t pattern, uint8_t offset);
  void drawBox(int16_t x,uint8_t y,uint8_t w,uint8_t h, uint8_t flags = 0);
  void drawSprite(int16_t x,uint8_t y,uint8_t w,uint8_t h, const uint8_t * pattern,uint16_t ptnLen=1, uint8_t flags = 0);
  void drawSpritePM(int16_t x,uint8_t y,uint8_t w,uint8_t h, const uint8_t * pattern,uint16_t ptnLen=1, uint8_t flags = 0);
  void drawText(uint8_t x, uint8_t y, char * str, const uint8_t font[], uint16_t startAddress=0x0);
  void drawDigit(uint8_t x, uint8_t y, uint8_t digit, const uint8_t font[], uint16_t startAddress=0x10);

  private:


  uint64_t colBuf;
  uint8_t colIndex;
  uint8_t pageStart;
  uint8_t pageEnd;
  void clear();

  static const uint8_t reverse_lookup[];
  static bool hasFlag(uint8_t flags, uint8_t flag);
  static uint8_t reverseBits(uint8_t n);
  
};
