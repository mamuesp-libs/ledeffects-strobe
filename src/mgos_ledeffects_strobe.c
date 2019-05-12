#include "mgos.h"
#include "led_master.h"

typedef struct {
    int count;
    int pause;
    tools_rgb_data color;
    tools_rgb_data out_pix;
} strobe_data;

static strobe_data sd;

static void mgos_intern_strobe_init(mgos_rgbleds* leds) {
    leds->timeout = mgos_sys_config_get_ledeffects_strobe_timeout();
    leds->dim_all = mgos_sys_config_get_ledeffects_strobe_dim_all();

    sd.count = mgos_sys_config_get_ledeffects_strobe_loops();
    sd.pause = mgos_sys_config_get_ledeffects_strobe_pause();
    sd.color = (tools_rgb_data)tools_get_random_color_fade(sd.color, &sd.color, 1, 30.0, 1.0, mgos_sys_config_get_ledeffects_strobe_value());
    sd.out_pix = mgos_rgbleds_lookup_gamma(leds, sd.color);
}

static void mgos_intern_strobe_loop(mgos_rgbleds* leds)
{
    static bool is_on = false;
    static int loops = 0;
    static int pause = 0;

    // reset the loop
    if (pause <= 0) {
        mgos_intern_strobe_init(leds);
        loops = sd.count;
        pause = sd.pause;
    }

    if (loops-- > 0) {
        LOG(LL_VERBOSE_DEBUG, ("Show LEDs in strobe ..."));
        if (is_on) {
            mgos_rgbleds_clear(leds);
            is_on = false;
        } else {
        
            mgos_rgbleds_set_all(leds, sd.out_pix);
            is_on = true;
        }
        mgos_rgbleds_show(leds);
        mgos_wdt_feed();
    } else if (pause-- > 0) {
        mgos_rgbleds_clear(leds);
        mgos_rgbleds_show(leds);
        mgos_wdt_feed();
    }
}

void mgos_ledeffects_strobe(void* param, mgos_rgbleds_action action)
{
    static bool do_time = false;
    static uint32_t max_time = 0;
    uint32_t time = (mgos_uptime_micros() / 1000);
    mgos_rgbleds* leds = (mgos_rgbleds*)param;

    switch (action) {
    case MGOS_RGBLEDS_ACT_INIT:
        LOG(LL_INFO, ("mgos_ledeffects_strobe: called (init)"));
        mgos_intern_strobe_init(leds);
        break;
    case MGOS_RGBLEDS_ACT_EXIT:
        LOG(LL_INFO, ("mgos_ledeffects_strobe: called (exit)"));
        break;
    case MGOS_RGBLEDS_ACT_LOOP:
        LOG(LL_VERBOSE_DEBUG, ("mgos_ledeffects_strobe: called (loop)"));
        mgos_intern_strobe_loop(leds);
        if (do_time) {
            time = (mgos_uptime_micros() /1000) - time;
            max_time = (time > max_time) ? time : max_time;
            LOG(LL_VERBOSE_DEBUG, ("VU meter loop duration: %d milliseconds, max: %d ...", time / 1000, max_time / 1000));
        }
        break;
    }
}

bool mgos_strobe_init(void) {
  LOG(LL_INFO, ("mgos_strobe_init ..."));
  ledmaster_add_effect("ANIM_STROBE", mgos_ledeffects_strobe);
  return true;
}
