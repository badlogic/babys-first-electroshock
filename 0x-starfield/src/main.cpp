#define USE_SPI_DMA
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Select driver
// #define TFT_ST7735
#define TFT_ST7789
// #define TFT_ILI9341

#define TFT_CS 10
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_SCL 13
#define TFT_DC 8
#define TFT_RST -1

#ifdef TFT_ST7735
#define TFT_WIDTH 128
#define TFT_HEIGHT 128
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#endif

#ifdef TFT_ST7789
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#endif

#ifdef TFT_ILI9341
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Doesn't got fullspeed with MISO??
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCL, TFT_RST, TFT_MISO);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
#endif


const int fbWidth = TFT_WIDTH;
const int fbHeight = TFT_HEIGHT;
uint16_t *frameBuffer;

void fb_clear(uint16_t color) {
	for (int i = 0, n = fbWidth * fbHeight; i < n; i++) {
		frameBuffer[i] = color;
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
	tft.startWrite();
	tft.setAddrWindow(0, 0, fbWidth, fbHeight);
	tft.writePixels(frameBuffer, fbWidth * fbHeight);
	tft.endWrite();
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
	uint16_t color = ((brightness >> 3) << 11) | ((brightness >> 2) << 5) | (brightness >> 3);

	fb_pset(sx, sy, color);
}

void setup() {
	Serial.begin(115200);

#ifdef TFT_ST7735
	tft.setSPISpeed(42000000);
	tft.initR(INITR_144GREENTAB);
#endif

#ifdef TFT_ST7789
	tft.init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE3);
	tft.setSPISpeed(80000000);
#endif

#ifdef TFT_ILI9341
	// read diagnostics (optional but can help debug problems)
	tft.begin(42000000);
#endif
	frameBuffer = (uint16_t *) malloc(fbWidth * fbHeight * sizeof(uint16_t));

	init_stars();
}

int frame = 0;

void loop() {
	auto start = millis();

	fb_clear(0x0000);

	auto clear = millis() - start;

	for (int i = 0; i < NUM_STARS; i++) {
		update_star(stars[i]);
		draw_star(stars[i]);
	}

	fb_show();

	if (frame % 30 == 0) {
		Serial.printf("clear: %i ms\nframe: %i\n", clear, millis() - start);
		Serial.println(" ms");
	}
	frame++;
}
