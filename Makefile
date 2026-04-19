CC     = gcc
CFLAGS = -std=c11 -pthread -Wall -Wextra -g

SRCS = main.c common.c logger.c \
		factory_f1.c factory_f2.c factory_f3.c \
		transport_ship.c transport_trucks.c watchdog.c

OBJS = $(SRCS:.c=.o)

all: factory_sim

factory_sim: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
#                     ^    ^^^
#              source file  output file
#              (was missing)

clean:
	rm -f $(OBJS) factory_sim