/* SPDX-License-Identifier: GPL-2.0-or-later */

/*
 * This keymap extends the stock RuEn implementation with additional
 * functionality derived from the Ergohaven RuEn mode documentation.  In
 * particular it adds custom keycodes for common punctuation and symbols
 * so that they can be typed consistently regardless of the current
 * language layout.  Where a symbol only exists in one layout (for
 * example, square brackets only exist on the US layout and the «№»
 * character only exists on the Russian layout) the firmware will
 * automatically switch to the appropriate layout, send the symbol and
 * then revert back to the previous layout.  A handful of helper
 * keycodes are also provided for synchronising the internal layout state
 * (`RUEN_SYNC`), temporarily switching layouts while a key is held
 * (`RUEN_MOD`), storing/restoring the current layout (`RUEN_STORE` and
 * `RUEN_REVERT`) and a simple “RuEn word” mode that allows entering a
 * single English word while typing in Russian (`RUEN_WORD`).
 */

#include QMK_KEYBOARD_H

/*
 * Custom keycodes for the RuEn mode.  These keycodes allow the firmware
 * to toggle between Russian and English layouts, force a specific
 * layout or insert punctuation/symbols consistently across layouts.
 *
 * New keycodes defined here MUST be mirrored in the customKeycodes
 * section of the corresponding `vial.json` so that Vial displays
 * friendly names in the GUI.
 */
enum custom_keycodes {
    RUEN_TOGGLE = SAFE_RANGE,
    RUEN_EN,
    RUEN_RU,
    /* Punctuation available on both layouts – no layout switching */
    RUEN_DOT,
    RUEN_COMMA,
    RUEN_SCLN,
    RUEN_COLON,
    RUEN_DQUOTE,
    RUEN_QST,
    RUEN_SLASH,
    /* Symbols only on the US layout – temporarily switch from RU */
    RUEN_LBRC,
    RUEN_RBRC,
    RUEN_LCBR,
    RUEN_RCBR,
    RUEN_LT,
    RUEN_GT,
    RUEN_GRAVE,
    RUEN_TILDE,
    RUEN_AT,
    RUEN_HASH,
    RUEN_DOLLAR,
    RUEN_CARET,
    RUEN_AMP,
    RUEN_PIPE,
    /* Russian‑only symbol */
    RUEN_NUM,
    /* Layout management helpers */
    RUEN_SYNC,
    RUEN_MOD,
    RUEN_WORD,
    RUEN_STORE,
    RUEN_REVERT,
};

/* Track whether the keyboard believes it is currently in Russian layout.
 * This variable is toggled whenever a RuEn key sends a layout switch
 * sequence so that future calls know whether to toggle again.
 *
 * The keyboard starts up assuming the host is in English layout.  If
 * your system boots into Russian by default you may set this to true.
 */
static bool ruen_is_russian = false;

/* Stored state for RUEN_STORE/RUEN_REVERT */
static bool ruen_saved_state = false;

/* Flag for RuEn word mode – when enabled the keyboard stays in English
 * until a delimiter (space, enter, minus or escape) is pressed.  At
 * that point the layout is returned to Russian and the flag cleared. */
static bool ruen_word_active = false;

/* Send the OS‑level layout switch.  Most operating systems toggle
 * keyboard layout when pressing Win+Space.  If your system uses a
 * different shortcut you can modify the tap_code16() call here
 * accordingly. */
static void ruen_send_layout_switch(void) {
    tap_code16(LGUI(KC_SPACE));
}

/* Toggle the internal and OS layout */
static void ruen_toggle_layout(void) {
    ruen_send_layout_switch();
    ruen_is_russian = !ruen_is_russian;
}

/* Force English layout */
static void ruen_set_en(void) {
    if (ruen_is_russian) {
        ruen_send_layout_switch();
        ruen_is_russian = false;
    }
}

/* Force Russian layout */
static void ruen_set_ru(void) {
    if (!ruen_is_russian) {
        ruen_send_layout_switch();
        ruen_is_russian = true;
    }
}

/* Temporarily switch to the opposite layout, send the keycode and return
 * back.  If shift_required is true then the key will be sent with
 * Shift. */
static void ruen_send_us_symbol(uint16_t kc, bool shift_required) {
    if (ruen_is_russian) {
        ruen_toggle_layout();
        if (shift_required) {
            tap_code16(LSFT(kc));
        } else {
            tap_code16(kc);
        }
        ruen_toggle_layout();
    } else {
        if (shift_required) {
            tap_code16(LSFT(kc));
        } else {
            tap_code16(kc);
        }
    }
}

/* Temporarily switch to Russian layout, send the keycode and return
 * back.  This is used for the «№» symbol. */
static void ruen_send_ru_symbol(uint16_t kc, bool shift_required) {
    if (!ruen_is_russian) {
        ruen_toggle_layout();
        if (shift_required) {
            tap_code16(LSFT(kc));
        } else {
            tap_code16(kc);
        }
        ruen_toggle_layout();
    } else {
        if (shift_required) {
            tap_code16(LSFT(kc));
        } else {
            tap_code16(kc);
        }
    }
}

/* Store the current RuEn state */
static void ruen_store(void) {
    ruen_saved_state = ruen_is_russian;
}

/* Restore the stored RuEn state */
static void ruen_revert(void) {
    if (ruen_is_russian != ruen_saved_state) {
        ruen_toggle_layout();
    }
}

