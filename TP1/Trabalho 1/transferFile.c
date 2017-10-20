#include "definitions.h"
#include "transferFile.h"
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

	printf("Transfer file successfully opened!\n");

    return 0;
}

void transferFileClose() {
	if(fclose(traF->file) < 0) {
		printf("Unable to close transfer file!\n");
		exit(1);
	}

	printf("Transfer file successfully closed!\n");
}
