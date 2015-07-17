#ifndef __QUEUE_INTEL_H__
#define __QUEUE_INTEL_H__

#define TMC_QUEUE_NO_FLAGS          (0)
#define TMC_QUEUE_SINGLE_SENDER     (1 << 0)
#define TMC_QUEUE_SINGLE_RECEIVER   (1 << 1)

#define INTEL_QUEUE(NAME, OBJ_TYPE, LOG2_ENTRIES, FLAGS)                                            \
typedef struct                                                                                      \
{                                                                                                   \
    uint64_t enqueue_count;                                                                             \
    uint64_t dequeue_count;                                                                             \
    OBJ_TYPE* node;                                                                                 \
    int item_size;                                                                                  \
    unsigned int total;                                                                             \
    unsigned int length;                                                                            \
    pthread_mutex_t enqueue_mutex;                                                                  \
    pthread_mutex_t dequeue_mutex;                                                                  \
    pthread_cond_t empty;                                                                           \
    pthread_cond_t full;                                                                            \
}NAME##_t;                                                                                          \
static inline NAME##_t * NAME##_init(NAME##_t * queue)                                              \
{                                                                                                   \
    exit_if_ptr_is_null(queue,"alloc queue error");                                                 \
    unsigned int numbers = (1 << LOG2_ENTRIES);                                                     \
    queue->node = malloc(numbers * sizeof(void *));                                                 \
    exit_if_ptr_is_null(queue->node,"alloc queue node error");                                      \
    bzero(queue->node,numbers);                                                                     \
    queue->item_size = sizeof(void *);                                                              \
    queue->total = numbers;                                                                         \
    queue->length = 0;                                                                              \
    queue->dequeue_count = 0;                                                                       \
    queue->enqueue_count = 0;                                                                       \
    pthread_mutex_init(&queue->enqueue_mutex,NULL);                                                 \
    pthread_mutex_init(&queue->dequeue_mutex,NULL);                                                 \
    pthread_cond_init(&queue->empty,NULL);                                                          \
    return queue;                                                                                   \
}                                                                                                   \
static inline int NAME##_enqueue(NAME##_t * queue,OBJ_TYPE data)                                    \
{                                                                                                   \
    pthread_mutex_lock(&queue->enqueue_mutex);                                                      \
    if(queue->enqueue_count - queue->dequeue_count >= (1 << LOG2_ENTRIES))                          \
    {                                                                                               \
        pthread_mutex_unlock(&queue->enqueue_mutex);                                            \
        return -1;                                                                                  \
    }                                                                                               \
    unsigned int i = queue->enqueue_count & ((1 << LOG2_ENTRIES) - 1);                              \
    queue->node[i] = data;                                                                          \
    ++queue->enqueue_count;                                                                         \
    ++queue->length;                                                                                \
    pthread_cond_signal(&queue->empty);                                                             \
        pthread_mutex_unlock(&queue->enqueue_mutex);                                                \
    return 0;                                                                                       \
}                                                                                                   \
static inline int NAME##_enqueue_multiple(NAME##_t * queue,OBJ_TYPE* data,unsigned int enqueue_num) \
{                                                                                                   \
    pthread_mutex_lock(&queue->enqueue_mutex);                                                      \
    if(queue->enqueue_count + enqueue_num - queue->dequeue_count > (1 << LOG2_ENTRIES))            \
    {                                                                                               \
        pthread_mutex_unlock(&queue->enqueue_mutex);                                                \
        return -1;                                                                                  \
    }                                                                                               \
    unsigned int enqueue_count = queue->enqueue_count;                                              \
    while(enqueue_num--)                                                                            \
    {                                                                                               \
        unsigned int i = enqueue_count & ((1 << LOG2_ENTRIES) - 1);                                 \
        queue->node[i] = *(data++);                                                                 \
        ++queue->length;                                                                            \
        ++enqueue_count;                                                                            \
    }                                                                                               \
    queue->enqueue_count = enqueue_count;                                                           \
    pthread_cond_signal(&queue->empty);                                                             \
    pthread_mutex_unlock(&queue->enqueue_mutex);                                                    \
    return 0;                                                                                       \
}                                                                                                   \
/*                                                                                                  \
*  本函数不用is_empty_queue，因为防止锁中锁                                                         \
* */                                                                                                \
static inline int NAME##_dequeue(NAME##_t * queue,OBJ_TYPE* data)                                   \
{                                                                                                   \
    int result = -1;                                                                                \
    pthread_mutex_lock(&queue->dequeue_mutex);                                                      \
    if(queue->enqueue_count == queue->dequeue_count)                                                \
    {                                                                                               \
        result =  -1;                                                                               \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        unsigned int i = queue->dequeue_count & ((1 << LOG2_ENTRIES) - 1);                          \
        *data = queue->node[i];                                                                     \
        ++queue->dequeue_count;                                                                     \
        --queue->length;                                                                            \
        result = 0;                                                                                 \
    }                                                                                               \
    pthread_mutex_unlock(&queue->dequeue_mutex);                                                    \
    return result;                                                                                  \
}                                                                                                   \
                                                                                                    \
static inline int NAME##_dequeue_multiple(NAME##_t * queue,void ** data,unsigned int dequeue_num)   \
{                                                                                                   \
    pthread_mutex_lock(&queue->dequeue_mutex);                                                      \
    if(queue->dequeue_count + dequeue_num  > queue->enqueue_count)                                  \
    {                                                                                               \
        pthread_mutex_unlock(&queue->dequeue_mutex);                                                \
        return -1;                                                                                  \
    }                                                                                               \
    unsigned int dequeue_count = queue->dequeue_count;                                              \
    while(dequeue_num--)                                                                            \
    {                                                                                               \
        unsigned int i = dequeue_count & ((1 << LOG2_ENTRIES) - 1);                          \
        *(data++) = queue->node[i];                                                                 \
        --queue -> length;                                                                          \
        dequeue_count++;                                                                            \
    }                                                                                               \
    queue->dequeue_count = dequeue_count;                                                           \
    pthread_mutex_unlock(&queue->dequeue_mutex);                                                    \
    return 0;                                                                                       \
}

#endif
