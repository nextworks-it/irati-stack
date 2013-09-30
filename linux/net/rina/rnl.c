/*
 * RNL (RINA NetLink support)
 *
 *    Francesco Salvestrini <f.salvestrini@nextworks.it>
 *    Leonardo Bergesio <leonardo.bergesio@i2cat.net>
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

#define RINA_PREFIX "rnl"

#include "logs.h"
#include "utils.h"
#include "debug.h"
#include "rnl.h"
#include "rnl-utils.h"

#define NETLINK_RINA "rina"

#define NETLINK_RINA_C_MIN (RINA_C_MIN + 1)
#define NETLINK_RINA_C_MAX (RINA_C_MAX - 1)

struct message_handler {
        void *             data;
        message_handler_cb cb;
};

struct rnl_set {
        struct message_handler handlers[NETLINK_RINA_C_MAX];
};

static struct rnl_set * default_set = NULL;

struct genl_family rnl_nl_family = {
        .id      = GENL_ID_GENERATE,
        /* .hdrsize = 0, */
        .hdrsize = sizeof(struct rina_msg_hdr),
        .name    = NETLINK_RINA,
        .version = 1,
        /* .maxattr = NETLINK_RINA_A_MAX, */
};

#if 0
/* FIXME: Why an external method to access our "private" data structures ??? */
struct genl_family * rnl_family(void)
{ return &nl_family; }
#endif

static int is_message_type_in_range(msg_type_t msg_type)
{ return is_value_in_range(msg_type, NETLINK_RINA_C_MIN, NETLINK_RINA_C_MAX); }

static int dispatcher(struct sk_buff * skb_in, struct genl_info * info)
{
        /*
         * Message handling code goes here; return 0 on success, negative
         * values on failure
         */

        /* FIXME: What do we do if the handler returns != 0 ??? */

        message_handler_cb cb_function;
        void *             data;
        msg_type_t         msg_type;
        int                ret_val;
        struct rnl_set *   tmp;

        LOG_DBG("Dispatching message (skb-in=%pK, info=%pK)", skb_in, info);
        if (!info) {
                LOG_ERR("Can't dispatch message, info parameter is empty");
                return -1;
        }

        if (!info->genlhdr) {
                LOG_ERR("Received message has no genlhdr field, "
                        "bailing out");
                return -1;
        }

        msg_type = (msg_type_t) info->genlhdr->cmd;
        LOG_DBG("Multiplexing message type %d", msg_type);

        if (!is_message_type_in_range(msg_type)) {
                LOG_ERR("Wrong message type %d received from Netlink, "
                        "bailing out", msg_type);
                return -1;
        }
        ASSERT(is_message_type_in_range(msg_type));

        tmp = default_set;
        if (!tmp) {
                LOG_ERR("There is no set registered, "
                        "first register a (default) set");
                return -1;
        }

        cb_function = tmp->handlers[msg_type].cb;
        LOG_DBG("[LDBG] Catching callback cb_function =%pK)", cb_function);
        if (!cb_function) {
                LOG_ERR("There's no handler callback registered for "
                        "message type %d", msg_type);
                return -1;
        }

        data = tmp->handlers[msg_type].data;
        /* Data might be empty */

        ret_val = cb_function(data, skb_in, info);
        if (ret_val) {
                LOG_ERR("Callback returned %d, bailing out", ret_val);
                return -1;
        }

        LOG_DBG("Message %d handled successfully", msg_type);

        return 0;
}

