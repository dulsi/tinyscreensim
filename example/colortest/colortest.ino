#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "TinyConfig.h"

#ifdef TINYARCADE_CONFIG
#include "TinyArcade.h"
#endif
#ifdef TINYSCREEN_GAMEKIT_CONFIG
#include "TinyGameKit.h"
#endif

TinyScreen display = TinyScreen(TinyScreenPlus);

void setup()
{
  arcadeInit();
  display.begin();
  display.setBitDepth(TSBitDepth16);
  display.setBrightness(15);
  display.setFlip(false);

#ifdef TINYARCADE_CONFIG
  USBDevice.init();
  USBDevice.attach();
#endif
  SerialUSB.begin(9600);
}

void loop()
{
  uint8_t btn = checkButton(TAButton1 | TAButton2);
  uint8_t joyDir = checkJoystick(TAJoystickUp | TAJoystickDown | TAJoystickLeft | TAJoystickRight);
  uint8_t lineBuffer[96 * 2];

  display.goTo(0,0);
  display.startData();
  for(int lines = 0; lines < 64; ++lines)
  {
    if (btn & TAButton1)
      memset(lineBuffer, 255, 96 * 2);
    else if (btn & TAButton2)
      memset(lineBuffer, 128, 96 * 2);
    else
      memset(lineBuffer, 0, 96 * 2);
    display.writeBuffer(lineBuffer,96 * 2);
  }
  display.endTransfer();
  delay(70);
}

