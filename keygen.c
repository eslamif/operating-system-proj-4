//Frank Eslami, CS 344-400, Program 4
//keygen program
//Run command: keygen keylength

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
		srand(time(NULL));

		//Obtain file size
		int file_size = atoi(argv[1]);
//		printf("file_size = %d\n", file_size);

		//Get random chars
		int i;
		char rand_char[2] = {0};
		char key[file_size + 1];
		memset(key, 0, sizeof(key));

		for (i = 0; i < file_size; i++) {
				rand_char[1] = "a bcdefghijklmnopqrstuvwxyz"[random () % 26];
				rand_char[2] = '\0';
//				printf("%c", rand_char[1]);
				key[strlen(key)] = rand_char[1];
		}
//				printf("\n");
				printf("%s\n", key);

		return 0;
}
