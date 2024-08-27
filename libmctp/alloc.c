/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */

#include <assert.h>
#include <hal.h> /* LX7 Infrastructure */
#include <hal_msgq.h>
#include <mctplib_usb.h>

#include "libmctp.h"
#include "libmctp-alloc.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* internal-only allocation functions */
void *__mctp_alloc(size_t size)
{
    /* Use Q */
    return msgq_request(mctp_usb_get_msgq_handle(), size);
}

void __mctp_free(void *ptr)
{
    /* Use Q !*/
    msgq_release(mctp_usb_get_msgq_handle(), ptr);
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
