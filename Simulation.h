#include "list.h"

enum state
{
    RUNNING,
    READY,
    BLOCKED
};

enum priority
{
    HIGH,
    NORMAL,
    LOW
};

typedef enum state state;
typedef enum priority priority;

char* priorityString[3]
{
    "HIGH",
    "NORMAL",
    "LOW"
};

typedef struct PCB
{
    int PID;
    int priority;
    state processState;
    List* proc_message;
} PCB;

typedef struct Semaphore
{
    int value;
    List* processListWaitingOnSemaphore;
} Semaphore;


