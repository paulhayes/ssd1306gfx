#include "stdint.h"
#include "ssd1306gfx.h"
#include "Wire.h"
#include <avr/pgmspace.h>
#include <string.h>
#include "font6x8.h"

void SSD1306Gfx::init()
{
    // Init the I2C interface (pins A4 and A5 on the Arduino Uno board) in Master Mode.
    Wire.begin();
    // keywords:
    // SEG = COL = segment = column byte data on a page
    // Page = 8 pixel tall row. Has 128 SEGs and 8 COMs
    // COM = row

    // Begin the I2C comm with SSD1306's address (SLA+Write)
    Wire.beginTransmission(OLED_I2C_ADDRESS);

    // Tell the SSD1306 that a command stream is incoming
    Wire.write(OLED_CONTROL_BYTE_CMD_STREAM);

    // Follow instructions on pg.64 of the dataSheet for software configuration of the SSD1306
    // Turn the Display OFF
    Wire.write(OLED_CMD_DISPLAY_OFF);
    // Set mux ration tp select max number of rows - 64
    Wire.write(OLED_CMD_SET_MUX_RATIO);
    Wire.write(0x3F);
    // Set the display offset to 0
    Wire.write(OLED_CMD_SET_DISPLAY_OFFSET);
    Wire.write(0x00);
    // Display start line to 0
    Wire.write(OLED_CMD_SET_DISPLAY_START_LINE);

    // Mirror the x-axis. In case you set it up such that the pins are north.
    // Wire.write(0xA0); - in case pins are south - default
    Wire.write(OLED_CMD_SET_SEGMENT_REMAP);

    // Mirror the y-axis. In case you set it up such that the pins are north.
    // Wire.write(0xC0); - in case pins are south - default
    Wire.write(OLED_CMD_SET_COM_SCAN_MODE);

    // Default - alternate COM pin map
    Wire.write(OLED_CMD_SET_COM_PIN_MAP);
    Wire.write(0x12);
    // set contrast
    Wire.write(OLED_CMD_SET_CONTRAST);
    Wire.write(0x7F);
    // Set display to enable rendering from GDDRAM (Graphic Display Data RAM)
    Wire.write(OLED_CMD_DISPLAY_RAM);
    // Normal mode!
    Wire.write(OLED_CMD_DISPLAY_NORMAL);
    // Default oscillator clock
    Wire.write(OLED_CMD_SET_DISPLAY_CLK_DIV);
    Wire.write(0x80);
    // Enable the charge pump
    Wire.write(OLED_CMD_SET_CHARGE_PUMP);
    Wire.write(0x14);
    // Set precharge cycles to high cap type
    Wire.write(OLED_CMD_SET_PRECHARGE);
    Wire.write(0x22);
    // Set the V_COMH deselect volatage to max
    Wire.write(OLED_CMD_SET_VCOMH_DESELCT);
    Wire.write(0x30);
    // Horizonatal addressing mode - same as the KS108 GLCD
    Wire.write(OLED_CMD_SET_MEMORY_ADDR_MODE);
    Wire.write(OLED_CMD_ADDR_MODE_COL);
    // Turn the Display ON
    Wire.write(OLED_CMD_DISPLAY_ON);

    // End the I2C comm with the SSD1306
    Wire.endTransmission();

    this->clear();
}

void SSD1306Gfx::startVertical(uint8_t pageStart, uint8_t pageEnd, uint8_t colStart, uint8_t colEnd)
{
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(OLED_CONTROL_BYTE_CMD_STREAM);
    Wire.write(OLED_CMD_SET_COLUMN_RANGE);
    Wire.write(0x00);
    Wire.write(0x7F);
    Wire.write(OLED_CMD_SET_PAGE_RANGE);
    Wire.write(pageStart);
    Wire.write(pageEnd);
    Wire.endTransmission();
    this->pageStart = pageStart;
    this->pageEnd = pageEnd + 1;
    this->colIndex = 0;
    this->colBuf = 0;
}

bool SSD1306Gfx::nextColumn()
{
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(OLED_CONTROL_BYTE_DATA_STREAM);
    uint8_t *buf = (uint8_t *)(&this->colBuf);
    uint8_t start = this->pageStart;
    uint8_t end = this->pageEnd;
    for (uint8_t i = start; i < end; i++)
    {
        Wire.write(buf[i]);
    }

    Wire.endTransmission();
    this->colBuf = 0;
    return (++this->colIndex) < 128;
}

void SSD1306Gfx::drawBackground(uint8_t pattern, uint8_t offset)
{
    ((uint8_t *)(&this->colBuf))[offset] = pattern;
}

void SSD1306Gfx::drawBox(int16_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t op)
{
    if ((x > this->colIndex) || ((x + w) <= (this->colIndex)) || (y >= (this->pageEnd * 8)) || (y + h) < (this->pageStart * 8))
    {
        return;
    }
    //uint8_t numRows = h / 8 + 1;
    x = this->colIndex - x;
    uint64_t box = ((~((uint64_t)0)) >> (64 - h)) << y;

    switch (op)
    {
    case SSD1306Gfx::BlendModeAdd:
        this->colBuf |= box;
        break;
    case SSD1306Gfx::BlendModeXor:
        this->colBuf ^= box;
        break;
    case SSD1306Gfx::BlendModeSubtract:
        this->colBuf &= ~box;
    }
}

