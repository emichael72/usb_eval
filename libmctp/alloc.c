/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */

#include <assert.h>
#include <stdio.h>
#include <hal.h>         /* Intel: LX7 infrastructure */
#include <test_defrag.h> /* Intel : libmctop integrationm test */
#include <hal_msgq.h>

#include "libmctp.h"
#include "libmctp-alloc.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

uintptr_t mcp_alloc_msg_andle     = 0;
uintptr_t mcp_alloc_context_andle = 0;

/* internal-only allocation functions */
void inline *__mctp_alloc(size_t size)
{
    /* Use Q */
    return msgq_request(mcp_alloc_msg_andle, size);
}

void inline __mctp_free(void *ptr)
{
    /* Use Q !*/
    msgq_release(mcp_alloc_msg_andle, ptr);
}

/* internal-only allocation functions */
void inline *__mctp_alloc_context(size_t size)
{
    /* Use Q */
    return msgq_request(mcp_alloc_context_andle, size);
}

void inline __mctp_free_context(void *ptr)
{
    /* Use Q !*/
    msgq_release(mcp_alloc_context_andle, ptr);
}

int __mctp_mem_init(void)
{
    mcp_alloc_msg_andle     = test_defrag_mctplib_get_handle(0);
    mcp_alloc_context_andle = test_defrag_mctplib_get_handle(1);

    if ( mcp_alloc_msg_andle && mcp_alloc_context_andle )
        return 0;

    return 1;
}
