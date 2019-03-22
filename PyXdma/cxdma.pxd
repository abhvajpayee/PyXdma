cdef extern from "cxdma.h":
    ctypedef struct DEVICE:
        char *path;
        unsigned int address;
        void *map_base;
        void *virt_addr;
        
    int devinfo(DEVICE *device);
    int openDev(char *device);
    int closeDev(int fd);
    void* getBase(int fd);
    uint32_t read(void* virt_addr);