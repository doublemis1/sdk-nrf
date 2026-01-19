/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief RF Switch control sample for MikroE Click RF Switch
 *
 * This sample controls the MikroE Click RF Switch board using:
 * - GPIO (P1.1) for ON pin: Controls power to the RF switch
 * - PWM (P1.2) for SEL pin: Selects RF channel (0% duty = RF1, 100% duty = RF2)
 *
 * Switching Mechanism:
 * When changing between open (RF1) and close (RF2) states, the following
 * sequence is executed to ensure clean transitions:
 * 1. Turn OFF the switch (ON pin → LOW)
 * 2. Change channel (SEL pin PWM duty cycle)
 * 3. Turn ON the switch (ON pin → HIGH)
 *
 * Default State:
 * - Power: ON (ON pin HIGH)
 * - Channel: RF1 (SEL pin PWM 0% duty cycle)
 * - State: OPEN (RF1 ↔ RFC)
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(rf_switch, CONFIG_LOG_DEFAULT_LEVEL);

/* GPIO pin definitions for nRF52840 DK */
/* Using GPIO1 pins from Arduino header to avoid conflicts with buttons */
#define RF_SWITCH_ON_PIN  1   /* P1.1 - ON pin (Arduino D0) */
#define RF_SWITCH_SEL_PIN 2   /* P1.2 - SEL pin (Arduino D1) */

/* GPIO devices */
#define GPIO1_NODE DT_NODELABEL(gpio1)
static const struct device *gpio1_dev = DEVICE_DT_GET(GPIO1_NODE);

/* PWM device for SEL pin */
#define PWM0_NODE DT_NODELABEL(pwm0)
static const struct device *pwm0_dev = DEVICE_DT_GET(PWM0_NODE);
#define PWM_SEL_CHANNEL 1  /* Use channel 1, channel 0 is used by LED */
#define PWM_PERIOD_USEC 1000  /* 1ms period = 1kHz PWM frequency */

/* RF Switch state */
static bool rf_switch_enabled = false;
static bool rf_switch_channel = false; /* false = RF1, true = RF2 */

/* Forward declarations */
static int rf_switch_select_channel(bool channel);
static int rf_switch_set_power(bool enable);

/**
 * @brief Initialize GPIO and PWM for RF Switch control
 *
 * Configures:
 * - ON pin (P1.1) as GPIO output, initially HIGH (power ON)
 * - SEL pin (P1.2) as PWM output, initially 0% duty cycle (RF1)
 *
 * Default state: Power ON, Channel RF1 (OPEN state)
 *
 * @return 0 on success, negative error code on failure
 */
static int rf_switch_init(void)
{
	int ret;

	if (!device_is_ready(gpio1_dev)) {
		LOG_ERR("GPIO1 device not ready");
		return -ENODEV;
	}

	if (!device_is_ready(pwm0_dev)) {
		LOG_ERR("PWM0 device not ready");
		return -ENODEV;
	}

	/* Configure ON pin as output, initially HIGH */
	ret = gpio_pin_configure(gpio1_dev, RF_SWITCH_ON_PIN,
				 GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH);
	if (ret < 0) {
		LOG_ERR("Failed to configure ON pin: %d", ret);
		return ret;
	}

	/* Initialize SEL pin with PWM - start with 0% duty cycle (RF1/LOW) */
	ret = pwm_set_cycles(pwm0_dev, PWM_SEL_CHANNEL,
			     PWM_USEC(PWM_PERIOD_USEC), 0, 0);
	if (ret < 0) {
		LOG_ERR("Failed to configure SEL pin PWM: %d", ret);
		return ret;
	}

	/* Set software state to match hardware (enabled, RF1) */
	rf_switch_enabled = true;
	rf_switch_channel = false; /* RF1 */

	LOG_INF("RF Switch initialized (ON: P1.%d GPIO HIGH, SEL: P1.%d PWM)",
		RF_SWITCH_ON_PIN, RF_SWITCH_SEL_PIN);
	LOG_INF("RF Switch enabled by default (RF1 ↔ RFC)");

	return 0;
}

/**
 * @brief Set RF switch power state
 *
 * @param enable true to enable (ON pin HIGH), false to disable (ON pin LOW)
 * @return 0 on success, negative error code on failure
 */
static int rf_switch_set_power(bool enable)
{
	int ret;
	int pin_value = enable ? 1 : 0;

	/* Set ON pin to enable/disable the switch */
	ret = gpio_pin_set_raw(gpio1_dev, RF_SWITCH_ON_PIN, pin_value);
	if (ret < 0) {
		LOG_ERR("Failed to set ON pin: %d", ret);
		return ret;
	}

	/* Delay to allow the switch to respond */
	k_sleep(K_MSEC(50));

	rf_switch_enabled = enable;
	LOG_INF("RF Switch power %s", enable ? "enabled" : "disabled");

	return 0;
}

/**
 * @brief Select RF channel (RF1 or RF2) via PWM
 *
 * Sets the SEL pin PWM duty cycle:
 * - RF1: 0% duty cycle (LOW equivalent)
 * - RF2: 100% duty cycle (HIGH equivalent)
 *
 * @param channel false for RF1, true for RF2
 * @return 0 on success, negative error code on failure
 */
