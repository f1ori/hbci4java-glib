/*
 * ghbci-statement.c
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

/**
 * SECTION:ghbci-statement
 * @short_description: bank statement
 *
 * Wraps a single bank statement
 *
 * Attributes of hbci4java UmsLine object are extracted and unified across
 * banks 
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
    gchar* transaction_type;
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
    PROP_TRANSACTION_TYPE,
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
     *
     * date, transaction accounted for value in bank account
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
     *
     * date transaction was booked and appeared in statements
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

    /**
     * GHbciStatement:transaction-type
     *
     * type of transaction
     **/
    g_object_class_install_property (obj_class,
                                     PROP_TRANSACTION_TYPE,
                                     g_param_spec_string ("transaction-type",
                                                          "type of transaction",
                                                          "type of transaction",
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
    priv->transaction_type = NULL;
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
    g_free(priv->transaction_type);

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

    case PROP_TRANSACTION_TYPE:
        g_value_set_string (value, priv->transaction_type);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
        return;
    }
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
    (*jni_env)->DeleteLocalRef(jni_env, jvaluta);

    jobject jbdate = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_bdate);
    jdate = (*jni_env)->CallIntMethod(jni_env, jbdate, context->priv->method_Date_getDate);
    jmonth = (*jni_env)->CallIntMethod(jni_env, jbdate, context->priv->method_Date_getMonth) + 1;
    jyear = (*jni_env)->CallIntMethod(jni_env, jbdate, context->priv->method_Date_getYear) + 1900;
    priv->booking_date = g_date_new_dmy(jdate, jmonth, jyear);
    (*jni_env)->DeleteLocalRef(jni_env, jbdate);

    jobject jvalue        = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_value);
    jobject jvalue_string = (*jni_env)->CallObjectMethod(jni_env, jvalue, context->priv->method_Value_toString);
    priv->value = ghbci_statement_jstring_to_cstring(jni_env, jvalue_string);
    (*jni_env)->DeleteLocalRef(jni_env, jvalue_string);
    (*jni_env)->DeleteLocalRef(jni_env, jvalue);

    jobject jsaldo        = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_saldo);
    jobject jsaldo_value  = (*jni_env)->GetObjectField(jni_env, jsaldo, context->priv->field_Saldo_value);
    jobject jsaldo_string = (*jni_env)->CallObjectMethod(jni_env, jsaldo_value, context->priv->method_Value_toString);
    priv->saldo = ghbci_statement_jstring_to_cstring(jni_env, jsaldo_string);
    (*jni_env)->DeleteLocalRef(jni_env, jsaldo_string);
    (*jni_env)->DeleteLocalRef(jni_env, jsaldo_value);
    (*jni_env)->DeleteLocalRef(jni_env, jsaldo);

    jobject jusage = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_usage);
    jobject jiterator = (*jni_env)->CallObjectMethod(jni_env, jusage, context->priv->method_List_iterator);

    GString* reference = g_string_new(NULL);
    while( (*jni_env)->CallBooleanMethod(jni_env, jiterator, context->priv->method_Iterator_hasNext) ) {
        jstring jusage_line = (*jni_env)->CallObjectMethod(jni_env, jiterator, context->priv->method_Iterator_next);
        if (jusage_line == NULL) {
            break;
        }

        const char* usage_line = (*jni_env)->GetStringUTFChars(jni_env, jusage_line, 0);
        gchar* usage_line_without_whitespace = g_strdup(usage_line);
        g_strchomp(usage_line_without_whitespace);

        g_string_append(reference, usage_line_without_whitespace);

        if (g_strcmp0(usage_line_without_whitespace, "") != 0) {
            g_string_append(reference, "\n");
        }

        (*jni_env)->ReleaseStringUTFChars(jni_env, jusage_line, usage_line);
        (*jni_env)->DeleteLocalRef(jni_env, jusage_line);
        g_free(usage_line_without_whitespace);
    }
    priv->reference = g_string_free(reference, FALSE);
    (*jni_env)->DeleteLocalRef(jni_env, jiterator);
    (*jni_env)->DeleteLocalRef(jni_env, jusage);
    
    jstring jgv_code = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_gvcode);
    priv->gv_code = ghbci_statement_jstring_to_cstring(jni_env, jgv_code);
    (*jni_env)->DeleteLocalRef(jni_env, jgv_code);

    jobject other = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_other);
    if (other != NULL) {
        jstring jname = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_name);
        jstring jname2 = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_name2);
        gchar* name = ghbci_statement_jstring_to_cstring(jni_env, jname);
        gchar* name2 = ghbci_statement_jstring_to_cstring(jni_env, jname2);
        (*jni_env)->DeleteLocalRef(jni_env, jname2);
        (*jni_env)->DeleteLocalRef(jni_env, jname);

        priv->other_name = g_strconcat(name, name2, NULL);
        g_free(name);
        g_free(name2);

        jstring jiban = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_number);
        priv->other_iban = ghbci_statement_jstring_to_cstring(jni_env, jiban);
        (*jni_env)->DeleteLocalRef(jni_env, jiban);

        jstring jbic = (*jni_env)->GetObjectField(jni_env, other, context->priv->field_Konto_blz);
        priv->other_bic = ghbci_statement_jstring_to_cstring(jni_env, jbic);
        (*jni_env)->DeleteLocalRef(jni_env, jbic);

        (*jni_env)->DeleteLocalRef(jni_env, other);
    }

    jstring jtransaction_type = (*jni_env)->GetObjectField(jni_env, jstatement, context->priv->field_GVRKUmsUmsLine_text);
    priv->transaction_type = ghbci_statement_jstring_to_cstring(jni_env, jtransaction_type);
    (*jni_env)->DeleteLocalRef(jni_env, jtransaction_type);

    return statement;
}

