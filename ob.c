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
  char type;
  char id[MAX_ID_LEN];
  struct order *next;
  struct order *prev;
};

struct book{
  struct order * asks;
  struct order * bids;
  unsigned int clock,oldclock,target,nTrades;
  float buyPrice;
  float sellPrice;
};

int updatePricesSide(struct order ** side,float *old,unsigned int * target,unsigned int * tstamp,char c){
  unsigned int volume=0;
  unsigned int ivol;
  float ep = 0.0;
  struct order * curr = side[0];
  unsigned int count =0;
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
  struct order * rv;
  rv = (struct order *) malloc(sizeof(struct order));
  rv->price = price;
  rv->size = size;
  rv->type = type;
  rv->tstamp = tstamp;
  rv->next = 0;
  rv->prev = 0;
  strcpy(rv->id,id);
  struct order * o = rv;
  if(o->type=='R'){
    //fprintf(stdout,"Mem - alloc - %s-%c %08u %p\n",o->id,o->type,o->tstamp,o);
  }
  else{
    //fprintf(stdout,"Mem - alloc - %s %08u %p\n",o->id,o->tstamp,o);
  }
  return rv;
}

int printOrder(FILE *stream,struct order * p){
  fprintf(stream,"-- *** --\nOrder %s\n",p->id);
  fprintf(stream,"Time: %u\n",p->tstamp);
  fprintf(stream,"Price %f, volume %u\n",p->price,p->size);
  fprintf(stream,"Type %c\n",p->type);
  fprintf(stream,"Previous %p, Next %p\n-- *** --\n",p->prev,p->next);
  return 0;
}

int freeOrder(struct order * o){
  if(o->type=='R'){
    //fprintf(stdout,"Mem - free - %s-%c %08u %p\n",o->id,o->type,o->tstamp,o);
  }
  else{
    //fprintf(stdout,"Mem - free - %s %08u %p\n",o->id,o->tstamp,o);
  }
  free(o);
  return 0;
}

unsigned int freeBook(struct book * b){
  struct order * curr = b->bids;
  struct order * temp;
  unsigned int count = 0;
  if(!curr){
    return count;
  }
  while(curr){
    temp = curr;
    freeOrder(temp);
    curr = curr->next;
    count++;
  }
  curr = b->asks;
  if(!curr){
    return count;
  }
  while(curr){
    temp = curr;
    freeOrder(temp);
    curr = curr->next;
    count++;
  }
  return count;
}

struct order * getOrderFromStream(FILE * stream){
  if(feof(stream)) return 0;
  char line[MAX_LINE_LEN] = {'\0'};
  fgets(line,MAX_LINE_LEN,stream);
  //fprintf(stdout,"line:\n%s-- +++ --\n",line);
  char type,temp;
  float price;
  unsigned int size,tstamp;
  char id[MAX_ID_LEN] = {'\0'};
  if(sscanf(line,"%u %c %s %c %f %u\n",&tstamp,&temp,id,&type,&price,&size)<6){
    if(sscanf(line,"%u %c %s %u\n",&tstamp,&type,id,&size)<4) return 0;
    return newOrder(0.0,size,type,id,tstamp);
  }
  return newOrder(price,size,type,id,tstamp);
}

