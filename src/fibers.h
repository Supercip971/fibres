#ifndef FIBERS_H
#define FIBERS_H

#include <stdint.h>
#include <stdbool.h>
typedef struct 
{

    uint64_t ip; 

    uint64_t sp;
    uint64_t rbx, rbp, r12, r13, r14, r15;
  
    uint32_t mxcsr; 
    uint32_t x86_fcw;

    
} FiberCtx;


typedef enum 
{
    FIBER_STATE_FREE,
    FIBER_STATE_READY,
    FIBER_STATE_RUNNING,
    FIBER_STATE_WAITING,
    FIBER_STATE_IDLE,
    FIBER_STATE_ERROR,
    FIBER_STATE_DEAD,
} FiberState;

typedef bool (*FiberBlockerFn)(void* ctx);  

typedef void (*FiberFn)();

typedef struct Fiber Fiber;
typedef struct 
{
    FiberBlockerFn fn;
    void* ctx;

    bool ended;
} FiberBlocker;

typedef int FiberID;
 struct Fiber {
    uint8_t* stack;
    uint64_t stack_size;
    uint64_t stack_top;
    FiberCtx ctx;

    void * args;
    FiberState state;
    FiberBlocker blocker;
    int waiter_count; 

    FiberFn func;
    FiberID id;
};


void fiber_init(void);


void yield(void);

FiberID fiber_launch(FiberFn func, void* args);

void fiber_exit(void);

Fiber* fiber_self(void);

bool fiber_block(FiberBlockerFn fn, void* ctx);
void fiber_sleep(uint64_t ms);
void fiber_join(FiberID fiber);

int fiber_count(void);

#endif