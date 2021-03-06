/*
 * ghbci-context.c
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
 * SECTION:ghbci-context
 * @short_description: Context object for all hbci4java actions
 *
 * This object is needed for all actions involving hbci4java. You have to
 * provide callbacks at least for the signal #callback. This gets triggered,
 * whenever hbci4java needs user-input like account information or credentials.
 *
 * On initialization, you have to specify a passport-file, which caches bank
 * account parameters. Before using a new bank account, it has to be
 * registered using ghbci_context_add_passport().
 *
 * Internally, this object sets up a java virtual machine with all necessary
 * references to java classes, methods and fields and initializes hbci4java.
 **/

#include <jni.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "ghbci-context.h"
#include "ghbci-context-private.h"
#include "ghbci-account.h"
#include "ghbci-account-private.h"
#include "ghbci-statement.h"
#include "ghbci-statement-private.h"
#include "ghbci-marshal.h"


/* signals */
enum
{
    CALLBACK,
    LOG,
    STATUS,
    LAST_SIGNAL
};

static guint ghbci_context_signals [LAST_SIGNAL] = { 0 };


static void     ghbci_context_class_init         (GHbciContextClass *class);
static void     ghbci_context_init               (GHbciContext *self);
static void     ghbci_context_finalize           (GObject *obj);
static void     ghbci_context_dispose            (GObject *obj);

G_DEFINE_TYPE (GHbciContext, ghbci_context, G_TYPE_OBJECT)


static void
ghbci_context_class_init (GHbciContextClass *class)
{
    GObjectClass *obj_class;

    obj_class = G_OBJECT_CLASS (class);

    obj_class->dispose = ghbci_context_dispose;
    obj_class->finalize = ghbci_context_finalize;

    /* install signals */

    /**
     * GHbciContext::callback:
     * @self: The #GHbciContext
     * @reason: reason code
     * @msg: message
     * @optional: optional string (depends on reason)
     *
     * Called when hbci4java needs additional input, like account information
     * or credentials
     *
     * Returns: (transfer full): answer, if required by reason
     **/
    ghbci_context_signals[CALLBACK] =
        g_signal_new ("callback",
              G_TYPE_FROM_CLASS (obj_class),
              G_SIGNAL_RUN_LAST,
              0,
              NULL /* accumulator */,
              NULL /* accumulator data */,
              ghbci_marshal_STRING__INT64_STRING_STRING,
              G_TYPE_STRING /* return_type */,
              3 /* n_params */,
              G_TYPE_INT64, G_TYPE_STRING, G_TYPE_STRING, NULL/* param_types */);

    /**
     * GHbciContext::log:
     * @self: The #GHbciContext
     * @msg: log message
     * @level: log level
     *
     * Called when hbci4java logs something
     **/
    ghbci_context_signals[LOG] =
        g_signal_new ("log",
              G_TYPE_FROM_CLASS (obj_class),
              G_SIGNAL_RUN_LAST,
              0,
              NULL /* accumulator */,
              NULL /* accumulator data */,
              ghbci_marshal_VOID__STRING_INT64,
              G_TYPE_NONE /* return_type */,
              2 /* n_params */,
              G_TYPE_STRING, G_TYPE_INT64, NULL/* param_types */);

    /**
     * GHbciContext::status:
     * @self: The #GHbciContext
     * @status: type
     * @obj: list of objects
     *
     * Called when hbci4java logs something
     **/
    ghbci_context_signals[STATUS] =
        g_signal_new ("status",
              G_TYPE_FROM_CLASS (obj_class),
              G_SIGNAL_RUN_LAST,
              0,
              NULL /* accumulator */,
              NULL /* accumulator data */,
              ghbci_marshal_VOID__INT64_STRING,
              G_TYPE_NONE /* return_type */,
              2 /* n_params */,
              G_TYPE_INT64, G_TYPE_STRING, NULL/* param_types */);


    /* add private structure */
    g_type_class_add_private (obj_class, sizeof (GHbciContextPrivate));
}

