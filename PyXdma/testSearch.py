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


    Version = np.frombuffer(b'0.01', dtype=np.uint32)
    hashPrevBlock = 0xc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedf
    hashMerkleRoot = 0xA0A1A2A3A4A5A6A7A8A9AaAbAcAdAeAfB0B1B2B3B4B5B6B7B8B9BaBbBcBdBeBf
    Time = int(time.time()) & 0xFFFFFFFF
    Bits = 25
    Nonce = 0x51525354
    msg = SHA256Ref.setBlockHeader(Version, hashPrevBlock, hashMerkleRoot, Time, Bits, Nonce)
    #print(msg)
    npData = np.array([], dtype = np.uint64)
    
    pktid = randrange(2**16)
    src = 0x0
    dest = 1
    paddedmsg = SHA256Ref.pad(msg)
    data = np.frombuffer(paddedmsg, dtype= '>u8') 
    data = (data << 32) ^ (data >> 32)

    packet = AXI4Pkt.addHeader(data, src = src, dest = dest, algo = modregistry.SHA256,
                               mode = modregistry.HASH_POW, pktid = pktid, DATA = DATA*WORD)
    npData = np.append(npData, packet)

    npData = np.append(npData, AXI4Pkt.addHeader(np.zeros(32, dtype = np.uint64),
                                                 src = src, dest = dest, algo = modregistry.SHA256, mode = modregistry.FLUSH,
                                                 pktid = pktid, DATA = DATA*WORD))
    npData = (npData << 32) ^ (npData >> 32)
    npData = npData.byteswap() #npData.view(np.dtype('>u8'))
    #print(npData)

    #data = np.fromfile('/home/abajpai/devel/linuxDriver/tests/data/shadata.bin', dtype=np.uint64)
    #print(data)

    #print("###############", len(data), len(data)%8)
    #data = np.append(np.array([0xFFFF]*4, dtype=np.uint64), data) 
    #data = np.append(data, np.array([0x0]*4, dtype=np.uint64)) 
    #data = np.random.randint(2**64, size=int(1024*128), dtype=np.uint64)
    #print('!!!!!!!!!!!!!!!')
    #print(data)
    #print('!!!!!!!!!!!!!!!')
    startTime = time.time()
    writeChannelProcess(2, npData)
    #writeChannelProcess(1, data)
    data = readChannelProcess(2, (256+32+4)) 
    stopTime = time.time()
    diff = stopTime - startTime
    print("Time Taken : ", diff, ' sec')
    data = data.byteswap()
    data = (data << 32) ^ (data >> 32)
    hashes = data[4]*2
    print('done ', hashes/1000000, ' million hashes in ', diff, ' seconds')
    print(hashes/diff*512/1000000, 'megabit/sec')
    print("hashe rate = ", hashes/diff/1000000, ' million hashes/sec')
    # creating thread 

    '''disableWrite = False
    for i in range(2):
        plist = []
        p1 = Process(target=readChannelProcess, args=(1, (256+32+4),))
        if not disableWrite:
            p2 = Process(target=writeChannelProcess, args=(1, data, ))
        p1.start() 
        if not disableWrite:
            p2.start() 
        plist.append(p1)
        if not disableWrite:
            plist.append(p2)
      
        for p in plist:
            p.join()
        print("")''' 
    print("Done!") 