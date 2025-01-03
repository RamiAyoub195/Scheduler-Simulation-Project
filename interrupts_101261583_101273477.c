#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "interrupts_101261583_101273477.h"

int main(int argc, char **argv){

    //Initializes the fixed partitions at the begining to be free as -1 in the begigning and when occupied represented by the pid number 
    memoryPartions fixedPartitions[FIXED_PARTITIONS_SIZE] = {{1, 40, -1}, {2, 25, -1}, {3, 15, -1}, {4, 10, -1}, {5, 8, -1}, {6, 2, -1}};

    //Initializes the memory status table which will be used to show the memory status output file each time a PCB enters or leaves memory 
    memoryStatusTable memoryST[MAX_EVENTS] = {0, 0, {-1, -1, -1, -1, -1, -1}, 100, 100};

    //Initializes the state changes table which will be used to show the execution output file each time a PCB switches from one state to another state
    stateChangeTable stateCT[MAX_EVENTS];

    PCBQueue *queue = importInputFile(argv[1]); //calls the import input file function to save and trace from the input file the process information for the PCB in a queue
    
    if(strcmp("FCFS", argv[2]) == 0){ //checks to see if we are running a FCFS
        FCFS_Scheduler(queue, fixedPartitions, stateCT, memoryST); //calls a function to implement a FCFS scheduling algorithm
    }
    else if (strcmp("EP", argv[2]) == 0){ //checks to see if we are running an external proirities scheduler 
       NOPREMPT_Scheduler(queue, fixedPartitions, stateCT, memoryST);
    } 
    else if (strcmp("RR", argv[2]) == 0){ //checks to see if we are running a RR sceduler 
        RR_Scheduler(queue, fixedPartitions, stateCT, memoryST); 
    }
    
    return 0; 
}

/**
 * This function will open the input file that contains a list of all processes to run with the following
 * trace information: pid, memory size, arrival time, total cpu time I/O frequency and duration. Each process 
 * is then saved as a PCB Queue to be used later for excecution. 
 */
PCBQueue *importInputFile(const char *filename){
    PCBQueue *queue = (PCBQueue *)malloc(sizeof(PCBQueue)); //initializes the queue through dynamic memeory allocation 

    queue->front = NULL; //initializes the queue front to be null
    queue->rear = NULL; //initializes the queue rearr to be null
    queue->size = 0; //initializes the queue size to be 0

    if(!queue){ //checks to see that the allocation of the queue was sucessful otherwise returns null and prints a message 
        printf("Memory allocation failed"); //prints the error message 
        return NULL; 
    }

    FILE *openFile = fopen(filename, "r"); //opens a file and saves a refrence to it and reads from the file 

    if(!openFile){ //if the file refrence is null meaning that the file cound not be opened returns null and prints a message 
        printf("The file was not opened sucessfully"); //prints the error message 
        
        return NULL; //returns null 
    }

    char line[CHARACTERS_MAX]; //will be used to parse each line in the file to get a processes information
    PCB arrayPCB[MAX_EVENTS]; //creates an array that will be temporary carying the processes from the file will be sorted based on AT and PID
    int count = 0; //will be used to iterate through the array of PCB Table traversing through each PCB entry

    while (fgets(line, sizeof(line), openFile)) //traverses through each line in the input file 
    {
        PCB tempPCB; //will be used to save the values of the PCB when reading from the file then will be appends to the queue

        sscanf(line, "%u, %u, %u, %u, %u, %u",
        &tempPCB.pid,
        &tempPCB.memorySize,
        &tempPCB.arrivalTime,
        &tempPCB.totalCPUTime,
        &tempPCB.ioFrequency,
        &tempPCB.ioTimeDuration
        );//gets all of the information from the line of the file and saves it for the PCB 

        tempPCB.partitionNumber = 0; //assigns the partition number as 0 in the begining

        tempPCB.countIOFrequencyDelay = 0; //will be used as a counter to count the time of a I/O delay 

        tempPCB.sliceTime = 100; //sets the slice time of the PCB to 100 for the RR even if not used 

        strcpy(tempPCB.state, "NEW"); //saves the state for the PCB as new

        enqueue(queue, tempPCB); //appdsnd the PCB to the queue using an enqueue function 
        
    }

    fclose(openFile); //closes the file after it has been read from

    return queue; //returns the pcbs containg all of teh processes from the file in an queue
}


