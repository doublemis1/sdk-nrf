.. _rf_switch_sample:

RF Switch Sample
################

This sample demonstrates how to control the MikroE Click RF Switch board using GPIO and PWM on the nRF52840 DK.

Overview
********

The RF Switch Click is a mikroBUS™ Click board built around the MASWSS0115 IC from Macom. It is a GaAs PHEMT MMIC SPDT (single-pole double-throw) switch that can route small-signal RF paths up to 3 GHz.

Hardware Requirements
*********************

* nRF52840 DK
* MikroE Click RF Switch board

Hardware Connections
********************

Connect the RF Switch module to the nRF52840 DK as follows:

* RF Switch 5V → nRF52840 DK 5V
* RF Switch GND → nRF52840 DK GND
* RF Switch ON pin → nRF52840 DK P1.1 (Arduino D0)
* RF Switch SEL pin → nRF52840 DK P1.2 (Arduino D1)

Pin Configuration
*****************

* **P1.1** (ON pin, Arduino D0, GPIO): Controls power to the RF switch. High = enabled, Low = disabled.
* **P1.2** (SEL pin, Arduino D1, PWM): Selects RF channel via PWM. 0% duty cycle = RF1, 100% duty cycle = RF2.

**Note:** These pins are on the Arduino header (GPIO1) to avoid conflicts with onboard buttons. UART1 is disabled to free these pins for GPIO/PWM use.

Default State
*************

Upon boot, the RF Switch is in the following default state:

* **Power**: ON (switch is enabled by default)
* **Channel**: RF1 (RF1 ↔ RFC connection)
* **State**: OPEN

The ON pin (P1.1) is set to HIGH by default, enabling the RF switch immediately upon boot. The SEL pin (P1.2) is configured with PWM at 0% duty cycle, selecting RF1. The switch defaults to **RF1 ↔ RFC** connection (open state), meaning RF1 is connected to the common port (RFC).

Connection States
*****************

The RF Switch provides two connection states:

* **Open** (RF1 ↔ RFC): RF1 port is connected to the common port (RFC). This is the default state when the switch is enabled. SEL pin PWM duty cycle: 0%.
* **Close** (RF2 ↔ RFC): RF2 port is connected to the common port (RFC). SEL pin PWM duty cycle: 100%.

Switching Mechanism
********************

When switching between open and close states, the following sequence is automatically executed to ensure clean transitions:

1. **Turn OFF** the switch (ON pin → LOW)
2. **Change channel** (SEL pin PWM duty cycle: 0% for RF1, 100% for RF2)
3. **Turn ON** the switch (ON pin → HIGH)

This sequence prevents glitches and ensures the switch is always in a known state during channel changes.

Building and Running
********************

Build the sample for the nRF52840 DK:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 nrf/samples/peripheral/rf_switch

Flash the application:

.. code-block:: console

   west flash

Testing
*******

After flashing, connect to the serial console (115200 baud) and use the following CLI commands:

* ``rf_switch open`` - Open switch: connect RF1 to RFC (RF1 ↔ RFC). Executes: OFF → RF1 → ON
* ``rf_switch close`` - Close switch: connect RF2 to RFC (RF2 ↔ RFC). Executes: OFF → RF2 → ON
* ``rf_switch status`` - Display current RF Switch status

Example Usage
*************

.. code-block:: console

   uart:~$ rf_switch status
   RF Switch Status:
     Power: ON
     Channel: RF1
     Connection: RF1 ↔ RFC
     ON Pin (P1.1 / Arduino D0): HIGH
     SEL Pin (P1.2 / Arduino D1): PWM - RF1 selected (0% duty)
   uart:~$ rf_switch close
   Closing switch: RF2 ↔ RFC
   RF Switch closed (RF2 ↔ RFC) - RF2 LED should be ON
   uart:~$ rf_switch status
   RF Switch Status:
     Power: ON
     Channel: RF2
     Connection: RF2 ↔ RFC
     ON Pin (P1.1 / Arduino D0): HIGH
     SEL Pin (P1.2 / Arduino D1): PWM - RF2 selected (100% duty)
   uart:~$ rf_switch open
   Opening switch: RF1 ↔ RFC
   RF Switch opened (RF1 ↔ RFC) - RF1 LED should be ON

Additional Information
**********************

For more information about the MikroE Click RF Switch, visit:
https://www.mikroe.com/rf-switch-click
