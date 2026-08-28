/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (C) 2026 OKTET Ltd. All rights reserved. */
/** @file
 * @brief Declarative typed configuration trees for Test Agents
 *
 * An alternative to the RCF_PCH_CFG_NODE_* macros: a configuration
 * subtree is a single static const literal written top-down, value
 * conversion between strings and C types is done by the framework,
 * and handlers receive a context object instead of positional
 * varargs.  Registered trees are mirrored onto plain
 * rcf_pch_cfg_object nodes, so they coexist with subtrees declared
 * the old way.
 */

#ifndef __TE_RCF_PCH_TREE_H__
#define __TE_RCF_PCH_TREE_H__

#include <inttypes.h>

#include "te_errno.h"
#include "te_defs.h"
#include "te_string.h"
#include "te_vector.h"
#include "te_enum.h"
#include "rcf_ch_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Value type of a configuration node. */
typedef enum ta_conf_type {
    TA_CONF_TYPE_NONE = 0, /**< No value: NA nodes, unvalued collections */
    TA_CONF_TYPE_STR,      /**< Free-form string */
    TA_CONF_TYPE_INT,      /**< Signed integer (intmax_t), decimal */
    TA_CONF_TYPE_UINT,     /**< Unsigned integer (uintmax_t), decimal */
    TA_CONF_TYPE_UINT0,    /**< Unsigned integer (uintmax_t), parsed
                                 with base 0 (accepts decimal, "0x"
                                 hexadecimal and "0"-prefixed octal)
                                 on set/add, printed as plain decimal
                                 on get, same as TA_CONF_TYPE_UINT */
    TA_CONF_TYPE_HEX32,    /**< Unsigned integer, hexadecimal
                                 ("0x%jx"), restricted to the 32-bit
                                 range [0, UINT32_MAX] by contract */
    TA_CONF_TYPE_BOOL,     /**< Boolean, "0"/"1" on the wire */
    TA_CONF_TYPE_ENUM,     /**< int mapped through a te_enum_map */
} ta_conf_type;

/** Opaque per-request context passed to every handler. */
typedef struct ta_conf_ctx ta_conf_ctx;

/**
 * Instance name of the nearest enclosing level with sub-identifier
 * @p sub_id (searched from the leaf up).  Unknown @p sub_id is a
 * programming error and aborts the agent via TE_FATAL_ERROR().
 *
 * The lookup never reaches the agent level: "agent" is not a valid
 * @p sub_id here and passing it is a programming error (it aborts,
 * same as any other unknown @p sub_id).  The agent name is not part
 * of the ta_conf OID; on the Unix agent it is available separately
 * as the global ta_name.
 *
 * @param ctx           request context
 * @param sub_id        sub-identifier of the enclosing level to look up
 *
 * @return Instance name of that level (never NULL)
 */
extern const char *ta_conf_ctx_inst(const ta_conf_ctx *ctx,
                                    const char *sub_id);

/**
 * Configuration group identifier of the request.
 *
 * @param ctx           request context
 *
 * @return Group identifier
 */
extern unsigned int ta_conf_ctx_gid(const ta_conf_ctx *ctx);

/**
 * Full instance OID of the request, or NULL in a commit handler
 * (commits get a truncated parsed OID internally).  In a list
 * handler (see ta_conf_list_fn) this is the PARENT instance OID,
 * not an OID of the collection being listed.
 *
 * @param ctx           request context
 *
 * @return Instance OID string, or NULL
 */
extern const char *ta_conf_ctx_oid(const ta_conf_ctx *ctx);

/**
 * Get handler; the member must match the node's type.
 *
 * For as_str, @p val is a te_string bound to a fixed-capacity
 * external buffer of RCF_MAX_VAL bytes supplied by the framework.
 * A value that does not fit is a programming error: unlike the
 * legacy RCF_PCH_CFG_NODE_* handlers, which silently truncate via
 * snprintf(), overflowing a te_string external buffer is fatal and
 * aborts the agent.
 *
 * @param ctx           request context
 * @param val           location for the retrieved value
 *
 * @return Status code
 */
typedef union ta_conf_get_fn {
    te_errno (*as_str)(ta_conf_ctx *ctx, te_string *val);
    te_errno (*as_int)(ta_conf_ctx *ctx, intmax_t *val);
    te_errno (*as_uint)(ta_conf_ctx *ctx, uintmax_t *val);
    te_errno (*as_bool)(ta_conf_ctx *ctx, bool *val);
    te_errno (*as_enum)(ta_conf_ctx *ctx, int *val);
} ta_conf_get_fn;

