#!/usr/bin/python

import copy

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

def runOrderBook(lines):
    orders = [lineToOrder(line) for line in lines]
    bids = {}
    asks = {}
    bestBid = 0.0
    bestAsk = 9999999.9
    for order in orders:
        #print(order)
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
                bestBid = max([bids[key]['price'] for key in bids.keys()])
                bestAsk = min([asks[key]['price'] for key in asks.keys()])
                allBids = [bids[key] for key in bids.keys()]
                allAsks = [asks[key] for key in asks.keys()]
                bestBids = []
                bestAsks = []
                for iter in ((allBids,bestBids,bestBid),(allAsks,bestAsks,bestAsk)):
                    for offer in iter[0]:
                        if offer['price'] == iter[-1]:
                            iter[1].append(offer)
                firstBid = min([foo['tstamp'] for foo in bestBids])
                firstAsk = min([foo['tstamp'] for key in bestAsks])
                theBestBid = False
                theBestAsk = False
                for iter in ((bestBids,theBestBid,firstBid),(bestAsks,theBestAsk,firstAsk)):
                    for offer in iter[0]:
                        if offer['tstamp'] == iter[-1]:
                            iter[1] = offer
                tradeSize = min(theBestBid['vol'],theBestAsk['vol'])
                for iter in ((theBestBid,bids),(theBestAsk,asks)):
                    iter[0]['vol'] = max(iter[0]['vol']-tradeSize,0)
                if iter[0]['vol'] == 0:
                    iter[1].pop(iter[0]['id'])
                if len(bids.keys()):
                    bestBid = max([bids[key]['price'] for key in bids.keys()])
                if len(asks.keys()):
                    bestAsk = min([asks[key]['price'] for key in asks.keys()])
    return (bids,asks)

def printOrderBook(bidsasks):
    bids = copy.deepcopy(bidsasks[0])
    asks = copy.deepcopy(bidsasks[1])
    for side in (('Bids',bids,1),('Asks',asks,-1)):
        print('Printing %s',side[0])
        bestPrice = side[2]*max([side[2]*side[1][key]['price'] for key in side[1].keys()])
        while len(side[1].keys()):
            allSide = [side[1][key] for key in side[1].keys()]
            bestPrice = side[2]*max([side[2]*side[1][key]['price'] for key in side[1].keys()])
            bestOffers = []
            for offer in allSide:
                if offer['price'] == bestPrice:
                        bestOffers.append(offer)
            firstOffer = min(bestOffers[key]['tstamp'] for key in bestOffers.keys())
            theBestOffer = False
            for offer in [bestOffers[key] for key in bestOffers.keys()]:
                if offer['tstamp'] == firstOffer:
                        theBestOffer = offer
            print('%08d - %.2f - %05d - %s'%(theBestOffer['tstamp'],theBestOffer['price'],theBestOffer['vol'],theBestOffer['id']))
            side.pop(theBestOffer['id'])

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

lines = getLines(endTime=31552034 )
bids,asks = runOrderBook(lines)
bidkeys = bids.keys
targetVol = 200
hitVol = 0
hitPrice = 0.0

print('Printing all asks')
for key in asks.keys():
    print('id %s - vol %d - price %f'%(key,asks[key]['vol'],asks[key]['price']))


while hitVol<targetVol:
    prices = [asks[key]['price'] for key in asks.keys()]
    minPrice = min(prices)
    for key in asks.keys():
        if asks[key]['price'] == minPrice:
            vol = min(targetVol-hitVol,asks[key]['vol'])
            hitPrice += minPrice*vol
            hitVol += vol
            if asks[key]['vol']==vol:
                asks.pop(key)
        
#obs = getNonCancelledOrders(endTime=31849108)
#obs2 = getNonCancelledOrders(endTime=31552034)
