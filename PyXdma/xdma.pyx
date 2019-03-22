# distutils: sources = PyXdma/cxdma.c
# distutils: include_dirs = PyXdma/

cimport cxdma
from libc.stdlib cimport malloc, free

def greeting():
    return 'hello World'

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
        if self._fd != 0:
            self._base = <unsigned long> cxdma.getBase(self._fd)
        print(self._fd, hex(self._base))
    
    def close(self):
        cxdma.closeDev(self._fd)
        return 0
    
    def getBase(self):
        self._base = <unsigned long> cxdma.getBase(self._fd)
        return 0
    