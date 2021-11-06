#include "common.hpp"

#define MAXLINE 2097300
#define FIRST_CHUNK 250
#define CHUNK 100
#define MAXARGS 10
#define LOG 0
#define TRUE 1
#define FALSE 0
#define DOCKTEST 1

int iscomm(const char *cmdline);
void save_substring(const char *cmdline, int argc, char **argv);
int make_argv(char *cmdline, char *argv[MAXARGS], ssize_t len, int *last_len);
int print_msg(char *raw_msg);

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    struct sockaddr_in serv_addr;
    struct hostent *host;

    if(DOCKTEST)
    {
      if((host = gethostbyname(argv[3])) == NULL) {
        printf("Server not found error\n");
        return 1;
      }
    }

    // For Debugging
    if(LOG)
    {
        printf("argc: %d\n", argc);
        printf("argv[0]: %s\n", argv[0]);
        printf("argv[1]: %s\n", argv[1]);
        printf("argv[2]: %s\n", argv[2]);
        printf("argv[3]: %s\n", argv[3]);
    }

    bzero(&serv_addr, sizeof(serv_addr));

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    if(DOCKTEST) {
      serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
    }
    else {
      serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    }
    serv_addr.sin_port = htons(6379);

    char *line = NULL;
    size_t len = 0;
    ssize_t r;

    char sendBuff[MAXLINE];
    char recvBuff[MAXLINE];
    char num_string[10];

    int loop_cnt = 0;

    while ((r = getline(&line, &len, stdin)) != -1) {
      if(LOG)
        printf("getline result: %s\n", line);
      if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
          if (LOG)
            printf("\n Error : Could not create socket \n");
          return 1;
      }
      if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
      {
          if (LOG)
            printf("\n Error : Connect Failed \n");
         return 1;
      }

      loop_cnt += 1;

      // For Debugging
      // printf("Retrieved line of length %zu\n", r);
      // printf("%s\n", line);

      // char argv[MAXARGS][MAXLINE];
      char *argv[MAXARGS];
      // memset(argv, '\0', sizeof(argv));
      int last_len = 0;
      int argc = make_argv(line, argv, r, &last_len);

      memset(sendBuff, '\0', sizeof(sendBuff));
      memset(num_string, '\0', sizeof(num_string));

      // *%d
      strcat(sendBuff, "*");
      sprintf(num_string, "%d", argc);
      strcat(sendBuff, num_string);
      strcat(sendBuff, "\r\n");

      for(int i = 0; i < argc-1; i++) {
        memset(num_string, '\0', sizeof(num_string));
        sprintf(num_string, "%ld", strlen(argv[i]));
        strcat(sendBuff, "$");
        strcat(sendBuff, num_string);
        strcat(sendBuff, "\r\n");
        strcat(sendBuff, argv[i]);
        strcat(sendBuff, "\r\n");
      }

      // This means there is no comment! Do it as normal!
      if(last_len == 0) {
        memset(num_string, '\0', sizeof(num_string));
        sprintf(num_string, "%ld", strlen(argv[argc-1]));
        strcat(sendBuff, "$");
        strcat(sendBuff, num_string);
        strcat(sendBuff, "\r\n");
        strcat(sendBuff, argv[argc-1]);
        strcat(sendBuff, "\r\n");
      }
      // This means there is a comment. Handle unusual characters!
      else {
        memset(num_string, '\0', sizeof(num_string));
        sprintf(num_string, "%d", last_len);
        strcat(sendBuff, "$");
        strcat(sendBuff, num_string);
        strcat(sendBuff, "\r\n");

        int currLength = strlen(sendBuff);
        memcpy(sendBuff + currLength, argv[argc-1], last_len);
        memcpy(sendBuff + currLength + last_len, "\r\n", 2);
      }

      if(LOG) {
        // Check whether sendBuff is correct or not
        printf("\n\n  ##SENDBUFF START \n\n");
        for (int i = 0; i < MAXLINE; i++)
          printf("%c", sendBuff[i]);
        printf("\n  ##FINISH\n\n");
      }

      write(sockfd, sendBuff, sizeof(sendBuff));

      /* Reading from Server */
      memset(recvBuff, 0, sizeof(recvBuff));
      int n = 0;

      // Make read work in very long input
      if((n = read(sockfd, recvBuff, FIRST_CHUNK)) == 0) {
        // Do nothing
      }

      if (n == FIRST_CHUNK){
        char *ptr = recvBuff + n;
        n = CHUNK;
        while (n == CHUNK) {
          n = read(sockfd, ptr, CHUNK);
          ptr += n;
          // printf("n = %d\n", n);
        }
      }

      // read(sockfd, recvBuff, sizeof(recvBuff));

      if(LOG) {
        // Check whether sendBuff is correct or not
        printf("\n\n  ##RECVBUFF START \n\n");
        for (int i = 0; i < MAXLINE; i++)
          printf("%c", recvBuff[i]);
        printf("\n  ##FINISH\n\n");
      }

      if (print_msg(recvBuff) == -1)
       printf("Parsed Error\n");

      close(sockfd);
    }

    free(line);

    return 0;
}

