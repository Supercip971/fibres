#include "external/vec.h"
#include "fibers.h"
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>

#define FIBER_STACK_SIZE 8192*2

vec_t(Fiber*) fiber_table = {};

static int fiber_current_table_id = 0;
static int next_fiber_uid = 0;
static int to_free_count = 0;

static Fiber* get_fiber(FiberID id) 
{
    for(int i = 0; i < fiber_table.length; i++)
    {
        if(fiber_table.data[i]->id == id)
        {
            return fiber_table.data[i];
        }
    }
    return NULL;
}
/* 
 This function launch the current main function as a fiber 
 it's transforming the current function into a fiber stack.

*/

static Fiber* fiber_alloc(void)
{
    
    for(int i = 0; i < fiber_table.length; i++)
    {
        if(fiber_table.data[i]->state == FIBER_STATE_FREE)
        {
            Fiber* c =  fiber_table.data[i];
            return c;
        }
    }
    Fiber* f = malloc(sizeof(Fiber));

    assert(f != NULL);
    *f = (Fiber){};

    vec_push(&fiber_table, f); 
    return f;

}
static void make_current_as_fiber()
{

    Fiber* fiber = fiber_alloc();
    *fiber = (Fiber){0};
    fiber->state = FIBER_STATE_RUNNING;
    fiber->id = 0;
    next_fiber_uid++;
    fiber_current_table_id = 0;
}


static bool update_blocker(FiberBlocker blocker)
{
    if(blocker.fn == NULL)
    {
        return true;
    }
    return blocker.fn(blocker.ctx);
}
static void _fiber_entry(void)
{
    Fiber* fiber = fiber_table.data[fiber_current_table_id];
    fiber->func(fiber->args);


    fiber->state = FIBER_STATE_DEAD;

    if(fiber->waiter_count == 0)
    {

        to_free_count++;
    }
    yield();
}
static FiberID fiber_launch_impl(FiberFn func, void* args, FiberState status)
{

    Fiber* fiber = fiber_alloc();

    void* stack_pointer = malloc(FIBER_STACK_SIZE);
    *fiber = (Fiber){
        .func = func, 
        .args = args, 
        .stack = stack_pointer, 
        .stack_size = FIBER_STACK_SIZE,
        .state = status,
        .id = next_fiber_uid
    };

    fiber->ctx = (FiberCtx) {
        .sp = (uintptr_t)stack_pointer + FIBER_STACK_SIZE - 8,
        .rbp = (uintptr_t)stack_pointer + FIBER_STACK_SIZE,
        .ip = (uintptr_t)_fiber_entry,
        .rbx = (uintptr_t)args,
        .mxcsr = 0x1F80, // default mxcsr value at reset (intel manual volume 1 - 11.6.4)
        .x86_fcw = 0x37F, // default x86 control word at reset (intel manual volume 1 - 8.1.5)
    };

    next_fiber_uid++;

    return fiber->id;
}
static FiberID fiber_launch_idle(FiberFn func, void* args)
{
    return fiber_launch_impl(func, args, FIBER_STATE_IDLE);
}


int fiber_runnable_counts(void)
{
    int count = 0;
    for(int i = 0; i < fiber_table.length; i++)
    {
        if(fiber_table.data[i]->state == FIBER_STATE_RUNNING || 
            fiber_table.data[i]->state == FIBER_STATE_WAITING)
        {
            count++;
        }
    }
    return count;
}
static void _fiber_idle_loop(void* args)
{
    (void)args;
    while(true)
    {

        for(int i = 0; i < fiber_table.length; i++)
        {
            Fiber* current = fiber_table.data[i];
            
            if(current->state == FIBER_STATE_WAITING)
            {
                if(update_blocker(current->blocker))
                {
                    current->state = FIBER_STATE_RUNNING;
                    yield();
                }
            }

        }

        if(fiber_runnable_counts() == 0)
        {
            printf("No more runnable fiber, exiting\n");
            exit(0);
        }

    }
}
static void fiber_init_if_needed(void) {

    if(fiber_table.data == NULL)
    {
        vec_init(&fiber_table);
        make_current_as_fiber();
        fiber_launch_idle(_fiber_idle_loop, NULL);
    }
}




FiberID fiber_launch(FiberFn func, void* args)
{
    fiber_init_if_needed();
    return fiber_launch_impl(func, args, FIBER_STATE_RUNNING);
}



