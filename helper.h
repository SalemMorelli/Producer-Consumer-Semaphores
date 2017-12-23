/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, as well as the function signatures and
 * the semaphore values (which are to be changed as needed).
 ******************************************************************/


# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/time.h>
# include <math.h>
# include <errno.h>
# include <string.h>
# include <pthread.h>
# include <ctype.h>
# include <iostream>
# include <time.h> 
# include <semaphore.h>
# include <cerrno> 
using namespace std;

# define SEM_KEY 0x543932 

struct Job {

    int id;                
    int duration;          

};


struct Producer_info {    //Producer_Data

    int producerid;  //producer_id
    int num_of_jobs;
};


union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

int check_arg (char *);
int sem_create (key_t, int);
int sem_init (int, int, int);
void sem_wait (int, short unsigned int);
void sem_signal (int, short unsigned int);
int sem_close (int);
int sem_timedwait(int, short unsigned int, int);
//Function that contains a struct and produces a job with an id and duration
struct Job produce_job(int); 


