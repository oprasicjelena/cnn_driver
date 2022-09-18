#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IMG_SIZE 28
#define OUT_SIZE 10

char loc0[] = "/dev/xlnx,ip-1.0";
char loc1[] = "/dev/xlnx,axi-bram-ctrl-4.1";

int comp2(float x) {
    if (x >= 0) {
        return (int)(x*2048);
    }
    else {
        return 524288 + (int)(x*2048);
    }
}

void img_write(char adr[]) {
    FILE *cnn;
    FILE *image;
    int i, j;
    int temp, a, b;
    int n[16];
    cnn = fopen(loc0, "w");
    image = fopen(adr, "r");

    for(i = 0; i< 16; i++){
        n[i] = 0;
    }

    for (i = 0; i < IMG_SIZE; ++i){//write matrix A into bram
        for (j = 0; j < IMG_SIZE; ++j){
            fscanf(image, "%d", &temp);
            a = i & 1;
            b = j & 1;
            if(a == 0){
                if(b == 0){
                    if(i != IMG_SIZE - 2 && j != IMG_SIZE - 2){
                        fprintf(cnn, "%d %d %d", 0, n[0], temp);
                        fflush(cnn);
                        n[0]++;
                    }

                    if(i != IMG_SIZE - 2 && j != 0){
                        fprintf(cnn, "%d %d %d", 2, n[2], temp);
                        fflush(cnn);
                        n[2]++;
                    }

                    if(i != 0 && j != IMG_SIZE - 2){
                        fprintf(cnn, "%d %d %d", 8, n[8], temp);
                        fflush(cnn);
                        n[8]++;
                    }
                    if(i != 0 && j != 0){
                        fprintf(cnn, "%d %d %d", 10, n[10], temp);
                        fflush(cnn);
                        n[10]++;
                    }
                }
                else{
                    if(i != IMG_SIZE - 2 && j != IMG_SIZE - 1){
                        fprintf(cnn, "%d %d %d", 1, n[1], temp);
                        fflush(cnn);
                        n[1]++;
                    }

                    if(i != 0 && j != IMG_SIZE - 1){
                        fprintf(cnn, "%d %d %d", 9, n[9], temp);
                        fflush(cnn);
                        n[9]++;
                    }

                    if(i != IMG_SIZE - 2 && j != 1){
                        fprintf(cnn, "%d %d %d", 3, n[3], temp);
                        fflush(cnn);
                        n[3]++;
                    }

                    if(i != 0 && j != 1){
                        fprintf(cnn, "%d %d %d", 11, n[11], temp);
                        fflush(cnn);
                        n[11]++;
                    }
                }
            }
            else{
                if(b == 0){
                    if(i != IMG_SIZE - 1 && j != IMG_SIZE - 2){
                        fprintf(cnn, "%d %d %d", 4, n[4], temp);
                        fflush(cnn);
                        n[4]++;
                    }
                    if(i != 1 && j != IMG_SIZE - 2){
                        fprintf(cnn, "%d %d %d", 12, n[12], temp);
                        fflush(cnn);
                        n[12]++;
                    }
                    if(i != IMG_SIZE - 1 && j != 0){
                        fprintf(cnn, "%d %d %d", 6, n[6], temp);
                        fflush(cnn);
                        n[6]++;
                    }
                    if(i != 1 && j != 0){
                        fprintf(cnn, "%d %d %d", 14, n[14], temp);
                        fflush(cnn);
                        n[14]++;
                    }
                }
                else{
                    if(i != IMG_SIZE - 1 && j != IMG_SIZE - 1){
                        fflush(cnn);
                        fprintf(cnn, "%d %d %d", 5, n[5], temp);
                        n[5]++;
                    }

                    if(i != 1 && j != IMG_SIZE - 1){
                        fprintf(cnn, "%d %d %d", 13, n[13], temp);
                        fflush(cnn);
                        n[13]++;
                    }

                    if(i != IMG_SIZE - 1 && j != 1){
                        fprintf(cnn, "%d %d %d", 0, n[7], temp);
                        fflush(cnn);
                        n[7]++;
                    }
                    if(i != 1 && j != 1){
                        fprintf(cnn, "%d %d %d", 15, n[15], temp);
                        fflush(cnn);
                        n[15]++;
                    }
                }
            }
        }
    }
    fclose(cnn);
    fclose(image);
}

void weight_write(char adr[]) {
	FILE *bram;
    FILE *weight;
    int i, j;
    int temp, a, b;
    float t;
    int n[16];
	bram = fopen(loc1, "w");
    weight = fopen(adr, "r");

    for (i = 0; i < 32; i++){
    	for (j = 0; j < 9; j++){
    		fscanf(weight, "%f", &t);
			temp = comp2(t);
    		fprintf(bram, "%d %d %d", 16+j, i, temp);
            fflush(bram);
    	}
    }

   /* for (i = 0; i < 7; i++){
    	for (j = 0; j < 10; j++){
            fprintf(bram, "%d %d %d", 16+9+j, i, 0);
            fflush(bram);
    	}
    }

    while (i < 13*13*32 + 7){
    	for (j = 0; j < 10; j++){
    		fscanf(weight, "%f", &t);
			temp = comp2(t);
            fprintf(bram, "%d %d %d", 16+9+j, i, temp);
            fflush(bram);
    	}
        i++;
    }*/
    
    fclose(bram);
    fclose(weight);
}


void write_ip(char adr0[], adr1[]) {

   weight_write(adr0);
   img_write(adr1);
}


int main(int argc, char* argv[])
{
	write_ip(argv[1], argv[2]);
}