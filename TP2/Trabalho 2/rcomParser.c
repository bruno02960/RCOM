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
	    return -1;
  }

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
			perror("connect()");
	    return -1;
	}

	if (port == COMMAND_PORT) {
		char buf[STR_SIZE];
		int noBytes;

		FILE* fp = fdopen(sockfd, "r");

		while(fgets(buf, 5, fp))
			if (strncmp(buf, "220 ", CODE_LENGTH + 1) == 0)
				{
					printf("HERE!\n");
					break;
				}
	}

	return sockfd;
}

int login(int socket) {
  char buf[STR_SIZE];
  int noBytes;

  sprintf(buf, "user %s\n", user->username);
	noBytes = write(socket, buf, strlen(buf));
	printf("%d\n", noBytes);
	printf("> %s\n", buf);

	bzero(buf, sizeof(buf));

	noBytes = read(socket, buf, STR_SIZE);
	buf[noBytes]='\0';
	printf("< %s\n", buf);

	bzero(buf, sizeof(buf));

	sprintf(buf, "pass %s\n", user->password);
	noBytes = write(socket, buf, strlen(buf));
	printf("%d\n", noBytes);
	printf("> %s\n", buf);

	bzero(buf, sizeof(buf));

	noBytes = read(socket, buf, STR_SIZE);
	buf[noBytes] = '\0';
	printf("< %s\n", buf);

	if (strncmp(buf, "230", CODE_LENGTH) != 0) {
		return -1;
	}

	return 0;

}

int getDataPort(int socket, int *data_port) {
	char buf[STR_SIZE];
	int noBytes;

	strcpy(buf, "pasv\n");
	noBytes = write(socket, buf, strlen(buf));
	printf("%d\n", noBytes);
	printf("> pasv\n");

	bzero(buf, sizeof(buf));

	noBytes = read(socket, buf, STR_SIZE);
	buf[noBytes] = '\0';

	if (strncmp(buf, "227", CODE_LENGTH) != 0) {
		return -1;
	}

	printf("< %s\n", buf);

	int ip1,ip2,ip3,ip4;
	int port1, port2;
	sscanf(buf,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1,&ip2,&ip3,&ip4,&port1,&port2);

	(*data_port) = port1 * 256 + port2;

	return 0;
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

  char fileName[strlen(url_path) + 1];
  strcpy(fileName, url_path);
  char* current;

	while ((current = strchr(fileName, '/'))) {
		strcpy(fileName, current + 1);
	}

	url->file_name = (char*)malloc(sizeof(strlen(fileName) + 1));

	strcpy(url->file_name, fileName);
}

int fileDownload(int command_sockfd, int data_sockfd) {
	char buf[STR_SIZE];
	int noBytes;
	int fd;

	sprintf(buf, "retr %s\n", url->path);
	write(command_sockfd, buf, strlen(buf));
	printf("> %s\n", buf);

	bzero(buf, sizeof(buf));

	noBytes = read(command_sockfd, buf, STR_SIZE);
	buf[noBytes] = '\0';

	if (strncmp(buf, "150", CODE_LENGTH) != 0) {
		return -1;
	}

	bzero(buf, sizeof(buf));

	fd = open(url->file_name, O_RDWR | O_CREAT | O_TRUNC, 0777);

	int r;
	while((r = read(data_sockfd, buf, STR_SIZE)) >= 1) {
			write(fd, buf, r);
	}

	close(fd);

	return 0;
}

int main(int argc, char* argv[]) {
  char * fullURL = argv[1];
  int command_sockfd, data_sockfd, data_port;

  url = (url_t*)malloc(sizeof(url_t));
  user = (user_t*)malloc(sizeof(user_t));

  parseURL(fullURL);
  printf("username = %s\tpassword = %s\t\nhost = %s\turl_path = %s\tfile_name = %s\n", user->username, user->password, url->host, url->path, url->file_name);

  getIP();
  printf("ip = %s\n", url->ip);

  command_sockfd = connectToSocket(COMMAND_PORT);

	if(login(command_sockfd)!=0)
		exit(1);

	getDataPort(command_sockfd, &data_port);

	data_sockfd = connectToSocket(data_port);

	fileDownload(command_sockfd, data_sockfd);

	close(command_sockfd);
	close(data_sockfd);

	free(url);
	free(user);

	return 0;

}
