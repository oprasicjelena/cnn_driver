#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int comp2(float x) {
    if (x >= 0) {
        return (int)(x*2048);
    }
    else {
        return 524288 + (int)(x*2048);
    }
}

int main()
{

	int i, j = 0;
	int temp;
	float t;
	FILE *my_file;
	FILE *weights;

	my_file = fopen("/home/jelena/file.txt", "w");
	weights = fopen("/home/jelena/cnn_driver/app/weights.txt", "r");
	

	 for (i = 0; i < 32; i++){
    	for (j = 0; j < 9; j++){
    		fscanf(weights, "%f", &t);
			temp = comp2(t);
    		fprintf(my_file, "%d %d %d", 16+j, i, temp);
            printf("%d %d %d", 16+j, i, temp);
            //fflush(my_file);
            printf("Packet sent\n");
    	}

    }

    	
    for (i = 0; i < 7; i++){
    	for (j = 0; j < 10; j++){
            fprintf(my_file, "%d %d %d", 16+9+j, i, 0);
            //fflush(bram);
    	}
    }

    while (i < 13*13*32 + 7){
    	for (j = 0; j < 10; j++){
    		fscanf(weights, "%f", &t);
			temp = comp2(t);
            fprintf(my_file, "%d %d %d", 16+9+j, i, temp);
            //fflush(bram);
    	}
        i++;
    }

    

    fclose(my_file);
    fclose(weights);
}