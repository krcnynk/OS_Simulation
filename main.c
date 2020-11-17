#include "list.h"
#include "Simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_READY_QUEUE 3
#define MAX_BLOCK_QUEUE 2
#define SEM_LENGTH 5

bool compare(void* pItem,void* pCompArg)
{

    if( ((PCB*)pItem)->PID == ((PCB*)pCompArg)->PID)
        return 1;
    else
        return 0;
}

void freeProcMsg(void* pItem)
{
    if(pItem != NULL)
        free((char*)pItem);
}

void freePCB(void* pItem)
{
    if( ((PCB*)pItem)->proc_message != NULL)
        List_free(((PCB*)pItem)->proc_message,&freeProcMsg);
    if(pItem != NULL)
        free((PCB*)pItem);
}

void closeUP(List** readyQ, List** srQ,Semaphore** s,PCB* runnin)
{
    for(int i = 0;i < MAX_READY_QUEUE;++i)
    {
        if(readyQ[i] != NULL)
            List_free(readyQ[i],&freePCB);
    }

    for(int i = 0;i < MAX_BLOCK_QUEUE;++i)
    {
        if(srQ[i] != NULL)
            List_free(srQ[i],&freePCB);
    }

    for(int i = 0;i < SEM_LENGTH;++i)
    {
        if(s[i] != NULL)
        {
            if(s[i]->processListWaitingOnSemaphore != NULL)
                List_free(s[i]->processListWaitingOnSemaphore,&freeProcMsg);
        }
    }

    if(runnin != NULL)
        free(runnin);
}

// Fetches a ready process and removes from the queue
PCB* fetchProcessFromReadyQueueRemove(List** rQ)
{
    for(int i = 0;i < MAX_READY_QUEUE;++i)
    {
        if(List_count(rQ[i]) != 0)
        {
            return List_trim(rQ[i]);
        }
    }

    return NULL;
}

// Searches process in all queues
PCB* searchProcess(List** Q,int qSize, int PID)
{
    for(int i = 0; i < qSize; ++i)
    {   
        if(Q[i] != NULL)
        {
            PCB temp;
            temp.PID = PID;
            List_first(Q[i]);
            PCB* process = List_search(Q[i],&compare,&temp);
            if(process != NULL)
            {
                return process;
            }
        }
    }

    return NULL;
}

PCB* searchProcessAndRemove(List** Q,int qSize, int PID)
{
    for(int i = 0; i < qSize; ++i)
    {   
        if(Q[i] != NULL)
        {
            PCB temp;
            temp.PID = PID;
            List_first(Q[i]);
            PCB* process = List_search(Q[i],&compare,&temp);
            if(process != NULL)
            {
                return List_remove(Q[i]);
            }
        }
    }

    return NULL;
}

int countProcess(List** rq,List**srq,Semaphore** s)
{
    int count = 0;
    if(rq != NULL)
    {
        for(int i = 0; i < MAX_READY_QUEUE; ++i)
            count = count + List_count(rq[i]);
    }
    if(srq != NULL)
    {
        for(int i = 0; i < MAX_BLOCK_QUEUE; ++i)
            count = count + List_count(srq[i]);
    }
    if(s != NULL)
    {
        for(int i = 0; i < SEM_LENGTH; ++i)
        {
            if(s[i] != NULL)
            {
                count = count + List_count(s[i]->processListWaitingOnSemaphore);
            }
        }
    }
    return count;
}