static int rf_switch_select_channel(bool channel)
{
	int ret;
	uint32_t pulse_usec;

	/* Set PWM duty cycle: 0% for RF1 (LOW), 100% for RF2 (HIGH) */
	if (channel) {
		/* RF2: 100% duty cycle (HIGH) */
		pulse_usec = PWM_PERIOD_USEC;
	} else {
		/* RF1: 0% duty cycle (LOW) */
		pulse_usec = 0;
	}

	ret = pwm_set_cycles(pwm0_dev, PWM_SEL_CHANNEL,
			     PWM_USEC(PWM_PERIOD_USEC), PWM_USEC(pulse_usec), 0);
	if (ret < 0) {
		LOG_ERR("Failed to set SEL pin PWM: %d", ret);
		return ret;
	}

	/* Delay to allow the switch and LED to update */
	k_sleep(K_MSEC(50));

	rf_switch_channel = channel;
	LOG_INF("RF Switch channel changed to: %s (RF connection: %s ↔ RFC)",
		channel ? "RF2" : "RF1", channel ? "RF2" : "RF1");

	return 0;
}

/* CLI Commands */

/**
 * @brief CLI command: Open switch (connect RF1 to RFC)
 *
 * Executes switching sequence: OFF → RF1 → ON
 */
static int cmd_rf_switch_open(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int ret;

	shell_print(sh, "Opening switch: RF1 ↔ RFC");

	/* Sequence: OFF → change to RF1 → ON */
	ret = rf_switch_set_power(false);
	if (ret < 0) {
		shell_error(sh, "Failed to turn OFF switch: %d", ret);
		return ret;
	}

	ret = rf_switch_select_channel(false); /* RF1 - SEL pin LOW/0% PWM */
	if (ret < 0) {
		shell_error(sh, "Failed to set RF1 channel: %d", ret);
		return ret;
	}

	ret = rf_switch_set_power(true);
	if (ret < 0) {
		shell_error(sh, "Failed to turn ON switch: %d", ret);
		return ret;
	}

	shell_print(sh, "RF Switch opened (RF1 ↔ RFC) - RF1 LED should be ON");
	return 0;
}

/**
 * @brief CLI command: Close switch (connect RF2 to RFC)
 *
 * Executes switching sequence: OFF → RF2 → ON
 */
static int cmd_rf_switch_close(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int ret;

	shell_print(sh, "Closing switch: RF2 ↔ RFC");

	/* Sequence: OFF → change to RF2 → ON */
	ret = rf_switch_set_power(false);
	if (ret < 0) {
		shell_error(sh, "Failed to turn OFF switch: %d", ret);
		return ret;
	}

	ret = rf_switch_select_channel(true); /* RF2 - SEL pin HIGH/100% PWM */
	if (ret < 0) {
		shell_error(sh, "Failed to set RF2 channel: %d", ret);
		return ret;
	}

	ret = rf_switch_set_power(true);
	if (ret < 0) {
		shell_error(sh, "Failed to turn ON switch: %d", ret);
		return ret;
	}

	shell_print(sh, "RF Switch closed (RF2 ↔ RFC) - RF2 LED should be ON");
	return 0;
}

/**
 * @brief CLI command: Get RF Switch status
 */
static int cmd_rf_switch_status(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int on_pin_state;
	const char *connection_state = rf_switch_channel ? "RF2 ↔ RFC" : "RF1 ↔ RFC";

	/* Read ON pin state */
	on_pin_state = gpio_pin_get_raw(gpio1_dev, RF_SWITCH_ON_PIN);

	shell_print(sh, "RF Switch Status:");
	shell_print(sh, "  Power: %s", rf_switch_enabled ? "ON" : "OFF");
	shell_print(sh, "  Channel: %s", rf_switch_channel ? "RF2" : "RF1");
	shell_print(sh, "  Connection: %s", connection_state);
	shell_print(sh, "  ON Pin (P1.%d / Arduino D0): %s",
		    RF_SWITCH_ON_PIN, on_pin_state > 0 ? "HIGH" : "LOW");
	shell_print(sh, "  SEL Pin (P1.%d / Arduino D1): PWM - %s",
		    RF_SWITCH_SEL_PIN,
		    rf_switch_channel ? "RF2 selected (100% duty)" : "RF1 selected (0% duty)");

	return 0;
}

/* Shell command definitions */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_rf_switch,
	SHELL_CMD(open, NULL, "Open switch: connect RF1 to RFC", cmd_rf_switch_open),
	SHELL_CMD(close, NULL, "Close switch: connect RF2 to RFC", cmd_rf_switch_close),
	SHELL_CMD(status, NULL, "Get RF Switch status", cmd_rf_switch_status),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(rf_switch, &sub_rf_switch, "RF Switch control commands", NULL);

/**
 * @brief Main function
 */
int main(void)
{
	int ret;

	LOG_INF("RF Switch Sample for nRF52840");
	LOG_INF("MikroE Click RF Switch Control");

	ret = rf_switch_init();
	if (ret < 0) {
		LOG_ERR("Failed to initialize RF Switch: %d", ret);
		return ret;
	}

	LOG_INF("RF Switch initialized successfully");
	LOG_INF("Default state: Power ON, Channel RF1 (RF1 ↔ RFC) - OPEN");
	LOG_INF("Use 'rf_switch' commands to control the switch:");
	LOG_INF("  rf_switch open    - Connect RF1 to RFC (OFF → RF1 → ON)");
	LOG_INF("  rf_switch close   - Connect RF2 to RFC (OFF → RF2 → ON)");
	LOG_INF("  rf_switch status  - Get status");

	/* Keep the main thread alive so shell can process commands */
	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
