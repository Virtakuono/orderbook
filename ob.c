#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef MAX_ARG
#define MAX_ARG(a,b) (a>b)?a:b
#endif

#ifndef MIN_ARG
#define MIN_ARG(a,b) (a>b)?b:a
#endif

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 40
#endif

#ifndef MAX_ID_LEN
#define MAX_ID_LEN 8
#endif

// Set to 1 to match bids into asks
// like in "real" exchanges

#ifndef ORDER_MATCHING
#define ORDER_MATCHING 0
#endif

struct order{
  unsigned int tstamp,size;
  int price;
  char type, id[MAX_ID_LEN];
  struct order *next;
};

struct book{
  struct order *asks, *bids;
  unsigned int clock,target,nTrades;
  int buyPrice,sellPrice;
};

int updatePricesSide(struct order ** side,int *old,unsigned int * target,unsigned int * tstamp,char c){
  unsigned int volume=0,ivol=0,count=0;
  int ep = 0;
  struct order *curr = side[0];
  while(curr){
    if(volume<target[0]){
      ivol = MIN_ARG(target[0]-volume,curr->size);
      volume += ivol;
      ep += ivol*(curr->price);
    }
    curr = curr->next;
    count++;
  }
  if(volume<target[0]){
    volume = 0;
    ep = 0;
  }
  if(ep!=old[0]){
    (ep)?fprintf(stdout,"%u %c %d.%02d\n",tstamp[0],c,ep/100,ep%100):fprintf(stdout,"%u %c NA\n",tstamp[0],c);
    old[0] = ep;
  }
  return count;
}

int updatePrices(struct book *b){
  int rv =0;
  rv += updatePricesSide(&(b->bids),&(b->sellPrice),&(b->target),&(b->clock),'S');
  rv += updatePricesSide(&(b->asks),&(b->buyPrice),&(b->target),&(b->clock),'B');
  return rv;
}

struct order * newOrder(int price,unsigned int size,char type, char * id,unsigned int tstamp){
  struct order *rv;
  rv = (struct order *) malloc(sizeof(struct order));
  rv->price = price;
  rv->size = size;
  rv->type = type;
  rv->tstamp = tstamp;
  rv->next = 0;
  strcpy(rv->id,id);
  return rv;
}

int printOrder(FILE *stream,struct order * p){
  fprintf(stream,"-- *** --\nOrder %s\n",p->id);
  fprintf(stream,"Time: %u\n",p->tstamp);
  fprintf(stream,"Price %.2f, volume %u\n",((float) (100*(p->price))),p->size);
  fprintf(stream,"Type %c\n",p->type);
  fprintf(stream,"Next %p\n-- *** --\n",p->next);
  return 0;
}

int freeOrder(struct order * o){
  if(o) free(o);
  return 0;
}

unsigned int freeSide(struct order **side){
  struct order *curr = side[0], *temp=0;
  unsigned int count = 0;
  if(!curr){
    return count;
  }
  while(curr){
    temp = curr->next;
    freeOrder(curr);
    curr = temp;
    count++;
  }
  return count;
}

unsigned int freeBook(struct book * b){
  return freeSide(&(b->bids))+freeSide(&(b->asks));
}

struct order * getOrderFromStream(FILE * stream){
  if(feof(stream)) return 0;
  char *line, *id;
  line = (char *) calloc(MAX_LINE_LEN,sizeof(char));
  id = (char *) calloc(MAX_ID_LEN,sizeof(char));
  fgets(line,MAX_LINE_LEN,stream);
  //fprintf(stdout,"line: %s ",line);
  char type=0,temp=0;
  int p1=0,p2=0;
  unsigned int size=0,tstamp=0;
  struct order *o;
  if(sscanf(line,"%u %c %s %c %d.%d %u",&tstamp,&temp,id,&type,&p1,&p2,&size)<6){
    if(sscanf(line,"%u %c %s %u",&tstamp,&type,id,&size)<4) return 0;
    o = newOrder(0,size,type,id,tstamp);
  }
  o = newOrder(p1*100+p2,size,type,id,tstamp);
  free(line);
  free(id);
  return o;
}

int resizeSide(struct order ** side,struct order * newOrder){
  struct order * curr = side[0];
  if(!curr) return 1;
  if(!strcmp(curr->id,newOrder->id)){
    curr->size -= MIN_ARG(newOrder->size,curr->size);
    if(!(curr->size)){
      side[0] = curr->next;
      freeOrder(curr);
    }
    return 0;
  }
  struct order * prev = curr;
  curr = curr->next;
  while(curr){
    if(!strcmp(curr->id,newOrder->id)){
      curr->size -= MIN_ARG(newOrder->size,curr->size);
      if(!(curr->size)){
	prev->next = curr->next;
	freeOrder(curr);
      }
      return 0;
    }
    prev = curr;
    curr = curr->next;
  }
  return 1;
}

unsigned int printSide(struct order ** side){
  struct order * curr = side[0];
  unsigned int count = 0;
  if(!curr){
    return count;
  }
  while(curr){
    fprintf(stdout,"  %u %u %.2f %s\n",curr->tstamp,curr->size,((float) 100*(curr->price)),curr->id);
    curr = curr->next;
    count++;
  }
  return count;
}

