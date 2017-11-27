#include <stdio.h>
#include <string.h>
#include "headerFile.h"

url_t* url;
user_t* user;

int connectToSocket(int port){
	int	sockfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(url->ip);  /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);                /*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket()");
	    return 0;
  }

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
			perror("connect()");
	    return 0;
	}

	/* DO WHAT*/


	return sockfd;
}

int login(int socket) {
  char* buf;
  int len;

  len = strlen(user->username) + 7;
  buf = (char*)malloc(sizeof(char)*len);

  snprintf(buf, len, "user %s\n", user->username);
  strcpy(user->username, buf);
	write(socket, buf, strlen(buf));
	printf(">%s\n", buf);

	bzero(buf, sizeof(buf));


}

int getIP() {

	struct hostent *h;
  char* ip;

  if ((h=gethostbyname(url->host)) == NULL) {
      herror("gethostbyname");
      exit(1);
  }

  strcpy(ip, inet_ntoa(*((struct in_addr *) h->h_addr)));
  url->ip = (char*)malloc(sizeof(strlen(ip) + 1));
  strcpy(url->ip, ip);

  return 0;
}

int parseURL(char* full_url) {
  char url_cpy[strlen(full_url) + 1];
  char init[] = "ftp://";
  int index = strlen(init);

  strcpy(url_cpy, full_url);

  if(strncmp(init, full_url, strlen(init)) != 0)
    return 1;

  char* username;
  username=strtok(&url_cpy[index], ":");

  char* rest;
  char* password;
  if((rest=strtok(NULL, ":"))!=NULL) {
      index+=strlen(username);

      password=strtok(rest, "@");
      rest=strtok(NULL, "@");

      index+=strlen(password) + 2;
  };

  user->username = (char*)malloc(sizeof(strlen(username) + 1));
  user->password = (char*)malloc(sizeof(strlen(password) + 1));

  if (strcmp(username, "") != 0) {
    strcpy(user->username, username);
    strcpy(user->password, password);
  }

  char* str_aux = &full_url[index];
  char* host=strtok(str_aux, "/");

  if(strcmp(host, "") == 0) {
    perror("host not provided\n");
    return 1;
  }

  index+=strlen(host) + 1;

  char* url_path = &full_url[index];
  char new_url_path[100];

  if(strcmp(url_path, "") == 0) {
    perror("url_path not provided\n");
    return 1;
  }

  url->host = (char*)malloc(sizeof(strlen(host) + 1));
  url->path = (char*)malloc(sizeof(strlen(url_path) + 1));

  strcpy(url->host, host);
  strcpy(url->path, url_path);

}

int main(int argc, char* argv[]) {
  char * fullURL = argv[1];
  int sockfd;

  url = (url_t*)malloc(sizeof(url_t));
  user = (user_t*)malloc(sizeof(user_t));

  parseURL(fullURL);
  printf("username = %s\tpassword = %s\t\nhost = %s\turl_path = %s\t\n", user->username, user->password, url->host, url->path);

  getIP();
  printf("ip = %s\n", url->ip);

  sockfd = connectToSocket(21);     /* WHY 21 */

}
