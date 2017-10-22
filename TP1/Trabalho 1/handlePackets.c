#include <string.h>
#include <stdlib.h>
#include "handlePackets.h"
#include "linkLayer.h"
#include "transferFile.h"
#include "definitions.h"

int writeControlPacket(int controlField, int fd) {
    unsigned char fileSize[14];  /* 10.7KB = 10 700B | log2(10 700)~=14 */
    sprintf((char*)fileSize, "%d", traF->fileSize);

    int ctrlPkSize = 5 + strlen((char*)fileSize) + strlen(FILE_PATH); /* 5 bytes
    from C, T1, L1, T2 and L2 */

    unsigned char controlPacket[ctrlPkSize];

    controlPacket[0] = controlField + '0';
    controlPacket[1] = FILE_SIZE + '0';
    controlPacket[2] = strlen((char*)fileSize) + '0';

    int index = 3;
    int k;

    for(k = 0; k < strlen((char*)fileSize); k++, index++)
      controlPacket[index] = fileSize[k];

    controlPacket[index++] = FILE_NAME + '0';
    controlPacket[index++] = strlen(FILE_PATH) + '0';

    for(k = 0; k < strlen(FILE_PATH); k++, index++)
      controlPacket[index] = FILE_PATH[k];

    if(llwrite(controlPacket, ctrlPkSize, fd) < 0) {
      printf("Error on llwrite!\n");
      return 1;
    }

    return 0;
}

int writeDataPacket(unsigned char* buffer, int noBytes, int seqNo, int fd) {
    int dataPkSize = noBytes + 4; /* 4 bytes from C, N, L2 and L1 */

    unsigned char dataPacket[dataPkSize];

    dataPacket[0] = DATA_BYTE + '0';
    dataPacket[1] = seqNo + '0';

    /* K = 256 * dataPacket[2] + dataPacket[3] */
    dataPacket[2] = noBytes / 256;
    dataPacket[3] = noBytes % 256;
    memcpy(&dataPacket[4], buffer, noBytes);

    if(llwrite(dataPacket, dataPkSize, fd) < 0) {
      printf("Error on llwrite!\n");
      return 1;
    }

    return 0;
}

int receiveControlPacket(int controlField, int* noBytes, unsigned char** filePath, int fd) {
    unsigned char* controlPacket;

    if (llread(&controlPacket, fd)) {
      printf("Error on receiving control packet at llread()!\n");
      return 1;
    }

    if((controlPacket[0] - '0') != controlField) {
      printf("Unexpected control field!\n");
      return 1;
    }

    if((controlPacket[1] - '0') != FILE_SIZE) {
      printf("Unexpected parameter!\n");
      return 1;
    }

    int lengthSize = (controlPacket[2] - '0');
    int i, valueIndex = 3;
    unsigned char fileSize[STR_SIZE];

    for (i = 0; i < lengthSize; i++)
      fileSize[i] = controlPacket[valueIndex++];

    fileSize[valueIndex - 3] = '\0';
    (*noBytes) = atoi((char*)fileSize);

    if((controlPacket[valueIndex++] - '0') != FILE_NAME)
      printf("Unexpected parameter!\n");

    int lengthPath = (controlPacket[valueIndex++] - '0');
    unsigned char path[STR_SIZE];

    for (i = 0; i < lengthPath; i++)
      path[i] = controlPacket[valueIndex++];

    path[i] = '\0';

    strcpy((char*)(*filePath), (char*)path);

    return 0;
}

int receiveDataPacket(unsigned char ** buffer, int sequenceNumber, int fd) {
    unsigned char* dataPacket;
    int read;

    if(llread(&dataPacket, fd)) {
      printf("Error on receiving data packet at llread()!\n");
      exit(1);
    }

    int controlField = dataPacket[0] - '0';
    int seqNo = dataPacket[1] - '0';

    if(controlField != DATA_BYTE) {
      printf("Unexpected control field!\n");
      exit(1);
    }

    if(seqNo != sequenceNumber) {
      printf("Unexpected sequence number!\n");
      exit(1);
    }

    int l2 = dataPacket[2], l1 = dataPacket[3];
    read = 256 * l2 + l1;

    memcpy((*buffer), &dataPacket[4], read);
    free(dataPacket);

    return read;
}
