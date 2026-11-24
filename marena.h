#ifndef MARENA_H_INCLUDED
#define MARENA_H_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct marena_chunk marena_chunk_t;

typedef struct {
	marena_chunk_t *chunk_used;
	marena_chunk_t *chunk_active;
	size_t chunk_size;
	size_t chunk_count;
} marena_t;

int marena_init(marena_t *arena, size_t block_size);
void *marena_alloc(marena_t *arena, size_t size);
int marena_reset(marena_t *arena);
int marena_destroy(marena_t *arena);

#ifdef __cplusplus
}
#endif

#endif // MARENA_H_INCLUDED 

