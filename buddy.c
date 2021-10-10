
//Coded by SURAKSHA MOTILAL 2108903

#include "buddy.h"
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

//Define your Constants here
const int MIN = 5;
const int LEVELS = 7;

enum AEflag
{
	Free,
	Taken
};

struct head
{
	enum AEflag status;
	//level is the size of the block
	short int level;
	struct head *next;
	struct head *prev;
};

struct head *flists[8] = {NULL};

//Complete the implementation of new here
struct head *new ()
{

	//map size 4KB to the address space
	struct head *block = mmap(
		/*put it at any memory location*/
		NULL,
		/*4kb*/
		1024 * sizeof(int),
		/* allows us to read and write to mem address*/
		PROT_READ | PROT_WRITE,

		MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	if (block == MAP_FAILED)
	{
		printf("Mapping Failed\n");
		return 1;
	}
	else
	{
		//set its values
		block->level = 7; //as it's the biggest block
		block->status = Free;
		block->next = NULL;
		block->prev = NULL;

		//set our highest level to the block
		flists[block->level] = block;
	}

	return block;
}

//Complete the implementation of level here
int level(int req)
{
	if (req > 0)
	{
		//to accommodate header size
		req = req + 24;
		//SMALLEST BLOCK: 32 bytes (2^5) -> level=0
		//LARGEST BLOCK: 4KB (2^12)-> level=7

		if (req <= 32)
		{
			return 0;
		}
		else if (req <= 64)
		{
			return 1;
		}
		else if (req <= 128)
		{
			return 2;
		}
		else if (req <= 256)
		{
			return 3;
		}
		else if (req <= 512)
		{
			return 4;
		}
		else if (req <= 1024)
		{
			return 5;
		}
		else if (req <= 2048)
		{
			return 6;
		}
		else if (req <= 4096)
		{
			return 7;
		}
	}

	//if it's -1, there is an error
	return -1;
}
void removeRef(struct head *ptr, int currLvl)
{
	struct head *tmpBlock = ptr;
	//if tmp block has a previous block
	if (tmpBlock->prev != NULL)
	{
		struct head *prevTmpBlock = tmpBlock->prev;

		//change its next pointer to the block that tmp block points to
		if (tmpBlock->next != NULL)
		{
			struct head *nextTmpBlock = tmpBlock->next;
			prevTmpBlock->next = nextTmpBlock;
			nextTmpBlock->prev = prevTmpBlock;
		}
		else
		//if tmp block doesn't point to a next block, the previous block now points to a null block
		{
			prevTmpBlock->next = NULL;
		}
	}
	else
	{
		//if tmp block doesnt have a previous block

		if (tmpBlock->next != NULL)
		{

			struct head *nextTmpBlock = tmpBlock->next;
			nextTmpBlock->prev = NULL;
			flists[currLvl] = nextTmpBlock;
			//since its previous block is gone, it doesnt point to a previous memory address space
		}
		else
		{
			//if we dont have a next or previous for that block, just remove it from that level entirely
			flists[currLvl] = NULL;
		}
	}
}

//we can split until we reach original level
//return memory address
//also set that block to ALLOCATED
//accounts for the fact that you now have to make that one null and the lower level now has 2 blocks to use
struct head *splitBlock(struct head *ptr, int currLvl, int originalLvl)
{
	int demoLvl = currLvl;
	//do this until we reach the original level

	// REMOVES REFERENCE TO THE CURRBLOCK in its original level!!
	//case when the block that is going to be split is in between other blocks, which will affect its next and prev pointers
	removeRef(ptr, currLvl);

	//split block and make its buddy attached to it
	struct head *currBlock = split(ptr);
	currBlock->level = demoLvl - 1;
	currBlock->status = Free;
	currBlock->next = NULL;

	struct head *buddyBlock = primary(currBlock);
	buddyBlock->level = demoLvl - 1;
	currBlock->prev = buddyBlock;
	buddyBlock->prev = NULL;
	buddyBlock->next = currBlock;
	buddyBlock->status = Free;

	//decrease the level as we have gone down one
	demoLvl--;

	//add it to list level if that level has nothing
	if (flists[demoLvl] == NULL)
	{
		flists[demoLvl] = buddyBlock;
	} //put currBlock there