/**
 * This function is used to append a PCB node to the queue. A PCB node is dynamically allocated an then added to the rear of the 
 * queue. This function will be used when appending the pcb from the input file and when a PCB needs to be put back again at the back
 * of the queue for any of the scheduling alogorithms.
 */
void enqueue(PCBQueue *queue, PCB PCBData){
    queueNode *node = (queueNode*)malloc(sizeof(queueNode)); //creates a dynamically alocated note to be added to the rear of the queue

    if(!node){ //checks to see if the node was sucessfully allocated
        printf("Node mememory allocation failed"); //prints the error message 
        return; //returns from the function 
    }

    node->pcbData = PCBData; //assigns the pcb node data to the node 
    node->next = NULL; //makes the pointer to the next node null 

    if(queue->front == NULL){ //checks if the queue is empty in this case makes the front and rear point to the node
        queue->front = node; //makes the head point to the node
        queue->rear = node; //makes the rear point to the node 
    }
    else{ //there is at least one node in the queue then appends it to the rear
        queue->rear->next = node; //makes the current rear node point to the node
        queue->rear = node; //makes the rear point to the node
    }

    queue->size++; //incremenets the size of the queue

}

/**
 * This function is used to remove a node from the front of the queue. This will be used when a PCB goes 
 * to the waiting state for I/O or when a PCB needs to be terminated. It returns the PCB that was removed from
 * the front of the queue. MIGHT NOT BE NEEDED
 */
PCB dequeue(PCBQueue *queue){

    PCB dequeuedNode = queue->front->pcbData; //stores the PCB that was at the front of the node
    queueNode* tempNode = queue->front; //assigns a temp pointer to point to the first node in the queue
    
    queue->front = queue->front->next; //moves the front pointer to move down to the next node in the queue as the new front 

    if(queue->front == NULL){ //checks to see if the queue is empty 
        queue->rear = NULL; //makes the rear pointer point to null if empty 
    }

    free(tempNode); //frees the temp node that was the previous node at the front of the queue 
    queue->size--; //decrearese the size of the queue

    return dequeuedNode; //returns the deueued PCB that was at the front of the queue 

}

/**
 * This function is used to sort the queue of PCB based on arrival time. If PCB's share the same arrival time
 * they will be sorted based on their PID from lowest to greatest. Will be helpful to run the scheduler when there are
 * multiple PCB's with the same AT for FCFS. For the other scheduling algorithms, it assigns the highest proirity to the shortest job first. 
 */
void sortQueue(PCBQueue *queue, int typeOfSchduling) {
    if (!queue || !queue->front || !queue->front->next) { //checks to see if the queue is empty 
        return; //if its empty returns from the function 
    }

    queueNode *current, *index; //assigns to pointers to the nodes in the queue 
    PCB tempPCB; //the PCB data that will be in the temp pointer 
    int priority = 0; //used to assign the prorities of the PCB for SJF non preemtive 

    if(typeOfSchduling == 0 || typeOfSchduling == 2){ //deals with FCFS and RR we dont need to wory about priority 
        for (current = queue->front; current->next != NULL; current = current->next) { //traverses through the the queue starring from the front and uses a current walking pointer 
            for (index = current->next; index != NULL; index = index->next) { //this will be the index pointer for traversing the queue 
                if (current->pcbData.arrivalTime > index->pcbData.arrivalTime || (current->pcbData.arrivalTime == index->pcbData.arrivalTime && current->pcbData.pid > index->pcbData.pid)) { //if the current pointer PCB has a larger AT or has the same arrival time but a larger PID
                    //bubble swap 
                    tempPCB = current->pcbData; //assigns the current to a temp PCB
                    current->pcbData = index->pcbData; //swaps the current with the index
                    index->pcbData = tempPCB; //the index gets the previous current 
                }
            }
        }
    }
    
    else if (typeOfSchduling == 1){ //only assigns the pririties if we are dealing with a nonPremtive assignsthe highest priority to the SJF
        for(current = queue->front; current->next != NULL; current = current->next){ //travreses through the queueu 
            for(index = current->next; index != NULL; index = index->next){ //this will be the index pointer for traversing the queue 
                if(current->pcbData.totalCPUTime > index->pcbData.totalCPUTime){ //compares the CPU burst time of the current node to the indexing node 
                    //bubble swap 
                    tempPCB = current->pcbData; //assigns the current to a temp PCB
                    current->pcbData = index->pcbData; //swaps the current with the index
                    index->pcbData = tempPCB; //the index gets the previous current 
                }
            }
        }
    }
}

