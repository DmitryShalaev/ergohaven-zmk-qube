#include <nice_view_hid/hid.h>

static int on_layout_changed(const zmk_event_t* eh) {
  struct layout_notification* notification = as_layout_notification(eh);

  if (notification) {
    if (notification->value == 0) {
      zmk_keymap_layer_activate(CONFIG_QUBE_HID_EN_LAYER);
      zmk_keymap_layer_deactivate(CONFIG_QUBE_HID_RU_LAYER);
    } else if (notification->value == 1) {
      zmk_keymap_layer_activate(CONFIG_QUBE_HID_RU_LAYER);
      zmk_keymap_layer_deactivate(CONFIG_QUBE_HID_EN_LAYER);
    }
  }

  return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layout, on_layout_changed);
ZMK_SUBSCRIPTION(layout, layout_notification);
