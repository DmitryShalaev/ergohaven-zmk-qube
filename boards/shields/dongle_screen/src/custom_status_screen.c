/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "custom_status_screen.h"

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
#include "widgets/output_status.h"
static struct zmk_widget_output_status output_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
#include "widgets/layer_status.h"
static struct zmk_widget_layer_status layer_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
#include "widgets/battery_status.h"
static struct zmk_widget_dongle_battery_status dongle_battery_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_WPM_ACTIVE
#include "widgets/wpm_status.h"
static struct zmk_widget_wpm_status wpm_status_widget;
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
#include "widgets/mod_status.h"
static struct zmk_widget_mod_status mod_widget;
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "fonts.h"

LV_IMG_DECLARE(eh_robot);

#define QUBE_LAYER_TOP_OFFSET 36

lv_obj_t *screen_splash;
lv_obj_t *screen_main;

static void on_screen_main_timer(struct k_timer *dummy) {
    lv_scr_load(screen_main);
}

K_TIMER_DEFINE(screen_main_timer, on_screen_main_timer, NULL);

lv_style_t global_style;

lv_obj_t *zmk_display_status_screen()
{
    screen_main = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_main, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_main, 255, LV_PART_MAIN);

    lv_style_init(&global_style);
    // lv_style_set_text_font(&global_style, &lv_font_unscii_8); // ToDo: Font is not recognized
    lv_style_set_text_color(&global_style, lv_color_white());
    lv_style_set_text_letter_space(&global_style, 1);
    lv_style_set_text_line_space(&global_style, 1);
    lv_obj_add_style(screen_main, &global_style, LV_PART_MAIN);

#if CONFIG_DONGLE_SCREEN_OUTPUT_ACTIVE
    zmk_widget_output_status_init(&output_status_widget, screen_main);
    lv_obj_align(zmk_widget_output_status_obj(&output_status_widget), LV_ALIGN_TOP_RIGHT, -30, 20);
#endif

#if CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
    zmk_widget_dongle_battery_status_init(&dongle_battery_status_widget, screen_main);
    lv_obj_align(zmk_widget_dongle_battery_status_obj(&dongle_battery_status_widget), LV_ALIGN_BOTTOM_MID, 0, 0);
#endif

#if CONFIG_DONGLE_SCREEN_WPM_ACTIVE
    zmk_widget_wpm_status_init(&wpm_status_widget, screen_main);
    lv_obj_align(zmk_widget_wpm_status_obj(&wpm_status_widget), LV_ALIGN_TOP_LEFT, 20, 20);
#endif

#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
    zmk_widget_layer_status_init(&layer_status_widget, screen_main);
    lv_obj_align(zmk_widget_layer_status_obj(&layer_status_widget), LV_ALIGN_TOP_MID, 0,
                 QUBE_LAYER_TOP_OFFSET);
#endif

#if CONFIG_DONGLE_SCREEN_MODIFIER_ACTIVE
    zmk_widget_mod_status_init(&mod_widget, screen_main);
#if CONFIG_DONGLE_SCREEN_LAYER_ACTIVE && CONFIG_DONGLE_SCREEN_BATTERY_ACTIVE
    lv_obj_update_layout(screen_main);

    lv_obj_t *layer_obj = zmk_widget_layer_status_obj(&layer_status_widget);
    lv_obj_t *battery_obj = zmk_widget_dongle_battery_status_obj(&dongle_battery_status_widget);
    lv_obj_t *modifier_obj = zmk_widget_mod_status_obj(&mod_widget);
    const lv_coord_t layer_bottom = lv_obj_get_y(layer_obj) + lv_obj_get_height(layer_obj);
    const lv_coord_t free_space = lv_obj_get_y(battery_obj) - layer_bottom;
    const lv_coord_t modifier_y = layer_bottom + (free_space - lv_obj_get_height(modifier_obj)) / 2;

    lv_obj_align(modifier_obj, LV_ALIGN_TOP_MID, 0, modifier_y);
#elif CONFIG_DONGLE_SCREEN_LAYER_ACTIVE
    lv_obj_align_to(zmk_widget_mod_status_obj(&mod_widget),
                    zmk_widget_layer_status_obj(&layer_status_widget), LV_ALIGN_OUT_BOTTOM_MID, 0,
                    16);
#else
    lv_obj_align(zmk_widget_mod_status_obj(&mod_widget), LV_ALIGN_CENTER, 0, 20);
#endif
#endif

    k_timer_start(&screen_main_timer, K_SECONDS(2), K_NO_WAIT);

    screen_splash = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_splash, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen_splash, 255, LV_PART_MAIN);
    lv_obj_add_style(screen_splash, &global_style, LV_PART_MAIN);

    lv_obj_t *img = lv_img_create(screen_splash);
    lv_img_set_src(img, &eh_robot);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_pad_top(img, 60, 0);
    // lv_obj_set_style_pad_bottom(img, 60, 0);

    // lv_obj_t *label = lv_label_create(screen_splash);
    // lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    // lv_label_set_text(label, CONFIG_ZMK_KEYBOARD_NAME);
    // lv_obj_set_style_text_font(label, &nerd_fonts_big, 0);

    return screen_splash;
}
