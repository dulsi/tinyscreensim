#include <SdFat.h>
#include <boost/filesystem.hpp>

std::string SdFat::savePath;

bool SdFat::begin(uint8_t csPin, uint8_t divisor)
{
#ifdef _WIN32
 std::string appName("sdfat");
#else
 std::string appName(".sdfat");
#endif
 char *home = getenv("HOME");
 if (home)
 {
  savePath = home;
  if (savePath[savePath.length() - 1] != '/')
  {
   savePath += "/";
  }
   savePath += appName;
 }
 else
 {
  savePath = appName;
 }
 if (!boost::filesystem::is_directory(savePath))
 {
  if (!boost::filesystem::create_directory(savePath))
  {
   fprintf(stderr, "Error creating directory %s\n", savePath);
   exit(2);
  }
 }
 return 1;
}

bool SdFat::exists(const char* path)
{
 std::string fullPath = savePath + "/" + path;
 return boost::filesystem::exists(fullPath);
}

bool SdFile::close()
{
 fclose(f);
 return true;
}

bool SdFile::open(const char* path, uint8_t oflag /*= O_READ*/)
{
 std::string fullPath = SdFat::savePath + "/" + path;
 if (oflag == O_READ)
 {
  f = fopen(fullPath.c_str(), "rb");
  return (f != NULL);
 }
 else if (oflag == (O_CREAT | O_WRITE | O_TRUNC))
 {
  f = fopen(fullPath.c_str(), "wb");
  return (f != NULL);
 }
 return 0; //failure
}

int SdFile::read(void* buf, size_t nbyte)
{
 return fread(buf, 1, nbyte, f);
}

bool SdFile::sync()
{
 return true;
}

size_t SdFile::write(const uint8_t *buf, size_t size)
{
 return fwrite(buf, 1, size, f);
}
