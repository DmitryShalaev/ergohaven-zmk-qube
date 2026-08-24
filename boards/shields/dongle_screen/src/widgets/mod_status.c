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

#include <dt-bindings/zmk/modifiers.h>
#include <zmk/display.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>

#include "mod_status.h"

#define ZMK_MOD_STATUS_CAPS_LOCK_LED 0x02U
#define ZMK_MOD_STATUS_REFRESH_MS 50
#define ZMK_MOD_STATUS_CHIP_HEIGHT 24
#define ZMK_MOD_STATUS_SECOND_ROW_Y 32
#define ZMK_MOD_STATUS_WIDGET_HEIGHT 56

#define RMK_COLOR_FOREGROUND 0xEFF7F7U
#define RMK_COLOR_DIM 0x98A4E8U
#define RMK_COLOR_ACCENT 0x189AFFU
#define RMK_COLOR_ACCENT_DIM 0x084194U

static const char *const chip_labels[ZMK_MOD_STATUS_CHIP_COUNT] = {
    "ALT", "SHIFT", "CTRL", "GUI", "CAPS",
};

static const lv_coord_t chip_x[ZMK_MOD_STATUS_CHIP_COUNT] = {
    23, 77, 147, 52, 114,
};

static const lv_coord_t chip_y[ZMK_MOD_STATUS_CHIP_COUNT] = {
    0, 0, 0, ZMK_MOD_STATUS_SECOND_ROW_Y, ZMK_MOD_STATUS_SECOND_ROW_Y,
};

static const lv_coord_t chip_width[ZMK_MOD_STATUS_CHIP_COUNT] = {
    44, 60, 52, 50, 56,
};

static lv_style_t chip_base_style;
static lv_style_t chip_active_style;
static bool chip_styles_initialized;

struct mod_status_state {
    uint8_t modifiers;
    bool caps_lock;
};

static struct mod_status_state mod_status_get_state(void) {
    zmk_hid_indicators_t indicators = zmk_hid_indicators_get_current_profile();

    return (struct mod_status_state){
        .modifiers = zmk_hid_get_keyboard_report()->body.modifiers,
        .caps_lock = (indicators & ZMK_MOD_STATUS_CAPS_LOCK_LED) != 0,
    };
}

static void init_chip_styles(void) {
    if (chip_styles_initialized) {
        return;
    }

    lv_style_init(&chip_base_style);
    lv_style_set_text_font(&chip_base_style, &lv_font_montserrat_16);
    lv_style_set_text_align(&chip_base_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_letter_space(&chip_base_style, 0);
    lv_style_set_text_color(&chip_base_style, lv_color_hex(RMK_COLOR_DIM));
    lv_style_set_pad_all(&chip_base_style, 0);
    lv_style_set_pad_top(&chip_base_style, 3);
    lv_style_set_pad_left(&chip_base_style, 6);
    lv_style_set_pad_right(&chip_base_style, 6);
    lv_style_set_radius(&chip_base_style, 8);
    lv_style_set_bg_color(&chip_base_style, lv_color_hex(RMK_COLOR_ACCENT_DIM));
    lv_style_set_bg_opa(&chip_base_style, LV_OPA_20);
    lv_style_set_border_color(&chip_base_style, lv_color_hex(RMK_COLOR_ACCENT));
    lv_style_set_border_width(&chip_base_style, 1);
    lv_style_set_border_opa(&chip_base_style, LV_OPA_30);

    lv_style_init(&chip_active_style);
    lv_style_set_text_color(&chip_active_style, lv_color_hex(RMK_COLOR_FOREGROUND));
    lv_style_set_bg_color(&chip_active_style, lv_color_hex(RMK_COLOR_ACCENT));
    lv_style_set_bg_opa(&chip_active_style, LV_OPA_COVER);
    lv_style_set_border_color(&chip_active_style, lv_color_hex(RMK_COLOR_ACCENT));
    lv_style_set_border_opa(&chip_active_style, LV_OPA_COVER);

    chip_styles_initialized = true;
}

static void set_chip_state(struct zmk_widget_mod_status *widget, size_t index, bool active) {
    if (widget->chip_active[index] == active) {
        return;
    }

    widget->chip_active[index] = active;
    if (active) {
        lv_obj_add_style(widget->chips[index], &chip_active_style, LV_PART_MAIN);
    } else {
        lv_obj_remove_style(widget->chips[index], &chip_active_style, LV_PART_MAIN);
    }

    lv_obj_invalidate(widget->chips[index]);
}

static void update_mod_status(struct zmk_widget_mod_status *widget) {
    struct mod_status_state state = mod_status_get_state();
    const bool active[ZMK_MOD_STATUS_CHIP_COUNT] = {
        (state.modifiers & (MOD_LALT | MOD_RALT)) != 0,
        (state.modifiers & (MOD_LSFT | MOD_RSFT)) != 0,
        (state.modifiers & (MOD_LCTL | MOD_RCTL)) != 0,
        (state.modifiers & (MOD_LGUI | MOD_RGUI)) != 0,
        state.caps_lock,
    };

    for (size_t i = 0; i < ZMK_MOD_STATUS_CHIP_COUNT; i++) {
        set_chip_state(widget, i, active[i]);
    }
}

static void mod_status_work_cb(struct k_work *work) {
    struct k_work_delayable *delayable = k_work_delayable_from_work(work);
    struct zmk_widget_mod_status *widget =
        CONTAINER_OF(delayable, struct zmk_widget_mod_status, update_work);

    update_mod_status(widget);
    k_work_reschedule_for_queue(zmk_display_work_q(), &widget->update_work,
                                K_MSEC(ZMK_MOD_STATUS_REFRESH_MS));
}

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent) {
    init_chip_styles();

    widget->obj = lv_obj_create(parent);
    lv_obj_remove_style_all(widget->obj);
    lv_obj_set_size(widget->obj, 222, ZMK_MOD_STATUS_WIDGET_HEIGHT);
    lv_obj_clear_flag(widget->obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < ZMK_MOD_STATUS_CHIP_COUNT; i++) {
        lv_obj_t *chip = lv_label_create(widget->obj);
        widget->chips[i] = chip;

        lv_obj_set_pos(chip, chip_x[i], chip_y[i]);
        lv_obj_set_size(chip, chip_width[i], ZMK_MOD_STATUS_CHIP_HEIGHT);
        lv_obj_clear_flag(chip, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        lv_label_set_text(chip, chip_labels[i]);
        lv_obj_add_style(chip, &chip_base_style, LV_PART_MAIN);
        widget->chip_active[i] = false;
    }

    k_work_init_delayable(&widget->update_work, mod_status_work_cb);
    update_mod_status(widget);
    k_work_schedule_for_queue(zmk_display_work_q(), &widget->update_work,
                              K_MSEC(ZMK_MOD_STATUS_REFRESH_MS));

    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget) {
    return widget->obj;
}