/**
 * Set handler; the member must match the node's type.
 *
 * @param ctx           request context
 * @param val           new value
 *
 * @return Status code
 */
typedef union ta_conf_set_fn {
    te_errno (*as_str)(ta_conf_ctx *ctx, const char *val);
    te_errno (*as_int)(ta_conf_ctx *ctx, intmax_t val);
    te_errno (*as_uint)(ta_conf_ctx *ctx, uintmax_t val);
    te_errno (*as_bool)(ta_conf_ctx *ctx, bool val);
    te_errno (*as_enum)(ta_conf_ctx *ctx, int val);
} ta_conf_set_fn;

/**
 * Add handler for collections: as_none for unvalued collections,
 * otherwise the member matching the node's type.  The new instance's
 * own name is ta_conf_ctx_inst(ctx, "<this node's name>").
 *
 * @param ctx           request context
 * @param val           value of the new instance (typed members only)
 *
 * @return Status code
 */
typedef union ta_conf_add_fn {
    te_errno (*as_none)(ta_conf_ctx *ctx);
    te_errno (*as_str)(ta_conf_ctx *ctx, const char *val);
    te_errno (*as_int)(ta_conf_ctx *ctx, intmax_t val);
    te_errno (*as_uint)(ta_conf_ctx *ctx, uintmax_t val);
    te_errno (*as_bool)(ta_conf_ctx *ctx, bool val);
    te_errno (*as_enum)(ta_conf_ctx *ctx, int val);
} ta_conf_add_fn;

/**
 * Delete handler for collections.
 *
 * @param ctx           request context
 *
 * @return Status code
 */
typedef te_errno (*ta_conf_del_fn)(ta_conf_ctx *ctx);

/**
 * List handler: append each instance name to @p names as a
 * heap-allocated char * (the framework owns them afterwards).
 *
 * Unlike add/del/get/set handlers of the collection's own instances,
 * a list handler runs one level up: @p ctx carries the PARENT
 * instance OID, not an instance OID of the collection being listed.
 * The collection's own sub-identifier is therefore not yet an
 * enclosing level, and ta_conf_ctx_inst(ctx, "<own sub_id>") is not
 * available here -- it aborts the agent, same as any other unknown
 * @p sub_id.  Look up the parent's instance names instead; the
 * names being listed are exactly what this handler produces.
 *
 * On the wire, the mirrored node joins @p names using the legacy
 * PCH list encoding: every name is emitted followed by a single
 * space, including an empty name -- so a single instance named ""
 * is distinguishable from an empty list, which the more common
 * "separator between names" join cannot represent.
 *
 * @param ctx           request context
 * @param names         vector of heap-allocated char * to append to
 *
 * @return Status code
 */
typedef te_errno (*ta_conf_list_fn)(ta_conf_ctx *ctx, te_vec *names);

/**
 * Commit handler; covers the node's whole subtree.
 *
 * @param ctx           request context
 *
 * @return Status code
 */
typedef te_errno (*ta_conf_commit_fn)(ta_conf_ctx *ctx);

/** One configuration tree node.  Normally built with the macros. */
typedef struct ta_conf_node {
    const char *name;       /**< Configurator sub-identifier */
    ta_conf_type type;      /**< Value type of get/set/add */
    const te_enum_map *map; /**< Name/value map, TA_CONF_TYPE_ENUM only */

    ta_conf_get_fn get;     /**< Get accessor or unset */
    ta_conf_set_fn set;     /**< Set accessor or unset */
    ta_conf_add_fn add;     /**< Collection add or unset */
    ta_conf_del_fn del;     /**< Collection delete or unset */
    ta_conf_list_fn list;   /**< Collection list or unset */
    ta_conf_commit_fn commit; /**< Commit covering the subtree */

    /** Substitutions passed through to the mirrored node */
    const rcf_pch_cfg_substitution *subst;

    /**
     * Children in sibling order.  Arrays built by TA_CONF_CHILDREN()
     * carry a leading NULL pad and a NULL terminator; iteration
     * starts at index 1.
     */
    const struct ta_conf_node *const *children;
} ta_conf_node;

/**
 * Validate @p root and graft it (and its subtree) under the object
 * OID @p father (e.g. "/agent"), like rcf_pch_add_node().  Must be
 * called during rcf_ch_conf_init(), i.e. from an agent conf init
 * function.  Returns TE_EINVAL (with an ERROR naming the node) on an
 * inconsistent tree.
 *
 * @param father        object OID to graft the subtree under
 * @param root          root of the subtree to register
 *
 * @return Status code
 */
extern te_errno ta_conf_register(const char *father,
                                 const ta_conf_node *root);

