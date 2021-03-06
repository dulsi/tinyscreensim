/*
 * This file was quickly hacked together to provide simple graphics output and
 * button controls for simulating a TinyScreen. It's really bad quality but does
 * its job: Providing an environment with visual output for compiling code that
 * runs on the TinyScreen+ in a similar way.
 */

#ifdef SDL2LIB
#include <SDL.h>
#else
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef PNGSAVE
#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_dynamic_io.hpp>
#endif

#define TINYSCREEN_WIDTH 96
#define TINYSCREEN_HEIGHT 64


#define SCREEN_TEXTURE_SIZE 128
#define SCREEN_UMAX (96.0f / (float)SCREEN_TEXTURE_SIZE)
#define SCREEN_VMAX (64.0f / (float)SCREEN_TEXTURE_SIZE)

#define SCREEN_Y 64
#define SCREEN_X 96

#define KEYBIT_UP 0x01
#define KEYBIT_DOWN 0x02
#define KEYBIT_LEFT 0x04
#define KEYBIT_RIGHT 0x08
#define KEYBIT_BUTTON1 0x10
#define KEYBIT_BUTTON2 0x20

#define TinyArcadePinX 42
#define TinyArcadePinY 1
#define TinyArcadePin1 45
#define TinyArcadePin2 44

SerialX Serial;
TwoWire Wire;
#ifdef SDL2LIB
unsigned int controls = 0;
#endif

void delay(int msec)
{
    struct timespec val;
    struct timespec rem;
    val.tv_sec = msec / 1000;
    val.tv_nsec = msec % 1000 * 1000000L;
    rem.tv_sec = 0;
    rem.tv_nsec = 0;
    nanosleep(&val, &rem);
}

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

typedef boost::minstd_rand base_generator_type;
#include <ctime>

static base_generator_type generator(42u);

void randomSeed(int seed)
{
 generator.seed(static_cast<unsigned int>(std::time(0)));
}

int random(int min, int max)
{
 boost::uniform_int<> ran_dist(min, max - 1);
 boost::variate_generator<base_generator_type&, boost::uniform_int<> > ran(generator, ran_dist);
 return ran();
/*    int x = random();
    x = x % (max - min);
    return x + min;*/
}

typedef struct {
    unsigned char screenData[SCREEN_TEXTURE_SIZE*SCREEN_TEXTURE_SIZE * 3];
    unsigned char rawFrameBuffer[TINYSCREEN_WIDTH * TINYSCREEN_HEIGHT * 2];
    unsigned char x,y;
    bool is16bit;
    bool isRecordingTSV;
#ifdef SDL2LIB
    SDL_Window *window;
    SDL_Renderer *mainRenderer;
    SDL_Texture *mainTexture;
    SDL_Surface *mainScreen;
    SDL_Rect *rect;
    int mult;
#else
    GLuint screenTexture;
    GLFWwindow* window;
#endif
    FILE *tsvFP;
} Emulator;

Emulator emulator;

static void writeFrameBufferToTSV() {
    if (emulator.isRecordingTSV && emulator.tsvFP) {
        printf("Writing framebuffer to tsv\n");
        fwrite(emulator.rawFrameBuffer,1,sizeof(emulator.rawFrameBuffer),emulator.tsvFP);
    }
}

static void updateScreen() {
#ifdef SDL2LIB
    SDL_UpdateTexture(emulator.mainTexture, emulator.rect, emulator.mainScreen->pixels, emulator.mainScreen->pitch);
    SDL_RenderClear(emulator.mainRenderer);
    SDL_RenderCopy(emulator.mainRenderer, emulator.mainTexture, NULL, NULL);
    SDL_RenderPresent(emulator.mainRenderer);
#else
    glBindTexture(GL_TEXTURE_2D, emulator.screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, SCREEN_TEXTURE_SIZE, SCREEN_TEXTURE_SIZE, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, emulator.screenData);
#endif
}

