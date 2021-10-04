#
# Typing 'make' or 'make count' will create the executable file.
#

# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC = gcc
CFLAGS  = -g -Wall

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: buddytest

# To create the executable file count we need the object files
#
buddytest:  test.o buddy.o
	$(CC) $(CFLAGS) -o buddytest test.o buddy.o 

# To create the object file test.o, we need the source
# files 
#
test.o:  test.c buddy.h 
	$(CC) $(CFLAGS) -c test.c

# To create the object file buddy.o, we need the source files
#
buddy.o:  buddy.c buddy.h 
	$(CC) $(CFLAGS) -c buddy.c



# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	$(RM) buddytest *.o *~