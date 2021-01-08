/*
    Assignment - 4
    Name: Ansh Arora
    Roll No: 2019022
*/

#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdatomic.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>  
#include <pthread.h> 

/*In non blocking, the major difference is that we cannot use lock to block, we can only use infomative details from the used locks*/

int think();
int eat();

struct new_semaphore{
    int count;
    pthread_mutex_t flaglock;
    //pthread_cond_t condlock;
};
typedef struct new_semaphore my_semaphore;

int numbering[5] = {0,1,2,3,4};
int state[5];
int numberOfEats[5];
my_semaphore roomlock;
my_semaphore forks[5];
my_semaphore saucebowl1;
my_semaphore saucebowl2;


//Here, the conditional variable waiting queues have been removed.
int my_wait(my_semaphore *sem){
    if(pthread_mutex_trylock(&(sem->flaglock))==0){;//trylock
        sem->count-=1;
        pthread_mutex_unlock(&(sem->flaglock));
    }
    else{
        return -1;//ERROR HANDLING
    }
    return 0;
}

int my_signal(my_semaphore *sem){
    if(pthread_mutex_trylock(&(sem->flaglock))==0){
        sem->count+=1;
        pthread_mutex_unlock(&(sem->flaglock));
    }
    else{
        return -1;//ERROR HANDLING
    }
    return 0;
} 

int my_signal_printvalue(my_semaphore *sem){
    int x = -1;//ERROR HANDLED
    if(pthread_mutex_trylock(&(sem->flaglock))==0){
        sem->count+=1;
        x = sem->count;
        pthread_mutex_unlock(&(sem->flaglock));
    }
    return x;
}

int sem_init(my_semaphore *sem,int val){
    if(pthread_mutex_init(&(sem->flaglock),NULL)!=0){
        return -1;//ERROR HANDLING
    }
    sem->count = val;
    return 0;
}

void * philo_thread(void *args){
	int * philo_num = (int *) args;
	while(1){
		if(numberOfEats[*philo_num]>=10000){
			break;
		}
        think();

		my_wait(&roomlock);
        printf("Philosopher %d has entered room.\n",*philo_num);
        int l = *philo_num;
        int r = ((*philo_num)+1)%5;
        my_wait(&forks[l]);
        my_wait(&forks[r]);

        numberOfEats[*philo_num]+=1;
        printf("Philosopher %d is eating with forks %d and %d. Meal #%d\n",(*philo_num)+1,*philo_num+1,(((*philo_num)+1)%5)+1,numberOfEats[*philo_num]);
        eat(l+1);
        printf("Philosopher %d has finished eating.\n",(*philo_num)+1);

        my_signal(&forks[l]);
        my_signal(&forks[r]);
        my_signal(&roomlock);
	}
    return NULL;
}

int think(){
	sleep(0);
    return 0;
}

int eat(int num){
    my_wait(&saucebowl1);
    my_wait(&saucebowl2);
    printf("Philosopher %d has picked saucebowls.\n",num);
	sleep(0);
    printf("Philosopher %d has put down saucebowls.\n",num);
    my_signal(&saucebowl2);
    my_signal(&saucebowl1);
    return 0;
}

int main(void) {
	pthread_t philopshers[5];
    sem_init(&saucebowl1,1);
    sem_init(&saucebowl2,1);

	for(int i=0;i<5;i++){
		state[i] = 0;
		numberOfEats[i] = 0;
	}

	sem_init(&roomlock,4);
	for(int i=0;i<5;i++){
		sem_init(&forks[i],1);
	}

	for(int i=0;i<5;i++){
		pthread_create(&philopshers[i],NULL,philo_thread,&numbering[i]);
		//printf("Philosopher %d is thinking. \n",i+1);
	}

	for(int i=0;i<5;i++){
		pthread_join(philopshers[i],NULL);
	}

	for(int i=0;i<5;i++){
		printf("Philosopher %d had %d meals.\n",i+1,numberOfEats[i]);
	}
    
	return 0;
}