	//add it to whatever content that level already has
	else
	{
		//if it has allocations there, add our curr block there + its buddy
		struct head *travBlock = flists[demoLvl];
		while (travBlock->next != NULL)
		{
			travBlock = travBlock->next;
		}
		travBlock->next = buddyBlock;
		buddyBlock->prev = travBlock;
	}

	if (demoLvl == originalLvl)
	{
		//allocate it
		buddyBlock->status = Taken;
		return buddyBlock;
	}
	else
	{
		return splitBlock(buddyBlock, demoLvl, originalLvl);
	}
}

//traverses through list to find available splitting
struct head *checkPrevLevels(int currLvl, int originalLvl)
{
	//=================PREVENTATIVE MEASURES==============================
	//means we could not find space
	if (currLvl == 8)
	{
		return NULL;
	}
	//if we reached the highest level without having found a solution, return NULL-> can't split at all.
	if (currLvl == 7 && flists[currLvl] == NULL)
	{
		return NULL;
	}

	//=====================================================================

	if (flists[currLvl] == NULL)
	{
		//check if the previous level isn't null
		return (checkPrevLevels(currLvl + 1, originalLvl));
	}
	else
	{

		//if we reached a block with something in it:

		//1. Check if there is at least some empty space in it:

		if (flists[currLvl]->status == Free)
		//check if curr block is unallocated and keep splitting accordingly
		{
			//if current level == original level then we just allocate it straight to that block
			if (currLvl == originalLvl)
			{
				flists[currLvl]->status = Taken;
				//return that current memory address
				return flists[currLvl];
			}
			else
			{
				return splitBlock(flists[currLvl], currLvl, originalLvl);
			}
		}
		else
		{
			//if flists[currlvl] isn't free
			//traverse continuously through block level to find a free one. If we don't, we go up again.
			struct head *currblock = flists[currLvl];
			while (currblock->next != NULL)
			{
				currblock = currblock->next;
				if (currblock->status == Free)
				{
					if (currLvl == originalLvl)
					{
						currblock->status = Taken;
						return currblock;
					}
					else
					{
						//split until we reach original level
						//return memory address-> which will end this segment of code
						return splitBlock(currblock, currLvl, originalLvl);
					}
				}
			}
			//since the while loop actually reached an end, we will need to traverse up again
			return checkPrevLevels(currLvl + 1, originalLvl);
		}
	}
}

void printLevel(int lvl)
{
	struct head *current = flists[lvl];
	while (current != NULL)
	{
		if (current->status == Taken)
		{
			printf("[Allocated (%p) ]", current);
		}
		else
		{
			printf("[Free (%p) ]", current);
		}
		current = current->next;
	}
	printf("%c", 0x0A);
}

void printFlists()
{
	printf("\n===================================\n");
	for (int i = 7; i >= 0; --i)
	{
		//Print out what the size of that block is
		if (i == 0)
		{
			printf("Level %d (size 32)\t:", i);
		}
		else if (i == 1)
		{

			printf("Level %d (size 64)\t:", i);
		}
		else if (i == 2)
		{
			printf("Level %d (size 128)\t:", i);
		}
		else if (i == 3)
		{
			printf("Level %d (size 256)\t:", i);
		}
		else if (i == 4)
		{
			printf("Level %d (size 512)\t:", i);
		}
		else if (i == 5)
		{
			printf("Level %d (size 1024)\t:", i);
		}
		else if (i == 6)
		{
			printf("Level %d (size 2048)\t:", i);
		}
		else if (i == 7)
		{
			printf("Level %d (size 4096)\t:", i);
		}
		struct head *current = flists[i];
		if (current != NULL)
		{
			printLevel(i);
		}
		else
		{
			printf("(Null)%c", 0x0A);
		}
	}
}
//Complete the implementation of balloc here
void *balloc(size_t size)
{
	//if flists is empty, make a new space
	bool empty = true;
	for (int i = 0; i < 8; i++)
	{
		if (flists[i] != NULL)
		{
			empty = false;
			break;
		}
	}

	if (empty == true)
	{
		//make a new block for flists to have something
		struct head *nBlock = new ();
	}

	struct head *allocAddress;

	//get level
	int currLvl = level(size);
	printf("\n===================================\n");
	//if someone requests for more than 4096
	if (currLvl == -1)
	{
		printf("Unable to allocate that amount. Request exceeds available size.\n");
	}
	else
	{
		printf("\nRequested for level %d with size %d  %c", currLvl, size + 24, 0x0A);

		//check for unallocated blocks in that level. If there aren't, then we have to go up a level and split it, continuously

		allocAddress = checkPrevLevels(currLvl, currLvl);
		if (allocAddress != NULL)
		{
			//SUCCESSFULLY ALLOCATED
			printFlists();
			printf("\nAllocated to %p with hidden header %p\n", allocAddress, hide(allocAddress));
			return (hide(allocAddress));
		}
		else
		{
			printf("\nUnable to allocate\n");
		}
	}
	printf("===================================\n");
}

void mergeMemory(struct head *block)
{
	//check if its primary is free too-> but only if the primary isn't it
	struct head *primeBlock = primary(block);
	struct head *bud = buddy(block);
	//*****************
	//IF PRIME BLOCKS ARE EQUAL, CHECK IF ITS BUDDY IS FREE AND DO THE SAME TOO!
	//TODO TODO TODO
	//***************************

	if (primeBlock->status == Free && primeBlock != block)
	{
		//can merge and go up the tree
		//take the primary and change its level

		//sort out pointers of previous and next
		if (primeBlock->prev != NULL)
		{
			struct head *prevBlock = primeBlock->prev;

			if (primeBlock->next != NULL && primeBlock->next != block)
			{
				primeBlock->next->prev = prevBlock;
				prevBlock->next = primeBlock->next;
			}
			else if (primeBlock->next == block)
			{ //if there is a next block
				if (block->next != NULL)
				{
					struct head *nextBlock = block->next;
					prevBlock->next = nextBlock;
					nextBlock->prev = prevBlock;
				}
				else
				{
					prevBlock->next = NULL;
				}
			}
			else if (primeBlock->prev == block)
			{
				if (block->prev == NULL)
				{
					if (primeBlock->next == NULL)
					{
						flists[block->level] = NULL;
					}
					else
					{
						flists[block->level] = primeBlock->next;
					}
				}
				else
				{
					if (primeBlock->next == NULL)
					{
						block->prev->next = NULL;
					}
					else
					{
						block->prev->next = primeBlock->next;
						primeBlock->next->prev = block->prev;
					}
				}
			}
			else if (primeBlock->next == NULL)
			{
				prevBlock->next = NULL;
			}
		}

		else
		{
			if (primeBlock->next != NULL && primeBlock->next != block)
			{
				primeBlock->next->prev = NULL;
				flists[block->level] = primeBlock->next;
			}
			else if (primeBlock->next == block)
			{
				//if there is no previous
				if (block->next != NULL)
				{
					struct head *nextBlock = block->next;
					nextBlock->prev = NULL;
					flists[block->level] = nextBlock;
				}
				else
				{
					//as there are no other blocks there
					flists[primeBlock->level] = NULL;
				}
			}
		}

		//account for if the block is in another place
		if (block->prev != NULL && block->prev != primeBlock)
		{
			if (block->next == NULL)
			{
				block->prev->next = NULL;
			}
			else
			{
				block->prev->next = block->next;
				block->next->prev = block->prev;
			}
		}
		else if (block->prev == NULL)
		{
			if (block->next != NULL && block->next != primeBlock)
			{
				flists[primeBlock->level] = block->next;
			}
		}

		primeBlock->next = NULL;
		primeBlock->prev = NULL;
		//goes up a level
		//get rid of buddy
		block = NULL;
		primeBlock->level = primeBlock->level + 1;

		//wedge it into the previous level
		//Update flists and free the block which the pointer points to
		struct head *currLvlBlock = flists[primeBlock->level];
		if (currLvlBlock == NULL)
		{
			flists[primeBlock->level] = primeBlock;
			return;
		}
		else
		{
			while (currLvlBlock->next != NULL)
			{
				currLvlBlock = currLvlBlock->next;
			}
			currLvlBlock->next = primeBlock;
			primeBlock->prev = currLvlBlock;
			//Perform merging up the list if possible
			mergeMemory(primeBlock);
		}
	}
	else //need to check if its buddy is free too in the case the block itself is the primary block
		if (bud == block->next && bud->status == Free)
	{

		//sort out pointers of previous and next
		if (block->prev != NULL)
		{
			struct head *prevBlock = block->prev;
			//if there is a next block
			if (bud->next != NULL)
			{
				struct head *nextBlock = bud->next;
				prevBlock->next = nextBlock;
				nextBlock->prev = prevBlock;
			}
			else
			{
				prevBlock->next = NULL;
			}
		}
		else
		{
			//if there is no previous
			if (bud->next != NULL)
			{
				struct head *nextBlock = bud->next;
				nextBlock->prev = NULL;
				flists[block->level] = nextBlock;
			}
			else
			{
				//as there are no other blocks there
				flists[block->level] = NULL;
			}
		}

		block->next = NULL;
		block->prev = NULL;
		//goes up a level
		//get rid of buddy
		bud = NULL;
		block->level = block->level + 1;

		//wedge it into the previous level
		//Update flists and free the block which the pointer points to
		struct head *currLvlBlock = flists[block->level];
		if (currLvlBlock == NULL)
		{
			flists[block->level] = block;
			return;
		}
		else
		{
			while (currLvlBlock->next != NULL)
			{
				currLvlBlock = currLvlBlock->next;
			}
			currLvlBlock->next = block;
			block->prev = currLvlBlock;
			//Perform merging up the list if possible
			mergeMemory(block);
		}

		//do nothing, as we are done with the merges
		return;
	}
	else if (bud == block->prev && bud->status == Free)
	{

		if (bud->prev == NULL)
		{
			if (block->next != NULL)
			{
				flists[block->level] = block->next;
			}
			else
			{
				flists[block->level] = NULL;
			}
		}
		else
		{
			//bud's prev isn't null
			if (block->next != NULL)
			{
				bud->prev->next = block->next;
				block->next->prev = bud->prev;
			}
			else
			{
				bud->prev->next = NULL;
			}
		}
		bud->next = NULL;
		bud->prev = NULL;
		//goes up a level
		//get rid of buddy
		block = NULL;
		bud->level = bud->level + 1;

		//wedge it into the previous level
		//Update flists and free the block which the pointer points to
		struct head *currLvlBlock = flists[bud->level];
		if (currLvlBlock == NULL)
		{
			flists[bud->level] = bud;
			return;
		}
		else
		{
			while (currLvlBlock->next != NULL)
			{
				currLvlBlock = currLvlBlock->next;
			}
			currLvlBlock->next = bud;
			bud->prev = currLvlBlock;
			//Perform merging up the list if possible
			mergeMemory(bud);
		}

		return;
	}
}

//Complete the implementation of bfree here

//memory is the place we want to free
void bfree(void *memory)
{

	printf("\n===================================\n");
	printf("LIST BEFORE:");
	printFlists();
	printf("\nAddress that was searched for with header unhidden: %p\n", magic(memory));
	//Check memory is a null pointer
	if (memory == NULL)
	{
		//checks if nothing exists there
		printf("The address you requested returns a NULL pointer. Unable to free memory.\n");
	}
	else
	{
		//Get the header of the pointer
		//memory given is one that doesn't include the header pointer
		struct head *block = magic(memory);
		printf("\nLevel we would like to free: %d\n", block->level);
		//set as unallocated
		block->status = Free;

		//the function below does the following:
		//put it as unallocated
		//check if it can be merged into 2 blocks and shifted up a level
		//check if parent blocks can also be merged into 2 blocks
		mergeMemory(block);

		//display what the list looks like after it has been freed
		printf("LIST AFTER:");
		printFlists();
	}
}

void testHelpers()
{
	//test my new function
	struct head *block = new ();
	blockinfo(block);

	//test splitting
	struct head *splitBlock = split(block);
	splitBlock->level = 6;

	//get the primary block from the split
	struct head *primBlock = primary(splitBlock);
	primBlock->level = 6;
	primBlock->next = splitBlock;
	primBlock->prev = NULL;
	splitBlock->next = NULL;
	splitBlock->prev = primBlock;

	printf("Primary test\n");
	blockinfo(primBlock);

	printf("Splitting test\n");
	blockinfo(splitBlock);

	//checking what will happen if buddy is called multiple times
	printf("Buddy test-> primary block with address %p\n", primBlock);
	struct head *buddyBlock = buddy(primBlock);
	buddyBlock->level = 6;

	blockinfo(buddyBlock);

	buddyBlock = buddy(splitBlock);
	buddyBlock->level = 6;
	printf("Buddy test-> split block with address %p\n", splitBlock);
	blockinfo(buddyBlock);
	//checking splitting again and buddies on multiple levels

	struct head *splitBlock2 = split(splitBlock);
	splitBlock2->level = 5;

	struct head *primBlock2 = primary(splitBlock2);
	primBlock2->level = 5;

	primBlock2->next = splitBlock2;
	primBlock2->prev = NULL;
	splitBlock2->next = NULL;
	splitBlock2->prev = primBlock2;

	printf("Primary test 2\n");
	blockinfo(primBlock2);

	//Test results-> primary is the start of the block of the split

	printf("Splitting test 2\n");
	blockinfo(splitBlock2);

	//Checking if a merge will allow it to go back up the list and find its proper buddy again
	printf("Should return primBlock's address: %p==%p\n", buddy(splitBlock2), primBlock2);
	primBlock2->level = 6;
	printf("Should return primBlock's address: %p == %p\n", primary(primBlock2), primBlock);

	//checking hiding and unhiding
	printf("hide");
	struct head *hiddenblockhead = hide(block);
	blockinfo(hiddenblockhead);

	printf("unhide");
	struct head *unhiddenblockhead = magic(hiddenblockhead);
	blockinfo(unhiddenblockhead);

	//test my level function
	int leveltest1 = level(32);
	printf("min required level = %d ==1\n", leveltest1);
	int leveltest2 = level(100);
	printf("min required level = %d ==3\n", leveltest2);
	int leveltest3 = level(1500);
	printf("min required level = %d ==6\n", leveltest3);
}

void testBalloc()
{
	//checking for huge size
	balloc(4092); //works-> should fail
	//checking Jared's test case
	struct head *ptr5 = balloc(32); //works
	struct head *ptr4 = balloc(32); //works
	struct head *ptr3 = balloc(32); //works

	//testing lower levels
	struct head *ptr6 = balloc(8); //works
	struct head *ptr7 = balloc(8); //works

	//testing when there are no more size 8
	struct head *ptr8 = balloc(8); //works

	//testing higher levels
	struct head *ptr9 = balloc(740); //works
	struct head *ptr1 = balloc(740); //works
	struct head *ptr2 = balloc(740); //works

	//testing when there are no more blocks to allocate
	balloc(740); //works-> should fail

	//PLEASE NOTE: All these tests were performed after my balloc tests were run so that the memory of contents were allocated to/ NULL for freeing

	//test pointer with contents in it
	bfree(ptr1); //trying to remove a size 740 block that was allocated earlier and whose head was hidden
	bfree(ptr2); //trying to 2 buddies next to one another to check merging up the list

	// //more testing
	// //free the 56 that is next to a free block (free block to the right, rather than to the left-> a buddy is free before a primary block)
	bfree(ptr3); //this should not merge as they are not continuous in memory (there are size 32 blocks in between them)
	bfree(ptr4); //this should not merge either as its buddy is allocated
	bfree(ptr5); //this should merge as its budy is free as well

	//DE-ALLOCATE EVERYTHING!
	bfree(ptr6);
	bfree(ptr7);
	bfree(ptr8);
	bfree(ptr9);
}

//Helper Functions
struct head *buddy(struct head *block)
{
	int index = block->level;
	long int mask = 0x1 << (index + MIN);
	return (struct head *)((long int)block ^ mask);
}

struct head *split(struct head *block)
{
	int index = block->level - 1;
	int mask = 0x1 << (index + MIN);
	return (struct head *)((long int)block | mask);
}

struct head *primary(struct head *block)
{
	int index = block->level;
	long int mask = 0xffffffffffffffff << (1 + index + MIN);
	return (struct head *)((long int)block & mask);
}

void *hide(struct head *block)
{
	return (void *)(block + 1);
}

struct head *magic(void *memory)
{
	return ((struct head *)memory - 1);
}

void dispblocklevel(struct head *block)
{
	printf("block level = %d\n", block->level);
}
void dispblockstatus(struct head *block)
{
	printf("block status = %d\n", block->status);
}

void blockinfo(struct head *block)
{
	printf("===================================================================\n");
	dispblockstatus(block);
	dispblocklevel(block);
	printf("start of block in memory: %p\n", block);
	printf("size of block in memory: %ld in bytes\n", sizeof(struct head));
	printf("===================================================================\n");
}
