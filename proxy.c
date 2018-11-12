#include "csapp.h"
#include <stdio.h>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void thread(int *arg);
int request(char *res, char *host, int client, char *str, char *port);
int response(int server, int client, char *content);

// DONE
int main(int argc, char *argv[]) {
  struct sockaddr_in client;
  int len = sizeof(client);
  int servfd;

  if (argc != 2) {
    printf("usage: ./proxy <port>\n");
  }

  servfd = Open_listenfd(argv[1]);
  while (1) {
    pthread_t tid;
    int *connfdp = Malloc(sizeof(int));
    *connfdp = -1;
    *connfdp = Accept(servfd, (SA *)&client, (socklen_t *)&len);
    Pthread_create(&tid, NULL, (void *)thread, (void *)connfdp);
  }
  return 0;
}

void thread(int *arg) {
  Pthread_detach(Pthread_self());

  int server = -1;
  int client = (int)(*arg);

  char req[MAXLINE], host[MAXLINE], port[MAXLINE], res[MAXLINE];

  int val = request(res, host, client, req, port);

  printf("%s:%s\n", host, port);
  fflush(stdout);
  fflush(stdout);

  server = Open_clientfd(host, port);
  Rio_writen(server, req, strlen(req));
  char payload[MAX_OBJECT_SIZE];
  response(server, client, payload);

  if (client) {
    if (client >= 0) {
      Close(client);
    }
  }

  if (server) {
    if (server >= 0) {
      Close(server);
    }
  }

  return;
}

int request(char *res, char *host, int client, char *str, char *port) {
  char tmp[MAXLINE];

  rio_t rio_client;

  Rio_readinitb(&rio_client, client);
  if (Rio_readlineb(&rio_client, tmp, MAXLINE) == -1) {
    return -1;
  }

  char url[MAXLINE], prot[MAXLINE], type[MAXLINE], port_host[MAXLINE],
      vers[MAXLINE];

  strcpy(res, "/");
  sscanf(tmp, "%s %s %s", type, url, vers);
  sscanf(url, "%[^:]://%[^/]%s", prot, port_host, res);

  char *tmpstring;
  char *rest = port_host;
  char *temphost = strtok_r(rest, ":", &rest);
  char *tempport = strtok_r(rest, ":", &rest);

  if (tempport == NULL) {
    tempport = "80";
  } else {
    strcpy(port, tempport);
  }

  strcpy(host, temphost);

  char *getstr[MAXLINE];
  sprintf(getstr, "%s %s ", type, res);
  strcpy(str, getstr);
  if (strlen(host) > 0) {
    sprintf(getstr, "Host: %s:%s\r\n", host, port);
    strcat(str, getstr);
  }

  sprintf(getstr, "%s", user_agent_hdr);
  strcat(str, getstr);
  printf("%s\n", str);

  while (Rio_readlineb(&rio_client, tmp, MAXLINE) > 0) {
    if (!strcmp(tmp, "\r\n")) {
      strcat(str, "\r\n");
      break;
    } else {
      strcat(str, tmp);
    }
  }

  return 0;
}

int response(int server, int client, char *content) {

  rio_t rio_server;
  char temp[MAXLINE];
  unsigned int size = 0, len = 0;
  int valid_size = 1;

  content[0] = '\0';

  Rio_readinitb(&rio_server, server);

  while ((len = Rio_readnb(&rio_server, temp, MAXLINE)) > 0) {
    rio_writen(client, temp, len);
  }

  return 0;
}