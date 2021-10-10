#include "buddy.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(){

//Uncomment to test the helper functions
//testHelpers();

//Uncomment to test balloc
testBalloc();


// void *init = sbrk(0);
// void *current;
// printf("The initial top of the heap is %p.\n", init);

// struct head* block = new();

// blockinfo(block);

// current = sbrk(0);
// int allocated = (int)((current - init) / 1024);
// printf("The current top of the heap is %p. \n", current);
// printf("increased by %d Kbyte\n", allocated);


// printf("buddy");
// struct head* bud=buddy(block);
// blockinfo(bud);

// printf("splitted");
// struct head* splitted = split(block);
// blockinfo(splitted);

// printf("buddy");
// struct head* bud = buddy(splitted);
// blockinfo(bud);

// printf("primes");
// struct head* primes = primary(bud);
// blockinfo(primes);

// printf("primeb");
// struct head* primeb = primary(splitted);
// blockinfo(primeb);

// printf("hide");
// struct head* hiddenblockhead = hide(block);
// blockinfo(hiddenblockhead);

// printf("unhide");
// struct head* unhiddenblockhead = magic(hiddenblockhead);
// blockinfo(unhiddenblockhead);

// int leveltest1 = level(32);
// printf("min required level = %d\n",leveltest1);
// int leveltest2 = level(100);
// printf("min required level = %d\n",leveltest2);
// int leveltest3 = level(1500);
// printf("min required level = %d\n",leveltest3);

return 0;
}
