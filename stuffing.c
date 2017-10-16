#include "stuffing.h"
#include "definitions.h"
#include <stdlib.h>
#include <string.h>

char* stuffing(char* buf) {
  char* stuffed = malloc(1024);
  unsigned int size = strlen(stuffed);

  stuffed[0] = buf[0];

  int i, j = 1;

  for (i = 1; i < (size - 1); i++, j++) {
    if(buf[i] == FLAG || buf[i] == ESC_BYTE) {
      size++;
      stuffed[j] = ESC_BYTE;
      j++;
      stuffed[j] = buf[i] ^ 0x20;
    }
    else
      stuffed[j] = buf[i];
  }

  stuffed[j] = buf[i];

  return stuffed;
}

char* destuffing(char* buf) {
  char* destuffed = malloc(1024);

  int i, j=0;

  for (i = 0; i < strlen(buf); i++, j++) {
    if (buf[i] == ESC_BYTE) {
      i++;
      destuffed[j] = buf[i] ^ 0x20;
    }
    else
      destuffed[j] = buf[i];
  }

  return destuffed;
}