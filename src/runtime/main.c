#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
  void* payload;
  bool marked;
} RaccoonHandle;

#define MAX_OBJECTS 1024
static RaccoonHandle* gc_tracked_objects[MAX_OBJECTS];
static size_t object_count = 0;

#define SHADOW_STACK_MAX 2048
static RaccoonHandle** shadow_stack[SHADOW_STACK_MAX];
static size_t shadow_stack_top = 0;

void mark_object(RaccoonHandle* handle) {
  if (handle == NULL || handle->marked) return;
  handle->marked = true;
}

void raccoon_gc_collect() {
  for (size_t i = 0; i < shadow_stack_top; i++) {
    RaccoonHandle* live_handle = *(shadow_stack[i]);
    if (live_handle != NULL) {
      mark_object(live_handle);
    }
  }

  size_t new_object_count = 0;
  for (size_t i = 0; i < object_count; i++) {
    RaccoonHandle* handle = gc_tracked_objects[i];
    if (handle->marked) {
      handle->marked = false;
      gc_tracked_objects[new_object_count++] = handle;
    } else {
      free(handle->payload);
      free(handle);
    }
  }
  object_count = new_object_count;
}

void raccoon_push_root(RaccoonHandle** root_address) {
  if (shadow_stack_top < SHADOW_STACK_MAX) {
    shadow_stack[shadow_stack_top++] = root_address;
  } else {
    fprintf(stderr, "Runtime Error: Shadow stack overflow!\n");
    exit(1);
  }
}

void raccoon_pop_roots(size_t count) {
  if (shadow_stack_top >= count) {
    shadow_stack_top -= count;
  } else {
    fprintf(stderr, "Runtime Error: Shadow stack underflow!\n");
    exit(1);
  }
}


RaccoonHandle* raccoon_alloc(uint64_t size) {
  if (object_count >= MAX_OBJECTS) {
    raccoon_gc_collect();

    fprintf(stderr, "Runtime Error: Out of Memory! (GC could not free enough space)\n");
    exit(1);
  }

  void* actual_data = malloc(size);
  if (actual_data == NULL) {
    fprintf(stderr, "Runtime Error: System out of memory for payload!\n");
    exit(1);
  }

  RaccoonHandle* handle = (RaccoonHandle*)malloc(sizeof(RaccoonHandle));
  if (handle == NULL) {
    free(actual_data); // Clean up the payload we just allocated
    fprintf(stderr, "Runtime Error: System out of memory for handle!\n");
    exit(1);
  }

  handle->payload = actual_data;
  handle->marked = false;

  gc_tracked_objects[object_count++] = handle;

  return handle;
}