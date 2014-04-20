#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_HACKERS 20
#define MAX_SERFS 20
#define MAX_BOAT 10
#define MAX_QUEUE 40
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

typedef struct queue {
  int start, end;
  int order[MAX_QUEUE];
} queue;

char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
int in_hackers, in_serfs, boat_size, n_hackers, n_serfs, is_captain;

sem_t hackers, serfs;
sem_t lock, queue_lock, board_lock, move_lock;

pthread_mutex_t mutex;

pthread_barrier_t barrier;

queue hackers_queue, serfs_queue;

// Funcoes de manipulacao da fila
void initQueue(queue *q){
  q->start = q->end = 0;
}

void insertQueue(queue *q, int id){
  q->order[q->end] = id;
  q->end = (q->end + 1) % MAX_QUEUE;
}

int dequeue(queue *q){
  int aux = q->order[q->start];
  q->start = (q->start + 1) % MAX_QUEUE;
  return aux;
}

int first(queue q){
  return q.order[q.start];
}

int second(queue q){
  return q.order[(q.start + 1) % MAX_QUEUE];
}

int isEmpty(queue q){
  return q.start == q.end;
}

// Espera a animacao de embarcar
void board() {
	sem_wait(&board_lock);
}

// Espera a animacao de mover o barco
void rowBoat() {
	sem_wait(&move_lock);
}


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

void* hacker(void* id) {

	int i, *aux;

	while (1) {
		sleep(rand() % 10);
		pthread_mutex_lock(&mutex);
		// Insere na fila
		aux = (int*) id;
		insertQueue(&hackers_queue, *aux);
		n_hackers ++;
		if (n_hackers == boat_size) {
			// Revome da fila n hackers
			for (i = 0; i < boat_size; i ++) {
				dequeue(&hackers_queue);
				sem_post(&hackers);
			}
			n_hackers = 0;
			is_captain = 1;
		} else if (n_hackers == (boat_size / 2) && n_serfs >= (boat_size / 2)) {
			// Revome da fila n / 2 hackers
			for (i = 0; i < (boat_size / 2); i ++) {
				dequeue(&hackers_queue);
				sem_post(&hackers);
			}
			// Revome da fila n / 2 serfs
			for (i = 0; i < (boat_size / 2); i ++) {
				dequeue(&serfs_queue);
				sem_post(&serfs);
			}
			n_hackers = 0;
			n_serfs = n_serfs - (boat_size / 2);
			is_captain = 1;
		} else {
			pthread_mutex_unlock(&mutex);
		}
		sem_wait(&hackers);
		board();
		pthread_barrier_wait(&barrier);
		if (is_captain == 1) {
			rowBoat();
			pthread_mutex_unlock(&mutex);
		}
	}
	return NULL;
}

void* serf(void* id) {

	int i, *aux;

	while (1) {
		sleep(rand() % 10);
		pthread_mutex_lock(&mutex);
		// Insere na fila
		aux = (int*) id;
		insertQueue(&serfs_queue, *aux);
		n_serfs ++;
		if (n_serfs == boat_size) {
			// Revome da fila n serfs
			for (i = 0; i < boat_size; i ++) {
				dequeue(&serfs_queue);
				sem_post(&serfs);
			}
			n_hackers = 0;
			is_captain = 1;
		} else if (n_hackers == (boat_size / 2) && n_serfs >= (boat_size / 2)) {
			// Revome da fila n / 2 hackers
			for (i = 0; i < (boat_size / 2); i ++) {
				dequeue(&hackers_queue);
				sem_post(&hackers);
			}
			// Revome da fila n / 2 serfs
			for (i = 0; i < (boat_size / 2); i ++) {
				dequeue(&serfs_queue);
				sem_post(&serfs);
			}
			n_serfs = 0;
			n_hackers = n_hackers - (boat_size / 2);
			is_captain = 1;
		} else {
			pthread_mutex_unlock(&mutex);
		}
		sem_wait(&serfs);
		board();
		pthread_barrier_wait(&barrier);
		if (is_captain == 1) {
			rowBoat();
			pthread_mutex_unlock(&mutex);
		}
	}
	return NULL;
}


// Le a entrada
// O numero de hackers (0 <= in_hackers <= 20)
// O numero de serfs (0 <= in_serfs <= 20)
// O tamanho do barco (0 < boat_size <= 10 | boat_size % 2 == 0)
int readInput() {

    printf("Digite o numero de Hackers. (MAX: 20)\n");
	scanf("%d", &in_hackers);
	if (in_hackers < 0 || in_hackers > MAX_HACKERS) {
		printf("Numero de Hackers invalido.\n");
		return 1;
	}
    printf("Digite o numero de Serfs. (MAX: 20)\n");
	scanf("%d", &in_serfs);
	if (in_serfs < 0 || in_serfs > MAX_SERFS) {
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

// Inicializa os componentes
void init() {

	n_hackers = 0;
	n_serfs = 0;
	is_captain = 0;

	pthread_mutex_init(&mutex, NULL);

	sem_init(&lock, 0, 1);
	sem_init(&queue_lock, 0, 1);
	sem_init(&board_lock, 0, 1);
	sem_init(&move_lock, 0, 1);
	sem_init(&hackers, 0, 0);
	sem_init(&serfs, 0, 0);

	initQueue(&hackers_queue);
	initQueue(&serfs_queue);

	pthread_barrier_init(&barrier, NULL, boat_size);

	srand(time(NULL));

}

int main () {

	int i, *aux;
	pthread_t *thr;

	// Lendo a entrada
	if (readInput())
		return 0;

	// Inicializando os componentes
	init();

	thr = malloc(sizeof(pthread_t) * (in_hackers + in_serfs));

	for (i = 0; i < in_hackers; i ++) {
		aux = (int*) malloc (sizeof(int));
		*aux = i + 1;
		pthread_create(&thr[i], NULL, hacker, (void *) aux);
	}
	for (i = in_hackers; i < in_hackers + in_serfs; i ++) {
		aux = (int*) malloc (sizeof(int));
		*aux = i + 1;
		pthread_create(&thr[i], NULL, serf, (void *) aux);
	}

	while (1) {
		usleep(300000);
//		changeScreenStatus();
		printScreen(); 
	}
    return 0;
}
