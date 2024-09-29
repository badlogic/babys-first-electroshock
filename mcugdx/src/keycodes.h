#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MCUGDX_KEY_UNKNOWN       = -1,

    MCUGDX_KEY_SPACE         = 32,
    MCUGDX_KEY_APOSTROPHE    = 39,
    MCUGDX_KEY_COMMA         = 44,
    MCUGDX_KEY_MINUS         = 45,
    MCUGDX_KEY_PERIOD        = 46,
    MCUGDX_KEY_SLASH         = 47,
    MCUGDX_KEY_0             = 48,
    MCUGDX_KEY_1             = 49,
    MCUGDX_KEY_2             = 50,
    MCUGDX_KEY_3             = 51,
    MCUGDX_KEY_4             = 52,
    MCUGDX_KEY_5             = 53,
    MCUGDX_KEY_6             = 54,
    MCUGDX_KEY_7             = 55,
    MCUGDX_KEY_8             = 56,
    MCUGDX_KEY_9             = 57,
    MCUGDX_KEY_SEMICOLON     = 59,
    MCUGDX_KEY_EQUAL         = 61,
    MCUGDX_KEY_A             = 65,
    MCUGDX_KEY_B             = 66,
    MCUGDX_KEY_C             = 67,
    MCUGDX_KEY_D             = 68,
    MCUGDX_KEY_E             = 69,
    MCUGDX_KEY_F             = 70,
    MCUGDX_KEY_G             = 71,
    MCUGDX_KEY_H             = 72,
    MCUGDX_KEY_I             = 73,
    MCUGDX_KEY_J             = 74,
    MCUGDX_KEY_K             = 75,
    MCUGDX_KEY_L             = 76,
    MCUGDX_KEY_M             = 77,
    MCUGDX_KEY_N             = 78,
    MCUGDX_KEY_O             = 79,
    MCUGDX_KEY_P             = 80,
    MCUGDX_KEY_Q             = 81,
    MCUGDX_KEY_R             = 82,
    MCUGDX_KEY_S             = 83,
    MCUGDX_KEY_T             = 84,
    MCUGDX_KEY_U             = 85,
    MCUGDX_KEY_V             = 86,
    MCUGDX_KEY_W             = 87,
    MCUGDX_KEY_X             = 88,
    MCUGDX_KEY_Y             = 89,
    MCUGDX_KEY_Z             = 90,
    MCUGDX_KEY_LEFT_BRACKET  = 91,
    MCUGDX_KEY_BACKSLASH     = 92,
    MCUGDX_KEY_RIGHT_BRACKET = 93,
    MCUGDX_KEY_GRAVE_ACCENT  = 96,
    MCUGDX_KEY_WORLD_1       = 161,
    MCUGDX_KEY_WORLD_2       = 162,

    MCUGDX_KEY_ESCAPE        = 256,
    MCUGDX_KEY_ENTER         = 257,
    MCUGDX_KEY_TAB           = 258,
    MCUGDX_KEY_BACKSPACE     = 259,
    MCUGDX_KEY_INSERT        = 260,
    MCUGDX_KEY_DELETE        = 261,
    MCUGDX_KEY_RIGHT         = 262,
    MCUGDX_KEY_LEFT          = 263,
    MCUGDX_KEY_DOWN          = 264,
    MCUGDX_KEY_UP            = 265,
    MCUGDX_KEY_PAGE_UP       = 266,
    MCUGDX_KEY_PAGE_DOWN     = 267,
    MCUGDX_KEY_HOME          = 268,
    MCUGDX_KEY_END           = 269,
    MCUGDX_KEY_CAPS_LOCK     = 280,
    MCUGDX_KEY_SCROLL_LOCK   = 281,
    MCUGDX_KEY_NUM_LOCK      = 282,
    MCUGDX_KEY_PRINT_SCREEN  = 283,
    MCUGDX_KEY_PAUSE         = 284,
    MCUGDX_KEY_F1            = 290,
    MCUGDX_KEY_F2            = 291,
    MCUGDX_KEY_F3            = 292,
    MCUGDX_KEY_F4            = 293,
    MCUGDX_KEY_F5            = 294,
    MCUGDX_KEY_F6            = 295,
    MCUGDX_KEY_F7            = 296,
    MCUGDX_KEY_F8            = 297,
    MCUGDX_KEY_F9            = 298,
    MCUGDX_KEY_F10           = 299,
    MCUGDX_KEY_F11           = 300,
    MCUGDX_KEY_F12           = 301,
    MCUGDX_KEY_F13           = 302,
    MCUGDX_KEY_F14           = 303,
    MCUGDX_KEY_F15           = 304,
    MCUGDX_KEY_F16           = 305,
    MCUGDX_KEY_F17           = 306,
    MCUGDX_KEY_F18           = 307,
    MCUGDX_KEY_F19           = 308,
    MCUGDX_KEY_F20           = 309,
    MCUGDX_KEY_F21           = 310,
    MCUGDX_KEY_F22           = 311,
    MCUGDX_KEY_F23           = 312,
    MCUGDX_KEY_F24           = 313,
    MCUGDX_KEY_F25           = 314,
    MCUGDX_KEY_KP_0          = 320,
    MCUGDX_KEY_KP_1          = 321,
    MCUGDX_KEY_KP_2          = 322,
    MCUGDX_KEY_KP_3          = 323,
    MCUGDX_KEY_KP_4          = 324,
    MCUGDX_KEY_KP_5          = 325,
    MCUGDX_KEY_KP_6          = 326,
    MCUGDX_KEY_KP_7          = 327,
    MCUGDX_KEY_KP_8          = 328,
    MCUGDX_KEY_KP_9          = 329,
    MCUGDX_KEY_KP_DECIMAL    = 330,
    MCUGDX_KEY_KP_DIVIDE     = 331,
    MCUGDX_KEY_KP_MULTIPLY   = 332,
    MCUGDX_KEY_KP_SUBTRACT   = 333,
    MCUGDX_KEY_KP_ADD        = 334,
    MCUGDX_KEY_KP_ENTER      = 335,
    MCUGDX_KEY_KP_EQUAL      = 336,
    MCUGDX_KEY_LEFT_SHIFT    = 340,
    MCUGDX_KEY_LEFT_CONTROL  = 341,
    MCUGDX_KEY_LEFT_ALT      = 342,
    MCUGDX_KEY_LEFT_SUPER    = 343,
    MCUGDX_KEY_RIGHT_SHIFT   = 344,
    MCUGDX_KEY_RIGHT_CONTROL = 345,
    MCUGDX_KEY_RIGHT_ALT     = 346,
    MCUGDX_KEY_RIGHT_SUPER   = 347,
    MCUGDX_KEY_MENU          = 348
} mcugdx_keycode_t;

#define MCUGDX_KEY_LAST MCUGDX_KEY_MENU

const char *mcugdx_keycode_to_string(mcugdx_keycode_t keycode);

#ifdef __cplusplus
}
#endif