/**
 * This is the function that takes care of the FCFS scheduling of PCBs. It will call different such as PCBNewToReady to assist with the PCb CPU burst time,
 * dealing with I/O, etc. 
 */
void FCFS_Scheduler(PCBQueue *queue, memoryPartions *fixedPartitions, stateChangeTable *stateCT, memoryStatusTable *memoryST) {

    PCBReadyToRunning(queue, fixedPartitions, stateCT, memoryST, 0); //calls a runction to log the CPU run times which also will handle I/O and AT logging 

    executionFunction(stateCT); //calls the execution function to log the state changes in the execution file 
    memoryStatusFunction(memoryST); //calls the function to log the memeory status chanenges in the memory status file 
}

/**
 * This is the function that takes care of the Non Premtive scheduling of the PCB. It will call differenet function such as PCBNewToReadyNP (NonPremribe)
 * to assist with CPU burst time, to deal with I/O etc. 
 */
void NOPREMPT_Scheduler(PCBQueue *queue, memoryPartions *fixedPartitions, stateChangeTable *stateCT, memoryStatusTable *memoryST){

    PCBReadyToRunning(queue, fixedPartitions, stateCT, memoryST, 1); //calls a runction to log the CPU run times which also will handle I/O and AT logging  

    executionFunction(stateCT); //calls the execution function to log the state changes in the execution file 
    memoryStatusFunction(memoryST); //calls the function to log the memeory status chanenges in the memory status file 
}

/**
 * This is the function that takes care of the RR scheduling of the PCB. It will call differenet function such as PCBNewToReadyNP (NonPremribe)
 * to assist with CPU burst time, to deal with I/O etc. 
 */
void RR_Scheduler(PCBQueue *queue, memoryPartions *fixedPartitions, stateChangeTable *stateCT, memoryStatusTable *memoryST){
    
    PCBReadyToRunning(queue, fixedPartitions, stateCT, memoryST, 2); //calls a runction to log the CPU run times which also will handle I/O and AT logging  

    executionFunction(stateCT); //calls the execution function to log the state changes in the execution file 
    memoryStatusFunction(memoryST); //calls the function to log the memeory status chanenges in the memory 
}

/**
 * This function takes care of logging the PCBs when they have arrived, it changes the states of each PCB from new to ready and logs their AT.
 * It also takes care of memeory partitions by occuping the space of the PCB in memory and updates the memory status table. 
 */
void PCBNewToReady(PCBQueue *queue, memoryPartions fixedPartitions[], stateChangeTable stateCT[], memoryStatusTable memoryST[], int time) {
    queueNode *tempNode = queue->front; //assigns a temp pointer that will be used to traverse through all of the nodes in the queue
    while(tempNode != NULL) { //travreses till the last node in the queue
        if (strcmp(tempNode->pcbData.state, "NEW") == 0 && tempNode->pcbData.arrivalTime == time) {  // Check if PCB is in the NEW state and has arrived 
            for (int j = FIXED_PARTITIONS_SIZE - 1; j >= 0; j--) {  // Check for available memory partition traversing from the rear of the array to ensure efficient memeory use 
                if (fixedPartitions[j].occupiedBy == -1 && fixedPartitions[j].size >= tempNode->pcbData.memorySize) { //checks to see tha the fixed partiton is free and that it can acomidate teh size of the PCB
                    fixedPartitions[j].occupiedBy = tempNode->pcbData.pid; //assigns the fixed memory partition the PID of the PCb to indiacte that its occupied 
                    tempNode->pcbData.partitionNumber = fixedPartitions[j].patritionNumber; //assigns the partiton number of the PCB to the partition number of the fixed mmemory size
                    strcpy(tempNode->pcbData.state, "READY"); //updtates the sate of teh PCb to a ready state 
                    logStateChange(stateCT, tempNode->pcbData.arrivalTime, tempNode->pcbData.pid, "NEW", "READY"); //calls a function to log the state change of teh PCb from new to ready 
                    logMemoryStatus(memoryST, fixedPartitions, tempNode->pcbData.arrivalTime, tempNode->pcbData.memorySize, 1); //calls a function to long the memory satus change after the PCB occupied space in memeory
                    break; //breaks and goes to the next node PCB 
                }
            }
        }
        tempNode = tempNode->next; //travreses to the next node PCB 
    }
}


