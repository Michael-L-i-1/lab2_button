#include <stdint.h>
#include "timer.h"
#include "../part2-push-button/button.h"

static volatile uint32_t* rcc_cr = (volatile uint32_t*) 0x40021000;
static volatile uint32_t* rcc_cfgr = (volatile uint32_t*) 0x40021008;
static volatile uint32_t* rcc_ahb2 = (volatile uint32_t*) 0x4002104C;
static volatile uint32_t* rcc_apb1 = (volatile uint32_t*) 0x40021058;

static volatile uint32_t* gpio_b_mode = (volatile uint32_t*) 0x48000400;
static volatile uint32_t* gpio_b_output = (volatile uint32_t*) 0x48000414;
static volatile uint32_t* gpio_c_mode = (volatile uint32_t*) 0x48000800;
static volatile uint32_t* gpio_c_output = (volatile uint32_t*) 0x48000814;

static volatile uint32_t* tim4_cr1 = (volatile uint32_t*) 0x40000800;
static volatile uint32_t* tim4_dier = (volatile uint32_t*) 0x4000080C;
static volatile uint32_t* tim4_sr = (volatile uint32_t*) 0x40000810;
static volatile uint32_t* tim4_egr = (volatile uint32_t*) 0x40000814;
static volatile uint32_t* tim4_psc = (volatile uint32_t*) 0x40000828;
static volatile uint32_t* tim4_arr = (volatile uint32_t*) 0x4000082C;
static volatile uint32_t* nvic_enable0 = (volatile uint32_t*) 0xE000E100;

static uint32_t color = 0;

// like in Lab 1 we do configuration here
static void show_color(void)
{
    *gpio_c_output &= ~(1u << 7);
    *gpio_b_output &= ~((1u << 7) | (1u << 14));

    if (color == 0)
        *gpio_c_output |= (1u << 7);
    else if (color == 1)
        *gpio_b_output |= (1u << 7);
    else
        *gpio_b_output |= (1u << 14);
}

void run_part3(void)
{
    // use HSI16 at 16 MHz
    *rcc_cr |= (1u << 8);
    while ((*rcc_cr & (1u << 10)) == 0) {}
    *rcc_cfgr |= 1u;
    while ((*rcc_cfgr & (3u << 2)) != (1u << 2)) {}

    // enable GPIOB, GPIOC and TIM4
    *rcc_ahb2 |= (1u << 1) | (1u << 2);
    *rcc_apb1 |= (1u << 2);
    *gpio_c_mode &= ~(3u << 14);
    *gpio_c_mode |= (1u << 14);
    *gpio_b_mode &= ~((3u << 14) | (3u << 28));
    *gpio_b_mode |= (1u << 14) | (1u << 28);

    // start with green
    show_color();

    // 1 Hz
    *tim4_psc = 15999;
    *tim4_arr = 999;
    *tim4_egr = 1u;
    *tim4_sr = 0;
    *tim4_dier = 1u;
    *nvic_enable0 = (1u << 30);
    *tim4_cr1 = 1u;
}

void TIM4_IRQHandler(void)
{
    // clear the flag even while the button is held
    *tim4_sr = 0;
    if (!button_held)
    {
        color = (color + 1u) % 3u;
        show_color();
    }
}
