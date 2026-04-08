/* Spiridon Papageorgiou, 1093464 */
/* Maximos Frountzos, 1093509 */
/* Kwnstantinos Papaspyrou, 1093470 */
/* Kwnstantinos Koukouvelas, 1093396 */

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

// Perigrafeis efarmogon
typedef struct Task{
	char name[10];
	int pid;
	int arrival_time;
	char execution_state[8];
}Task;

// List nodes
typedef struct Node{
    Task key;
    struct Node* prev;
    struct Node* next;
}Node;

// Head & Tail of list
Node* first = NULL;
Node* last = NULL;

// Working process
Task current_process;

// Functions
void insert(struct Task);
Task extract();
void readFile(struct Task*, char[]);
double get_wtime(void);
void sigchldHandler(int);
void FCFS(char[80]);
void RR(char[80],int);
void printTime();


int main(int argc,char **argv){

	// Signals
	struct sigaction sact;

	sact.sa_handler = sigchldHandler;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = SA_NOCLDSTOP;
	
	if(sigaction(SIGCHLD,&sact,NULL) < 0)
		perror("could not set action for SIGCHLD\n");

	// Inputs declarations
	Task tasks[15] ={0};
	int quantum;
	char alg[10] = {0};
	strcpy(alg,argv[1]);

	// Gets the inputs
	if(argc == 3 && strcmp(alg,"FCFS")==0)
		readFile(tasks,argv[2]);
	else if(argc == 4 && strcmp(alg,"RR")==0){
		quantum = atoi(argv[2]);
		readFile(tasks,argv[3]);	
	}
	else{
		printf("Wrong arguments.\n");
		exit(0);
	}


	// Insert tasks in list
	int i = 0;
	while(strcmp(tasks[i].name,"\000") != 0){
		strcpy(tasks[i].execution_state, "NEW");
		insert(tasks[i]);
		i++;
	}
	

    char path[80] = {0};

    while (first != NULL) 
    {
		// Extraction from list
        current_process = extract();		
		
        // Path for execl
		strcpy(path,"/home/max/project2/work/");
        strcat(path,current_process.name);

        if(strcmp(alg,"RR") == 0)        // RR 
			RR(path,quantum);

        else if(strcmp(alg,"FCFS") == 0)       // FCFS
			FCFS(path);
    }
	return 0;
}


void sigchldHandler(int signum)
{
	strcpy(current_process.execution_state,"EXITED");
}


void readFile(Task *tasks, char fileName[])
{
    char line[50]; // String
    char* last_word; // program name
    int i=-1;

    FILE *apps;
    apps= fopen(fileName, "r"); 
    
    if(apps == NULL){
		printf("Error in reading file.\n");
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

Task extract()
{
	Task output = first->key;
	if(first->next != NULL){
		first = first->next;
		first->prev = NULL;
		return output;
	}else{
		first = NULL;
		return output;
	}
}

void insert(struct Task task)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->key = task;
	newNode->next = NULL;
	if(strcmp(task.execution_state,"NEW") == 0) // if inserted after stop 
		newNode->key.arrival_time = get_wtime();

	if(first == NULL){
		newNode->prev = NULL;
		first = last = newNode;
	}else{
		newNode->prev = last;
		last->next = newNode;
		last = newNode;
	}
}

double get_wtime(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
}


void FCFS(char path[80])
{
    int pid = fork();
    if(pid == 0)         // Paidi
	{
		printf("Dequeue process with name %s\n",current_process.name);
		printf("Executing %s\n",current_process.name);
        execl(path,current_process.name,(char*)NULL);
	}
    else				// Goneas
    {                    
		strcpy(current_process.execution_state, "RUNNING");
		wait(NULL);
		current_process.pid = pid;
        printTime();
    }
}

void RR(char path[80], int quantum)
{
		// Struct for nanosleep
	struct timespec remaining, request = { 0, 0 }; 
	if(quantum >= 1000)
		request.tv_sec = quantum / 1000;
	else
		request.tv_nsec = quantum * 1000000;
	
	if(strcmp(current_process.execution_state,"NEW") == 0)
	{
		int pid = fork();
		
		if(pid == -1) // Fork failure
			exit(0);

		if(pid == 0)         // Paidi
		{
			printf("Dequeue process with name %s\n",current_process.name);
			printf("Executing %s\n",current_process.name);

			execl(path,current_process.name,(char*)NULL);
			
		}
		else if (pid>0)				// Goneas
		{                     
			strcpy(current_process.execution_state, "RUNNING");
			nanosleep(&request, &remaining);	
			current_process.pid = pid;
			
			if(strcmp(current_process.execution_state,"EXITED") == 0)
			{
				printTime();
			}
			else if(strcmp(current_process.execution_state,"EXITED") != 0)
			{
				strcpy(current_process.execution_state, "STOPPED");
				kill(current_process.pid, SIGSTOP);
				insert(current_process);
			}
		}
	}
	else // Mpainoyme sto else mono otan to current process htan stopped 
	{
		strcpy(current_process.execution_state, "RUNNING");
		printf("Dequeue process with name %s\n",current_process.name);
		printf("Executing %s\n",current_process.name);
		kill(current_process.pid, SIGCONT);
		nanosleep(&request, &remaining);

		if(strcmp(current_process.execution_state,"EXITED") == 0)
		{
			printTime();
		}
		else if(strcmp(current_process.execution_state, "EXITED") != 0)
		{
			strcpy(current_process.execution_state, "STOPPED");
			kill(current_process.pid, SIGSTOP);
			insert(current_process);
		}
	}
}

void printTime()
{
	printf("Process %s finished\n",current_process.name);
	printf("	Elapsed time: %lf\n", get_wtime() - current_process.arrival_time);
	printf("	pid: %d\n\n",current_process.pid); 		
}