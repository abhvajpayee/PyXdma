cimport cpci
from libc.stdlib cimport malloc, free
from cython.view cimport array as cvarray
import numpy as np
cimport numpy as np
import cython
import ctypes

cdef class pci:
    cdef cpci.DEVICES* _c_devices
    cdef cpci.KBUFFER* _c_kbuff
    
    def __cinit__(self):
        self._c_devices = <cpci.DEVICES *> malloc(sizeof(cpci.DEVICES))   
        cpci.find_device(self._c_devices)
        
    def info(self):
        print 'No of Devices:', self._c_devices.devcount
        for i in range(self._c_devices.devcount):
            print 'Device:', i
            print 'vendor_id:', self._c_devices.device[i].vendor_id
            print 'device_id:', self._c_devices.device[i].device_id
            print 'interrupt_pin:', self._c_devices.device[i].interrupt_pin
            print 'interrupt_line:', self._c_devices.device[i].interrupt_line
            for j in range(6):
                print '\tBAR:{} - Start: {} Length: {}'.format(j, 
                                                               hex(self._c_devices.device[i].bar_start[j]), 
                                                               hex(self._c_devices.device[i].bar_length[j]))
                
        return 0    
    
    def devopen(self, unsigned int devNo):
        ret = cpci.devopen(self._c_devices, devNo)
        return ret
    
    def devclose(self, unsigned int devNo):
        ret = cpci.devclose(self._c_devices, devNo)
        return ret    
        
    def allocateKernelMemory(self, unsigned int size, unsigned int devNo):
        cpci.devopen(self._c_devices, devNo)
        self._c_kbuff = <cpci.KBUFFER*> malloc(sizeof(cpci.KBUFFER))
        cpci.allocateKernelMemory(self._c_devices, devNo, self._c_kbuff, size)
        #print hex(self._c_kbuff.vma)
        #for i in range(10):
        #    print hex(buff[i])
        #print hex(self.vma)
        cdef unsigned long[:] view = <unsigned long[:size]> self._c_kbuff.kbuffer        
        return np.asarray(view)
        
    def deallocateKernelMemory(self):
        return cpci.deallocateKernelMemory(self._c_devices, self._c_kbuff)
    
    def writeDMA(self, size, unsigned int barNo):
        return cpci.write_DMA(self._c_devices, self._c_kbuff, barNo, size)
    
    def readDMA(self, size, unsigned int barNo):
        return cpci.read_DMA(self._c_devices, self._c_kbuff, barNo, size)
    
    def resetDMA(self):
        return cpci.reset_DMA(self._c_devices, self._c_kbuff)
    
    def status(self):
        return cpci.status_read()
    
    def command_read(self):
        return cpci.command_read()
    
    def command(self, unsigned long cmd):
        return cpci.command_write(cmd)
    
    def rstBAR(self, bar):
        if cpci.ext_RST(bar) == 0:
            if(bar == 0):
                print "DMA Resetted"
            elif (bar == 1):
                print "BAR1 Resetted"
    
    @cython.boundscheck(False)
    @cython.wraparound(False)
    def read(self, int ptr, int size):
        cdef np.ndarray[dtype=unsigned long, mode="c"] buff = np.zeros(size, dtype=np.uint64)
        buff = np.zeros(size, dtype=np.uint64)
        buff = np.arange(size, dtype=np.uint64)
        cpci.read_buff(&buff[0], ptr, size)
        return buff
    
    @cython.boundscheck(False)
    @cython.wraparound(False)
    def write(self, np.ndarray[dtype=unsigned long, mode="c"] buff not None, int ptr):
        size = len(buff)
        return cpci.write_buff(&buff[0], ptr, size)

    