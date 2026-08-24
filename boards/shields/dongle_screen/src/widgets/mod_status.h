#pragma once

#include <lvgl.h>
#include <zmk/display.h>

#define ZMK_MOD_STATUS_CHIP_COUNT 5

struct zmk_widget_mod_status {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *chips[ZMK_MOD_STATUS_CHIP_COUNT];
};

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget);