static bool fiber_waiter(void* ctx)
{
    Fiber* target = ctx;
    if(target->state == FIBER_STATE_DEAD)
    {
        return true;
    }
    return false;
}

void fiber_join(FiberID id)
{
    Fiber* fiber = get_fiber(id);
    if(fiber == NULL)
    {
        return;
    }

    if(fiber->state == FIBER_STATE_DEAD)
    {
        return;
    }
    if(fiber->state == FIBER_STATE_FREE)
    {
        return;
    }


    fiber->waiter_count += 1;
    fiber_block(fiber_waiter, fiber);
    fiber->waiter_count -= 1;

    if(fiber->waiter_count == 0 && fiber->state == FIBER_STATE_DEAD)
    {

        to_free_count++;
    }
    return;    
}

Fiber* fiber_self(void)
{
    fiber_init_if_needed();
    return fiber_table.data[fiber_current_table_id];
}



static int idle_fiber()
{
    for(int i = 0; i < fiber_table.length; i++)
    {
        if(fiber_table.data[i]->state == FIBER_STATE_IDLE)
        {
            return i;
        }
    }
    return -1;
}

static void fiber_frees(void)
{
    
    for(int i = 0; i < fiber_table.length; i++)
    {
        if(to_free_count == 0)
        {
            return;
        }
 
        Fiber* current = fiber_table.data[i];
        if(current->state == FIBER_STATE_DEAD && current->waiter_count == 0)
        {

            free(current->stack);
            current->id = -1;
            current->state = FIBER_STATE_FREE;
            to_free_count -= 1;
        }
    }

    
}
static int fiber_next( int start_from)
{
    for(int i = start_from; i < fiber_table.length; i++)
    {
        if(fiber_table.data[i]->state == FIBER_STATE_RUNNING)
        {
            return i;
        }
        else if(fiber_table.data[i]->state == FIBER_STATE_WAITING && update_blocker(fiber_table.data[i]->blocker))
        {
            fiber_table.data[i]->state = FIBER_STATE_RUNNING;
            return i;
        }
    }
    
    if(start_from == 0)
    {
        return -1;
    }
    // ok so we checked every fiber from current so now, we check all of them from 0
    // before: 
    // (a) (b) (c) (current) | [x] [b] [c]
    // after: 
    // | [a] [b] [c] [current] [x] [b] [c]
 
    return fiber_next(0);
}

bool fiber_block(FiberBlockerFn fn, void* ctx)
{
    Fiber* fiber = fiber_self();
    fiber->state = FIBER_STATE_WAITING;
    fiber->blocker = (FiberBlocker){.fn = fn, .ctx = ctx};
    yield();
    return true;
}

extern void fibers_switch(void* from, void* to);

void yield(void)
{
    fiber_frees();
    while(fiber_table.length > 0 && fiber_table.data[fiber_table.length - 1]->state == FIBER_STATE_FREE)
    {
        free(fiber_table.data[fiber_table.length - 1]);
        (void)vec_pop(&fiber_table);
    }
    Fiber* previous = fiber_self();
    int next_id = fiber_next(fiber_current_table_id+1);

    if(next_id < 0)
    {

        next_id = idle_fiber();
    }

    Fiber* next = fiber_table.data[next_id];


    if(previous == next)
    {
        return; // no need to switch context if we are already on the next fiber
    }

    fiber_current_table_id = next_id;
    fibers_switch(&previous->ctx, &next->ctx);
    
}

void fiber_exit(void)
{
    Fiber* fiber = fiber_self();
    fiber->state = FIBER_STATE_DEAD;
    yield();
}

static uint64_t current_time_ms()
{
    struct timeval time; 
    gettimeofday(&time, NULL); 
    uint64_t milliseconds = time.tv_sec*1000LL + time.tv_usec/1000; 
    return milliseconds;
}


static bool fiber_sleep_blocker(void* ctx)
{
    uint64_t ms = (uint64_t)ctx;
    uint64_t now = current_time_ms();
    
    if(now >= ms)
    {
        return true;
    }
    return false;
}


void fiber_sleep(uint64_t ms)
{
    Fiber* fiber = fiber_self();
    uint64_t end = current_time_ms() + ms;
    fiber->state = FIBER_STATE_WAITING;
    fiber->blocker = (FiberBlocker){.fn = fiber_sleep_blocker, .ctx = (void*)end};
    yield();
}

int fiber_count(void)
{
    return fiber_table.length;
}
