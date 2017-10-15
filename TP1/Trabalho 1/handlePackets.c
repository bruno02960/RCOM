
#include "handlePackets.h"

void writeControlPacket(int controlField) {
    char fileSize[14];  /* 10.7KB = 10 700B | log2(10 700)~=14 */
    sprintf(fileSize, "%d", traF->fileSize);

    int ctrlPkSize = 5 + strlen(fileSize) + strlen(FILE_PATH); /* 5 bytes
    from C, T1, L1, T2 and L2 */

    unsigned char controlPacket[ctrlPkSize];

    controlPacket[0] = controlField + '0';
    controlPacket[1] = FILE_SIZE + '0';
    controlPacket[2] = strlen(fileSize) + '0';

    int index = 3;
    int k;

    for(k = 0; k < strlen(fileSize); k++, index++)
      controlPacket[index] = fileSize[k];

    controlPacket[index++] = FILE_NAME + '0';
    controlPacket[index++] = strlen(FILE_PATH) + '0';

    for(k = 0; k < strlen(FILE_PATH); k++, index++)
      controlPacket[index] = fileSize[k];

    if(llwrite(controlPacket, ctrlPkSize) < 0) {
      printf("Error on llwrite!\n");
      exit(1);
    }
}

void writeDataPacket(char* buffer, int noBytes, int seqNo) {
    int dataPkSize = noBytes + 4; /* 4 bytes from C, N, L2 and L1 */

    unsigned char dataPacket[dataPkSize];

    dataPacket[0] = DATA_BYTE + '0';
    dataPacket[1] = seqNo + '0';

    /* K = 256 * dataPacket[2] + dataPacket[3] */
    dataPacket[2] = noBytes / 256;
    dataPacket[3] = noBytes % 256;
    memcpy(&dataPacket[4], buffer, noBytes);



    if(llwrite(dataPacket, dataPkSize) < 0) {
      printf("Error on llwrite!\n");
      exit(1);
    }
}

int receiveControlPacket(int controlField, int* noBytes, char** filePath) {
    unsigned char* controlPacket;

    /* Handle possible errors */
    llread(&controlPacket);

    if((controlPacket[0] - '0') != controlField) {
      printf("Unexpected control field!\n");
      exit(1);
    }

    if((controlPacket[1] - '0') != FILE_SIZE) {
      printf("Unexpected parameter!\n");
      exit(1);
    }

    int lengthSize = (controlPacket[2] - '0');
    int i, valueIndex = 3;
    char fileSize[STR_SIZE];

    for (i = 0; i < lengthSize; i++)
      fileSize[i] = controlPacket[valueIndex++];

    fileSize[valueIndex - 3] = '\0';
    (*noBytes) = atoi(fileSize);

    if((controlPacket[valueIndex++] - '0') != FILE_NAME)
      printf("Unexpected parameter!\n");

    int lengthPath = (controlPacket[valueIndex++] - '0');
    char path[STR_SIZE];

    for (i = 0; i < lengthPath; i++)
      path[i] = controlPacket[valueIndex++];

    path[i] = '\0';
    strcpy((*filePath), path);

    return 0;
}

int receiveDataPacket(unsigned char ** buffer, int sequenceNumber) {
    unsigned char* dataPacket;
    int read;

    /* Handle possible errors */
    llread(&dataPacket);

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
    read = 256 * l2 - l1;

    memcpy((*buffer), &dataPacket[4], read);
    free(dataPacket);

    return read;
}