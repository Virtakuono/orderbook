#!/usr/bin/python

import copy

debugStop = 31552033+1
debugStart = 31552033-1
target = 200

def getLines(fn='pricer.in.txt',startTime=0,endTime=10**20):
    f = open(fn,'r')
    rv = f.readlines()
    f.close()
    startInd = 0
    endInd = len(rv)
    while(int(rv[startInd][:rv[startInd].find(' ')])<startTime):
        startInd += 1
    while (int(rv[endInd-1][:rv[endInd-1].find(' ')])>endTime) and endInd>0:
        endInd -= 1
    return rv[startInd:endInd]

def lineToOrder(line):
    rv = {}
    rv['tstamp'] = int(line[:line.find(' ')])
    if 'R' in line:
        rv['type'] = 'R'
        line = line[11:-1]
        rv['id'] = line[:line.find(' ')]
        line = line[line.find(' ')+1:]
        rv['vol'] = int(line)
        return rv
    else:
        line = line[11:-1]
        rv['id'] = line[:line.find(' ')]
        line = line[line.find(' ')+1:]
        rv['type'] = line[:line.find(' ')]
        line = line[line.find(' ')+1:]
        rv['price'] = float(line[:line.find(' ')])
        line = line[line.find(' ')+1:]
        rv['vol'] = int(line)
        return rv
    return rv

def listBids(offers):
    if offers:
        bestPrice = max([offer['price'] for offer in offers])
        keyl = lambda offer: '%08d%08d%s%s'%(int(1000*abs(offer['price']-bestPrice)),offer['ind'],len(offer['id']),offer['id'])
        return sorted(offers,key=keyl)
    return []

def listAsks(offers):
    if offers:
        bestPrice =min([offer['price'] for offer in offers])
        keyl = lambda offer: '%08d%08d%s%s'%(int(1000*abs(offer['price']-bestPrice)),offer['ind'],len(offer['id']),offer['id'])
        return sorted(offers,key=keyl)
    return []

def runOrderBook(lines):
    orders = [lineToOrder(line) for line in lines]
    bids = {}
    asks = {}
    bestBid = 0.0
    bestAsk = 9999999.9
    clock = 0
    count=0
    for order in orders:
        order['ind'] = count
        count += 1
        try:
            print('line: %d A %s %s %.2f %d'%(order['tstamp'],order['id'],order['type'],order['price'],order['vol']))
        except KeyError:
            print('line: %d %s %s %d'%(order['tstamp'],order['type'],order['id'],order['vol']))
        clock = max(order['tstamp'],clock)
        if order['type'] == 'R':
            #print('resize')
            if order['id'] in bids.keys():
                bids[order['id']]['vol'] -= order['vol']
                if bids[order['id']]['vol'] <= 0:
                    bids.pop(order['id'])
            if order['id'] in asks.keys():
                asks[order['id']]['vol'] -= order['vol']
                if asks[order['id']]['vol'] <= 0 :
                    asks.pop(order['id'])
        if order['type'] == 'B':
            #print('buy')
            bids[order['id']] = order
            bestBid = max(order['price'],bestBid)
        if order['type'] == 'S':
            #print('sell')
            asks[order['id']] = order
            bestAsk = min(order['price'],bestAsk)
        if clock>=debugStart:
            printOrderBook((bids,asks))
        '''
        if (len(bids.keys()) + len(asks.keys())):
            while bestBid >= bestAsk:
                #print('oogaooga')
                sortedBids = listBids([bids[key] for key in bids.keys()])
                sortedAsks = listAsks([asks[key] for key in asks.keys()])
                theBestBid = False
                theBestAsk = False
                if len(sortedBids):
                    theBestBid = sortedBids[0]
                if len(sortedAsks):
                    theBestAsk = sortedAsks[0]
                if (theBestAsk and theBestBid):
                    tradeSize = min(theBestBid['vol'],theBestAsk['vol'])
                    theBestBid['vol'] -= tradeSize
                    theBestAsk['vol'] -= tradeSize
                    if theBestBid:
                        if not (theBestBid['vol']):
                            bids.pop(theBestBid['id'])
                    if theBestAsk:
                        if not (theBestAsk['vol']):
                            asks.pop(theBestAsk['id'])
                    if len(bids.keys()):
                        bestBid = max([bids[key]['price'] for key in bids.keys()])
                    if len(asks.keys()):
                        bestAsk = min([asks[key]['price'] for key in asks.keys()])
            if len(bids.keys()):
                bestBid = max([bids[key]['price'] for key in bids.keys()])
            if len(asks.keys()):
                bestAsk = min([asks[key]['price'] for key in asks.keys()])
            if (clock > debugStart):
                print(' OrderBook time %d'%(clock,))
            printOrderBook((bids,asks))
        '''
    return (bids,asks)

def printOrderBook(bidsasks):
    print(' Printing order book')
    bids = copy.deepcopy(bidsasks[0])
    asks = copy.deepcopy(bidsasks[1])
    print('  Printing all bids.')
    for order in listBids([bids[key] for key in bids.keys()]):
        print('  %d %d %.2f %s'%(order['tstamp'],order['vol'],order['price'],order['id']))
    print('  Printing all asks.')
    for order in listAsks([asks[key] for key in asks.keys()]):
        print('  %d %d %.2f %s'%(order['tstamp'],order['vol'],order['price'],order['id']))

def allOrders(startTime=0,endTime=10**20):
    return [lineToOrder(foo) for foo in getLines(startTime=startTime, endTime=endTime)]

def orderLib(startTime=0,endTime=10**20):
    orders = allOrders(startTime=startTime,endTime=endTime)
    rv = {}
    c = 0
    for order in orders:
        if not (order['id'] in rv.keys()):
            rv[order['id']] = []
        rv[order['id']].append(order)
        c += 1
        print(order['id'])
    return rv

def getNonCancelledOrders(startTime=0,endTime=10**20):
    lib = orderLib(startTime=startTime,endTime=endTime)
    rv = {}
    for key in lib.keys():
        #print('Checking order %s'%(key,))
        vol = 0
        vol += lib[key][0]['vol']
        #print('  Initial volume %d'%(vol,))
        for foo in lib[key][1:]:
            vol -= foo['vol']
            #print('  Reduced by %d'%(foo['vol']))
        if vol>0:
            rv[key] = vol
    return rv

lines = getLines(endTime=debugStop )
printOrderBook(runOrderBook(lines))

