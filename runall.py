#!/usr/bin/python

import os
import time

sizes = (1,200,10000)

print('Recompiling.')
os.system('gcc ob.c -g -o ob -Wall -Wextra')

for size in sizes:
    t1 = time.time()
    os.system('./ob %d pricer.in.txt > own.%d.out.txt'%(size,size))
    t2 = time.time()
    print('Pricer run for target size %d in %d seconds.'%(size,t2-t1))
    f1 = 'pricer.out.%d.txt'%(size,)
    f2 = 'own.%d.out.txt'%(size,)
    f3 = 'diff.%d.txt'%(size,)
    os.system('diff %s %s > %s'%(size,size,size))
    diffFile = open('diff.%d.txt'%(size),'r')
    diffLines = diffFile.readlines()
    diffFile.close()
    errorLog = []
    for line in diffLines:
        if ((('>' in line) or ('<' in line))):
            key = line[2:]
            key = key[:key.find(' ')]
            key = int(key)
            if len(errorLog)<200:
                if key not in errorLog:
                    errorLog.append(int(key))
    print('First erroneous time steps (discrepancies between %s and %s )'%(f1,f2))
    for foo in errorLog[:5]:
        print('Outputs differ for timestamp %d.'%(foo,))
    print('Doing post-processing')
    ownFile = open('own.%d.out.txt'%size)
    ownLines = ownFile.readlines()
    ownFile.close()
    ownLinesPP = [ownLines[0]]
    for ii in range(1,len(ownLines)):
        tprev = int(ownLines[ii-1][:ownLines[ii-1].find(' ')])
        tcurr = int(ownLines[ii][:ownLines[ii].find(' ')])
        if (tcurr != tprev):
            ownLinesPP.append('%s'%(ownLines[ii]))
    ownFilePP = open('own.%d.pp.txt'%(size,),'w')
    ownFilePP.writelines(ownLinesPP)
    ownFilePP.close()
    os.system('diff pricer.out.%d.txt own.%d.pp.txt > diff.%d.pp.txt'%(size,size,size))
    diffFile = open('diff.%d.pp.txt'%(size),'r')
    diffLines = diffFile.readlines()
    diffFile.close()
    errorLog = []
    for line in diffLines:
        if ((('>' in line) or ('<' in line))):
            key = line[2:]
            key = key[:key.find(' ')]
            key = int(key)
            if len(errorLog)<200:
                if key not in errorLog:
                    errorLog.append(int(key))
    print('First erroneous time steps in pp file')
    for foo in errorLog[:5]:
        print('Outputs differ for timestamp %d.'%(foo,))

        



print('Quitting. Bye.')

