#include <stdio.h>
#include <stdlib.h>
#include "fibers.h"
#include "config.h"
#include <math.h>




struct entity {
    double x;
    double y;
    double speed;
};


void fiber_update_speed(struct entity* e)
{
    for (int i = 0; i < 100; i++)
    {
        printf("hello (1) !\n");

        e->speed += sqrt(e->speed);
        yield();
    }
}

void fiber_update_pos(struct entity* e)
{
    for (int i = 0; i < 100; i++)
    {
        printf("hello (2) !\n");


        e->x += sqrt(e->speed);
        e->y += sqrt(e->speed);
        yield();
    }
}
int main(int argc, char **argv)
{
    (void)(argc);
    (void)(argv);
    FiberID ws[32];
    FiberID wp[32];


    struct entity entities[32];
    for(int i = 0; i < 32; i++  )
    {

        wp[i] = fiber_launch(fiber_update_pos, &entities[i]);
        ws[i] = fiber_launch(fiber_update_speed, &entities[i]);


    }

    for(int i = 0; i < 32; i++  )
    {
        fiber_join(ws[i]);
        fiber_join(wp[i]);
    }

    
    printf(" == end == ");
    return 0;
}
