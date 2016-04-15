#!/usr/bin/python

import os
import time

sizes = (1,200,10000)

for size in sizes:
    t1 = time.time()
    os.system('./ob %d pricer.in.txt > own.%d.out.txt'%(size,size))
    t2 = time.time()
    print('Pricer run for target size %d in %d seconds.'%(size,t2-t1))
    os.system('diff pricer.out.%d.txt own.%d.out.txt > diff.%d.txt'%(size,size,size))


