LED Button Sample
================

Overview
--------

This sample demonstrates how to control LEDs using buttons on Nordic development kits.
When a button is pressed, the corresponding LED turns on. When the button is released,
the LED turns off.

Supported Boards
---------------

This sample supports the following Nordic development kits:

* nRF52840 DK (nrf52840dk/nrf52840)
* nRF5340 DK (nrf5340dk/nrf5340/cpuapp)
* nRF54L15 DK (nrf54l15dk/nrf54l15)
* nRF54L20 PDK (nrf54l20pdk/nrf54l20)

Button to LED Mapping
---------------------

The sample maps buttons to LEDs as follows:

* Button 0 (sw0) → LED 0 (led0)
* Button 1 (sw1) → LED 1 (led1)
* Button 2 (sw2) → LED 2 (led2)
* Button 3 (sw3) → LED 3 (led3)

Hardware Requirements
--------------------

The sample requires boards with:
* GPIO buttons (configured as gpio-keys)
* GPIO LEDs (configured as gpio-leds)

Building and Running
-------------------

Build the sample for a specific board:

.. code-block:: console

   west build -b <board> nrf/samples/led_button

For example, to build for nRF52840 DK:

.. code-block:: console

   west build -b nrf52840dk_nrf52840 nrf/samples/led_button

Flash the sample:

.. code-block:: console

   west flash

Expected Behavior
----------------

1. The sample initializes all available buttons and LEDs
2. When you press a button, the corresponding LED turns on
3. When you release the button, the LED turns off
4. The sample supports up to 4 button-LED pairs (button0-led0, button1-led1, etc.)

Note: The sample uses the DK library which provides reliable button and LED handling across all Nordic devices.

Implementation Details
---------------------

The sample uses:
* DK library for reliable button and LED handling across all Nordic devices
* Button state detection with proper debouncing
* LED control with automatic GPIO configuration
* Cross-platform compatibility for all supported Nordic boards
* Error handling for missing or misconfigured devices 