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
    gchar* eref;
    gchar* mref;
    gchar* cred;
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
    PROP_EREF,
    PROP_MREF,
    PROP_CRED,
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
                                                         G_PARAM_READWRITE));

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
                                                         G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));
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
                                                          G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));

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
                                                          G_PARAM_READWRITE));

    /**
     * GHbciStatement:eref
     *
     * end-to-end reference
     **/
    g_object_class_install_property (obj_class,
                                     PROP_EREF,
                                     g_param_spec_string ("eref",
                                                          "end-to-end reference",
                                                          "end-to-end reference",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READWRITE));

    /**
     * GHbciStatement:mref
     *
     * mandate reference
     **/
    g_object_class_install_property (obj_class,
                                     PROP_MREF,
                                     g_param_spec_string ("mref",
                                                          "mandate reference",
                                                          "mandate reference",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READWRITE));

    /**
     * GHbciStatement:cred
     *
     * creditor id
     **/
    g_object_class_install_property (obj_class,
                                     PROP_CRED,
                                     g_param_spec_string ("cred",
                                                          "creditor id",
                                                          "creditor id",
                                                          "not-set" /* default value*/,
                                                          G_PARAM_READWRITE));


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
    priv->eref = NULL;
    priv->mref = NULL;
    priv->cred = NULL;
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
    g_free(priv->eref);
    g_free(priv->mref);
    g_free(priv->cred);

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
    GHbciStatement *self;
    GHbciStatementPrivate *priv;

    self = GHBCI_STATEMENT (obj);
    priv = GHBCI_STATEMENT_GET_PRIVATE (self);

    switch (prop_id)
    {
    case PROP_VALUTA:
        g_free (priv->valuta);
        priv->valuta = g_value_dup_boxed (value);
        break;

    case PROP_BOOKING_DATE:
        g_free (priv->booking_date);
        priv->booking_date = g_value_dup_boxed (value);
        break;

    case PROP_VALUE:
        g_free (priv->value);
        priv->value = g_value_dup_string (value);
        break;

    case PROP_SALDO:
        g_free (priv->saldo);
        priv->saldo = g_value_dup_string (value);
        break;

    case PROP_REFERENCE:
        g_free (priv->reference);
        priv->reference = g_value_dup_string (value);
        break;

    case PROP_GV_CODE:
        g_free (priv->gv_code);
        priv->gv_code = g_value_dup_string (value);
        break;

    case PROP_OTHER_NAME:
        g_free (priv->other_name);
        priv->other_name = g_value_dup_string (value);
        break;

    case PROP_OTHER_IBAN:
        g_free (priv->other_iban);
        priv->other_iban = g_value_dup_string (value);
        break;

    case PROP_OTHER_BIC:
        g_free (priv->other_bic);
        priv->other_bic = g_value_dup_string (value);
        break;

    case PROP_TRANSACTION_TYPE:
        g_free (priv->transaction_type);
        priv->transaction_type = g_value_dup_string (value);
        break;

    case PROP_EREF:
        g_free (priv->eref);
        priv->eref = g_value_dup_string (value);
        break;

    case PROP_MREF:
        g_free (priv->mref);
        priv->mref = g_value_dup_string (value);
        break;

    case PROP_CRED:
        g_free (priv->cred);
        priv->cred = g_value_dup_string (value);
        break;

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

    case PROP_EREF:
        g_value_set_string (value, priv->eref);
        break;

    case PROP_MREF:
        g_value_set_string (value, priv->mref);
        break;

    case PROP_CRED:
        g_value_set_string (value, priv->cred);
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

        g_string_append(reference, usage_line);

        if (strlen(usage_line) < 27) {
            g_string_append(reference, " ");
        }

        g_string_append(reference, "\n");

        (*jni_env)->ReleaseStringUTFChars(jni_env, jusage_line, usage_line);
        (*jni_env)->DeleteLocalRef(jni_env, jusage_line);
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
ghbci_statement_remove_newlines(gchar* str)
{
    gint i = 0;
    gint skipped = 0;
    while(str[i] != '\0') {
        if (str[i] == '\n') {
            skipped++;
        } else {
            str[i - skipped] = str[i];
        }
        i++;
    }
    str[i - skipped] = str[i];
}

void
ghbci_statement_prettify_statement (GObject* statement)
{
    gchar *reference;
    g_object_get(statement,
                 "reference", &reference,
                 NULL);

    // extract SEPA fields from reference (ING DiBa)
    while (reference[4] == '+') {
        if (g_str_has_prefix(reference, "EREF+")
                || g_str_has_prefix(reference, "MREF+")
                || g_str_has_prefix(reference, "CRED+")) {

            // detemine field size
            gint end = 4;
            while (reference[end] != '\n')
                ++end;
            ++end;
            while (reference[end] != '\n')
                ++end;

            gchar* value = g_strndup(reference + 5, end - 5);
            ghbci_statement_remove_newlines(value);
            g_strstrip(value);

            // save field
            if (g_str_has_prefix(reference, "EREF+"))
                g_object_set(statement, "eref", value, NULL);
            else if (g_str_has_prefix(reference, "MREF+"))
                g_object_set(statement, "mref", value, NULL);
            else if (g_str_has_prefix(reference, "CRED+"))
                g_object_set(statement, "cred", value, NULL);
            g_free(value);

            // remove field
            memmove(reference, reference + end + 1, strlen(reference + end + 1) + 1);

        } else if (g_str_has_prefix(reference, "SVWZ+")) {
            // always last field
            memmove(reference, reference + 5, strlen(reference + 5) + 1);
            // remove newlines, they never make sense in SEPA fields
            ghbci_statement_remove_newlines(reference);
            break;

        } else {
            // doesn't seem to be a SEPA field, don't look for further fields
            break;
        }
    }

    // volksbank way
    ghbci_statement_remove_newlines(reference);
    g_strstrip(reference);
    gchar** words = g_strsplit_set(reference, " ", -1);
    gint i = g_strv_length(words) - 2;
    while (i >= 0) {
        if (g_strcmp0(words[i], "BIC:") == 0) {
            g_object_set(statement, "other-bic", words[i + 1], NULL);
            g_free(words[i + 1]);
            g_free(words[i]);
            words[i] = NULL;
            i -= 2;
        } else if (g_strcmp0(words[i], "IBAN:") == 0) {
            g_object_set(statement, "other-iban", words[i + 1], NULL);
            g_free(words[i + 1]);
            g_free(words[i]);
            words[i] = NULL;
            i -= 2;
        } else if (g_strcmp0(words[i], "CRED:") == 0) {
            g_object_set(statement, "cred", words[i + 1], NULL);
            g_free(words[i + 1]);
            g_free(words[i]);
            words[i] = NULL;
            i -= 2;
        } else if (g_strcmp0(words[i], "MREF:") == 0) {
            g_object_set(statement, "mref", words[i + 1], NULL);
            g_free(words[i + 1]);
            g_free(words[i]);
            words[i] = NULL;
            i -= 2;
        } else if (g_strcmp0(words[i], "EREF:") == 0) {
            g_object_set(statement, "eref", words[i + 1], NULL);
            g_free(words[i + 1]);
            g_free(words[i]);
            words[i] = NULL;
            i -= 2;
        } else {
            break;
        }
    }
    g_free(reference);
    gchar* result = g_strjoinv(" ", words);
    reference = result;
    g_strfreev(words);

    g_object_set(statement, "reference", reference, NULL);
}


// vim: sw=4 expandtab
