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
#define HAL_MSGQ_MAGIC_VAL (0xa55aa55a)

/*! @brief  States for a stored message queue item */
typedef enum
{
    MSGQ_ITEM_FREE = 0, /*!< Stored item is attached to the free items list */
    MSGQ_ITEM_BUSY      /*!< Stored item is attached to the busy items list */

} msgq_item_state;

/*! @brief The message queue storage descriptor */
typedef struct _msgq_storage_t
{
    msgq_buf *busy;         /*!< List of busy (in use) elements */
    msgq_buf *free;         /*!< List of free (available) elements */
    msgq_buf *elmArray;     /*!< Array of message queue items */
    void *    p_storage;    /*!< Pointer to memory region for storing the elements */
    uint32_t  elem_size;    /*!< Size of a single element in the queue */
    uint32_t  tot_elements; /*!< Total number of elements in the queue */
    uint32_t  magic;        /*!< Memory protection marker */
} msgq_storage;

/**
 * @brief Requests a data pointer from the queue, moving the item to the busy list.
 * @param msgq_handle Handle to the storage instance.
 * @param data Pointer to data that should be copied to the storage, can be NULL.
 * @param size Size in bytes of 'data'.
 * @param reset If true, the storage buffer will be set to zeros before copying.
 * @retval Pointer to a message queue item or NULL on error.
 */

msgq_buf *msgq_request(uintptr_t msgq_handle, void *data, uint32_t size, bool reset)
{
    msgq_buf *    p_buf = NULL;
    msgq_storage *pfs   = (msgq_storage *) msgq_handle; /* Handle to pointer */

#if ( HAL_MSGQ_SANITY_CHECKS == 1 )
    if ( pfs == NULL || pfs->magic != HAL_MSGQ_MAGIC_VAL )
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

    p_buf->next  = NULL;
    p_buf->prev  = NULL;
    p_buf->type  = MSGQ_ITEM_BUSY; /* Mark as busy */
    p_buf->state = 0;              /* Initial state */

    /* Copy data to the item's buffer if applicable */
    if ( data && size && size <= pfs->elem_size )
    {
        if ( reset )
            hal_zero_buf(p_buf->p_data, pfs->elem_size);

        hal_memcpy(p_buf->p_data, data, size);
    }

    HAL_MSGQ_ENTER_CRITICAL();

    /* Attach to the busy list */
    DL_APPEND(pfs->busy, p_buf);

    HAL_MSGQ_EXIT_CRITICAL();

    return p_buf;
}

/**
 * @brief Releases an element back to the free elements container.
 * @param msgq_handle Handle to the storage instance.
 * @param p_buf Pointer to the element that should be released.
 * @retval 0 on success, 1 on error.
 */

int msgq_release(uintptr_t msgq_handle, msgq_buf *p_buf)
{
    msgq_storage *pfs = (msgq_storage *) msgq_handle; /* Handle to pointer */

#if ( HAL_MSGQ_SANITY_CHECKS == 1 )
    if ( pfs == NULL || pfs->magic != HAL_MSGQ_MAGIC_VAL || p_buf == NULL || p_buf->type == MSGQ_ITEM_FREE )
    {
        return 1; /* Error releasing item to storage */
    }
#endif

    HAL_MSGQ_ENTER_CRITICAL();

    /* Detach from the busy list */
    DL_DELETE(pfs->busy, p_buf);

    p_buf->next = NULL;
    p_buf->prev = NULL;
    p_buf->type = MSGQ_ITEM_FREE;

    /* Attach to the free list */
    DL_APPEND(pfs->free, p_buf);

    HAL_MSGQ_EXIT_CRITICAL();

    return 0; /* Success */
}

/**
 * @brief Constructs a message queue storage instance.
 * @param elem_size Size in bytes of a single stored element.
 * @param tot_elements Maximum number of elements to store.
 * @retval Handle to the message queue (cast to uintptr_t) or 0 on error.
 */

uintptr_t msgq_create(uint32_t elem_size, uint32_t tot_elements)
{
    uint32_t      elm         = 0;
    size_t        allocSize   = 0;
    uint8_t *     pStoragePtr = NULL;
    msgq_storage *pfs         = NULL;

    if ( elem_size == 0 || tot_elements == 0 )
        return 0;

    allocSize = sizeof(msgq_storage) + (elem_size * tot_elements);

    /* Allocate space for the storage descriptor and the element storage */
    pfs = (msgq_storage *) hal_alloc(allocSize);
    if ( pfs == NULL )
        return 0;

    pStoragePtr = (uint8_t *) pfs + sizeof(msgq_storage);

    pfs->magic        = 0;
    pfs->elem_size    = elem_size;
    pfs->tot_elements = tot_elements;
    pfs->p_storage    = (void *) pStoragePtr;
    pfs->busy         = NULL;
    pfs->free         = NULL;

    pfs->elmArray = (msgq_buf *) hal_alloc(sizeof(msgq_buf) * tot_elements);
    if ( pfs->elmArray == NULL )
    {
        return 0; /* Memory error */
    }

    /* Initialize free items list */
    while ( elm < tot_elements )
    {
        pfs->elmArray[elm].p_data = (uint8_t *) pfs->p_storage + (elm * elem_size);
        pfs->elmArray[elm].type   = MSGQ_ITEM_FREE;
        pfs->elmArray[elm].next   = NULL;
        pfs->elmArray[elm].prev   = NULL;

        DL_APPEND(pfs->free, &pfs->elmArray[elm]);

        elm++;
    }

    pfs->magic = HAL_MSGQ_MAGIC_VAL; /* Set only when fully initialized */
    return (uintptr_t) pfs;
}
