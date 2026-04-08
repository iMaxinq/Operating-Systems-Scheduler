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

// Working process
Task current_process;
Task io_process;
int io_state=0;

// Head & Tail of list
Node* first = NULL;
Node* last = NULL;

// Head & Tail of priority list
Node* prio_first = NULL;
Node* prio_last = NULL;


// Functions
void insert(struct Task, Node**, Node**);
Task extract(Node**);
void readFile(struct Task*, char[]);
double get_wtime(void);
void sigchldHandler(int);
void sigIOHandler1(int);
void sigIOHandler2(int);
void FCFS(char[80]);
void RR(char[80],int);
void printTime();


int main(int argc,char **argv)
{
	
	// Signals
	struct sigaction sact, ioSig1, ioSig2;

	sact.sa_handler = sigchldHandler;
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = SA_NOCLDSTOP;

	ioSig1.sa_handler = sigIOHandler1;
	sigemptyset(&ioSig1.sa_mask);
	ioSig1.sa_flags = 0;

	ioSig2.sa_handler = sigIOHandler2;
	sigemptyset(&ioSig2.sa_mask);
	ioSig2.sa_flags = SA_RESTART;

	
	if(sigaction(SIGCHLD, &sact, NULL) < 0)
		perror("could not set action for SIGCHLD\n");

	if(sigaction(SIGUSR1, &ioSig1, NULL) < 0)
		perror("could not set action for SIGUSR1\n");	

	if(sigaction(SIGUSR2, &ioSig2, NULL) < 0)
		perror("could not set action for SIGUSR2\n");		

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
	while(strcmp(tasks[i].name,"\000") != 0)
	{
		strcpy(tasks[i].execution_state, "NEW");
		insert(tasks[i], &first, &last);
		i++;
	}
	
    char path[80] = {0};

    while ((first != NULL) || (prio_first !=NULL || io_state)) 
    {
		if(io_state==1 && (first == NULL) && (prio_first == NULL)){continue;}
		else
		{
			// Extraction from list
			if(prio_first==NULL)
				{current_process = extract(&first);}                        
			else
				{current_process = extract(&prio_first);}  

			// Path for execl
			strcpy(path,"/home/max/project2/work/");
			strcat(path,current_process.name);
		
			if(strcmp(alg,"RR") == 0)        // RR
				RR(path,quantum);

			else if(strcmp(alg,"FCFS") == 0)           // FCFS
				FCFS(path);
    	}
	}
	return 0;
}


void sigchldHandler(int signum)
{
	strcpy(current_process.execution_state,"EXITED");
}

void sigIOHandler1(int signum) //SIGUSR1
{
	io_state=1;
}

void sigIOHandler2(int signum) //SIGUSR2 
{
	printf("Inserted to priority list\n");
	insert(io_process, &prio_first, &prio_last);
	io_state=0;
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

Task extract(Node** head)
{
	Task output = (*head)->key;
	if((*head)->next != NULL){
		*head = (*head)->next;
		(*head)->prev = NULL;
		return output;
	}else
    {
		*head = NULL;
		return output;
	}
}

void insert(struct Task task, Node** head, Node** tail)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->key = task;
	newNode->next = NULL;
	if(strcmp(task.execution_state,"NEW") == 0) // if inserted after stop 
		newNode->key.arrival_time = get_wtime();

	if(*head == NULL){
		newNode->prev = NULL;
		*head = *tail = newNode;
	}else{
		newNode->prev = *tail;
		(*tail)->next = newNode;
		*tail = newNode;
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
	if(strcmp(current_process.execution_state,"NEW") == 0)
	{	
		int pid = fork();

		if(pid == -1) // Fork failure
			exit(0);

		if(pid == 0)         // Paidi
		{
			printf("Dequeue process with name %s\n",current_process.name);
			printf("Executing %s\n",current_process.name);

			strcpy(current_process.execution_state, "RUNNING");
			execl(path,current_process.name,(char*)NULL);
		}
		else if(pid>0)				// Goneas
		{                    
			current_process.pid = pid;
			if(wait(NULL)==-1) // elegxoyme an diakopei apo shma
			{
				strcpy(current_process.execution_state, "IO"); 
				io_process=current_process;
			}
			else				
			{
				printTime();
			}
		}
	}
	else 			//mpainoume edw mono otan exoyme task poy proerxetai apo to priority list
	{
		printf("Dequeue process with name %s\n",current_process.name);
		printf("Executing %s\n",current_process.name);
		strcpy(current_process.execution_state, "RUNNING");
		kill(current_process.pid, SIGCONT);
		wait(NULL);

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
				current_process.pid = pid;

				if(nanosleep(&request, &remaining)==-1 && (io_state==1)&& strcmp(current_process.execution_state,"EXITED")!=0)	 // an termatisei apo signal 
				{
					strcpy(current_process.execution_state, "IO"); 
					io_process=current_process;
				}
				else	// h nanosleep termatizei kanonika
				{
					if(io_state==0 && strcmp(current_process.execution_state,"EXITED") != 0)
						nanosleep(&remaining, &remaining);

					if(strcmp(current_process.execution_state,"EXITED") == 0) //tha exei oristei ws exited apo ton handler
					{
						printTime();
					}
					else if(strcmp(current_process.execution_state,"EXITED") != 0)
					{
						strcpy(current_process.execution_state, "STOPPED");
						kill(current_process.pid, SIGSTOP);
						insert(current_process, &first, &last);
					}
				}
				
			}
		}
		else // Mpainoyme sto else mono otan to current process htan stopped h IO 
		{
			strcpy(current_process.execution_state, "RUNNING");
			printf("Dequeue process with name %s\n",current_process.name);
			printf("Executing %s\n",current_process.name);
			kill(current_process.pid, SIGCONT);
			
			if(nanosleep(&request, &remaining)==-1 && (io_state==1)&& strcmp(current_process.execution_state,"EXITED")!=0)	 // an termatisei apo signal 
			{
				strcpy(current_process.execution_state, "IO"); 
				io_process=current_process;
			}
			else
			{
				if(io_state==0 && strcmp(current_process.execution_state,"EXITED") != 0)
					nanosleep(&remaining, &remaining);

				if(strcmp(current_process.execution_state,"EXITED") == 0)
				{
					printTime();
				}
				else if(strcmp(current_process.execution_state, "EXITED") != 0)
				{
					strcpy(current_process.execution_state, "STOPPED");
					kill(current_process.pid, SIGSTOP);
					insert(current_process, &first, &last);
				}
			}
		}
}

void printTime()
{
	printf("Process %s finished\n",current_process.name);
	printf("	Elapsed time: %lf\n", get_wtime() - current_process.arrival_time);
	printf("	pid: %d\n\n",current_process.pid); 		
}