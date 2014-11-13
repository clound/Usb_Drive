#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/usb.h>
/* Define these values to match your devices */
#define USB_SKEL_VENDOR_ID	0x1f75 //获得usb的ID号
#define USB_SKEL_PRODUCT_ID	0x0916
#define USB_SKEL_MINOR_BASE	192
#define to_skel_dev(d) container_of(d, struct xusb_dev, kref)//从内核引用计数获得usb（接口）设备指针
/*该驱动程序支持的设备列表*/
static const struct usb_device_id skel_table[] = {
	{ USB_DEVICE(USB_SKEL_VENDOR_ID, USB_SKEL_PRODUCT_ID) },
	{ }					/* 终止入口项*/
};
MODULE_DEVICE_TABLE(usb, skel_table);//判断出该程序可以控制什么设备

struct xusb_dev{
	struct usb_device	*udev;	
	struct usb_interface	*interface;	
	struct kref		kref;
};
static const struct file_operations skel_fops = {
	.owner =	THIS_MODULE,
	.read =		NULL,
	.write =	NULL,
	.open =		NULL,
	.release =	NULL,
	.flush =	NULL,
	.llseek =	NULL,
};


static struct usb_class_driver skel_class = {
	.name =		"skel%d",
	.fops =		&skel_fops,
	.minor_base =	USB_SKEL_MINOR_BASE,
};


static void skel_delete(struct kref *kref)//kref减为0时的删除函数
{
	struct xusb_dev *dev = to_skel_dev(kref);
	usb_put_dev(dev->udev);
	kfree(dev);//释放设备
}
static int skel_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct xusb_dev *dev;
	int retval = -ENOMEM;
	/* allocate memory for our device state and initialize it */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}
	kref_init(&dev->kref);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));//获得此接口所在设备的usb_device的指针
	dev->interface = interface;
	printk("product: %s,manufacturer: %s,serial: %s\n",dev->udev->product,dev->udev->manufacturer,dev->udev->serial);
	
	usb_set_intfdata(interface, dev);	
	retval = usb_register_dev(interface, &skel_class);//注册一个interface
	if (retval) {
		/* something prevented us from registering this driver */
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}
	dev_info(&interface->dev,"USB Skeleton device now attached to USBSkel#%d",interface->minor);//次设备号
error:
	if (dev)
		kref_put(&dev->kref, skel_delete);
	printk("usb connected!\n");
	return retval;
}
static void skel_disconnect(struct usb_interface *interface)
{
	struct xusb_dev *dev;
	printk("usb disconnected!\n");
	dev = usb_get_intfdata(interface);//获得（接口）设备
	usb_set_intfdata(interface, NULL);//恢复设备对应的device结构体的p字段
	
	usb_deregister_dev(interface, &skel_class);//返回次设备号

	kref_put(&dev->kref, skel_delete);

	dev_info(&interface->dev, "USB Skeleton #%d now disconnected",interface->minor);//次设备号	
	
}
static struct usb_driver skel_driver = {	
	.name =		"xusb",//设备名字
	.probe =	skel_probe,//连接响应函数
	.disconnect =	skel_disconnect,//断开连接函数
	.id_table =	skel_table,
};
static int __init
init_usb(void)
{
	int result;
	result = usb_register(&skel_driver);//注册设备
	if(result)
		printk("usb_register failed,Error number %d\n",result);
	return 0;
}
static void __exit
exit_usb(void)
{
	usb_deregister(&skel_driver);//注销设备	
	printk("GoodBye kernel\n");
}
module_init(init_usb);
module_exit(exit_usb);
MODULE_LICENSE("GPL");

