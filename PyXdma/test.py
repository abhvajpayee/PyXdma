import PyXdma.xdma as xdma

print(xdma.greeting())
print(xdma.devinfo())

pci = xdma.Pci(b'/dev/xdma0_control')
pci.close()