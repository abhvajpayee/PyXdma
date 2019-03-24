#include <time.h>

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
void* getBase(int fd, void* map_base);
uint32_t readDev(void* virt_addr);
int openChannel(char *device);
int closeChannel(int fd);
uint64_t getopt_integer(char *optarg);
uint64_t read_to_buffer(char *fname, int fd, char *buffer, uint64_t size, uint64_t base);
uint64_t write_from_buffer(char *fname, int fd, char *buffer, uint64_t size, uint64_t base);
//static int timespec_check(struct timespec *t);
//void timespec_sub(struct timespec *t1, struct timespec *t2);