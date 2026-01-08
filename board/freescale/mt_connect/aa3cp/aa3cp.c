//******************************************************************************
// @file      : aa3cp.c
// @brief     : AA3CP Control
//******************************************************************************
// @attention
//
// Copyright (c) 2026 MultiTracks.com, LLC.
// All rights reserved.
//
//******************************************************************************

//******************************************************************************
// Includes
//******************************************************************************
#include <asm-generic/gpio.h>
#include <asm/mach-imx/gpio.h>
#include <common.h>
#include <dm.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <linux/delay.h> 
#include <stdio.h>

#include "aa3cp.h"

//******************************************************************************
// Private defines
//******************************************************************************
#define AA3CP_DEVICE                3
#define AA3CP_I2C_ADDRESS           0x10

// status register
#define AA3CP_REG_DEVICE_VERSION    0
#define AA3CP_REG_AUTH_REV          1
#define AA3CP_REG_AUTH_MAJOR_VER    2
#define AA3CP_REG_AUTH_MINOR_VER    3
#define AA3CP_REG_DEVICE_ID         4

//******************************************************************************
// Private typedefs
//******************************************************************************

//******************************************************************************
// Private macros
//******************************************************************************

//******************************************************************************
// Private variables
//******************************************************************************

//******************************************************************************
// Private function prototypes
//******************************************************************************
static bool AA3CP_ReadRegister(struct udevice *dev, uint8_t reg, int len, uint8_t *pBuf);

//******************************************************************************
// Private user code
//******************************************************************************

//******************************************************************************
// @brief  AA3CP Init
// @param  None
// @retval True if the AA3CP was initialized, false otherwise
//******************************************************************************
bool AA3CP_Init(void)
{
    uint8_t data[8];
    struct udevice *bus, *dev;

    // enable the AA3CP
    gpio_request(IMX_GPIO_NR(1, 1), "AA3CP Enable");
    gpio_direction_output(IMX_GPIO_NR(1, 1) , 1);

    // mandatory 2ms delay
    mdelay(2);

    // get the bus
    if (uclass_get_device_by_seq(UCLASS_I2C, AA3CP_DEVICE, &bus) != 0)
    {
        // disable the AA3CP
        gpio_set_value(IMX_GPIO_NR(1, 1) , 0);
        return false;
    }

    // mandatory 2ms delay
    mdelay(2);

    // get the device
    if (dm_i2c_probe(bus, AA3CP_I2C_ADDRESS, AA3CP_DEVICE, &dev) != 0)
    {
        // disable the AA3CP
        gpio_set_value(IMX_GPIO_NR(1, 1) , 0);
        return false;
    }

    // mandatory 2ms delay
    mdelay(2);

    // read device ver, auth rev, auth maj ver, auth min ver, device id
    if (AA3CP_ReadRegister(dev, AA3CP_REG_DEVICE_VERSION, 8, data) == false)
    {
        // error
        return false;
    }
    printf("AA3CP:%d:%d:%d:%d:%02X%02X%02X%02X\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

    // disable the AA3CP
    gpio_set_value(IMX_GPIO_NR(1, 1) , 0);

    // success
    return true;
}

//******************************************************************************
// @brief  Reads the specified number of bytes starting from the specified register
// @param  dev: I2C device
// @param  reg: the register to start reading from
// @param  len: the number of bytes to read
// @param  pBuf: pointer to where the read data will be stored
// @retval True if the read succeeded, false otherwise
//******************************************************************************
static bool AA3CP_ReadRegister(struct udevice *dev, uint8_t reg, int len, uint8_t *pBuf)
{
    struct i2c_msg msg[1];

    // configure write message
    msg[0].addr  = AA3CP_I2C_ADDRESS;
    msg[0].flags = 0;
    msg[0].len   = 1;
    msg[0].buf   = &reg;

    // write register address
    if (dm_i2c_xfer(dev, msg, 1) != 0)
    {
        // error
        return false;
    }

    // mandatory 2ms delay
    mdelay(2);

    // configure read message
    msg[0].addr  = AA3CP_I2C_ADDRESS;
    msg[0].flags = I2C_M_RD;
    msg[0].len   = len;
    msg[0].buf   = pBuf;

    // read register
    if (dm_i2c_xfer(dev, msg, 1) != 0)
    {
        // error
        return false;
    }

    // success
    return true;
}
