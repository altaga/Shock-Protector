/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/hrs.h>

#define SLEEP_TIME_MS 1

#define NUMBER_OF_STRING 5
#define MAX_STRING_SIZE 40

const char menu[NUMBER_OF_STRING][MAX_STRING_SIZE] =
{ "Start  ",
  "Stop   ",
  "Set Amp",
  "Set Vol",
  "Reset  "
};

// LCD Modude

/*
Library Based on https://www.electronicwings.com/pic/interfacing-lcd-16x2-in-4-bit-mode-with-pic18f4550-

Adapted by Victor Altamirano 25/04/2021, board NRF5340 DK

*/

const struct device *dev;

#define PIN1 6
#define PIN2 7
#define PIN3 8
#define PIN4 9
#define RS 10
#define E 11
#define Back 12

//

void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char dat);
void NOP(void);
void LCD_Init();
void LCD_String(const char *msg);
void LCD_String_xy(char row, char pos, const char *msg);
void LCD_Clear();

// LCD Module End

/*
 * Get button configuration from the devicetree sw0 alias.
 *
 * At least a GPIO device and pin number must be provided. The 'flags'
 * cell is optional.
 */

#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define SW2_NODE DT_ALIAS(sw2)
#define SW3_NODE DT_ALIAS(sw3)

#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
#define SW0_GPIO_LABEL DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(SW1_NODE, okay)
#define SW1_GPIO_LABEL DT_GPIO_LABEL(SW1_NODE, gpios)
#define SW1_GPIO_PIN DT_GPIO_PIN(SW1_NODE, gpios)
#define SW1_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW1_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(SW2_NODE, okay)
#define SW2_GPIO_LABEL DT_GPIO_LABEL(SW2_NODE, gpios)
#define SW2_GPIO_PIN DT_GPIO_PIN(SW2_NODE, gpios)
#define SW2_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW2_NODE, gpios))
#endif

#if DT_NODE_HAS_STATUS(SW3_NODE, okay)
#define SW3_GPIO_LABEL DT_GPIO_LABEL(SW3_NODE, gpios)
#define SW3_GPIO_PIN DT_GPIO_PIN(SW3_NODE, gpios)
#define SW3_GPIO_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(SW3_NODE, gpios))
#endif

const struct device *button1;
const struct device *button2;
const struct device *button3;
const struct device *button4;
uint8_t button_selected = 0U;

bool flag = false;
char *button = "0";

void checkButton();

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,BT_UUID_16_ENCODE(BT_UUID_HRS_VAL))
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(void)
{
	int err;

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.cancel = auth_cancel,
};


void main(void)
{
    dev = device_get_binding("GPIO_1");
    LCD_Init();
    LCD_Clear();
    LCD_String_xy(1, 0, "Shock Protector");
    LCD_String_xy(2, 0, "LINK START!");
    LCD_String_xy(2, 12, ".");
    int err;
    err = bt_enable(NULL);
    if (err) {
            printk("Bluetooth init failed (err %d)\n", err);
            return;
    }
    bt_ready();
    bt_conn_cb_register(&conn_callbacks);
    bt_conn_auth_cb_register(&auth_cb_display);

    LCD_String_xy(2, 13, ".");

    int ret;
    button1 = device_get_binding(SW0_GPIO_LABEL);
    button2 = device_get_binding(SW1_GPIO_LABEL);
    button3 = device_get_binding(SW2_GPIO_LABEL);
    button4 = device_get_binding(SW3_GPIO_LABEL);

    if (button1 == NULL)
    {
        printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
        return;
    }
    if (button2 == NULL)
    {
        printk("Error: didn't find %s device\n", SW1_GPIO_LABEL);
        return;
    }
    if (button3 == NULL)
    {
        printk("Error: didn't find %s device\n", SW2_GPIO_LABEL);
        return;
    }
    if (button4 == NULL)
    {
        printk("Error: didn't find %s device\n", SW3_GPIO_LABEL);
        return;
    }

    ret = gpio_pin_configure(button1, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
    if (ret != 0)
    {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
        return;
    }
    ret = gpio_pin_configure(button2, SW1_GPIO_PIN, SW1_GPIO_FLAGS);
    if (ret != 0)
    {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, SW1_GPIO_LABEL, SW1_GPIO_PIN);
        return;
    }
    ret = gpio_pin_configure(button3, SW2_GPIO_PIN, SW2_GPIO_FLAGS);
    if (ret != 0)
    {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, SW2_GPIO_LABEL, SW2_GPIO_PIN);
        return;
    }
    ret = gpio_pin_configure(button4, SW3_GPIO_PIN, SW3_GPIO_FLAGS);
    if (ret != 0)
    {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, SW3_GPIO_LABEL, SW3_GPIO_PIN);
        return;
    }
    LCD_String_xy(2, 14, ".");
    k_msleep(5000);
    LCD_Clear();
    LCD_String_xy(1, 0, "1:UP  | 3:DOWN");
    LCD_String_xy(2, 0, "2:SEL | 4:RES");
    k_msleep(5000);
    LCD_Clear();
    LCD_String_xy(1, 0, "Menu:");
    int scroll = 0;
    while (1)
    {
        checkButton();
        if (flag){
            LCD_String_xy(2, 0, "                ");
            if(button == "1"){
            scroll++;
            }
            else if(button == "2"){
            LCD_String_xy(2, 0, "Sending...");
            bt_hrs_notify((uint16_t) scroll);
            k_msleep(1000);
            LCD_String_xy(2, 0, "Sent      ");
            }
            else if(button == "3"){
            scroll--;
            }
            else if(button == "4"){
            //to be done...
            }
            if(scroll > 4)
            {
             scroll = 0;
            }
            else if(scroll < 0)
            {
             scroll = 4;
            }
            LCD_String_xy(1, 5,menu[scroll]);
            flag = false;
        }
        k_msleep(SLEEP_TIME_MS);
    }
}

