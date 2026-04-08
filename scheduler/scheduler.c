#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>


typedef struct Task{
	char name[10];
	int pid;
	int arrival_time;
	char execution_state[8];
}Task;

typedef struct Node{
    Task key;
    struct Node* prev;
    struct Node* next;
}Node;

Node* first = NULL;
Node* last = NULL;

Task current_process;

void insert(struct Task);
void display();
Task extract();
void readFile(struct Task*, char[]);
double get_wtime(void);
void sigchldHandler(int);

int process_ended = 0;

int main(int argc,char **argv){

	// Signals
    struct sigaction sact;

    sact.sa_handler = sigchldHandler;
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;

    if(sigaction(SIGCHLD,&sact,NULL) < 0)
        perror("could not set action for SIGCHLD\n");

	// Pipe for IPC to get child PID
	int fd[2];
	fd[0] = open("write.txt", O_WRONLY);
	fd[1] = open("read.txt", O_RDONLY);
	char buff[20];
	int out = -1;
	pipe(fd);

	
	// Inputs declarations
	Task tasks[10] ={0};
	int quantum = 1000;
	char alg[5] = {0};
	strcpy(alg,"RR");
	//strcpy(alg,argv[1]);
	double t1;


	// Gets the inputs
	// if(argc == 3)
	// 	readFile(tasks,argv[2]);
	// else if(argc == 4){
	// 	quantum = atoi(argv[2]);
	// 	readFile(tasks,argv[3]);	
	// }
	// else{
	// 	printf("Wrong number of arguments.");
	// 	exit(0);
	// }

	// Struct for nanosleep
	struct timespec remaining, request = { 0, 0 }; 
	request.tv_sec = quantum / 1000;
	

	// Read tasks
	readFile(tasks,"reverse.txt");
	
	// Insert tasks in list
	int i = 0;
	while(strcmp(tasks[i].name,"\000") != 0){
		strcpy(tasks[i].execution_state, "NEW");
		current_process=tasks[i];
		insert(tasks[i]);
		i++;
	}
	
	
	// // Iterator
    // Node* ptr = first;


    char path[50] = {0};
	int pid;

    while (first != NULL) 
    {
		// Extraction from list
        current_process = extract();		
		

        // Path for execl
		strcpy(path,"/home/max/project2/work/");
        strcat(path,current_process.name);


        if(strcmp(alg,"RR") == 0)        // RR
        { 
			pid=-2;
			if(strcmp(current_process.execution_state,"NEW") == 0)
				pid = fork();

			if(pid==-1) //fork fail
				exit(0);


            if(pid == 0)         // Paidi
			{
				printf("Dequeue process with name %s\n",current_process.name);
				printf("Executing %s\n",current_process.name);

				if(strcmp(current_process.execution_state,"STOPPED") == 0)
				{
					strcpy(current_process.execution_state, "RUNNING");
					kill(current_process.pid, SIGCONT);
				}
				else
				{
					sprintf(buff,"%d",getpid());
					write(fd[1],buff,sizeof(buff));
					strcpy(current_process.execution_state, "RUNNING");
					execl(path,current_process.name,(char*)NULL);
				}
			}
            else if (pid>0)				// Goneas
            {                     
				nanosleep(&request, &remaining);	
				read(fd[0],buff,sizeof(buff));
				out = atoi(buff);
				current_process.pid = out;
				
				if(strcmp(current_process.execution_state,"EXITED") != 0)
				{
					strcpy(current_process.execution_state, "STOPPED");
					kill(current_process.pid, SIGSTOP);
					insert(current_process);
				}
				else if(strcmp(current_process.execution_state,"EXITED") == 0)
				{
					t1=get_wtime();
					printf("Process %s finished\n",current_process.name);
					printf("	Elapsed time: %lf\n", t1 - current_process.arrival_time);
					printf("	pid: %d\n\n",current_process.pid); 									 //printaroyme lathos pid logo ths grammhs 134
				}
            }
			else if (pid==-2)														// mpainoyme sto else mono otan to current process htan stopped 
			{
				strcpy(current_process.execution_state, "RUNNING");
				kill(current_process.pid, SIGCONT);
				nanosleep(&request, &remaining);

				if(strcmp(current_process.execution_state, "EXITED") != 0)
				{
					strcpy(current_process.execution_state, "STOPPED");
					kill(current_process.pid, SIGSTOP);
					insert(current_process);
				}
				else if(strcmp(current_process.execution_state,"EXITED") == 0)
				{
					t1=get_wtime();
					printf("Process %s finished\n",current_process.name);
					printf("	Elapsed time: %lf\n", t1 - current_process.arrival_time);
					printf("	pid: %d\n\n",current_process.pid); 		
				}
			}
        }
        else            // FCFS
        { 
            pid = fork();
            if(pid == 0)         // Paidi
			{
				printf("Dequeue process with name %s\n",current_process.name);
				printf("Executing %s\n",current_process.name);
				sprintf(buff,"%d",getpid());
				write(fd[1],buff,sizeof(buff));
				strcpy(current_process.execution_state, "RUNNING");
                execl(path,current_process.name,(char*)NULL);
			}
            else				// Goneas
            {                    
				wait(NULL);

				read(fd[0],buff,sizeof(buff));
				out = atoi(buff);
				current_process.pid = out;

				strcpy(current_process.execution_state, "EXITED");
                t1=get_wtime();
                printf("	Elapsed time: %lf\n", t1 - current_process.arrival_time);
				printf("	pid: %d\n\n",current_process.pid);
            }
        }
        //ptr = ptr->next;
    }
	// Closing the pipe
	close(fd[0]);
	close(fd[1]);

	return 0;
}


void sigchldHandler(int signum){
	if(strcmp(current_process.execution_state,"STOPPED") != 0)
		strcpy(current_process.execution_state,"EXITED");
}

void readFile(Task *tasks, char fileName[]){
    char line[50]; // String
    char* last_word; // program name
    int i=-1;

    FILE *apps;
    apps= fopen(fileName, "r"); 
    
    if(apps == NULL){
		printf("Error in reading file.");
		exit(0);
	}

    while (fgets(line,sizeof(line),apps)){
        i++;
        char* token=strtok(line,"/");
        last_word=NULL;

        while(token!=NULL)
        {
            last_word=token;
            token=strtok(NULL,"/");
        }
		int size = strlen(last_word);
        last_word[size-1] = '\0';
        strcpy(tasks[i].name,last_word);
		tasks[i].pid = -1; // initialization
    }
}

Task extract(){
	Task output = first->key;
	if(first->next != NULL){
		first = first->next;
		first->prev = NULL;
		return output;
	}
	else
	{
		first=NULL;
		return output;
	}

}

void insert(struct Task task){
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->key = task;
	newNode->next = NULL;
	if(strcmp(current_process.execution_state,"NEW") == 0)
        newNode->key.arrival_time = get_wtime(); // if inserted after stop

	if(first == NULL){
		newNode->prev = NULL;
		first = last = newNode;
	}else{
		newNode->prev = last;
		last->next = newNode;
		last = newNode;
	}
}

void display() {
    Node* ptr = first;
    while (ptr != NULL) {
        printf("%s %d %d %s\n", ptr->key.name, ptr->key.pid, ptr->key.arrival_time, ptr->key.execution_state);
        ptr = ptr->next;
    }
	free(ptr);
}

double get_wtime(void) 
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
}