/**
 * This function handels the CPU run time of all of the processes PCB, it handles it for all of the types of algorithms
 * by running there CPU time and logging them. It also handles calling other function to deal with new PCBs arriving, PCB waiting for IO,
 * PCB in ready queue and PCb when they ave terminated 
 */

void PCBReadyToRunning(PCBQueue *queue, memoryPartions fixedPartitions[], stateChangeTable stateCT[], memoryStatusTable memoryST[], int typeOfScheduling){
    PCBQueue *waitingQueue = (PCBQueue *)malloc(sizeof(PCBQueue)); //initializes the waiting queue through dynamic memeory allocation 

    waitingQueue->front = NULL; //initializes the queue front to be null
    waitingQueue->rear = NULL; //initializes the queue rear to be null
    waitingQueue->size = 0; //initializes the queue size to be 0
    
    int time = 0; //tracks the time 

    PCBNewToReady(queue, fixedPartitions, stateCT, memoryST, time); //calls a funtion to log the PCBs if any PCBs have arrived and need to be transitioned from new to ready at the very begining 

    sortQueue(queue, typeOfScheduling); //sorts the queue so that if there are PCBs with the same AT the one with the lowest PID is first for RR and FCFS, for a SJF non premetive sorts based on the lowest CPU time 
    
    queueNode *tempNode = getPriorityPCB(queue, time, typeOfScheduling); //calls a function to get the higest priority PCB that is read, works also for FCFS as they has the same priority 

    //queueNode *tempNode = queue->front; //assigns a pointer that will traverse the queue and find a ready PCB to be ran 
    
    while (tempNode != NULL || waitingQueue->size > 0){ //travreses until all PCBs have been terminated 
        if(tempNode != NULL && strcmp(tempNode->pcbData.state, "READY") == 0){ //if the PCB is ready to be ran
            strcpy(tempNode->pcbData.state, "RUNNING"); //updates the state as running 
            logStateChange(stateCT, time, tempNode->pcbData.pid, "READY", "RUNNING"); //logs that the PCb has been changed from ready to running at a different time.
            for(int i = 0; i < tempNode->pcbData.ioFrequency; i++){ //travreses through the CPU time until the PCb terminates or we have hit an IO

                time++; //increments the time lapse

                if(typeOfScheduling == 2){ //only decrements the time slicing if its a RR scheduling 
                    tempNode->pcbData.sliceTime--; //decrements the time slicing 
                }

                PCBNewToReady(queue, fixedPartitions, stateCT, memoryST, time); //calls a funtion to log the PCBs if any PCBs have arrived and need to be transitioned from new to ready
                countIOfrequencyDelay(waitingQueue, queue, stateCT, time, 1); //clas a function to check if a PCB has finished its IO and is now ready to be executed
                countIOfrequencyDelay(waitingQueue, queue, stateCT, time, 0); //clas a function to check if a PCB has finished its IO and is now ready to be executed
                //calls the function twice one to increment the time and one to checks if a PCb in the waiting queue is ready to enter the ready queue after servicing its I/O time

                tempNode->pcbData.totalCPUTime--; //decrements the cpu burst time 

                if(tempNode->pcbData.totalCPUTime == 0){ //if the process termianted and has no more CPU burst time
                    PCBRunningToTerminated(tempNode, fixedPartitions, stateCT, memoryST, time); //calls a function to take care of the PCB that has termianted 
                    removeNodeFromQueue(queue, tempNode->pcbData.pid); //deques the node from the queue 
                    tempNode = getPriorityPCB(queue, time, typeOfScheduling); //calls a function to get the higest priority PCB that is read, works also for FCFS as they has the same priority 
                    break; //breaks out of the loop 
                }

                if(tempNode->pcbData.sliceTime == 0){ //if the process PCB has finished its time slicing then it need to be removed and put at the back of the ready queue and start the next processing time 
                    tempNode->pcbData.sliceTime = 100; //resets the time slice back to 100
                    strcpy(tempNode->pcbData.state, "WAITING"); //makes the state of the PCB waiting 
                    logStateChange(stateCT, time, tempNode->pcbData.pid, "RUNNING", "WAITING"); //logs the update of the status
                    strcpy(tempNode->pcbData.state, "READY"); //makes the state of the PCB ready again
                    logStateChange(stateCT, time, tempNode->pcbData.pid, "WAITING", "READY"); //logs the update of the status 
                    enqueue(queue, tempNode->pcbData); //adds the node PCB back to the queue 
                    removeNodeFromQueue(queue, tempNode->pcbData.pid); //deques the node from the queue
                    tempNode = getPriorityPCB(queue, time, typeOfScheduling); //calls a function to get the higest priority PCB that is read, works also for FCFS as they has the same priority 
                    break; //breaks out of the loop 
                }
            }

            if(tempNode != NULL && strcmp(tempNode->pcbData.state, "RUNNING") == 0){ //if the PCB did not terminate and ran the CPU burst time until it was scheduled an I/O
                PCBRunningToWaiting(tempNode, waitingQueue, stateCT, time); //calls a function to log and change the state of the PCB from running to waiting 
                removeNodeFromQueue(queue, tempNode->pcbData.pid); //deques the node from the queue 
                tempNode = getPriorityPCB(queue, time, typeOfScheduling); //calls a function to get the higest priority PCB that is read, works also for FCFS as they has the same priority 
            }    
        }
         
        else{ //There isnt a PCB that is read to be ran is in the wait queue, wait until the I/O duration has finished 
            time++; //increments the time 
            PCBNewToReady(queue, fixedPartitions, stateCT, memoryST, time); //calls a funtion to log the PCBs if any PCBs have arrived and need to be transitioned from new to ready
            countIOfrequencyDelay(waitingQueue, queue, stateCT, time, 1); //calls a function to check if a PCB has finished its IO and is now ready to be executed
            countIOfrequencyDelay(waitingQueue, queue, stateCT, time, 0); //calls a function to check if a PCB has finished its IO and is now ready to be executed
            //calls the function twice one to increment the time and one to checks if a PCb in the waiting queue is ready to enter the ready queue after servicing its I/O time 
            tempNode = getPriorityPCB(queue, time, typeOfScheduling); //calls a function to get the higest priority PCB that is read, works also for FCFS as they has the same priority
        }   
    }
}

/**
 * This function returns the higest proirity PCB node that is in the ready queue. Must ensure that it has arrived in the ready 
 * queue by cheking the arrival time. If its a FCFS no pririty returns the first one in the queue, othersise it returns
 * the shortest job first. 
 */
queueNode *getPriorityPCB(PCBQueue *queue, int time, int typeOfScheduling){
    queueNode *tempNode; //declairs a pointer to the PCB node that will travrese the queue 
    queueNode *highestPriorityNode = NULL; //declares a pointer to the PCB node that will store the node with the higest priority makes it null in case it is an empty queue and for at the begining 

    if(typeOfScheduling == 0 || typeOfScheduling == 2){ //for FCFS and RR there is no condition they are both non premtive and will process the PCb that is ready in the queue 
        for(tempNode = queue->front; tempNode != NULL; tempNode = tempNode->next){ //travreses through the queue
            if(tempNode->pcbData.arrivalTime <= time){ //makes sure that the highest proirty PCB node has arrived 
               return tempNode; //returns the first node in the ready queue as nog as it has arrived  
            }
        }
    }

    else{ //for SJF non premtive finds the PCb with the shortest time first 
        for(tempNode = queue->front; tempNode != NULL; tempNode = tempNode->next){ //travreses through the queue
            if(tempNode->pcbData.arrivalTime <= time){ //makes sure that the highest proirty PCB node has arrived 
                if(highestPriorityNode == NULL || tempNode->pcbData.totalCPUTime < highestPriorityNode->pcbData.totalCPUTime){ //if the highest priory hasnt been assigned yet or checks for the hiegst prority by comparing to the previous node to the current PCB
                    highestPriorityNode = tempNode; //assigns the higest priority node 
                }
            }
        } 
    }
    
    return highestPriorityNode; //returns the higest priority node
}

