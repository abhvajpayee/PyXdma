typedef struct {    
    unsigned int  handle;
    void *dev;
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short bus;
	unsigned short slot;
	unsigned short devfn;
	unsigned char interrupt_pin;
	unsigned char interrupt_line;
	unsigned int irq;
	unsigned long bar_start[6];
	unsigned long bar_length[6];
} DEVICE;

typedef struct {
    DEVICE device[100];
	unsigned int  devcount;
} DEVICES;

typedef struct {
    unsigned long vma;
    int  devno;
    int size;
	unsigned long* kbuffer;
    void *km_handle;
} KBUFFER;

typedef struct {
	unsigned long PA_h;
	unsigned long PA_l;
	unsigned long HA_h;
	unsigned long HA_l;
	unsigned long next_bda_h;
	unsigned long next_bda_l;
	unsigned long length;
	unsigned long control;
	unsigned long status;
} bda_t;

typedef struct {
	unsigned long *bar0;
	unsigned long *bar1;
	int opened;
} pci_dev;

int bar_Map(int dev_id);
unsigned long status_read(void);
unsigned long command_read(void);
int command_write(unsigned long command);
int read_buff (unsigned long * buff, int ptr, int size);
int write_buff(unsigned long * buff, int ptr, int size);
int ext_RST(int b);
int bar_size(int b);

int find_device(DEVICES *devices);
int write_DMA(DEVICES *devices, volatile KBUFFER* buff, unsigned int barNo, unsigned int size);
int read_DMA(DEVICES *devices, volatile KBUFFER* buff, unsigned int barNo, unsigned int size);