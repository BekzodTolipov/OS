#include "clock.h"
#include <stdio.h>

#define ONE_BILLION 1000000000

void incr_clock(struct Clock* Clock, int elapsed_sec) {
    Clock->ns += elapsed_sec;
    if (Clock->ns >= ONE_BILLION) {
        Clock->sec += 1;
        Clock->ns -= ONE_BILLION;
    }
}

struct Clock add_clocks(struct Clock clock_1, struct Clock clock_2) {
    struct Clock out = {
        .sec = 0,
        .ns = 0
    };
    out.sec = clock_1.sec + clock_2.sec;
    incr_clock(&out, clock_1.ns + clock_2.ns);
    return out;
}

int compare_clocks(struct Clock clock_1, struct Clock clock_2) {
    if (clock_1.sec > clock_2.sec) {
        return 1;
    }
    if ((clock_1.sec == clock_2.sec) && (clock_1.ns > clock_2.ns)) {
        return 1;
    }
    if ((clock_1.sec == clock_2.sec) && (clock_1.ns == clock_2.ns)) {
        return 0;
    }
    return -1;
}

long double Clock_to_sec(struct Clock c) {
    long double sec = c.sec;
    long double ns = (long double)c.ns / ONE_BILLION; 
    sec += ns;
    return sec;
}

struct Clock sec_to_Clock(long double sec) {
    struct Clock clk = { .sec = (int)sec };
    sec -= clk.sec;
    clk.ns = sec * ONE_BILLION;
    return clk;
}

struct Clock calculate_avg_time(struct Clock clk, int divisor) {
    long double sec = Clock_to_sec(clk);
    long double avg_sec = sec / divisor;
    return sec_to_Clock(avg_sec);
}

struct Clock subtract_Clocks(struct Clock clock_1, struct Clock clock_2) {
    long double sec1 = Clock_to_sec(clock_1);
    long double sec2 = Clock_to_sec(clock_2);
    long double result = sec1 - sec2;
    return sec_to_Clock(result);
}

struct Clock ns_to_Clock(int ns) {
    struct Clock clk = { 
        .sec = 0, 
        .ns = 0 
    };

    if (ns >= ONE_BILLION) {
        ns -= ONE_BILLION;
        clk.sec = 1;
    }

    clk.ns = ns;
    
    return clk;
}

struct Clock get_clock() {
    struct Clock out = {
        .sec = 0,
        .ns = 0
    };
    return out;
}

void set_clock(struct Clock* clk) {
    clk->sec = 0;
    clk->ns = 0;
}
