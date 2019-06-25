import PyXdma.xdma as xdma
import numpy as np
import threading 
from multiprocessing import Process
import time
from shahdl import SHA256Ref
from random import randrange
from pyhdllib import AXI4Pkt
import modregistry


DATA  = 64
WORD = 8

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
    print(readChannelDev)
    readChannel  = xdma.Channel(readChannelDev)    
    print("########################@@")
    if readChannel.opened():
        data = readChannel.read(size)
        readChannel.close()
        return data

def writeChannelProcess(channel, data):
    writeChannelDev = '/dev/xdma0_h2c_{0}'.format(h2cChannels[channel]['index']).encode('utf-8')
    #print(writeChannelDev)
    writeChannel = xdma.Channel(writeChannelDev)
    if writeChannel.opened():
        writeChannel.write(data)
        writeChannel.close()
        
if __name__ == "__main__": 


    npData = np.array([], dtype = np.uint64)
    pktid = randrange(2**16)
    src = 0x0
    dest = 1

    packet = AXI4Pkt.addHeader(np.array([32], dtype = np.uint64), src = src, dest = dest, algo = modregistry.RANDOM,
                               mode = modregistry.GENBITS, pktid = pktid, DATA = 128 + 64)
    packet = np.append(packet, np.zeros(4, dtype = np.uint64))
    npData = np.append(npData, packet)

    npData = np.append(npData, AXI4Pkt.addHeader(np.zeros(32, dtype = np.uint64),
                                                 src = src, dest = dest, algo = modregistry.RANDOM, mode = modregistry.FLUSH,
                                                 pktid = pktid, DATA = DATA*WORD))

    print(npData)
    npData = (npData << 32) ^ (npData >> 32)
    npData = npData.byteswap()
    print(npData)
    d = npData.tobytes()
    for i in d:
        print(hex(i))


    startTime = time.time()
    #writeChannelProcess(3, npData)
    #data = readChannelProcess(3, (256+32+4)) 
    stopTime = time.time()
    diff = stopTime - startTime
    print("Time Taken : ", diff, ' sec')
    #data = data.byteswap()
    #data = (data << 32) ^ (data >> 32)
    #print(data)
    
    print("Done!") 