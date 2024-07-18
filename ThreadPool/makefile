#
# 'make depend' uses makedepend to automatically generate dependencies
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

FILE_NAME = thread_pool#$(TARGET)

VER = $1
# define the C compiler to use
CC = g++

CFLAGS = $(GC)
VERSION = $(VER)
# define any compile-time flags

ifeq ($(VER), debug)
VERSION = _$(VER)
define CFLAGS =
	$(GD)
endef
else
define CFLAGS =
	$(GC) 
endef
endif



# define debug CFLAGS
GC = -ansi -pedantic-errors -Wall -Wextra -DNDEBUG -O3
GD = -ansi -pedantic-errors -Wall -Wextra -g 
VLG = valgrind --leak-check=yes --track-origins=yes
# define any directories containing header files other than /usr/include
#
INCLUDES = -I./include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like: -L../lib
LFLAGS = -lboost_thread -pthread -lboost_chrono -Wl,-rpath=$(PWD) 

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname
#   option, something like (this will link in libmylib.so and libm.so: -lm
LIBS =	$(PWD)/lib/libprojects$(VERSION).so
# define the C source files
SRC_DIR := src
OBJS_DIR := obj$(VERSION)

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRC = test/test_$(FILE_NAME).cpp

# define the CPP object files

OBJS = $(addprefix $(OBJS_DIR)/,$(notdir $(SRCS:.cpp=$(VERSION).o)))
TEST_OBJ = 	$(addprefix $(OBJS_DIR)/,$(notdir $(TEST_SRC:.cpp=.o)))

# define the executable file
MAIN = bin/$(FILE_NAME)$(VERSION).out

# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all: $(MAIN)
	@echo  Simple compiler named $(FILE_NAME)$(VERSION) has been compiled

$(MAIN): $(OBJS) $(LIBS) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $(MAIN) $(TEST_OBJ) $(LIBS) $(INCLUDES) $(LFLAGS)

$(OBJS_DIR)/%$(VERSION).o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^ -fPIC


$(LIBS): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -shared $(OBJS) $(LFLAGS)

$(TEST_OBJ): $(TEST_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^ -fPIC


# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
#.c.o:
#	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM)  $(wildcard obj*/*.o)
	$(RM)  $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
