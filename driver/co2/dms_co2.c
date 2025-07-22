/*
 * dms_co2.c - SCD4x CO2 sensor kernel module driver
 *
 * Supports: Sensirion SCD40/SCD41 via I2C
 * Provides: /dev/dms_co2 (readable CO2, temperature, humidity data)
 *
 * Usage:
 *   cat /dev/dms_co2  # returns "426 24.50 43.00\n"
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>

/* -------------------- Sensirion CRC & Byte Utils -------------------- */
static u8 sensirion_crc8(const u8 *data, int len)
{
    u8 crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
    return crc;
}

static u16 sensirion_bytes_to_u16(const u8 *data)
{
    return ((u16)data[0] << 8) | data[1];
}

/* -------------------- SCD4x Sensor Core Logic -------------------- */
static struct i2c_client *g_client;

static int scd4x_measure_single_shot(u16 *co2, int *temp_milli, int *rh_milli)
{
    int ret;
    u8 cmd[2] = { 0x21, 0x9D };     // measure_single_shot
    u8 read_cmd[2] = { 0xEC, 0x05 }; // read_measurement
    u8 rx[9];
    struct i2c_msg msgs[2];

    // 1. Send measure command
    msgs[0].addr = g_client->addr;
    msgs[0].flags = 0;
    msgs[0].len = 2;
    msgs[0].buf = cmd;
    ret = i2c_transfer(g_client->adapter, msgs, 1);
    if (ret < 0)
        return ret;

    // 2. Wait for measurement
    msleep(5000);  // Required by SCD4x datasheet (5s)

    // 3. Send read command
    msgs[0].buf = read_cmd;
    msgs[0].len = 2;
    msgs[0].flags = 0;
    msgs[1].addr = g_client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 9;
    msgs[1].buf = rx;
    ret = i2c_transfer(g_client->adapter, msgs, 2);
    if (ret < 0)
        return ret;

    // 4. CRC check for each 2-byte field
    for (int i = 0; i < 3; i++)
        if (sensirion_crc8(&rx[i * 3], 2) != rx[i * 3 + 2])
            return -EIO;

    // 5. Parse values
    u16 co2_raw = sensirion_bytes_to_u16(&rx[0]);
    u16 temp_raw = sensirion_bytes_to_u16(&rx[3]);
    u16 rh_raw = sensirion_bytes_to_u16(&rx[6]);

    *co2 = co2_raw;
    *temp_milli = ((17500 * temp_raw) / 65535) - 4500;  // in 0.01 °C
    *rh_milli = (10000 * rh_raw) / 65535;               // in 0.01 %RH
    return 0;
}

/* -------------------- /dev/dms_co2 Interface -------------------- */
static ssize_t dms_co2_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    u16 co2;
    int temp_milli, rh_milli;
    char result[64];
    int len, ret;

    if (*ppos != 0)
        return 0;  // EOF on second read

    ret = scd4x_measure_single_shot(&co2, &temp_milli, &rh_milli);
    if (ret)
        return ret;

    len = snprintf(result, sizeof(result), "%u %d.%02d %d.%02d\n",
                   co2,
                   temp_milli / 100, temp_milli % 100,
                   rh_milli / 100, rh_milli % 100);

    if (copy_to_user(buf, result, len))
        return -EFAULT;

    *ppos += len;
    return len;
}

static const struct file_operations dms_fops = {
    .owner = THIS_MODULE,
    .read  = dms_co2_read,
};

static struct miscdevice dms_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "dms_co2",
    .fops  = &dms_fops,
    .mode  = 0444,
};

/* -------------------- I2C Driver Bindings -------------------- */
static int dms_probe(struct i2c_client *client)
{
    int ret;
    g_client = client;

    ret = misc_register(&dms_miscdev);
    if (ret) {
        dev_err(&client->dev, "Failed to register misc device\n");
        return ret;
    }

    dev_info(&client->dev, "dms_co2 sensor initialized\n");
    return 0;
}

static void dms_remove(struct i2c_client *client)
{
    misc_deregister(&dms_miscdev);
    g_client = NULL;
}

static const struct of_device_id dms_dt_ids[] = {
    { .compatible = "dms,scd4x" },
    { }
};
MODULE_DEVICE_TABLE(of, dms_dt_ids);

static struct i2c_driver dms_driver = {
    .driver = {
        .name = "dms_co2",
        .of_match_table = dms_dt_ids,
    },
    .probe = dms_probe,
    .remove = dms_remove,
};

module_i2c_driver(dms_driver);

MODULE_AUTHOR("안전운전해조");
MODULE_DESCRIPTION("SCD4x CO2 Sensor Driver with /dev Interface");
MODULE_LICENSE("GPL");
