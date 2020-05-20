#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define M 10

pthread_barrier_t thread_barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void * pthread_function(void * arg){
    int rnd = (rand() % 4);
    int length, check;
    int fd = *((int*) arg);
    pthread_t tid = pthread_self();
 
    char output[sizeof(pthread_t)];
    sprintf (output,"%lx", tid);
    char tmp [2];
    sprintf(tmp,"%d", rnd);
    
    sleep(rnd);

    check = pthread_mutex_lock(&mutex);
	
    char * t1 = "\nfase 1, thread id= ";
    length = strlen(t1);
    write(fd, t1, length);
    write(fd, output, sizeof(pthread_t));

    char * t2 = ", sleep period= ";
    length = strlen(t2);
    write(fd, t2, length);
    write(fd, tmp, sizeof(tmp)-1);
	
    check = pthread_mutex_unlock(&mutex);

    check = pthread_barrier_wait(&thread_barrier);
    
    check = pthread_mutex_lock(&mutex);
	
    char * t3 = "\nfase 2, thread id= ";
    length = strlen(t3);
    write(fd, t3, length);
    write(fd, output, sizeof(pthread_t));

    char * t4 = ", dopo la barriera";
    length = strlen(t4);
    write(fd, t4, length);

    struct timespec t;
    struct timespec remaining;
    t.tv_sec = 0;  // seconds
    t.tv_nsec = 10; // nanoseconds
    if (nanosleep(&t, &remaining) == -1) {
        perror("nanosleep");
    }

    char * t5 = "\nthread id= ";
    length = strlen(t5);
    write(fd, t5, length);
    write(fd, output, sizeof(pthread_t));

    write(fd, "\nbye", 4);

    check = pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char * argv[]) {
    int res;
    pthread_t threads[M];
    int * fd = malloc(sizeof(int));

    *fd = open("file.txt",
			  O_CREAT | O_TRUNC | O_WRONLY,
			  S_IRUSR | S_IWUSR 
			 );

    res = pthread_barrier_init(&thread_barrier, NULL, M); 
    if(res != 0) {
            perror("pthread_barrier_init");
            exit(EXIT_FAILURE);	
    }
    
    for(int i = 0; i < M; i++) {
        res = pthread_create(&threads[i], NULL, pthread_function, fd);
        if(res != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for(int i=0; i < M; i++) {
		res = pthread_join(threads[i], NULL);
		if (res != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
    }

    res = pthread_barrier_destroy(&thread_barrier);
    if(res != 0) {
            perror("pthread_barrier_destroy");
            exit(EXIT_FAILURE);
    }

    return 0;
}
