#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_HACKERS 10
#define MAX_SERFS 10
#define MAX_QUEUE 20
#define SCREEN_WIDTH 150
#define SCREEN_HEIGHT 30

typedef struct queue {
	int start, end;
	int order[MAX_QUEUE];
} queue;

char screen[SCREEN_HEIGHT][SCREEN_WIDTH];
int in_hackers, in_serfs, boat_size, n_hackers, n_serfs, is_captain, frames, status, hackers_on_the_boat, serfs_on_the_boat, boat_x_position, flag;

sem_t hackers, serfs, move_lock;

pthread_mutex_t mutex, hackers_queue_lock, serfs_queue_lock;

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

/*Funcao que retorna o numero de elementos na fila*/
int length(queue q){
	if(q.end >= q.start)
		return q.end - q.start;
	return MAX_QUEUE - q.end + q.start;
}

// Espera a animacao de mover o barco
void rowBoat() {
	flag = 1;
	sem_wait(&move_lock);
	flag = 0;
}

// Inicializa a "moldura" e o rio na tela
void initScreen() {
	int i, j;
	for (i = 0; i < SCREEN_HEIGHT; i ++) {
		for (j = 0; j < SCREEN_WIDTH; j ++) {
			if (i == 0 || i == SCREEN_HEIGHT - 1) {
				screen[i][j] = '-';
			} else if (j == 0 || j == SCREEN_WIDTH - 1 || SCREEN_WIDTH - 10 == j || SCREEN_WIDTH - 80 == j) {
				screen[i][j] = '|';
			} else {
				screen[i][j] = ' ';
			}
		}
	}
}

// Funcao que desenha o barco, o parametro indica o quao descolado para direita esta' o barco
void drawBoat(int aux) {
	int i;
	screen[SCREEN_HEIGHT / 2][boat_x_position - 2 + aux] = '\\';
	screen[(SCREEN_HEIGHT / 2) + 1][boat_x_position - 1 + aux] = '\\';
	for (i = 0; i < boat_size * 10; i ++) {
		screen[(SCREEN_HEIGHT / 2) + 1][boat_x_position + i + aux] = '_';
	}
	screen[SCREEN_HEIGHT / 2][boat_x_position + i + 1 + aux] = '/';
	screen[(SCREEN_HEIGHT / 2) + 1][boat_x_position + i + aux] = '/';
}

// Desenha o hacker na tela
void drawHacker(int i, int j) {
	screen[i][j - 1] = '"';
	screen[i][j + 1] = '"';
	screen[i - 1][j - 1] = '_';
	screen[i - 1][j + 1] = '_';
	screen[i - 1][j] = '/';
	screen[i - 1][j + 2] = '/';
	screen[i - 1][j - 2] = '<';
	screen[i - 2][j + 1] = ')';
	screen[i - 2][j - 1] = '/';
	screen[i - 2][j + 2] = '\\';
	screen[i - 3][j - 1] = '(';
	screen[i - 3][j + 1] = 'o';
	screen[i - 3][j + 2] = '>';
	screen[i - 4][j] = '_';
	screen[i - 4][j + 1] = '_';
}

// Desenha o serf na tela
void drawSerf(int i, int j) {
	screen[i][j] = ')';
	screen[i][j - 2] = '|';
	screen[i - 1][j - 2] = '|';
	screen[i - 2][j - 2] = 'M';
	screen[i][j + 1] = '(';
	screen[i - 1][j - 1] = '-';
	screen[i - 1][j] = '>';
	screen[i - 1][j + 1] = '<';
	screen[i - 2][j] = '(';
	screen[i - 2][j + 1] = ')';
}

/* Funcao que calcula a posicao do hacker localizado na
  n-esima posicao na fila de hackers e coloca
  tais valores nas variaveis x e y*/
void hackerPosition(int position, int *x, int *y){
	*x = ((SCREEN_WIDTH - 90) / 5) * (5 - (position % 5));
	if (position % 10 < 5) {
		*y = 5;
	} else {
		*y = 10;
	}
}

/* Funcao que calcula a posicao do serf localizado na
  n-esima posicao na fila de serfs e coloca
  tais valores nas variaveis x e y*/
void serfPosition(int position, int *x, int *y){
	*x = ((SCREEN_WIDTH - 90) / 5) * (5 - (position % 5));
	if (position % 10 < 5) {
		*y = 20;
	} else{
		*y = 25;
	}
}