void TwoWire::requestFrom(byte address, int quantity)
{
#ifdef SDL2LIB
    where = 0;
    int RXY = 511;
    data[0] = RXY >> 2;
    data[1] = RXY >> 2;
    int LX = ((controls & KEYBIT_RIGHT) ? -511 : 0) + ((controls & KEYBIT_LEFT) ? 511 : 0)+511;
    data[2] = LX >> 2;
    int LY = ((controls & KEYBIT_UP) ? 511 : 0) + ((controls & KEYBIT_DOWN) ? -511 : 0)+511;
    data[3] = LY >> 2;
    data[4] = ((((((LX & 3) << 2) | (LY & 3)) << 2) | (RXY & 3)) << 2) | (RXY & 3);
    data[5] = ((controls & KEYBIT_BUTTON1) ? 0 : 4) | ((controls & KEYBIT_BUTTON2) ? 0 : 8);
#else
    GLFWwindow* window = emulator.window;
    where = 0;
    int RXY = 511;
    data[0] = RXY >> 2;
    data[1] = RXY >> 2;
    int LX = (glfwGetKey(window, GLFW_KEY_RIGHT) ? -511 : 0) + (glfwGetKey(window, GLFW_KEY_LEFT) ? 511 : 0)+511;
    data[2] = LX >> 2;
    int LY = (glfwGetKey(window, GLFW_KEY_UP) ? 511 : 0) + (glfwGetKey(window, GLFW_KEY_DOWN) ? -511 : 0)+511;
    data[3] = LY >> 2;
    data[4] = ((((((LX & 3) << 2) | (LY & 3)) << 2) | (RXY & 3)) << 2) | (RXY & 3);
    data[5] = (glfwGetKey(window, GLFW_KEY_G) ? 0 : 4) | (glfwGetKey(window, GLFW_KEY_H) ? 0 : 8);
#endif
}

int digitalRead(int pin) {
#ifndef SDL2LIB
    GLFWwindow* window = emulator.window;
    switch (pin) {
        case 4: case TinyArcadePin1: return (glfwGetKey(window, GLFW_KEY_G) ? 0 : 1);
        case 5: case TinyArcadePin2: return (glfwGetKey(window, GLFW_KEY_H) ? 0 : 1);
        default: return 0;
    }
#endif
    return 0;
}

int analogWrite(int pin, int val) {
 return 0;
}

int analogRead(int pin) {
 #ifndef SDL2LIB
    GLFWwindow* window = emulator.window;
    switch (pin) {
        case 2: case TinyArcadePinX: return (glfwGetKey(window, GLFW_KEY_RIGHT) ? -511 : 0) + (glfwGetKey(window, GLFW_KEY_LEFT) ? 511 : 0)+511;
        case 3: case TinyArcadePinY: return (glfwGetKey(window, GLFW_KEY_UP) ? -511 : 0) + (glfwGetKey(window, GLFW_KEY_DOWN) ? 511 : 0)+511;
        default: return 0;
    }
#endif
    return 0;
}

void setup();

void loop();

#ifndef SDL2LIB
static void drawCircle(float x, float y, float radius, int div) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i=0;i<=div;i+=1) {
        float ang = (float)i/(float)div * 3.141593f * 2.0f;
        float px = sinf(ang) * radius;
        float py = cosf(ang) * radius;
        glVertex2f(px+x,py+y);
    }
    glEnd();
}