static void
ghbci_context_init (GHbciContext *self)
{
    GHbciContextPrivate *priv;

    priv = GHBCI_CONTEXT_GET_PRIVATE (self);
    self->priv = priv;

    priv->hbci_handlers = NULL;
    priv->accounts = NULL;
    priv->passport_directory = NULL;
    priv->passports = NULL;

    priv->jvm = NULL;
    priv->jni_env = NULL;
    priv->class_Konto = NULL;
    priv->class_Saldo = NULL;
    priv->class_Value = NULL;
    priv->class_AbortException = NULL;
    priv->class_HBCIUtilsInternal = NULL;
    priv->class_HBCIUtils = NULL;
    priv->class_HBCICallbackConsole = NULL;
    priv->class_HBCICallbackNative = NULL;
    priv->class_HBCIHandler = NULL;
    priv->class_HBCIJob = NULL;
    priv->class_HBCIJobResult = NULL;
    priv->class_HBCIStatus = NULL;
    priv->class_HBCIPassport = NULL;
    priv->class_AbstractHBCIPassport = NULL;
    priv->class_AbstractPinTanPassport = NULL;
    priv->class_HBCIJobResultImpl = NULL;
    priv->class_GVRSaldoReq = NULL;
    priv->class_GVRSaldoReqInfo = NULL;
    priv->class_GVRKUms = NULL;
    priv->class_GVRKUmsUmsLine = NULL;
    priv->class_Hashtable = NULL;
    priv->class_Properties = NULL;
    priv->class_Enumeration = NULL;
    priv->class_Iterator = NULL;
    priv->class_List = NULL;
    priv->class_StringBuffer = NULL;
    priv->class_Date = NULL;
    priv->method_HBCIUtils_getNameForBLZ = NULL;
    priv->method_HBCIUtils_getPinTanURLForBLZ = NULL;
    priv->method_HBCIUtils_init = NULL;
    priv->method_HBCIUtils_setParam = NULL;
    priv->method_HBCIHandler_constructor = NULL;
    priv->method_HBCIHandler_newJob = NULL;
    priv->method_HBCIHandler_execute = NULL;
    priv->method_HBCIHandler_getPassport = NULL;
    priv->method_HBCIHandler_reset = NULL;
    priv->method_HBCICallbackConsole_constructor = NULL;
    priv->method_HBCICallbackNative_constructor = NULL;
    priv->method_HBCIJob_setParam = NULL;
    priv->method_HBCIJob_addToQueue = NULL;
    priv->method_HBCIJob_getJobResult = NULL;
    priv->method_HBCIJobResult_getJobStatus = NULL;
    priv->method_HBCIStatus_getErrorString = NULL;
    priv->method_HBCIPassport_getAccounts = NULL;
    priv->method_AbstractHBCIPassport_getInstance = NULL;
    priv->method_AbstractPinTanPassport_getTwostepMechanisms = NULL;
    priv->method_AbstractPinTanPassport_getAllowedTwostepMechanisms = NULL;
    priv->method_AbstractPinTanPassport_setCurrentTANMethod = NULL;
    priv->method_HBCIJobResultImpl_isOK = NULL;
    priv->method_Konto_constructor = NULL;
    priv->method_Value_toString = NULL;
    priv->method_Properties_keys = NULL;
    priv->method_Properties_getProperty = NULL;
    priv->method_Hashtable_toString = NULL;
    priv->method_Hashtable_get = NULL;
    priv->method_Enumeration_hasMoreElements = NULL;
    priv->method_Enumeration_nextElement = NULL;
    priv->method_Iterator_hasNext = NULL;
    priv->method_Iterator_next = NULL;
    priv->method_List_iterator = NULL;
    priv->method_List_contains = NULL;
    priv->method_StringBuffer_replace = NULL;
    priv->method_StringBuffer_setLength = NULL;
    priv->method_StringBuffer_toString = NULL;
    priv->method_Date_toString = NULL;
    priv->method_Date_getDate = NULL;
    priv->method_Date_getMonth = NULL;
    priv->method_Date_getYear = NULL;
    priv->method_Date_getTime = NULL;
    priv->field_HBCIUtilsInternal_blzs = NULL;
    priv->field_Konto_country = NULL;
    priv->field_Konto_blz = NULL;
    priv->field_Konto_number = NULL;
    priv->field_Konto_subnumber = NULL;
    priv->field_Konto_acctype = NULL;
    priv->field_Konto_type = NULL;
    priv->field_Konto_curr = NULL;
    priv->field_Konto_customerid = NULL;
    priv->field_Konto_name = NULL;
    priv->field_Konto_name2 = NULL;
    priv->field_Konto_bic = NULL;
    priv->field_Konto_iban = NULL;
    priv->field_GVRKUmsUmsLine_valuta = NULL;
    priv->field_GVRKUmsUmsLine_bdate = NULL;
    priv->field_GVRKUmsUmsLine_value = NULL;
    priv->field_GVRKUmsUmsLine_saldo = NULL;
    priv->field_GVRKUmsUmsLine_gvcode = NULL;
    priv->field_GVRKUmsUmsLine_usage = NULL;
    priv->field_GVRKUmsUmsLine_other = NULL;
    priv->field_GVRKUmsUmsLine_text = NULL;
}

static void
ghbci_context_dispose (GObject *obj)
{
    GHbciContext *self = GHBCI_CONTEXT (obj);

    if (self->priv->jvm != NULL) {
        (*self->priv->jvm)->DestroyJavaVM(self->priv->jvm); 
        self->priv->jvm = NULL;
    }

    GSList* iter = self->priv->passports;
    while(iter != NULL) {
        GFile* file = g_file_new_for_path (iter->data);
        g_debug("delete file: %s", (gchar*)iter->data);
        g_file_delete(file, NULL, NULL);
        g_object_unref(file);
        iter = g_slist_next(iter);
    }
    g_slist_free_full(self->priv->passports, g_free);
    g_free(self->priv->passport_directory);

    G_OBJECT_CLASS (ghbci_context_parent_class)->dispose (obj);
}

static void
ghbci_context_finalize (GObject *obj)
{
  //GHbciContext *self = GHBCI_CONTEXT (obj);

  G_OBJECT_CLASS (ghbci_context_parent_class)->finalize (obj);
}

/*
 * native implementation for log events
 */
void my_log(JNIEnv *jni_env, jobject this, jstring jmsg, jint level, jobject date, jobject trace)
{
    GHbciContext* context = (*jni_env)->reserved3;

    const char *msg = (*jni_env)->GetStringUTFChars(jni_env, jmsg, NULL);
    g_signal_emit (context, ghbci_context_signals[LOG], 0, msg, level);
    (*jni_env)->ReleaseStringUTFChars(jni_env, jmsg, msg);
}

/*
 * native implementation for generic callbacks
 */
void my_callback(JNIEnv *jni_env, jobject this, jobject passport, jint reason, jstring jmsg, jint datatype, jobject retData)
{
    GHbciContext* context = (*jni_env)->reserved3;

    // retrieve optional argument from string buffer 
    jstring joptional = (*jni_env)->CallObjectMethod(jni_env, retData, context->priv->method_StringBuffer_toString);

    // emit signal (convert j* to native string)
    const char *msg = (*jni_env)->GetStringUTFChars(jni_env, jmsg, NULL);
    const char *optional = (*jni_env)->GetStringUTFChars(jni_env, joptional, NULL);
    gchar* retvalue;
    g_signal_emit (context, ghbci_context_signals[CALLBACK], 0, reason, msg, optional, &retvalue);
    (*jni_env)->ReleaseStringUTFChars(jni_env, jmsg, msg);
    (*jni_env)->ReleaseStringUTFChars(jni_env, joptional, optional);

    // return result as StringBuffer in parameter retData
    (*jni_env)->CallObjectMethod(jni_env, retData, context->priv->method_StringBuffer_setLength, 0);
    if (retvalue != NULL) {
        jstring jretvalue = (*jni_env)->NewStringUTF(jni_env, retvalue);
        (*jni_env)->CallObjectMethod(jni_env, retData, context->priv->method_StringBuffer_replace, 0, strlen(retvalue), jretvalue);
        (*jni_env)->DeleteLocalRef(jni_env, jretvalue);
    } else {
        (*jni_env)->ThrowNew(jni_env, context->priv->class_AbortException, "Aborted by User");
    }

    g_free(retvalue);
}

