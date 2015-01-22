/*
 * ghbci-statement.c
 *
 * ghbci - A GObject wrapper of the libfreenect library
 * Copyright (C) 2015 Florian Richter
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
 * SECTION:ghbci-statement
 * @short_description: Object representing a hbci4java java vm
 *
 **/

#include <jni.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "ghbci-statement.h"
#include "ghbci-statement-private.h"
#include "ghbci-context.h"
#include "ghbci-context-private.h"
#include "ghbci-marshal.h"

#define GHBCI_STATEMENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                           GHBCI_TYPE_STATEMENT, \
                                           GHbciStatementPrivate))

/* private data */
struct _GHbciStatementPrivate
{
    GMainContext *glib_context;

    jobject statement_jobj;
    JNIEnv* jni_env;
    GHbciContext* context;

    GDate* valuta;
    GDate* booking_date;
    gchar* value;
    gchar* saldo;
    gchar* gv_code;
    gchar* reference;
    gchar* other_name;
    gchar* other_bic;
    gchar* other_iban;
};

/* properties */
enum
{
    PROP_0,
    PROP_VALUTA,
    PROP_BOOKING_DATE,
    PROP_VALUE,
    PROP_SALDO,
    PROP_GV_CODE,
    PROP_REFERENCE,
    PROP_OTHER_NAME,
    PROP_OTHER_IBAN,
    PROP_OTHER_BIC,
};

static void     ghbci_statement_class_init         (GHbciStatementClass *class);
static void     ghbci_statement_init               (GHbciStatement *self);
static void     ghbci_statement_finalize           (GObject *obj);
static void     ghbci_statement_dispose            (GObject *obj);
static void     ghbci_statement_set_property       (GObject *obj,
                                                    guint prop_id,
                                                    const GValue *value,
                                                    GParamSpec *pspec);
static void     ghbci_statement_get_property       (GObject *obj,
                                                    guint prop_id,
                                                    GValue *value,
                                                    GParamSpec *pspec);

G_DEFINE_TYPE (GHbciStatement, ghbci_statement, G_TYPE_OBJECT)


static void
ghbci_statement_class_init (GHbciStatementClass *class)
{
    GObjectClass *obj_class;

    obj_class = G_OBJECT_CLASS (class);

    obj_class->dispose = ghbci_statement_dispose;
    obj_class->finalize = ghbci_statement_finalize;
    obj_class->get_property = ghbci_statement_get_property;
    obj_class->set_property = ghbci_statement_set_property;

    /**
     * GHbciStatement:valuta
     **/
    g_object_class_install_property (obj_class,
                                     PROP_VALUTA,
                                     g_param_spec_boxed ("valuta",
                                                         "Valuta date", /* nick */
                                                         "Valuta date", /* blurb */
                                                         G_TYPE_DATE,
                                                         G_PARAM_READABLE));

    /**
     * GHbciStatement:booking-date
     **/
    g_object_class_install_property (obj_class,
                                     PROP_BOOKING_DATE,
                                     g_param_spec_boxed ("booking-date",
                                                         "Booking date",
                                                         "Booking date",
                                                         G_TYPE_DATE,
                                                         G_PARAM_READABLE));

    /**
     * GHbciStatement:value
     *
     * transaction amount
     **/
    g_object_class_install_property (obj_class,
                                     PROP_VALUE,
                                     g_param_spec_string ("value",
                                                          "Value",
                                                          "Value (transaction amount)",
                                                          "0.00" /* default value*/,
                                                          G_PARAM_READABLE));
    /**
     * GHbciStatement:saldo
     *
     * saldo after transaction
     **/
    g_object_class_install_property (obj_class,
                                     PROP_SALDO,
                                     g_param_spec_string ("saldo",
                                                          "Saldo",
                                                          "Saldo after transaction",
                                                          "0.00" /* default value*/,
                                                          G_PARAM_READABLE));

    /**
     * GHbciStatement:gv-code
     *
     * Code des Geschaeftsvorfalls
     **/
    g_object_class_install_property (obj_class,
                                     PROP_GV_CODE,
                                     g_param_spec_string ("gv-code",
                                                          "GV Code",
                                                          "GV Code",
                                                          "0" /* default value*/,
                                                          G_PARAM_READABLE));

    /**
     * GHbciStatement:reference
     *
     * reference / description
     **/
    g_object_class_install_property (obj_class,
                                     PROP_REFERENCE,
                                     g_param_spec_string ("reference",
                                                          "Reference",
                                                          "Reference / Description",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READABLE));

    /**
     * GHbciStatement:other-name
     *
     * Name of other account
     **/
    g_object_class_install_property (obj_class,
                                     PROP_OTHER_NAME,
                                     g_param_spec_string ("other-name",
                                                          "Name of other account",
                                                          "Name of other account",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READABLE));

    /**
     * GHbciStatement:other-iban
     *
     * IBAN of other account
     **/
    g_object_class_install_property (obj_class,
                                     PROP_OTHER_IBAN,
                                     g_param_spec_string ("other-iban",
                                                          "IBAN of other account",
                                                          "IBAN of other account",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READABLE));

    /**
     * GHbciStatement:other-bic
     *
     * BIC of other account
     **/
    g_object_class_install_property (obj_class,
                                     PROP_OTHER_BIC,
                                     g_param_spec_string ("other-bic",
                                                          "BIC of other account",
                                                          "BIC of other account",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READABLE));

    /* add private structure */
    g_type_class_add_private (obj_class, sizeof (GHbciStatementPrivate));
}

