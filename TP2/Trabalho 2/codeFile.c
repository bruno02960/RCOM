#include "headerFile.h"

int connectToSocket(/* IP & PORT */){

	int	sockfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(/* IP */);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(/* PORT */);						/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
			perror("socket()");
	    exit(1);
  }

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
			perror("connect()");
	    exit(1);
	}

	/* THIS MAY BE INCLUDED IN ANOTHER FUNCTION  ---------------- */

	/*send a string to the server*/
	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
	int	bytes;

	bytes = write(sockfd, buf, strlen(buf));
	printf("Bytes escritos %d\n", bytes);

	/* ---------------------------------------------------------- */

	return sockfd;


		//close(sockfd);  fechar no fim do programa

}

int parseURL(/* URL */) {
  //Verify "ftp://"

  //Verify if there's an username and password
    //If it has parse it otherwise skip?

  //Verify host

  //Verify path

  return 0;
}

	/*
		First, we get ipByHost -> not sure if it's for now
		FTP FUNCTIONS AND IN ORDER OF CALL:
			FTPConnect;
			FTPLogin;
			FTPPassiveMode;
			FTPGetPathForFile
			FTPTransfer
			FTPDisconnect
	*/

int getIP(/* host or struct containing host */) {

	struct hostent *h;

  /*
  struct hostent {
  	char *h_name;	Official name of the host.
    char  **h_aliases;	A NULL-terminated array of alternate names for the host.
  	int  h_addrtype;	The type of address being returned; usually AF_INET.
    int   h_length;	The length of the address in bytes.
  	char   **h_addr_list;	A zero-terminated array of network addresses for the host.
  	Host addresses are in Network Byte Order.
  };

  #define h_addr h_addr_list[0]	The first address in h_addr_list.
  */

  if ((h=gethostbyname(/* host */)) == NULL) {
      herror("gethostbyname");
      exit(1);
  }

  printf("Host name  : %s\n", h->h_name);
  printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

  /* SAVE IP FOR LATER ? */

  return 0;
}

/* download ftp://ftp.up.pt/pub/... */
int main(int argc, char *argv[])
{
  if (argc != 2) {
      printf("Usage:\tPath\n\tex: ftp://ftp.up.pt/pub/...\n");
      exit(0);
  }

	/* PARSE PATH */

    return 0;
}
