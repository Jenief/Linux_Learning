#include <linux/module.h>
#include <linux/kernel.h>
#include<linux/init.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/uaccess.h>
#include <linux/ioctl.h>

#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include<linux/mutex.h>


#define DEVICE_NAME "my_sensor"
#define CLASS_NAME "my_sensor_class"


/* --------------------------------------------------------- */
/* IOCTL definitions                                         */
/* --------------------------------------------------------- */

#define SENSOR_MAGIC 'S'

#define SENSOR_START _IO(SENSOR_MAGIC, 1)
#define SENSOR_STOP _IO(SENSOR_MAGIC, 2)
#define SENSOR_GET_VAL _IOR(SENSOR_MAGIC, 3, int)

/* --------------------------------------------------------- */
/* Driver data                                               */
/* --------------------------------------------------------- */

static dev_t dev_number;

static struct cdev sensor_cdev;
static struct class *sensor_class;
static struct device *sensor_device;

/* SENSOR VALUE */
static int sensor_value = 0;

/* SENSOR RUNNING STATE */
static bool sensor_running = false;

/*Data available flag*/
static bool data_available = false;

/* Protect sensor data*/
static DEFINE_MUTEX(sensor_lock);

/* Wait quee for poll/read*/
static DECLARE_WAIT_QUEUE_HEAD(sensor_wait_queue);

/* Time to simulate sensor */
static struct timer_list sensor_timer;


/* -------------------------------------------------- */
/* RELEASE                                            */
/* -------------------------------------------------- */
static void sensor_timer_Callback(struct timer_list *timer)
{
    mutex_lock(&sensor_lock);

    if(sensor_running)
    {
        /* Simulate sensor_value */
        sensor_value++;

        if(sensor_value > 100)
            sensor_value = 0;

        data_available = true;
        /* wake process waiting in
        *   poll(), select(), epoll(), read()
        */
        wake_up_interruptible(&sensor_wait_queue);
    }

    mutex_unlock(&sensor_lock);

    /*
    *   Generate another sensor event after 1 second
    */
   mod_timer(&sensor_timer, jiffies + msecs_to_jiffies(1000));
}

/* -------------------------------------------------- */
/* OPEN                                               */
/* -------------------------------------------------- */

static int sensor_open(struct inode *inode, struct file *file)
{
    pr_info("my_sensor: device opened\n");
    return 0;
}

/* -------------------------------------------------- */
/* MODULE RELEASE                                     */
/* -------------------------------------------------- */

static int sensor_release(struct inode *inode, struct file *file)
{
    pr_info("my_sensor: device closed\n");
    return 0;
}


/* -------------------------------------------------- */
/* READ                                               */
/* -------------------------------------------------- */
static ssize_t sensor_read(struct file *file,char __user *buf,size_t count,loff_t *offset)
{
    char data[64];
    int len;

    /* wait untill sensor data become avialable*/
    if(!data_available)
    {
        if(file->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }
        if(wait_event_interruptible(sensor_wait_queue, data_available))
        {
            return -ERESTARTSYS;
        }
    }

    mutex_lock(&sensor_lock);

    len = snprintf(data, sizeof(data), "sensor value = %d\n", sensor_value);

    if(count < len)
    {
        mutex_unlock(&sensor_lock);
        return -EFAULT;
    }

    if(copy_to_user(buf, data, len))
    {
        mutex_unlock(&sensor_lock);
        return -EFAULT;
    }

    data_available = false;

    mutex_unlock(&sensor_lock);

    pr_info("my_Sensor: read() -> %d\n", sensor_value);

    return len;
}


/* -------------------------------------------------- */
/* WRITE                                              */
/* -------------------------------------------------- */

static ssize_t sensor_write(struct file *file,const char __user *buffer,size_t count, loff_t *offset)
{
    char command[64];

    if(count >= sizeof(command))
        return -EINVAL;

    if(copy_from_user(command, buffer, count))
    {
        return -EFAULT;
    }
    command[count] = '\0';

    if(strncmp(command, "STATUS", 6) == 0)
    {
        mutex_lock(&sensor_lock);
        pr_info("my_sensor: STATUS value=%d running=%d\n", sensor_value, sensor_running);
        mutex_unlock(&sensor_lock);
    }

    return count;

}

/* -------------------------------------------------- */
/* IOCTL                                              */
/* -------------------------------------------------- */

