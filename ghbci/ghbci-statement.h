/*
 * ghbci-statement.h
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

#ifndef __GHBCI_STATEMENT_H__
#define __GHBCI_STATEMENT_H__

#include <glib.h>
#include <glib-object.h>

#include "ghbci-context.h"

G_BEGIN_DECLS

typedef struct _GHbciStatement GHbciStatement;
typedef struct _GHbciStatementClass GHbciStatementClass;
typedef struct _GHbciStatementPrivate GHbciStatementPrivate;

struct _GHbciStatement
{
  GObject parent;

  GHbciStatementPrivate *priv;
};

/**
 * GHbciStatementClass:
 **/
struct _GHbciStatementClass
{
    GObjectClass parent_class;
};

#define GHBCI_TYPE_STATEMENT           (ghbci_statement_get_type ())
#define GHBCI_STATEMENT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GHBCI_TYPE_STATEMENT, GHbciStatement))
#define GHBCI_STATEMENT_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), GHBCI_TYPE_STATEMENT, GHbciStatementClass))
#define GHBCI_IS_STATEMENT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GHBCI_TYPE_STATEMENT))
#define GHBCI_IS_STATEMENT_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GHBCI_TYPE_STATEMENT))
#define GHBCI_STATEMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GHBCI_TYPE_STATEMENT, GHbciStatementClass))


GType             ghbci_statement_get_type                      (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __GHBCI_STATEMENT_H__ */
