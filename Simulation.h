#include "list.h"

enum state
{
    RUNNING,
    READY,
    BLOCKED
};

static const char *priorityString[] = {
    "HIGH", "NORMAL", "LOW"
};

enum priority
{
    HIGH,
    NORMAL,
    LOW
};

typedef enum state state;
typedef enum priority priority;

typedef struct PCB
{
    int PID;
    int priority;
    state processState;
    List* proc_message;
    int receivedUnblocked;
} PCB;

typedef struct Semaphore
{
    int value;
    List* processListWaitingOnSemaphore;
} Semaphore;


