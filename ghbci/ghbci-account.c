/*
 * ghbci-account.c
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

/**
 * SECTION:ghbci-account
 * @short_description: Object representing a hbci4java java vm
 *
 **/

#include <jni.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "ghbci-account.h"
#include "ghbci-account-private.h"
#include "ghbci-context.h"
#include "ghbci-context-private.h"
#include "ghbci-marshal.h"

#define GHBCI_ACCOUNT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                           GHBCI_TYPE_ACCOUNT, \
                                           GHbciAccountPrivate))

/* private data */
struct _GHbciAccountPrivate
{
    GMainContext *glib_context;

    jobject account_jobj;
    JNIEnv* jni_env;
    GHbciContext* context;
};

/* properties */
enum
{
    PROP_0,
    PROP_ACCOUNT_TYPE,
    PROP_TYPE,
    PROP_BIC,
    PROP_BLZ,
    PROP_COUNTRY,
    PROP_CURRENCY,
    PROP_CUSTOMERID,
    PROP_IBAN,
    PROP_NAME,
    PROP_NUMBER,
    PROP_SUBNUMBER
};

static void     ghbci_account_class_init         (GHbciAccountClass *class);
static void     ghbci_account_init               (GHbciAccount *self);
static void     ghbci_account_finalize           (GObject *obj);
static void     ghbci_account_dispose            (GObject *obj);
static void     ghbci_account_set_property       (GObject *obj,
                                                  guint prop_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec);
static void     ghbci_account_get_property       (GObject *obj,
                                                  guint prop_id,
                                                  GValue *value,
                                                  GParamSpec *pspec);

G_DEFINE_TYPE (GHbciAccount, ghbci_account, G_TYPE_OBJECT)


static void
ghbci_account_class_init (GHbciAccountClass *class)
{
    GObjectClass *obj_class;

    obj_class = G_OBJECT_CLASS (class);

    obj_class->dispose = ghbci_account_dispose;
    obj_class->finalize = ghbci_account_finalize;
    obj_class->get_property = ghbci_account_get_property;
    obj_class->set_property = ghbci_account_set_property;

    /**
     * GHbciAccount:account-type
     **/
    g_object_class_install_property (obj_class,
                                     PROP_ACCOUNT_TYPE,
                                     g_param_spec_string ("account-type",
                                                          "Account Type",
                                                          "Account Type (Girokonto, Sparkonto, Festgeldkonto, Kreditkartenkonto, etc)",
                                                          "no-name-set" /* default value*/,
                                                          G_PARAM_READWRITE));

    /**
     * GHbciAccount:bic
     *
     * International Bank Identifier (for SEPA)
     **/
    g_object_class_install_property (obj_class,
                                     PROP_BIC,
                                     g_param_spec_string ("bic",
                                                          "BIC",
                                                          "BIC (International Bank Identifier)",
                                                          "no-name-set" /* default value*/,
                                                          G_PARAM_READWRITE));

    /**
     * GHbciAccount:number
     *
     * account number
     **/
    g_object_class_install_property (obj_class,
                                     PROP_NUMBER,
                                     g_param_spec_string ("number",
                                                          "Number",
                                                          "Account number",
                                                          "no-name-set" /* default value*/,
                                                          G_PARAM_READWRITE));

    /* add private structure */
    g_type_class_add_private (obj_class, sizeof (GHbciAccountPrivate));
}

static void
ghbci_account_init (GHbciAccount *self)
{
    GHbciAccountPrivate *priv;

    priv = GHBCI_ACCOUNT_GET_PRIVATE (self);
    self->priv = priv;

    priv->jni_env = NULL;
    priv->context = NULL;
    priv->account_jobj = NULL;
}

static void
ghbci_account_dispose (GObject *obj)
{
    GHbciAccountPrivate *priv;
    GHbciContextPrivate *context_priv;

    GHbciAccount *self = GHBCI_ACCOUNT (obj);
    priv = GHBCI_ACCOUNT_GET_PRIVATE (self);
    context_priv = priv->context->priv;

    (*context_priv->jni_env)->DeleteGlobalRef(context_priv->jni_env, priv->account_jobj);

    G_OBJECT_CLASS (ghbci_account_parent_class)->dispose (obj);
}

