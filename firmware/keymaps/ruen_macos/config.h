/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/* Number of dynamic layers.  Matches the vial keymap. */
#define DYNAMIC_KEYMAP_LAYER_COUNT 8

/* Unique identifier required for Vial.  It is copied from the vial keymap. */
#define VIAL_KEYBOARD_UID {0x97, 0xD0, 0xA5, 0x97, 0x57, 0x48, 0xD0, 0x4F}

/* Unlock combo – allows entering bootloader via a specific key combination. */
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }