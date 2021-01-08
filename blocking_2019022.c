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

int think();
int eat();

struct new_semaphore{
    int count;//Maintains count of people, acts as the counter for the counting semaphore
    pthread_mutex_t flaglock;//Mutex lock used to lock the semaphore when <=0 count val
    pthread_cond_t condlock;//Conditional Variable lock
};

typedef struct new_semaphore my_semaphore;//Renaming h=the structure to my_semaphore

int numbering[5] = {0,1,2,3,4};
int numberOfEats[5];
my_semaphore roomlock;//Roomlock semaphore, which only allows 4 people at a time to avoid deadlocks
my_semaphore forks[5];//Binary semaphores for forks, denoting whether it is picked up or not
my_semaphore saucebowl1;//Binary Semaphores for the two saucebowls
my_semaphore saucebowl2;

/*The following is the declaration of the wait primitive for my_semaphore. It takes in the semaphore that needs to wait,
and then checks if the semaphore count is currently <=0. If it is, then it puts it into a conditional wait stage. If it isn't,
then the semaphore count is decreased by 1, basically allowing the philosopher into the room.*/

int my_wait(my_semaphore *sem){
    if(pthread_mutex_lock(&(sem->flaglock))!=0){
        return -1;
    }
    while(sem->count<=0){
        pthread_cond_wait(&(sem->condlock),&(sem->flaglock));
    };
    sem->count-=1;
    pthread_mutex_unlock(&(sem->flaglock));
    return 0;
}

/*The following is the declaration for the signal primitive for my_semaphore. It again takes the semaphore that needs to 
be signalled to. On aquring, it first locks the flaglock to avoid race condition, and then increases the counting 
semaphore's count by 1. Additionally, if the count reaches 1, then the conditional variables are signalled to send through.*/

int my_signal(my_semaphore *sem){
    if(pthread_mutex_lock(&(sem->flaglock))!=0){
        return -1;
    }
    sem->count+=1;
    if(sem->count == 1)
    {
        pthread_cond_signal(&(sem->condlock));
    }
    pthread_mutex_unlock(&(sem->flaglock));
    return 0;
} 

/*Pretty much same as above, except that it has additional capability to return current occupancy of the semaphore.*/

int my_signal_printvalue(my_semaphore *sem){
    if(pthread_mutex_lock(&(sem->flaglock))!=0){
        return -1;
    }
    sem->count+=1;
    int x = sem->count;
    if(sem->count == 1)
    {
        pthread_cond_signal(&(sem->condlock));
    }
    pthread_mutex_unlock(&(sem->flaglock));
    return x;
}

/*To initialize the semaphore, takes the value.*/

int sem_init(my_semaphore *sem,int val){
    if(pthread_mutex_init(&(sem->flaglock),NULL)!=0){
        return -1;
    }
    if(pthread_cond_init(&(sem->condlock),NULL)!=0){
        return -1;
    }
    sem->count = val;
    return 0;
}

/*The philosopher thread uses the roomlock counting semaphore, which allows at max 4 people in the room simultaneously
The left and right forks are also binary semaphores*/
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

/*The saucebowls are held inside binary semaphores. The second saucebowl is nested inside the first one's critical section to 
avoid deadlocks between threads.*/
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

	sem_init(&roomlock,4);
	for(int i=0;i<5;i++){
        numberOfEats[i] = 0;
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