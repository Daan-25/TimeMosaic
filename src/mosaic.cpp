#include "mosaic.h"
#include <ctime>
#include <cmath>

namespace tmosaic {

static bool is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int days_in_month(int year, int month) {
    static const int dm[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && is_leap(year)) return 29;
    return dm[month];
}

// Zeller-like: returns 0=Mon..6=Sun for a Gregorian date
int day_of_week(const CalendarDate& d) {
    // Use Tomohiko Sakamoto's algorithm
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = d.year;
    if (d.month < 3) y--;
    int dow = (y + y/4 - y/100 + y/400 + t[d.month - 1] + d.day) % 7;
    // dow: 0=Sun,1=Mon,...6=Sat → convert to 0=Mon..6=Sun
    return (dow + 6) % 7;
}

CalendarDate next_day(const CalendarDate& d) {
    CalendarDate n = d;
    n.day++;
    if (n.day > days_in_month(n.year, n.month)) {
        n.day = 1;
        n.month++;
        if (n.month > 12) {
            n.month = 1;
            n.year++;
        }
    }
    return n;
}

int week_number(const CalendarDate& d) {
    // Simple: count days from Jan 1 of that year, divide by 7
    int yday = 0;
    for (int m = 1; m < d.month; m++) yday += days_in_month(d.year, m);
    yday += d.day - 1;
    // ISO week: week starts on Monday
    CalendarDate jan1{d.year, 1, 1};
    int jan1_dow = day_of_week(jan1); // 0=Mon
    return (yday + jan1_dow) / 7;
}

} // namespace tmosaic