static void drawG(float x, float y, float size) {
    glBegin(GL_QUADS);
    glVertex2f(x, y + size);
    glVertex2f(x + size, y + size);
    glVertex2f(x + size, y + size - size/8);
    glVertex2f(x, y + size - size/8);
    glVertex2f(x + size - size/8, y + size);
    glVertex2f(x + size, y + size);
    glVertex2f(x + size, y + size - size/4);
    glVertex2f(x + size - size/8, y + size - size/4);
    glVertex2f(x, y);
    glVertex2f(x, y + size);
    glVertex2f(x + size/8, y + size);
    glVertex2f(x + size/8, y);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size/8);
    glVertex2f(x, y + size/8);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size/2 + size/16);
    glVertex2f(x + size - size/8, y + size/2 + size/16);
    glVertex2f(x + size - size/8, y);
    glVertex2f(x + size/2, y + size/2 + size/16);
    glVertex2f(x + size/2, y + size/2 - size/16);
    glVertex2f(x + size, y + size/2 - size/16);
    glVertex2f(x + size, y + size/2 + size/16);
    glEnd();
}

static void drawH(float x, float y, float size) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + size);
    glVertex2f(x + size/8, y + size);
    glVertex2f(x + size/8, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size);
    glVertex2f(x + size - size/8, y + size);
    glVertex2f(x + size - size/8, y);
    glVertex2f(x + size/8, y + size/2 + size/16);
    glVertex2f(x + size - size/8, y + size/2 + size/16);
    glVertex2f(x + size - size/8, y + size/2 - size/16);
    glVertex2f(x + size/8, y + size/2 - size/16);
    glEnd();
}
#endif

