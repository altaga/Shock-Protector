# Shock Protector
 System that is capable of monitoring the medical instruments or areas of contact with the patient.

<img src="./Images/back.png" width="1000">

# Table of contents

- [Shock Protector](#shock-protector)
- [Table of contents](#table-of-contents)
- [Introduction:](#introduction)
- [Solution:](#solution)
- [Materials:](#materials)
- [Connection Diagram:](#connection-diagram)
- [nRF5340 Setup:](#nrf5340-setup)
    - [**Ble**:](#ble)
    - [**LCD**:](#lcd)
    - [**Buttons**:](#buttons)
- [RPi Gateway Setup:](#rpi-gateway-setup)
- [RPi Zero Power Profiler Device Setup:](#rpi-zero-power-profiler-device-setup)
    - [**Setup MQTT API**:](#setup-mqtt-api)
    - [**Setup PPK Python API**:](#setup-ppk-python-api)
- [Test Metodolody:](#test-metodolody)
- [Laptop Test:](#laptop-test)
- [Bt Test:](#bt-test)
- [Full Test:](#full-test)
- [The Final Product:](#the-final-product)
    - [Epic DEMO:](#epic-demo)
- [References:](#references)

# Introduction:

Nowadays, all electrical devices that we use, be it your cell phone charger to a washing machine, require to be connected to a 120-240 volt socket at 50-60 Hz. Even in the surgery rooms of hospitals the same thing happens. In the same way, every medical device is connected to a wall socket. However we have a very serious problem, if one of the devices is damaged and some of the current is transmitted to the human body it can be dangerous and even lethal .

This current that can reach the human body is known as a leakage current and depending on its intensity, it can injure or even kill the patient [1].

The most common case of these accidents is known as macro shock, which occurs when a current of more than 10mA passes through the human body.

<img src="https://i.ibb.co/m8H9RcZ/image.png" width="1000">

Less common than the previous one, is a leakage current known as micro-shock. This occurs when a leakage current of at least 10uA reaches a person through a catheter to the heart.

<img src="https://i.ibb.co/6PTfw2g/image.png" width="1000">

I want to solve the problem of early detection of these leakage currents before they can cause harm to patients.

[1]. https://www.sciencedirect.com/topics/biochemistry-genetics-and-molecular-biology/macroshock

# Solution:

We will build a system that is capable of monitoring the medical instruments or areas of contact with the patient to prevent the patient from receiving one of these shocks and, in the case of detecting these leakage currents, activate an alarm so that there is proper maintenance for the equipment or they can cancel their use.

Current Solutions:

Isolated Power System - Schneider:
https://www.se.com/ca/en/product-range-presentation/7367-isolated-power-system/
- It is to complex and expensive
- Has to be built inside the hospital’s infrastructure at the time of its construction
- Requires constant maintenance

Double insulation cables:
- These types of cables should be used in all medical equipment.
- You must follow the regulations not to use this cable for lengths greater than 36 meters.
- The cable must be placed in the entire electrical installation of the hospital from its construction.

The system that I am going to make is going to be plug and play, since it can be placed in any device that is going to be used with the patient before they come into contact with it in order to maintain the 10uA maximum current regulation leakage.

# Materials:

Hardware:
- nRF5340 DK                            x1.
https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF5340-DK
- 1602 LCD Keypad Shield For Arduino.   x1.
https://www.amazon.com/dp/B006D903KE/ref=cm_sw_em_r_mt_dp_CTDF7KGCKPRN9QMDETXT?_encoding=UTF8&psc=1
- Power Profiler Kit II.                x1.
https://www.nordicsemi.com/Software-and-tools/Development-Tools/Power-Profiler-Kit-2
- Rpi Zero W.                           x1.
https://www.amazon.com/dp/B07BHMRTTY/ref=cm_sw_em_r_mt_dp_9NRSPWM9RA6SGEH4BBS0
- 4x4 WS2812B NeoPixel                  x1.
https://www.aliexpress.com/i/32901162313.html
- Rpi 4.                                x1.
https://www.raspberrypi.org/products/raspberry-pi-4-model-b/

Software:
- nRF Connect:
https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Connect-for-desktop
- Python:
https://www.python.org/
- Segger Embedded Studio:
https://www.segger.com/products/development-tools/embedded-studio/
- nRF Cloud:
https://nrfcloud.com/
- OmniROM:
https://omnirom.org/

# Connection Diagram:

  **NOTE: The solution was made using the nRF5340 as a BT control to completely isolate the user from the system leakage current measurements since the measurements are so precise that the same static electricity from the body could affect the measurements.**

This is the connection diagram of the system:

<img src="./Images/system.png" width="1000">

# nRF5340 Setup:

All the code will be in the folder "nRF Software"

To program the nRF5340,  install Segger Embedded Studio from nRF Connect -> Toolchain Manager -> Segger Embedded Studio v1.5.1

<img src="./Images/studio.png" width="1000">

In the case of CORE NETWORK, the example application "hci_rpmsg" was used without making any changes to it.

<img src="./Images/hci.png" width="1000">

In CORE APPLICATION an application was made with the following characteristics.

- Modules utilized:
  - BLE.
  - LCD 16x2 Screen (4bits mode).
  - HW Buttons.

### **BLE**:

For the use of BLE, the "peripheral_hr" project was taken as an example, as it is very easy to send data to nRF Cloud and to send our commands.

<img src="./Images/btcloud.gif" width="1000">

In this case the command comes through the Heart Rate Measurement as a notification. You can see how it changes from 0 to 4 indicating which command is required.

| Number | Command           |
|--------|-------------------|
| 00     | Start             |
| 01     | Stop              |
| 02     | Set Ampere Meter  |
| 03     | Set Source Meter  |
| 04     | Restart           |

For more details, the code inside the folder "nRF Software/cpuapp/main.c" has all the details commented.

### **LCD**:

This is the most complicated part of making an adaptation of the LCD libraries already created. As you need it to work on the pins of the board, an Arduino shield was used to connect it to the board, the connection diagram is as follows:

<img src="./Images/lcd.png" width="1000">

In the software the connections were defined in this way.

| LCD PIN          | Board PIN |
|------------------|-----------|
| D4               | P1.06     |
| D5               | P1.07     |
| D6               | P1.08     |
| D7               | P1.09     |
| E                | P1.10     |
| RS               | P1.11     |
| Backlight shield | P1.12     |

For more details on how the library works, the code inside the folder "nRF Software/cpuapp/main.c".

### **Buttons**:

The buttons were used using the polling technique with a 10ms debounce.

    while (gpio_pin_get(button1, SW0_GPIO_PIN) == 1)
        {
            k_msleep(10);
            if (gpio_pin_get(button1, SW0_GPIO_PIN) == 1)
            {
                button = "1";
                flag = true;
            }
            if (gpio_pin_get(button1, SW0_GPIO_PIN) == 0)
            {
                k_msleep(10);
                if (gpio_pin_get(button1, SW0_GPIO_PIN) == 0)
                {
                    break;
                }
            }
        }

All the details can be found in the code at "nRF Software/cpuapp/main.c"

# RPi Gateway Setup:

To avoid having to have a cell phone constantly using the nRF Cloud Gateway, I decided to install an Android operating system on a RPi 4, in order to serve as a constant Gateway.

RPi Android OS: OmniROM
https://omnirom.org/

<img src="./Images/btrasp.jpg" width="1000">

# RPi Zero Power Profiler Device Setup:

To use the power profiler, we use a raspberry pi zero to perform the Serial control of the profiler and also be able to receive the commands through the MQTT API of nRF Cloud.

### **Setup MQTT API**:

To obtain the credentials to use the nRF MQTT API follow the steps in the following official documentation.

https://nrfcloud.com/#/docs/guides/mqtt

To obtain all the necessary credentials and certificates use the Postman software.

<img src="./Images/postman.png" width="1000">

In order to receive messages through MQTT use the paho-mqtt library, all the code will be in the "PPK rpi" folder.

Important considerations:

- Save the 3 certificates in the Certs folder.
- Write your corresponding Endpoint in the main.py code
- Write your corresponding clientId in the main.py code

        EndPoint = "XXXXXXXXXXXXXXX.iot.us-east-1.amazonaws.com"
        Client = "account-XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
        sub_topic = 'prod/XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX/a/gateways'

### **Setup PPK Python API**:

Everything you need to use the API is in the "PPK rpi" folder.

It is necessary to install the following libraries on the RPi.

    pip3 install paho-mqtt adafruit-circuitpython-neopixel 

To run the gateway correctly it is necessary to run the program with "sudo".

    sudo python3 main.py

Here is an example of how the MQTT broker receives messages.

Video: Click on the image
[![Car](./Images/mqtt.png)](https://youtu.be/jLefm-G0hh8)

# Test Metodolody:

For this project we decided to test the operation of the device in a controlled situation where the measured currents were standardized, in this case the following platform was used.

<img src="./Images/TestPlatform.png" width="1000">

# Laptop Test:

Testing the power profiler with the resistors, checking that the measurements are correct in the nRF Power Profiles App:

Video: Click on the image
[![Laptop](./Images/lap.png)](https://youtu.be/cVlkMNsqXnY)

# BT Test:

Checking that the Raspberry Pi works correctly as a Gateway.

Video: Click on the image
[![Bt](./Images/btrasp.jpg)](https://youtu.be/8QyenPii7Fg)

# Full Test:

Here is a test of the entire system running at the same time.

Video: Click on the image
[![Full](./Images/back.png)](https://youtu.be/RgJ5gom6gcs)

# The Final Product:

**nRF5340 BLE Control:**

Open case:

<img src="./Images/device1open.png" width="600">
<img src="./Images/device1semi.png" width="600">

Closed case:

<img src="./Images/device1.png" width="600">

**Shock Detector Device:**

Open case:

<img src="./Images/device2open.png" width="600">

Closed case:

<img src="./Images/device2.png" width="600">

### Epic DEMO:

Video: Click on the image
[![DEMO](./Images/back.png)](https://youtu.be/b37HCgYLNZs)

## References:

Links:

[1]. https://www.sciencedirect.com/topics/biochemistry-genetics-and-molecular-biology/macroshock
