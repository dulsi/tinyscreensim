#include <stdio.h>
#include <time.h>

class SerialX {
public:
    void print(const char* msg) {
        printf("%s",msg);
    }
    void println(const char *msg) {
        printf("%s\n",msg);
    }
    void println(double num) {
        printf("%f\n",num);
    }
    void begin(int spd) {}
};

extern SerialX Serial;

void delay(int msec);
void randomSeed(int seed);
int random(int min, int max);
int analogRead(int pin);