/* Utility Functions for supporting Redis Operations */

int iscomm(const char *cmdline) {
  char line[MAXLINE];
  strcpy(line, cmdline);

  char *subString;
  subString = strtok(line, "\""); // find the first double quote
  subString = strtok(NULL, "\"");   // find the second double quote

  if(!subString) {
    return 0;
  }

  return 1;
}

void save_substring(char *cmdline, int argc, char **argv) {
  // char line[MAXLINE];
  //
  // strcpy(line, cmdline);
  char *ptr = cmdline;

  ptr += 1;

  char *ptr2 = strchr(ptr, '\"');

  *ptr2 = '\0';

  argv[argc] = ptr;

  return;
}

int make_argv(char* cmdline, char *argv[MAXARGS], ssize_t len, int *last_len) {
  int argc = 0;
  char buf[MAXLINE];

  memset(buf, '\0', sizeof(buf));

  // strcpy(buf, cmdline);
  if(LOG)
  {
    printf("make_argv len: %ld\n", len);
    printf("strlen: %ld\n", strlen(cmdline));
  }

  memcpy(buf, cmdline, len);

  if(LOG)
    printf("buf init: %s\n", buf);

  // Detecting escape sequences
  char *temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'n')) {
    *temp = '\n';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == '0')) {
    *temp = '0';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'a')) {
    *temp = '\a';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'b')) {
    *temp = '\b';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'e')) {
    *temp = '\e';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'f')) {
    *temp = '\f';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'r')) {
    *temp = '\r';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 't')) {
    *temp = '\t';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == 'v')) {
    *temp = '\v';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == '\\')) {
    *temp = '\\';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == '\'')) {
    *temp = '\'';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  temp = strchr(buf, '\\');
  while((temp != NULL) && (temp + 1 != NULL) && (*(temp + 1) == '\"')) {
    *temp = '\"';
    temp += 1;
    *temp = '\0';
    temp += 1;
    strcat(buf, temp);
    len -= 1;
    temp = strchr(buf, '\\');
  }

  if(LOG)
    printf("buf: %s\n", buf);

  char *commptr = strchr(buf, '\"');

  int start_idx = 0;
  int end_idx = 0;

  if(commptr != NULL) {
    // comment always have space infront of it
    *(commptr-1) = '\0';
    start_idx = commptr - buf + 1;
    if(LOG)
    {
      printf("start_idx: %d\n", start_idx);
      printf("char at start_idx: %c\n", *(buf + start_idx));
    }

    for(int i = len - 1; i >= 0; i--)
    {
      if(buf[i] == '\"') {
        // printf("buf[%d]: %c\n", i, buf[i]);
        // printf("buf[%d]: %c\n", i-1, buf[i-1]);
        end_idx = i - 1;
        if(LOG)
        {
          printf("end_idx: %d\n", end_idx);
          printf("char at end_idx: %c\n", *(buf + end_idx));
        }
        *(buf + i) = '\0';
        break;
      }
    }
  }

  if(LOG)
    printf("commptr: %s\n", commptr);

  char *ptr = buf;

  if(LOG)
    printf("buf 2: %s\n", buf);

  char *token = strtok(buf, " \n");

  while(token != NULL)
  {
    // strcpy(argv[argc++], token);
    argv[argc++] = token;
    token = strtok(NULL, " \n");
  }

  // Special Case when set foo "Unusual Char Characters"
  if(commptr != NULL) {
    // Not Sure about this...
    if(LOG)
    {
      printf("end: %c\n", *(ptr + end_idx));
    }
    // memcpy(argv[argc], commptr + 1, end_idx - start_idx + 1);
    argv[argc] = commptr + 1;
    *last_len = end_idx - start_idx + 1;
    if(LOG)
      printf("last_len: %d\n", *last_len);
    argc += 1;
  }

  if(LOG)
  {
    printf("argv[0]: %s\n", argv[0]);
    printf("argv[1]: %s\n", argv[1]);
    printf("argv[2]: %s\n", argv[2]);
  }

  return argc;
}

int print_msg(char *raw_msg) {

    if(LOG)
      printf("\nprint_msg_result: ");

    char dataType = raw_msg[0];

    char *start_string = raw_msg + 1;
    char *end_string = strstr(raw_msg, "\r\n");
    if (end_string == NULL)
        return -1;
    *end_string = '\0';

    int length = atoi(start_string);

    switch(dataType) {
        case ':':
            printf("%d\n", length);
            break;
        case '$':
            *end_string = '\r';
            start_string = strstr(raw_msg, "\r\n") + 2;
            end_string = start_string + length;
            *end_string = '\0';
            // print by characters
            // and print newline
            for (int i = 0; i < length; i++)
              printf("%c", *(start_string + i));
            printf("\n");
            break;
        case '+':
            printf("%s\n", start_string);
            break;
        case '-':
            printf("%s\n", start_string);
            break;
        default:
            return -1;
    }

    return 0;

}