/*
 * native implementation for status events
 */
void my_status(JNIEnv *jni_env, jobject this, jobject passport, jint statusTag, jarray o)
{
    GHbciContext* context = (*jni_env)->reserved3;
    //const char *msg = (*jni_env)->GetStringUTFChars(jni_env, jmsg, NULL);
    g_signal_emit (context, ghbci_context_signals[STATUS], 0, statusTag, "");
    //(*jni_env)->ReleaseStringUTFChars(jni_env, jmsg, msg);
}

/*
 * Helper to fetch HBCIHandler-object from internal cache
 */
jobject get_hbci_handler(GHbciContext* self, const gchar* blz, const gchar* userid) {
    GHbciContextPrivate* priv;
    jobject hbci_handler;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), NULL);
    priv = self->priv;

    // get HBCIHandler from hashtable
    char* key = g_strconcat(blz, "+", userid, NULL);
    hbci_handler = g_hash_table_lookup(priv->hbci_handlers, key);

    g_free(key);
    return hbci_handler;
}

/*
 * Helper to fetch Konto-object from internal cache
 */
jobject get_account(GHbciContext* self, const gchar* blz, const gchar* userid, const gchar* number) {
    GHbciContextPrivate* priv;
    jobject account;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), NULL);
    priv = self->priv;

    char* key = g_strconcat(blz, "+", userid, "+", number, NULL);
    account = g_hash_table_lookup(priv->accounts, key);

    g_free(key);
    return account;
}


/* public methods */

/**
 * ghbci_context_new: (constructor)
 * @directory: temporary directory to save passports
 *
 * Sets up a new #GHbciContext object
 *
 * Returns: (transfer full): A New #GHbciContext
 **/
GHbciContext*
ghbci_context_new (const gchar* directory)
{
    GHbciContext* context;
    GHbciContextPrivate* priv;
    JavaVMInitArgs vm_args;
    JavaVMOption options; 
    JNINativeMethod methods[3];
    jobject console;

    context = g_object_new (GHBCI_TYPE_CONTEXT, NULL);
    priv = context->priv;

    priv->hbci_handlers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    priv->accounts      = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    priv->passport_directory = g_strdup(directory);

    // initialize java virtual machine
    // Path to hbci4java.jar
    options.optionString = "-Djava.class.path=" DATA_DIR "/hbci4java.jar"; 
    vm_args.version = JNI_VERSION_1_6; //JDK version. This indicates version 1.6
    vm_args.nOptions = 1;
    vm_args.options = &options;
    vm_args.ignoreUnrecognized = 0;

    int ret = JNI_CreateJavaVM(&priv->jvm, (void**)&priv->jni_env, &vm_args);
    if(ret < 0) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return NULL;
    }
    // pure evil, save context object in reserved field of jni environment struct,
    // so it can be accessed from native callback functions
    (*(struct JNINativeInterface_**)priv->jni_env)->reserved3 = context;

    // save references to java classes and methods
#define defineJavaClass(class, path) \
    priv->class_##class = (*priv->jni_env)->FindClass(priv->jni_env, path);\
    if (priv->class_##class == NULL) { \
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env); \
        return NULL; \
    }

    defineJavaClass(Konto, "org/kapott/hbci/structures/Konto")
    defineJavaClass(Saldo, "Lorg/kapott/hbci/structures/Saldo;")
    defineJavaClass(Value, "Lorg/kapott/hbci/structures/Value;")
    defineJavaClass(AbortException, "org/kapott/hbci/exceptions/AbortedException")
    defineJavaClass(HBCIUtilsInternal, "org/kapott/hbci/manager/HBCIUtilsInternal")
    defineJavaClass(HBCIUtils, "org/kapott/hbci/manager/HBCIUtils")
    defineJavaClass(HBCICallbackConsole, "org/kapott/hbci/callback/HBCICallbackConsole")
    defineJavaClass(HBCICallbackNative, "org/kapott/hbci/callback/HBCICallbackNative")
    defineJavaClass(HBCIHandler, "org/kapott/hbci/manager/HBCIHandler")
    defineJavaClass(HBCIJob, "org/kapott/hbci/GV/HBCIJob")
    defineJavaClass(HBCIJobResult, "org/kapott/hbci/GV_Result/HBCIJobResult")
    defineJavaClass(HBCIStatus, "org/kapott/hbci/status/HBCIStatus")
    defineJavaClass(HBCIPassport, "org/kapott/hbci/passport/HBCIPassport")
    defineJavaClass(AbstractHBCIPassport, "org/kapott/hbci/passport/AbstractHBCIPassport")
    defineJavaClass(AbstractPinTanPassport, "org/kapott/hbci/passport/AbstractPinTanPassport")
    defineJavaClass(HBCIJobResultImpl, "org/kapott/hbci/GV_Result/HBCIJobResultImpl")
    defineJavaClass(GVRSaldoReq, "org/kapott/hbci/GV_Result/GVRSaldoReq")
    defineJavaClass(GVRSaldoReqInfo, "org/kapott/hbci/GV_Result/GVRSaldoReq$Info")
    defineJavaClass(GVRKUms, "org/kapott/hbci/GV_Result/GVRKUms")
    defineJavaClass(GVRKUmsUmsLine, "org/kapott/hbci/GV_Result/GVRKUms$UmsLine")
    defineJavaClass(Hashtable, "java/util/Hashtable")
    defineJavaClass(Properties, "java/util/Properties")
    defineJavaClass(Enumeration, "java/util/Enumeration")
    defineJavaClass(Iterator, "java/util/Iterator")
    defineJavaClass(List, "java/util/List")
    defineJavaClass(StringBuffer, "java/lang/StringBuffer")
    defineJavaClass(Date, "java/util/Date")

