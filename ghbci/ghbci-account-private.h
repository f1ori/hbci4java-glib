/*
 * ghbci-account-private.h
 *
 * ghbci - A GObject wrapper of the libfreenect library
 * Copyright (C) 2014 Florian Richter
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

#ifndef __GHBCI_ACCOUNT_PRIVATE_H__
#define __GHBCI_ACCOUNT_PRIVATE_H__

#include <glib.h>
#include <glib-object.h>

GHbciAccount* ghbci_account_new_with_jobject (GHbciContext* context, jobject jobj);

#endif /* __GHBCI_ACCOUNT_PRIVATE_H__ */

