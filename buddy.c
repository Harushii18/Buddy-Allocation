
#include "buddy.h"
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
//QUESTIONS TO ASK TUTORS:
//why is block memory size 24?
// why is my block's value not changing in the debugger?


//Define your Constants here

//TODO: Must change this to a valid num!!
const MIN=5;

enum AEflag { Free, Taken };

struct head {
	enum AEflag status;
	//level is the size of the block
	short int level;
	struct head* next;
	struct head* prev;
};


//Complete the implementation of new here
struct head *new() {

    //map size 4KB to the address space
    struct head *block = mmap ( 
    /*put it at any memory location*/
    NULL, 
    /*4kb*/
    1024*sizeof(int),
    /* allows us to read and write to mem address*/
     PROT_READ | PROT_WRITE, 
     
     MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );
    
    if(block == MAP_FAILED){
        printf("Mapping Failed\n");
        return 1;
    }else{
        //set its values
        block->level=7; //as it's the biggest block
        block->status=Free;
        block->next=NULL;
        block->prev=NULL;
    }
    
    return block;
}

//Complete the implementation of level here
int level(int req) {
	//SMALLEST BLOCK: 32 bytes (2^5) -> level=0
	//LARGEST BLOCK: 4KB (2^12)-> level=7
	if (req<=32){
		return 0;
	}else if (req<=64){
		return 1;
	}else if (req<=128){
		return 2;
	}else if (req<=256){
		return 3;
	}else if (req<=512){
		return 4;
	}else if (req<=1024){
		return 5;
	}else if (req<=2048){
		return 6;
	}else if (req<=4096){
		return 7;
	}

	//if it's -1, there is an error 
	return -1;
}

//Complete the implementation of balloc here
void* balloc(size_t size) {
	



}

//Complete the implementation of bfree here
void bfree(void* memory) {
	




}



//Helper Functions
struct head* buddy(struct head* block) {
	int index = block->level;
	long int mask = 0x1 << (index + MIN);
	return (struct head*)((long int)block ^ mask);

}

struct head* split(struct head* block) {
	int index = block->level - 1;
	int mask = 0x1 << (index + MIN);
	return (struct head*)((long int)block | mask);
}

struct head* primary(struct head* block) {
	int index = block->level;
	long int mask = 0xffffffffffffffff << (1 + index + MIN);
	return (struct head*)((long int)block & mask);
}

void* hide(struct head* block) {
	return (void*)(block + 1);
}


struct head* magic(void* memory) {
	return ((struct head*)memory - 1);
}
	


void dispblocklevel(struct head* block){
	printf("block level = %d\n",block->level);
}
void dispblockstatus(struct head* block){
	printf("block status = %d\n",block->status);
}

void blockinfo(struct head* block){
	printf("===================================================================\n");
	dispblockstatus(block);
	dispblocklevel(block);
	printf("start of block in memory: %p\n", block);
	printf("size of block in memory: %ld in bytes\n",sizeof(struct head));
	printf("===================================================================\n");
}






