   
cdef extern from "pcieSMA.h":   
    ctypedef struct DEVICE:
        unsigned int  handle
        void  *dev
        unsigned short vendor_id
        unsigned short device_id
        unsigned short bus
        unsigned short slot
        unsigned short devfn
        unsigned char interrupt_pin
        unsigned char interrupt_line
        unsigned int irq
        unsigned long bar_start[6]
        unsigned long bar_length[6]
        
    ctypedef struct DEVICES:
        DEVICE device[100]
        unsigned int  devcount
        
    ctypedef struct KBUFFER:
        unsigned long vma
        int  devno
        int  size
        unsigned long* kbuffer
        void *km_handle
    
    int bar_Map(int dev_id)
    unsigned long status_read()
    unsigned long command_read()
    int command_write(unsigned long command)
    int read_buff (unsigned long * buff, int ptr, int size)
    int write_buff(unsigned long * buff, int ptr, int size)
    int ext_RST(int b)
    int bar_size(int b)
    
    int find_device(DEVICES *devices)
    
    int devopen(DEVICES *devices, unsigned int devNo)
    int devclose(DEVICES *devices, unsigned int devNo)
    int mapUserMemory(DEVICES *devices, unsigned int devNo, unsigned long * buff, unsigned int size)
    int allocateKernelMemory(DEVICES *devices, unsigned int devNo, KBUFFER* buff, unsigned int size)
    int deallocateKernelMemory(DEVICES *devices, KBUFFER* buff)
    int write_DMA(DEVICES *devices, KBUFFER* buff, unsigned int barNo, unsigned int size)
    int read_DMA(DEVICES *devices, KBUFFER* buff, unsigned int barNo, unsigned int size)
    int reset_DMA(DEVICES *devices, KBUFFER* buff)