static void
ghbci_statement_init (GHbciStatement *self)
{
    GHbciStatementPrivate *priv;

    priv = GHBCI_STATEMENT_GET_PRIVATE (self);
    self->priv = priv;

    priv->valuta = NULL;
    priv->booking_date = NULL;
    priv->value = NULL;
    priv->saldo = NULL;
    priv->gv_code = NULL;
    priv->reference = NULL;
    priv->other_name = NULL;
    priv->other_bic = NULL;
    priv->other_iban = NULL;
}

static void
ghbci_statement_dispose (GObject *obj)
{
    GHbciStatementPrivate *priv;

    GHbciStatement *self = GHBCI_STATEMENT (obj);
    priv = GHBCI_STATEMENT_GET_PRIVATE (self);

    g_free(priv->valuta);
    g_free(priv->booking_date);
    g_free(priv->value);
    g_free(priv->saldo);
    g_free(priv->gv_code);
    g_free(priv->reference);
    g_free(priv->other_name);
    g_free(priv->other_bic);
    g_free(priv->other_iban);

    G_OBJECT_CLASS (ghbci_statement_parent_class)->dispose (obj);
}

static void
ghbci_statement_finalize (GObject *obj)
{
  //GHbciStatement *self = GHBCI_STATEMENT (obj);

  G_OBJECT_CLASS (ghbci_statement_parent_class)->finalize (obj);
}

