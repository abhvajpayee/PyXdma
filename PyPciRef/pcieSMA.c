
#include "pcieSMA.h"
#include <driver/pciDriver.h>
#include <lib/pciDriver.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <wchar.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <inttypes.h>

#define STATUS 0
#define COMMAND 1

#define RD_DMA 			20
#define WR_DMA 			23

#define BASE_DMA_UP		(0x0028 >> 3)
#define BASE_DMA_DOWN		(0x0050 >> 3)
#define INT_STATUS		(0x0008 >> 3)
#define INT_EN			(0x0010 >> 3)
#define INT_CTRL		(0x0080 >> 3)
#define INT_LATENCY		(0x0084 >> 3)
#define INT_ON			(0x0088 >> 3)
#define INT_OFF			(0x008c >> 3)
#define BRAM_SPACE		(0x8000 >> 3)

pd_device_t dev;
pd_kmem_t kmw, kmr;
pci_dev pci;
bda_t dma;

void transact_DMA(unsigned long *base, int op); 
void status_DMA(unsigned long *base, int op);

/********************************************** Driver Interface Functions ******************************************/

#define DEV "/dev/fpga"
    
int find_device(DEVICES *devices)
{
    char temp[50];
    int i, j, ret;
    int handle;
    pci_board_info info;
    
    devices->devcount = 0;
    for (i=0; i < 100; i++){
        sprintf( temp, "%s%d", DEV, i);
        //printf("searching for %s\n", temp);
        handle = open(temp, O_RDWR);
        //devices->handle[i] = handle;
        //printf("%d:%d\n", devices->handle[i], handle);
        if (handle < 0){
            //printf( "Device not present.\n" );
            break;
        }
        else{            
            printf(" Handle: %x\n", handle );
            ret = ioctl( handle, PCIDRIVER_IOC_PCI_INFO, &info );
            if (ret != 0)
                printf( " failed! (%d)\n", ret );
            else {
                devices->device[i].vendor_id     = info.vendor_id;
                devices->device[i].device_id     = info.device_id;
                devices->device[i].bus           = info.bus;
                devices->device[i].slot          = info.slot;
                devices->device[i].devfn         = info.devfn;
                devices->device[i].interrupt_pin = info.interrupt_pin;
                devices->device[i].irq           = info.irq;
                for(j=0; j<6; j++) {
                    devices->device[i].bar_start[j]     = info.bar_start[j];
                    devices->device[i].bar_length[j]    = info.bar_length[j];
                }
            }
            devices->device[i].handle = handle;
            devices->devcount = i+1;
        }
    }
    return 0;
}
    
int devopen(DEVICES *devices, unsigned int devNo)
{
    pd_device_t *dev = (pd_device_t *) malloc(sizeof(pd_device_t));    
    devices->device[devNo].dev = dev;
    return pd_open(devNo, dev); 
}

int devclose(DEVICES *devices, unsigned int devNo)
{
    pd_device_t *dev;
    int ret;
    dev = (pd_device_t *) devices->device[devNo].dev;
    ret = pd_close(dev);
    return ret;
}

   
int mapUserMemory(DEVICES *devices, unsigned int devNo, unsigned long * buff, unsigned int size)
{
    pd_device_t *dev;
    pd_umem_t umem_handle;
    int ret;
    
    printf("%d\n", size);
    dev = (pd_device_t *)devices->device[devNo].dev;
    printf("%s\n", dev->name);
    printf("%lx\n", *buff);
    pd_mapUserMemory(dev, buff, size, &umem_handle);
    printf("%d\n", ret);
    printf("%lx\n", *buff);
    return umem_handle.vma; 
}

  
int allocateKernelMemory(DEVICES *devices, unsigned int devNo, volatile KBUFFER* buff, unsigned int size)
{
    pd_device_t *dev;
    pd_kmem_t km_handle;
    
    dev = (pd_device_t *) devices->device[devNo].dev; 
    buff->kbuffer = (unsigned long *)pd_allocKernelMemory(dev, size, &km_handle);
    buff->vma = km_handle.pa;
    buff->devno = devNo;
    buff->size = size;
    buff->km_handle = &km_handle;
    return 0; 
}

 
int deallocateKernelMemory(DEVICES *devices, volatile KBUFFER* buff)
{
    pd_device_t *dev = (pd_device_t *) devices->device[buff->devno].dev;
    pd_kmem_t *km_handle = (pd_kmem_t*) buff->km_handle;
    pd_freeKernelMemory(km_handle);
    pd_close(dev);
    return 0; 
}

