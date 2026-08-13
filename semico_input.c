#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>

#define SEMICO_VENDOR_ID 0x1a2c
#define SEMICO_PRODUCT_ID 0x4c5e

static struct input_dev *virtual_keyboard;

/*
*	Receive events from the physical keyboard
*/

static void semico_event(struct input_handle *handle,unsigned int type,unsigned int code,int value)
{
	/*A KEY Pressed*/
	if(type == EV_KEY && code == KEY_A)
	{
		pr_info("SEMICO_INPUT: A Detected value=%d\n",value);
		
		/*Generate B Instead*/
		input_report_key(virtual_keyboard,KEY_B,value);
		input_sync(virtual_keyboard);

	}
}

/*
*	Connect to Physical Keyboard
*/
static int semico_connect(struct input_handler *handler,struct input_dev *dev,const struct input_device_id *id)
{
	struct input_handle *handle;
	int ret;

	pr_info("SEMICO_INPUT: Found device name=%s\n",dev->name);
/*	pr_info("SEMICO_INPUT: name= %s\n",dev->name);
	pr_info("SEMICO_INPUT: phys=%s\n",dev->phys);
	pr_info("SEMICO_IPUT: uniq=%s\n",dev->uniq);
	pr_info("SEMICO_INPUT: bustype=%u\n", dev->id.bustype);
*/
	handle = kzalloc(sizeof(*handle),GFP_KERNEL);
	if(!handle)
	{
		pr_err("SEMICO_INPUT: Failed to allocate handle\n");
		return -ENOMEM;
	}

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "semico_input_handle";

	ret = input_register_handle(handle);

	if(ret)
	{
		pr_err("SEMICO_INPUT: input_register_handle failed\n");
		kfree(handle);
		return ret;
	}

	ret = input_open_device(handle);
	if(ret)
	{
		pr_err("SEMICO_INPUT: input_open_device Failed ret=%d\n",ret);
		input_register_handle(handle);
		kfree(handle);
		return ret;
	}

//	pr_info("SEMICO_INPUT: input_open_Device SUCSESS\n");

	pr_info("SEMICO_INPUT: Connected to %s\n",dev->name);

	return 0;
}

static void semico_disconnect(struct input_handle *handle)
{
	pr_info("SEMICO_INPUT: Disconnected from %s\n",handle->dev->name);
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id semico_ids[]={
	{
	.flags =  INPUT_DEVICE_ID_MATCH_BUS | INPUT_DEVICE_ID_MATCH_VENDOR | INPUT_DEVICE_ID_MATCH_PRODUCT,
	.bustype = BUS_USB,
	.vendor = SEMICO_VENDOR_ID,
	.product = SEMICO_PRODUCT_ID,
	 },
	{}
};

MODULE_DEVICE_TABLE(input,semico_ids);

static struct input_handler semico_handler = {
	.event = semico_event,
	.connect = semico_connect,
	.disconnect = semico_disconnect,
	.name = "semico_input",
	.id_table = semico_ids,
};

static int __init semico_init(void)
{
	int ret;
	pr_info("SEMICO_INPUT: Driver Loading\n");
	ret = input_register_handler(&semico_handler);

	/*
	*	Create Virtual Keyboard
	*/
	virtual_keyboard = input_allocate_device();

	if(!virtual_keyboard)
	{
		pr_err("SEMICO_INPUT: Failed to allocate virtual keyboard\n");
		return -ENOMEM;
	}

	virtual_keyboard->name="SEMICO_VIRTUAL_KEYBOARD";
	virtual_keyboard->phys="semico/input0";
	virtual_keyboard->id.bustype= BUS_VIRTUAL;

	/*
	* Tell Linux that this device can generate KEyboard events
	*/
	__set_bit(EV_KEY,virtual_keyboard->evbit);
	__set_bit(KEY_B,virtual_keyboard->keybit);

	/*
	* Register Virtual Keyboard
	*/
	ret = input_register_device(virtual_keyboard);
	if(ret)
	{
		pr_err("SEMICO_INPUT: Failed to register virtual keyboard\n");
		input_free_device(virtual_keyboard);
		return ret;
	}

	pr_info("SEMICO_INPUT: Virtual Keyboard Registered\n");

	/*
	* Register our physical keyboard handler
	*/
	ret = input_register_handler(&semico_handler);
	if(ret)
	{
		pr_err("SEMICO_INPUT: Handler Registeration failed\n");
		input_unregister_device(virtual_keyboard);
		return ret;
	}

	pr_info("SEMICO_INPUT: Driver Registered\n");

	return 0;
}

/*
* Driver Cleanup
*/
static void  __exit semico_exit(void)
{
	input_unregister_handler(&semico_handler);

	if(virtual_keyboard)
	{
		input_unregister_device(virtual_keyboard);
		virtual_keyboard =NULL;
	}
	pr_info("SEMICO_INPUT: Driver Unloaded\n");
}

module_init(semico_init);
module_exit(semico_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jenief");
MODULE_DESCRIPTION("Custom Semico keyboard input handler");
