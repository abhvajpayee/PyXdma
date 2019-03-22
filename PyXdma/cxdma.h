typedef struct { 
    char *path;
    unsigned int address;
    void *map_base;
    void *virt_addr;
    unsigned int h2cChannels[4];
    unsigned int h2cstreamEnable[4];
    unsigned int c2hChannels[4];
    unsigned int c2hstreamEnable[4];
} DEVICE;

int devinfo(DEVICE *device);
int openDev(char *device);
int closeDev(int fd);
void* getBase(int fd);
uint32_t read(void* virt_addr);