void SSD1306Gfx::drawSprite(int16_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *pattern, uint16_t ptnLen, uint8_t flags)
{
    if ((x > this->colIndex) || ((x + w) <= (this->colIndex)) || (y >= (this->pageEnd * 8)) || (y + h) < (this->pageStart * 8))
    {
        return;
    }
    uint8_t numRows = h / 8 + 1;
    x = this->colIndex - x;
    uint64_t box = ((~((uint64_t)0)) >> (64 - h)) << y;
    for (int i = 0; i < numRows; i++)
    {
        uint8_t c;
        uint16_t hy = hasFlag(flags, SpriteFlagFlipV) ? numRows - i - 1 : i;
        uint16_t hx = hasFlag(flags, SpriteFlagFlipH) ? w - x - 1 : x;

        uint16_t n = (hx + w * hy);
        if (SSD1306Gfx::hasFlag(flags, SpriteFlagUseProgmem))
            c = pgm_read_byte(&pattern[n % ptnLen]);
        else
            c = pattern[n % ptnLen];

        if (hasFlag(flags, SpriteFlagFlipV))
        {
            c = reverseBits(c);
            if(h<8){
              c>>=(8-h);
            }
        }
        uint64_t sprite = box & ((uint64_t)(c) << (y + i * 8));

        
        switch (flags&AllBlends)
        {        
        case SSD1306Gfx::BlendModeXor:
            this->colBuf ^= sprite;
            break;
        case SSD1306Gfx::BlendModeSubtract:
            this->colBuf &= ~sprite;
        case SSD1306Gfx::BlendModeAdd:
        default:
        this->colBuf |= sprite;
        break;
        }
    }
}

/*
void SSD1306Gfx::drawSpritePM(int16_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t pattern[], uint16_t ptnLen)
{
    if ((x > this->colIndex) || ((x + w) <= (this->colIndex)) || (y >= (this->pageEnd * 8)) || (y + h) < (this->pageStart * 8))
    {
        return;
    }

    uint8_t numRows = h / 8 + 1;
    x = this->colIndex - x;
    uint64_t box = ((~((uint64_t)0)) >> (64 - h)) << y;
    for (int i = 0; i < numRows; i++)
    {
        uint8_t c = pgm_read_byte(&pattern[(x + w * i) % ptnLen]);
        this->colBuf |= box & ((uint64_t)(c) << (y + i * 8));
    }
}
*/

void SSD1306Gfx::drawSpritePM(int16_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t pattern[], uint16_t ptnLen, uint8_t flags){
  drawSprite(x,y,w,h,pattern,ptnLen, flags|SpriteFlagUseProgmem);
}

void SSD1306Gfx::drawText(uint8_t x, uint8_t y, char *str)
{
    int len = strlen(str);
    const int charWidth = 6;
    int w = len * charWidth;
    int sample = this->colIndex - x;
    int d = sample / charWidth;
    char c = str[d];
    this->drawSpritePM(x, y, w, 8, &ssd1306xled_font6x8[c * 6], 6);
}

void SSD1306Gfx::drawDigit(uint8_t x, uint8_t y, uint8_t digit)
{
    digit = digit % 10;
    const int charWidth = 6;
    //int sample = this->colIndex-x;
    //int d = sample/charWidth;
    char c = digit + 0x10;
    this->drawSpritePM(x, y, charWidth, 8, &ssd1306xled_font6x8[c * 6], 6);
}

void SSD1306Gfx::clear()
{
    Wire.beginTransmission(OLED_I2C_ADDRESS);
    Wire.write(OLED_CONTROL_BYTE_CMD_STREAM);
    Wire.write(OLED_CMD_SET_COLUMN_RANGE);
    Wire.write(0x00);
    Wire.write(0x7F);
    Wire.write(OLED_CMD_SET_PAGE_RANGE);
    Wire.write(0);
    Wire.write(7);
    Wire.endTransmission();

    for (uint16_t i = 0; i < 128; i++)
    {
        Wire.beginTransmission(OLED_I2C_ADDRESS);
        Wire.write(OLED_CONTROL_BYTE_DATA_STREAM);
        for (uint8_t x = 0; x < 8; x++)
        {
            Wire.write(0b00000000);
        }

        Wire.endTransmission();
    }
}

bool SSD1306Gfx::hasFlag(uint8_t flags, uint8_t flag)
{
    return (flags & flag) != 0;
}

const uint8_t SSD1306Gfx::reverse_lookup[] = {
      0x0,
      0x8,
      0x4,
      0xc,
      0x2,
      0xa,
      0x6,
      0xe,
      0x1,
      0x9,
      0x5,
      0xd,
      0x3,
      0xb,
      0x7,
      0xf
  };
uint8_t SSD1306Gfx::reverseBits(uint8_t n)
{  
    // Reverse the top and bottom nibble then swap them.
    return (reverse_lookup[n & 0b1111] << 4) | reverse_lookup[n >> 4];
}