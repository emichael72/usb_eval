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
#include <hal_llist.h>

#if (HAL_MSGQ_USE_CRITICAL > 0)


/* Define a static variable to hold the old interrupt level */
static uint32_t msgq_old_int_level;

#define HAL_MSGQ_ENTER_CRITICAL()  \
    do { \
        msgq_old_int_level = xos_disable_interrupts(); \
    } while(0)

#define HAL_MSGQ_EXIT_CRITICAL()  \
    do { \
        xos_restore_interrupts(msgq_old_int_level); \
    } while(0)

#else
    #define HAL_MSGQ_ENTER_CRITICAL()   /* No-op */
    #define HAL_MSGQ_EXIT_CRITICAL()    /* No-op */
#endif

/*! Handle validity protection using a known marker. */
#define HAL_MSGQ_MAGIC_VAL (0xa55aa55a)

/*! @brief  States for a stored message queue item */
typedef union {
    enum {
        MSGQ_ITEM_FREE = 0, /*!< Stored item is attached to the free items list */
        MSGQ_ITEM_BUSY      /*!< Stored item is attached to the busy items list */
    } value;
    int32_t force32BitSize;  /*!< Ensures the enum occupies 32 bits */
} msgq_item_state;

/*! @brief The message queue item structure */
typedef struct __msgq_items_t
{
    struct __msgq_items_t *next, *prev; /*!< List next and previous pointers */
    uint8_t *          p_data;          /*!< Pointer to the actual memory space containing the message */
    int                state;           /*!< State of the item, application-defined */
    msgq_item_state    type;            /*!< Type of stored element (free or busy) */

} msgq_items;

/*! @brief The message queue storage descriptor */
typedef struct _msgq_storage_t
{
    msgq_items *       busy;         /*!< List of busy (in use) elements */
    msgq_items *       free;         /*!< List of free (available) elements */
    msgq_items *       elmArray;     /*!< Array of message queue items */
    void *             p_storage;    /*!< Pointer to memory region for storing the elements */
    uint32_t           elem_size;    /*!< Size of a single element in the queue */
    uint32_t           tot_elements; /*!< Total number of elements in the queue */
    uint32_t           magic;        /*!< Memory protection marker */
} msgq_storage;


/**
 * @brief Requests a data pointer from the queue, moving the item to the busy list.
 * @param msgq_handle Handle to the storage instance.
 * @param data Pointer to data that should be copied to the storage, can be NULL.
 * @param size Size in bytes of 'data'.
 * @param reset If true, the storage buffer will be set to zeros before copying.
 * @retval Pointer to a message queue item or NULL on error.
 */
msgq_items *msgq_request(uintptr_t msgq_handle, void *data, uint32_t size, bool reset)
{
    msgq_items *pdata = NULL;
    msgq_storage *pfs = (msgq_storage*)msgq_handle; /* Handle to pointer */

    if (pfs == NULL || pfs->magic != HAL_MSGQ_MAGIC_VAL)
        return NULL;

    HAL_MSGQ_ENTER_CRITICAL();
    do
    {
        pdata = pfs->free; /* Point to the free list head */

        if (pdata == NULL)
            break;

        /* Detach from the free storage list */
        DL_DELETE(pfs->free, pdata);

    } while (0);
    HAL_MSGQ_EXIT_CRITICAL();

    if (pdata == NULL)
        return NULL;

    pdata->next  = NULL;
    pdata->prev  = NULL;
    pdata->type.value  = MSGQ_ITEM_BUSY;    /* Mark as busy */
    pdata->state = 0;                       /* Initial state */

    /* Copy data to the item's buffer if applicable */
    if (data && size && size <= pfs->elem_size)
    {
        if (reset)
            hal_zero_buf(pdata->p_data, pfs->elem_size);

        hal_memcpy(pdata->p_data, data, size);
    }

    HAL_MSGQ_ENTER_CRITICAL();

    /* Attach to the busy list */
    DL_APPEND(pfs->busy, pdata);

    HAL_MSGQ_EXIT_CRITICAL();

    return pdata;
}

/**
 * @brief Releases an element back to the free elements container.
 * @param msgq_handle Handle to the storage instance.
 * @param pdata Pointer to the element that should be released.
 * @retval 0 on success, 1 on error.
 */

int msgq_release(uintptr_t msgq_handle, msgq_items *pdata)
{
    msgq_storage *pfs = (msgq_storage*)msgq_handle; /* Handle to pointer */

    if (pfs == NULL || pfs->magic != HAL_MSGQ_MAGIC_VAL || pdata == NULL || pdata->type.value == MSGQ_ITEM_FREE)
    {
        return 1; /* Error releasing item to storage */
    }

    HAL_MSGQ_ENTER_CRITICAL();

    /* Detach from the busy list */
    DL_DELETE(pfs->busy, pdata);

    pdata->next = NULL;
    pdata->prev = NULL;
    pdata->type.value = MSGQ_ITEM_FREE;

    /* Attach to the free list */
    DL_APPEND(pfs->free, pdata);

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
    uint32_t      elm = 0;
    size_t        allocSize = 0;
    uint8_t *     pStoragePtr = NULL;
    msgq_storage *pfs = NULL;

    if (elem_size == 0 || tot_elements == 0)
        return 0;

    allocSize = sizeof(msgq_storage) + (elem_size * tot_elements);

    /* Allocate space for the storage descriptor and the element storage */
    pfs = (msgq_storage *)hal_alloc(allocSize);
    if (pfs == NULL)
        return 0;

    hal_zero_buf(pfs, allocSize);

    pStoragePtr = (uint8_t *)pfs + sizeof(msgq_storage);

    pfs->magic        = 0;
    pfs->elem_size    = elem_size;
    pfs->tot_elements = tot_elements;
    pfs->p_storage    = (void *)pStoragePtr;
    pfs->busy         = NULL;
    pfs->free         = NULL;

    pfs->elmArray = (msgq_items *)hal_alloc(sizeof(msgq_items) * tot_elements);
    if (pfs->elmArray == NULL) {
        return 0; /* Memory error */
    }

    /* Initialize free items list */
    while (elm < tot_elements)
    {
        pfs->elmArray[elm].p_data = (uint8_t *)pfs->p_storage + (elm * elem_size);
        pfs->elmArray[elm].type.value   = MSGQ_ITEM_FREE;
        pfs->elmArray[elm].next         = NULL;
        pfs->elmArray[elm].prev         = NULL;

        DL_APPEND(pfs->free, &pfs->elmArray[elm]);

        elm++;
    }

    pfs->magic = HAL_MSGQ_MAGIC_VAL; /* Set only when fully initialized */
    return (uintptr_t)pfs;
}
