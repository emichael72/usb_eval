/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */

#include <assert.h>
#include <hal.h> /* Intel: LX7 infrastructure */
#include <test_mctplib.h> /* Intel : libmctop integrationm test */
#include <hal_msgq.h>

#include "libmctp.h"
#include "libmctp-alloc.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* internal-only allocation functions */
void *__mctp_alloc(size_t size)
{
    /* Use Q */
    return msgq_request(test_mctplib_get_msgq(), size);
}

void __mctp_free(void *ptr)
{
    /* Use Q !*/
    msgq_release(test_mctplib_get_msgq(), ptr);
}

void *__mctp_realloc(void *ptr, size_t size)
{
    /* No buffer resize ! */
    assert(0);
    return NULL;
}

void mctp_set_alloc_ops(void *(*m_alloc)(size_t), void (*m_free)(void *), void *(m_realloc) (void *, size_t))
{
    /* Not implimented */
}
