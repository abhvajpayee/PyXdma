import PyXdma.xdma as xdma
import numpy as np
import threading 
from multiprocessing import Process
import time
np.set_printoptions(formatter={'int':hex})
def getConfig(device = b'/dev/xdma0_control'):
    pci = xdma.Pci(device)
    base = pci.getBase()
    h2cChannels = []
    c2hChannels = []
    for i in range(4):
        virt_addr = base + (i<<8)
        res = pci.read(virt_addr)
        if (res >> 20) == 0x1fc:
            h2cChannels.append({'index': i, 'virt_addr': virt_addr, 'stream': True if ((res >> 12) & 0xf) == 0x08 else False})
    for i in range(4):
        virt_addr = base + ((16+i)<<8)
        res = pci.read(virt_addr)
        if (res >> 20) == 0x1fc:
            c2hChannels.append({'index': i, 'virt_addr': virt_addr, 'stream': True if ((res >> 12) & 0xf) == 0x08 else False})
    pci.close()
    return h2cChannels, c2hChannels

h2cChannels, c2hChannels = getConfig(device = b'/dev/xdma0_control')
#print(h2cChannels, c2hChannels)

def readChannelProcess(channel, size):
    readChannelDev = '/dev/xdma0_c2h_{0}'.format(c2hChannels[channel]['index']).encode('utf-8')
    #print(readChannelDev)
    readChannel  = xdma.Channel(readChannelDev)    
    #print("########################@@")
    if readChannel.opened():
        data = readChannel.read(size)
        readChannel.close()
        print(data)

def writeChannelProcess(channel, data):
    writeChannelDev = '/dev/xdma0_h2c_{0}'.format(h2cChannels[channel]['index']).encode('utf-8')
    #print(writeChannelDev)
    writeChannel = xdma.Channel(writeChannelDev)
    if writeChannel.opened():
        writeChannel.write(data)
        writeChannel.close()
        
if __name__ == "__main__": 
    
    data = np.fromfile('/home/abajpai/devel/linuxDriver/tests/data/shadata.bin', dtype=np.uint64)
    #print("###############", len(data), len(data)%8)
    #data = np.append(np.array([0xFFFF]*4, dtype=np.uint64), data) 
    #data = np.append(data, np.array([0x0]*4, dtype=np.uint64)) 
    #data = np.random.randint(2**64, size=int(1024*128), dtype=np.uint64)
    print(data)
    
    #writeChannelProcess(data)
    #readChannelProcess(len(data)) 
    # creating thread 

    disableWrite = False
    for i in range(2):
        plist = []
        for j in range(1):
            p1 = Process(target=readChannelProcess, args=(j, (256+32+4),))
            if not disableWrite:
                p2 = Process(target=writeChannelProcess, args=(j, data, ))
            p1.start() 
            if not disableWrite:
                p2.start() 
            plist.append(p1)
            if not disableWrite:
                plist.append(p2)
      
        for p in plist:
            p.join()
        print("") 
    print("Done!") 