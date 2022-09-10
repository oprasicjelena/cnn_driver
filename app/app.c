#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IMG_SIZE 28
#define OUT_SIZE 10

//char loc0[] = "/dev/xlnx,cnn";
char loc1[] = "/dev/xlnx,bramm";

int comp2(float x) {
    if (x >= 0) {
        return (int)(x*2048);
    }
    else {
        return 524288 + (int)(x*2048);
    }
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
    	}
    }

    for (i = 0; i < 7; i++){
    	for (j = 0; j < 10; j++){
            fprintf(bram, "%d %d %d", 16+9+j, i, 0);
    	}
    }

    while (i < 13*13*32 + 7){
    	for (j = 0; j < 10; j++){
    		fscanf(weight, "%f", &t);
			temp = comp2(t);
            fprintf(bram, "%d %d %d", 16+9+j, i, temp);
    	}
        i++;
    }
    
    fclose(bram);
    fclose(weight);
}


void write_ip(char adr0[]) {

   weight_write(adr0);

}


int main(int argc, char* argv[])
{
	write_ip(argv[1]);

}