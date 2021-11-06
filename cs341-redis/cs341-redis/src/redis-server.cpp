#include "common.hpp"

using namespace std;

//2097152

#define LOG 0
#define MAXLINE 2097300
#define FIRST_CHUNK 250
#define CHUNK 100
#define MAXARGS 10

#define PRINT 0
#define START 2097000
#define END 2097200

int isCRLF(const char *ptr);
// int parse_bulk(char **raw_bulk, char *par_bulk);
// int parse_cmd(const char *raw_cmd, char argv[MAXARGS][MAXLINE], int *argc);
int redis_operation(char *argv[MAXARGS], char *sendBuff, int argc, int valueLength, int *sendLength);
char* toLower(char *s);
// Global map structure for saving key-value pairs
map<string, pair<string, int>> server;


int main(int argc, char *argv[])
{
  int listenfd = 0, connfd = 0, n = 0, req_num = 0;
  struct sockaddr_in serv_addr, client;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  // memset(sendBuff, '0', sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(6379);

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  listen(listenfd, 5);
  socklen_t len = sizeof(struct sockaddr_in);

  char error_msg[] = "-ERR\r\n";

  char reqBuff[MAXLINE];
  char sendBuff[MAXLINE];
  char *arglist[MAXARGS];
  int valueLength=0;
  char buf[MAXARGS];
  int argcnt;

  while(1) {
    n = 0;
    connfd = accept(listenfd, (struct sockaddr*)&client, &len);

    // memset(reqBuff, 0, sizeof(reqBuff));
    // memset(arglist, 0, sizeof(arglist));

    if((n = read(connfd, reqBuff, FIRST_CHUNK)) == 0) {
      close(connfd);
      printf("Connection closed.\n");
      break;
    }
    // printf("n = %d\n", n);
    printf("Read n = %d\n", n);
    if (n == FIRST_CHUNK){
      char *ptr = reqBuff + n;
      n = CHUNK;
      while (n == CHUNK) {
        n = read(connfd, ptr, CHUNK);
        ptr += n;
        // printf("n = %d\n", n);
      }
    }

    // if(1) {
    //   printf("\n\n  ##Read Start\n");
    //   for (int i=START; i<END; i++)
    //     printf("reqBuff[%d] = %c\n",i, reqBuff[i]);
    //   printf("\n  ##FINISH\n\n");
    // }
    // return 0;
    // printf("reqBuff %s\n", reqBuff);

    char *p = reqBuff;
    char *q = strstr(reqBuff, "\r\n");
    *q = '\0';
    argcnt = atoi(p+1);

    for (int i=0; i<argcnt; i++) {
      p = q+2;
      q = strstr(p, "\r\n");
      *q = '\0';
      int length = atoi(p+1);
      printf("length = %d\n", length);
      p = q + 2;
      q = p + length;
      *q = '\0';
      if (i == 2)
        valueLength = length;
      arglist[i] = p;

    }
    // if (0) {
    //   printf("\n\n  ##Value Start\n");
    //   for (int i=START; i<END; i++)
    //     printf("arglist[%d] = %c\n",i, arglist[2][i]);
    //   printf("\n  ##FINISH\n\n");
    // }

    bzero(sendBuff, sizeof(sendBuff));
    int sendLength;
    int sum = 0;
    if(redis_operation(arglist, sendBuff, argcnt, valueLength, &sendLength)) {
        // write(connfd, sendBuff, sendLength);
        printf("  ##Write Start\n");
        write(connfd, sendBuff, sendLength);
        printf("sum = %d\n", sum);

    }

    printf("One Iteration\n");
    close(connfd);
  }
  close(listenfd);

  return 0;
}

int isCRLF(const char *ptr) {
    if (*ptr != '\r')
        return 0;
    if (*(ptr + 1) != '\n')
        return 0;
    return 1;
}

/*
 * redis_operation - If the user has typed a redis operation,
 * Do proper action to it.
 *
 */
