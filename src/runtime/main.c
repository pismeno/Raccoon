#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

typedef struct {
  void* payload;
} RaccoonHandle;

#define MAX_OBJECTS 1024
static RaccoonHandle* gc_tracked_objects[MAX_OBJECTS];
static size_t object_count = 0;

RaccoonHandle* raccoon_alloc(uint64_t size) {
  void* actual_data = malloc(size);
  if (!actual_data) {
    fprintf(stderr, "Runtime Error: Out of memory for payload.\n");
    exit(1);
  }

  RaccoonHandle* handle = (RaccoonHandle*)malloc(sizeof(RaccoonHandle));
  if (!handle) {
    fprintf(stderr, "Runtime Error: Out of memory for handle.\n");
    free(actual_data);
    exit(1);
  }

  handle->payload = actual_data;

  if (object_count < MAX_OBJECTS) {
    gc_tracked_objects[object_count++] = handle;
  } else {
    fprintf(stderr, "Runtime Warning: GC tracking limit reached!\n");
  }

  printf("allocating new object at %p (handle %p)\n", actual_data, handle);

  return handle;
}

void raccoon_gc_collect() {
  printf("[GC] Initiating sweep pass... Tracked objects: %zu\n", object_count);
}