#define defineJavaStaticMethod(class, method, signatur) \
    priv->method_##class##_##method = (*priv->jni_env)->GetStaticMethodID(priv->jni_env, priv->class_##class, #method, signatur); \
    if (priv->method_##class##_##method == NULL) { \
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env); \
        return NULL; \
    }

    defineJavaStaticMethod(HBCIUtils, getNameForBLZ, "(Ljava/lang/String;)Ljava/lang/String;")
    defineJavaStaticMethod(HBCIUtils, getPinTanURLForBLZ, "(Ljava/lang/String;)Ljava/lang/String;")
    defineJavaStaticMethod(HBCIUtils, init, "(Ljava/util/Properties;Lorg/kapott/hbci/callback/HBCICallback;)V")
    defineJavaStaticMethod(HBCIUtils, setParam, "(Ljava/lang/String;Ljava/lang/String;)V")
    defineJavaStaticMethod(AbstractHBCIPassport, getInstance, "(Ljava/lang/String;)Lorg/kapott/hbci/passport/HBCIPassport;")

#define defineJavaMethod(class, method, signatur) \
    priv->method_##class##_##method = (*priv->jni_env)->GetMethodID(priv->jni_env, priv->class_##class, #method, signatur); \
    if (priv->method_##class##_##method == NULL) { \
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env); \
        return NULL; \
    }
    defineJavaMethod(HBCIHandler, newJob, "(Ljava/lang/String;)Lorg/kapott/hbci/GV/HBCIJob;")
    defineJavaMethod(HBCIHandler, execute, "()Lorg/kapott/hbci/status/HBCIExecStatus;")
    defineJavaMethod(HBCIHandler, getPassport, "()Lorg/kapott/hbci/passport/HBCIPassport;")
    defineJavaMethod(HBCIHandler, reset, "()V")
    defineJavaMethod(HBCIJob, setParam, "(Ljava/lang/String;Ljava/lang/String;)V")
    defineJavaMethod(HBCIJob, addToQueue, "()V")
    defineJavaMethod(HBCIJob, getJobResult, "()Lorg/kapott/hbci/GV_Result/HBCIJobResult;")
    defineJavaMethod(HBCIJobResult, getJobStatus, "()Lorg/kapott/hbci/status/HBCIStatus;")
    defineJavaMethod(HBCIStatus, getErrorString, "()Ljava/lang/String;")
    defineJavaMethod(HBCIPassport, getAccounts, "()[Lorg/kapott/hbci/structures/Konto;")
    defineJavaMethod(HBCIJobResultImpl, isOK, "()Z")
    defineJavaMethod(AbstractPinTanPassport, getTwostepMechanisms, "()Ljava/util/Hashtable;")
    defineJavaMethod(AbstractPinTanPassport, getAllowedTwostepMechanisms, "()Ljava/util/List;")
    defineJavaMethod(AbstractPinTanPassport, setCurrentTANMethod, "(Ljava/lang/String;)V")
    defineJavaMethod(GVRSaldoReq, getEntries, "()[Lorg/kapott/hbci/GV_Result/GVRSaldoReq$Info;")
    defineJavaMethod(GVRKUms, toString, "()Ljava/lang/String;")
    defineJavaMethod(GVRKUms, getFlatData, "()Ljava/util/List;")
    defineJavaMethod(Properties, keys, "()Ljava/util/Enumeration;")
    defineJavaMethod(Properties, getProperty, "(Ljava/lang/String;)Ljava/lang/String;")
    defineJavaMethod(Enumeration, hasMoreElements, "()Z")
    defineJavaMethod(Enumeration, nextElement, "()Ljava/lang/Object;")
    defineJavaMethod(Iterator, hasNext, "()Z")
    defineJavaMethod(Iterator, next, "()Ljava/lang/Object;")
    defineJavaMethod(List, iterator, "()Ljava/util/Iterator;")
    defineJavaMethod(List, contains, "(Ljava/lang/Object;)Z")
    defineJavaMethod(StringBuffer, replace, "(IILjava/lang/String;)Ljava/lang/StringBuffer;")
    defineJavaMethod(StringBuffer, setLength, "(I)V")
    defineJavaMethod(StringBuffer, toString, "()Ljava/lang/String;")
    defineJavaMethod(Hashtable, toString, "()Ljava/lang/String;")
    defineJavaMethod(Hashtable, get, "(Ljava/lang/Object;)Ljava/lang/Object;")
    defineJavaMethod(Value, toString, "()Ljava/lang/String;")
    defineJavaMethod(Date, toString, "()Ljava/lang/String;")
    defineJavaMethod(Date, getDate, "()I")
    defineJavaMethod(Date, getMonth, "()I")
    defineJavaMethod(Date, getYear, "()I")
    defineJavaMethod(Date, getTime, "()J")

#define defineJavaConstructor(class, signatur) \
    priv->method_##class##_constructor = (*priv->jni_env)->GetMethodID(priv->jni_env, priv->class_##class, "<init>", signatur); \
    if (priv->method_##class##_constructor == NULL) { \
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env); \
        return NULL; \
    }
    defineJavaConstructor(Konto, "()V")
    defineJavaConstructor(HBCIHandler, "(Ljava/lang/String;Lorg/kapott/hbci/passport/HBCIPassport;)V")
    defineJavaConstructor(HBCICallbackConsole, "()V")
    defineJavaConstructor(HBCICallbackNative, "()V")

    // register native methods for callbacks
    methods[0].name = "nativeLog";
    methods[0].signature = "(Ljava/lang/String;ILjava/util/Date;Ljava/lang/StackTraceElement;)V";
    methods[0].fnPtr = my_log;
    methods[1].name = "nativeCallback";
    methods[1].signature = "(Lorg/kapott/hbci/passport/HBCIPassport;ILjava/lang/String;ILjava/lang/StringBuffer;)V";
    methods[1].fnPtr = my_callback;
    methods[2].name = "nativeStatus";
    methods[2].signature = "(Lorg/kapott/hbci/passport/HBCIPassport;I[Ljava/lang/Object;)V";
    methods[2].fnPtr = my_status;
    jint result = (*priv->jni_env)->RegisterNatives(priv->jni_env, priv->class_HBCICallbackNative, methods, 3);
    if (result != 0) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return NULL;
    }

#define defineGenericJavaField(class, name, signature, type) \
    priv->field_##class##_##name = (*priv->jni_env)->Get##type##FieldID(priv->jni_env, priv->class_##class, #name, signature); \
    if (priv->field_##class##_##name == NULL) { \
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env); \
        return NULL; \
    }