/* Intercept keypresses for our custom RuEn keycodes.  When a RuEn
 * key is pressed the appropriate helper function is called and the
 * event is consumed so that no other action occurs.  For RuEn word
 * mode the keypress is also examined to determine whether the mode
 * should be terminated. */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    /* When RuEn word is active, watch for delimiters to revert to
     * Russian layout.  Do this before handling custom keycodes so
     * that the delimiter itself still registers normally. */
    if (ruen_word_active && record->event.pressed) {
        switch (keycode) {
            case KC_SPC:
            case KC_ENT:
            case KC_ESC:
            case KC_MINS:
                ruen_set_ru();
                ruen_word_active = false;
                break;
            default:
                break;
        }
    }

    if (record->event.pressed) {
        switch (keycode) {
            case RUEN_TOGGLE:
                ruen_toggle_layout();
                return false;
            case RUEN_EN:
                ruen_set_en();
                return false;
            case RUEN_RU:
                ruen_set_ru();
                return false;
            case RUEN_DOT:
                tap_code16(KC_DOT);
                return false;
            case RUEN_COMMA:
                tap_code16(KC_COMM);
                return false;
            case RUEN_SCLN:
                tap_code16(KC_SCLN);
                return false;
            case RUEN_COLON:
                tap_code16(LSFT(KC_SCLN));
                return false;
            case RUEN_DQUOTE:
                tap_code16(LSFT(KC_QUOT));
                return false;
            case RUEN_QST:
                tap_code16(LSFT(KC_SLSH));
                return false;
            case RUEN_SLASH:
                tap_code16(KC_SLSH);
                return false;
            case RUEN_LBRC:
                ruen_send_us_symbol(KC_LBRC, false);
                return false;
            case RUEN_RBRC:
                ruen_send_us_symbol(KC_RBRC, false);
                return false;
            case RUEN_LCBR:
                ruen_send_us_symbol(KC_LBRC, true);
                return false;
            case RUEN_RCBR:
                ruen_send_us_symbol(KC_RBRC, true);
                return false;
            case RUEN_LT:
                ruen_send_us_symbol(KC_COMM, true);
                return false;
            case RUEN_GT:
                ruen_send_us_symbol(KC_DOT, true);
                return false;
            case RUEN_GRAVE:
                ruen_send_us_symbol(KC_GRV, false);
                return false;
            case RUEN_TILDE:
                ruen_send_us_symbol(KC_GRV, true);
                return false;
            case RUEN_AT:
                ruen_send_us_symbol(KC_2, true);
                return false;
            case RUEN_HASH:
                ruen_send_us_symbol(KC_3, true);
                return false;
            case RUEN_DOLLAR:
                ruen_send_us_symbol(KC_4, true);
                return false;
            case RUEN_CARET:
                ruen_send_us_symbol(KC_6, true);
                return false;
            case RUEN_AMP:
                ruen_send_us_symbol(KC_7, true);
                return false;
            case RUEN_PIPE:
                ruen_send_us_symbol(KC_BSLS, true);
                return false;
            case RUEN_NUM:
                /* The «№» symbol is shift+3 on the Russian layout */
                ruen_send_ru_symbol(KC_3, true);
                return false;
            case RUEN_SYNC:
                /* Toggle internal state without sending anything to the OS */
                ruen_is_russian = !ruen_is_russian;
                return false;
            case RUEN_MOD:
                /* While held, temporarily switch layout; revert on release */
                ruen_toggle_layout();
                return false;
            case RUEN_WORD:
                /* Start a RuEn word: switch to English and stay until delim */
                ruen_set_en();
                ruen_word_active = true;
                return false;
            case RUEN_STORE:
                ruen_store();
                return false;
            case RUEN_REVERT:
                ruen_revert();
                return false;
            default:
                break;
        }
    } else {
        /* key release events */
        switch (keycode) {
            case RUEN_MOD:
                ruen_toggle_layout();
                return false;
            default:
                break;
        }
    }
    return true;
}

/* Keymap layers.
 *
 * Layer 0 is the default typing layer.  The bottom row has been
 * modified to expose the RuEn functions:
 *  - RUEN_TOGGLE replaces the left GUI key (switches the current layout).
 *  - MO(1) still momentarily accesses layer 1.
 *  - KC_SPC and KC_ENT remain unchanged.
 *  - RUEN_EN and RUEN_RU replace the right control and right alt keys
 *    respectively.  Press them to force English or Russian layout.
 *
 * Additional RuEn symbols can be added to unused keys in any layer via
 * Vial.
 */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,
                               KC_6,    KC_7,    KC_8,    KC_9,    KC_0,
        KC_MINS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,
                               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,
        KC_BSPC,
        KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
                               KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN,
        KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,
                               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,
        KC_RSFT,
                                                    RUEN_TOGGLE, MO(1), KC_SPC,
        KC_ENT,  RUEN_EN,  RUEN_RU
    ),
    [1] = LAYOUT(
        KC_GRV,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,
                               KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,
        KC_F11,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_PGUP, KC_PGDN, KC_HOME, KC_END,  KC_DEL,
        KC_F12,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_LBRC,
        KC_RBRC,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    ),
    [2] = LAYOUT(
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    ),
    [3] = LAYOUT(
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    ),
    [4] = LAYOUT(
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    ),
    [5] = LAYOUT(
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    ),
    [6] = LAYOUT(
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    ),
    [7] = LAYOUT(
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
        KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                               KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,
                                                    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,  KC_TRNS,  KC_TRNS
    )
};

/* Chordal hold layout is retained from the original Vial keymap.  It is
 * used to determine which side of the split keyboard a key resides on
 * when applying chord-hold behaviour.  There is no need to modify this
 * for RuEn. */
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM =
    LAYOUT(
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
                       'L', 'L', 'L',  'R', 'R', 'R'
    );