void TinyScreen::startData(void) {
#ifdef SDL2LIB
    SDL_LockSurface(emulator.mainScreen);
#endif
}
void TinyScreen::startCommand(void) {}
void TinyScreen::endTransfer(void) {
#ifdef SDL2LIB
    SDL_UnlockSurface(emulator.mainScreen);
#endif
    updateScreen();
#ifndef SDL2LIB
    GLFWwindow* window = emulator.window;
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.25f,0.25f,0.5f,0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
   // glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
    /*glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.6f, 0.f);
    glEnd();*/
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, emulator.screenTexture);
    float scale = ratio > 1.3f ? 2.25f : (2.25f/1.3f) * ratio;

    glScalef(scale,scale,scale);
    glTranslatef(-0.48f, -0.12f,0);

    glColor3f(.0f,.0f,0.f);
    drawCircle(.15f,-.25f,.18f,16);
    glColor3f(.8f,.8f,0.8f);
    float stickX = -(float)analogRead(2) / 1023.f + .5f;
    float stickY = -(float)analogRead(3) / 1023.f + .5f;
    drawCircle(.15f + stickX*.15f,-.25f + stickY * .15f,.10f,12);



    glColor3f(.0f,.0f,0.f);
    drawCircle(.6f,-.325f,.075f,12);
    drawCircle(.8f,-.225f,.075f,12);
    glColor3f(.8f,.2f,0.f);
    float buttonY = !digitalRead(4) ? -.32f : -.3f;
    drawCircle(.6f,buttonY,.075f,12);
    glColor3f(.0f,.0f,0.f);
    drawG(.6f - .0375f, buttonY - .0375f, .075f);
    glColor3f(.8f,.2f,0.f);
    buttonY = !digitalRead(5) ? -.22f : -.2f;
    drawCircle(.8f,buttonY,.075f,12);
    glColor3f(.0f,.0f,0.f);
    drawH(.8f - .0375f, buttonY - .0375f, .075f);
    glColor3f(.8f,.2f,0.f);
    int buttons = getButtons();
    glBegin(GL_QUADS);
    // bottom left
    float buttonX = buttons & 1 ? -.05f : -.075f;
    glVertex2f(buttonX, .2f);
    glVertex2f(buttonX, .1f);
    glVertex2f(.1f, .1f);
    glVertex2f(.1f, .2f);
    glEnd();

    buttonX = (buttons & 2) ? -.05f : -.075f;
    glColor3f(.8f,.2f,0.f);
    glBegin(GL_QUADS);
    glVertex2f(buttonX, .54f);
    glVertex2f(buttonX, .44f);
    glVertex2f(.1f, .44f);
    glVertex2f(.1f, .54f);
    glEnd();
    glColor3f(.8f,.2f,0.f);
    buttonX = (buttons & 4) ? 1.01f : 1.035f;
    glBegin(GL_QUADS);
    glVertex2f(.5f, .54f);
    glVertex2f(.5f, .44f);
    glVertex2f(buttonX, .44f);
    glVertex2f(buttonX, .54f);
    glEnd();
    buttonX = (buttons & 8) ? 1.01f : 1.035f;
    glBegin(GL_QUADS);
    glVertex2f(.5f, .2f);
    glVertex2f(.5f, .1f);
    glVertex2f(buttonX, .1f);
    glVertex2f(buttonX, .2f);
    glEnd();


    float margin = .02f;
    glColor3f(0,0,0);
    glBegin(GL_QUADS);
    glVertex3f(-margin, -margin, 0);
    glVertex3f(-margin, 0.64f+margin, 0);
    glVertex3f(0.96f+margin, 0.64f+margin, 0);
    glVertex3f(0.96f+margin, -margin, 0);
    glEnd();



    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, SCREEN_VMAX); glVertex3f(0, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(0, 0.64f, 0);
    glTexCoord2f(SCREEN_UMAX, 0); glVertex3f(0.96f, 0.64f, 0);
    glTexCoord2f(SCREEN_UMAX, SCREEN_VMAX); glVertex3f(0.96f, 0, 0);
    glEnd();

    if (emulator.isRecordingTSV) {
        writeFrameBufferToTSV();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    if (glfwWindowShouldClose(window)) {
        if (emulator.tsvFP) {
            fclose(emulator.tsvFP);
            emulator.tsvFP = 0;
        }
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_SUCCESS);
    }
#endif
}
void TinyScreen::begin(void) {}
void TinyScreen::begin(uint8_t) {}
void TinyScreen::on(void) {}
void TinyScreen::off(void) {}
void TinyScreen::setFlip(uint8_t) {}
void TinyScreen::setMirror(uint8_t) {}
void TinyScreen::setBitDepth(uint8_t is16bit) {
    emulator.is16bit = (is16bit & TSBitDepth16) ? true : false;
}
void TinyScreen::setBrightness(uint8_t) {}
void TinyScreen::setWindowTitle(const char *title)
{
#ifndef SDL2LIB
    glfwSetWindowTitle(emulator.window, title);
#endif
}
//void TinyScreen::writeRemap(void) {}
//accelerated drawing commands
void TinyScreen::drawPixel(uint8_t, uint8_t, uint16_t) {}
void TinyScreen::drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
void TinyScreen::drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
void TinyScreen::drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
void TinyScreen::drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
void TinyScreen::clearWindow(uint8_t, uint8_t, uint8_t, uint8_t) {}
//basic graphics commands
void TinyScreen::writePixel(uint16_t) {
}
void TinyScreen::writeBuffer(uint8_t *rgb, int num) {
    if (num > TINYSCREEN_WIDTH * (emulator.is16bit ? 2 : 1)) {
        printf("line too long: %d\n",num);
    }
    //uint16_t *rgb565_16 = (uint16_t*)&rgb565[0];
    int idx = emulator.x + emulator.y * SCREEN_TEXTURE_SIZE;
    int bufferIdx = (emulator.x + emulator.y * TINYSCREEN_WIDTH) * 2;
#ifdef SDL2LIB
    Uint8 *pixels = (Uint8*)emulator.mainScreen->pixels;
#endif
    uint8_t *rgb565 = rgb;
    for (int i=0;i<num; i+=1) {
        uint8_t r,g,b;
        if (emulator.is16bit) {
            uint16_t word = 0;
            word = rgb565[i]<<8 | rgb565[i+1];
            r = word & 31;
            g = (word >> 5) & 63;
            b = word >> 11;

            r = (r << 3 | r >> 2);
            g = (g << 2 | g >> 4);
            b = (b << 3 | b >> 2);
            emulator.rawFrameBuffer[bufferIdx++ % sizeof(emulator.rawFrameBuffer)] = rgb565[i];
            emulator.rawFrameBuffer[bufferIdx++ % sizeof(emulator.rawFrameBuffer)] = rgb565[i+1];
            i+=1;
        } else {
            uint8_t rgb233 = rgb[i];
            r = rgb233 & 3;
            r = (r << 6) | (r << 4) | (r << 2) | r;
            g = (rgb233 >> 2) & 7;
            g = g << 5 | g << 2 | g >> 1;
            b = rgb233 >> 5 & 7;
            b = b << 5 | b << 2 | b >> 1;
            uint16_t word = 0;
            word |= (((uint16_t)r) & 0x00F8) >> 3;
            word |= (((uint16_t)g) & 0x00FC) << 2;
            word |= (((uint16_t)b) & 0x00F8) << 8;
            emulator.rawFrameBuffer[bufferIdx++ % sizeof(emulator.rawFrameBuffer)] = word >> 8;
            emulator.rawFrameBuffer[bufferIdx++ % sizeof(emulator.rawFrameBuffer)] = word & 0x00FF;
        }
        // my gif screencsat program doesn't like 00ff00
        if (r == 0 && g >= 250 && b == 0) g = 250;
        emulator.screenData[idx*3+0] = r;
        emulator.screenData[idx*3+1] = g;
        emulator.screenData[idx*3+2] = b;
#ifdef SDL2LIB
        int startIdx = ((emulator.x + ((i / (emulator.is16bit ? 2 : 1)) % TINYSCREEN_WIDTH) + emulator.y * TINYSCREEN_WIDTH * emulator.mult) * emulator.mult) * 4;
        for (int yMod = 0; yMod < emulator.mult; yMod++)
        {
         for (int xMod = 0; xMod < emulator.mult; xMod++)
         {
          pixels[startIdx + xMod * 4] = b;
          pixels[startIdx + xMod * 4 + 1] = g;
          pixels[startIdx + xMod * 4 + 2] = r;
          pixels[startIdx + xMod * 4 + 3] = 255;
         }
         startIdx += TINYSCREEN_WIDTH * emulator.mult * 4;
        }
#endif
        if (idx % SCREEN_TEXTURE_SIZE == TINYSCREEN_WIDTH-1) {
            idx = emulator.x + (++emulator.y) * SCREEN_TEXTURE_SIZE;//SCREEN_TEXTURE_SIZE - TINYSCREEN_WIDTH + 1;
        } else {
            idx += 1;
        }
    }
    //emulator.y+=1;
}
/*void TinyScreen::setX(uint8_t, uint8_t);
void TinyScreen::setY(uint8_t, uint8_t);*/
void TinyScreen::goTo(uint8_t x, uint8_t y) {
    emulator.x = x;
    emulator.y = y;
}

