#define byte unsigned char

class TwoWire {
public:
    void begin() {}
    void requestFrom(byte address, int quantity);
    byte read() { return data[where++]; }

private:
    int where;
    byte data[6];
};

extern TwoWire Wire;
