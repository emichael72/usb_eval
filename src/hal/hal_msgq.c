/**
  ******************************************************************************
  * @file    hal_msgq.c
  * @author  IMCv2 Team
  * @brief   Implementation of a message queue based on a free/busy list.
  * 
  * This module provides a message queue implementation where elements 
  * are managed using a free - busy list. It includes functions for 
  * requesting and releasing queue elements, as well as initializing the queue 
  * storage.
  * 
  ******************************************************************************
  * @attention
  * 
  * @copyright
  * @par Copyright (c) 2024 Intel Corporation.
  * All rights reserved.
  * 
  * This code is proprietary to Intel Corporation and may not be used, modified,
  * or distributed without the express written permission of Intel Corporation.
  * 
  ******************************************************************************
  */

#include <hal.h>
#include <hal_msgq.h>
#include <hal_llist.h>

/* Use XOS interfcae to impliment critical section using 
   brief interrupts disabling.
*/

#if ( HAL_MSGQ_USE_CRITICAL > 0 )

/* Define a static variable to hold the old interrupt level */
static uint32_t msgq_old_int_level;

#define HAL_MSGQ_ENTER_CRITICAL()                      \
    do                                                 \
    {                                                  \
        msgq_old_int_level = xos_disable_interrupts(); \
    } while ( 0 )

#define HAL_MSGQ_EXIT_CRITICAL()                    \
    do                                              \
    {                                               \
        xos_restore_interrupts(msgq_old_int_level); \
    } while ( 0 )

#else
#define HAL_MSGQ_ENTER_CRITICAL() /* No-op */
#define HAL_MSGQ_EXIT_CRITICAL()  /* No-op */
#endif

/*! Handle validity protection using a known marker. */
#define HAL_MSGQ_MAGIC_VAL      (0xa55aa55a)
#define HAL_MSGQ_MINI_MAGIC_VAL (0xa55a)

typedef struct
{
    uint16_t marker   : 16; /* Memory marker */
    uint8_t  status   : 1;  /* busy or free */
    uint16_t reserved : 15; /* reserved */
} buf_bits;

/*! @brief The message queue item structure */
typedef struct __attribute__((packed)) __msgq_buf_t
{
    struct __msgq_buf_t *next, *prev; /*!< List next and previous pointers */
    buf_bits             bits;        /*!< Vasrious flags */
    uint8_t              data[0];     /*!< Payload */
} msgq_buf;

/*! @brief The message queue storage descriptor */
typedef struct _msgq_storage_t
{
    msgq_buf *busy;          /*!< List of busy (in use) elements */
    msgq_buf *free;          /*!< List of free (available) elements */
    msgq_buf *last_accessed; /*!< Pointer to the last accessed message buffer. */
    uint16_t  item_size;     /*!< Size of a single element in the queue */
    uint16_t  items_count;   /*!< Total number of elements in the queue */
    uint32_t  magic;         /*!< Memory protection marker */

} msgq_storage;

/**
 * Retrieves the next element from the specified message queue list, either busy or free,
 * and optionally switches its state depending on the 'readonly' flag.
 * 
 * @param msgq_handle Handle to the message queue storage.
 * @param list_type Specifies the list to traverse: 0 for free list, 1 for busy list.
 * @param order Set to true for natural order (next), or false for reversed order (prev).
 * @param readonly If true, the function does not alter the list or item statuses; it only reads data.
 * @return Pointer to the next message buffer data or NULL if there are no more elements or an error occurs.
 */

void *msgq_get_next(uintptr_t msgq_handle, int list_type, bool order)
{
    msgq_storage *storage = (msgq_storage *) msgq_handle;
    if ( ! storage || storage->magic != HAL_MSGQ_MAGIC_VAL )
    {
        return NULL;
    }

    msgq_buf *current  = NULL;
    msgq_buf *selected = (list_type == 0) ? storage->free : storage->busy;

    if ( storage->last_accessed && ((storage->last_accessed->bits.status && list_type == 1) || (! storage->last_accessed->bits.status && list_type == 0)) )
    {
        current = order ? storage->last_accessed->next : storage->last_accessed->prev;
    }

    if ( ! current )
    {
        current = selected;
    }

    if ( ! current )
    {
        return NULL;
    }

    storage->last_accessed = current;

    return current->data;
}

/**
 * @brief Requests a data pointer from the queue, moving the item to the busy list.
 * @param msgq_handle Handle to the storage instance.
 * @param size Size in bytes of 'data', coiuld be 0 sinse it's a preallocated fixed size pool.
 * @retval Pointer biffer or NULL on error.
 */

