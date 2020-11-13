#include "list.h"
#include "Simulation.h"
#define MAX_READY_QUEUE 3

bool compare(void* pItem,void* pCompArg)
{

    if((PCB*)pItem->PID == (PCB*)pCompArg->PID)
        return 1;
    return 0;
}

void List_free(List* pList, FREE_FN pItemFreeFn);

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

PCB* fetchProcess(List* rQ)
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

PCB* searchProcess(List* rQ,int PID)
{
    PCB* process = NULL;
    for(int i = 0; i < MAX_READY_QUEUE; ++i)
    {   
        // bool (*functionPtr)(void*,void*);
        PCB temp;
        // functionPtr = &compare;
        temp.PID = input - '0';
        process = List_search(rQ[i],&compare,&temp);
    }

    return process;
}

int main()
{
    List* readyQueueHigh = List_create();
    List* readyQueueNormal = List_create();
    List* readyQueueLow = List_create();
    List** ReadyQueues[3];
    List* processSendWaitQueue = List_create();
    List* processReceiveWaitQueue = List_create();
    List** SendReceiveWaitQueue[2];
    int globalPID = 1;
    char input;
    char msg[40];
    int processCount = 1;

    ReadyQueues[0] = readyQueueHigh;
    ReadyQueues[1] = readyQueueNormal;
    ReadyQueues[2] = readyQueueLow;
    SendReceiveWaitQueue[0] = processSendWaitQueue;
    SendReceiveWaitQueue[1] = processReceiveWaitQueue;

    PCB* running;

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
                        ++processCount;
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
                if(processCount == 1)
                {
                    printf("Attempting to fork init is illegal.\n");
                    break;
                }
                PCB* forkedP = malloc(sizeof(PCB));
                forkedP->PID = globalPID;
                forkedP->priority = running->priority;
                forkedP->processState = READY;
                forkedP->proc_message = List_create();

                if(List_prepend(ReadyQueues[running->priority],forkedP) == 0)
                {
                    ++processCount;
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

                if(processCount == 1 && input == 0)
                {
                    printf("Exiting the program"\n);
                    closeUP(ReadyQueues,SendReceiveWaitQueue,running);
                    return 0;
                }
                else
                {
                    PCB* ret = searchProcess(ReadyQueues, input);
                    if(ret != NULL)
                    {
                        printf("Killing process %d",input);
                        free(ret);
                    }
                    else
                    {
                        printf("Could not find the process: %d",input);
                    }
                }
            }

            case 'E':
            {
                if(processCount == 1)
                {
                    printf("Exiting the program"\n);
                    closeUP(ReadyQueues,SendReceiveWaitQueue,running);
                    return 0;
                }
                else
                {
                    printf("Removing running process %d",running->PID);
                    free(running);
                    --processCount;

                    PCB* fetched = fetchProcess(ReadyQueues);
                    if(fetched != NULL)
                    {
                        running = ret;
                        fetched->processState = RUNNING;
                        printf("Process %d is running now",running->PID);
                    }
                    else
                    {
                        printf("Could not find any process,FATAL ERROR");
                    }
                }

            }
            case 'Q':
            {
                if(processCount == 1)
                {
                    printf("Attempting to Quantum init is illegal.\n");
                    break;
                }
                else
                {
                    running->processState = READY;
                    if(List_prepend(ReadyQueues[running->priority] == 0)
                    {
                        printf("Running process %d has been placed back into %s queue", running->PID, priorityString[running->priority]);

                        PCB* fetched = fetchProcess(ReadyQueues);
                        if(ret != NULL)
                        {
                            running = fetched;
                            fetched->processState = RUNNING;
                            printf("Process %d is running now",running->PID);
                        }
                        else
                        {
                            printf("Could not find any process,FATAL ERROR");
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

                if(processCount == 1)
                {
                    printf("Attempting Send to init is illegal.\n");
                    break;
                }
                else
                {
                    PCB* ret = searchProcess(rQ,input);
                    if(ret == NULL)
                    {
                        printf("No process with this id lives in this system\n");
                    }
                    else
                    {
                        List_prepend(ret->proc_message, msg);
                        running->processState = BLOCKED;
                        List_prepend(SendReceiveWaitQueue[0],running);
                        PCB* fetched = fetchProcess(ReadyQueues);
                        fetched->processState = RUNNING;
                        running = fetched;
                    }
                }
            }
            case 'R':
            {
                if(processCount == 1)
                {
                    printf("Attempting Receive to init is illegal.\n");
                    break;
                }
                else
                {
                    if(List_count(running->proc_message) == 0)
                    {
                        printf("There are no messages to be received blocking\n");
                        running->processState = BLOCKED;
                        List_prepend(SendReceiveWaitQueue[1],running);
                        PCB* fetched = fetchProcess(ReadyQueues);
                        fetched->processState = RUNNING;
                        running = fetched;
                    }
                    else
                    {
                        char* msg = List_trim(running->proc_message);
                        if(msg == NULL)
                        {
                            printf("NULL message\n");
                        }
                        else
                        {
                            printf("Received message %s\n",msg);
                            free(msg);
                        }
                        
                    }
                }
            }
            case 'Y':
            {
                
            }
        }
    }


}