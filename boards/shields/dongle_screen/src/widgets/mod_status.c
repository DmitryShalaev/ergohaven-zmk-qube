/*
 * Copyright (c) 2026 Dmitry Shalaev
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lvgl.h>
#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>

#include "mod_status.h"

#define ZMK_MOD_STATUS_CAPS_LOCK_LED 0x02U

#define RMK_COLOR_FOREGROUND 0xEFF7F7U
#define RMK_COLOR_DIM 0x293073U
#define RMK_COLOR_ACCENT 0x189AFFU
#define RMK_COLOR_ACCENT_DIM 0x084194U

static const char *const chip_labels[ZMK_MOD_STATUS_CHIP_COUNT] = {
    "CAPS", "CTRL", "SHIFT", "ALT", "GUI",
};

static const lv_coord_t chip_x[ZMK_MOD_STATUS_CHIP_COUNT] = {
    0, 46, 92, 146, 188,
};

static const lv_coord_t chip_width[ZMK_MOD_STATUS_CHIP_COUNT] = {
    38, 38, 46, 34, 34,
};

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct mod_status_state {
    uint8_t modifiers;
    bool caps_lock;
};

static struct mod_status_state mod_status_get_state(const zmk_event_t *event) {
    (void)event;

    zmk_hid_indicators_t indicators = zmk_hid_indicators_get_current_profile();

    return (struct mod_status_state){
        .modifiers = zmk_hid_get_keyboard_report()->body.modifiers,
        .caps_lock = (indicators & ZMK_MOD_STATUS_CAPS_LOCK_LED) != 0,
    };
}

static void set_chip_state(lv_obj_t *chip, bool active) {
    lv_obj_set_style_bg_opa(chip, active ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(chip, active ? LV_OPA_COVER : LV_OPA_TRANSP,
                                LV_PART_MAIN);
    lv_obj_set_style_text_color(
        chip, lv_color_hex(active ? RMK_COLOR_FOREGROUND : RMK_COLOR_DIM), LV_PART_MAIN);
}

static void mod_status_update_cb(struct mod_status_state state) {
    const bool active[ZMK_MOD_STATUS_CHIP_COUNT] = {
        state.caps_lock,
        (state.modifiers & (MOD_LCTL | MOD_RCTL)) != 0,
        (state.modifiers & (MOD_LSFT | MOD_RSFT)) != 0,
        (state.modifiers & (MOD_LALT | MOD_RALT)) != 0,
        (state.modifiers & (MOD_LGUI | MOD_RGUI)) != 0,
    };

    struct zmk_widget_mod_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        for (size_t i = 0; i < ZMK_MOD_STATUS_CHIP_COUNT; i++) {
            set_chip_state(widget->chips[i], active[i]);
        }
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_mod_status, struct mod_status_state, mod_status_update_cb,
                            mod_status_get_state)

ZMK_SUBSCRIPTION(widget_mod_status, zmk_modifiers_state_changed);
ZMK_SUBSCRIPTION(widget_mod_status, zmk_hid_indicators_changed);
ZMK_SUBSCRIPTION(widget_mod_status, zmk_endpoint_changed);

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_remove_style_all(widget->obj);
    lv_obj_set_size(widget->obj, 222, 16);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < ZMK_MOD_STATUS_CHIP_COUNT; i++) {
        lv_obj_t *chip = lv_label_create(widget->obj);
        widget->chips[i] = chip;

        lv_obj_set_pos(chip, chip_x[i], 0);
        lv_obj_set_size(chip, chip_width[i], 16);
        lv_obj_clear_flag(chip, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        lv_label_set_text(chip, chip_labels[i]);
        lv_obj_set_style_text_font(chip, &lv_font_unscii_8, LV_PART_MAIN);
        lv_obj_set_style_text_align(chip, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_letter_space(chip, 0, LV_PART_MAIN);

        lv_obj_set_style_pad_all(chip, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_top(chip, 3, LV_PART_MAIN);
        lv_obj_set_style_radius(chip, 7, LV_PART_MAIN);

        lv_obj_set_style_bg_color(chip, lv_color_hex(RMK_COLOR_ACCENT_DIM), LV_PART_MAIN);
        lv_obj_set_style_border_color(chip, lv_color_hex(RMK_COLOR_ACCENT), LV_PART_MAIN);
        lv_obj_set_style_border_width(chip, 1, LV_PART_MAIN);
        set_chip_state(chip, false);
    }

    sys_slist_append(&widgets, &widget->node);
    widget_mod_status_init();

    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget) {
    return widget->obj;
}
