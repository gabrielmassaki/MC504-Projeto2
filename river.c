#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_HACKERS 20
#define MAX_SERFS 20
#define MAX_BOAT 10
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
int n_hackers, n_serfs, boat_size;
sem_t lock;

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

// Le a entrada
// O numero de hackers (0 <= n_hackers <= 20)
// O numero de serfs (0 <= n_serfs <= 20)
// O tamanho do barco (0 < boat_size <= 10 | boat_size % 2 == 0)
int readInput() {

    printf("Digite o numero de Hackers. (MAX: 20)\n");
	scanf("%d", &n_hackers);
	if (n_hackers < 0 || n_hackers > MAX_HACKERS) {
		printf("Numero de Hackers invalido.\n");
		return 1;
	}
    printf("Digite o numero de Serfs. (MAX: 20)\n");
	scanf("%d", &n_serfs);
	if (n_serfs < 0 || n_serfs > MAX_SERFS) {
		printf("Numero de Serfs invalido.\n");
		return 1;
	}
    printf("Digite o tamanho do barco. Precisa ser um numero par e diferente de zero. (MAX: 10)\n");
	scanf("%d", &boat_size);
	if (boat_size <= 0 || boat_size > MAX_BOAT || (boat_size % 2 != 0)) {
		printf("Tamanho do barco invalido.\n");
		return 1;
	}
	return 0;
}

int main () {

	// Lendo a entrada
	if (readInput())
		return 0;

    return 0;

}
