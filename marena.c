#include "marena.h"

#include <stddef.h>

#include <stdlib.h>
#define MARENA_MALLOC(sz) malloc(sz)
#define MARENA_FREE(pt) free(pt)

#define MARENA_MIN_FREE_THRESHOLD 8

struct marena_chunk {
	marena_chunk_t *next_used;
	marena_chunk_t *next_active;
	size_t free_bytes;
};

static marena_chunk_t *marena_create_chunk(size_t size, marena_chunk_t *active, marena_chunk_t *used) {
	marena_chunk_t *new_chunk = MARENA_MALLOC(size + sizeof(marena_chunk_t));
	if (new_chunk) {
		new_chunk->free_bytes = size;
		new_chunk->next_active = active;
		new_chunk->next_used = used; 
		return new_chunk;
	}
	return NULL;
}

int marena_init(marena_t *arena, size_t chunk_size) {
	if (arena && chunk_size) {
		marena_chunk_t *new_chunk = marena_create_chunk(chunk_size, NULL, NULL);
		if (new_chunk != NULL) {
			arena->chunk_size = chunk_size;
			arena->chunk_count = 1;
			arena->chunk_used = new_chunk;
			arena->chunk_active = new_chunk;
			/* return NOERROR=0 */
			return 0;
		}
	}
	/* return ERROR=1 */
	return 1;
}
	
void *marena_alloc(marena_t *arena, size_t size) {
	if (arena && size && size <= arena->chunk_size) {
		marena_chunk_t **active = &arena->chunk_active;
		while (*active) {
			marena_chunk_t *a = *active;
			if (size <= a->free_bytes) {
				void *value = (void *)((char *)a + arena->chunk_size - a->free_bytes); 
				a->free_bytes -= size;
				if (a->free_bytes < MARENA_MIN_FREE_THRESHOLD) {
					*active = a->next_active;
				}
				return value;
			}
			if (a->next_active) {
				active = &a->next_active;
			} else {
				a = marena_create_chunk (arena->chunk_size, arena->chunk_active, arena->chunk_used);
				if (a) {
					arena->chunk_active = a;
					arena->chunk_used = a;
					arena->chunk_count++;
					active = &arena->chunk_active;
				} else break;
			}
		}
	}
	return NULL;
}
	
int marena_reset(marena_t *arena) {
	if (arena && arena->chunk_count) {
		size_t tofree = arena->chunk_count/2;
		marena_chunk_t *current = arena->chunk_used;
		arena->chunk_count -= tofree;
		while (current) {
			marena_chunk_t *next = current->next_used;
			if (tofree) {
				MARENA_FREE(current);
				tofree--;
				if (!tofree && next) {
					next->next_active = NULL;
					next->next_used = NULL;
				}
			} else {
				current->free_bytes = arena->chunk_size;
				current->next_active = current->next_used;
			}
			current = next; 
		}
	}
	return 1;
}

int marena_destroy(marena_t *arena) {
	if (arena && arena->chunk_count) {
		marena_chunk_t *current = arena->chunk_used;
		while (current) {
			marena_chunk_t *next = current->next_used;
			MARENA_FREE(current);
			current = next;
		}
		arena->chunk_active = NULL;
		arena->chunk_used = NULL;
		arena->chunk_count = 0;
		return 0;
	}
	return 1;
}
