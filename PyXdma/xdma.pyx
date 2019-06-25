# distutils: sources = PyXdma/cxdma.c
# distutils: include_dirs = PyXdma/

cimport cxdma
from libc.stdlib cimport malloc, free
from libc.stdint cimport uint32_t, int64_t
from cython.view cimport array as cvarray
import numpy as np
cimport numpy as np
import cython
import ctypes

def devinfo():
    cdef cxdma.DEVICE* _c_device
    _c_device = <cxdma.DEVICE *> malloc(sizeof(cxdma.DEVICE)) 
    _c_device.path = "/dev/xdma0_control"
    for i in range(4):
        _c_device.address = 0
        cxdma.devinfo(_c_device)
        print('map_base : ', hex(<unsigned long> _c_device.map_base))
        print('virt_addr : ', hex(<unsigned long> _c_device.virt_addr))
    return 0


cdef class Pci:
    cdef int _fd;
    cdef unsigned long _base;
    def __init__(self, char* device):
        self._fd = cxdma.openDev(device)
        #print(self._fd)
    
    def close(self):
        cxdma.closeDev(self._fd)
        return 0
    
    def getBase(self):
        if self._fd != 0:
            self._base = <unsigned long> cxdma.getBase(self._fd, <void*> self._base)
        return self._base
    
    def read(self, unsigned long virtualAddress):
        cdef uint32_t result;
        if self._fd != 0:
            result = cxdma.readDev(<void *> virtualAddress)
        return result
    
cdef class Channel:
    cdef int _fd;
    cdef char* _device;
    
    def __init__(self, char *device):
        self._device = device;
        self._fd = cxdma.openChannel(device);
        
    def opened(self):
        return self._fd >= 0

    @cython.boundscheck(False)
    @cython.wraparound(False)    
    def write(self, np.ndarray[dtype=unsigned long, mode="c"] buff not None):
        size = (len(buff)*8)
        print(hex(size))
        
        cxdma.write_from_buffer(self._device, self._fd, <char*> &buff[0], size, 0);
        return 0
    
    @cython.boundscheck(False)
    @cython.wraparound(False)
    def read(self, int size):
        cdef np.ndarray[dtype=unsigned long, mode="c"] buff = np.zeros(size, dtype=np.uint64)
        #buff = np.zeros(size/8, dtype=np.uint64)
        #buff = np.arange(size, dtype=np.uint64)
        cxdma.read_to_buffer(self._device, self._fd, <char*> &buff[0], size*8, 0);
        return buff
        
    def close(self):
        return cxdma.closeChannel(self._fd);