// Atualiza a tela de acordo com o estado do sistema
void changeScreen() {

	int i, j, x, y;

	// Inicializando tela
	initScreen();

	if (flag == 0) { // O barco nao precisa se mover
		drawBoat(0);
	} else {
		drawBoat(frames);
		if (boat_x_position + frames == SCREEN_WIDTH - 50) { // Verifica se o barco chegou na margem direita
			hackers_on_the_boat = 0;
			serfs_on_the_boat = 0;
			status = 1;
		} else if (boat_x_position + frames == SCREEN_WIDTH - 80) { // Verifica se o barco chegou na margem esquerda
			status = 0;
			frames = 0;
			sem_post(&move_lock); // Libera o barco
		}
		if (status == 0) // A variavel status controla se o barco vai para direita ou para esquerda
			frames ++;
		else 
			frames --;	
	}

	// Desenha a fila de hackers
	pthread_mutex_lock(&hackers_queue_lock);
	for(i = 0; i < length(hackers_queue); i++){
		hackerPosition(i, &x, &y);
		drawHacker(y, x);
	}
	pthread_mutex_unlock(&hackers_queue_lock);

	// Desenha a fila de serfs
	pthread_mutex_lock(&serfs_queue_lock);
	for(i = 0; i < length(serfs_queue); i++){
		serfPosition(i, &x, &y);
		drawSerf(y, x);
	}
	pthread_mutex_unlock(&serfs_queue_lock);

	// Desenha as pessoas no barco
	for (i = 0; i < hackers_on_the_boat; i ++) {
		drawHacker((SCREEN_HEIGHT / 2) - 1, boat_x_position + 5 + 10 * i + frames);
	}
	for (j = i; j < serfs_on_the_boat + hackers_on_the_boat; j ++) {
		drawSerf((SCREEN_HEIGHT / 2) - 1, boat_x_position + 5 + 10 * j + frames);
	}
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

// Funcao baseada na resposta do livro The Little Book on Semaphores, de Allen B. Downey.
void* hacker(void* id) {

	int i, *aux;

	sleep(rand() % 2);
	// Insere na fila
	aux = (int*) id;
	pthread_mutex_lock(&hackers_queue_lock);
	insertQueue(&hackers_queue, *aux);
	pthread_mutex_unlock(&hackers_queue_lock);
	pthread_mutex_lock(&mutex);
	n_hackers ++;
	if (n_hackers == boat_size) {
		// Revome da fila n hackers
		for (i = 0; i < boat_size; i ++) {
			pthread_mutex_lock(&hackers_queue_lock);
			dequeue(&hackers_queue);
			pthread_mutex_unlock(&hackers_queue_lock);
			sem_post(&hackers);
		}
		n_hackers = 0;
		is_captain = 1;
	} else if (n_hackers == (boat_size / 2) && n_serfs >= (boat_size / 2)) {
		// Revome da fila n / 2 hackers
		for (i = 0; i < (boat_size / 2); i ++) {
			pthread_mutex_lock(&hackers_queue_lock);
			dequeue(&hackers_queue);
			pthread_mutex_unlock(&hackers_queue_lock);
			sem_post(&hackers);
		}
		// Revome da fila n / 2 serfs
		for (i = 0; i < (boat_size / 2); i ++) {
			pthread_mutex_lock(&serfs_queue_lock);
			dequeue(&serfs_queue);
			pthread_mutex_unlock(&serfs_queue_lock);
			sem_post(&serfs);
		}
		n_hackers = 0;
		n_serfs = n_serfs - (boat_size / 2);
		is_captain = 1;
	} else {
		pthread_mutex_unlock(&mutex);
	}
	sem_wait(&hackers);
	hackers_on_the_boat ++;
	pthread_barrier_wait(&barrier);
	if (is_captain == 1) {
		rowBoat();
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

// Funcao baseada na resposta do livro The Little Book on Semaphores, de Allen B. Downey.
void* serf(void* id) {

	int i, *aux;

	sleep(rand() % 2);
	// Insere na fila
	aux = (int*) id;
	pthread_mutex_lock(&serfs_queue_lock);
	insertQueue(&serfs_queue, *aux);
	pthread_mutex_unlock(&serfs_queue_lock);
	pthread_mutex_lock(&mutex);
	n_serfs ++;
	if (n_serfs == boat_size) {
		// Revome da fila n serfs
		for (i = 0; i < boat_size; i ++) {
			pthread_mutex_lock(&serfs_queue_lock);
			dequeue(&serfs_queue);
			pthread_mutex_unlock(&serfs_queue_lock);
			sem_post(&serfs);
		}
		n_serfs = 0;
		is_captain = 1;
	} else if (n_serfs == (boat_size / 2) && n_hackers >= (boat_size / 2)) {
		// Revome da fila n / 2 hackers
		for (i = 0; i < (boat_size / 2); i ++) {
			pthread_mutex_lock(&hackers_queue_lock);
			dequeue(&hackers_queue);
			pthread_mutex_unlock(&hackers_queue_lock);
			sem_post(&hackers);
		}
		// Revome da fila n / 2 serfs
		for (i = 0; i < (boat_size / 2); i ++) {
			pthread_mutex_lock(&serfs_queue_lock);
			dequeue(&serfs_queue);
			pthread_mutex_unlock(&serfs_queue_lock);
			sem_post(&serfs);
		}
		n_serfs = 0;
		n_hackers = n_hackers - (boat_size / 2);
		is_captain = 1;
	} else {
		pthread_mutex_unlock(&mutex);
	}
	sem_wait(&serfs);
	serfs_on_the_boat ++;
	pthread_barrier_wait(&barrier);
	if (is_captain == 1) {
		rowBoat();
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}


// Le a entrada
// O numero de hackers (0 <= in_hackers <= 10)
// O numero de serfs (0 <= in_serfs <= 10)
int readInput() {
    printf("Digite o numero de Hackers. (MAX: 10)\n");
	scanf("%d", &in_hackers);
	if (in_hackers < 0 || in_hackers > MAX_HACKERS) {
		printf("Numero de Hackers invalido.\n");
		return 1;
	}
    printf("Digite o numero de Serfs. (MAX: 10)\n");
	scanf("%d", &in_serfs);
	if (in_serfs < 0 || in_serfs > MAX_SERFS) {
		printf("Numero de Serfs invalido.\n");
		return 1;
	}
	return 0;
}

// Inicializa os componentes
void init() {

	flag = 0;
	n_hackers = 0;
	n_serfs = 0;
	is_captain = 0;
	boat_size = 4;
	frames = 0;
	status = 0;
	hackers_on_the_boat = 0;
	serfs_on_the_boat = 0;
	boat_x_position = SCREEN_WIDTH - 77;

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&hackers_queue_lock, NULL);
	pthread_mutex_init(&serfs_queue_lock, NULL);

	sem_init(&move_lock, 0, 0);
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
		usleep(200000);
		changeScreen();
		printScreen(); 
	}
    return 0;
}