#define defineStaticJavaField(class, name, signature) defineGenericJavaField(class, name, signature, Static)
#define defineJavaField(class, name, signature) defineGenericJavaField(class, name, signature, )
    defineStaticJavaField(HBCIUtilsInternal, blzs, "Ljava/util/Properties;");
    defineJavaField(Konto, country, "Ljava/lang/String;");
    defineJavaField(Konto, blz, "Ljava/lang/String;");
    defineJavaField(Konto, number, "Ljava/lang/String;");
    defineJavaField(Konto, subnumber, "Ljava/lang/String;");
    defineJavaField(Konto, acctype, "Ljava/lang/String;");
    defineJavaField(Konto, type, "Ljava/lang/String;");
    defineJavaField(Konto, curr, "Ljava/lang/String;");
    defineJavaField(Konto, customerid, "Ljava/lang/String;");
    defineJavaField(Konto, name, "Ljava/lang/String;");
    defineJavaField(Konto, name2, "Ljava/lang/String;");
    defineJavaField(Konto, bic, "Ljava/lang/String;");
    defineJavaField(Konto, iban, "Ljava/lang/String;");
    defineJavaField(GVRSaldoReqInfo, ready, "Lorg/kapott/hbci/structures/Saldo;");
    defineJavaField(Saldo, value, "Lorg/kapott/hbci/structures/Value;");
    defineJavaField(GVRKUmsUmsLine, valuta, "Ljava/util/Date;");
    defineJavaField(GVRKUmsUmsLine, bdate, "Ljava/util/Date;");
    defineJavaField(GVRKUmsUmsLine, value, "Lorg/kapott/hbci/structures/Value;");
    defineJavaField(GVRKUmsUmsLine, saldo, "Lorg/kapott/hbci/structures/Saldo;");
    defineJavaField(GVRKUmsUmsLine, gvcode, "Ljava/lang/String;");
    defineJavaField(GVRKUmsUmsLine, usage, "Ljava/util/List;");
    defineJavaField(GVRKUmsUmsLine, other, "Lorg/kapott/hbci/structures/Konto;");
    defineJavaField(GVRKUmsUmsLine, text, "Ljava/lang/String;");

    // initialize hbci4java
    console = (*priv->jni_env)->NewObject(priv->jni_env, priv->class_HBCICallbackNative, priv->method_HBCICallbackNative_constructor);
    (*priv->jni_env)->CallStaticVoidMethod(priv->jni_env, priv->class_HBCIUtils, priv->method_HBCIUtils_init, 0, console);

    return context;
}


/**
 * ghbci_context_get_name_for_blz:
 * @self: The #GHbciContext
 * @blz: BLZ to resolve
 *
 * get bank name for BLZ
 *
 * Returns: (transfer full): bank name
 **/
const gchar*
ghbci_context_get_name_for_blz (GHbciContext* self, const gchar* blz)
{
    GHbciContextPrivate* priv;
    const gchar* result;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), "");
    priv = self->priv;

    jstring java_blz = (*priv->jni_env)->NewStringUTF(priv->jni_env, blz);

    jobject name = (*priv->jni_env)->CallStaticObjectMethod(priv->jni_env, priv->class_HBCIUtils, priv->method_HBCIUtils_getNameForBLZ, java_blz);
    if (name == NULL) {
        g_warning("empty result\n");
        result = "";
        goto clean_blz;
    }

    const char *nativeString = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, name, 0);
    // create extra string outside of JVM
    result = g_strdup (nativeString);
    (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, name, nativeString);

    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, name);
clean_blz:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, java_blz);
    return result;
}

/**
 * ghbci_context_get_pin_tan_url_for_blz:
 * @self: The #GHbciContext
 * @blz: BLZ to resolve
 *
 * get pin-tan url for BLZ
 *
 * Returns: (transfer full): url
 **/
const gchar*
ghbci_context_get_pin_tan_url_for_blz (GHbciContext* self, const gchar* blz)
{
    GHbciContextPrivate* priv;
    const gchar* result;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), "");
    priv = self->priv;

    jstring java_blz = (*priv->jni_env)->NewStringUTF(priv->jni_env, blz);

    // url = HBCIUtils.getPinTanURLForBLZ(blz)
    jobject url = (*priv->jni_env)->CallStaticObjectMethod(priv->jni_env,
            priv->class_HBCIUtils, priv->method_HBCIUtils_getPinTanURLForBLZ, java_blz);
    if (url == NULL) {
        g_warning("empty result\n");
        result = "";
        goto clean_blz;
    }

    // create extra string outside of JVM
    const char *nativeString = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, url, 0);
    result = g_strdup (nativeString);
    (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, url, nativeString);

    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, url);
clean_blz:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, java_blz);
    return result;
}

/**
 * ghbci_context_blz_foreach:
 * @self: The #GHbciContext
 * @func: (scope call): function called for each bank
 * @user_data: data for @func
 *
 * iterator over list of german banks included with hbci4java and call the
 * callback for each of them
 **/
void
ghbci_context_blz_foreach (GHbciContext* self, GHbciBlzFunc func, gpointer user_data)
{
    GHbciContextPrivate* priv;

    g_return_if_fail (GHBCI_IS_CONTEXT (self));
    g_return_if_fail (func != NULL);
    priv = self->priv;

    jobject blzs = (*priv->jni_env)->GetStaticObjectField(priv->jni_env, priv->class_HBCIUtilsInternal, priv->field_HBCIUtilsInternal_blzs);

    jobject blzs_keys = (*priv->jni_env)->CallObjectMethod(priv->jni_env, blzs, priv->method_Properties_keys);

    while((*priv->jni_env)->CallBooleanMethod(priv->jni_env, blzs_keys, priv->method_Enumeration_hasMoreElements)) {

        jobject element = (*priv->jni_env)->CallObjectMethod(priv->jni_env, blzs_keys, priv->method_Enumeration_nextElement);

        const gchar* nativeString = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, element, 0);
        (*func) (nativeString, user_data);
        (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, element, nativeString);

        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, element);
    }

    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, blzs_keys);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, blzs);
    return;
}

/**
 * ghbci_context_add_passport:
 * @self: The #GHbciContext
 * @blz: blz
 * @userid: userid
 *
 * Add bank account to passport file. Triggers #callback signal for additional
 * account details and credentials and fetches account capabilities online.
 *
 * Returns: TRUE if successful
 **/