int compareTwoOrders(struct order * o1,struct order * o2){
  // Compare two orders, o1 and o2
  // Returns 1 if the first one is more competitive than the latter
  // Returns -1 if the latter is more competitive
  // Otherwise, returns 0
  if ((o1->type) != (o2->type)) return 0;
  int coef = ((o1->type)=='B')?1:-1;
  if(coef*(o1->price)>coef*(o2->price)) return 1;
  if(coef*(o1->price)<coef*(o2->price)) return -1;
  if((o1->tstamp)<(o2->tstamp)) return 1;
  if((o1->tstamp)>(o2->tstamp)) return -1;
  return 0;
}

int checkOrderType(struct order * o){
  if(o->type=='B') return 0;
  if(o->type=='S') return 0;
  return 1;
}

int printBook(struct book * b){
  fprintf(stdout,"OrderBook time %u\n Printing order book\n",b->clock);
  fprintf(stdout,"  Printing all bids.\n");
  printSide(&(b->bids));
  fprintf(stdout,"  Printing all asks.\n");
  printSide(&(b->asks));
  return 0;
}

int checkRidiculousOrderSizes(struct order ** side){
  unsigned int comp = 0;
  comp--;
  comp/=2;
  if(!(side[0])) return 0;
  if(!((side[0])->next)) return 0;
  for(struct order * i=side[0];i->next;i=i->next){
    if(i->size>comp) return 1;
  }
  return 0;
}

int checkSideOrdering(struct order ** side){
  if(!(side[0])) return 0;
  if(!((side[0])->next)) return 0;
  for(struct order * i=side[0];i->next;i=i->next){
    if(compareTwoOrders(i,i->next)<0){
      return 1;
    }
    if(checkOrderType(i)){
      return 1;
    }
  }
  return 0;
}

int priceSide(struct order * side,unsigned int target){
  unsigned int total=0;
  unsigned int inc;
  int price=0;
  while(side->next){
    inc = MIN_ARG(target-total,side->size);
    price += inc*(side->price);
    total += inc;
    if(total>=target) return price;
    side = side->next;
  }
  return 0.0;
}

int addNewOrder(struct book * orderBook, struct order * newOrder){
  orderBook->clock = newOrder->tstamp;
  if(newOrder->type=='R'){
    resizeSide(&(orderBook->asks),newOrder);
    resizeSide(&(orderBook->bids),newOrder);
    freeOrder(newOrder);
    return 0;
  }
  struct order ** side = ((newOrder->type)=='S')?(&(orderBook->asks)):(&(orderBook->bids));
  int coef = ((newOrder->type)=='S')?-1:1;
  if(ORDER_MATCHING){
    struct order ** opposingSide = (!((newOrder->type)=='S'))?(&(orderBook->asks)):(&(orderBook->bids));
    while(opposingSide[0] && (coef*((opposingSide[0])->price)<=coef*(newOrder->price))){
      unsigned int tradeSize =0 ;
      tradeSize = MIN_ARG((newOrder->size),((opposingSide[0])->size));
      orderBook->nTrades += tradeSize;
      newOrder->size -= tradeSize;
      opposingSide[0]->size -= tradeSize;
      if(!((opposingSide[0])->size)){
	newOrder->next = (opposingSide[0])->next;
	opposingSide[0] = newOrder->next;
	newOrder->next=0;
      }
      if(!(newOrder->size)){
	freeOrder(newOrder);
	return 0;
      }
    }
  }
  if(!(side[0])){
    side[0] = newOrder;
    newOrder->next=0;
    return 0;
  }
  if(!(compareTwoOrders(side[0],newOrder)+1)){
    newOrder->next=side[0];
    side[0]=newOrder;
    return 0;
  }
  struct order * worstBetter = side[0];
  while(worstBetter){
    if (!(worstBetter->next)){
      worstBetter->next = newOrder;
      return 0;
    }
    if(!(compareTwoOrders(worstBetter->next,newOrder)+1)){
      newOrder->next = worstBetter->next;
      worstBetter->next = newOrder;
      return 0;
    }
    worstBetter = worstBetter->next;
  }
  return 1;
}

int main(int argc, char * argv[]){
  if(argc<2){
    fprintf(stdout,"Usage %s target [inputfile]\n",argv[0]);
    return EXIT_SUCCESS;
  }
  FILE *output=stdout, *input=stdin;
  (void) output;
  if(argc==3){
    input = fopen(argv[2],"r");
  }
  struct book theBook;
  theBook.asks = 0;
  theBook.bids = 0;
  theBook.nTrades = 0;
  if(!(sscanf(argv[1],"%u",&(theBook.target)))){
    fclose(input);
    fprintf(stdout,"Usage %s target [inputfile]\n",argv[0]);
    return EXIT_SUCCESS;
  }
  struct order *q=(struct order *)1;
  while(!(feof(input))){
    q = getOrderFromStream(input);
    if(q){
      addNewOrder(&theBook,q);
      updatePrices(&theBook);
    }
  }
  if(argc==3){
    fclose(input);
  }
  freeBook(&theBook);
  fclose(input);
  return EXIT_SUCCESS;
}

