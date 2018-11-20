/*
 *   libperfmap: a JVM agent to create perf-<pid>.map files for consumption
 *               with linux perf-tools
 *   Copyright (C) 2013 Johannes Rudolph<johannes.rudolph@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <jni.h>
#include <jvmti.h>
#include <string.h>

FILE *method_file = NULL;

void open_file() {
    char methodFileName[500];
    sprintf(methodFileName, "/tmp/perf-%d.map", getpid());
    method_file = fopen(methodFileName, "w");
}

void JNICALL
cbVMInit(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread) {
    if (!method_file)
        open_file();
}

static void JNICALL
cbVMStart(jvmtiEnv *jvmti, JNIEnv *env) {
    jvmtiJlocationFormat format;
    (*jvmti)->GetJLocationFormat(jvmti, &format);
    //printf("[tracker] VMStart LocationFormat: %d\n", format);
}

static void JNICALL
cbCompiledMethodLoad(jvmtiEnv *env,
            jmethodID method,
            jint code_size,
            const void* code_addr,
            jint map_length,
            const jvmtiAddrLocationMap* map,
            const void* compile_info) {
    int i;

    char *name;
    char *msig;
    (*env)->GetMethodName(env, method, &name, &msig, NULL);

    jclass class;
    (*env)->GetMethodDeclaringClass(env, method, &class);
    char *csig;
    (*env)->GetClassSignature(env, class, &csig, NULL);

    fprintf(method_file, "%llx %x %s.%s%s\n", code_addr, code_size, csig, name, msig);
    fflush(method_file);
    (*env)->Deallocate(env, name);
    (*env)->Deallocate(env, msig);
    (*env)->Deallocate(env, csig);

    /*for (i = 0; i < map_length; i++) {
      printf("[tracker] Entry: start_address: 0x%lx location: %d\n", map[i].start_address, map[i].location);
    }*/
}

void JNICALL
cbDynamicCodeGenerated(jvmtiEnv *jvmti_env,
            const char* name,
            const void* address,
            jint length) {
    if (!method_file)
        open_file();

    fprintf(method_file, "%llx %x %s\n", address, length, name);
    fflush(method_file);
    //printf("[tracker] Code generated: %s %lx %x\n", name, address, length);
}

void JNICALL
cbCompiledMethodUnload(jvmtiEnv *jvmti_env,
            jmethodID method,
            const void* code_addr) {
    //printf("[tracker] Unloaded %ld code_addr: 0x%lx\n", method, code_addr);
}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char *options, void *reserved) {
    jvmtiEnv              *jvmti;
    jvmtiError             error;
    jint                   res;
    jvmtiCapabilities      capabilities;
    jvmtiEventCallbacks    callbacks;

    // Create the JVM TI environment (jvmti).
    res = (*vm)->GetEnv(vm, (void **)&jvmti, JVMTI_VERSION_1);
    // If res!=JNI_OK generate an error.

    // Parse the options supplied to this agent on the command line.
    //parse_agent_options(options);
    // If options don't parse, do you want this to be an error?

    // Clear the capabilities structure and set the ones you need.
    (void)memset(&capabilities,0, sizeof(capabilities));
    capabilities.can_generate_all_class_hook_events  = 1;
    capabilities.can_tag_objects                     = 1;
    capabilities.can_generate_object_free_events     = 1;
    capabilities.can_get_source_file_name            = 1;
    capabilities.can_get_line_numbers                = 1;
    capabilities.can_generate_vm_object_alloc_events = 1;
    capabilities.can_generate_compiled_method_load_events = 1;

    // Request these capabilities for this JVM TI environment.
    error = (*jvmti)->AddCapabilities(jvmti, &capabilities);
    // If error!=JVMTI_ERROR_NONE, your agent may be in trouble.

    // Clear the callbacks structure and set the ones you want.
    (void)memset(&callbacks,0, sizeof(callbacks));
    callbacks.VMInit           = &cbVMInit;
    callbacks.VMStart           = &cbVMStart;
    callbacks.CompiledMethodLoad  = &cbCompiledMethodLoad;
    callbacks.CompiledMethodUnload  = &cbCompiledMethodUnload;
    callbacks.DynamicCodeGenerated = &cbDynamicCodeGenerated;
    error = (*jvmti)->SetEventCallbacks(jvmti, &callbacks,
                      (jint)sizeof(callbacks));
    //  If error!=JVMTI_ERROR_NONE, the callbacks were not accepted.

    // For each of the above callbacks, enable this event.
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_VM_INIT, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_VM_START, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_COMPILED_METHOD_LOAD, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_COMPILED_METHOD_UNLOAD, (jthread)NULL);
    error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE,
                      JVMTI_EVENT_DYNAMIC_CODE_GENERATED, (jthread)NULL);
    // In all the above calls, check errors.

    return JNI_OK; // Indicates to the VM that the agent loaded OK.
}


