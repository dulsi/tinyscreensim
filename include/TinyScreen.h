#ifndef __TINYSCREEN_H__
#define __TINYSCREEN_H__

#ifndef PROGMEM
#define PROGMEM
#endif

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
unsigned long millis();

// TinyScreen types
const uint8_t TinyScreenDefault   = 1;
const uint8_t TinyScreenAlternate = 2;
const uint8_t TinyScreenPlus      = 3;
const uint8_t TinyScreenGamepad   = 4;

// TinyScreen bitDepths
const uint8_t TSBitDepth8  = 0;
const uint8_t TSBitDepth16 = 1;

typedef struct
{
	const uint8_t width;
	const uint16_t offset;

} FONT_CHAR_INFO;

struct FONT_INFO
{
    FONT_INFO() : height(0), startCh(0), endCh(0), charDesc(0), bitmap(0) {}

	const unsigned char height;
	const char startCh;
	const char endCh;
	const FONT_CHAR_INFO*	charDesc;
	const unsigned char* bitmap;

};

class TinyScreen {
public:
    TinyScreen (int) {}
    void startData(void);
    void startCommand(void);
    void endTransfer(void);
    void begin(void);
    void begin(uint8_t);
    void on(void);
    void off(void);
    void setFlip(uint8_t);
    void setMirror(uint8_t);
    void setBitDepth(uint8_t);
    void setBrightness(uint8_t);
    void setWindowTitle(const char *title);
    void writeRemap(void);
    //accelerated drawing commands
    void drawPixel(uint8_t, uint8_t, uint16_t);
    void drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void clearWindow(uint8_t, uint8_t, uint8_t, uint8_t);
    //basic graphics commands
    void writePixel(uint16_t);
    void writeBuffer(uint8_t *, int);
    void setX(uint8_t, uint8_t);
    void setY(uint8_t, uint8_t);
    void goTo(uint8_t x, uint8_t y);
    //I2C GPIO related
    uint8_t getButtons(void);
    void writeGPIO(uint8_t, uint8_t);
    //font
    void setFont(const FONT_INFO&);
    void setCursor(uint8_t, uint8_t);
    void fontColor(uint8_t, uint8_t);
    virtual size_t write(uint8_t);
    void print(const char *s) {}
    void print(int n) {}

    static const uint8_t xMax=95;
    static const uint8_t yMax=63;
    private:

    uint8_t _addr, _cursorX, _cursorY, _fontHeight, _fontFirstCh, _fontLastCh, _fontColor, _fontBGcolor, _bitDepth, _flipDisplay, _mirrorDisplay;
    const FONT_CHAR_INFO* _fontDescriptor;
    const unsigned char* _fontBitmap;
};

const FONT_INFO liberationSans_8ptFontInfo;

#endif // __TINYSCREEN_H__
