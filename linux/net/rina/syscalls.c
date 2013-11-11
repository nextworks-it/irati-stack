/*
 * System calls
 *
 *    Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/stringify.h>
#include <linux/uaccess.h>

#define RINA_PREFIX "syscalls"

#include "logs.h"
#include "utils.h"
#include "personality.h"
#include "debug.h"
#include "ipcp-utils.h"
#include "du.h"

#define CALL_PERSONALITY(RETVAL, PERS, HOOK, ARGS...)                   \
        do {                                                            \
                LOG_DBG("Handling personality hook %s",                 \
                        __stringify(HOOK));                             \
                                                                        \
                if (PERS == NULL) {                                     \
                        LOG_ERR("No personality registered");           \
                        return -1;                                      \
                }                                                       \
                                                                        \
                ASSERT(PERS);                                           \
                ASSERT(PERS -> ops);                                    \
                                                                        \
                if (PERS -> ops -> HOOK == NULL) {                      \
                        LOG_ERR("Personality has no %s hook",           \
                                __stringify(HOOK));                     \
                        return -1;                                      \
                }                                                       \
                                                                        \
                LOG_DBG("Calling personality hook %s",                  \
                        __stringify(HOOK));                             \
                                                                        \
                RETVAL = PERS -> ops -> HOOK (PERS->data, ##ARGS);      \
        } while (0)

#define CALL_DEFAULT_PERSONALITY(RETVAL, HOOK, ARGS...)                 \
        CALL_PERSONALITY(RETVAL, default_personality, HOOK, ##ARGS)

SYSCALL_DEFINE3(ipc_create,
                const struct name __user *, name,
                ipc_process_id_t,           id,
                const char __user *,        type)
{
        long          retval;

        struct name * tn;
        char *        tt;

        tn = name_dup_from_user(name);
        if (!tn)
                return -EFAULT;
        tt = strdup_from_user(type);
        if (!tt) {
                name_destroy(tn);
                return -EFAULT;
        }

        CALL_DEFAULT_PERSONALITY(retval, ipc_create, tn, id, tt);

        name_destroy(tn);
        rkfree(tt);

        return retval;
}

SYSCALL_DEFINE1(ipc_destroy,
                ipc_process_id_t, id)
{
        long retval;

        CALL_DEFAULT_PERSONALITY(retval, ipc_destroy, id);

        return retval;
}

SYSCALL_DEFINE3(sdu_read,
                port_id_t,     id,
                void __user *, buffer,
                size_t,        size)
{
        ssize_t      retval;

        struct sdu * tmp;
        size_t       retsize;

        tmp = NULL;

        CALL_DEFAULT_PERSONALITY(retval, sdu_read, id, &tmp);
        /* Taking ownership from the internal layers */

        LOG_DBG("Personality returned value %zd", retval);

        if (retval)
                return -EFAULT;

        if (!is_sdu_ok(tmp))
                return -EFAULT;

        /* NOTE: We don't handle partial copies */
        if (tmp->buffer->size > size) {
                LOG_ERR("Partial copies not handled. SDU size: %zd, "
                        "User space buffer size: %zd",
                        tmp->buffer->size, size);
                sdu_destroy(tmp);
                return -EFAULT;
        }

        if (copy_to_user(buffer,
                         tmp->buffer->data,
                         tmp->buffer->size)) {
                LOG_ERR("Error copying data to user-space");
                sdu_destroy(tmp);
                return -EFAULT;
        }

        retsize = tmp->buffer->size;
        sdu_destroy(tmp);

        return retsize;
}

SYSCALL_DEFINE3(sdu_write,
                port_id_t,           id,
                const void __user *, buffer,
                size_t,              size)
{
        ssize_t         retval;

        struct sdu *    sdu;
        struct buffer * tmp_buffer;

        if (!buffer || !size)
                return -EFAULT;

        LOG_DBG("Syscall write SDU of size %zd called with port-id %d",
                size, id);

        tmp_buffer = buffer_create(size);
        if (!tmp_buffer)
                return -EFAULT;

        ASSERT(is_buffer_ok(tmp_buffer));
        ASSERT(buffer_data(tmp_buffer));

        /* NOTE: We don't handle partial copies */
        if (copy_from_user(buffer_data(tmp_buffer), buffer, size)) {
                buffer_destroy(tmp_buffer);
                return -EFAULT;
        }

        /* NOTE: sdu_create takes the ownership of the buffer */
        sdu = sdu_create_from_buffer(tmp_buffer);
        if (!sdu) {
                buffer_destroy(tmp_buffer);
                return -EFAULT;
        }
        ASSERT(is_sdu_ok(sdu));

        /* Passing ownership to the internal layers */
        CALL_DEFAULT_PERSONALITY(retval, sdu_write, id, sdu);
        if (retval) {
                sdu_destroy(sdu);
                return -EFAULT;
        }

        return size;
}

SYSCALL_DEFINE2(allocate_port,
                ipc_process_id_t, id,
                int,             to_app)
{
        port_id_t retval;

        if (!id) 
                return -EFAULT;

        CALL_DEFAULT_PERSONALITY(retval, allocate_port, id, (to_app != 0) );

        return retval;
}

SYSCALL_DEFINE1(deallocate_port,
                port_id_t, id)
{
        int retval;

        CALL_DEFAULT_PERSONALITY(retval, deallocate_port, id);

        return retval;
}