/**
 * Logs when a function has terminated and removes it from the memory partion.
 */
void PCBRunningToTerminated(queueNode *tempNode, memoryPartions fixedPartitions[], stateChangeTable stateCT[], memoryStatusTable memoryST[], int time){
    strcpy(tempNode->pcbData.state, "TERMINATED"); //updates the state as terminated
    logStateChange(stateCT, time, tempNode->pcbData.pid, "RUNNING", "TERMINATED"); //logs the state change
    for(int i = 0; i < FIXED_PARTITIONS_SIZE; i++){ //travreses through the fixed memeory partitons
        if(fixedPartitions[i].occupiedBy == tempNode->pcbData.pid){ //finds the memeory partion associated with the PID of the PCB
            fixedPartitions[i].occupiedBy = -1; //sets it as free 
        }
    }

    logMemoryStatus(memoryST, fixedPartitions, time, tempNode->pcbData.memorySize, 0); //calls a function that updates the memory status after the PCB has been removed
}

/**
 * Logs that a process has went from running to waiting and appends it to the waiting queue to begin the implmenetation of the I/O
 * duration for the PCB.  
 */
void PCBRunningToWaiting(queueNode *tempNode, PCBQueue *waitingQueue, stateChangeTable stateCT[], int time){
    strcpy(tempNode->pcbData.state, "WAITING"); //makes the state of the PCB waiting 
    logStateChange(stateCT, time, tempNode->pcbData.pid, "RUNNING", "WAITING"); //logs the update of the status 
    enqueue(waitingQueue, tempNode->pcbData); //appends the PCB to the waiting queue
}

/**
 * Counts the I/O duration of a PCB and ensures that it goes back to the ready queue after it serves it I/O duration.
 * This ensures that the PCB does not run again until it has served its I/O duration. 
 */
void countIOfrequencyDelay(PCBQueue *waitingQueue, PCBQueue *mainQueue, stateChangeTable stateCT[], int time, int increment){
    queueNode *tempNode = waitingQueue->front; //assigns a temp pointer that will travrese the node of the I/O in the waiting queue 
    int backInQueue = 0; //back in queue flag that indicates that a PCb has been put back in the main/running queue 
    while(tempNode != NULL){ //travreses through the PCB queueu 
        if(strcmp(tempNode->pcbData.state, "WAITING") == 0 && tempNode->pcbData.countIOFrequencyDelay == tempNode->pcbData.ioTimeDuration){ //checks to see if the PCB has served its I/O duration
            tempNode->pcbData.countIOFrequencyDelay = 0; //resets the counter of the I/O frequency delay which is ment to count the I/O duration 
            strcpy(tempNode->pcbData.state, "READY"); //makes the state of the PCB ready again
            logStateChange(stateCT, time, tempNode->pcbData.pid, "WAITING", "READY"); //logs the update of the status 
            enqueue(mainQueue, tempNode->pcbData); //adds the node PCB back to the main queue
            queueNode *remove = tempNode; //tho noes that will be removed
            tempNode = tempNode->next; //travreses to the next node in the queue 
            removeNodeFromQueue(waitingQueue, remove->pcbData.pid); //dequeues from the waiting queue 
            backInQueue = 1; //a node was put back in queue 
        }
        else if (strcmp(tempNode->pcbData.state, "WAITING") == 0 && increment){ //checks to see that the PCB has not yet lapsed the I/O frequency time 
            tempNode->pcbData.countIOFrequencyDelay++; //increments the count of the I/O frequency counter
        }  

        if(!backInQueue){ //if the wasnt a PCB that was put back in the queue then traverse to the next node only once as it is already called previously to ensure the walking pointer isnt null 
            tempNode = tempNode->next; //traverses to the next node in the queue 
        }
        
        backInQueue = 0; //resets the flag 
        
    }
}

