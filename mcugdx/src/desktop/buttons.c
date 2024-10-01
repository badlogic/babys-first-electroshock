#include "buttons.h"
#include <SDL.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BUTTONS 32
#define MAX_EVENTS 32

typedef struct {
    int pin;
    uint32_t debounce_time_ms;
    SDL_Keycode keycode;
    bool is_pressed;
    uint32_t last_event_time;
} mcugdx_button_t;

static mcugdx_button_t buttons[MAX_BUTTONS] = {0};
static mcugdx_button_event_t event_queue[MAX_EVENTS] = {0};
static int event_queue_head = 0;
static int event_queue_tail = 0;
static bool is_initialized = false;

static SDL_Keycode map_keycode(mcugdx_keycode_t keycode);

static void enqueue_event(mcugdx_button_event_t *event) {
    if ((event_queue_tail + 1) % MAX_EVENTS == event_queue_head) {
        // Queue is full, drop the oldest event
        event_queue_head = (event_queue_head + 1) % MAX_EVENTS;
    }
    memcpy(&event_queue[event_queue_tail], event, sizeof(mcugdx_button_event_t));
    event_queue_tail = (event_queue_tail + 1) % MAX_EVENTS;
}

mcugdx_button_handle_t mcugdx_button_create(int pin, uint32_t debounce_time_ms, mcugdx_keycode_t keycode) {
    if (!is_initialized) {
        for (int i = 0; i < MAX_BUTTONS; i++) {
            buttons[i].pin = -1;
        }
        is_initialized = true;
    }

    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (buttons[i].pin == -1) {
            buttons[i].pin = pin;
            buttons[i].debounce_time_ms = debounce_time_ms;
            buttons[i].keycode = keycode;
            buttons[i].is_pressed = false;
            buttons[i].last_event_time = 0;
            return i;
        }
    }
    return -1;  // No free slots
}

void mcugdx_button_destroy(mcugdx_button_handle_t handle) {
    if (handle >= 0 && handle < MAX_BUTTONS) {
        buttons[handle].pin = -1;
    }
}

bool mcugdx_button_is_pressed(mcugdx_button_handle_t handle) {
    if (handle >= 0 && handle < MAX_BUTTONS && buttons[handle].pin != -1) {
        return buttons[handle].is_pressed;
    }
    return false;
}

bool mcugdx_button_get_event(mcugdx_button_event_t *event) {
    if (event_queue_head != event_queue_tail) {
        memcpy(event, &event_queue[event_queue_head], sizeof(mcugdx_button_event_t));
        event_queue_head = (event_queue_head + 1) % MAX_EVENTS;
        return true;
    }
    return false;
}

void mcugdx_button_clear_events(void) {
    event_queue_head = event_queue_tail = 0;
}

void mcugdx_desktop_update_button(SDL_KeyboardEvent *event) {
    uint32_t current_time = SDL_GetTicks();
    SDL_Keycode key_code = event->keysym.sym;
    bool is_pressed = (event->type == SDL_KEYDOWN);

    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (buttons[i].pin != -1 && map_keycode(buttons[i].keycode) == key_code) {
            uint32_t time_diff = current_time - buttons[i].last_event_time;
            if (time_diff >= buttons[i].debounce_time_ms) {
                if (buttons[i].is_pressed == is_pressed) return;
                mcugdx_button_event_t button_event = {
                    .button = i,
                    .type = is_pressed ? MCUGDX_BUTTON_PRESSED : MCUGDX_BUTTON_RELEASED,
                    .timestamp = current_time,
                    .pin = buttons[i].pin,
                    .keycode = buttons[i].keycode
                };
                enqueue_event(&button_event);
                buttons[i].is_pressed = is_pressed;
                buttons[i].last_event_time = current_time;
            }
            break;
        }
    }
}