void *msgq_request(uintptr_t msgq_handle, size_t size)
{
    msgq_buf     *p_buf = NULL;
    msgq_storage *pfs   = (msgq_storage *) msgq_handle; /* Handle to pointer */

#if ( HAL_MSGQ_SANITY_CHECKS == 1 )

    if ( pfs == NULL || pfs->magic != HAL_MSGQ_MAGIC_VAL | pfs->free == NULL )
        return NULL;

    /* If we got size, use it for vliadation */
    if ( size && pfs->item_size < size )
        return NULL;

#endif

    HAL_MSGQ_ENTER_CRITICAL();
    do
    {
        p_buf = pfs->free; /* Point to the free list head */

        if ( p_buf == NULL )
            break;

        /* Detach from the free storage list */
        DL_DELETE(pfs->free, p_buf);

    } while ( 0 );

    HAL_MSGQ_EXIT_CRITICAL();

    if ( p_buf == NULL )
        return NULL;

    p_buf->next        = NULL;
    p_buf->prev        = NULL;
    p_buf->bits.status = 1; /* Mark as busy */

    HAL_MSGQ_ENTER_CRITICAL();

    /* Attach to the busy list */
    DL_APPEND(pfs->busy, p_buf);

    HAL_MSGQ_EXIT_CRITICAL();

    return (void *) p_buf->data;
}

/**
 * @brief Releases an element back to the free elements container.
 * @param msgq_handle Handle to the storage instance.
 * @param data Pointer to the element that should be released.
 * @retval 0 on success, 1 on error.
 */

int msgq_release(uintptr_t msgq_handle, void *data)
{
    msgq_buf     *p_buf = NULL;
    msgq_storage *pfs   = (msgq_storage *) msgq_handle; /* Handle to pointer */

    /* Calculate the offset of p_data within msgq_buf */
    size_t offset = offsetof(msgq_buf, data);

    /* Subtract the offset from the data_ptr to get the msgq_buf pointer */
    p_buf = (msgq_buf *) ((uint8_t *) data - offset);

#if ( HAL_MSGQ_SANITY_CHECKS == 1 )
    if ( pfs == NULL || pfs->magic != HAL_MSGQ_MAGIC_VAL || p_buf == NULL || p_buf->bits.marker != HAL_MSGQ_MINI_MAGIC_VAL )
    {
        return 1; /* Error releasing item to storage */
    }
#endif

    HAL_MSGQ_ENTER_CRITICAL();

    /* Detach from the busy list */
    DL_DELETE(pfs->busy, p_buf);

    p_buf->next        = NULL;
    p_buf->prev        = NULL;
    p_buf->bits.status = 0;

    /* Attach to the free list */
    DL_APPEND(pfs->free, p_buf);

    HAL_MSGQ_EXIT_CRITICAL();

    return 0; /* Success */
}

/**
 * @brief Constructs a message queue storage instance.
 * @param item_size Size in bytes of a single stored element.
 * @param items_count Maximum number of elements to store.
 * @retval Handle to the message queue (cast to uintptr_t) or 0 on error.
 */

uintptr_t msgq_create(size_t item_size, size_t items_count)
{

    size_t        q_item_size = 0;
    msgq_buf     *p_buf       = NULL;
    msgq_storage *p_storage   = NULL;

    if ( item_size == 0 || items_count == 0 )
        return 0;

    /* Allocate space for the container */
    p_storage = (msgq_storage *) hal_alloc(sizeof(msgq_storage));
    assert(p_storage != NULL);

    /* Place pointers */
    p_storage->magic       = 0;
    p_storage->item_size   = item_size;
    p_storage->items_count = items_count;
    p_storage->busy        = NULL;
    p_storage->free        = NULL;

    /* A single node size in bytes */
    q_item_size = sizeof(msgq_buf) + item_size;

    /* Allocater notes and attch them to the free list */
    for ( int i = 0; i < items_count; i++ )
    {
        p_buf = (msgq_buf *) hal_alloc(q_item_size);
        assert(p_buf != NULL);

        p_buf->bits.status = 0; /* Free */
        p_buf->bits.marker = HAL_MSGQ_MINI_MAGIC_VAL;
        p_buf->next        = NULL;
        p_buf->prev        = NULL;

        DL_APPEND(p_storage->free, p_buf);
    }

    /* Set only when fully initialized */
    p_storage->magic = HAL_MSGQ_MAGIC_VAL;

    return (uintptr_t) p_storage;
}
