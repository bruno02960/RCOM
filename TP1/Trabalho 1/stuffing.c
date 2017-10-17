#include "stuffing.h"
#include "definitions.h"
#include <stdlib.h>
#include <string.h>

char* stuffing(char* buf, int* size) {
  char* stuffed = malloc(1024);
  int stuffedSize = (*size);
	
  stuffed[0] = buf[0];

  int i, j = 1;

  for (i = 1; i < ((*size) - 1); i++, j++) {
    if(buf[i] == FLAG || buf[i] == ESC_BYTE) {
      stuffedSize++;
      stuffed[j] = ESC_BYTE;
      j++;
      stuffed[j] = buf[i] ^ 0x20;
    }
    else
      stuffed[j] = buf[i];
  }

  stuffed[j] = buf[i];	

(*size)=stuffedSize;

printf("SIZE=%d\n",(*size));

  return stuffed;
}

char* destuffing(char* buf, int* size) {
  char* destuffed = malloc(1024);

	printf("SIZE=%d\n",(*size));

	int destuffedSize = (*size);

  int i, j=4;

	for(i = 0; i < 4; i++)
		destuffed[i] = buf[i];

	i=0;

  for (i = 4; i < (*size) - 2; i++) {
    if (buf[i] == ESC_BYTE) {
      i++;
      destuffed[j] = buf[i] ^ 0x20;
		destuffedSize--;	
    }
    else
      destuffed[j] = buf[i];

	j++;
  }

	for(i = (*size) - 2; i < (*size); i++)
		destuffed[i] = buf[i];

	int k;

      for(k = 0; k < destuffedSize; k++) {
        printf("destuffed[%d]=%02x\n",k,destuffed[k]);
      }

  return destuffed;
}
