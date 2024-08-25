/**
 ******************************************************************************
 * @file    hal_alloc.c
 * @author  IMCv2 Team
 * @brief   Basic one-way 'brk' style allocator. By "one-way," we mean that 
 *          there is no support for freeing allocated memory (i.e., 
 *          no `free()` function).
 * @ref     https://en.wikipedia.org/wiki/Sbrk    
 *
 ******************************************************************************
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

/* Define a pool when it's in the hal base include */
#if defined(HAL_POOL_SIZE) && (HAL_POOL_SIZE > 0)
    uint8_t hal_mem_pool[HAL_POOL_SIZE];
#endif

#define HAL_BRK_MEM_MARKER_32  (0xa55aa55a)

/* Generic type to manage 'brk, sbrk' style allocations */
typedef struct __hal_brk_ctx
{
    uint8_t *p_mem_start;  /* Pointer to raw memory region to be managed */
    uint8_t *p_data_start; /* Pointer to the actual user data: raw data pointer + size of this structure (aligned) */
    uint8_t *p_mem_end;    /* Pointer to the end address within the raw memory region */
    uint8_t *brk;          /* Pointer to where the caller is currently at */
    uint8_t *ptr;          /* Return address after an allocation */
    uint32_t cur_size;     /* Current available size in the memory pool */
    uint32_t tot_size;     /* Total size of the memory pool */
    uint32_t mem_marker;   /* Marker to validate the memory context */
} hal_brk_ctx;

/**
 * @brief Align (upwards) a decimal value to the specified alignment.
 *
 * @param size      The size to align.
 * @param alignment The required alignment.
 * @retval Aligned value.
 */

static size_t hal_brk_align_up(size_t size, size_t alignment) {
    if (size == 0) {
        return 0;
    }

    size_t remainder = size % alignment;
    if (remainder == 0) {
        return size;
    }

    return size + (alignment - remainder);
}

/**
 * @brief Initialize an allocation context for managing memory.
 *
 * Prepares an object of type 'hal_brk_ctx' to be used by the sbrk allocator.
 *
 * @param mem_start Pointer to the start of the memory region to be managed.
 * @param tot_size  Size in bytes available for allocation starting from 'mem_start'.
 * @retval Pointer to an initialized memory context or 0 on error.
 */

uintptr_t hal_brk_alloc_init(void) {

#if defined(HAL_POOL_SIZE) && (HAL_POOL_SIZE > 0)

    const void *mem_start = hal_mem_pool;
    hal_brk_ctx *pCtx = (hal_brk_ctx *)mem_start;
    uint8_t ctxSize = hal_brk_align_up(sizeof(hal_brk_ctx), 8);
    uintptr_t addr;
    const size_t tot_size = HAL_POOL_SIZE;
    uint8_t *mem_end = NULL;

    if (!pCtx || tot_size == 0) {
        return 0;
    }

    /* Address must be 32-bit aligned, which could be critical for some platforms */
    if (((uintptr_t)mem_start & 0x3) != 0) {
        return 0;
    }

    mem_end = ((uint8_t *)(mem_start) + tot_size);

    /* Ensure the memory pool is large enough to include the brk context */
    if (tot_size <= sizeof(hal_brk_ctx)) {
        return 0;
    }

    #if(HAL_BRK_ALLOC_ZERO_MEM == 1)
        hal_zero_buf((char *)mem_start, tot_size);
    #endif
    
    pCtx->p_mem_start = (uint8_t *)mem_start;
    pCtx->p_mem_end   = (uint8_t *)mem_end;

    addr = (uintptr_t)pCtx->p_mem_start + ctxSize;
    pCtx->p_data_start  = (uint8_t *)addr;
    pCtx->brk           = pCtx->p_data_start; /* Initialize the allocation pointer just after this context header */
    pCtx->tot_size      = tot_size;
    pCtx->cur_size      = tot_size - ctxSize; /* Net size after reserving bytes for this context header */
    pCtx->ptr           = NULL;
    pCtx->mem_marker    = HAL_BRK_MEM_MARKER_32;

    return (uintptr_t)pCtx;

#else
    return 0; /* Pool size not defined */
#endif

}

/**
 * @brief Allocate a memory chunk from a pre-initialized region.
 *
 * This function allocates a memory block from the memory region managed by 
 * a pre-initialized 'hal_brk_ctx' structure.
 *
 * @param ctx Pointer to an initialized sbrk context (using hal_brk_alloc_init()).
 * @param size Size in bytes to allocate, this will be aligned up as needed.
 * @retval Valid pointer or NULL on error.
 */

void *hal_brk_alloc(__IO uintptr_t ctx, size_t size) {

#if defined(HAL_POOL_SIZE) && (HAL_POOL_SIZE > 0)

    hal_brk_ctx *pCtx = (hal_brk_ctx *)ctx; /* Handle to pointer */

    size_t size_aligned = 0;

    /* Note: Memory cannot be freed, negative values are not allowed */
    if (size == 0) {
        return NULL;
    }

    size_aligned = hal_brk_align_up(size,8);

    /* Validate the context pointer */
    if (!pCtx || pCtx->mem_marker != HAL_BRK_MEM_MARKER_32 || pCtx->cur_size == 0 || (pCtx->cur_size <= size_aligned)) {
        return NULL;
    }

    pCtx->ptr = pCtx->brk;          /* Set the return pointer */
    pCtx->brk += size_aligned;      /* Advance the next allocation pointer */
    pCtx->cur_size -= size_aligned; /* Decrease the total pool bytes */

    #if(HAL_BRK_ALLOC_ZERO_MEM == 1)
        /* Memory reset. */
        hal_zero_buf(pCtx->ptr,size_aligned);
    #endif

    return (void *)pCtx->ptr;
#else

    return NULL; /* Pool size not defined */
#endif

}