static struct genl_ops nl_ops[] = {
        {
                .cmd    = RINA_C_IPCM_ASSIGN_TO_DIF_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ASSIGN_TO_DIF_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_IPC_PROCESS_DIF_REGISTRATION_NOTIFICATION,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_IPC_PROCESS_DIF_UNREGISTRATION_NOTIFICATION,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ENROLL_TO_DIF_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ENROLL_TO_DIF_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_DISCONNECT_FROM_NEIGHBOR_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_DISCONNECT_FROM_NEIGHBOR_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ALLOCATE_FLOW_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ALLOCATE_FLOW_REQUEST_ARRIVED,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ALLOCATE_FLOW_REQUEST_RESULT,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_ALLOCATE_FLOW_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_DEALLOCATE_FLOW_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_DEALLOCATE_FLOW_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_FLOW_DEALLOCATED_NOTIFICATION,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_REGISTER_APPLICATION_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_REGISTER_APPLICATION_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_UNREGISTER_APPLICATION_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_UNREGISTER_APPLICATION_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_QUERY_RIB_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_IPCM_QUERY_RIB_RESPONSE,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_RMT_ADD_FTE_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_RMT_DELETE_FTE_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_RMT_DUMP_FT_REQUEST,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
        {
                .cmd    = RINA_C_RMT_DUMP_FT_REPLY,
                .flags  = 0,
                //.policy = nl_rina_policy,
                .doit   = dispatcher,
                .dumpit = NULL,
        },
};

int rnl_handler_register(struct rnl_set *   set,
                         msg_type_t         msg_type,
                         void *             data,
                         message_handler_cb handler)
{
        if (!set) {
                LOG_ERR("Bogus set passed, cannot register handler");
                return -1;
        }

        LOG_DBG("Registering handler callback %pK and data %pK "
                "for message type %d", handler, data, msg_type);

        if (!handler) {
                LOG_ERR("Handler for message type %d is empty, "
                        "use unregister to remove it", msg_type);
                return -1;
        }

        if (!is_message_type_in_range(msg_type)) {
                LOG_ERR("Message type %d is out-of-range, "
                        "cannot register", msg_type);
                return -1;
        }
        ASSERT(msg_type >= NETLINK_RINA_C_MIN &&
               msg_type <= NETLINK_RINA_C_MAX);
        ASSERT(handler != NULL);

        if (set->handlers[msg_type].cb) {
                LOG_ERR("The message handler for message type %d "
                        "has been already registered, unregister it first",
                        msg_type);
                return -1;
        }

        set->handlers[msg_type].cb   = handler;
        set->handlers[msg_type].data = data;

        LOG_DBG("Handler %pK (data %pK) registered for message type %d",
                handler, data, msg_type);

        return 0;
}
EXPORT_SYMBOL(rnl_handler_register);

int rnl_handler_unregister(struct rnl_set * set,
                           msg_type_t       msg_type)
{
        if (!set) {
                LOG_ERR("Bogus set passed, cannot register handler");
                return -1;
        }

        LOG_DBG("Unregistering handler for message type %d", msg_type);

        if (!is_message_type_in_range(msg_type)) {
                LOG_ERR("Message type %d is out-of-range, "
                        "cannot unregister", msg_type);
                return -1;
        }
        ASSERT(msg_type >= NETLINK_RINA_C_MIN &&
               msg_type <= NETLINK_RINA_C_MAX);

        bzero(&set->handlers[msg_type], sizeof(set->handlers[msg_type]));

        LOG_DBG("Handler for message type %d unregistered successfully",
                msg_type);

        return 0;
}
EXPORT_SYMBOL(rnl_handler_unregister);

int rnl_set_register(struct rnl_set * set)
{
        if (!set) {
                LOG_ERR("Bogus set passed, cannot register it");
                return -1;
        }

        if (default_set != NULL) {
                LOG_ERR("Default set already registered");
                return -2;
        }

        default_set = set;

        LOG_DBG("Set %pK registered", set);

        return 0;
}
EXPORT_SYMBOL(rnl_set_register);

int rnl_set_unregister(struct rnl_set * set)
{
        if (!set) {
                LOG_ERR("Bogus set passed, cannot unregister it");
                return -1;
        }

        if (default_set != set) {
                LOG_ERR("Target set is different than the registered one");
                return -2;
        }

        default_set = NULL;

        LOG_DBG("Set %pK unregistered", set);

        return 0;
}
EXPORT_SYMBOL(rnl_set_unregister);

struct rnl_set * rnl_set_create(personality_id id)
{
        struct rnl_set * tmp;

