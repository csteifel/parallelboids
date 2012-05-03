#include <stdio.h>
#include <stdlib.h>

int main(){
	FILE * fd = fopen("input", "r");
	FILE * od = fopen("output", "wb+");
	unsigned int i, j, width, height;
	short shorty;

	fscanf(fd, "%u %u", &width, &height);
	printf("%u %u\n", width, height);
	fwrite(&width, sizeof(unsigned int), 1, od);
	fwrite(&height, sizeof(unsigned int), 1, od);
	//Go through parsing the file
	for(j = 0; j < height; j++){
		for(i = 0; i < width; i++){
			fscanf(fd, "%1hd", &shorty);
			fwrite(&shorty, sizeof(short), 1, od);
		}
	}
}