int redis_operation(char *argv[MAXARGS], char *sendBuff, int argc, int valueLength, int *sendLength)
{
  char *command = toLower(argv[0]);
  printf("Command %s\n", command);
  int retvalue = 1;

  map<string, pair<string, int>>::iterator it;

  char num_string[10];

  bzero(num_string, sizeof(num_string));

  if(!strcmp(command, "ping")) {
    if (argc == 1)
      strcpy(sendBuff, "+PONG\r\n");
    else {
      strcat(sendBuff, "$");
      sprintf(num_string, "%ld", strlen(argv[1]));
      strcat(sendBuff, num_string);
      strcat(sendBuff, "\r\n");
      strcat(sendBuff, argv[1]);
      strcat(sendBuff, "\r\n");
    }
    *sendLength = strlen(sendBuff);
  }

  // Case-sensitive
  else if(!strcmp(command, "get")){
    string key(argv[1]);
    if ((it = server.find(key)) == server.end()){
        strcat(sendBuff, "$-1\r\n");
        *sendLength = strlen(sendBuff);
    }
    else {
      int length = it->second.second;
      strcat(sendBuff, "$");
      sprintf(num_string, "%ld", length);
      strcat(sendBuff, num_string);
      strcat(sendBuff, "\r\n");
      int currLength = strlen(sendBuff);
      memcpy(sendBuff + currLength, it->second.first.data(), length);
      memcpy(sendBuff + currLength + length, "\r\n", 2);
      *sendLength = currLength + length + 2;
    }
  }

  else if(!strcmp(command, "set")) {
    if (argc == 1) {
      strcpy(sendBuff, "+OK\r\n");
    }
    else if (argc == 2){ // malloc
      char a2[] = "";
      string key(argv[1]);
      string value(a2);
      int num = 0;
      pair<string, int> valAndnum(value, num);

      if ((it = server.find(key)) != server.end()){
          server.erase(it->first);
      }
      server.insert(pair<string, pair<string, int>>(key, valAndnum));
      strcpy(sendBuff, "+OK\r\n");
    }
    else if (argc == 3) { // malloc
      string key(argv[1]);
      string value;
      int num = valueLength;
      value.resize(valueLength);
      memcpy((char *)value.data(), argv[2], num);
      pair<string, int> valAndnum(value, num);
      if ((it = server.find(key)) != server.end()){
          server.erase(it->first);
      }
      server.insert(pair<string, pair<string, int>>(key, valAndnum));
      strcpy(sendBuff, "+OK\r\n");
    }
    else {
      // Syntax Error
    }
    *sendLength = strlen(sendBuff);
  }

  else if(!strcmp(command, "strlen")) {

    if (server.empty()){
      strcat(sendBuff, ":0\r\n");
    }
    // server map has something
    else {
      string key(argv[1]);

      // Key not found
      if ((it = server.find(key)) == server.end()){
          // strcat(sendBuff, "$13\r\nKey Not Found\r\n");
          strcat(sendBuff, ":0\r\n");
      }
      else {
        strcat(sendBuff, ":");
        sprintf(num_string, "%ld", it->second.second);
        strcat(sendBuff, num_string);
        strcat(sendBuff, "\r\n");
      }
    }
    *sendLength = strlen(sendBuff);
  }

  else if(!strcmp(command, "del")) {
    int key_num = argc - 1;
    int cnt = 0;

      for (int i = 0; i < key_num; i++) {
        string key(argv[i+1]);
        if ((it = server.find(key)) == server.end())
          continue;
        server.erase(it->first);
        cnt++;
      }

      strcat(sendBuff, ":");
      sprintf(num_string, "%d", cnt);
      strcat(sendBuff, num_string);
      strcat(sendBuff, "\r\n");
      *sendLength = strlen(sendBuff);
  }

  else if(!strcmp(command, "exists")) {
    int key_num = argc - 1;
    int exist = 0;
    for (int i = 0; i < key_num; i++) {
      string key(argv[i+1]);
      if ((it = server.find(key)) == server.end())
        continue;
      exist++;
    }

    strcat(sendBuff, ":");
    sprintf(num_string, "%d", exist);
    strcat(sendBuff, num_string);
    strcat(sendBuff, "\r\n");
    *sendLength = strlen(sendBuff);
  }

  else {
    strcat(sendBuff, "-ERR unknown command ");
    strcat(sendBuff, "'");
    strcat(sendBuff, command);
    strcat(sendBuff, "'");
    strcat(sendBuff, "\r\n");
    retvalue = 0;
    *sendLength = strlen(sendBuff);
  }

  return retvalue;
}

char* toLower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}
