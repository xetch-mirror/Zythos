#ifndef SYS_TYPES_H
#define SYS_TYPES_H

typedef unsigned char  uint8_t;
typedef signed char    int8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int   uintptr_t;
typedef int            task_id_t;

#define INVALID_TASK_ID -1

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DEAD
} task_state_t;

typedef struct {
    task_id_t id;
    task_state_t state;
    uint32_t priority;
} task_info_t;

#endif