gboolean
ghbci_context_add_passport (GHbciContext* self, const gchar* blz, const gchar* userid)
{
    GHbciContextPrivate* priv;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), FALSE);
    g_return_val_if_fail (blz != NULL, FALSE);
    g_return_val_if_fail (userid != NULL, FALSE);
    priv = self->priv;

    gchar* key = g_strconcat(blz, "+", userid, NULL);

    // set passport filename
    gchar* filename = g_strconcat(priv->passport_directory, "/passport-", key, ".dat", NULL);
    jstring filename_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "client.passport.PinTan.filename");
    jstring filename_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, filename);
    (*priv->jni_env)->CallStaticVoidMethod(priv->jni_env, priv->class_HBCIUtils, priv->method_HBCIUtils_setParam, filename_key, filename_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, filename_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, filename_value);

    if (g_slist_find_custom(priv->passports, filename, (GCompareFunc)g_strcmp0) == NULL) {
        priv->passports = g_slist_prepend(priv->passports, filename);
    } else {
        g_free(filename);
    }

    // force check certificates
    jstring checkcert_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "client.passport.PinTan.checkcert");
    jstring checkcert_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, "1");
    (*priv->jni_env)->CallStaticVoidMethod(priv->jni_env, priv->class_HBCIUtils, priv->method_HBCIUtils_setParam, checkcert_key, checkcert_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, checkcert_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, checkcert_value);

    // require reinitialization of pinTan
    jstring pinTanInit_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "client.passport.PinTan.init");
    jstring pinTanInit_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, "1");
    (*priv->jni_env)->CallStaticVoidMethod(priv->jni_env, priv->class_HBCIUtils, priv->method_HBCIUtils_setParam, pinTanInit_key, pinTanInit_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, pinTanInit_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, pinTanInit_value);

    // set log level
    jstring loglevel_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "log.loglevel.default");
    jstring loglevel_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, "5");
    (*priv->jni_env)->CallStaticVoidMethod(priv->jni_env, priv->class_HBCIUtils, priv->method_HBCIUtils_setParam, loglevel_key, loglevel_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, loglevel_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, loglevel_value);

    // create HBCIPassport object
    jstring type = (*priv->jni_env)->NewStringUTF(priv->jni_env, "PinTan");
    jobject passport = (*priv->jni_env)->CallStaticObjectMethod(priv->jni_env, priv->class_AbstractHBCIPassport, priv->method_AbstractHBCIPassport_getInstance, type);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, type);

    if (passport == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return FALSE;
    }

    // create HBCIHandler from passport
    jstring version = (*priv->jni_env)->NewStringUTF(priv->jni_env, "300");
    jobject handler = (*priv->jni_env)->NewObject(priv->jni_env, priv->class_HBCIHandler, priv->method_HBCIHandler_constructor, version, passport);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, version);

    if (handler == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, passport);
        return FALSE;
    }
    
    g_hash_table_insert(priv->hbci_handlers, key, handler);
    return TRUE;
}

/**
 * ghbci_context_get_accounts:
 * @self: The #GHbciContext
 * @blz: blz
 * @userid: userid
 *
 * Get list of bank accounts visible by this userid
 *
 * Returns: (element-type GHbciAccount) (transfer full): List of #GHbciAccount objects
 **/
GSList*
ghbci_context_get_accounts (GHbciContext* self, const gchar* blz, const gchar* userid)
{
    GHbciContextPrivate* priv;
    GSList *account_list = NULL;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), NULL);
    priv = self->priv;

    jobject hbci_handler = get_hbci_handler(self, blz, userid);
    if(hbci_handler == NULL) {
        g_warning("no handler found");
        return NULL;
    }

    // get passport from HBCIHandler
    jobject passport = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_getPassport);
    if (passport == NULL) {
        g_warning("creating passport failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return NULL;
    }

    // get accounts
    jobject accounts = (*priv->jni_env)->CallObjectMethod(priv->jni_env, passport, priv->method_HBCIPassport_getAccounts);
    if (accounts == NULL) {
        g_warning("fetching accounts failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, passport);
        return NULL;
    }
    int i;
    for(i = 0; i < (*priv->jni_env)->GetArrayLength(priv->jni_env, accounts); i++) {
        jobject element = (*priv->jni_env)->GetObjectArrayElement(priv->jni_env, accounts, i);

        GHbciAccount* account = ghbci_account_new_with_jobject(self, element);
        account_list = g_slist_append (account_list, account);

        gchar* number;
        g_object_get(account, "number", &number, NULL);

        char* key = g_strconcat(blz, "+", userid, "+", number, NULL);
        g_hash_table_insert(priv->accounts, key, element);

        g_free (number);
    }

    return account_list;
}

/**
 * ghbci_context_get_tan_methods:
 * @self: The #GHbciContext
 * @blz: blz
 * @userid: userid
 *
 * Get list of tan methods supported by this account
 *
 * Returns: (element-type gchar* gchar*) (transfer full): List of tan methods
 **/
GHashTable*
ghbci_context_get_tan_methods (GHbciContext* self, const gchar* blz, const gchar* userid)
{
    GHbciContextPrivate* priv;
    GHashTable *tan_methods_result = NULL;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), NULL);
    priv = self->priv;

    jobject hbci_handler = get_hbci_handler(self, blz, userid);
    if(hbci_handler == NULL) {
        g_warning("no handler found");
        return NULL;
    }

    // get passport from HBCIHandler
    jobject passport = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_getPassport);
    if (passport == NULL) {
        g_warning("creating passport failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return NULL;
    }

    (*priv->jni_env)->CallVoidMethod(priv->jni_env, passport, priv->method_AbstractPinTanPassport_setCurrentTANMethod, NULL);

    // get tan methods
    jobject tan_methods = (*priv->jni_env)->CallObjectMethod(priv->jni_env, passport, priv->method_AbstractPinTanPassport_getTwostepMechanisms);
    if (tan_methods == NULL) {
        g_warning("fetching tan methods failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_passport;
    }

    // get allowed tan methods
    jobject allowed_tan_methods = (*priv->jni_env)->CallObjectMethod(priv->jni_env, passport, priv->method_AbstractPinTanPassport_getAllowedTwostepMechanisms);
    if (allowed_tan_methods == NULL) {
        g_warning("fetching allowed tan methods failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_tan_methods;
    }

    jobject tan_methods_keys = (*priv->jni_env)->CallObjectMethod(priv->jni_env, tan_methods, priv->method_Properties_keys);

    jstring name_str = (*priv->jni_env)->NewStringUTF(priv->jni_env, "name");

    tan_methods_result = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    while((*priv->jni_env)->CallBooleanMethod(priv->jni_env, tan_methods_keys, priv->method_Enumeration_hasMoreElements)) {

        jobject key = (*priv->jni_env)->CallObjectMethod(priv->jni_env, tan_methods_keys, priv->method_Enumeration_nextElement);

        if ((*priv->jni_env)->CallBooleanMethod(priv->jni_env, allowed_tan_methods, priv->method_List_contains, key)) {
            jobject properties = (*priv->jni_env)->CallObjectMethod(priv->jni_env, tan_methods, priv->method_Hashtable_get, key);
            jobject name = (*priv->jni_env)->CallObjectMethod(priv->jni_env, properties, priv->method_Properties_getProperty, name_str);

            const gchar* native_key = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, key, 0);
            const gchar* native_name = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, name, 0);

            g_hash_table_insert(tan_methods_result, g_strdup(native_key), g_strdup(native_name));

            (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, name, native_name);
            (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, key, native_key);

            (*priv->jni_env)->DeleteLocalRef(priv->jni_env, properties);
            (*priv->jni_env)->DeleteLocalRef(priv->jni_env, name);
        }
        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, key);
    }

    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, name_str);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, tan_methods_keys);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, allowed_tan_methods);