//I2C GPIO related
uint8_t TinyScreen::getButtons(void) {
#ifdef SDL2LIB
    return 0;
#else
    GLFWwindow* window = emulator.window;
    int tr = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    int br = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    int tl = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    int bl = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    return tl << 1 | bl | tr << 2 | br << 3;
#endif
}
/*void TinyScreen::writeGPIO(uint8_t, uint8_t);*/
//font
void TinyScreen::setFont(const FONT_INFO&) {}
void TinyScreen::setCursor(uint8_t, uint8_t) {}
void TinyScreen::fontColor(uint8_t, uint8_t) {}
size_t TinyScreen::write(uint8_t) { return 0; }


static void init() {

    for (int x=0;x<TINYSCREEN_WIDTH;x+=1) {
        for (int y=0;y<TINYSCREEN_HEIGHT;y+=1) {
            int idx = (x + y * SCREEN_TEXTURE_SIZE) * 3;
            emulator.screenData[idx] = x * 255 / TINYSCREEN_WIDTH;
            emulator.screenData[idx + 1] = y * 255 / TINYSCREEN_HEIGHT;
        }
    }

#ifndef SDL2LIB
    glGenTextures(1, &emulator.screenTexture);
#endif
    updateScreen();
#ifndef SDL2LIB
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
}

