#include <stdio.h>
#include "c2redis.h"

int main()
{

  
  printf("\n+---------------------------------------+\n");
  doPing();
  char key[]={"foo"};
  char value[]={"test"};
  
  if ( !doSET(key, value, "60") ) {
    printf("error");
  }

  if ( !doGET(key) ) {
    printf("error");
  }
  printf("\n+---------------------------------------+\n");
  //doCommand("*5\r\n$3\r\nset\r\n$4\r\nfoo2\r\n$1\r\nu\r\n$2\r\nex\r\n$2\r\n60\r\n");
  
  return 0;
}



