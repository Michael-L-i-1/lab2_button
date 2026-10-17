#include <stdint.h>
#include "button.h"

volatile uint32_t button_held = 0;
static uint32_t hold_mode = 0;

// clock registers
static volatile uint32_t* rcc_ahb2 = (volatile uint32_t*) 0x4002104C;
static volatile uint32_t* rcc_apb2 = (volatile uint32_t*) 0x40021060;

// blue LED (PB7) and button (PC13)
static volatile uint32_t* gpio_b_mode = (volatile uint32_t*) 0x48000400;
static volatile uint32_t* gpio_b_output = (volatile uint32_t*) 0x48000414;
static volatile uint32_t* gpio_c_mode = (volatile uint32_t*) 0x48000800;
static volatile uint32_t* gpio_c_input = (volatile uint32_t*) 0x48000810;

// route PC13 to EXTI13, and then enable its interrupt.
static volatile uint32_t* syscfg_exticr4 = (volatile uint32_t*) 0x40010014;
static volatile uint32_t* exti_mask = (volatile uint32_t*) 0x40010400;
static volatile uint32_t* exti_rising = (volatile uint32_t*) 0x40010408;
static volatile uint32_t* exti_falling = (volatile uint32_t*) 0x4001040C;
static volatile uint32_t* exti_pending = (volatile uint32_t*) 0x40010414;
static volatile uint32_t* nvic_enable1 = (volatile uint32_t*) 0xE000E104;

void run_part2(void)
{
    *rcc_ahb2 |= (1u << 1) | (1u << 2);  // GPIOB and GPIOC clocks
    *rcc_apb2 |= 1u; // SYSCFG clock

    // PB7 output, PC13 input
    *gpio_b_mode &= ~(3u << 14);
    *gpio_b_mode |= (1u << 14);
    *gpio_c_mode &= ~(3u << 26);

    // EXTI13 uses port C
    *syscfg_exticr4 |= (2u << 4);
    *exti_rising |= (1u << 13); // interrupt on press
    *exti_mask |= (1u << 13);
    *nvic_enable1 = (1u << 8);
}

void setup_button_hold(void)
{
    run_part2();
    hold_mode = 1;
    *exti_falling |= (1u << 13); // also interrupt on release
}

// The startup vector table calls this function automatically.
void EXTI15_10_IRQHandler(void)
{
    *exti_pending = (1u << 13);  // clear EXTI13
    if (hold_mode)
        button_held = (*gpio_c_input >> 13) & 1u;
    else
        *gpio_b_output ^= (1u << 7);
}