// External DMA write
int write_DMA(DEVICES *devices, volatile KBUFFER* buff, unsigned int barNo, unsigned int size)
{
    pd_device_t *dev;
    volatile unsigned long *bar0;
    unsigned long head, conf, add;
    
    dev = (pd_device_t *) devices->device[buff->devno].dev; 
    
    bar0 = pd_mapBAR(dev, 0);
    head = 0xAFAFAFAFAFAFAFAF;
    conf = ((0x00LL ^ (barNo << 1)) << 32) ^ size*2;
    add = ((buff->vma & 0xFFFFFFFF) << 32) ^ (devices->device[buff->devno].bar_start[barNo] & 0xFFFFFFFF);
    bar0[0] = head;
    bar0[1] = conf;
    bar0[2] = add;        
    sync();
    pd_unmapBAR(dev, 0, (void *)bar0);
    return 0;
}

// External DMA write
int reset_DMA(DEVICES *devices, volatile KBUFFER* buff)
{
    pd_device_t *dev;
    volatile unsigned long *bar0;
    unsigned long head, conf, add;
    
    dev = (pd_device_t *) devices->device[buff->devno].dev; 
    
    bar0 = pd_mapBAR(dev, 0);
    head = 0xAFAFAFAFAFAFAFAFL;
    conf = 0x0000010000000000L;
    
    add  = 0x0000000000000000L;
    bar0[0] = head;
    bar0[1] = conf;
    bar0[2] = add;        
    sync();
    pd_unmapBAR(dev, 0, (void *)bar0);
    return 0;
}


// External DMA write
int read_DMA(DEVICES *devices, volatile KBUFFER* buff, unsigned int barNo, unsigned int size)
{
    pd_device_t *dev;
    volatile unsigned long *bar0;
    unsigned long head, conf, add;
    
    dev = (pd_device_t *) devices->device[buff->devno].dev; 
    
    bar0 = pd_mapBAR(dev, 0);
    head = 0xAFAFAFAFAFAFAFAF;
    conf = ((0x01LL ^ (barNo << 1)) << 32) ^ size*2;
    add = ((buff->vma & 0xFFFFFFFF) << 32) ^ (devices->device[buff->devno].bar_start[barNo] & 0xFFFFFFFF);
    bar0[0] = head;
    bar0[1] = conf;
    bar0[2] = add;        
    sync();
    pd_unmapBAR(dev, 0, (void *)bar0);
    return 0;
}
// External DMA write
int write_ODMA(DEVICES *devices, unsigned int devNo, unsigned int barNo, unsigned long * buff, unsigned int size)
{
    pd_device_t dev;
    pd_kmem_t kmlocalbuff;
    volatile uint64_t *localbuff;
    int ret; 
    volatile unsigned long *bar0;
    unsigned long head, conf, add;
           
    ret = pd_open(devNo, &dev);
    if (ret != 0) {
        printf("failed\n");
        return -1;
    }
    //dev = devices->device[devNo].dev;
    bar0 = pd_mapBAR(&dev, 0);
    
    localbuff = (uint64_t*)pd_allocKernelMemory(&dev, size, &kmlocalbuff );
    memcpy((void*)localbuff, (void*)buff, sizeof(uint64_t)*size);    
    sync();
    
    head = 0xAFAFAFAFAFAFAFAF;
    conf = ((0x00LL ^ (barNo << 1)) << 32) ^ size*2;
    add = ((kmlocalbuff.pa & 0xFFFFFFFF) << 32) ^ (devices->device[devNo].bar_start[barNo] & 0xFFFFFFFF);
    bar0[0] = head;
    bar0[1] = conf;
    bar0[2] = add;
    //printf("%lx\n", head);
    //printf("%lx\n", conf);
    //printf("%lx\n", add);
    sync();
    pd_freeKernelMemory(&kmlocalbuff);
    pd_unmapBAR(&dev, 0, (void *)bar0);
    pd_close(&dev);
    return 0;
}

