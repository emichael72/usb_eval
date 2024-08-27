
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

/**
 * @brief Requests a data pointer from the queue, moving the item to the busy list.
 * @param msgq_handle Handle to the storage instance.
 * @param size Size in bytes of 'data'.
 * @retval Pointer to a message queue item or NULL on error.
 */

void *msgq_request(uintptr_t msgq_handle, size_t size);

/**
 * @brief Releases an element back to the free elements container.
 * @param msgq_handle Handle to the storage instance.
 * @param data Pointer to the buffer that should be released.
 * @retval 0 on success, 1 on error.
 */

int msgq_release(uintptr_t msgq_handle, void *data);

/**
 * @brief Constructs a message queue storage instance.
 * @param item_size Size in bytes of a single stored element.
 * @param items_count Maximum number of elements to store.
 * @retval Handle to the message queue (cast to uintptr_t) or 0 on error.
 */

uintptr_t msgq_create(size_t item_size, size_t items_count);

#endif /* _HAL_MESSAGE_Q_H */