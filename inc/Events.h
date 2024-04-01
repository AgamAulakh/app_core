#pragma once
#include <zephyr/smf.h>

#define EVENT_SIG_PROC_COMPLETE BIT(1)

struct s_object {
    struct smf_ctx ctx;

    /* Events */
    struct k_event button_press_event;
    struct k_event sig_proc_complete;
    int32_t events;
    
} s_obj;

extern struct s_object s_obj;
