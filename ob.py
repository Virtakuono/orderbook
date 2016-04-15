#!/usr/bin/python

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
    return (bids,asks)
    

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