int main()
{
    List* readyQueueHigh = List_create();
    List* readyQueueNormal = List_create();
    List* readyQueueLow = List_create();
    List* processSendWaitQueue = List_create();
    List* processReceiveWaitQueue = List_create();
    List* ReadyQueues[3];
    List* SendReceiveWaitQueues[2];
    List* allQueues[MAX_BLOCK_QUEUE+MAX_READY_QUEUE+SEM_LENGTH] = {};
    Semaphore* sem[5] = {};
    int globalPID = 1;
    char input;
    char msg[40];

    ReadyQueues[0] = readyQueueHigh;
    ReadyQueues[1] = readyQueueNormal;
    ReadyQueues[2] = readyQueueLow;
    SendReceiveWaitQueues[0] = processSendWaitQueue;
    SendReceiveWaitQueues[1] = processReceiveWaitQueue;
    allQueues[0] = ReadyQueues[0];
    allQueues[1] = ReadyQueues[1];
    allQueues[2] = ReadyQueues[2];
    allQueues[3] = SendReceiveWaitQueues[0];
    allQueues[4] = SendReceiveWaitQueues[1];

    PCB* running = NULL;
    List* init_MSG = List_create();

    while(1)
    {
        char ch;
        fgets(&ch, 2, stdin);
        switch (ch)
        {
            case 'C': //DONE
            {
                char c[10];
                printf("Enter priority 0(HIGH),1(NORMAL),2(LOW)\n");
                fgets(c, 11, stdin);

                int input = atoi(c);

                if(input == HIGH || input == NORMAL || input == LOW)
                {
                    PCB* pcbP = malloc(sizeof(PCB));
                    pcbP->PID = globalPID;
                    pcbP->priority = input;
                    pcbP->processState = READY;
                    pcbP->proc_message = List_create();

                    if(running == NULL)
                    {
                        ++globalPID;
                        running = pcbP;
                        printf("Process PID:%d successfully created and running\n",pcbP->PID);
                    }
                    else if(List_prepend(ReadyQueues[input],pcbP) == 0)
                    {
                        ++globalPID;
                        printf("Process PID:%d successfully created\n",pcbP->PID);
                    }
                    else
                    {
                        printf("Process PID:%d cannot be created, %s priority ready buffer is full\n",pcbP->PID, priorityString[pcbP->priority] );
                    }
                }
                else
                {
                    printf("This number is not a priority\n");
                }
                break;
            }

            case 'F': //DONE
            {
                if(running == NULL)
                {
                    printf("Can't fork init process\n");
                }
                else
                {
                    PCB* forkedP = malloc(sizeof(PCB));
                    forkedP->PID = globalPID;
                    forkedP->priority = running->priority;
                    forkedP->processState = READY;
                    forkedP->proc_message = List_create();
                    if(List_prepend(ReadyQueues[forkedP->priority],forkedP) == 0)
                    {
                        ++globalPID;
                        printf("Process PID:%d successfully created\n",forkedP->PID);
                    }
                    else
                    {
                        List_free(forkedP->proc_message,&freeProcMsg);
                        free(forkedP);
                        printf("Process PID:%d cannot be created, %s priority ready buffer is full\n",forkedP->PID,priorityString[forkedP->priority]);
                    }
                }
                break;
            }

            case 'K': // DONE
            {
                char c[10];
                printf("Enter PID of the process to kill.\n");
                fgets(c, 11, stdin);
                int input = atoi(c);

                if(countProcess(ReadyQueues,SendReceiveWaitQueues,sem) == 0 && input == 0) // only init is alive and killed
                {
                    printf("Exiting the program\n");
                    closeUP(ReadyQueues,SendReceiveWaitQueues,sem,running);
                    return 0;
                }
                else if (countProcess(ReadyQueues,SendReceiveWaitQueues,sem) >= 1 && input == 0) //There are processes but init kill
                {
                    printf("Killing init is illegal while there are other processes are in the system\n");
                }
                else if (running != NULL && running->PID == input) //Killing the running process
                {
                    printf("Killed the running process %d\n",running->PID);
                    if(running->proc_message != NULL)
                        List_free(running->proc_message,&freeProcMsg);
                    free(running);

                    PCB* fetched = fetchProcessFromReadyQueueRemove(ReadyQueues);
                    if(fetched == NULL)
                    {
                        printf("Init is running now\n");
                        running = NULL;
                    }
                    else
                    {
                        fetched->processState = RUNNING;
                        running = fetched;
                        printf("Process %d is running now\n",input);
                    }
                }
                else // Killing in readyQ or blockedQ
                {
                    PCB* ret = searchProcessAndRemove(allQueues,MAX_READY_QUEUE + MAX_BLOCK_QUEUE + SEM_LENGTH,input);
                    if(ret != NULL)
                    {
                        if(ret->proc_message != NULL)
                            List_free(ret->proc_message,&freeProcMsg);
                        printf("Killed the process %d\n",input);
                    }
                    else
                    {
                        printf("Could not find the process in the system\n");
                    }
                }
                break;
            }

            case 'E': //DONE
            {
                if(countProcess(ReadyQueues,SendReceiveWaitQueues,sem) == 0 && running == NULL)
                {
                    printf("Exiting the program\n");
                    closeUP(ReadyQueues,SendReceiveWaitQueues,sem,running);
                    return 0;
                }
                else if(countProcess(ReadyQueues,SendReceiveWaitQueues,sem) > 0 && running == NULL)
                {
                    printf("Can't exit init while there are other processes in the system\n");
                }
                else
                {
                    printf("Killed the running process %d\n",running->PID);
                    if(running->proc_message != NULL)
                        List_free(running->proc_message,&freeProcMsg);
                    free(running);

                    PCB* fetched = fetchProcessFromReadyQueueRemove(ReadyQueues);
                    if(fetched == NULL)
                    {
                        printf("Init is running now\n");
                        running = NULL;
                    }
                    else
                    {
                        fetched->processState = RUNNING;
                        running = fetched;
                        printf("Process %d is running now\n",running->PID);
                    }
                }
                break;
            }

            case 'Q': //DONE
            {

                if(countProcess(ReadyQueues,NULL,NULL) == 0 || running == NULL)
                {
                    printf("Ready queue is empty,Quantum not applicable\n");
                    break;
                }
                else
                {
                    running->processState = READY;
                    if(List_prepend(ReadyQueues[running->priority],running) == 0)
                    {
                        printf("Running process %d has been placed back into %s priority queue", running->PID, priorityString[running->priority]);

                        PCB* fetched = fetchProcessFromReadyQueueRemove(ReadyQueues);
                        if(fetched == NULL)
                        {
                            printf("Init is running now\n");
                            running = NULL;
                        }
                        else
                        {
                            fetched->processState = RUNNING;
                            running = fetched;
                            printf("Process %d is running now\n",running->PID);
                        }
                    }
                }
                break;
            }

            case 'S':
            {
                char c[10];
                char msg[40];
                printf("Enter PID of the process\n");
                fgets(c, 11, stdin);
                printf("Enter and reply message(max 40 chars)\n");
                fgets(msg, 41, stdin);

                int input = atoi(c);

                if((input == 0 && running == NULL) || (input == running->PID))
                {
                    printf("Can't send to running process\n");
                }
                else if(countProcess(ReadyQueues,SendReceiveWaitQueues,sem) == 0)
                {
                    printf("There is no process to send message\n"); // only init is in the system
                }
                else
                {
                    if(input == 0)
                    {
                        List_prepend(init_MSG,msg);
                        printf("Message has been sent to init\n");
                    }
                    else
                    {
                        PCB* ret = searchProcess(allQueues,MAX_READY_QUEUE + MAX_BLOCK_QUEUE + SEM_LENGTH,input);
                        if(ret == NULL)
                        {
                            printf("No process with this PID lives in this system\n");
                        }
                        else
                        {
                            char* msgMal = malloc(sizeof(char)*40);
                            strcpy(msgMal,msg);
                            if(List_prepend(ret->proc_message, msgMal) == -1)
                            {
                                free(msgMal);
                                printf("Buffer overflow in Receiving process,message is getting dumped\n");
                            }
                            else
                            {
                                printf("Message has been sent to process %d\n",input);
                                running->processState = BLOCKED;
                                List_prepend(SendReceiveWaitQueues[0],running);

                                PCB* fetched = fetchProcessFromReadyQueueRemove(ReadyQueues);
                                if(fetched == NULL)
                                {
                                    printf("Init is running\n");
                                }
                                else
                                {
                                    fetched->processState = RUNNING;
                                    running = fetched;
                                }
                            }
                        }
                    }
                }
                break;
            }

            case 'R':
            {
                if(running == NULL)
                {
                    if(List_count(init_MSG) == 0)
                    {
                        printf("There are no messages to be received for init, not blocking\n");
                    }
                    else
                    {
                        char* received = List_trim(init_MSG);
                        printf("Received message %s\n",received);
                        free(received);
                    }
                    
                }
                else if(List_count(running->proc_message) == 0) // RUNNING PROCESS HAS NO MESSAGE
                {
                    printf("There are no messages to be received, blocking the running process\n");
                    running->processState = BLOCKED;
                    List_prepend(SendReceiveWaitQueues[1],running);
                    PCB* fetched = fetchProcessFromReadyQueueRemove(ReadyQueues);
                    if(fetched == NULL)
                    {
                        printf("Init is running\n");
                        running = NULL;
                    }
                    else
                    {
                        fetched->processState = RUNNING;
                        running = fetched;
                    }
                }
                else // RUNNING PROCESS HAS A MESSAGE
                {
                    char* received = List_trim(running->proc_message);
                    printf("Received message %s\n",received);
                    free(received);
                } 
                break;  
            }

            case 'Y':
            {
                char c[10];
                char msg[40];
                printf("Enter PID of the process\n");
                fgets(c, 11, stdin);
                printf("Enter and reply message(max 40 chars)\n");
                fgets(msg, 41, stdin);

                int input = atoi(c);

                if(input == 0)
                {
                    printf("Replying to init\n");
                    List_prepend(init_MSG,msg);
                }
                else
                {
                    PCB* ret = searchProcessAndRemove((List**)processSendWaitQueue,1,input);
                    if(ret == NULL)
                    {
                        printf("No process with this PID blocked on \"Send\" in this system\n");
                    }
                    else
                    {
                        ret->processState = READY;
                        if(List_prepend(ReadyQueues[running->priority],ret) == 0)
                        {
                            printf("Sender id: %d has been unblocked and put into %s priority queue", ret->PID, priorityString[running->priority]);
                            char* msgMal = malloc(sizeof(char)*40);
                            strcpy(msgMal,msg);
                            if(List_prepend(ret->proc_message, msg) == -1)
                            {
                                free(msgMal);
                                printf("Buffer overflow in Sending process,reply is getting dumped\n");
                            }
                            else
                            {
                                printf("Reply has been sent to process %d\n",input);
                            }
                            
                        }
                    }
                }
                break;
            }

            case 'N':
            {
                char c1[10];
                char c2[10];

                printf("Enter SID of the semaphore\n");
                fgets(c1, 11, stdin);
                int input1 = atoi(c1);

                printf("Enter value of the semaphore\n");
                fgets(c2, 11, stdin);
                int input2 = atoi(c2);
                if(input1 <0 || input1>5)
                {
                    printf("Enter a 0-4 SID number\n");
                    break;
                }
                if(sem[input1] != NULL)
                {
                    printf("This semaphore has been already created\n");
                }
                else
                {
                    printf("Created semaphore with id: %d , value: %d\n",input1,input2);
                    sem[input1] = malloc(sizeof(Semaphore));
                    sem[input1]->value = input2;
                    sem[input1]->processListWaitingOnSemaphore = List_create();
                    allQueues[5+input1] = sem[input1]->processListWaitingOnSemaphore;
                }
                break;
            }
            case 'P':
            {
                char c[10];
                printf("Enter SID of the semaphore\n");
                fgets(c, 11, stdin);
                int input1 = atoi(c);

                if(sem[input1] == NULL)
                {
                    printf("This semaphore doesn't exist yet\n");
                    break;
                }

                if(input >= 0 && input < 5 )
                {
                    --sem[input1]->value;
                    if(sem[input1]->value > -1)
                    {
                        printf("Process goes into the semaphore, no blocking, success\n");
                    }
                    else
                    {
                        if(running == NULL)
                        {
                            printf("Failure all semaphores are taken,running process needs to block but init cannot be blocked, failure\n");
                        }
                        else
                        {
                             printf("Process is blocked on the semaphore\n");
                            List_prepend(sem[input1]->processListWaitingOnSemaphore,running);

                            PCB* fetched = fetchProcessFromReadyQueueRemove(ReadyQueues);
                            if(fetched == NULL)
                            {
                                printf("Init is running now\n");
                                running = NULL;
                            }
                            else
                            {
                                fetched->processState = RUNNING;
                                running = fetched;
                                printf("Process %d is running now\n",input);
                            }
                        }

                    }
                }
                else
                {
                    printf("SID is out of the range\n");
                }
                break;
            }
            case 'V':
            {
                char c[10];
                printf("Enter SID of the semaphore\n");
                fgets(c, 11, stdin);
                int input1 = atoi(c);
                
                if(sem[input1] == NULL)
                {
                    printf("This semaphore doesn't exist yet\n");
                    break;
                }
                if(input >= 0 && input < 5 )
                {
                    int preINC = sem[input1]->value;
                    ++sem[input1]->value;
                    if(preINC < 0)
                    {
                        PCB* fetched = List_trim(sem[input1]->processListWaitingOnSemaphore);
                        printf("Unblocking the process: %d on this semaphore\n",fetched->PID);
                        List_prepend(ReadyQueues[fetched->priority],fetched);
                    }
                    else
                    {
                        printf("There are no blocked processes on this semaphore\n");
                    }
                }
                break;
            }

            case 'I': //DONE
            {
                char c[10];
                printf("Enter PID of the process\n");
                fgets(c, 11, stdin);
                int input1 = atoi(c);

                if(input1 == 0)
                {
                    if(running == NULL)
                    {
                        printf("Process is init currently running\n");
                        goto jump;
                    }
                    else
                    {
                        printf("Process is init waiting\n");
                        goto jump;
                    }
                    
                    break;
                }
                if(running != NULL && input1 == running->PID)
                {
                    printf("Process is currently running has priority %d\n",running->priority);
                    goto jump;
                }
                for(int i = 0; i < MAX_READY_QUEUE; ++i)
                {
                    PCB temp;
                    temp.PID = input1;
                    List_first(ReadyQueues[i]);
                    PCB* process = List_search(ReadyQueues[i],&compare,&temp);
                    if(process != NULL)
                    {
                        printf("Process id:%d is in %s priority Ready queue\n",process->PID,priorityString[process->priority]);
                        goto jump;
                    }
                }
                for(int i = 0; i < MAX_BLOCK_QUEUE; ++i)
                {   
                    PCB temp;
                    temp.PID = input1;
                    List_first(SendReceiveWaitQueues[i]);
                    PCB* process = List_search(SendReceiveWaitQueues[i],&compare,&temp);
                    if(process != NULL)
                    {
                        if(i == 0)
                        {
                            printf("Process id:%d has %s priority and blocked on send\n",process->PID,priorityString[process->priority]);
                            goto jump;
                        }
                        else
                        {
                            printf("Process id:%d has %s priority and blocked on receive\n",process->PID,priorityString[process->priority]);
                            goto jump;
                        }
                        
                    }
                }
                for(int i = 0; i < 5; ++i)
                {   
                    if(sem[i] != NULL)
                    {
                        PCB temp;
                        temp.PID = input1;
                        List_first(sem[i]->processListWaitingOnSemaphore);
                        PCB* process = List_search(sem[i]->processListWaitingOnSemaphore,&compare,&temp);
                        if(process != NULL)
                        {
                                printf("Process id:%d has %s priority and waiting on semaphore %d\n",process->PID,priorityString[process->priority],i);
                                goto jump;
                        }
                    }
                }
                printf("Cannot find the process\n");
            jump:
                break;
            }
            case 'T': //DONE
            {
                if(running == NULL)
                {
                    printf("Process init is running\n");
                }
                else
                {
                    printf("Process init waiting\n");
                    printf("Process id:%d and %s priority is RUNNING\n",running->PID,priorityString[running->priority]);
                } 
                for(int i = 0; i < MAX_READY_QUEUE; ++i)
                {   
                    PCB* process = List_first(ReadyQueues[i]);
                    while(process != NULL)
                    {
                        printf("Process id:%d is in %s priority READY queue\n",process->PID,priorityString[process->priority]);
                        process = List_next(ReadyQueues[i]);
                    }
                }
                for(int i = 0; i < MAX_BLOCK_QUEUE; ++i)
                {   
                    PCB* process = List_first(SendReceiveWaitQueues[i]);
                    while(process != NULL)
                    {
                        if(i == 0)
                        {
                            printf("Process id:%d has %s priority and BLOCKED on SEND\n",process->PID,priorityString[process->priority]);
                            process = List_next(SendReceiveWaitQueues[i]);
                        }
                        else
                        {
                            printf("Process id:%d has %s priority and BLOCKED on RECEIVE\n",process->PID,priorityString[process->priority]);
                            process = List_next(SendReceiveWaitQueues[i]);
                        }
                    }
                }
                for(int i = 0; i < 5; ++i)
                {   
                    if(sem[i] != NULL)
                    {
                        PCB* process = List_first(sem[i]->processListWaitingOnSemaphore);
                        while(process != NULL)
                        {
                                printf("Process id:%d has %s priority and BLOCKED on semaphore %d\n",process->PID,priorityString[process->priority],i);
                                process = List_next(sem[i]->processListWaitingOnSemaphore);
                        }
                    }
                }
                break;
            }
        }

        if(List_count(processReceiveWaitQueue) > 0)
        {
            PCB* lastPCB = List_last(processReceiveWaitQueue);
            if(List_count(lastPCB->proc_message) > 0)
            {
                printf("Unblocking receiving process id:%d",lastPCB->PID);
                while(List_count(lastPCB->proc_message) > 0)
                {
                    char* message = List_trim(lastPCB->proc_message);
                    printf("Messages received %s",message);
                    free(message);
                }
            }
        }
    }
}