#include "ftp.h"

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
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
			perror("connect()");
	    return -1;
	}

	/*initial call*/
	if (port == COMMAND_PORT) {
		char buf[STR_SIZE];
		FILE* fp;

		if ((fp = fdopen(sockfd, "r")) == NULL) {
			perror("fdopen()");
			return -1;
		}

		printf("< ");

		while(fgets(buf, 5, fp)) {
			printf("%s", buf);
			if (strncmp(buf, "220 ", CODE_LENGTH + 1) == 0)
					break;
		}
	}

	return sockfd;
}

int login(int socket) {
  char buf[STR_SIZE];
  int noBytes;

	/*parse user*/
  sprintf(buf, "user %s\n", user->username);
	if ((noBytes = write(socket, buf, strlen(buf))) == -1) {
		perror("write()");
		return -1;
	}
	printf("\n\n> %s", buf);

	bzero(buf, sizeof(buf));

	if ((noBytes = read(socket, buf, STR_SIZE)) == -1) {
		perror("read()");
		return -1;
	}
	buf[noBytes]='\0';
	printf("< %s\n", buf);

	bzero(buf, sizeof(buf));

	/*parse pass*/
	sprintf(buf, "pass %s\n", user->password);
	if ((noBytes = write(socket, buf, strlen(buf))) == -1) {
		perror("write()");
		return -1;
	}
	printf("> %s", buf);

	bzero(buf, sizeof(buf));

	/*read answer*/
	if ((noBytes = read(socket, buf, STR_SIZE)) == -1) {
		perror("read()");
		return -1;
	}
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

	/*send pasv*/
	strcpy(buf, "pasv\n");
	if ((noBytes = write(socket, buf, strlen(buf))) == -1) {
		perror("write()");
		return -1;
	}
	printf("> pasv\n");

	bzero(buf, sizeof(buf));

	/* read answer */
	if ((noBytes = read(socket, buf, STR_SIZE)) == -1) {
		perror("read()");
		return -1;
	}
	buf[noBytes] = '\0';

	if (strncmp(buf, "227", CODE_LENGTH) != 0) {
		return -1;
	}

	printf("< %s\n", buf);

	/*parse answer*/
	int ip1,ip2,ip3,ip4;
	int port1, port2;
	sscanf(buf,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1,&ip2,&ip3,&ip4,&port1,&port2);

	/*calculate data_port*/
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
	  url->ip = (char*)malloc(strlen(ip) + 1);
	  strcpy(url->ip, ip);

	return 0;
}

int parseURL(char* full_url) {
  char url_cpy[strlen(full_url) + 1];
  char init[] = "ftp://";
  int index = strlen(init);

  strcpy(url_cpy, full_url);

	/*check init*/
  if(strncmp(init, full_url, strlen(init)) != 0)
    return 1;

	/*try username parse*/
  char* username;
  username=strtok(&url_cpy[index], ":");

	if (strcmp(username, &full_url[index]) !=0) {
		char* rest;
		char* password;

		/*parse password*/
	  if ((rest=strtok(NULL, ":"))!=NULL) {
	      index+=strlen(username);

	      password=strtok(rest, "@");
	      rest=strtok(NULL, "@");

	      index+=strlen(password) + 2;

		}

		user->username = (char*)malloc(strlen(username) + 1);
		user->password = (char*)malloc(strlen(password) + 1);
		strcpy(user->username, username);
		strcpy(user->password, password);
	}
	else {
		/*case no username nor password*/
		user->username = (char*)malloc(strlen("anonymous"+1));
		user->password = (char*)malloc(strlen("anonymous"+1));
		strcpy(user->username,"anonymous");
		strcpy(user->password,"anonymous");
	}

	/*parse host*/
  char* str_aux = &full_url[index];
	char* host=strtok(str_aux, "/");

  if(strcmp(host, "") == 0) {
    perror("host not provided\n");
    return 1;
  }

  index+=strlen(host) + 1;

	/*parse url_path*/
  char* url_path = &full_url[index];

  if(strcmp(url_path, "") == 0) {
    perror("url_path not provided\n");
    return 1;
  }

  url->host = (char*)malloc(strlen(host) + 1);
  url->path = (char*)malloc(strlen(url_path) + 1);



  strcpy(url->host, host);
printf("HOST = %s\n", url->host);
  strcpy(url->path, url_path);
	

	/*get filename*/
  char fileName[strlen(url_path) + 1];
  strcpy(fileName, url_path);
  char* current;

	while ((current = strchr(fileName, '/'))) {
		strcpy(fileName, current + 1);
	}

	url->file_name = (char*)malloc(strlen(fileName) + 1);

	strcpy(url->file_name, fileName);
}

int fileDownload(int command_sockfd, int data_sockfd) {
	char buf[STR_SIZE];
	int noBytes;
	int fd;

	/*send retr file*/
	sprintf(buf, "retr %s\n", url->path);
	if ((noBytes=write(command_sockfd, buf, strlen(buf))) == -1) {
		perror("write()");
		return -1;
	}
	printf("> %s\n", buf);

	bzero(buf, sizeof(buf));

	/*get answer*/
	if ((noBytes = read(command_sockfd, buf, STR_SIZE)) == -1) {
		perror("read()");
		return -1;
	}
	buf[noBytes] = '\0';

	if (strncmp(buf, "150", CODE_LENGTH) != 0) {
		return -1;
	}

	bzero(buf, sizeof(buf));

	/*open output file and write to it*/
	if ((fd = open(url->file_name, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1) {
		perror("open()");
		return -1;
	}

	int r;
	while ((r = read(data_sockfd, buf, STR_SIZE)) >= 1) {
			if (r == -1) {
				perror("read()");
				return -1;
			}

			if (write(fd, buf, r) == -1) {
				perror("write()");
				return -1;
			}
	}

	if (close(fd) == -1) {
		perror("close()");
		return -1;
	}

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

  if ((command_sockfd = connectToSocket(COMMAND_PORT)) == -1)	{
		printf("Error on connectToSocket\n");
		exit(1);
	}
  printf("username = %s\tpassword = %s\t\nhost = %s\turl_path = %s\tfile_name = %s\n", user->username, user->password, url->host, url->path, url->file_name);

	if (login(command_sockfd) == -1) {
		printf("Error on login()\n");
		exit(1);
	}
  printf("username = %s\tpassword = %s\t\nhost = %s\turl_path = %s\tfile_name = %s\n", user->username, user->password, url->host, url->path, url->file_name);

	if (getDataPort(command_sockfd, &data_port) == -1) {
		printf("Error on getDataPort()\n");
		exit(1);
	}

	if ((data_sockfd = connectToSocket(data_port)) == -1) {
		printf("Error on connectToSocket\n");
		exit(1);
	}

	if (fileDownload(command_sockfd, data_sockfd) == -1) {
		printf("Error on fileDownload\n");
		exit(1);
	}

	if (close(command_sockfd) == -1) {
		perror("close()");
		exit(1);
	}

	if (close(data_sockfd) == -1) {
		perror("close()");
		exit(1);
	}

	free(url);
	free(user);

	return 0;
}
