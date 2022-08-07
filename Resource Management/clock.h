#ifndef Clock_H
#define Clock_H

struct Clock {
    unsigned long sec;
    unsigned long ns;
};

void incr_clock(struct Clock* Clock, int elapsed_sec);
struct Clock add_clocks(struct Clock c1, struct Clock c2);
int compare_clocks(struct Clock c1, struct Clock c2);
long double Clock_to_sec(struct Clock c);
struct Clock sec_to_Clock(long double sec);
struct Clock calculate_avg_time(struct Clock clk, int divisor);
struct Clock subtract_Clocks(struct Clock c1, struct Clock c2);
struct Clock ns_to_Clock(int ns);
struct Clock get_clock();
void set_clock(struct Clock* clk);

#endif
