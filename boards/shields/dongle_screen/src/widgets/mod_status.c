#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <zmk/hid_indicators.h>
#include <lvgl.h>

#include "mod_status.h"
#include "fonts.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define LED_NLCK 0x01
#define LED_CLCK 0x02
#define LED_SLCK 0x04

static void update_mod_status(struct zmk_widget_mod_status *widget)
{
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;

    // Temporäre Puffer für Symbole
    char *syms[5];
    int n = 0;

    zmk_hid_indicators_t indicators = zmk_hid_indicators_get_current_profile();

    if (indicators & LED_CLCK)
        syms[n++] = "󰘲";
    if (mods & (MOD_LCTL | MOD_RCTL))
        syms[n++] = "󰘴";
    if (mods & (MOD_LSFT | MOD_RSFT))
        syms[n++] = "󰘶";
    if (mods & (MOD_LALT | MOD_RALT))
        syms[n++] = "󰘵";
    if (mods & (MOD_LGUI | MOD_RGUI))
        syms[n++] = "󰘳";

    char text[32] = "";
    int idx = 0;
    for (int i = 0; i < n; ++i)
        idx += snprintf(&text[idx], sizeof(text) - idx, "%s ", syms[i]);

    lv_label_set_text(widget->label, idx ? text : "");
}

static void mod_status_timer_cb(struct k_timer *timer)
{
    struct zmk_widget_mod_status *widget = k_timer_user_data_get(timer);
    update_mod_status(widget);
}

static struct k_timer mod_status_timer;

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 180, 40);

    widget->label = lv_label_create(widget->obj);
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &nerd_fonts_big, 0);

    k_timer_init(&mod_status_timer, mod_status_timer_cb, NULL);
    k_timer_user_data_set(&mod_status_timer, widget);
    k_timer_start(&mod_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget)
{
    return widget->obj;
}
