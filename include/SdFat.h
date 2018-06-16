#ifndef __SDFAT_H__
#define __SDFAT_H__

#include <stdio.h>
#include <inttypes.h>
#include <string>

uint8_t const SPI_FULL_SPEED = 2;
uint8_t const O_READ = 0X01;
uint8_t const O_WRITE = 0X02;
uint8_t const O_TRUNC = 0X10;
uint8_t const O_CREAT = 0X40;

class SdFat
{
  friend class SdFile;

 public:
  bool begin(uint8_t csPin, uint8_t divisor);

  bool exists(const char* path);

 protected:
  static std::string savePath;
};

class SdFile
{
 public:
  SdFile() : f(NULL) { }

  bool close();

  bool open(const char* path, uint8_t oflag = O_READ);
  
  int read(void* buf, size_t nbyte);

  bool sync();

  size_t write(const uint8_t *buf, size_t size);

 protected:
  FILE *f;
};

#endif
