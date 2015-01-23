/*
 * ghbci-context.h
 *
 * ghbci - A GObject wrapper of the hbci4java library
 * Copyright (C) 2014-2015 Florian Richter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
 * for more details.
 */

#ifndef __GHBCI_CONTEXT_H__
#define __GHBCI_CONTEXT_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GHbciContext GHbciContext;
typedef struct _GHbciContextClass GHbciContextClass;
typedef struct _GHbciContextPrivate GHbciContextPrivate;

struct _GHbciContext
{
  GObject parent;

  GHbciContextPrivate *priv;
};

/**
 * GHbciContextClass:
 **/
struct _GHbciContextClass
{
    GObjectClass parent_class;
};

#define GHBCI_TYPE_CONTEXT           (ghbci_context_get_type ())
#define GHBCI_CONTEXT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHBCI_TYPE_CONTEXT, GHbciContext))
#define GHBCI_CONTEXT_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), GHBCI_TYPE_CONTEXT, GHbciContextClass))
#define GHBCI_IS_CONTEXT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHBCI_TYPE_CONTEXT))
#define GHBCI_IS_CONTEXT_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GHBCI_TYPE_CONTEXT))
#define GHBCI_CONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GHBCI_TYPE_CONTEXT, GHbciContextClass))


/**
 * GHbciReason:
 *
 * Possible reasons for callback
 **/
typedef enum {
	GHBCI_REASON_ENUM_NEED_CHIPCARD = 2,
	GHBCI_REASON_ENUM_NEED_HARDPIN = 3,
	GHBCI_REASON_ENUM_NEED_SOFTPIN = 4,
	GHBCI_REASON_ENUM_HAVE_HARDPIN = 5,
	GHBCI_REASON_ENUM_HAVE_CHIPCARD = 6,
	GHBCI_REASON_ENUM_NEED_COUNTRY = 7,
	GHBCI_REASON_ENUM_NEED_BLZ = 8,
	GHBCI_REASON_ENUM_NEED_HOST = 9,
	GHBCI_REASON_ENUM_NEED_PORT = 10,
	GHBCI_REASON_ENUM_NEED_USERID = 11,
	GHBCI_REASON_ENUM_NEED_NEW_INST_KEYS_ACK = 12,
	GHBCI_REASON_ENUM_HAVE_NEW_MY_KEYS = 13,
	GHBCI_REASON_ENUM_HAVE_INST_MSG = 14,
	GHBCI_REASON_ENUM_NEED_REMOVE_CHIPCARD = 15,
	GHBCI_REASON_ENUM_NEED_PT_PIN = 16,
	GHBCI_REASON_ENUM_NEED_PT_TAN = 17,
	GHBCI_REASON_ENUM_NEED_CUSTOMERID = 18,
	GHBCI_REASON_ENUM_HAVE_CRC_ERROR = 19,
	GHBCI_REASON_ENUM_HAVE_ERROR = 20,
	GHBCI_REASON_ENUM_NEED_PASSPHRASE_LOAD = 21,
	GHBCI_REASON_ENUM_NEED_PASSPHRASE_SAVE = 22,
	GHBCI_REASON_ENUM_NEED_SIZENTRY_SELECT = 23,
	GHBCI_REASON_ENUM_NEED_CONNECTION = 24,
	GHBCI_REASON_ENUM_CLOSE_CONNECTION = 25,
	GHBCI_REASON_ENUM_NEED_FILTER = 26,
	GHBCI_REASON_ENUM_NEED_PT_SECMECH = 27,
	GHBCI_REASON_ENUM_NEED_PROXY_USER = 28,
	GHBCI_REASON_ENUM_NEED_PROXY_PASS = 29,
	GHBCI_REASON_ENUM_HAVE_IBAN_ERROR = 30,
	GHBCI_REASON_ENUM_NEED_INFOPOINT_ACK = 31,
	GHBCI_REASON_ENUM_NEED_PT_TANMEDIA = 32,
	GHBCI_REASON_ENUM_WRONG_PIN = 40,
	GHBCI_REASON_ENUM_USERID_CHANGED = 41,
} GHbciReason;

/**
 * GHbciLogLevel:
 *
 * Log levels support by hbci4java
 **/
typedef enum {
	GHBCI_LOGLEVEL_ENUM_ERROR = 1,
	GHBCI_LOGLEVEL_ENUM_WARN = 2,
	GHBCI_LOGLEVEL_ENUM_INFO = 3,
	GHBCI_LOGLEVEL_ENUM_DEBUG = 4,
	GHBCI_LOGLEVEL_ENUM_DEBUG2 = 5,
} GHbciLogLevel;


typedef void (*GHbciBlzFunc) (const gchar* blz, gpointer user_data);

GType             ghbci_context_get_type                      (void) G_GNUC_CONST;

GHbciContext*     ghbci_context_new                           ();

const gchar*      ghbci_context_get_name_for_blz              (GHbciContext* self, const gchar* blz);

void              ghbci_context_blz_foreach                   (GHbciContext* self, GHbciBlzFunc func, gpointer user_data);

gboolean          ghbci_context_add_passport                  (GHbciContext* self, const gchar* blz, const gchar* userid);

GSList*           ghbci_context_get_accounts                  (GHbciContext* self, const gchar* blz, const gchar* userid);

gchar*            ghbci_context_get_balances                  (GHbciContext* self, const gchar* blz, const gchar* userid, const gchar* number);

GSList*           ghbci_context_get_statements                (GHbciContext* self, const gchar* blz, const gchar* userid, const gchar* number);

G_END_DECLS

#endif /* __GHBCI_CONTEXT_H__ */
