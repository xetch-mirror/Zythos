#include "keyboard.h"
#include "keycodes.h"
#include "keymap.h"
#include "ps2.h"

/* Текущее состояние модификаторов */
static uint32_t current_mods = 0;

/* Таблица перевода Set 1 scancode -> keycode_t (без extended-байта 0xE0) */
static const uint16_t set1_table[128] = {
    [0x01] = KEY_ESC,
    [0x02] = KEY_1, [0x03] = KEY_2, [0x04] = KEY_3, [0x05] = KEY_4,
    [0x06] = KEY_5, [0x07] = KEY_6, [0x08] = KEY_7, [0x09] = KEY_8,
    [0x0A] = KEY_9, [0x0B] = KEY_0,
    [0x0C] = KEY_MINUS,
    [0x0D] = KEY_EQUAL,
    [0x0E] = KEY_BACKSPACE,
    [0x0F] = KEY_TAB,

    [0x10] = KEY_Q, [0x11] = KEY_W, [0x12] = KEY_E, [0x13] = KEY_R,
    [0x14] = KEY_T, [0x15] = KEY_Y, [0x16] = KEY_U, [0x17] = KEY_I,
    [0x18] = KEY_O, [0x19] = KEY_P,
    [0x1A] = KEY_LBRACKET,
    [0x1B] = KEY_RBRACKET,
    [0x1C] = KEY_ENTER,
    [0x1D] = KEY_LEFT_CTRL,

    [0x1E] = KEY_A, [0x1F] = KEY_S, [0x20] = KEY_D, [0x21] = KEY_F,
    [0x22] = KEY_G, [0x23] = KEY_H, [0x24] = KEY_J, [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_SEMICOLON,
    [0x28] = KEY_APOSTROPHE,
    [0x29] = KEY_GRAVE,

    [0x2A] = KEY_LEFT_SHIFT,
    [0x2B] = KEY_BACKSLASH,
    [0x2C] = KEY_Z, [0x2D] = KEY_X, [0x2E] = KEY_C, [0x2F] = KEY_V,
    [0x30] = KEY_B, [0x31] = KEY_N, [0x32] = KEY_M,
    [0x33] = KEY_COMMA,
    [0x34] = KEY_DOT,
    [0x35] = KEY_SLASH,
    [0x36] = KEY_RIGHT_SHIFT,

    [0x37] = KEY_KP_STAR,
    [0x38] = KEY_LEFT_ALT,
    [0x39] = KEY_SPACE,
    [0x3A] = KEY_CAPS_LOCK,

    [0x3B] = KEY_F1, [0x3C] = KEY_F2, [0x3D] = KEY_F3, [0x3E] = KEY_F4,
    [0x3F] = KEY_F5, [0x40] = KEY_F6, [0x41] = KEY_F7, [0x42] = KEY_F8,
    [0x43] = KEY_F9, [0x44] = KEY_F10,
    [0x45] = KEY_NUM_LOCK,
    [0x46] = KEY_SCROLL_LOCK,

    [0x47] = KEY_KP_7, [0x48] = KEY_KP_8, [0x49] = KEY_KP_9,
    [0x4A] = KEY_KP_MINUS,
    [0x4B] = KEY_KP_4, [0x4C] = KEY_KP_5, [0x4D] = KEY_KP_6,
    [0x4E] = KEY_KP_PLUS,
    [0x4F] = KEY_KP_1, [0x50] = KEY_KP_2, [0x51] = KEY_KP_3,
    [0x52] = KEY_KP_0,
    [0x53] = KEY_KP_DOT,

    [0x57] = KEY_F11,
    [0x58] = KEY_F12,
};

/* Таблица extended (0xE0-префикс) scancode -> keycode_t */
static const uint16_t set1_extended_table[128] = {
    [0x1C] = KEY_KP_ENTER,
    [0x1D] = KEY_RIGHT_CTRL,
    [0x35] = KEY_KP_SLASH,
    [0x38] = KEY_RIGHT_ALT,

    [0x47] = KEY_HOME,
    [0x48] = KEY_ARROW_UP,
    [0x49] = KEY_PAGE_UP,
    [0x4B] = KEY_ARROW_LEFT,
    [0x4D] = KEY_ARROW_RIGHT,
    [0x4F] = KEY_END,
    [0x50] = KEY_ARROW_DOWN,
    [0x51] = KEY_PAGE_DOWN,
    [0x52] = KEY_INSERT,
    [0x53] = KEY_DELETE,
};

uint16_t keyboard_keycode_from_set1(uint8_t scancode, bool extended)
{
    uint8_t index = scancode & 0x7F; /* сбросить bit7 (release flag) */

    if (index >= 128) {
        return KEY_NONE;
    }

    if (extended) {
        return set1_extended_table[index];
    }

    return set1_table[index];
}

static void update_modifier(uint16_t keycode, bool pressed)
{
    switch (keycode) {
        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT:
            if (pressed) current_mods |= KB_MOD_SHIFT;
            else         current_mods &= ~KB_MOD_SHIFT;
            break;

        case KEY_LEFT_CTRL:
        case KEY_RIGHT_CTRL:
            if (pressed) current_mods |= KB_MOD_CTRL;
            else         current_mods &= ~KB_MOD_CTRL;
            break;

        case KEY_LEFT_ALT:
            if (pressed) current_mods |= KB_MOD_ALT;
            else         current_mods &= ~KB_MOD_ALT;
            break;

        case KEY_RIGHT_ALT:
            if (pressed) current_mods |= KB_MOD_ALTGR;
            else         current_mods &= ~KB_MOD_ALTGR;
            break;

        case KEY_CAPS_LOCK:
            /* переключается по нажатию, не по удержанию */
            if (pressed) current_mods ^= KB_MOD_CAPS;
            break;

        case KEY_NUM_LOCK:
            if (pressed) current_mods ^= KB_MOD_NUM;
            break;

        case KEY_SCROLL_LOCK:
            if (pressed) current_mods ^= KB_MOD_SCROLL;
            break;

        default:
            break;
    }
}

bool keyboard_handle_set1_scancode(uint8_t scancode, keyboard_event_t *ev)
{
    static bool pending_extended = false;

    if (scancode == 0xE0) {
        pending_extended = true;
        return false; /* ещё не событие, ждём следующий байт */
    }

    bool extended = pending_extended;
    pending_extended = false;

    bool pressed = (scancode & 0x80) == 0;
    uint16_t keycode = keyboard_keycode_from_set1(scancode, extended);

    if (keycode == KEY_NONE) {
        return false;
    }

    update_modifier(keycode, pressed);

    keymap_result_t translated = keymap_translate_keycode(keycode, current_mods);

    ev->keycode   = keycode;
    ev->pressed   = pressed;
    ev->repeat    = false; /* повтор определяется на уровне PS/2, не здесь */
    ev->mods      = current_mods;
    ev->codepoint = translated.codepoint;
    ev->is_text   = translated.is_text && pressed;

    return true;
}

void keyboard_init(void)
{
    current_mods = 0;
    keymap_init();
    /* ps2_init() должен быть вызван до этого в общей инициализации ядра */
}

bool keyboard_shift_pressed(void)
{
    return (current_mods & KB_MOD_SHIFT) != 0;
}

bool keyboard_ctrl_pressed(void)
{
    return (current_mods & KB_MOD_CTRL) != 0;
}

uint32_t keyboard_get_modifiers(void)
{
    return current_mods;
}