unsigned long millis() {
    struct timeval ret;
    gettimeofday(&ret, NULL);
    return ret.tv_sec * 1000 + ret.tv_usec / 1000;
}

static void error_callback(int /*error*/, const char* description)
{
    fputs(description, stderr);
}

#ifndef SDL2LIB
static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
        emulator.isRecordingTSV = !emulator.isRecordingTSV;
        if (emulator.isRecordingTSV) {
            char filename[128];
            sprintf(filename,"rec-%s.tsv","test");
            printf("Starting recording to %s\n", filename);
            emulator.tsvFP = fopen(filename, "wb");
        } else {
            printf("Recording finished\n");
            if (emulator.tsvFP) {
                fclose(emulator.tsvFP);
                emulator.tsvFP = 0;
            }
        }
    }
#ifdef PNGSAVE
    if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
        boost::gil::rgb8_image_t img(TINYSCREEN_WIDTH, TINYSCREEN_HEIGHT);
        for (int y = 0; y < TINYSCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < TINYSCREEN_WIDTH; x++)
            {
                int indx = (x + y * SCREEN_TEXTURE_SIZE) * 3;
                boost::gil::rgb8_pixel_t p(emulator.screenData[indx], emulator.screenData[indx + 1], emulator.screenData[indx + 2]);
                *(view(img).at(x, y)) = p;
            }
        }
        boost::gil::png_write_view("screenshot.png", const_view(img));
    }
#endif
}
#endif

