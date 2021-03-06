/*******************************************************************************

  
  Copyright(c) 1999 - 2002 Intel Corporation. All rights reserved.
  
  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation; either version 2 of the License, or (at your option) 
  any later version.
  
  This program is distributed in the hope that it will be useful, but WITHOUT 
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
  
  Contact Information:
  Linux NICS <linux.nics@intel.com>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/


/* Linux PRO/1000 Ethernet Driver main header file */

#ifndef _E1000_H_
#define _E1000_H_
#ifndef __E1000_MAIN__
#define __NO_VERSION__
#endif

#include <linux/stddef.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pagemap.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/capability.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/pkt_sched.h>
#include <linux/list.h>
#include <linux/reboot.h>
#ifdef SIOCETHTOOL
#include <linux/ethtool.h>
#endif
#ifdef NETIF_F_HW_VLAN_TX
#include <linux/if_vlan.h>
#endif

#define BAR_0		0
#define BAR_1		1
#define BAR_5		5
#define PCI_DMA_64BIT	0xffffffffffffffffULL
#define PCI_DMA_32BIT	0x00000000ffffffffULL

#include "kcompat.h"

struct e1000_adapter;

#include "e1000_hw.h"
#ifdef IANS
#include "base_comm.h"
#include "ans_driver.h"
#include "ans.h"
#endif
#ifdef IDIAG
#include "idiag_pro.h"
#endif

#if DBG
#define E1000_DBG(args...) printk(KERN_DEBUG "e1000: " args)
#else
#define E1000_DBG(args...)
#endif

#define E1000_ERR(args...) printk(KERN_ERR "e1000: " args)

#define E1000_MAX_INTR 10

/* Supported Rx Buffer Sizes */
#define E1000_RXBUFFER_2048  2048
#define E1000_RXBUFFER_4096  4096
#define E1000_RXBUFFER_8192  8192
#define E1000_RXBUFFER_16384 16384

/* How many Tx Descriptors do we need to call netif_wake_queue ? */
#define E1000_TX_QUEUE_WAKE	16
/* How many Rx Buffers do we bundle into one write to the hardware ? */
#define E1000_RX_BUFFER_WRITE	16

#define E1000_JUMBO_PBA      0x00000028
#define E1000_DEFAULT_PBA    0x00000030

#define AUTO_ALL_MODES       0

/* only works for sizes that are powers of 2 */
#define E1000_ROUNDUP(i, size) ((i) = (((i) + (size) - 1) & ~((size) - 1)))

/* wrapper around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer */
struct e1000_buffer {
	struct sk_buff *skb;
	uint64_t dma;
	unsigned long length;
	unsigned long time_stamp;
};

struct e1000_desc_ring {
	/* pointer to the descriptor ring memory */
	void *desc;
	/* physical address of the descriptor ring */
	dma_addr_t dma;
	/* length of descriptor ring in bytes */
	unsigned int size;
	/* number of descriptors in the ring */
	unsigned int count;
	/* next descriptor to associate a buffer with */
	unsigned int next_to_use;
	/* next descriptor to check for DD status bit */
	unsigned int next_to_clean;
	/* array of buffer information structs */
	struct e1000_buffer *buffer_info;
};

#define E1000_DESC_UNUSED(R) \
((((R)->next_to_clean + (R)->count) - ((R)->next_to_use + 1)) % ((R)->count))
#define E1000_RX_DESC_UNUSED(R) \
((((R)->next_to_clean + (R)->count) - ((R)->next_to_use)) % ((R)->count))

#define E1000_GET_DESC(R, i, type)	(&(((struct type *)((R).desc))[i]))
#define E1000_RX_DESC(R, i)		E1000_GET_DESC(R, i, e1000_rx_desc)
#define E1000_TX_DESC(R, i)		E1000_GET_DESC(R, i, e1000_tx_desc)
#define E1000_CONTEXT_DESC(R, i)	E1000_GET_DESC(R, i, e1000_context_desc)

/* board specific private data structure */

struct e1000_adapter {
#ifdef IANS
	void *iANSReserved;
	piANSsupport_t iANSdata;
	uint32_t ans_link;
	uint32_t ans_speed;
	uint32_t ans_duplex;
	uint32_t ans_suspend;
	IANS_BD_TAGGING_MODE tag_mode;
#endif
	struct timer_list watchdog_timer;
	struct timer_list phy_info_timer;
#ifdef CONFIG_PROC_FS
	struct list_head proc_list_head;
#endif
#ifdef NETIF_F_HW_VLAN_TX
	struct vlan_group *vlgrp;
#endif
	char *id_string;
	uint32_t bd_number;
	uint32_t rx_buffer_len;
	uint32_t part_num;
	uint32_t wol;
	uint16_t link_speed;
	uint16_t link_duplex;
	spinlock_t stats_lock;
	atomic_t irq_sem;

#ifdef ETHTOOL_PHYS_ID
	struct timer_list blink_timer;
	unsigned long led_status;
#endif

	/* TX */
	struct e1000_desc_ring tx_ring;
	uint32_t txd_cmd;
	uint32_t tx_int_delay;
	uint32_t tx_abs_int_delay;
	int max_data_per_txd;

	/* RX */
	struct e1000_desc_ring rx_ring;
	uint64_t hw_csum_err;
	uint64_t hw_csum_good;
	uint32_t rx_int_delay;
	uint32_t rx_abs_int_delay;
	boolean_t rx_csum;

	/* OS defined structs */
	struct net_device *netdev;
	struct pci_dev *pdev;
	struct net_device_stats net_stats;

	/* structs defined in e1000_hw.h */
	struct e1000_hw hw;
	struct e1000_hw_stats stats;
	struct e1000_phy_info phy_info;
	struct e1000_phy_stats phy_stats;

#ifdef IDIAG
	uint32_t diag_icr;
	struct e1000_desc_ring diag_tx_ring;
	struct e1000_desc_ring diag_rx_ring;
#endif

#ifdef E1000_COUNT_ICR
	uint64_t icr_txdw;
	uint64_t icr_txqe;
	uint64_t icr_lsc;
	uint64_t icr_rxseq;
	uint64_t icr_rxdmt;
	uint64_t icr_rxo;
	uint64_t icr_rxt;
	uint64_t icr_mdac;
	uint64_t icr_rxcfg;
	uint64_t icr_gpi;
#endif

	uint32_t pci_state[16];

	/* Semaphore for locking HW access vs. i2c-i8254x SMBUS driver */
	struct semaphore smbus_lock;

        int do_poll_watchdog; /* Click polling */
};
#endif /* _E1000_H_ */
