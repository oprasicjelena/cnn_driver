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
                        fprintf(cnn, "%d %d %d", 5, n[5], temp);
                        fflush(cnn);
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
    int i, j, k;
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
            
            k++;
            
            if(k == 16)
            {
                fflush(bram);
                k = 0;
                printf("Packet sent\n");
            }
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


void write_ip(char adr0[], char adr1[]) {
   FILE* cnn;
   cnn = fopen(loc0, "w");

   img_write(adr1);
   weight_write(adr0);
   
   fprintf (cnn, "%d %d %d", 42, 0, 0);
}

float cast(int x){
    if (x < 262144) {
        return x/2048.0;
    }
    else {
        return (x-524288)/2048.0;
    }
    
}

void read_ip(float buff[]) {
    FILE* cnn;
    cnn = fopen(loc1, "r");
    int temp;        
    int i;
    for(i = 0; i < OUT_SIZE; i++){
        fscanf (cnn, "%d", &temp);
        buff[i] = cast(temp);
        printf("%d", buff[i]);
    }
}

int main(int argc, char* argv[])
{
    float result[OUT_SIZE];
    int i, j;

    write_ip(argv[1], argv[2]);

    read_ip(result);

    double temp = result[0];
    j = 0;
    for(i = 1; i < 10; i++){
        if(temp < result[i]){
            temp = result[i];
            j = i;
        }
    }

    usleep(2000);

    printf("Image showed: \n");//set message
    switch(j){
        case 0:
           printf("T-shirt/top\n");
            break;
        case 1:
            printf("Trousers\n");
            break;
        case 2:
            printf("Pullover\n");
            break;
        case 3:
            printf("Dress\n");
            break;
        case 4:
            printf("Coat\n");
            break;
        case 5:
            printf("Sandal\n");
            break;
        case 6:
            printf("Shirt\n");
            break;
        case 7:
            printf("Sneaker\n");
            break;
        case 8:
            printf("Bag\n");
            break;
        case 9:
            printf("Ankle boot\n");
            break;
        default:
            printf("NaN\n");
            break;
    }
    
    return 0;
}