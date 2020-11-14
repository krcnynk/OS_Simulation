#include "list.h"
#include "Simulation.h"
#define MAX_READY_QUEUE 3
#define MAX_BLOCK_QUEUE 2

bool compare(void* pItem,void* pCompArg)
{

    if((PCB*)pItem->PID == (PCB*)pCompArg->PID)
        return 1;
    return 0;
}

//void List_free(List* pList, FREE_FN pItemFreeFn);

void freeProcMsg(void* pItem)
{
    free((char*)pItem);
}

void freePCB(void* pItem)
{
    List_free((PCB*)pItem->proc_message,&freeProcMsg);
}

void closeUP(List** readyQ, List** srQ,PCB* runnin)
{
    List_free(readyQ[0],&freePCB);
    List_free(readyQ[1],&freePCB);
    List_free(readyQ[2],&freePCB);
    List_free(srQ[0],&freePCB);
    List_free(srQ[1],&freePCB);
    free(readyQ[0]);
    free(readyQ[1]);
    free(readyQ[2]);
    free(srQ[0]);
    free(srQ[1]);
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

// PCB* fetchBlockedProcess(List** srQ)
// {
//     for(int i = 0;i < MAX_BLOCK_QUEUE;++i)
//     {
//         if(List_count(srQ[i]) != 0)
//         {
//             return List_trim(srQ[i]);
//         }
//     }

//     return NULL;
// }

// Searches process in all queues
PCB* searchProcessIfFoundRemove(List** Q,int qSize, int PID)
{
    for(int i = 0; i < qSize; ++i)
    {   
        PCB temp;
        temp.PID = input - '0';
        process = List_search(Q[i],&compare,&temp);
        if(process != NULL)
        {
            return List_remove(Q[i]);
        }
    }

    return NULL;
}

// PCB* removeReadyProcess(List** rQ,int PID)
// {
//     PCB* process = NULL;
//     for(int i = 0; i < MAX_READY_QUEUE; ++i)
//     {   
//         PCB temp;
//         temp.PID = input - '0';
//         process = List_search(rQ[i],&compare,&temp);
//         if(process != NULL)
//         {
//             PCB* removed = List_remove(rQ[i]);
//             free(removed);
//             break;
//         }
//     }
//     return process;
// }

int countProcess(List** rq,List**srq)
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
            count = count + List_count(srq[i])
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
    List* SendReceiveWaitQueue[2];
    List* allQueues[5];
    int globalPID = 1;
    char input;
    char msg[40];

    ReadyQueues[0] = readyQueueHigh;
    ReadyQueues[1] = readyQueueNormal;
    ReadyQueues[2] = readyQueueLow;
    SendReceiveWaitQueue[0] = processSendWaitQueue;
    SendReceiveWaitQueue[1] = processReceiveWaitQueue;
    allQueues[0] = ReadyQueues[0];
    allQueues[1] = ReadyQueues[1];
    allQueues[2] = ReadyQueues[2];
    allQueues[3] = SendReceiveWaitQueue[0];
    allQueues[4] = SendReceiveWaitQueue[1];


    PCB* running = NULL;
    List* init_MSG = List_create();

    while(1)
    {
        scanf("%c",input);

        switch (input)
        {
            case 'C':
            {
                printf("Enter priority 0(HIGH),1(NORMAL),2(LOW)\n");
                scanf("%c",input);
                if(input == HIGH || input == NORMAL || input == LOW)
                {
                    PCB* pcbP = malloc(sizeof(PCB));
                    pcbP->PID = globalPID;
                    pcbP->priority = input;
                    pcbP->processState = READY;
                    pcbP->proc_message = List_create();

                    if(List_prepend(ReadyQueues[input],pcbP) == 0)
                    {
                        ++globalPID;
                        printf("Process PID:%d successfully created.",PID);
                    }
                    else
                    {
                        printf("Process PID:%d cannot be created.",PID);
                    }
                }
                else
                {
                    printf("Out of bound number.\n");
                }
                break;
            }

            case 'F':
            {
                PCB* forkedP = malloc(sizeof(PCB));
                forkedP->PID = globalPID;
                forkedP->priority = running->priority;
                forkedP->processState = READY;
                forkedP->proc_message = List_create();

                if(List_prepend(ReadyQueues[forkedP->priority],forkedP) == 0)
                {
                    ++globalPID;
                    printf("Process PID:%d successfully forked.",running->PID);
                }
                else
                {
                    printf("Process PID:%d cannot be forked.",running->PID);
                }
            }

            case 'K':
            {
                printf("Enter PID of the process to kill.\n");
                scanf("%c",input);

                if(countProcess(ReadyQueues,SendReceiveWaitQueue) == 0 && input == 0)
                {
                    printf("Exiting the program"\n);
                    closeUP(ReadyQueues,SendReceiveWaitQueue,running);
                    return 0;
                }
                else if (countProcess(ReadyQueues,SendReceiveWaitQueue) > 1 && input == 0)
                {
                    printf("Killing init is illegal while there are other processes are in the system"\n);
                }
                else if (running->PID == input)
                {
                    free(running);
                    printf("Killed the running process %d\n");
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
                else
                {
                    PCB* ret = searchProcessIfFoundRemove(allQueues,MAX_READY_QUEUE + MAX_BLOCK_QUEUE,input);
                    if(ret != NULL)
                    {
                        free(ret);
                        printf("Killed the process %d",input);
                    }
                    else
                    {
                        printf("Could not find the process in the system: %d",input);
                    }
                }
            }

            case 'E':
            {
                if(countProcess(ReadyQueues,SendReceiveWaitQueue) == 0)
                {
                    printf("Exiting the program"\n);
                    closeUP(ReadyQueues,SendReceiveWaitQueue,running);
                    return 0;
                }
                else
                {
                    free(running);
                    printf("Killed the running process %d\n");
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

            case 'Q':
            {
                if(countProcess(ReadyQueues,SendReceiveWaitQueue) == 0)
                {
                    printf("Ready queue is empty,Quantum not applicable\n");
                    break;
                }
                else
                {
                    running->processState = READY;
                    if(List_prepend(ReadyQueues[running->priority] == 0)
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
                            printf("Process %d is running now\n",input);
                        }
                    }
                }
            }

            case 'S':
            {
                printf("Enter PID of the process\n");
                scanf("%c",input);
                printf("Enter and message(max 40 chars)\n");
                scanf("%s",msg);

                if(countProcess(ReadyQueues,SendReceiveWaitQueue) == 0)
                {
                    printf("There is no process to send message\n");
                }
                else
                {
                    PCB* ret = searchProcessIfFoundRemove(allQueues,MAX_READY_QUEUE + MAX_BLOCK_QUEUE,input);
                    if(ret == NULL)
                    {
                        printf("No process with this PID lives in this system\n");
                    }
                    else
                    {
                        if(List_prepend(ret->proc_message, msg) == -1)
                        {
                            printf("Buffer overflow in Receiving process,message is getting dumped\n");
                        }
                        else
                        {
                            printf("Message has been sent to process %d\n",input);
                            running->processState = BLOCKED;
                            List_prepend(SendReceiveWaitQueue[0],running);
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

            case 'R':
            {
                if(List_count(running->proc_message) == 0) // RUNNING PROCESS HAS NO MESSAGE
                {
                    printf("There are no messages to be received, blocking the running process\n");
                    running->processState = BLOCKED;
                    List_prepend(SendReceiveWaitQueue[1],running);
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
                else // RUNNING PROCESS HAS A MESSAGE
                {
                    char* msg = List_trim(running->proc_message);
                    printf("Received message %s\n",msg);
                    free(msg);
                }   
            }

            case 'Y':
            {
                printf("Enter PID of the process\n");
                scanf("%c",input);
                printf("Enter and reply message(max 40 chars)\n");
                scanf("%s",msg);

                PCB* ret = searchProcessIfFoundRemove(processSendWaitQueue,1,input);
                if(ret == NULL)
                {
                    printf("No process with this PID blocked on \"Send\" in this system\n");
                }
                else
                {
                    ret->processState = READY;
                    if(List_prepend(ReadyQueues[running->priority] == 0)
                    {
                        printf("Running process %d has been placed back into %s priority queue", running->PID, priorityString[running->priority]);
                        if(List_prepend(ret->proc_message, msg) == -1)
                        {
                            printf("Buffer overflow in Sending process,reply is getting dumped\n");
                        }
                        else
                        {
                            printf("Reply has been sent to process %d\n",input);
                        }
                        
                    }
                }
            }
            case 'N':
            case 'P':
            case 'V':
            case 'I':
            case 'T':
        }
        if(running)
    }


}