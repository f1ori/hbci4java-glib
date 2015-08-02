/*
 * ghbci-context-private.h
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

#ifndef __GHBCI_CONTEXT_PRIVATE_H__
#define __GHBCI_CONTEXT_PRIVATE_H__

#include <glib.h>
#include <glib-object.h>


#define GHBCI_CONTEXT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                           GHBCI_TYPE_CONTEXT, \
                                           GHbciContextPrivate))

/* private data */
struct _GHbciContextPrivate
{
    GMainContext *glib_context;

    GHashTable* hbci_handlers;
    GHashTable* accounts;

    JavaVM* jvm;
    JNIEnv* jni_env;
    jclass class_Konto;
    jclass class_Saldo;
    jclass class_Value;
    jclass class_AbortException;
    jclass class_HBCIUtilsInternal;
    jclass class_HBCIUtils;
    jclass class_HBCICallbackConsole;
    jclass class_HBCICallbackNative;
    jclass class_HBCIHandler;
    jclass class_HBCIJob;
    jclass class_HBCIJobResult;
    jclass class_HBCIStatus;
    jclass class_HBCIPassport;
    jclass class_AbstractHBCIPassport;
    jclass class_HBCIJobResultImpl;
    jclass class_GVRSaldoReq;
    jclass class_GVRSaldoReqInfo;
    jclass class_GVRKUms;
    jclass class_GVRKUmsUmsLine;
    jclass class_Hashtable;
    jclass class_Properties;
    jclass class_Enumeration;
    jclass class_Iterator;
    jclass class_List;
    jclass class_StringBuffer;
    jclass class_Date;
    jmethodID method_HBCIUtils_getNameForBLZ;
    jmethodID method_HBCIUtils_getPinTanURLForBLZ;
    jmethodID method_HBCIUtils_init;
    jmethodID method_HBCIUtils_setParam;
    jmethodID method_HBCIHandler_constructor;
    jmethodID method_HBCIHandler_newJob;
    jmethodID method_HBCIHandler_execute;
    jmethodID method_HBCIHandler_getPassport;
    jmethodID method_HBCIHandler_reset;
    jmethodID method_HBCICallbackConsole_constructor;
    jmethodID method_HBCICallbackNative_constructor;
    jmethodID method_HBCIJob_setParam;
    jmethodID method_HBCIJob_addToQueue;
    jmethodID method_HBCIJob_getJobResult;
    jmethodID method_HBCIJobResult_getJobStatus;
    jmethodID method_HBCIStatus_getErrorString;
    jmethodID method_HBCIPassport_getAccounts;
    jmethodID method_AbstractHBCIPassport_getInstance;
    jmethodID method_HBCIJobResultImpl_isOK;
    jmethodID method_GVRSaldoReq_getEntries;
    jmethodID method_GVRKUms_toString;
    jmethodID method_GVRKUms_getFlatData;
    jmethodID method_Konto_constructor;
    jmethodID method_Value_toString;
    jmethodID method_Properties_keys;
    jmethodID method_Enumeration_hasMoreElements;
    jmethodID method_Enumeration_nextElement;
    jmethodID method_Iterator_hasNext;
    jmethodID method_Iterator_next;
    jmethodID method_List_iterator;
    jmethodID method_StringBuffer_replace;
    jmethodID method_StringBuffer_setLength;
    jmethodID method_StringBuffer_toString;
    jmethodID method_Date_toString;
    jmethodID method_Date_getDate;
    jmethodID method_Date_getMonth;
    jmethodID method_Date_getYear;
    jmethodID method_Date_getTime;
    jfieldID field_HBCIUtilsInternal_blzs;
    jfieldID field_GVRSaldoReqInfo_ready;
    jfieldID field_Saldo_value;
    jfieldID field_Konto_country;
    jfieldID field_Konto_blz;
    jfieldID field_Konto_number;
    jfieldID field_Konto_subnumber;
    jfieldID field_Konto_acctype;
    jfieldID field_Konto_type;
    jfieldID field_Konto_curr;
    jfieldID field_Konto_customerid;
    jfieldID field_Konto_name;
    jfieldID field_Konto_name2;
    jfieldID field_Konto_bic;
    jfieldID field_Konto_iban;
    jfieldID field_GVRKUmsUmsLine_valuta;
    jfieldID field_GVRKUmsUmsLine_bdate;
    jfieldID field_GVRKUmsUmsLine_value;
    jfieldID field_GVRKUmsUmsLine_saldo;
    jfieldID field_GVRKUmsUmsLine_gvcode;
    jfieldID field_GVRKUmsUmsLine_usage;
    jfieldID field_GVRKUmsUmsLine_other;
    jfieldID field_GVRKUmsUmsLine_text;
};

#endif /* __GHBCI_CONTEXT_PRIVATE_H__ */

