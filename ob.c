#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_ID_LEN
#define MAX_ID_LEN 50
#endif

struct order{
  float price;
  unsigned int size;
  char type;
  char id[MAX_ID_LEN];
  struct order *next;
};

struct order * resizeOrder(char * id, unsigned int size){
  struct order * rv = calloc(sizeof(struct order),1);
  strcpy(id,rv->id);
  rv->size = size;
  return rv;
};

struct order * newOrder(float price, unsigned int size, char type, char * id){
  struct order * rv = calloc(sizeof(struct order),1);
  printf("moi");
  strcpy(id,rv->id);
  rv->size = size;
  rv->type = type;
  rv->price = price;
  return rv;
}

struct book{
  struct order * asks;
  struct order * bids;
};

int resizeSide(struct order * side,struct order * newOrder){
  if(!side) return 0;
  struct order * curr=side;
  struct order * prev=0;
  while((curr->next)){
    if (!(strcmp(curr->id,newOrder->id))){
      curr->size -= newOrder->size;
      if(!(curr->size)){
	prev->next=curr->next;
	free(curr);
      }
      return 0;
    }
    prev = curr;
    curr = curr->next;
  }
  return 1;
}

int addNewOrder(struct book * orderBook, struct order * newOrder){
  if(newOrder->type=='R')
    return resizeSide(orderBook->asks,newOrder)?resizeSide(orderBook->bids,newOrder):1;
  struct order ** side = ((newOrder->type)=='S')?(&(orderBook->asks)):(&(orderBook->bids));
  struct order ** opposingSide = (!((newOrder->type)=='S'))?(&(orderBook->asks)):(&(orderBook->bids));
  int coef = ((newOrder->type)=='S')?-1:1;
  while(opposingSide[0] && (coef*((opposingSide[0])->price)<=coef*(newOrder->price))){
    // The new order can be at least partially filled
    printf("moiggeli2\n");
    unsigned int tradeSize;
    tradeSize = (newOrder->size < (opposingSide[0])->size)?(newOrder->size):(opposingSide[0]->size);
    newOrder->size -= tradeSize;
    opposingSide[0] -= tradeSize;
    if(!((opposingSide[0])->size)){
      newOrder->next = (opposingSide[0])->next;
      opposingSide[0] = (opposingSide[0])->next;
      free(newOrder->next);
      newOrder->next = 0;
    }
    if(!(newOrder->size)){
      // the order was filled and will be forgotten
      free(newOrder);
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
  struct order * bestWorse = side[0];
  while ((bestWorse->next)&&(coef*(bestWorse->next->price)<coef*(newOrder->price)))
    bestWorse = bestWorse->next;
  newOrder->next = bestWorse->next;
  bestWorse->next = newOrder;
  return 0;
}

int main(){
  printf("moimoi2\n");
  struct book theBook;
  theBook.asks = 0;
  theBook.bids = 0;
  printf("moimoi1\n");
  struct order p;
  strcpy(p.id,"moimoi");
  p.price = 19.3;
  p.size = 4;
  p.type = 'B';
  addNewOrder(&theBook,&p);
  return EXIT_SUCCESS;
}

