#pragma once

#include <stdbool.h>

#include <lvgl.h>
#include <zephyr/kernel.h>

#define ZMK_MOD_STATUS_CHIP_COUNT 5

struct zmk_widget_mod_status {
    lv_obj_t *obj;
    lv_obj_t *chips[ZMK_MOD_STATUS_CHIP_COUNT];
    bool chip_active[ZMK_MOD_STATUS_CHIP_COUNT];
    struct k_work_delayable update_work;
};

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget);
