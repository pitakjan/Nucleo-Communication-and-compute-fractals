/*
 * Queue
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


/* Queue structure which holds all necessary data */
typedef struct {
    void** Arr; //pointer on queue
    int Allocated;/*size of allocated memory (in elements),
                        *it means how many elements could be stored */
    int Size; //numbers of elements in the queue
    int First; // pointer on the First element of the queue
    int Last; // pointer on the Last element of the queue
} queue_t;

#define MIN_SIZE_QUEUE 10 /* if allocated size is under this value,
the queue will not shrink*/

#define MEMORY_ERROR NULL //return statement on memory error

#define EXPAND_QUEUE 2 //multiplier, when makes queue bigger due to max size

/* creates a new queue with a given size
 * returns: NULL(MEMORY_ERROR) on memory error,
                Else pointer on the structure  */
queue_t* create_queue(int capacity)
{
    //allocation structure
    queue_t* Queue = (queue_t*) malloc(sizeof(queue_t));
    if (Queue == NULL) {
        fprintf(stderr,"ERROR: Cannot Allocated Memory\n");
        return MEMORY_ERROR;
    }
    //allocation memory for the elements in queue
    Queue->Arr = (void**) malloc(capacity * sizeof(void*));
    if (Queue->Arr == NULL) {
        fprintf(stderr,"ERROR: Cannot Allocated Memory\n");
        free(Queue);
        return MEMORY_ERROR;
    }

    //Initialization
    Queue->Allocated = capacity;
    Queue->First = 0;
    Queue->Last = 0;
    Queue->Size = 0;

    return Queue;
}

/* deletes the queue and allocated memory,
 * If some elements remained in the queue, that elements will NOT be freed */
void delete_queue(queue_t *Array)
{
    free(Array->Arr);
    free(Array);
    return;
}

/* Inserts a reference to the element into the queue
 * returns: true on success, NULL(MEMORY_ERROR) on memory error */
bool push_to_queue(queue_t *queue, void *data)
{
    //If queue is full, it will be expanded
    if ((queue->Size) == (queue->Allocated)) {
        void** tmp = (void**) realloc(queue->Arr,(queue->Allocated)*EXPAND_QUEUE*sizeof(void*));
        if (tmp == NULL) {
            delete_queue(queue);
            return MEMORY_ERROR;
        }
        (queue->Arr) = tmp;
        //If elements goes over the end of the queue
        if ((queue->Last) < (queue->First)) {
            //Elements at the old end will be moved to the new end of the queue
            for (int i = (queue->First); i < (queue->Allocated); i++) {
                (queue->Arr)[i + ((queue->Allocated)*(EXPAND_QUEUE-1))] = (queue->Arr)[i];
            }
            (queue->First) = (queue->First) + ((queue->Allocated)*(EXPAND_QUEUE-1));
        }
        (queue->Allocated) = (queue->Allocated) * EXPAND_QUEUE;
    }
    //Move a pointer on the end of the queue correctly
    if (queue->Size != 0) {
        if ((queue->Last) == ((queue->Allocated)-1)) {
            (queue->Last) = 0;
        } else {
            (queue->Last)++;
        }
    }
    //Add element
    queue->Arr[queue->Last] = data;
    (queue->Size)++;
    return true;
}

/* Gets the first element from the queue and removes it from the queue
 * returns: the first element on success; NULL otherwise */
char pop_from_queue(queue_t *queue)
{
    if (queue->Size == 0 || queue->Arr == NULL) {
        return 100;
    }
    //remember Return value
    void* Return = queue->Arr[queue->First];
    //Move a pointer on the Start of the queue correctly
    if ((queue->First) == ((queue->Allocated)-1)) {
        (queue->First) = 0;
    } else {
        (queue->First)++;
    }
    (queue->Size)--;
    //move pointers for better efficiency if queue is free
    if (queue->Size == 0) {
        queue->First = 0;
        queue->Last = 0;
    }
    //if queue is filled only by 1/3, it will be shrink
    /*if ((queue->Size) < ((queue->Allocated)*(1.0/3)) && (queue->Allocated) > MIN_SIZE_QUEUE) {
        if ((queue->First) < (queue->Last)) {
            //if all elements are in one row
            for (int i = 0; i < (queue->Size); i++) {
                (queue->Arr)[i] = (queue->Arr)[(queue->First) + i];
            }
            queue->First = 0;
            queue->Last = (queue->Size) - 1;
        } else {
            // if elements goes over the end of the queue
            for (int i = 0; i < (queue->Allocated - queue->First); i++) {
                (queue->Arr)[queue->Last + i + 1] = (queue->Arr)[queue->First + i];
            }
            queue->First = (queue->Last) + 1;
        }
        //shrink queue
        void** tmp = (void**) realloc(queue->Arr,(queue->Size)*sizeof(void*));
        if (tmp == NULL) {
            for (int i = 0; i < 1100000; i++) {
                led = !led;
                wait_ms(50);
            }
            delete_queue(queue);
            return MEMORY_ERROR;
        }
        (queue->Arr) = tmp;
        (queue->Allocated) = (queue->Size);
    }*/
    return *(char*) Return;
}

/* gets idx-th element from the queue,
 * returns: the idx-th element on success; NULL otherwise */
void* get_from_queue(queue_t *queue, int idx)
{
    if (queue->Size == 0 || queue->Size <= idx || idx < 0) {
        return NULL;
    }

    if ((queue->First) < (queue->Last)) {
        //if all elements are in one row
        return (queue->Arr)[idx+(queue->First)];
    } else {
        // if elements goes over the end of the queue
        if (((queue->Allocated)-(queue->First)) > idx) {
            //if idx element is before end
            return (queue->Arr)[idx+(queue->First)];
        } else {
            //if idx element is after start
            idx = idx - ((queue->Allocated) - (queue->First));
            return (queue->Arr[idx]);
        }
    }
}

/* gets number of stored elements */
int get_queue_size(queue_t *queue)
{
    return (queue->Size);
}






