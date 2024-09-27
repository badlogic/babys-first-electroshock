#include "mcugdx.h"

#define TAG "Button example"

#define DEBOUNCE_TIME 25

// Big & Little blue fucker
#if 1
mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ST7789,
		.native_width = 200,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1,
		.reset = 5};
#else
// ILI9341 2,8" 240x320
mcugdx_display_config_t display_config = {
		.driver = MCUGDX_ILI9341,
		.native_width = 240,
		.native_height = 320,
		.mosi = 3,
		.sck = 4,
		.dc = 2,
		.cs = 1,
		.reset = 11
};
#endif

extern "C" void app_main(void) {
    mcugdx_display_init(&display_config);
    mcugdx_display_set_orientation(MCUGDX_LANDSCAPE);

    mcugdx_button_create(2, DEBOUNCE_TIME, MCUGDX_KEY_K);
    mcugdx_button_create(3, DEBOUNCE_TIME, MCUGDX_KEY_L);
    mcugdx_button_create(4, DEBOUNCE_TIME, MCUGDX_KEY_ESCAPE);
    mcugdx_button_create(5, DEBOUNCE_TIME, MCUGDX_KEY_ENTER);
    mcugdx_button_create(21, DEBOUNCE_TIME, MCUGDX_KEY_D);
    mcugdx_button_create(16, DEBOUNCE_TIME, MCUGDX_KEY_S);
    mcugdx_button_create(15, DEBOUNCE_TIME, MCUGDX_KEY_A);
    mcugdx_button_create(14, DEBOUNCE_TIME, MCUGDX_KEY_W);

    while(true) {
        mcugdx_button_event_t event;
        if (mcugdx_button_get_event(&event)) {
            mcugdx_log(TAG, "Button %s %s", mcugdx_keycode_to_string(event.keycode), event.type == MCUGDX_BUTTON_PRESSED ? "pressed" : "released");
        }

        mcugdx_display_show();
        mcugdx_sleep(0);
    }
}