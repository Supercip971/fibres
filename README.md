
# Fibers (or coroutines) in C99 !

Just a sample project for a blog article I'm writing about fibers in C (for linux), that you can read here: [cyp.sh/blog/coroutines-in-c](https://cyp.sh/blog/coroutines-in-c) .


It's mainly inspired by an old code I wrote for [Brutal](https://github.com/brutal-org/brutal).

It's an experimentation and it should be used for learning purpose, there is some issues with 
this code.


## Examples: 

```c

#include "fibers.h" 

void foo(void* args)
{
    for(int i = 0; i < 10; i++)
    {
        printf("hello");
        yield(); 
    }
}

void bar(void* args)
{
    for(int i = 0; i < 20; i++)
    {
        printf(" world! \n");
        yield();
    }
}

int main(int argc, char** argv) 
{
    
    FiberID a = fiber_launch(foo, NULL);
    FiberID b = fiber_launch(bar, NULL);

    fiber_join(a);
    fiber_join(b);

    return 0;

}
```

