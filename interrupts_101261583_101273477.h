#ifndef ASSIGNMENT3_H
#define ASSIGNMENT3_H

#define CHARACTERS_MAX 50 //the max characters a line can have in input and vector table file. 
#define MAX_EVENTS 500 //the max number of events in input file
#define FIXED_PARTITIONS_SIZE 6  //the size of the array for the 6 fixed partition sizes

typedef struct //This will be the structure for the partitioned memory having and id, size and code name (init, free or a program name)
{
    unsigned int patritionNumber;
    unsigned int size;
    unsigned int occupiedBy;

} memoryPartions;


typedef struct //This will be the structure for the PCB 
{
    unsigned int pid;
    unsigned int memorySize; 
    unsigned int arrivalTime;
    unsigned int totalCPUTime;
    unsigned int ioFrequency;
    unsigned int ioTimeDuration;
    unsigned int partitionNumber;
    unsigned int countIOFrequencyDelay; 
    unsigned int sliceTime;
    char state[25];

} PCB; 

typedef struct //This will be the structure for a state change table
{
    unsigned int timeOfTransition; //when the event change occured
    unsigned int PID; //the PID of the PCB
    char oldState[25]; //the previous state
    char newState[25]; //the new state
} stateChangeTable; 

typedef struct //This will be the structure for a memory status table
{
    unsigned int timeOfEvent;  //when a proceess has eneter or left memory 
    unsigned int memoryUsed; //the memeory being used 
    unsigned int partitionState[6]; //the partiton states 
    unsigned int totalFreeMemory; //the free amount of memeory
    unsigned int usableFreeMemory; //the usable amount of memeory 
} memoryStatusTable;
 
typedef struct queueNode { //This will be a queue node structure that will have all of the PCB's in a queue line
    PCB pcbData; //a PCB data
    struct queueNode *next; //pointer to the next node
} queueNode;

typedef struct{ //This will be the queue its self having a pointer to the first node and rear pointing to the last node 
    queueNode *front; //pointer to the front node
    queueNode *rear; //pointer to the rear node
    int size; //keeps the size of the queue
} PCBQueue;

PCBQueue *importInputFile(const char *filename);
void FCFS_Scheduler(PCBQueue *queue, memoryPartions *fixedPartitions, stateChangeTable *stateCT, memoryStatusTable *memoryST);
void NOPREMPT_Scheduler(PCBQueue *queue, memoryPartions *fixedPartitions, stateChangeTable *stateCT, memoryStatusTable *memoryST);
void RR_Scheduler(PCBQueue *queue, memoryPartions *fixedPartitions, stateChangeTable *stateCT, memoryStatusTable *memoryST);
void PCBReadyToRunning(PCBQueue *queue, memoryPartions fixedPartitions[], stateChangeTable stateCT[], memoryStatusTable memoryST[], int typeOfScheduling);
void PCBNewToReady(PCBQueue *queue, memoryPartions fixedPartitions[], stateChangeTable stateCT[], memoryStatusTable memoryST[], int time);
void PCBRunningToWaiting(queueNode *tempNode, PCBQueue *waitingQueue, stateChangeTable stateCT[], int time);
void PCBRunningToTerminated(queueNode *tempNode, memoryPartions fixedPartitions[], stateChangeTable stateCT[], memoryStatusTable memoryST[], int time);
void logStateChange(stateChangeTable stateCT[], int time, int pid, const char *oldState, const char *newState);
void logMemoryStatus(memoryStatusTable memoryST[], memoryPartions fixedPartitions[], int time, int memoryChange, int wasAdded);
void memoryStatusFunction(memoryStatusTable memoryST[]);
void executionFunction(stateChangeTable stateCT[]);
void countIOfrequencyDelay(PCBQueue *waitingQueue, PCBQueue *mainQueue, stateChangeTable stateCT[], int time, int increment);
void enqueue(PCBQueue *queue, PCB PCBData);
PCB dequeue(PCBQueue *queue);
void removeNodeFromQueue(PCBQueue *queue, int pid);
void sortQueue(PCBQueue *queue, int typeOfSchduling);
queueNode *getPriorityPCB(PCBQueue *queue, int time, int typeOfScheduling);

#endif
