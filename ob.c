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
#define MAX_ID_LEN 10
#endif

struct order{
  unsigned int tstamp,size;
  float price;
  char type, id[MAX_ID_LEN];
  struct order *next, *prev;
};

struct book{
  struct order *asks, *bids;
  unsigned int clock,oldclock,target,nTrades;
  float buyPrice,sellPrice;
};

int updatePricesSide(struct order ** side,float *old,unsigned int * target,unsigned int * tstamp,char c){
  unsigned int volume=0,ivol=0,count=0;
  float ep = 0.0;
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
    volume = 0.0;
    ep = 0.0;
  }
  if(ep!=old[0]){
    (ep!=0)?fprintf(stdout,"%u %c %.2f\n",tstamp[0],c,ep):fprintf(stdout,"%u %c NA\n",tstamp[0],c);
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

struct order * newOrder(float price,unsigned int size,char type, char * id,unsigned int tstamp){
  struct order *rv;
  rv = (struct order *) malloc(sizeof(struct order));
  rv->price = price;
  rv->size = size;
  rv->type = type;
  rv->tstamp = tstamp;
  rv->next = 0;
  rv->prev = 0;
  strcpy(rv->id,id);
  return rv;
}

int freeOrder(struct order * o){
  free(o);
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
  char line[MAX_LINE_LEN] = {'\0'}, id[MAX_ID_LEN] = {'\0'};
  fgets(line,MAX_LINE_LEN,stream);
  //fprintf(stdout,"line:\n%s-- +++ --\n",line);
  char type=0,temp=0;
  float price=0.0;
  unsigned int size=0,tstamp=0;
  if(sscanf(line,"%u %c %s %c %f %u",&tstamp,&temp,id,&type,&price,&size)<6){
    if(sscanf(line,"%u %c %s %u",&tstamp,&type,id,&size)<4) return 0;
    return newOrder(0.0,size,type,id,tstamp);
  }
  return newOrder(price,size,type,id,tstamp);
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
    fprintf(stdout,"   No orders.\n");
    return count;
  }
  while(curr){
    fprintf(stdout,"   Price %.5f   size %05u  %08u   id %s\n",curr->price,curr->size,curr->tstamp,curr->id);
    curr = curr->next;
    count++;
  }
  return count;
}

int compareTwoOffers(struct order * o1,struct order * o2){
  // Compare two offers, o1 and o2
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
    if(compareTwoOffers(i,i->next)<0){
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
  float price=0.0;
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
  orderBook->oldclock = orderBook->clock;
  orderBook->clock = newOrder->tstamp;
  if(newOrder->type=='R'){
    resizeSide(&(orderBook->asks),newOrder);
    resizeSide(&(orderBook->bids),newOrder);
    freeOrder(newOrder);
    return 0;
  }
  struct order ** side = ((newOrder->type)=='S')?(&(orderBook->asks)):(&(orderBook->bids));
  struct order ** opposingSide = (!((newOrder->type)=='S'))?(&(orderBook->asks)):(&(orderBook->bids));
  int coef = ((newOrder->type)=='S')?-1:1;
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
  // The new order was not filled
  if(!(side[0])){
    side[0] = newOrder;
    newOrder->next=0;
    return 0;
  }
  if(coef*(side[0]->price)<coef*(newOrder->price)){
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
    if(coef*(worstBetter->next->price)<coef*(newOrder->price)){
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