/**
 * Removes a specific node from the queueu based on the PID provided. This function is used for the waiting queue when a PCB
 * has finished servicing its I/O time and is removed and put back in the ruuning queue. 
 */

void removeNodeFromQueue(PCBQueue *queue, int pid) {
   
    if (queue->front == NULL) { //checks to see if the queue is empty 
        return; //returns the function if the queue is empty 
    }

    queueNode *current = queue->front; //a pointer that will point to the curent node when traversing 
    queueNode *previous = NULL; //a pointer that will point to the previous node before the current when traversing

    while (current != NULL) { //travreses through the queue 
        if (current->pcbData.pid == pid) { //if the current pointer matched the same PID
            if (previous == NULL) { //deals with the case that the node is at the front of the queue
                queue->front = current->next; //makes the front of the queue point to the next pointer after the current 
                if (queue->front == NULL) { //if that was the last node in the queue 
                    queue->rear = NULL; //makes the rear also point to null 
                }
            } 
            else {//deals with the case that the pointer is at the middle or the end of the queue
                previous->next = current->next; //the previous pointer skips over the node to be removed 
                if (current->next == NULL) {  //if the node to be removed is at the end 
                    queue->rear = previous; //makes the rear point to the previous node 
                }
            }

            free(current); //frees the node from dynamic allocation 
            queue->size--; //decrements the size of the queue 
            return; //returns from the function 
        }


        previous = current; //the previous becomes the currnt node 
        current = current->next; //the current travreses to the next node 
    }
}


/**
 * A function that logs each time a PCB has switched state, it stores the time it occured, the PID of the PCB
 * and the old and new states of the PCB in ana array. 
 */
void logStateChange(stateChangeTable stateCT[], int time, int pid, const char *oldState, const char *newState) {
    static int stateChanges = 0; //counts the number of state changes for the array index, static so that it holds value 
    stateCT[stateChanges].timeOfTransition = time; //assigns the time of transtion as the time passed
    stateCT[stateChanges].PID = pid; //assigns the PID as the pid passed 
    strcpy(stateCT[stateChanges].oldState, oldState); //assigns the old state as the old state passed
    strcpy(stateCT[stateChanges].newState, newState); //assigns the new state as the new state passed
    stateChanges++; //incremenet the state changes
}

/**
 * A function that handels the logging of the memory each time a PCB has been put into memory (Has arrived and there is space)
 * or when a PCB terminates and is removed from memory. 
 */
void logMemoryStatus(memoryStatusTable memoryST[], memoryPartions fixedPartitions[], int time, int memoryChange, int wasAdded) {
    static int memoryStatus = 1; //holds the index of the memory status, starts at 1 as the first stae is nitialized at the begining
    int usableMemory = 0; //counts up the usable memory based on the free memory partitions 

    for (int i = 0; i < FIXED_PARTITIONS_SIZE; i++) { //travreses through the fixed memory partions 
        memoryST[memoryStatus].partitionState[i] = fixedPartitions[i].occupiedBy; //saves the partition states based on the current memory occupied by the PCBs
        if (fixedPartitions[i].occupiedBy == -1) { //if the memory is free
            usableMemory += fixedPartitions[i].size; //adds up the uable memory size 
        }
    }

    if(wasAdded){ //if there is a PCB being added to memory 
        memoryST[memoryStatus].timeOfEvent = time; //saves the time its being added
        memoryST[memoryStatus].memoryUsed = memoryST[memoryStatus - 1].memoryUsed + memoryChange; //increments the memory that is being used by the new added memory size and the previous used memeory size 
        memoryST[memoryStatus].totalFreeMemory =  memoryST[memoryStatus - 1].totalFreeMemory  - memoryChange; //subtracts the new memory added by the free memeory from the previous state 
        memoryST[memoryStatus].usableFreeMemory =  usableMemory; //sets the calculated usable memory 
        
    }
    else{ //A PCb is being removed from memory
        memoryST[memoryStatus].timeOfEvent = time; //saves the time ist being removed 
        memoryST[memoryStatus].memoryUsed = memoryST[memoryStatus - 1].memoryUsed - memoryChange; //decrements the memeory that is being removed by the new memeory size and the previous used memeory size
        memoryST[memoryStatus].totalFreeMemory =  memoryST[memoryStatus - 1].totalFreeMemory + memoryChange; //adds the new memory by the free memeory from the previous state 
        memoryST[memoryStatus].usableFreeMemory = usableMemory; //sets the calculated usable memory 
    }

    memoryStatus++; //incremenets to the next state 

}


