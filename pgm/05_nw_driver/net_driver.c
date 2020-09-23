#include<linux/module.h>
#include<linux/kernel.h>

#include<linux/netdevice.h>
#include<linux/string.h>

static int dummy_open(struct net_device *dev)
{
	pr_alert("%s\n", __func__);
	netif_start_queue(dev);
	return 0;
}

static int dummy_close(struct net_device *dev)
{
	pr_alert("%s\n", __func__);
	netif_stop_queue(dev);
	return 0;
}

static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *dev)
{
	pr_alert("%s\n", __func__);
	dev_kfree_skb(skb);
	return 0;
}

static int dummy_net_init(struct net_device *dev)
{
//	dev->netdev_ops->ndo_open 	= 	dummy_open;
//	dev->netdev_ops->ndo_stop 	=	dummy_close;
//	dev->netdev_ops->ndo_start_xmit = dummy_xmit;

	pr_alert("dummy dev initialized\n");

	return 0;

}

struct net_device_ops dummy_ndev_ops = {
	.ndo_open = dummy_open,
	.ndo_stop = dummy_close,
	.ndo_start_xmit = dummy_xmit,
};

struct net_device dummy_device; // = {.netdev_ops->ndo_init = dummy_net_init};

static int __init net_driver_init(void)
{
	int result;

	pr_alert("net_driver_init\n");

	strcpy(dummy_device.name, "dummy");

	if(result = register_netdev(&dummy_device)) {
		pr_alert("dummy: Error %d init card failed\n", result);
		return result;
	}

	dummy_device.netdev_ops = &dummy_ndev_ops;

	return 0;
}

static void __exit net_driver_exit(void)
{
	pr_alert("net_driver_exit\n");

	unregister_netdev(&dummy_device);

	return;
}

module_init(net_driver_init);
module_exit(net_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vinai");
MODULE_DESCRIPTION("net dummy");
MODULE_VERSION("0.1");


