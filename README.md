# c2redis

Example on how to implement a C Redis client.

https://redis.io/docs/reference/protocol-spec/

sysctl net.ipv4.tcp_tw_reuse=1 

gcc -o main main.c


##### example
```
...
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
  
  doCommand("*5\r\n$3\r\nset\r\n$4\r\nfoo2\r\n$1\r\nu\r\n$2\r\nex\r\n$2\r\n60\r\n");


  printf("\n+---------------------------------------+\n");
  return 0;
}
```

output:
```
+---------------------------------------+

out +PONG
 : 7

out +OK
 : 5
out $4
test
 : 10

out +OK


+---------------------------------------+ 
```
