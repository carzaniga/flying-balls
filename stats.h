#ifndef STATS_H_INCLUDED
#define STATS_H_INCLUDED

struct timing_stats {
    guint64 usec;
    unsigned int samples;
    unsigned int target_samples;
};

static inline float stats_avg_usec (const timing_stats * s) {
    return 1.0 * s->usec / s->samples;
}

static inline

#define STATS_TAKE_SAMPLE(s,f) \
    if ((s)->target_samples > 0) { \
	guint64 start = g_get_monotonic_time (); \
	do { f; } while 0;
	(s)->usec += g_get_monotonic_time () - start;
	++(s)->samples;
    } else { do { f; } while 0; }

#endif
