#define DT_DRV_COMPAT zmk_behavior_delayed_release

// Dependencies
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/behavior.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

// Instance-specific Data struct (Optional)
struct delayed_release_data {
    bool data_param1;
    bool data_param2;
    bool data_param3;
};

// Initialization Function (Optional)
static int delayed_release_init(const struct device *dev) {
    return 0;
};

static int on_delayed_release_binding_pressed(struct zmk_behavior_binding *binding,
                                                 struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_delayed_release_binding_released(struct zmk_behavior_binding *binding,
                                                  struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

// API struct
static const struct behavior_driver_api delayed_release_driver_api = {
    .binding_pressed = on_delayed_release_binding_pressed,
    .binding_released = on_delayed_release_binding_pressed,
};

#define BEHAVIOR_DELAYED_RELEASE_INST(n)                                      \
    static struct behavior_delayed_release_config_##n {                       \
        .config_param1 = bar1;                                                \
    };                                                                        \
                                                                              \
    BEHAVIOR_DT_INST_DEFINE(n,                                                \ // Instance Number (Automatically populated by macro)
                            delayed_release_init,                             \ // Initialization Function
                            NULL,                                             \ // Power Management Device Pointer
                            NULL,                                             \ // Behavior Data Pointer
                            &behavior_delayed_release_config_##n,             \ // Behavior Configuration Pointer
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT  \ // Initialization Level, Device Priority
                            &delayed_release_driver_api);                       // API struct

DT_INST_FOREACH_STATUS_OKAY(BEHAVIOR_DELAYED_RELEASE_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