int resizeSide(struct order ** side,struct order * newOrder){
  //fprintf(stdout,"In resize mode resizing order %s\n",newOrder->id);
  struct order * curr = side[0];
  if(!curr) return 1;
  //fprintf(stdout,"There is at least one order on this side of the book\n");
  //fprintf(stdout,"looking for a match to %s\n",newOrder->id);
  //fprintf(stdout,"comparing %s\n",curr->id);
  if(!strcmp(curr->id,newOrder->id)){
    //fprintf(stdout,"Found a match!\n");
    curr->size -= MIN_ARG(newOrder->size,curr->size);
    //fprintf(stdout,"New size of %s is %u units.\n",curr->id,curr->size);
    if(!(curr->size)){
      //fprintf(stdout,"Freeing %s.\n",curr->id);
      side[0] = curr->next;
      freeOrder(curr);
    }
    return 0;
  }
  //fprintf(stdout,"%s did not match with %s\n",curr->id,newOrder->id);
  struct order * prev = curr;
  curr = curr->next;
  while(curr){
    //fprintf(stdout,"New iteration, comparing to %s\n",curr->id);
    if(!strcmp(curr->id,newOrder->id)){
      //fprintf(stdout,"Found a match!\n");
      curr->size -= MIN_ARG(newOrder->size,curr->size);
      //fprintf(stdout,"New size of %s is %u units.\n",curr->id,curr->size);
      if(!(curr->size)){
	//fprintf(stdout,"Freeing %s.\n",curr->id);
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

int printBook(struct book * b){
  fprintf(stdout,"   --- *** ---\n   Printing the order book\n");
  fprintf(stdout,"   Time in the book: %u\n",b->clock);
  fprintf(stdout,"   Asks:\n");
  printSide(&(b->asks));
  fprintf(stdout,"   Bids:\n");
  printSide(&(b->bids));
  fprintf(stdout,"   -- -- --\n");
  fprintf(stdout,"   Price to reach target volume of %u : Bying %.2f, Selling %.2f\n",b->target,b->buyPrice,b->sellPrice);
  fprintf(stdout,"   Traded volume %u units.\n",b->nTrades);
  fprintf(stdout,"   --- *** ---\n");
  return 0;
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
      //fprintf(stdout,"%s more competitive than %s - FAIL\n",i->id,i->next->id);
      return 1;
    }
    if(checkOrderType(i)){
      //fprintf(stdout,"%s has broken type",i->id);
      return 1;
    }
  }
  return 0;
}

int orderBookSanityCheck(struct book * orderBook){
  if((orderBook->bids)&&(orderBook->asks)){
    if((orderBook->bids->price) > (orderBook->asks->price)) return 1;
  }
  if(checkSideOrdering(&(orderBook->bids))) return 1;
  if(checkSideOrdering(&(orderBook->asks))) return 1;
  if(checkRidiculousOrderSizes(&(orderBook->bids))) return 1;
  if(checkRidiculousOrderSizes(&(orderBook->asks))) return 1;
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
    //fprintf(stdout,"Pointer to new order freed.\n");
    return 0;
  }
  struct order ** side = ((newOrder->type)=='S')?(&(orderBook->asks)):(&(orderBook->bids));
  struct order ** opposingSide = (!((newOrder->type)=='S'))?(&(orderBook->asks)):(&(orderBook->bids));
  int coef = ((newOrder->type)=='S')?-1:1;
  while(opposingSide[0] && (coef*((opposingSide[0])->price)<=coef*(newOrder->price))){
    // The new order can be at least partially filled
    //fprintf(stdout,"There is open interest in the opposing side of the book. Matching.\n");
    //fprintf(stdout,"Order %s is favourable to match %s. Price is %f.\n",(opposingSide[0])->id,newOrder->id,(opposingSide[0])->price);
    unsigned int tradeSize;
    tradeSize = MIN_ARG((newOrder->size),((opposingSide[0])->size));
    //tradeSize = (newOrder->size < (opposingSide[0])->size)?(newOrder->size):(opposingSide[0]->size);
    //fprintf(stdout,"Matching offers %s and %s. Reducing both by %u units.\n",opposingSide[0]->id,newOrder->id,tradeSize);
    //printOrder(stdout,opposingSide[0]);
    //printOrder(stdout,newOrder);
    orderBook->nTrades += tradeSize;
    newOrder->size -= tradeSize;
    opposingSide[0]->size -= tradeSize;
    //printOrder(stdout,opposingSide[0]);
    //printOrder(stdout,newOrder);
    if(!((opposingSide[0])->size)){
      //fprintf(stdout,"Order %s filled.\n",(opposingSide[0])->id);
      newOrder->next = (opposingSide[0])->next;
      opposingSide[0] = newOrder->next;
      newOrder->next=0;
    }
    if(!(newOrder->size)){
      //fprintf(stdout,"Order %s filled.\n",newOrder->id);
      // the order was filled and will be forgotten
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
  // Find the best offer in the order book that is still
  // worse than the new order
  struct order * worstBetter = side[0];
  //fprintf(stdout,"Adding the non-filled part of the order into the order book.\n");
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
  FILE * output = stdout;
  (void) output;
  FILE * input = stdin;
  if(argc==3){
    input = fopen(argv[2],"r");
  }
  struct book theBook;
  theBook.asks = 0;
  theBook.bids = 0;
  theBook.nTrades = 0;
  if(!(sscanf(argv[1],"%u",&(theBook.target)))){
    fclose(input);
    fprintf(stdout,"Unable to determine target.\n");
    return 10;
  }
  struct order *q;
  (void) q;
  unsigned int lineNum = 0;
  q = (struct order *) 1;
  while(!(feof(input))){
    q = getOrderFromStream(input);
    if(q){
      lineNum++;
      addNewOrder(&theBook,q);
      //fprintf(stdout,"order added %u\n",theBook.clock);
      updatePrices(&theBook);
      //fprintf(stdout,"prices updated %u\n",theBook.clock);
      if(theBook.clock==31849106)
	printBook(&theBook);
      if(orderBookSanityCheck(&theBook)){
	fprintf(stdout,"Broken order book!!!\n");
	printBook(&theBook);
	freeBook(&theBook);
	return 1;
      }
      //printBook(&theBook);
      //printOrder(stdout,q);
    }
  }
  if(argc==3){
    fclose(input);
  }
  //printBook(&theBook);
  freeBook(&theBook);
  fclose(input);
  return EXIT_SUCCESS;
}