static long sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;

    switch(cmd)
    {
        case SENSOR_START:
            mutex_lock(&sensor_lock);
            sensor_running = true;
            data_available = false;
            sensor_value =0;
            mutex_unlock(&sensor_lock);
            mod_timer(&sensor_timer, jiffies + msecs_to_jiffies(1000));
            pr_info("my_sensor: SENSOR_START\n");
            break;
        case SENSOR_STOP:
            mutex_lock(&sensor_lock);
            sensor_running = false;
            mutex_unlock(&sensor_lock);
            /*STOP_TIMER*/
            timer_delete_sync(&sensor_timer);
            pr_info("my__sensor: SENSOR_STOP\n");
            break;
        case SENSOR_GET_VAL:
            mutex_lock(&sensor_lock);
            value = sensor_value;
            mutex_unlock(&sensor_lock);
            if(copy_to_user((int __user *)arg, &value, sizeof(value)))
            {
                return -EFAULT;
            }
            pr_info("my_sensor: SENSOR_GET_VAL -> %d\n", value);
            break;
        default:
        return -ENOTTY;
    }
    return 0;            

}

/* --------------------------------------------------------- */
/* poll()                                                    */
/* --------------------------------------------------------- */

static __poll_t sensor_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;

    /*register process with wait queue*/
    poll_wait(file, &sensor_wait_queue, wait);

    mutex_lock(&sensor_lock);

    if(data_available)
    {
        mask |= POLLIN | POLLRDNORM;
    }

    mutex_unlock(&sensor_lock);

    return mask;
}

/* -------------------------------------------------- */
/* FILE OPERATIONS                                    */
/* -------------------------------------------------- */

static const struct file_operations sensor_fops = {
    .owner = THIS_MODULE,
    .open = sensor_open,
    .release = sensor_release,
    .read = sensor_read,
    .write = sensor_write,
    .unlocked_ioctl = sensor_ioctl,
    .poll = sensor_poll,
};

/* -------------------------------------------------- */
/* MODULE INIT                                        */
/* -------------------------------------------------- */

static int __init sensor_init(void)
{
    int ret;

    pr_info("my_sensor: Initalizing driver\n");

    /*Allocate major, minor number*/
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);

    if(ret < 0)
    {
        pr_err("my_sensor: failed to allocate chrdev region\n");
        return ret;
    }

    pr_info("my_sensor: major=%d, minor=%d\n", MAJOR(dev_number), MINOR(dev_number));

    /*Initialize cdev*/

    cdev_init(&sensor_cdev, &sensor_fops);
    sensor_cdev.owner = THIS_MODULE;
    
    /*Add Charater device*/
    ret = cdev_add(&sensor_cdev, dev_number, 1);
    if(ret < 0)
    {
        pr_err("mydevice: Failed to add cddev\n");
        unregister_chrdev_region(dev_number,1);
        return ret;
    }

    /*Create class*/
    sensor_class = class_create(CLASS_NAME);
    if(IS_ERR(sensor_class))
    {
        ret = PTR_ERR(sensor_class);
        cdev_del(&sensor_cdev);
        unregister_chrdev_region(dev_number,1);
        return ret;
    }

    /*Create /dev/my_sensor */
    sensor_device = device_create(sensor_class, NULL, dev_number, NULL, DEVICE_NAME);
    if(IS_ERR(sensor_device))
    {
        ret = PTR_ERR(sensor_device);
        class_destroy(sensor_class);
        cdev_del(&sensor_cdev);
        unregister_chrdev_region(dev_number,1);
        pr_err("mydevice: Failed to create device\n");
        return ret;
    }

   /* Initalize timer*/
   timer_setup(&sensor_timer, sensor_timer_Callback, 0);

   pr_info("my_sensor: Driver loaded\n");
   pr_info("my_sensor: /dev/my_sensor created\n");
   return 0;
}


/* --------------------------------------------------------- */
/* Module cleanup                                             */
/* --------------------------------------------------------- */

static void __exit sensor_exit(void)
{
    /*Stop timer */
    timer_shutdown_sync(&sensor_timer);

    /*Destroy device*/
    device_destroy(sensor_class,dev_number);

    /*Destroy Class*/
    class_destroy(sensor_class);

    /*Remove cdev*/
    cdev_del(&sensor_cdev);

    /*Free Majot/Minor*/
    unregister_chrdev_region(dev_number,1);

    pr_info("my_sensor: Driver unloaded\n");

}

module_init(sensor_init);
module_exit(sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jenief");
MODULE_DESCRIPTION("QEMU Linux Sesnor Character Device");
MODULE_VERSION("0.1");
