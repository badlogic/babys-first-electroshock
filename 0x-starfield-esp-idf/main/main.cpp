#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include "esp_task_wdt.h"

// Select driver
#define TFT_ST7735
// #define TFT_ST7789
// #define TFT_ILI9341

#define TFT_CS 1
#define TFT_DC 2
#define TFT_MOSI 3
#define TFT_SCL 4
#define TFT_MISO -1
#define TFT_RST -1

#ifdef TFT_ST7735
#define TFT_WIDTH 128
#define TFT_HEIGHT 128
#endif

#ifdef TFT_ST7789
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#endif

#ifdef TFT_ILI9341
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
#endif


int fbWidth = TFT_WIDTH;
int fbHeight = TFT_HEIGHT;
uint16_t *frameBuffer;
Adafruit_SPITFT *tft = nullptr;

SPIClass spi = SPIClass();

double GetTime() { return (double) esp_timer_get_time() / 1000000; }

void fb_clear(uint16_t color) {
	uint32_t d = color << 16 | color;
	uint32_t *out = (uint32_t *) frameBuffer;
	for (int i = 0, n = fbWidth * fbHeight / 2; i < n; i++) {
		out[i] = d;
	}
}

void fb_clear_black() {
	memset(frameBuffer, 0, fbWidth * fbHeight * sizeof(uint16_t));
}

void fb_pset(int x, int y, uint16_t color) {
	if (x < 0 || x >= fbWidth || y < 0 || y >= fbHeight) return;
	frameBuffer[x + y * fbWidth] = color;
}

void fb_show() {
	tft->startWrite();
	tft->setAddrWindow(0, 0, fbWidth, fbHeight);
	tft->writePixels(frameBuffer, fbWidth * fbHeight);
	tft->endWrite();
}

struct star_t {
	int x, y, z;
};

const int NUM_STARS = 200;
star_t stars[NUM_STARS];

void init_stars() {
	for (int i = 0; i < NUM_STARS; i++) {
		stars[i].x = random(-fbWidth / 2, fbWidth / 2);
		stars[i].y = random(-fbHeight / 2, fbHeight / 2);
		stars[i].z = random(1, fbWidth);
	}
}

void update_star(star_t &star) {
	star.z -= 1;
	if (star.z <= 0) {
		star.x = random(-fbWidth / 2, fbWidth / 2);
		star.y = random(-fbHeight / 2, fbHeight / 2);
		star.z = fbWidth;
	}
}

void draw_star(const star_t &star) {
	int sx = (star.x * fbWidth / 2) / star.z + fbWidth / 2;
	int sy = (star.y * fbWidth / 2) / star.z + fbHeight / 2;

	uint8_t brightness = map(star.z, 1, fbWidth, 255, 20);
	uint16_t color = 0xffff; // ((brightness >> 3) << 11) | ((brightness >> 2) << 5) | (brightness >> 3);

	fb_pset(sx, sy, color);
}


extern "C" void app_main() {
	initArduino();
	spi.begin(TFT_SCL, TFT_MISO, TFT_MOSI, -1);

#ifdef TFT_ST7735
	Adafruit_ST7735 *st7735 = new Adafruit_ST7735(&spi, TFT_CS, TFT_DC, TFT_RST);
	tft = st7735;
	st7735->setSPISpeed(42000000);
	st7735->initR(INITR_144GREENTAB);
#endif

#ifdef TFT_ST7789
	Adafruit_ST7789 *st7789 = new Adafruit_ST7789(&spi, TFT_CS, TFT_DC, TFT_RST);
	tft = st7789;
	st7789->init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE3);
	st7789->setSPISpeed(80000000);
#endif

#ifdef TFT_ILI9341
	// read diagnostics (optional but can help debug problems)
	Adafruit_ILI9341 *ili9341 = new Adafruit_ILI9341(&spi, TFT_DC, TFT_CS, TFT_RST);
	ili9341->begin(42000000);
	tft = ili9341;
#endif
	frameBuffer = (uint16_t *) heap_caps_malloc(fbWidth * fbHeight * sizeof(uint16_t), MALLOC_CAP_INTERNAL);

	init_stars();


	int frame = 0;

	while (true) {
		auto start = GetTime();

		fb_clear(0x0000);
		// fb_clear_black();

		auto clear = GetTime() - start;

		for (int i = 0; i < NUM_STARS; i++) {
			update_star(stars[i]);
			draw_star(stars[i]);
		}

		fb_show();

		if (frame % 30 == 0) {
			printf("clear: %f ms\nframe: %f ms\n", clear * 1000, (GetTime() - start) * 1000);
		}
		frame++;
	}
}