void
ghbci_statement_prettify_statement (GObject* statement)
{
    gchar *reference;
    gchar *other_iban;
    gchar *other_bic;
    g_object_get(statement,
                 "reference", &reference,
                 "other-iban", &other_iban,
                 "other-bic", &other_bic,
                 NULL);

    // fix iban and bic for volksbank
    // TODO rewrite with different lengths of iban and bics in mind
    gint len = strlen (reference);
    if (len > 44) {
        gchar buffer[7];
        g_strlcpy (buffer, reference + len - 44, 7);
        if (g_strcmp0(buffer, "IBAN: ") == 0) {
            g_strlcpy (buffer, reference + len - 16, 7);
            if (g_strcmp0(buffer, "BIC: ") == 0) {
                g_free (other_iban);
                g_free (other_bic);
                other_iban = g_strndup (reference + len - 39, 22);
                other_bic = g_strndup (reference + len - 11, 11);
                reference[len - 46] = '\0';
            }
        }
    }

    // extract SEPA fields from reference
    while (reference[4] == '+') {
        if (g_str_has_prefix(reference, "EREF+")
                || g_str_has_prefix(reference, "MREF+")
                || g_str_has_prefix(reference, "CRED+")) {

            // detemine field size
            gint end = 4;
            while (!g_ascii_isspace(reference[end]))
                ++end;

            // save field
            gchar* value = g_strndup(reference + 5, end - 5);
            if (g_str_has_prefix(reference, "EREF+"))
                g_object_set(statement, "eref", value, NULL);
            else if (g_str_has_prefix(reference, "MREF+"))
                g_object_set(statement, "mref", value, NULL);
            else if (g_str_has_prefix(reference, "CRED+"))
                g_object_set(statement, "cred", value, NULL);

            // remove field
            memmove(reference, reference + end + 1, strlen(reference + end + 1) + 1);

        } else if (g_str_has_prefix(reference, "SVWZ+")) {
            // always last field
            memmove(reference, reference + 5, strlen(reference + 5) + 1);
            // remove newlines, they never make sense in SEPA fields
            for (gint i = 0; reference[i] != '\0'; ++i) {
                if (reference[i] == '\n') {
                    memmove(reference + i, reference + i + 1, strlen(reference + i + 1) + 1);
                }
            }
            break;

        } else {
            // doesn't seem to be a SEPA field, don't look for further fields
            break;
        }
    }

    g_object_set(statement,
                 "reference", reference,
                 "other-iban", other_iban,
                 "other-bic", other_bic,
                 NULL);
}


// vim: sw=4 expandtab
