import PyXdma.xdma as xdma

xdma.devinfo()

pci = xdma.Pci(b'/dev/xdma0_control')
base = pci.getBase()
for i in range(4):
    virt_addr = base + (i<<8)
    res = pci.read(virt_addr)
    print(i, hex(virt_addr), hex(res))
for i in range(4):
    virt_addr = base + ((16+i)<<8)
    res = pci.read(virt_addr)
    print(i, hex(virt_addr), hex(res))
pci.close()