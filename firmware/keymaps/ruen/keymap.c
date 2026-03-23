/* SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H

/* Custom keycodes for the RuEn mode.  These keycodes allow the firmware to
 * toggle between Russian and English layouts and force a specific layout.
 *
 * The values begin at SAFE_RANGE so they do not conflict with existing QMK
 * keycodes.  When used in a keymap the corresponding action is defined in
 * process_record_user() below.
 */
enum custom_keycodes {
    RUEN_TOGGLE = SAFE_RANGE,
    RUEN_EN,
    RUEN_RU,
};

/* Track whether the keyboard believes it is currently in Russian layout.
 * This variable is toggled whenever a RuEn key sends a layout switch
 * sequence so that future calls know whether to toggle again.
 *
 * The keyboard starts up assuming the host is in English layout.  If your
 * system boots into Russian by default you may set this to true.
 */
static bool ruen_is_russian = false;

/* Send the OS‑level layout switch.  Most operating systems toggle keyboard
 * layout when pressing Win+Space.  If your system uses a different shortcut
 * you can modify the tap_code16() call here accordingly.
 */
static void ruen_send_layout_switch(void) {
    tap_code16(LGUI(KC_SPACE));
}

static void ruen_toggle_layout(void) {
    ruen_send_layout_switch();
    ruen_is_russian = !ruen_is_russian;
}

static void ruen_set_en(void) {
    if (ruen_is_russian) {
        ruen_send_layout_switch();
        ruen_is_russian = false;
    }
}

static void ruen_set_ru(void) {
    if (!ruen_is_russian) {
        ruen_send_layout_switch();
        ruen_is_russian = true;
    }
}

/* Intercept keypresses for our custom RuEn keycodes.  When a RuEn key is
 * pressed the appropriate helper function is called and the event is
 * consumed so that no other action occurs.
 */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
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
            default:
                break;
        }
    }
    return true;
}

/* Keymap layers.
 *
 * Layer 0 is the default typing layer.
 * The bottom row has been modified to expose the RuEn functions:
 *  - RUEN_TOGGLE replaces the left GUI key (switches the current layout).
 *  - MO(1) still momentarily accesses layer 1.
 *  - KC_SPC and KC_ENT remain unchanged.
 *  - RUEN_EN and RUEN_RU replace the right control and right alt keys
 *    respectively.  Press them to force English or Russian layout.
 *
 * Other layers mirror the stock Vial keymap and can be customised further.
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

/* Chordal hold layout is retained from the original Vial keymap.  It is used
 * to determine which side of the split keyboard a key resides on when
 * applying chord-hold behaviour.  There is no need to modify this for RuEn.
 */
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM =
    LAYOUT(
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
        'L', 'L', 'L', 'L', 'L', 'L',  'R', 'R', 'R', 'R', 'R', 'R',
                       'L', 'L', 'L',  'R', 'R', 'R'
    );