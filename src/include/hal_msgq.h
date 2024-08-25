
/**
  ******************************************************************************
  * @file    hal_msgq.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _HAL_MESSAGE_Q_H
#define _HAL_MESSAGE_Q_H

#include <stdint.h>
#include <stdbool.h>

/*! @brief The message queue item structure */
typedef struct __msgq_buf_t
{
    struct __msgq_buf_t *next, *prev;   /*!< List next and previous pointers */
    uint8_t *          p_data;          /*!< Pointer to the actual memory space containing the message */
    int32_t            state;           /*!< State of the item, application-defined */
    uint32_t           type;            /*!< Type of stored element (free or busy) */

} msgq_buf;


/**
 * @brief Requests a data pointer from the queue, moving the item to the busy list.
 * @param msgq_handle Handle to the storage instance.
 * @param data Pointer to data that should be copied to the storage, can be NULL.
 * @param size Size in bytes of 'data'.
 * @param reset If true, the storage buffer will be set to zeros before copying.
 * @retval Pointer to a message queue item or NULL on error.
 */

msgq_buf *msgq_request(uintptr_t msgq_handle, void *data, uint32_t size, bool reset);

/**
 * @brief Releases an element back to the free elements container.
 * @param msgq_handle Handle to the storage instance.
 * @param pdata Pointer to the element that should be released.
 * @retval 0 on success, 1 on error.
 */

int msgq_release(uintptr_t msgq_handle, msgq_buf *pdata);

/**
 * @brief Constructs a message queue storage instance.
 * @param elem_size Size in bytes of a single stored element.
 * @param tot_elements Maximum number of elements to store.
 * @retval Handle to the message queue (cast to uintptr_t) or 0 on error.
 */

uintptr_t msgq_create(uint32_t elem_size, uint32_t tot_elements);

#endif /* _HAL_MESSAGE_Q_H */