static void
ghbci_account_finalize (GObject *obj)
{
  //GHbciAccount *self = GHBCI_ACCOUNT (obj);

  G_OBJECT_CLASS (ghbci_account_parent_class)->finalize (obj);
}

static void
ghbci_account_set_property (GObject      *obj,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GHbciAccount *self;
    GHbciAccountPrivate *priv;
    GHbciContextPrivate *context_priv;

    self = GHBCI_ACCOUNT (obj);
    priv = GHBCI_ACCOUNT_GET_PRIVATE (self);
    context_priv = priv->context->priv;

    const gchar* native_string = g_value_get_string (value);
    jstring jvalue = (*context_priv->jni_env)->NewStringUTF(context_priv->jni_env, native_string);

    switch (prop_id)
    {
    case PROP_COUNTRY:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_country, jvalue);
        break;
    case PROP_BLZ:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_blz, jvalue);
        break;
    case PROP_NUMBER:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_number, jvalue);
        break;
    case PROP_SUBNUMBER:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_subnumber, jvalue);
        break;
    case PROP_ACCOUNT_TYPE:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_acctype, jvalue);
        break;
    case PROP_TYPE:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_type, jvalue);
        break;
    case PROP_CURRENCY:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_curr, jvalue);
        break;
    case PROP_CUSTOMERID:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_customerid, jvalue);
        break;
    case PROP_NAME:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_name, jvalue);
        break;
    case PROP_BIC:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_bic, jvalue);
        break;
    case PROP_IBAN:
        (*context_priv->jni_env)->SetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_iban, jvalue);
        break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
    (*context_priv->jni_env)->DeleteGlobalRef(context_priv->jni_env, jvalue);
}

static void
ghbci_account_get_property (GObject    *obj,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    GHbciAccount *self;
    GHbciAccountPrivate *priv;
    GHbciContextPrivate *context_priv;
    jstring java_value;

    self = GHBCI_ACCOUNT (obj);
    priv = GHBCI_ACCOUNT_GET_PRIVATE (self);
    context_priv = priv->context->priv;

    switch (prop_id)
    {
    case PROP_COUNTRY:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_country);
        break;
    case PROP_BLZ:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_blz);
        break;
    case PROP_NUMBER:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_number);
        break;
    case PROP_SUBNUMBER:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_subnumber);
        break;
    case PROP_ACCOUNT_TYPE:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_acctype);
        break;
    case PROP_TYPE:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_type);
        break;
    case PROP_CURRENCY:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_curr);
        break;
    case PROP_CUSTOMERID:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_customerid);
        break;
    case PROP_NAME:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_name);
        break;
    case PROP_BIC:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_bic);
        break;
    case PROP_IBAN:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->account_jobj, context_priv->field_Konto_iban);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
        return;
    }
    const char* nativeString = (*context_priv->jni_env)->GetStringUTFChars(context_priv->jni_env, java_value, 0);
    g_value_set_string (value, nativeString);
    (*context_priv->jni_env)->ReleaseStringUTFChars(context_priv->jni_env, java_value, nativeString);
}

GHbciAccount*
ghbci_account_new_with_jobject (GHbciContext* context, jobject jobj)
{
    GHbciAccount* account;
    GHbciAccountPrivate* priv;

    account = g_object_new (GHBCI_TYPE_ACCOUNT, NULL);
    priv = account->priv;
    priv->context = context;
    priv->account_jobj = jobj;

    return account;
}


/* public methods */

/**
 * ghbci_account_new: (constructor)
 * @context: #GHbciContext object
 *
 * TODO
 *
 * Returns: (transfer full): A New #GHbciAccount
 **/
GHbciAccount*
ghbci_account_new (GHbciContext* context)
{
    GHbciAccount* account;
    GHbciAccountPrivate* priv;
    GHbciContextPrivate* context_priv;
    JNIEnv* jni_env;

    account = g_object_new (GHBCI_TYPE_ACCOUNT, NULL);
    priv = account->priv;
    priv->context = context;
    context_priv = context->priv;
    jni_env = context_priv->jni_env;
    priv->account_jobj = (*jni_env)->NewObject(jni_env, context_priv->class_Konto, context_priv->method_Konto_constructor);
    if (priv->account_jobj == NULL) {
        (*jni_env)->ExceptionDescribe(jni_env);
        return NULL;
    }

    return account;
}


// vim: sw=4 expandtab
