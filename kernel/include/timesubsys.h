#ifndef TIMESUBSYS_H
#define TIMESUBSYS_H

// subsystem identifier structure
typedef struct {
    const char *tag; // e.g., "serio", "rtc_cmos", "clocksource"
} ksubsys_t;

// pre-declared common kernel subsystems
#define DEFINE_SUBSYS(name, tag_str) static const ksubsys_t name = { tag_str }

DEFINE_SUBSYS(LOG_SERIO, "serio");
DEFINE_SUBSYS(LOG_INPUT, "input");
DEFINE_SUBSYS(LOG_RTC,   "rtc_cmos");
DEFINE_SUBSYS(LOG_CLOCK, "clocksource");
DEFINE_SUBSYS(LOG_NET,   "NET");
DEFINE_SUBSYS(LOG_DISK,  "disk");

#endif // TIMESUBSYS_H