cleanup_tan_methods:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, tan_methods);
cleanup_passport:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, passport);

    return tan_methods_result;
}

/**
 * ghbci_context_get_balances:
 * @self: The #GHbciContext
 * @blz: blz
 * @userid: userid
 * @number: number of account to inquery
 *
 * Fetch balances of bank accounts
 *
 * Returns: (transfer full): balance
 **/
gchar*
ghbci_context_get_balances (GHbciContext* self, const gchar* blz, const gchar* userid, const gchar* number)
{
    GHbciContextPrivate* priv;
    gchar *value = NULL;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), NULL);
    priv = self->priv;

    jobject hbci_handler = get_hbci_handler(self, blz, userid);
    if(hbci_handler == NULL) {
        g_warning("no handler found");
        return NULL;
    }

    // create HBCIJob
    jstring jobname = (*priv->jni_env)->NewStringUTF(priv->jni_env, "SaldoReq");
    jobject job = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_newJob, jobname);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jobname);

    if (job == NULL) {
        g_warning("newJob failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return NULL;
    }

    // set country
    jstring country_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "my.country");
    jstring country_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, "DE");
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, country_key, country_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, country_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, country_value);

    // set blz
    jstring blz_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "my.blz");
    jstring blz_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, blz);
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, blz_key, blz_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, blz_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, blz_value);

    // set account number
    jstring number_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "my.number");
    jstring number_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, number);
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, number_key, number_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, number_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, number_value);

    // add to job queue
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_addToQueue);

    // execute queue
    jobject status = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_execute);
    if (status == NULL) {
        g_warning("HBCIHandler execute failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_job;
    }

    jobject result = (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_getJobResult);
    if (result == NULL) {
        g_warning("getJobResult failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_status;
    }

    jboolean isOK = (*priv->jni_env)->CallBooleanMethod(priv->jni_env, result, priv->method_HBCIJobResultImpl_isOK);
    if (!isOK) {
        g_warning("job failed");
        goto cleanup_result;
    }

    // GVRSaldoReq.Info[] saldi = res.getEntries();
    jobject entries = (*priv->jni_env)->CallObjectMethod(priv->jni_env, result, priv->method_GVRSaldoReq_getEntries);
    if (entries == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_result;
    }
    jobject element = (*priv->jni_env)->GetObjectArrayElement(priv->jni_env, entries, 0);
    if (element == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_entries;
    }
    jobject ready = (*priv->jni_env)->GetObjectField(priv->jni_env, element, priv->field_GVRSaldoReqInfo_ready);
    if (ready == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_element;
    }
    jobject jvalue = (*priv->jni_env)->GetObjectField(priv->jni_env, ready, priv->field_Saldo_value);
    if (jvalue == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_ready;
    }
    jobject jvaluestr = (*priv->jni_env)->CallObjectMethod(priv->jni_env, jvalue, priv->method_Value_toString);
    if (jvaluestr == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_jvalue;
    }

    // to native string
    const gchar* nativeString = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, jvaluestr, 0);
    value = g_strdup(nativeString);
    (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, jvaluestr, nativeString);

    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jvaluestr);
cleanup_jvalue:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jvalue);
cleanup_ready:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, ready);
cleanup_element:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, element);
cleanup_entries:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, entries);
cleanup_result:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, result);
cleanup_status:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, status);
cleanup_job:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, job);
    return value;
}


/**
 * ghbci_context_get_statements:
 * @self: The #GHbciContext
 * @blz: blz
 * @userid: userid
 * @number: bank account number
 *
 * Fetch all statements for a bank account
 *
 * Returns: (element-type GHbciStatement) (transfer full): List of #GHbciStatement objects
 **/
