# Linux usb_drive 内核模块
##查看USB的一些信息
    关键函数:
> usb_register(&skel_driver);//注册设备  
      usb_deregister(&skel_driver);//注销设备	
    
##<a name="code"/>
```C
static struct usb_driver skel_driver = {	
    .name =		"xusb",//设备名字
    .probe =	skel_probe,//连接响应函数
    .disconnect =	skel_disconnect,//断开连接函数
    .id_table =	skel_table,
  }; 
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
```