static void
ghbci_statement_set_property (GObject      *obj,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    //GHbciStatement *self;
    //GHbciStatementPrivate *priv;

    //self = GHBCI_STATEMENT (obj);
    //priv = GHBCI_STATEMENT_GET_PRIVATE (self);

    switch (prop_id)
    {

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
ghbci_statement_get_property (GObject    *obj,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GHbciStatement *self;
    GHbciStatementPrivate *priv;

    self = GHBCI_STATEMENT (obj);
    priv = GHBCI_STATEMENT_GET_PRIVATE (self);

    switch (prop_id)
    {
    case PROP_VALUTA:
        g_value_set_boxed (value, priv->valuta);
        break;

    case PROP_BOOKING_DATE:
        g_value_set_boxed (value, priv->booking_date);
        break;

    case PROP_VALUE:
        g_value_set_string (value, priv->value);
        break;

    case PROP_SALDO:
        g_value_set_string (value, priv->saldo);
        break;

    case PROP_REFERENCE:
        g_value_set_string (value, priv->reference);
        break;

    case PROP_GV_CODE:
        g_value_set_string (value, priv->gv_code);
        break;

    case PROP_OTHER_NAME:
        g_value_set_string (value, priv->other_name);
        break;

    case PROP_OTHER_IBAN:
        g_value_set_string (value, priv->other_iban);
        break;

    case PROP_OTHER_BIC:
        g_value_set_string (value, priv->other_bic);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
        return;
    }
#if 0
    switch (prop_id)
    {
    case PROP_VALUTA:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_valuta);
        //java_value = (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, java_value, context_priv->method_Date_toString);
        jdate = (*context_priv->jni_env)->CallIntMethod(context_priv->jni_env, java_value, context_priv->method_Date_getDate);
        jmonth = (*context_priv->jni_env)->CallIntMethod(context_priv->jni_env, java_value, context_priv->method_Date_getMonth) + 1;
        jyear = (*context_priv->jni_env)->CallIntMethod(context_priv->jni_env, java_value, context_priv->method_Date_getYear) + 1900;
        printf("date: %d.%d.%d\n", jdate, jmonth, jyear);
        date = g_date_new_dmy(jdate, jmonth, jyear);
        g_value_take_boxed (value, date);
        return;

    case PROP_BOOKING_DATE:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_bdate);
        //java_value = (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, java_value, context_priv->method_Date_toString);
        jdate = (*context_priv->jni_env)->CallIntMethod(context_priv->jni_env, java_value, context_priv->method_Date_getDate);
        jmonth = (*context_priv->jni_env)->CallIntMethod(context_priv->jni_env, java_value, context_priv->method_Date_getMonth) + 1;
        jyear = (*context_priv->jni_env)->CallIntMethod(context_priv->jni_env, java_value, context_priv->method_Date_getYear) + 1900;
        printf("date: %d.%d.%d\n", jdate, jmonth, jyear);
        date = g_date_new_dmy(jdate, jmonth, jyear);
        g_value_take_boxed (value, date);
        return;

    case PROP_VALUE:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_value);
        java_value = (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, java_value, context_priv->method_Value_toString);
        break;

    case PROP_SALDO:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_saldo);
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, java_value, context_priv->field_Saldo_value);
        java_value = (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, java_value, context_priv->method_Value_toString);
        break;

    case PROP_REFERENCE:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_usage);
        jobject iterator = (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, java_value, context_priv->method_List_iterator);
        GString* reference = g_string_new(NULL);
        while( (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, iterator, context_priv->method_Iterator_hasNext) ) {
            jstring jusage_line = (*context_priv->jni_env)->CallObjectMethod(context_priv->jni_env, iterator, context_priv->method_Iterator_next);
            if (jusage_line == NULL) {
                break;
            }

            const char* usage_line = (*context_priv->jni_env)->GetStringUTFChars(context_priv->jni_env, jusage_line, 0);
            g_string_append(reference, usage_line);
            (*context_priv->jni_env)->ReleaseStringUTFChars(context_priv->jni_env, jusage_line, usage_line);
        }
        g_value_take_string (value, g_string_free(reference, FALSE) );
        return;

    case PROP_GV_CODE:
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_gvcode);
        break;

    case PROP_OTHER_NAME:
        other = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_other);
        if (other == NULL) return;
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, other, context_priv->field_Konto_name);
        break;

    case PROP_OTHER_BLZ:
        other = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_other);
        if (other == NULL) return;
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, other, context_priv->field_Konto_blz);
        break;

    case PROP_OTHER_NUMBER:
        other = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_other);
        if (other == NULL) return;
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, other, context_priv->field_Konto_number);
        break;

    case PROP_OTHER_IBAN:
        other = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_other);
        if (other == NULL) return;
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, other, context_priv->field_Konto_iban);
        break;

    case PROP_OTHER_BIC:
        other = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, priv->statement_jobj, context_priv->field_GVRKUmsUmsLine_other);
        if (other == NULL) return;
        java_value = (*context_priv->jni_env)->GetObjectField(context_priv->jni_env, other, context_priv->field_Konto_bic);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
        return;
    }
    if(java_value == NULL) {
        (*context_priv->jni_env)->ExceptionDescribe(context_priv->jni_env);
        return;
    }
    const char* nativeString = (*context_priv->jni_env)->GetStringUTFChars(context_priv->jni_env, java_value, 0);
    g_value_set_string (value, nativeString);
    (*context_priv->jni_env)->ReleaseStringUTFChars(context_priv->jni_env, java_value, nativeString);
#endif
}

gchar* ghbci_statement_jstring_to_cstring(JNIEnv* jni_env, jstring jstr)
{
    gchar* c_string;
    const char* native_string;

    if (jstr == NULL) {
        return NULL;
    }

    native_string = (*jni_env)->GetStringUTFChars(jni_env, jstr, 0);
    c_string = g_strdup(native_string);
    (*jni_env)->ReleaseStringUTFChars(jni_env, jstr, native_string);

    return c_string;
}

