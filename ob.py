#!/usr/bin/python

import copy

debugStop = 28845606+5+100000

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
    bestPrice = max(offer['price'])
    keyl = lambda offer: '%08d%s%s'%(int(1000*abs(offer['price']-bestPrice)),len(offer['id']),offer['id'])
    return sorted(offers,key=keyl)

def listAsks(offers):
    bestPrice =min(offer['price'])
    keyl = lambda offer: '%08d%s%s'%(int(1000*abs(offer['price']-bestPrice)),len(offer['id']),offer['id'])
    return sorted(offers,key=keyl)

def listOffers(offers)

def runOrderBook(lines):
    orders = [lineToOrder(line) for line in lines]
    bids = {}
    asks = {}
    bestBid = 0.0
    bestAsk = 9999999.9
    clock = 0
    for order in orders:
        try:
            print('line: %d A %s %s %.2f %d'%(order['tstamp'],order['id'],order['type'],order['price'],order['vol']))
        except KeyError:
            print('line: %d %s %s %d'%(order['tstamp'],order['type'],order['id'],order['vol']))
        clock = max(order['tstamp'],clock)
        if order['type'] == 'R':
            if order['id'] in bids.keys():
                bids[order['id']]['vol'] -= order['vol']
                if bids[order['id']]['vol'] <= 0:
                    bids.pop(order['id'])
            if order['id'] in asks.keys():
                asks[order['id']]['vol'] -= order['vol']
                if asks[order['id']]['vol'] <= 0 :
                    asks.pop(order['id'])
        if order['type'] == 'B':
            bids[order['id']] = order
            bestBid = max(order['price'],bestBid)
        if order['type'] == 'S':
            asks[order['id']] = order
            bestAsk = min(order['price'],bestAsk)
        if (len(bids.keys()) + len(asks.keys())):
            while bestBid >= bestAsk:
                #print('offsetting orders !!!')
                bestBid = max([bids[key]['price'] for key in bids.keys()])
                bestAsk = min([asks[key]['price'] for key in asks.keys()])
                allBids = [bids[key] for key in bids.keys()]
                allAsks = [asks[key] for key in asks.keys()]
                bestBids = []
                bestAsks = []
                for offer in allBids:
                    if offer['price'] == bestBid:
                        bestBids.append(offer)
                for offer in allAsks:
                    if offer['price'] == bestAsk:
                        bestAsks.append(offer)
                firstBidName = min([len(foo['id']) for foo in bestBids])
                firstAskName = min([len(foo['id']) for foo in bestAsks])
                theBestBid = False
                theBestAsk = False
                for offer in bestBids:
                    if offer['id'] == firstBid:
                        theBestBid = offer
                for offer in bestAsks:
                    if offer['id'] == firstAsk:
                        theBestAsk = offer
                if (theBestAsk and theBestBid):
                    tradeSize = min(theBestBid['vol'],theBestAsk['vol'])
                    for iter in ((theBestBid,bids),(theBestAsk,asks)):
                        iter[0]['vol'] = max(iter[0]['vol']-tradeSize,0)
                    if iter[0]['vol'] == 0:
                        iter[1].pop(iter[0]['id'])
                    if len(bids.keys()):
                        bestBid = max([bids[key]['price'] for key in bids.keys()])
                    if len(asks.keys()):
                        bestAsk = min([asks[key]['price'] for key in asks.keys()])
            if len(bids.keys()):
                bestBid = max([bids[key]['price'] for key in bids.keys()])
            if len(asks.keys()):
                bestAsk = min([asks[key]['price'] for key in asks.keys()])
            print(' OrderBook time %d'%(clock,))
            printOrderBook((bids,asks))
    return (bids,asks)

def printOrderBook(bidsasks):
    print(' Printing order book')
    bids = copy.deepcopy(bidsasks[0])
    asks = copy.deepcopy(bidsasks[1])
    print('  Printing all bids.')
    while (len(bids.keys())):
        bestBid = max([bids[key]['price'] for key in bids.keys()])
        allBids = [bids[key] for key in bids.keys()]
        bestBids = []
        for offer in allBids:
            if offer['price'] == bestBid:
                bestBids.append(offer)
        firstBid = min([foo['id'] for foo in bestBids])
        theBestBid = False
        for offer in bestBids:
            if offer['id'] == firstBid:
                theBestBid = offer
        if (theBestBid):
            print('  %d %d %.2f %s'%(theBestBid['tstamp'],theBestBid['vol'],theBestBid['price'],theBestBid['id']))
            bids.pop(theBestBid['id'])
    print('  Printing all asks.')
    while (len(asks.keys())):
        bestAsk = min([asks[key]['price'] for key in asks.keys()])
        allAsks = [asks[key] for key in asks.keys()]
        bestAsks = []
        for offer in allAsks:
            if offer['price'] == bestAsk:
                bestAsks.append(offer)
        firstAsk = min([foo['id'] for foo in bestAsks])
        theBestAsk = False
        for offer in bestAsks:
            if offer['id'] == firstAsk:
                theBestAsk = offer
        if (theBestAsk):
            print('  %d %d %.2f %s'%(theBestAsk['tstamp'],theBestAsk['vol'],theBestAsk['price'],theBestAsk['id']))
            asks.pop(theBestAsk['id'])

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
runOrderBook(lines)