int read_ODMA(DEVICES *devices, unsigned int devNo, unsigned int barNo, unsigned long * buff, unsigned int size)
{
    pd_device_t dev;
    pd_kmem_t kmlocalbuff;
    volatile uint64_t *localbuff;
    int ret; 
    volatile unsigned long *bar0;
    unsigned long head, conf, add;
        
    ret = pd_open(devNo, &dev);
    if (ret != 0) {
        printf("failed\n");
        return -1;
    }
    bar0 = pd_mapBAR(&dev, 0);
    
    localbuff = (uint64_t*)pd_allocKernelMemory(&dev, size, &kmlocalbuff );
    
    head = 0xAFAFAFAFAFAFAFAFLL;
    conf = ((0x01LL ^ (barNo << 1)) << 32) ^ size*2;
    add = ((kmlocalbuff.pa & 0xFFFFFFFF) << 32) ^ (devices->device[devNo].bar_start[barNo] & 0xFFFFFFFF);
    bar0[0] = head;
    bar0[1] = conf;
    bar0[2] = add;
    //printf("%lx\n", head);
    //printf("%lx\n", conf);
    //printf("%lx\n", add);
    sync();
    
    memcpy((void*)buff, (void*)localbuff, sizeof(uint64_t)*size); 
    pd_freeKernelMemory(&kmlocalbuff);
    pd_unmapBAR(&dev, 0, (void *)bar0);
    pd_close(&dev);
    return 0;
}

unsigned long status_read(void)
{
    sync();
    return pci.bar1[STATUS];
    sync();
}

unsigned long command_read(void)
{
    sync();
    return pci.bar1[COMMAND];
    sync();
}

int command_write(unsigned long command)
{
    sync();
    pci.bar1[COMMAND] = command;
    sync();
    return 0;
}

int read_buff (unsigned long * buff, int ptr, int size)
{
    void *source, *sink;
    source = (void*) &pci.bar1[ptr];
    //buff = (unsigned long *) malloc(size*sizeof(unsigned long));
    sink = (void*) buff;
    sync();
    sync();
    memcpy( sink, source, size*sizeof(unsigned long) );
    sync();
    sync();
    return 0;
}

int write_buff(unsigned long * buff, int ptr, int size)
{
    void *source, *sink;
    sink = (void*) &pci.bar1[ptr];
    source = (void*) buff;
    sync();
    sync();
    memcpy( sink, source, size*sizeof(unsigned long) );
    sync();
    sync();
    return 0;
}

int bar_Map(int dev_id)
{
	int ret;
	// Opening PCI device identified by X(=0) in device file name /dev/fpgaX,
	printf("\nTrying device %d ....", dev_id);
	ret = pd_open(dev_id, &dev);
	if(ret != 0) {
		printf("failed\n");
		return -1;
	}
	printf("Device OK\n");

	// Mapping BAR area of device into user space
	pci.bar0 = pd_mapBAR(&dev, 0 );
	pci.bar1 = pd_mapBAR(&dev, 1 );
	printf("BAR0 physical address: %lx\n", (long unsigned int)pci.bar0);
	printf("BAR1 physical address: %lx\n", (long unsigned int)pci.bar1);
	return 0;
}

int bar_size(int b){
	return pd_getBARsize(&dev, b);
}

// External (Python) Reset
int ext_RST(int b){
	sync();
	if(b == 0)	//BAR0
		memset( pci.bar0, 0x0a, 0xfff );
	else if(b == 1)	//BAR1
		memset( pci.bar1, 0x00, 0xfff );
	sync();
	sleep(0.01);
	return 0;
}

/******************************************************* DMA Functions ****************************************************/



// Local send DMA data
void transact_DMA(unsigned long *base, int op) 
{
    if(op == WR_DMA){
        base[0] = ((dma.PA_l 		<< 32) 	| (dma.PA_h 		& 0x00000000ffffffff));
        base[1] = ((dma.HA_l 		<< 32) 	| (dma.HA_h 		& 0x00000000ffffffff));
        base[2] = ((dma.next_bda_l 	<< 32) 	| (dma.next_bda_h 	& 0x00000000ffffffff));
        base[3] = ((dma.control    	<< 32) 	| (dma.length     	& 0x00000000ffffffff));
    }
    else if(op == RD_DMA){
        base[0] = ((dma.PA_h 		<< 32) 	| (base[0]        	& 0x00000000ffffffff));
        base[1] = ((dma.HA_h 		<< 32) 	| (dma.PA_l 		& 0x00000000ffffffff));
        base[2] = ((dma.next_bda_h 	<< 32) 	| (dma.HA_l 		& 0x00000000ffffffff));
        base[3] = ((dma.length    	<< 32) 	| (dma.next_bda_l 	& 0x00000000ffffffff));
        base[4] = ((base[4] & 0xffffffff00000000) | (dma.control 	& 0x00000000ffffffff));
    }
}

// Local DMA status check
void status_DMA(unsigned long *base, int op)
{
    if(op == WR_DMA){
        while((base[4] & 0x00000000ffffffff) != 0x00000001);
        dma.status =  (base[4] & 0x00000000ffffffff);
    }
    else if(op == RD_DMA){
        while((base[4] >> 32) != 0x00000001);
        dma.status =  (base[4] >> 32);
    }
}