// Button Check Button State

void checkButton()
{
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
    while (gpio_pin_get(button2, SW1_GPIO_PIN) == 1)
    {
        k_msleep(10);
        if (gpio_pin_get(button2, SW1_GPIO_PIN) == 1)
        {
            button = "2";
            flag = true;
        }
        if (gpio_pin_get(button2, SW1_GPIO_PIN) == 0)
        {
            k_msleep(10);
            if (gpio_pin_get(button2, SW1_GPIO_PIN) == 0)
            {
                break;
            }
        }
    }
    while (gpio_pin_get(button3, SW2_GPIO_PIN) == 1)
    {
        k_msleep(10);
        if (gpio_pin_get(button3, SW2_GPIO_PIN) == 1)
        {
            button = "3";
            flag = true;
        }
        if (gpio_pin_get(button3, SW2_GPIO_PIN) == 0)
        {
            k_msleep(10);
            if (gpio_pin_get(button3, SW2_GPIO_PIN) == 0)
            {
                break;
            }
        }
    }
    while (gpio_pin_get(button4, SW3_GPIO_PIN) == 1)
    {
        k_msleep(10);
        if (gpio_pin_get(button4, SW3_GPIO_PIN) == 1)
        {
            button = "4";
            flag = true;
        }
        if (gpio_pin_get(button4, SW3_GPIO_PIN) == 0)
        {
            k_msleep(10);
            if (gpio_pin_get(button4, SW3_GPIO_PIN) == 0)
            {
                break;
            }
        }
    }
}

// LCD Functions

void LCD_Init()
{
    gpio_pin_configure(dev, PIN1, GPIO_OUTPUT);
    gpio_pin_configure(dev, PIN2, GPIO_OUTPUT);
    gpio_pin_configure(dev, PIN3, GPIO_OUTPUT);
    gpio_pin_configure(dev, PIN4, GPIO_OUTPUT);
    gpio_pin_configure(dev, RS, GPIO_OUTPUT);
    gpio_pin_configure(dev, E, GPIO_OUTPUT);
    gpio_pin_configure(dev, Back, GPIO_OUTPUT);
    gpio_pin_set(dev, Back, 1);
    k_msleep(15);      /* 15 ms, Power-On delay*/
    LCD_Command(0x02); /*send for initialization of LCD with nibble method */
    LCD_Command(0x28); /*use 2 line and initialize 5*7 matrix in (4-bit mode)*/
    LCD_Command(0x01); /*clear display screen*/
    LCD_Command(0x0c); /*display on cursor off*/
    LCD_Command(0x06); /*increment cursor (shift cursor to right)*/
}

void LCD_Command(unsigned char cmd)
{
    gpio_pin_set(dev, PIN1, (cmd & 0b00010000) >> 4);
    gpio_pin_set(dev, PIN2, (cmd & 0b00100000) >> 5);
    gpio_pin_set(dev, PIN3, (cmd & 0b01000000) >> 6);
    gpio_pin_set(dev, PIN4, (cmd & 0b10000000) >> 7);
    gpio_pin_set(dev, RS, 0);
    gpio_pin_set(dev, E, 1);
    NOP();
    gpio_pin_set(dev, E, 0);
    k_msleep(1);
    gpio_pin_set(dev, PIN1, (cmd & 0b00000001));
    gpio_pin_set(dev, PIN2, (cmd & 0b00000010) >> 1);
    gpio_pin_set(dev, PIN3, (cmd & 0b00000100) >> 2);
    gpio_pin_set(dev, PIN4, (cmd & 0b00001000) >> 3);
    gpio_pin_set(dev, RS, 0);
    gpio_pin_set(dev, E, 1);
    NOP();
    gpio_pin_set(dev, E, 0);
    k_msleep(3);
}

void LCD_Char(unsigned char dat)
{
    gpio_pin_set(dev, PIN1, (dat & 0b00010000) >> 4);
    gpio_pin_set(dev, PIN2, (dat & 0b00100000) >> 5);
    gpio_pin_set(dev, PIN3, (dat & 0b01000000) >> 6);
    gpio_pin_set(dev, PIN4, (dat & 0b10000000) >> 7);
    gpio_pin_set(dev, RS, 1);
    gpio_pin_set(dev, E, 1);
    NOP();
    gpio_pin_set(dev, E, 0);
    k_msleep(1);
    gpio_pin_set(dev, PIN1, (dat & 0b00000001));
    gpio_pin_set(dev, PIN2, (dat & 0b00000010) >> 1);
    gpio_pin_set(dev, PIN3, (dat & 0b00000100) >> 2);
    gpio_pin_set(dev, PIN4, (dat & 0b00001000) >> 3);
    gpio_pin_set(dev, E, 1);
    NOP();
    gpio_pin_set(dev, E, 0);
    k_msleep(3);
}

void NOP(void)
{
    k_usleep(1);
}

void LCD_String(const char *msg)
{
    while ((*msg) != 0)
    {
        LCD_Char(*msg);
        msg++;
    }
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char location = 0;
    if (row <= 1)
    {
        location = (0x80) | ((pos)&0x0f); /*Print message on 1st row and desired location*/
        LCD_Command(location);
    }
    else
    {
        location = (0xC0) | ((pos)&0x0f); /*Print message on 2nd row and desired location*/
        LCD_Command(location);
    }
    LCD_String(msg);
}
void LCD_Clear()
{
    LCD_Command(0x01); /*clear display screen*/
    k_msleep(3);
}

// LCD Functions END