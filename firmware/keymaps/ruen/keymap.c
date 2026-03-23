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
    RUEN_TOGGLE = QK_KB_0,
    RUEN_EN,
    RUEN_RU,
    RUEN_DOT,
    RUEN_COMMA,
    RUEN_SCLN,
    RUEN_COLON,
    RUEN_DQUOTE,
    RUEN_QST,
    RUEN_SLASH,
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
    RUEN_NUM,
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
static bool ruen_mod_restore_state = false;
static uint16_t ruen_mod_timer = 0;


static bool ruen_word_active = false;

/* Send the OS‑level layout switch.  Most operating systems toggle
 * keyboard layout when pressing Win+Space.  If your system uses a
 * different shortcut you can modify the tap_code16() call here
 * accordingly. */
static void ruen_send_layout_switch(void) {
    uint8_t mods = get_mods();

    if (mods != 0) {
        del_mods(mods);
    }

    if (keymap_config.swap_lctl_lgui) {
        register_code(KC_LCTL);
        tap_code(KC_SPACE);
        wait_ms(50);
        unregister_code(KC_LCTL);
        wait_ms(50);
    } else {
        register_code(KC_LGUI);
        tap_code(KC_SPACE);
        wait_ms(50);
        unregister_code(KC_LGUI);
        wait_ms(50);
    }

    if (mods != 0) {
        add_mods(mods);
    }
}

static void ruen_set_layout(bool russian) {
    if (ruen_is_russian == russian) {
        return;
    }

    ruen_send_layout_switch();
    ruen_is_russian = russian;
}

/* Toggle the internal and OS layout */
static void ruen_toggle_layout(void) {
    ruen_set_layout(!ruen_is_russian);
}

/* Force English */
static void ruen_set_en(void) {
    ruen_set_layout(false);
}
static void ruen_set_ru(void) {
    ruen_set_layout(true);
}

/* Send a symbol that exists on both layouts but uses different
 * keycodes in Russian and English. */
static void ruen_send_layout_symbol(uint16_t en_keycode, uint16_t ru_keycode) {
    tap_code16(ruen_is_russian ? ru_keycode : en_keycode);
}

/* Temporarily switch to the requested layout, send the symbol and then
 * restore the previous layout. */
static void ruen_send_temporary_symbol(bool russian_layout, uint16_t keycode) {
    bool previous_layout = ruen_is_russian;

    ruen_set_layout(russian_layout);
    tap_code16(keycode);
    ruen_set_layout(previous_layout);
}

static void ruen_send_us_symbol(uint16_t keycode) {
    ruen_send_temporary_symbol(false, keycode);
}

/* Temporarily switch to Russian layout, send the keycode and return
 * back.  This is used for the «№» symbol. */
static void ruen_send_ru_symbol(uint16_t keycode) {
    ruen_send_temporary_symbol(true, keycode);
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
                caps_word_off();
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
                ruen_send_layout_symbol(KC_DOT, KC_SLSH);
                return false;
            case RUEN_COMMA:
                ruen_send_layout_symbol(KC_COMM, LSFT(KC_SLSH));
                return false;
            case RUEN_SCLN:
                ruen_send_layout_symbol(KC_SCLN, LSFT(KC_4));
                return false;
            case RUEN_COLON:
                ruen_send_layout_symbol(LSFT(KC_SCLN), LSFT(KC_6));
                return false;
            case RUEN_DQUOTE:
                ruen_send_layout_symbol(LSFT(KC_QUOT), LSFT(KC_2));
                return false;
            case RUEN_QST:
                ruen_send_layout_symbol(LSFT(KC_SLSH), LSFT(KC_7));
                return false;
            case RUEN_SLASH:
                ruen_send_layout_symbol(KC_SLSH, LSFT(KC_BSLS));
                return false;
            case RUEN_LBRC:
                ruen_send_us_symbol(KC_LBRC);
                return false;
            case RUEN_RBRC:
                ruen_send_us_symbol(KC_RBRC);
                return false;
            case RUEN_LCBR:
                ruen_send_us_symbol(LSFT(KC_LBRC));
                return false;
            case RUEN_RCBR:
                ruen_send_us_symbol(LSFT(KC_RBRC));
                return false;
            case RUEN_LT:
                ruen_send_us_symbol(LSFT(KC_COMM));
                return false;
            case RUEN_GT:
                ruen_send_us_symbol(LSFT(KC_DOT));
                return false;
            case RUEN_GRAVE:
                ruen_send_us_symbol(KC_GRV);
                return false;
            case RUEN_TILDE:
                ruen_send_us_symbol(LSFT(KC_GRV));
                return false;
            case RUEN_AT:
                ruen_send_us_symbol(LSFT(KC_2));
                return false;
            case RUEN_HASH:
                ruen_send_us_symbol(LSFT(KC_3));
                return false;
            case RUEN_DOLLAR:
                ruen_send_us_symbol(LSFT(KC_4));
                return false;
            case RUEN_CARET:
                ruen_send_us_symbol(LSFT(KC_6));
                return false;
            case RUEN_AMP:
                ruen_send_us_symbol(LSFT(KC_7));
                return false;
            case RUEN_PIPE:
                ruen_send_us_symbol(LSFT(KC_BSLS));
                return false;
            case RUEN_NUM:
                ruen_send_ru_symbol(LSFT(KC_3));
                return false;
            case RUEN_SYNC:
                ruen_is_russian = !ruen_is_russian;
                return false;
            case RUEN_MOD:
                ruen_toggle_layout();
                ruen_mod_restore_state = !ruen_is_russian;
                ruen_mod_timer = timer_read();
                return false;
            case RUEN_WORD:
                if (ruen_is_russian && !ruen_word_active) {
                    bool shift_active =
                        (get_mods() | get_oneshot_mods() | get_weak_mods()) &
                        MOD_MASK_SHIFT;

                    if (get_oneshot_mods() & MOD_MASK_SHIFT) {
                        clear_oneshot_mods();
                    }

                    ruen_set_en();
                    ruen_word_active = true;

                    if (shift_active) {
                        caps_word_on();
                    }
                }
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
                if (timer_elapsed(ruen_mod_timer) >=
                    get_tapping_term(keycode, record)) {
                    ruen_set_layout(ruen_mod_restore_state);
                }
                return false;
            default:
                break;
        }
    }
    return true;
}


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

const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM =
    LAYOUT(
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
                       'L', 'L', 'L',  'R', 'R', 'R'
    );
