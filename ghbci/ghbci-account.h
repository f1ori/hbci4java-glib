/*
 * ghbci-account.h
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

#ifndef __GHBCI_ACCOUNT_H__
#define __GHBCI_ACCOUNT_H__

#include <glib.h>
#include <glib-object.h>

#include "ghbci-context.h"

G_BEGIN_DECLS

typedef struct _GHbciAccount GHbciAccount;
typedef struct _GHbciAccountClass GHbciAccountClass;
typedef struct _GHbciAccountPrivate GHbciAccountPrivate;

struct _GHbciAccount
{
  GObject parent;

  GHbciAccountPrivate *priv;
};

/**
 * GHbciAccountClass:
 **/
struct _GHbciAccountClass
{
    GObjectClass parent_class;
};

#define GHBCI_TYPE_ACCOUNT           (ghbci_account_get_type ())
#define GHBCI_ACCOUNT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHBCI_TYPE_ACCOUNT, GHbciAccount))
#define GHBCI_ACCOUNT_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), GHBCI_TYPE_ACCOUNT, GHbciAccountClass))
#define GHBCI_IS_ACCOUNT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHBCI_TYPE_ACCOUNT))
#define GHBCI_IS_ACCOUNT_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GHBCI_TYPE_ACCOUNT))
#define GHBCI_ACCOUNT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GHBCI_TYPE_ACCOUNT, GHbciAccountClass))


GType             ghbci_account_get_type                      (void) G_GNUC_CONST;

GHbciAccount*     ghbci_account_new                           (GHbciContext* context);


G_END_DECLS

#endif /* __GHBCI_ACCOUNT_H__ */
