/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************///
#include "helper.h" 

//Global Variables
#define space 1
#define full 2  
int semid;
int in = 0;   
int out = 0;  
struct Job *buffer;
struct Job job;
struct Producer_info producer_info;
int parameter;
unsigned int size_of_queue;
int consumed;
int totalJobs; 
pthread_mutex_t mutex;

//Producer & Consumer Function declarations
void *producer (void *parameter);
void *consumer (void *id);
bool removeJob(Job *job);
bool insertJob(Job job);

//Main Program
int main (int argc, char **argv)
{
    if (argc != 5){

        cout<< "ERROR: Input exactly 4 arguments.\n";
        return 0;
    };

    //Checking command line arguments
    size_of_queue = check_arg(argv[1]);
    int num_of_jobs = check_arg(argv[2]);
    int num_producers = check_arg(argv[3]);
    int num_consumers = check_arg(argv[4]);

    //Obtaining command line arguments   
    size_of_queue = atoi(argv[1]); 
    num_of_jobs = atoi(argv[2]);
    num_producers = atoi(argv[3]); 
    num_consumers = atoi(argv[4]); 

    //creating  and initalise semaphores
    semid = sem_create(SEM_KEY, 3);
    int sem1 = pthread_mutex_init(&mutex, NULL);
    int sem2 = sem_init(semid, space, size_of_queue); 
    int sem3 = sem_init(semid, full, 0);

    cout<<"_______________________Program Started_________________________"<<endl<<endl;

    if(semid == -1){

        cerr<<"ERROR 1: Semaphore creation failure. Error value is: "<<semid<<endl;
    }
    else
        cout<<"SEMAPHORE ID: "<<semid<<endl<<endl; 

    if(sem1 != 0 || sem2 != 0 || sem3 != 0){

        cerr<<"ERROR 2: Semaphore initialisation failure."<<endl<<endl;
    }

    consumed = 0;
    totalJobs = num_producers * num_of_jobs;
    srand(time(NULL));  //generates a random number everytime the computer run the program 
    buffer = new struct Job [size_of_queue]; 
    //referencing each thread created
    pthread_t producerid[num_producers];       
    pthread_t consumerid[num_consumers];       


    struct Producer_info producer_info[num_producers];   //making an struct array
    int consumer_id[num_consumers]; 

    //creating a thread for each producer 
    for(int i = 0; i< num_producers; i++){

        producer_info[i].producerid = i + 1;    //creates array of ID's
        producer_info[i].num_of_jobs = num_of_jobs; 

        int t = pthread_create (&producerid[i], NULL, producer, &producer_info[i]);
        if(t != 0){

            cerr<<"ERROR: Failure to create producer pthreads. Error value is: ";
            cerr<<t<<endl;
        }
    };

    //creating a thread for each consumer
    for(int j = 0; j< num_consumers; j++){

        consumer_id[j] = j + 1;   //creates array of ID's 
        int k =  pthread_create (&consumerid[j], NULL, consumer, &consumer_id[j]);

        if(k != 0){

            cerr<<"ERROR: Failure to create consumer pthreads. Error value is: ";
            cerr<<k<<endl;
        }

    };

    for(int i = 0; i< num_producers; i++){

        int p = pthread_join (producerid[i], NULL);

        if(p != 0){

            cerr<<"ERROR: Failure to join pthreads. Error value is: "<<p<<endl;
            cerr<<" for producers."<<endl;

        }
    };
    for(int j = 0; j< num_consumers; j++){

        int c = pthread_join (consumerid[j], NULL);
        if(c != 0){

            cerr<<"ERROR: Failure to join pthreads. Error value is: "<<c;
            cerr<<" for consumers."<<endl; 

        }
    };

    int semclose = sem_close(semid);

    if(semclose != 0){

        cerr<<"ERROR: Semaphores not closing. Error value is: "<<semclose<<endl;
    }

    cout<<endl;
    cout<<"____________________________End of Program___________________________"<<endl;

    return 0;
}

//---------------------------------------------------------------------------
void *producer(void *parameter) 
{
    struct Producer_info *producer_info = (struct Producer_info*) parameter;
    struct Job job; 

    for (int i =0; i< producer_info->num_of_jobs; ++i){ 

        if(sem_timedwait(semid, space, 20) == -1){    //waiting on signal from consumer

            cerr<< "Producer("<< producer_info->producerid <<") Buffer is full and no";
            cerr<<" jobs are being consumed within 20 seconds - TERMINATION"<<endl;

            pthread_exit(0);
        }    

        pthread_mutex_lock(&mutex);    //ENTERING CRITICAL SECTION

        job = produce_job(in);         //produces job 
        sleep(rand() % 5 + 1);         //waits between 1-5 seconds 
        if(insertJob(job) != 1){      //inserts job into buffer
            cerr<<"ERROR: Cannot insert job into buffer."<<endl;
        }

        cerr<< "Producer("<< producer_info->producerid << "): Job id (" << job.id<<")";
        cerr<< " to be executed in "<< job.duration <<" seconds. "<<endl;

        pthread_mutex_unlock(&mutex);  //EXITING CRITICAL SECTION

        sem_signal(semid, full);     //signal to consumer that there is a job in buffer 
    }

    cerr<<"Producer("<<producer_info->producerid<<"): No more jobs to produce."<<endl;

    pthread_exit(0);
} 

//---------------------------------------------------------------------------------//
void *consumer (void *id) 
{  
    int *consumer_id = (int*) id;
    Job job;

    while(1){

        if(sem_timedwait(semid, full, 20) == -1){   //waiting on signal from producer
            cerr<<"Consumer("<< *consumer_id <<") No jobs are being produced within";
            cerr<<" 20 seconds - TERMINATION"<<endl;
            pthread_exit(0);

        }
        pthread_mutex_lock(&mutex);    //ENTERING CRITICAL SECTION

        if(removeJob(&job) != 1 ){      //removing/consuming job
            cerr<< "ERROR: Job cannot be removed."<<endl;

        }
        cerr<<"Consumer("<<*consumer_id <<"): Job id ("<< job.id<<")";
        cerr<<" executing job within "<< job.duration <<" seconds."<<endl;

        pthread_mutex_unlock(&mutex);   //EXITING CRITICAL SECTION
        sem_signal(semid, space);       //signal empty space to producer
        sleep(job.duration);           //consume job for 'duration' before exiting 

        cerr<<"Consumer("<<*consumer_id <<"): Job id ("<<job.id<<") COMPLETED."<<endl; 
    }

    pthread_exit(0);
}  

//---------------------------------------------------------------------------------//
//Function describing removing a job from the buffer. Takes in a pointer to a job. Returns 1 on success.
bool removeJob(Job *job){
    *job = buffer[out];
    out = (out + 1) % size_of_queue;  //ensures a circular queue
    return 1;
}

//---------------------------------------------------------------------------------//
//Function decribing inserting a job into the buffer. Takes in a job as an argument. Returns 1 on success.
bool insertJob(Job job){
    buffer[in] = job;
    in = (in + 1) % size_of_queue;   //ensures a  circular queue
    return 1;
}

