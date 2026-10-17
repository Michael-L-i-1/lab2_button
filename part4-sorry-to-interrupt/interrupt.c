#include "interrupt.h"
#include "../part2-push-button/button.h"
#include "../part3-timer/timer.h"

void run_part4(void)
{
    // watch both button edges, then start the timer
    setup_button_hold();
    run_part3();
}
