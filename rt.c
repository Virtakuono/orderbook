#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int main(){
  FILE* fh = fopen("oneline.txt","r");
 
 if (fh == NULL){
  printf("file does not exist");
  return 0;
 }
 
 const size_t line_size = 300;
 char* line = malloc(line_size);
 while (fgets(line, line_size, fh) != NULL)  {
   printf("%s",line);
 }
 free(line); 
}