        tmp = rkzalloc(sizeof(struct rnl_set), GFP_ATOMIC);
        if (!tmp)
                return NULL;

        LOG_DBG("Set %pK created successfully", tmp);

        return tmp;
}
EXPORT_SYMBOL(rnl_set_create);

int rnl_set_destroy(struct rnl_set * set)
{
        int    i;
        size_t count;

        if (!set) {
                LOG_ERR("Bogus set passed, cannot destroy");
                return -1;
        }

        count = 0;
        for (i = 0; i < ARRAY_SIZE(set->handlers); i++) {
                if (set->handlers[i].cb != NULL) {
                        count++;
                        LOG_DBG("Set %pK has at least one hander still "
                                "registered, it will be unregistered", set);
                        break;
                }
        }
        if (count)
                LOG_WARN("Set %pK had %zd handler(s) that have not been "
                         "unregistered ...", set, count);
        rkfree(set);

        LOG_DBG("Set %pK destroyed %s", set, count ? "" : "successfully");

        return 0;
}
EXPORT_SYMBOL(rnl_set_destroy);

/*
 * Invoked when an event related to a NL socket occurs. We're only interested
 * in socket closed events.
 */
static int kipcm_netlink_notify(struct notifier_block * nb,
                                unsigned long           state,
                                void *                  notification)
{
	int                     result;
	struct netlink_notify * notify = notification;

	if (state != NETLINK_URELEASE)
		return NOTIFY_DONE;

	if (!notify) {
		LOG_ERR("Wrong data obtained in netlink notifier callback");
		return NOTIFY_BAD;
	}

	/* FIXME: Is the IPCM available ??? */
	result = rnl_ipcm_sock_closed_notif_msg(notify->portid, 1);
	if (result)
		LOG_ERR("Error notifying IPC Manager in user space, %d",
                        result);
	else
		LOG_DBG("IPC Manager notification sent");

	return NOTIFY_DONE;
}

static struct notifier_block kipcm_netlink_notifier = {
        .notifier_call = kipcm_netlink_notify,
};

int rnl_init(void)
{
        int ret;

        LOG_DBG("Initializing Netlink layer");

        ret = genl_register_family_with_ops(&rnl_nl_family,
                                            nl_ops,
                                            ARRAY_SIZE(nl_ops));
        if (ret != 0) {
                LOG_ERR("Cannot register Netlink family and ops (error=%i), "
                        "bailing out", ret);
                return -1;
        }
        LOG_DBG("NL family registered (id = %d)", rnl_nl_family.id);

        /*
         * Register a NETLINK notifier so that the kernel is
         * informed when a Netlink socket in user-space is closed
         */
        ret = netlink_register_notifier(&kipcm_netlink_notifier);
        if (ret) {
                LOG_ERR("Cannot register Netlink notifier (error=%i), "
                        "bailing out", ret);
                genl_unregister_family(&rnl_nl_family);
                return -1;
        }

        LOG_DBG("NetLink layer initialized successfully");

        return 0;
}

void rnl_exit(void)
{
        int ret;

        LOG_DBG("Finalizing Netlink layer");

        /* Unregister the notifier */
        netlink_unregister_notifier(&kipcm_netlink_notifier);

        ret = genl_unregister_family(&rnl_nl_family);
        if (ret) {
                LOG_ERR("Could not unregister Netlink family (error=%i), "
                        "bailing out. Your system might become unstable", ret);
                return;
        }

        /*
         * FIXME:
         *   Add checks here to prevent misses of finalizations and/or
         *   destructions
         */

        ASSERT(!default_set);

        LOG_DBG("NetLink layer finalized successfully");
}
