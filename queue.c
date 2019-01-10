#include "queue.h"

void init_queue(Queue *q) {
	q->begin = q->end = 0;
}

int queue_is_empty(Queue *q) {
	return q->begin == q->end;
}

void queue_put(Queue *q, char* c) {
	q->buffer[q->end] = c[0];
	q->end =(q->end + 1) % QUEUE_SIZE;
}

uint32_t queue_get(Queue *q, char *buffer, uint32_t size) {
	uint32_t len = 0;
	while(len < size && q->begin != q->end ) {
		buffer[len] = q->buffer[q->begin];
		len++;
		q->begin = (q->begin + 1) % QUEUE_SIZE;
	}
	return len;
}