GHbciStatement*
ghbci_statement_new_with_jobject (GHbciContext* context, jobject jstatement)
{
    GHbciStatement* statement;
    GHbciStatementPrivate* priv;
    JNIEnv* jni_env;

    statement = g_object_new (GHBCI_TYPE_STATEMENT, NULL);
    priv = statement->priv;
    jni_env = context->priv->jni_env;

    jobject jvaluta = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_valuta);
    jint jdate = (*jni_env)->CallIntMethod(jni_env, jvaluta, context->priv->method_Date_getDate);
    jint jmonth = (*jni_env)->CallIntMethod(jni_env, jvaluta, context->priv->method_Date_getMonth) + 1;
    jint jyear = (*jni_env)->CallIntMethod(jni_env, jvaluta, context->priv->method_Date_getYear) + 1900;
    priv->valuta = g_date_new_dmy(jdate, jmonth, jyear);

    jobject jbdate = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_bdate);
    jdate = (*jni_env)->CallIntMethod(jni_env, jbdate, context->priv->method_Date_getDate);
    jmonth = (*jni_env)->CallIntMethod(jni_env, jbdate, context->priv->method_Date_getMonth) + 1;
    jyear = (*jni_env)->CallIntMethod(jni_env, jbdate, context->priv->method_Date_getYear) + 1900;
    priv->booking_date = g_date_new_dmy(jdate, jmonth, jyear);

    jobject jvalue        = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_value);
    jobject jvalue_string = (*jni_env)->CallObjectMethod(jni_env, jvalue, context->priv->method_Value_toString);
    priv->value = ghbci_statement_jstring_to_cstring(jni_env, jvalue_string);

    jobject jsaldo        = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_saldo);
    jobject jsaldo_value  = (*jni_env)->GetObjectField(jni_env, jsaldo, context->priv->field_Saldo_value);
    jobject jsaldo_string = (*jni_env)->CallObjectMethod(jni_env, jsaldo_value, context->priv->method_Value_toString);
    priv->saldo = ghbci_statement_jstring_to_cstring(jni_env, jsaldo_string);

    jobject jusage = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_usage);
    jobject jiterator = (*jni_env)->CallObjectMethod(jni_env, jusage, context->priv->method_List_iterator);

    GString* reference = g_string_new(NULL);
    while( (*jni_env)->CallObjectMethod(jni_env, jiterator, context->priv->method_Iterator_hasNext) ) {
        jstring jusage_line = (*jni_env)->CallObjectMethod(jni_env, jiterator, context->priv->method_Iterator_next);
        if (jusage_line == NULL) {
            break;
        }

        const char* usage_line = (*jni_env)->GetStringUTFChars(jni_env, jusage_line, 0);
        g_string_append(reference, usage_line);
        (*jni_env)->ReleaseStringUTFChars(jni_env, jusage_line, usage_line);
    }
    priv->reference = g_string_free(reference, FALSE);
    
    jstring jgv_code = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_gvcode);
    priv->gv_code = ghbci_statement_jstring_to_cstring(jni_env, jgv_code);

    jobject other = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_other);
    if (other != NULL) {
        jstring jname = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_name);
        jstring jname2 = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_name2);
        gchar* name = ghbci_statement_jstring_to_cstring(jni_env, jname);
        gchar* name2 = ghbci_statement_jstring_to_cstring(jni_env, jname2);
        priv->other_name = g_strconcat(name, name2, NULL);
        g_free(name);
        g_free(name2);

        jstring jiban = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_number);
        priv->other_iban = ghbci_statement_jstring_to_cstring(jni_env, jiban);

        jstring jbic = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_blz);
        priv->other_bic = ghbci_statement_jstring_to_cstring(jni_env, jbic);
    }

    // fix iban and bic for volksbank
    // TODO rewrite with different lengths of iban and bics in mind
    gint len = strlen (priv->reference);
    if (len > 44) {
        gchar buffer[7];
        g_strlcpy (buffer, priv->reference + len - 44, 7);
        if (g_strcmp0(buffer, "IBAN: ")) {
            g_strlcpy (buffer, priv->reference + len - 16, 7);
            if (g_strcmp0(buffer, "BIC: ")) {
                g_free (priv->other_iban);
                g_free (priv->other_bic);
                priv->other_iban = g_strndup (priv->reference + len - 39, 22);
                priv->other_bic = g_strndup (priv->reference + len - 11, 11);
                priv->reference[len - 46] = '\0';
            }
        }
    }

    return statement;
}



// vim: sw=4 expandtab