GSList*
ghbci_context_get_statements (GHbciContext* self, const gchar* blz, const gchar* userid, const gchar* number)
{
    GHbciContextPrivate* priv;
    GSList* statements = NULL;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), NULL);
    priv = self->priv;

    jobject hbci_handler = get_hbci_handler(self, blz, userid);
    if(hbci_handler == NULL) {
        printf("no handler found!\n");
        return NULL;
    }

    // create HBCIJob
    jstring jobname = (*priv->jni_env)->NewStringUTF(priv->jni_env, "KUmsAll");
    jobject job = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_newJob, jobname);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jobname);

    if (job == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return NULL;
    }

    jstring country_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "my.country");
    jstring country_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, "DE");
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, country_key, country_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, country_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, country_value);

    jstring blz_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "my.blz");
    jstring blz_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, blz);
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, blz_key, blz_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, blz_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, blz_value);

    jstring number_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, "my.number");
    jstring number_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, number);
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, number_key, number_value);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, number_key);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, number_value);

    // add to queue
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_addToQueue);

    // run queue
    jobject status = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_execute);
    if (status == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_job;
    }

    jobject result = (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_getJobResult);
    if (result == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_status;
    }
    jboolean isOK = (*priv->jni_env)->CallBooleanMethod(priv->jni_env, result, priv->method_HBCIJobResultImpl_isOK);
    if (!isOK) {
        printf("job failed\n");
        goto cleanup_result;
    }
    jobject jstatements = (*priv->jni_env)->CallObjectMethod(priv->jni_env, result, priv->method_GVRKUms_getFlatData);
    if (jstatements == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_result;
    }

    jobject jstatements_iter = (*priv->jni_env)->CallObjectMethod(priv->jni_env, jstatements, priv->method_List_iterator);
    if (jstatements_iter == NULL) {
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_jstatements;
    }

    while((*priv->jni_env)->CallBooleanMethod(priv->jni_env, jstatements_iter, priv->method_Iterator_hasNext)) {
        jobject jstatement = (*priv->jni_env)->CallObjectMethod(priv->jni_env, jstatements_iter, priv->method_Iterator_next);
        if (jstatement == NULL) {
            (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
            goto cleanup_jstatements_iter;
        }
        GHbciStatement* statement = ghbci_statement_new_with_jobject(self, jstatement);
        statements = g_slist_append (statements, statement);

        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jstatement);
    }

cleanup_jstatements_iter:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jstatements_iter);
cleanup_jstatements:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jstatements);
cleanup_result:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, result);
cleanup_status:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, status);
cleanup_job:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, job);
    return statements;
}


/**
 * ghbci_context_send_transfer:
 * @self: The #GHbciContext
 * @blz: blz
 * @userid: userid
 * @number: account number
 * @destination_name: name of recipient
 * @destination_bic: bic
 * @destination_iban: iban
 * @reference: reference used in transfer
 * @amount: amount to transfer
 *
 * Send SEPA transfer
 *
 * Returns: true if successful
 **/
gboolean
ghbci_context_send_transfer (GHbciContext* self, const gchar* blz, const gchar* userid, const gchar* number,
        const gchar* source_name, const gchar* source_bic, const gchar* source_iban,
        const gchar* destination_name, const gchar* destination_bic, const gchar* destination_iban,
        const gchar* reference, const gchar* amount)
{
    GHbciContextPrivate* priv;
    gboolean return_value = FALSE;

    g_return_val_if_fail (GHBCI_IS_CONTEXT (self), FALSE);
    priv = self->priv;

    jobject hbci_handler = get_hbci_handler(self, blz, userid);
    if(hbci_handler == NULL) {
        g_warning("no handler found");
        return FALSE;
    }

    (*priv->jni_env)->CallVoidMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_reset);

    jstring jobname = (*priv->jni_env)->NewStringUTF(priv->jni_env, "UebSEPA"); // TODO: support TermUebSEPA
    jobject job = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_newJob, jobname);
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, jobname);

    if (job == NULL) {
        g_warning("newJob failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        return FALSE;
    }
#define HBCIJob_setParam(variable, key, value) \
    jstring variable##_key = (*priv->jni_env)->NewStringUTF(priv->jni_env, key); \
    jstring variable##_value = (*priv->jni_env)->NewStringUTF(priv->jni_env, value); \
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_setParam, variable##_key, variable##_value); \
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, variable##_key); \
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, variable##_value);

    HBCIJob_setParam(src_country, "src.country", "DE");
    HBCIJob_setParam(src_blz, "src.blz", blz);
    HBCIJob_setParam(src_number, "src.number", number);
    HBCIJob_setParam(src_name, "src.name", source_name);
    HBCIJob_setParam(src_bic, "src.bic", source_bic);
    HBCIJob_setParam(src_iban, "src.iban", source_iban);
    HBCIJob_setParam(dst_name, "dst.name", destination_name);
    HBCIJob_setParam(dst_bic, "dst.bic", destination_bic);
    HBCIJob_setParam(dst_iban, "dst.iban", destination_iban);
    HBCIJob_setParam(usage, "usage", reference);
    HBCIJob_setParam(btg_value, "btg.value", amount);
    HBCIJob_setParam(btg_curr, "btg.curr", "EUR");

    // add to queue
    (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_addToQueue);

    jobject status = (*priv->jni_env)->CallObjectMethod(priv->jni_env, hbci_handler, priv->method_HBCIHandler_execute);
    if (status == NULL) {
        g_warning("HBCIHandler execute failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_job;
    }

    jobject result = (*priv->jni_env)->CallObjectMethod(priv->jni_env, job, priv->method_HBCIJob_getJobResult);
    if (result == NULL) {
        g_warning("getJobResult failed");
        (*priv->jni_env)->ExceptionDescribe(priv->jni_env);
        goto cleanup_status;
    }
    jboolean isOK = (*priv->jni_env)->CallBooleanMethod(priv->jni_env, result, priv->method_HBCIJobResultImpl_isOK);
    if (!isOK) {
        g_warning("job failed");

        jobject job_status = (*priv->jni_env)->CallObjectMethod(priv->jni_env, result, priv->method_HBCIJobResult_getJobStatus);
        jstring errorstring = (*priv->jni_env)->CallObjectMethod(priv->jni_env, status, priv->method_HBCIStatus_getErrorString);

        if (errorstring != NULL) {
            const gchar* nativeString = (*priv->jni_env)->GetStringUTFChars(priv->jni_env, errorstring, 0);
            g_warning("error: %s", nativeString);
            (*priv->jni_env)->ReleaseStringUTFChars(priv->jni_env, errorstring, nativeString);
        }

        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, job_status);
        (*priv->jni_env)->DeleteLocalRef(priv->jni_env, errorstring);
        return_value = FALSE;
    }
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, result);
cleanup_status:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, status);
cleanup_job:
    (*priv->jni_env)->DeleteLocalRef(priv->jni_env, job);
    return return_value;
}


// vim: sw=4 expandtab
