#include "definitions.h"
#include "configs.h"
#include "transferFile.h"
#include "handlePackets.h"
#include <stdlib.h>

transferFile_t* traF;
char* FILE_PATH="./pinguim.gif";

int transferFileInit(int status) {
	struct stat st;

    traF = (transferFile_t*) malloc(sizeof(transferFile_t));

    if (status == TRANSMITTER) {
        if (!(traF->file = fopen(FILE_PATH, "rb"))) {
            printf("Unable to open file!\n");
            exit(1);
        }

        if(stat(FILE_PATH,&st)==0)
            traF->fileSize = st.st_size;
        else
        {
            printf("Unable to get file size!\n");
            exit(1);
        }
    }
    else if (!(traF->file = fopen(FILE_PATH, "wb"))) {
        printf("Unable to open file!\n");
        exit(1);
    }

	printf("Transfer file opened!\n");

    return 0;
}

void transferFileClose() {
	if(fclose(traF->file) < 0) {
		printf("Unable to close transfer file!\n");
		exit(1);
	}

	printf("Transfer file closed!\n");
}

int sendFile(int fd) {
  unsigned char * packetBuffer = malloc(PACKET_SIZE * sizeof(unsigned char));
  int read, seqNo=0, noBytes=0;

  if(writeControlPacket(CTRL_START, fd)) {
    printf("Error on writing control packet in sendFile!\n");
    exit(1);
  }
  else {
    printf("Start control packet written!\n\n");
  }

  while((read=fread(packetBuffer, sizeof(unsigned char), PACKET_SIZE, traF->file)) > 0 ){
    printf("%d bytes read from file!\n",read);

    if(writeDataPacket(packetBuffer, read, (seqNo % 255), fd)) {    //seqNo is module 255
      printf("Error on writing data packet in sendFile!\n");
      exit(1);
    }

    noBytes+=read;

    printf("Bytes accumulated = %d\n", noBytes);
    printf("seqNo sent = %d\n\n", (seqNo % 255));

    seqNo++;
  }

  transferFileClose();

  if(writeControlPacket(CTRL_END, fd)) {
    printf("Error on writing control packet in sendFile!\n");
    exit(1);
  }
	else {
	printf("End control packet written!\n");
	}

  return 0;
}


int receiveFile(int fd) {
  int fileSize;
  unsigned char *filePath = (unsigned char *) malloc(100);

  if(receiveControlPacket(START_BYTE, &fileSize, &filePath, fd)) {
    printf("Error on receiving control packet in receiveFile!\n");
    exit(1);
  }
  else {
	printf("Star control packet received!\n\n");
  }

  int read, noBytes = 0, seqNo = 0, written = 0;
  unsigned char * buffer = malloc(PACKET_SIZE * sizeof(char));

  while(noBytes < fileSize) {
    if((read = receiveDataPacket(&buffer, (seqNo % 255), fd))<0)
      exit(1);

    noBytes += read;
    printf("seqNo received = %d\n",(seqNo%255));
	   printf("Bytes accumulated = %d\n",noBytes);

    if((written=fwrite(buffer, sizeof(char), read, traF->file))>0) {
		printf("%d bytes written to file!\n\n", written);
	}
	else {
		return 1;
	}

    seqNo++;
  }

  transferFileClose();

  if(receiveControlPacket(END_BYTE, &fileSize, &filePath, fd)) {
    printf("Error on receiving control packet in receiveFile!\n");
    exit(1);
  }
	else {
	printf("End control packet received!\n");
	}

  return 0;
}
