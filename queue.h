#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

#define QUEUE_SIZE 1024

typedef struct q {
	char buffer[QUEUE_SIZE];
	uint32_t begin;
	uint32_t end;
} Queue;

void init_queue(Queue *q);
int queue_is_empty(Queue *q);
void queue_put(Queue *q, char* c);
uint32_t queue_get(Queue *q, char *buf, uint32_t size);

#endif