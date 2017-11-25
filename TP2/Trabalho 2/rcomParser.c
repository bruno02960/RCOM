#include <stdio.h>
#include <string.h>

int parseURL(char* url) {
  char url_cpy[strlen(url) + 1];
  char init[] = "ftp://";
  int index = strlen(init);
  char* username;
  char* rest;
  char* password;
  char* host;

  strcpy(url_cpy, url);

  if(strncmp(init, url, strlen(init)) != 0)
    return 1;

  username=strtok(&url_cpy[index], ":");

  if((rest=strtok(NULL, ":"))!=NULL) {
      printf("username = %s\n", username);

      index+=strlen(username);

      password=strtok(rest, "@");
      rest=strtok(NULL, "@");

      index+=strlen(password) + 2;

      printf("password = %s\n", password);
  };

  char* str_aux = &url[index];

  host=strtok(str_aux, "/");

  index+=strlen(host) + 1;

  printf("host = %s\n", host);

  char* url_path = &url[index];

  printf("url_path = %s\n", url_path);

}

int main(int argc, char* argv[]) {
  parseURL(argv[1]);
}