/**
 * Child node list.
 *
 * The expansion is a file-scope compound literal array of node
 * pointers with a leading NULL pad (so that the GNU ", ##" comma
 * deletion works for empty lists) and a NULL terminator.
 *
 * By convention, children of a node macro are indented one 4-space
 * level per tree depth, so that the source layout mirrors the tree.
 *
 * The node kind macros may be nested arbitrarily as children
 * arguments, including the same macro inside itself: argument
 * prescan fully expands nested invocations.
 *
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_CHILDREN(...) \
    (const ta_conf_node *const []){ NULL, ## __VA_ARGS__, NULL }

/** @cond internal */
#define TA_CONF__NODE(...) (&(const ta_conf_node){ __VA_ARGS__ })
/** @endcond */

/**
 * Non-accessible intermediate node; extra args are children.
 *
 * @param name_         node's Configurator sub-identifier
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_NA(name_, ...) \
    TA_CONF__NODE(.name = (name_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Non-accessible node whose commit covers its subtree.
 *
 * @param name_         node's Configurator sub-identifier
 * @param commit_       commit handler covering the subtree
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_NA_COMMIT(name_, commit_, ...) \
    TA_CONF__NODE(.name = (name_), .commit = (commit_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only string leaf; extra args are children.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_STR(name_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write string leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param set_          set handler (as_str)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_STR(name_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, \
                  .set = { .as_str = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only unsigned integer leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_UINT(name_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT, \
                  .get = { .as_uint = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write unsigned integer leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param set_          set handler (as_uint)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_UINT(name_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT, \
                  .get = { .as_uint = (get_) }, \
                  .set = { .as_uint = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only base-0 unsigned integer leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_UINT0(name_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT0, \
                  .get = { .as_uint = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write base-0 unsigned integer leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param set_          set handler (as_uint)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_UINT0(name_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT0, \
                  .get = { .as_uint = (get_) }, \
                  .set = { .as_uint = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only signed integer leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_int)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_INT(name_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_INT, \
                  .get = { .as_int = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write signed integer leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_int)
 * @param set_          set handler (as_int)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_INT(name_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_INT, \
                  .get = { .as_int = (get_) }, \
                  .set = { .as_int = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only hexadecimal unsigned leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_HEX32(name_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_HEX32, \
                  .get = { .as_uint = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write hexadecimal unsigned leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param set_          set handler (as_uint)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_HEX32(name_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_HEX32, \
                  .get = { .as_uint = (get_) }, \
                  .set = { .as_uint = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only boolean leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_bool)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_BOOL(name_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_BOOL, \
                  .get = { .as_bool = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write boolean leaf.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_bool)
 * @param set_          set handler (as_bool)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_BOOL(name_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_BOOL, \
                  .get = { .as_bool = (get_) }, \
                  .set = { .as_bool = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only enumerated leaf mapped through @p map_.
 *
 * @param name_         node's Configurator sub-identifier
 * @param map_          te_enum_map used to convert between the
 *                      wire string and the C int value
 * @param get_          get handler (as_enum)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_ENUM(name_, map_, get_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_ENUM, \
                  .map = (map_), \
                  .get = { .as_enum = (get_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write enumerated leaf mapped through @p map_.
 *
 * @param name_         node's Configurator sub-identifier
 * @param map_          te_enum_map used to convert between the
 *                      wire string and the C int value
 * @param get_          get handler (as_enum)
 * @param set_          set handler (as_enum)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_ENUM(name_, map_, get_, set_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_ENUM, \
                  .map = (map_), \
                  .get = { .as_enum = (get_) }, \
                  .set = { .as_enum = (set_) }, \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Unvalued collection (add/del/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param add_          add handler (as_none)
 * @param del_          delete handler
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_COLL(name_, add_, del_, list_, ...) \
    TA_CONF__NODE(.name = (name_), \
                  .add = { .as_none = (add_) }, .del = (del_), \
                  .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * String-valued collection (get/add/del/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param add_          add handler (as_str)
 * @param del_          delete handler
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_COLL_STR(name_, get_, add_, del_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, \
                  .add = { .as_str = (add_) }, .del = (del_), \
                  .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Fully read-write string-valued collection (get/set/add/del/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param set_          set handler (as_str)
 * @param add_          add handler (as_str)
 * @param del_          delete handler
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_COLL_STR_RW(name_, get_, set_, add_, del_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, \
                  .set = { .as_str = (set_) }, \
                  .add = { .as_str = (add_) }, .del = (del_), \
                  .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Fully read-write signed integer-valued collection
 * (get/set/add/del/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_int)
 * @param set_          set handler (as_int)
 * @param add_          add handler (as_int)
 * @param del_          delete handler
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_COLL_INT_RW(name_, get_, set_, add_, del_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_INT, \
                  .get = { .as_int = (get_) }, \
                  .set = { .as_int = (set_) }, \
                  .add = { .as_int = (add_) }, .del = (del_), \
                  .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Fully read-write string-valued collection (get/set/add/del/list)
 * whose commit covers its subtree.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param set_          set handler (as_str)
 * @param add_          add handler (as_str)
 * @param del_          delete handler
 * @param list_         list handler
 * @param commit_       commit handler covering the subtree
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_COLL_STR_RW_COMMIT(name_, get_, set_, add_, del_, \
                                   list_, commit_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, \
                  .set = { .as_str = (set_) }, \
                  .add = { .as_str = (add_) }, .del = (del_), \
                  .list = (list_), .commit = (commit_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Fully read-write base-0 unsigned integer-valued collection
 * (get/set/add/del/list) whose commit covers its subtree.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param set_          set handler (as_uint)
 * @param add_          add handler (as_uint)
 * @param del_          delete handler
 * @param list_         list handler
 * @param commit_       commit handler covering the subtree
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_COLL_UINT0_RW_COMMIT(name_, get_, set_, add_, del_, \
                                     list_, commit_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT0, \
                  .get = { .as_uint = (get_) }, \
                  .set = { .as_uint = (set_) }, \
                  .add = { .as_uint = (add_) }, .del = (del_), \
                  .list = (list_), .commit = (commit_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only string-valued collection (get/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_COLL(name_, get_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only signed integer-valued collection (get/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_int)
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_COLL_INT(name_, get_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_INT, \
                  .get = { .as_int = (get_) }, .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-only unsigned integer-valued collection (get/list).
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RO_COLL_UINT(name_, get_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT, \
                  .get = { .as_uint = (get_) }, .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write string-valued collection with no add/del (get/set/list):
 * instances are enumerable but not created or destroyed through this
 * node (e.g. their identity comes from elsewhere), only read and
 * written.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_str)
 * @param set_          set handler (as_str)
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_COLL_STR(name_, get_, set_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_STR, \
                  .get = { .as_str = (get_) }, \
                  .set = { .as_str = (set_) }, .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write unsigned integer-valued collection with no add/del
 * (get/set/list): instances are enumerable but not created or
 * destroyed through this node.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_uint)
 * @param set_          set handler (as_uint)
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_COLL_UINT(name_, get_, set_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_UINT, \
                  .get = { .as_uint = (get_) }, \
                  .set = { .as_uint = (set_) }, .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * Read-write boolean-valued collection with no add/del (get/set/list):
 * instances are enumerable but not created or destroyed through this
 * node.
 *
 * @param name_         node's Configurator sub-identifier
 * @param get_          get handler (as_bool)
 * @param set_          set handler (as_bool)
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_RW_COLL_BOOL(name_, get_, set_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .type = TA_CONF_TYPE_BOOL, \
                  .get = { .as_bool = (get_) }, \
                  .set = { .as_bool = (set_) }, .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/**
 * List-only collection: no value, no add/del, just enumerable
 * instances (e.g. a set of read-only sub-identifiers with no
 * per-instance data of their own).
 *
 * @param name_         node's Configurator sub-identifier
 * @param list_         list handler
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_LIST(name_, list_, ...) \
    TA_CONF__NODE(.name = (name_), .list = (list_), \
                  .children = TA_CONF_CHILDREN(__VA_ARGS__))

/** @cond internal */
#define TA_CONF__UNPAREN(...) __VA_ARGS__
/** @endcond */

/**
 * Generic node, for shapes with no dedicated macro (odd handler
 * combinations, .subst, etc.).  Unlike a hand-written literal of the
 * form &(const ta_conf_node){ ..., .children = TA_CONF_CHILDREN(...) },
 * the children passed here through @p ... are macro arguments of
 * TA_CONF_NODE() itself, so macro-generated children (e.g. other
 * TA_CONF_* node macros) are expanded before TA_CONF_CHILDREN() ever
 * sees them, same as with every other node macro in this file.
 *
 * @param fields_       designated field initializers for the node,
 *                      as a single parenthesized group, e.g.
 *                      (.name = "x", .type = TA_CONF_TYPE_STR, ...)
 * @param ...           child node pointers, in sibling order
 */
#define TA_CONF_NODE(fields_, ...) \
    (&(const ta_conf_node){ TA_CONF__UNPAREN fields_, \
                            .children = TA_CONF_CHILDREN(__VA_ARGS__) })

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* __TE_RCF_PCH_TREE_H__ */
