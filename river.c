#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_HACKERS 20
#define MAX_SERFS 20
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

char screen[SCREEN_HEIGHT][SCREEN_WIDTH];

// Imprime o estado da animacao
void printScreen () {

    int i, j;

    for (i = 0; i < SCREEN_HEIGHT; i ++) {
        for (j = 0; j < SCREEN_WIDTH; j ++) {
            printf("%c", screen[i][j]);
        }
        printf("\n");
    }
}


int main () {

    
    return 0;

}