int main(int argc, char *argv[])
{
    bool full = false;
    bool softRender = false;
    int opt;
    static struct option long_options[] =
    {
        {"multiplier", 1, 0, 'u'},
        {0, 0, 0, 0}
    };
#ifdef SDL2LIB
    int screenX, screenY;
    emulator.mult = 4;
    emulator.rect = NULL;
    screenX = SCREEN_X * emulator.mult;
    screenY = SCREEN_Y * emulator.mult;
#endif
    while ((opt = getopt_long(argc,argv,"pu:fw", long_options, NULL)) != -1)
    {
        switch (opt)
        {
#ifdef SDL2LIB
            case 'p':
                emulator.rect = new SDL_Rect;
                emulator.rect->x = 48;
                emulator.rect->y = 8;
                emulator.rect->w = 384;
                emulator.rect->h = 256;
                emulator.mult = 4;
                softRender = true;
                screenX = 480;
                screenY = 272;
                full = true;
                break;
            case 'u':
                if (optarg)
                {
                    emulator.mult = atol(optarg);
                    screenX = SCREEN_X * emulator.mult;
                    screenY = SCREEN_Y * emulator.mult;
                }
                break;
#endif
            case 'f':
                full = true;
                break;
            case 'w':
                softRender = true;
                break;
            default:
                break;
        }
    }
#ifdef SDL2LIB
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
        printf("Failed - SDL_Init\n");
        exit(0);
    }
    SDL_Window* window;
    window = SDL_CreateWindow("TinyScreen Simulator",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              screenX,
                              screenY,
                              (full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    emulator.window = window;
    if (window == NULL)
    {
        printf("Failed - SDL_CreateWindow\n");
        exit(0);
    }

    emulator.mainRenderer = SDL_CreateRenderer(emulator.window, -1, (softRender ? SDL_RENDERER_SOFTWARE : 0));
    if (emulator.mainRenderer == NULL)
    {
        printf("Failed - SDL_CreateRenderer\n");
        exit(0);
    }
    emulator.mainTexture = SDL_CreateTexture(emulator.mainRenderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                screenX,
                                screenY);
    if (emulator.mainTexture == NULL)
    {
        printf("Failed - SDL_CreateTexture\n");
        exit(0);
    }
    emulator.mainScreen = SDL_CreateRGBSurface(0, SCREEN_X * emulator.mult, SCREEN_Y * emulator.mult, 32,
                                           0x00FF0000,
                                           0x0000FF00,
                                           0x000000FF,
                                           0xFF000000);
    if (emulator.mainScreen == NULL)
    {
        printf("Failed - SDL_CreateRGBSurface\n");
        exit(0);
    }
#else
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(420, 420, "TinyScreen Simulator", NULL, NULL);
    emulator.window = window;
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
#endif
    init();
    setup();
    int frame = 0;
#ifdef SDL2LIB
    while (true)
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        loop();
        frame++;
#ifdef SDL2LIB
        SDL_Event sdlevent;
        while (SDL_PollEvent(&sdlevent))
        {
            if (sdlevent.type == SDL_QUIT)
            {
                SDL_Quit();
                exit(EXIT_SUCCESS);
            }
            else if (sdlevent.type == SDL_KEYDOWN)
            {
                if ((sdlevent.key.keysym.sym == SDLK_UP) || (sdlevent.key.keysym.sym == SDLK_KP_8))
                {
                    controls |= KEYBIT_UP;
                }
                else if ((sdlevent.key.keysym.sym == SDLK_LEFT) || (sdlevent.key.keysym.sym == SDLK_KP_4))
                {
                    controls |= KEYBIT_LEFT;
                }
                else if ((sdlevent.key.keysym.sym == SDLK_DOWN) || (sdlevent.key.keysym.sym == SDLK_KP_2))
                {
                    controls |= KEYBIT_DOWN;
                }
                else if ((sdlevent.key.keysym.sym == SDLK_RIGHT) || (sdlevent.key.keysym.sym == SDLK_KP_6))
                {
                    controls |= KEYBIT_RIGHT;
                }
                else if (sdlevent.key.keysym.sym == SDLK_g)
                {
                    controls |= KEYBIT_BUTTON1;
                }
                else if (sdlevent.key.keysym.sym == SDLK_h)
                {
                    controls |= KEYBIT_BUTTON2;
                }
                else if (sdlevent.key.keysym.sym == SDLK_ESCAPE)
                {
                    SDL_Quit();
                    exit(EXIT_SUCCESS);
                }
            }
            else if (sdlevent.type == SDL_KEYUP)
            {
                if ((sdlevent.key.keysym.sym == SDLK_UP) || (sdlevent.key.keysym.sym == SDLK_KP_8))
                {
                    controls &= ~KEYBIT_UP;
                }
                else if ((sdlevent.key.keysym.sym == SDLK_LEFT) || (sdlevent.key.keysym.sym == SDLK_KP_4))
                {
                    controls &= ~KEYBIT_LEFT;
                }
                else if ((sdlevent.key.keysym.sym == SDLK_DOWN) || (sdlevent.key.keysym.sym == SDLK_KP_2))
                {
                    controls &= ~KEYBIT_DOWN;
                }
                else if ((sdlevent.key.keysym.sym == SDLK_RIGHT) || (sdlevent.key.keysym.sym == SDLK_KP_6))
                {
                    controls &= ~KEYBIT_RIGHT;
                }
                else if (sdlevent.key.keysym.sym == SDLK_g)
                {
                    controls &= ~KEYBIT_BUTTON1;
                }
                else if (sdlevent.key.keysym.sym == SDLK_h)
                {
                    controls &= ~KEYBIT_BUTTON2;
                }
            }
        }
#endif
/*        if ((emulator.isRecordingTSV) && ((emulator.is16bit) || (frame % 2))) {
            writeFrameBufferToTSV();
        }*/
    }
    if (emulator.tsvFP) {
        fclose(emulator.tsvFP);
        emulator.tsvFP = 0;
    }
#ifdef SDL2LIB
    SDL_Quit();
#else
    glfwDestroyWindow(window);
    glfwTerminate();
#endif
    exit(EXIT_SUCCESS);
}
