#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#define BUFFER_SIZE 6
#define NUMBER_OF_PRODUCERS 4
#define NUMBER_OF_CONSUMERS 3

#define TOTAL_PRODUCTS_TO_PRODUCED 120
#define TOTAL_PRODUCTS_TO_CONSUMED 120
#define MAX_RANDOM_PRODUCTS 1000000
#define ERROR_CODE 255
#define PRODUCER_TYPE 1
#define CONSUMER_TYPE 0

int buf[BUFFER_SIZE];
int empty;
int full;
sem_t sem_full;
sem_t sem_empty;


pthread_mutex_t mutex_thread;


int create_random_product(int max_number) {
    return rand() % max_number;
}

void add_to_buffer(int item) {
    buf[empty] = item;
    empty = (empty + 1) % BUFFER_SIZE;
}

void print_product(int thread_type, int item, int thread_number) {
    if (thread_type == PRODUCER_TYPE) {
        printf("[P%d] Producing %d \n", thread_number, item);
    } else if (thread_type == CONSUMER_TYPE) {
        printf("[C%d] Consuming  %d\n", thread_number, item);
    }
}

int remove_from_buffer() {
    int item = buf[full];
    full = (full + 1) % BUFFER_SIZE;
    return item;
}

void *producer(void *arg) {
    int i, index, item;
    index = *(int *) arg;

    for (i = 0; i < TOTAL_PRODUCTS_TO_PRODUCED / NUMBER_OF_PRODUCERS; i++) {
        sem_wait(&sem_empty);//if the buffer is full and no empty slots as wait
        pthread_mutex_lock(&mutex_thread);

        item = create_random_product(MAX_RANDOM_PRODUCTS);
        add_to_buffer(item);
        print_product(PRODUCER_TYPE, item, index);

        pthread_mutex_unlock(&mutex_thread);
        sem_post(&sem_full);
    }
    return NULL;
}

void *consumer(void *arg) {
    int i, item, index;

    index = *(int *) arg;
    for (i = TOTAL_PRODUCTS_TO_CONSUMED / NUMBER_OF_CONSUMERS; i > 0; i--) {

        sem_wait(&sem_full);
        pthread_mutex_lock(&mutex_thread);

        item = remove_from_buffer();
        print_product(CONSUMER_TYPE, item, index);

        pthread_mutex_unlock(&mutex_thread);
        sem_post(&sem_empty);

    }
    return NULL;
}

int main() {
    pthread_t producer_id[NUMBER_OF_PRODUCERS];
    pthread_t consumer_id[NUMBER_OF_CONSUMERS];
    int index;

    sem_init(&sem_full, 0, 0);
    sem_init(&sem_empty, 0, BUFFER_SIZE);
    pthread_mutex_init(&mutex_thread, NULL);
    srand(time(NULL));

    for (index = 0; index < NUMBER_OF_PRODUCERS; ++index) {
        if (pthread_create(&producer_id[index], NULL, producer, (void *) &index)) {
            perror("Pthread is not created");
        }
    }
    /*create a new consumer*/
    for (index = 0; index < NUMBER_OF_CONSUMERS; ++index) {

        if (pthread_create(&consumer_id[index], NULL, consumer, (void *) &index)) {
            perror("Pthread is not created");
            exit(ERROR_CODE);
        }
    }

    for (index = 0; index < NUMBER_OF_CONSUMERS ; ++index)
        pthread_join(consumer_id[index], NULL);

    for (index = 0; index < NUMBER_OF_PRODUCERS; ++index)
        pthread_join(producer_id[index], NULL);

    pthread_exit(NULL);
    return 0;
}