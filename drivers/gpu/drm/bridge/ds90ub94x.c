#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/delay.h>

#define DS90UB941_I2C_ADDR 0x0c
#define DS90UB948_I2C_ADDR 0x2c

struct ds90ub94x {
	struct i2c_client *ds90ub941_i2c;
	struct i2c_client *ds90ub948_i2c;
};

static struct ds90ub94x *g_ds90ub94x;

static void ds90ub94x_write_reg(struct i2c_client *client, u8 reg, u8 data)
{
	int ret;
	u8 b[2];
	struct i2c_msg msg;

	b[0] = reg;
	b[1] = data;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = b;
	msg.len = 2;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret != 1)
		printk("i2c write failed: ret=%d reg=%02x\n",ret, reg);
}

static void ds90ub94x_display_setting(void)
{
        printk("------------>%s, %s\r\n", __FILE__, __FUNCTION__);
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x01, 0x0f); /* Reset DSI/DIGITLE */
	usleep_range(5000, 6000);	/* time cannot be too short */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x03, 0xBA); /* Enable FPD-Link I2C pass through */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x1E, 0x01); /* Select FPD-Link III Port 0 */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x5B, 0x21); /* FPD3_TX_MODE=single, Reset PLL */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x4F, 0x8C); /* DSI Continuous Clock Mode,DSI 4 lanes */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x40, 0x04); /* Set DSI0 TSKIP_CNT value ???*/
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x41, 0x05);
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x42, 0x14);
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x01, 0x00); /* Release DSI/DIGITLE reset */

	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x01, 0x01); /* ds90ub948 reset */
	usleep_range(10000, 11000);	/* time cannot be too short */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x49, 0x62); /* Set FPD_TX_MODE, MAPSEL=1(SPWG), Single OLDI output */
        /* Set GPIO via local I2C */
        //ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x34, 0x01); /* Select FPD-Link III Port 0, GPIOx instead of D_GPIOx */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x34, 0x02); /* Select FPD-Link III Port 0, GPIOx instead of D_GPIOx */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x1D, 0x19); /* GPIO0, MIPI_BL_EN */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x1E, 0x99); /* GPIO1, MIPI_VDDEN; GPIO2, MIPI_BL_PWM */

        /* Configure remote devcie ID in local SER */
        #if 0
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x7, 0xac); /* SlaveID_0: EEPROM */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x8, 0xac); /* SlaveAlias_0: EEPROM */

	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x8, 0xac); /* SlaveID_0: EEPROM */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x10, 0xac); /* SlaveAlias_0: EEPROM */
	#endif

        //Reset touch interrupt
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x34, 0x02);
        //msleep(10);
        //ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x1f, 0x01);
        //msleep(10);
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x1f, 0x09);
        msleep(10);

        /* exc80w46 */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x7, 0x54); /* SlaveID_0: touch panel */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x8, 0x54); /* SlaveAlias_0: touch panel */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x8, 0x54); /* SlaveID_0: touch panel */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x10, 0x54); /* SlaveAlias_0: touch panel */
#if 0
        /* edt-ft5x06 */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x7, 0x70); /* SlaveID_0: touch panel */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x8, 0x70); /* SlaveAlias_0: touch panel */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x8, 0x70); /* SlaveID_0: touch panel */
        ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0x10, 0x70); /* SlaveAlias_0: touch panel */
#endif
        /* Configure remote interrupt for touch panel */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub941_i2c, 0x30, 0x1); /* REM_INTB_CTRL: port 0 remote interrupt */
	ds90ub94x_write_reg(g_ds90ub94x->ds90ub948_i2c, 0xC6, 0x21); /* INTB: enable INTB_IN on remote DES */
}

static int ds90ub94x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct ds90ub94x *ds90ub94x;

	ds90ub94x = devm_kzalloc(dev, sizeof(*ds90ub94x), GFP_KERNEL);
	if (!ds90ub94x)
		return -ENOMEM;

	ds90ub94x->ds90ub941_i2c = client;
	ds90ub94x->ds90ub948_i2c = i2c_new_dummy_device(client->adapter, DS90UB948_I2C_ADDR);
	if (!ds90ub94x->ds90ub948_i2c) {
		return -ENODEV;
	}

	i2c_set_clientdata(client, ds90ub94x);

	g_ds90ub94x = ds90ub94x;

	/* config display */
	ds90ub94x_display_setting();

	return 0;
}

static int ds90ub94x_remove(struct i2c_client *client)
{
	return 0;
}

static const struct of_device_id ds90ub94x_of_match[] = {
	{
		.compatible = "ti,ds90ub94x"
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ds90ub94x_of_match);

static struct i2c_driver ds90ub94x_driver = {
	.driver = {
		.name = "ds90ub94x",
		.of_match_table = ds90ub94x_of_match,
	},
	.probe = ds90ub94x_probe,
	.remove = ds90ub94x_remove,
};
module_i2c_driver(ds90ub94x_driver);

MODULE_DESCRIPTION("TI. DS90UB941/DS90UB948 SerDer bridge");
MODULE_LICENSE("GPL");