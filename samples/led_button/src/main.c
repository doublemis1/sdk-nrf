/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <dk_buttons_and_leds.h>

/* Button press callback function */
static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	/* Map buttons to LEDs - check each button that changed */
	if (has_changed & DK_BTN1_MSK) {
		bool led_state = (button_state & DK_BTN1_MSK) ? 1 : 0;
		dk_set_led(DK_LED1, led_state);
	}

	if (has_changed & DK_BTN2_MSK) {
		bool led_state = (button_state & DK_BTN2_MSK) ? 1 : 0;
		dk_set_led(DK_LED2, led_state);
	}

	if (has_changed & DK_BTN3_MSK) {
		bool led_state = (button_state & DK_BTN3_MSK) ? 1 : 0;
		dk_set_led(DK_LED3, led_state);
	}

	if (has_changed & DK_BTN4_MSK) {
		bool led_state = (button_state & DK_BTN4_MSK) ? 1 : 0;
		dk_set_led(DK_LED4, led_state);
	}
}

int main(void)
{
	int ret;

	printk("LED Button Sample started\n");

	/* Initialize buttons */
	ret = dk_buttons_init(button_handler);
	if (ret) {
		printk("Cannot init buttons (error: %d)\n", ret);
		return 0;
	}

	/* Initialize LEDs */
	ret = dk_leds_init();
	if (ret) {
		printk("Cannot init leds (error: %d)\n", ret);
		return 0;
	}

	printk("Press buttons to control LEDs\n");

	/* Main loop - keep the application running */
	while (1) {
		k_sleep(K_MSEC(1000));
	}

	return 0;
} 