from libc.stdint cimport uint32_t, int64_t;

cdef extern from "cxdma.h":
    ctypedef struct DEVICE:
        char *path;
        unsigned int address;
        void *map_base;
        void *virt_addr;
        
    int devinfo(DEVICE *device);
    int openDev(char *device);
    int closeDev(int fd);
    void* getBase(int fd, void* map_base);
    uint32_t readDev(void* virt_addr, uint32_t read_result);