static SDL_Keycode map_keycode(mcugdx_keycode_t keycode) {
    switch(keycode) {
        case MCUGDX_KEY_UNKNOWN: return SDLK_UNKNOWN;
        case MCUGDX_KEY_SPACE: return SDLK_SPACE;
        case MCUGDX_KEY_APOSTROPHE: return SDLK_QUOTE;
        case MCUGDX_KEY_COMMA: return SDLK_COMMA;
        case MCUGDX_KEY_MINUS: return SDLK_MINUS;
        case MCUGDX_KEY_PERIOD: return SDLK_PERIOD;
        case MCUGDX_KEY_SLASH: return SDLK_SLASH;
        case MCUGDX_KEY_0: return SDLK_0;
        case MCUGDX_KEY_1: return SDLK_1;
        case MCUGDX_KEY_2: return SDLK_2;
        case MCUGDX_KEY_3: return SDLK_3;
        case MCUGDX_KEY_4: return SDLK_4;
        case MCUGDX_KEY_5: return SDLK_5;
        case MCUGDX_KEY_6: return SDLK_6;
        case MCUGDX_KEY_7: return SDLK_7;
        case MCUGDX_KEY_8: return SDLK_8;
        case MCUGDX_KEY_9: return SDLK_9;
        case MCUGDX_KEY_SEMICOLON: return SDLK_SEMICOLON;
        case MCUGDX_KEY_EQUAL: return SDLK_EQUALS;
        case MCUGDX_KEY_A: return SDLK_a;
        case MCUGDX_KEY_B: return SDLK_b;
        case MCUGDX_KEY_C: return SDLK_c;
        case MCUGDX_KEY_D: return SDLK_d;
        case MCUGDX_KEY_E: return SDLK_e;
        case MCUGDX_KEY_F: return SDLK_f;
        case MCUGDX_KEY_G: return SDLK_g;
        case MCUGDX_KEY_H: return SDLK_h;
        case MCUGDX_KEY_I: return SDLK_i;
        case MCUGDX_KEY_J: return SDLK_j;
        case MCUGDX_KEY_K: return SDLK_k;
        case MCUGDX_KEY_L: return SDLK_l;
        case MCUGDX_KEY_M: return SDLK_m;
        case MCUGDX_KEY_N: return SDLK_n;
        case MCUGDX_KEY_O: return SDLK_o;
        case MCUGDX_KEY_P: return SDLK_p;
        case MCUGDX_KEY_Q: return SDLK_q;
        case MCUGDX_KEY_R: return SDLK_r;
        case MCUGDX_KEY_S: return SDLK_s;
        case MCUGDX_KEY_T: return SDLK_t;
        case MCUGDX_KEY_U: return SDLK_u;
        case MCUGDX_KEY_V: return SDLK_v;
        case MCUGDX_KEY_W: return SDLK_w;
        case MCUGDX_KEY_X: return SDLK_x;
        case MCUGDX_KEY_Y: return SDLK_y;
        case MCUGDX_KEY_Z: return SDLK_z;
        case MCUGDX_KEY_LEFT_BRACKET: return SDLK_LEFTBRACKET;
        case MCUGDX_KEY_BACKSLASH: return SDLK_BACKSLASH;
        case MCUGDX_KEY_RIGHT_BRACKET: return SDLK_RIGHTBRACKET;
        case MCUGDX_KEY_GRAVE_ACCENT: return SDLK_BACKQUOTE;
        case MCUGDX_KEY_ESCAPE: return SDLK_ESCAPE;
        case MCUGDX_KEY_ENTER: return SDLK_RETURN;
        case MCUGDX_KEY_TAB: return SDLK_TAB;
        case MCUGDX_KEY_BACKSPACE: return SDLK_BACKSPACE;
        case MCUGDX_KEY_INSERT: return SDLK_INSERT;
        case MCUGDX_KEY_DELETE: return SDLK_DELETE;
        case MCUGDX_KEY_RIGHT: return SDLK_RIGHT;
        case MCUGDX_KEY_LEFT: return SDLK_LEFT;
        case MCUGDX_KEY_DOWN: return SDLK_DOWN;
        case MCUGDX_KEY_UP: return SDLK_UP;
        case MCUGDX_KEY_PAGE_UP: return SDLK_PAGEUP;
        case MCUGDX_KEY_PAGE_DOWN: return SDLK_PAGEDOWN;
        case MCUGDX_KEY_HOME: return SDLK_HOME;
        case MCUGDX_KEY_END: return SDLK_END;
        case MCUGDX_KEY_CAPS_LOCK: return SDLK_CAPSLOCK;
        case MCUGDX_KEY_SCROLL_LOCK: return SDLK_SCROLLLOCK;
        case MCUGDX_KEY_NUM_LOCK: return SDLK_NUMLOCKCLEAR;
        case MCUGDX_KEY_PRINT_SCREEN: return SDLK_PRINTSCREEN;
        case MCUGDX_KEY_PAUSE: return SDLK_PAUSE;
        case MCUGDX_KEY_F1: return SDLK_F1;
        case MCUGDX_KEY_F2: return SDLK_F2;
        case MCUGDX_KEY_F3: return SDLK_F3;
        case MCUGDX_KEY_F4: return SDLK_F4;
        case MCUGDX_KEY_F5: return SDLK_F5;
        case MCUGDX_KEY_F6: return SDLK_F6;
        case MCUGDX_KEY_F7: return SDLK_F7;
        case MCUGDX_KEY_F8: return SDLK_F8;
        case MCUGDX_KEY_F9: return SDLK_F9;
        case MCUGDX_KEY_F10: return SDLK_F10;
        case MCUGDX_KEY_F11: return SDLK_F11;
        case MCUGDX_KEY_F12: return SDLK_F12;
        case MCUGDX_KEY_F13: return SDLK_F13;
        case MCUGDX_KEY_F14: return SDLK_F14;
        case MCUGDX_KEY_F15: return SDLK_F15;
        case MCUGDX_KEY_F16: return SDLK_F16;
        case MCUGDX_KEY_F17: return SDLK_F17;
        case MCUGDX_KEY_F18: return SDLK_F18;
        case MCUGDX_KEY_F19: return SDLK_F19;
        case MCUGDX_KEY_F20: return SDLK_F20;
        case MCUGDX_KEY_F21: return SDLK_F21;
        case MCUGDX_KEY_F22: return SDLK_F22;
        case MCUGDX_KEY_F23: return SDLK_F23;
        case MCUGDX_KEY_F24: return SDLK_F24;
        case MCUGDX_KEY_KP_0: return SDLK_KP_0;
        case MCUGDX_KEY_KP_1: return SDLK_KP_1;
        case MCUGDX_KEY_KP_2: return SDLK_KP_2;
        case MCUGDX_KEY_KP_3: return SDLK_KP_3;
        case MCUGDX_KEY_KP_4: return SDLK_KP_4;
        case MCUGDX_KEY_KP_5: return SDLK_KP_5;
        case MCUGDX_KEY_KP_6: return SDLK_KP_6;
        case MCUGDX_KEY_KP_7: return SDLK_KP_7;
        case MCUGDX_KEY_KP_8: return SDLK_KP_8;
        case MCUGDX_KEY_KP_9: return SDLK_KP_9;
        case MCUGDX_KEY_KP_DECIMAL: return SDLK_KP_DECIMAL;
        case MCUGDX_KEY_KP_DIVIDE: return SDLK_KP_DIVIDE;
        case MCUGDX_KEY_KP_MULTIPLY: return SDLK_KP_MULTIPLY;
        case MCUGDX_KEY_KP_SUBTRACT: return SDLK_KP_MINUS;
        case MCUGDX_KEY_KP_ADD: return SDLK_KP_PLUS;
        case MCUGDX_KEY_KP_ENTER: return SDLK_KP_ENTER;
        case MCUGDX_KEY_KP_EQUAL: return SDLK_KP_EQUALS;
        case MCUGDX_KEY_LEFT_SHIFT: return SDLK_LSHIFT;
        case MCUGDX_KEY_LEFT_CONTROL: return SDLK_LCTRL;
        case MCUGDX_KEY_LEFT_ALT: return SDLK_LALT;
        case MCUGDX_KEY_LEFT_SUPER: return SDLK_LGUI;
        case MCUGDX_KEY_RIGHT_SHIFT: return SDLK_RSHIFT;
        case MCUGDX_KEY_RIGHT_CONTROL: return SDLK_RCTRL;
        case MCUGDX_KEY_RIGHT_ALT: return SDLK_RALT;
        case MCUGDX_KEY_RIGHT_SUPER: return SDLK_RGUI;
        case MCUGDX_KEY_MENU: return SDLK_MENU;
        default: return SDLK_UNKNOWN;
    }
}
