#define DT_DRV_COMPAT zmk_behavior_delayed_release

// Dependencies
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define DELAYED_RELEASE_PREREQUISITE_KEY 0
#define DELAYED_RELEASE_TRIGGER_KEY 1

#define DELAYED_RELEASE_MAX_CONFIGS 10

// #if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_delayed_release_config {
    struct zmk_behavior_binding prerequisite_behavior;
    struct zmk_behavior_binding delayed_release_behavior;
    struct zmk_behavior_binding trigger_behavior;
    int index;
};

struct behavior_delayed_release_data {
    bool trigger_key_pressed;
    bool prerequisite_key_pressed;
    bool delayed_release_behavior_active;
};

static int behavior_delayed_release_init(const struct device *dev) {
    return 0;
};

static int on_delayed_release_binding_pressed(struct zmk_behavior_binding *binding,
                                              struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_delayed_release_config *cfg = dev->config;
    struct behavior_delayed_release_data *state = dev->data;

    if (binding->param1 == DELAYED_RELEASE_PREREQUISITE_KEY) {
        // Prerequisite key pressed; trigger its behavior and record it in the state.
        zmk_behavior_invoke_binding(&cfg->prerequisite_behavior, event, true);
        state->prerequisite_key_pressed = true;
    } else if (binding->param1 == DELAYED_RELEASE_TRIGGER_KEY) {
        // If the delayed release behavior has not yet been triggered, and the prerequisite key is pressed,
        // trigger it now and record it in the state.
        if (!state->delayed_release_behavior_active && state->prerequisite_key_pressed) {
            zmk_behavior_invoke_binding(&cfg->delayed_release_behavior, event, true);
            state->delayed_release_behavior_active = true;
        }

        // Also invoke the primary behavior of the trigger key and record that it is pressed.
        zmk_behavior_invoke_binding(&cfg->trigger_behavior, event, true);
        state->trigger_key_pressed = true;
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_delayed_release_binding_released(struct zmk_behavior_binding *binding,
                                               struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_delayed_release_config *cfg = dev->config;
    struct behavior_delayed_release_data *state = dev->data;

    if (binding->param1 == DELAYED_RELEASE_PREREQUISITE_KEY) {
        // Pre-requisite key released.
        // If the delayed release behavior was active, deactivate it first.
        // However, if the trigger key is still pressed, we'll wait until the trigger key is released
        // to release the delayed release behavior. (See below)
        if (state->delayed_release_behavior_active && !state->trigger_key_pressed) {
            zmk_behavior_invoke_binding(&cfg->delayed_release_behavior, event, false);
            state->delayed_release_behavior_active = false;
        }

        zmk_behavior_invoke_binding(&cfg->prerequisite_behavior, event, false);
        state->prerequisite_key_pressed = false;
    } else if (binding->param1 == DELAYED_RELEASE_TRIGGER_KEY) {
        // Trigger key released.
        zmk_behavior_invoke_binding(&cfg->trigger_behavior, event, false);
        state->trigger_key_pressed = false;

        if (!state->prerequisite_key_pressed && state->delayed_release_behavior_active) {
            // (From above) in this case, the prerequisite key was released, but the trigger
            // key is still pressed. Now we can release the delayed release behavior.
            zmk_behavior_invoke_binding(&cfg->delayed_release_behavior, event, false);
            state->delayed_release_behavior_active = false;
        }
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

// API struct
static const struct behavior_driver_api behavior_delayed_release_driver_api = {
    .binding_pressed = on_delayed_release_binding_pressed,
    .binding_released = on_delayed_release_binding_released,
};

#define TRANSFORM_ENTRY(idx, node)                                                             \
{                                                                                              \
    .behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(node, bindings, idx)),               \
    .param1 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(node, bindings, idx, param1), (0),       \
    (DT_INST_PHA_BY_IDX(node, bindings, idx, param1))),                                        \
    .param2 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(node, bindings, idx, param2), (0),       \
    (DT_INST_PHA_BY_IDX(node, bindings, idx, param2))),                                        \
}

#define BEHAVIOR_DELAYED_RELEASE_INST(n)                                                    \
    static struct behavior_delayed_release_config behavior_delayed_release_config_##n = {   \
        .prerequisite_behavior = TRANSFORM_ENTRY(0, n),                                     \
        .delayed_release_behavior = TRANSFORM_ENTRY(1, n),                                  \
        .trigger_behavior = TRANSFORM_ENTRY(2, n),                                          \
        .index = n,                                                                         \
    };                                                                                      \
    static struct behavior_delayed_release_data behavior_delayed_release_data_##n = {       \
        .trigger_key_pressed = false,                                                       \
        .prerequisite_key_pressed = false,                                                  \
        .delayed_release_behavior_active = false,                                           \
    };                                                                                      \
    BEHAVIOR_DT_INST_DEFINE(n,                                                              \
                            behavior_delayed_release_init,                                  \
                            NULL,                                                           \
                            &behavior_delayed_release_data_##n,                             \
                            &behavior_delayed_release_config_##n,                           \
                            POST_KERNEL,                                                    \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                            \
                            &behavior_delayed_release_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BEHAVIOR_DELAYED_RELEASE_INST)

// #endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