/**
 * This function creates the execution file that prints all of the state changes for the PCB, ie when a PCB goes from New to Ready, etc.
 */
void executionFunction(stateChangeTable stateCT[]){
    FILE *executionFile = fopen("execution_101261583_101273477.txt", "w"); //creates a file refrence to the execution file and writes to it 

    if (executionFile == NULL) { // If the file could not be opened, print an error and return 
        printf("Error opening execution log file.\n"); //prints the message 
        return; //returns from the function 
    }

    fprintf(executionFile, "+------------------------------------------------+\n"); //prints the header format 
    fprintf(executionFile, "|Time of Transition|PID|   Old State|   New State|\n"); //pritns the details of the exectution file 
    fprintf(executionFile, "+------------------------------------------------+\n"); //prints the header format

    for(int i = 0; i < MAX_EVENTS; i++){ //travreses through the array of state changes 
        if (stateCT[i].PID == 0 && stateCT[i].timeOfTransition == 0){ //checks to make suer that the event is of impotnace as the array allocates more size than needed
            break; //breaks the loop of we reach state change that re not relevent 
        }
        fprintf(executionFile, "| %12u | %5u | %10s | %10s |\n", stateCT[i].timeOfTransition, stateCT[i].PID, stateCT[i].oldState, stateCT[i].newState); //appends the infomation of a state change to the file 
    }

    fprintf(executionFile, "+------------------------------------------------+\n"); //prints the bottom format
    
    fclose(executionFile); //closes the file 
}

/**
 * This function creates the memeory status table if the memory thatus output file. It will show the partitions states when a PCB has entered or left memory.
 * It will also include the time of transitions and the memory at each time the PCb entered or left. 
 */
void memoryStatusFunction(memoryStatusTable memoryST[]){
    FILE *memoryStatusFile = fopen("memory_status_101261583_101273477.txt", "w"); //creates a file refrence to the memory status file and writes to it 


    if (memoryStatusFile == NULL) { // If the file could not be opened, print an error and return 
        printf("Error opening execution log file.\n"); //prints the message 
        return; //returns from the function 
    }

    fprintf(memoryStatusFile, "+------------------------------------------------------------------------------+\n"); //prints the header format 
    fprintf(memoryStatusFile, "|Time of Event|Memory Used|Partition State|Total Free Memory|Usable Free Memory|\n"); //pritns the details of the exectution file 
    fprintf(memoryStatusFile, "+------------------------------------------------------------------------------+\n"); //prints the header format

    for (int i = 0; i < MAX_EVENTS; i++) { //travreses through the array list of memory status 

        if(memoryST[i].memoryUsed == 0 && memoryST[i].timeOfEvent == 0 && memoryST[i].totalFreeMemory == 0 && memoryST[i].usableFreeMemory == 0){ //makes sure that it only print the memeory status that are relevent as the array allocates more events than needed
            break; //breaks out of the function 
        }
    
        fprintf(memoryStatusFile, "| %11u | %9u |", memoryST[i].timeOfEvent, memoryST[i].memoryUsed); //put the time of event and the memeory used in the file 

        for (int j = 0; j < FIXED_PARTITIONS_SIZE; j++) { //travreses through the partions 
            if (j == FIXED_PARTITIONS_SIZE - 1) { //if we are at the last partition
                fprintf(memoryStatusFile, " %d", memoryST[i].partitionState[j]); //doesnt include a comma when puting it in the file 
            } 
            else {
                fprintf(memoryStatusFile, " %d,", memoryST[i].partitionState[j]); //puts the partions with a comma 
            }
        }

        fprintf(memoryStatusFile, "| %7u | %16u |\n", memoryST[i].totalFreeMemory, memoryST[i].usableFreeMemory); //puts the free and usable memory in the file 
    }

    fprintf(memoryStatusFile, "+------------------------------------------------------------------------------+\n"); //prints the bottom format
    
    fclose(memoryStatusFile); //closes the file 

}






