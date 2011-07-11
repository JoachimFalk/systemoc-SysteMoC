#include <iostream>
#include <string.h>
#include <jni.h>
#include <smoc/detail/TraceLogJNI.hpp>

namespace SysteMoC { namespace Detail {

using std::string;

TraceLogJNI::TraceLogJNI(/*bool b*/){
  
}
  //liefert Java Environment
JNIEnv* TraceLogJNI::getVM() {
    
    JavaVM *jvm;
    JNIEnv *env;
    JavaVMInitArgs vm_args;

    jsize bufLen = 4;
    jsize numVMs;
    
    jint res = JNI_GetCreatedJavaVMs(&jvm, bufLen, &numVMs);
    if(res < 0) res = JNI_CreateJavaVM(&jvm, (void **)env, &vm_args);

    //hängt aktuellen Thread an die Java VM an
    jvm->AttachCurrentThread((void**)&env, NULL);
    //jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    return env;
}


// call java method setTableItem(int, string[])) from systemoc.plugin.views.MainView.java 
void TraceLogJNI::callJavaMethod_SetTableItem(int num, string items[]){
  
  jclass cl; 
  jmethodID method;
  JNIEnv *env = getVM();

  int NUM_ITEMS = 5;
  jobjectArray array;

  jclass testClass = env->FindClass("java/lang/String");

  array = env->NewObjectArray(NUM_ITEMS, testClass, env->NewStringUTF(""));

  for(int i=0; i < NUM_ITEMS; i++){
    
      jstring tmp[NUM_ITEMS];
      string s1 = items[i];

      char *data = new char[s1.size()+1];
      strcpy(data, s1.c_str());

      tmp[i] = env->NewStringUTF(data);

      env->SetObjectArrayElement(array, i, tmp[i]);
    //env->DeleteLocalRef(tmp[i]);
  }

  cl = env->FindClass("systemoc/plugin/views/MainView"); 
  if (!cl){
    std::cout << "Error: Class not found" << std::endl;
  }
  method = env->GetStaticMethodID(cl, "setTableItem", "(I[Ljava/lang/String;)V"); 
  if (!method){
    std::cout << "Error: Method not found" << std::endl;
  } 
  
  //ruft Methode name auf
  env->CallStaticVoidMethod(cl, method, num, array);
  //env->NewObject(cl, method, num, array);
}

void TraceLogJNI::callJavaMethod_SetList(const char *name, int size, string items[]){
  
  jclass cl; 
  jmethodID method;

  JNIEnv *env = getVM();

  jobjectArray array;

  jclass testClass = env->FindClass("java/lang/String");
  array = env->NewObjectArray(size, testClass, env->NewStringUTF(""));
  jstring tmp[size];
        
  for(int i=0; i < size; i++){
 
      string s1 = items[i];

      char *data = new char[s1.size()+1];
      strcpy(data, s1.c_str());

      tmp[i] = env->NewStringUTF(data);

      env->SetObjectArrayElement(array, i, tmp[i]);
    //env->DeleteLocalRef(tmp[i]);
  }

  cl = env->FindClass("systemoc/plugin/views/MainView"); 
  if (!cl){
    std::cout << "Error: Class not found" << std::endl;
  }
  method = env->GetStaticMethodID(cl, name, "(I[Ljava/lang/String;)V"); 
  if (!method){
    std::cout << "Error: Method not found" << std::endl;
  } 
  
  env->CallStaticVoidMethod(cl, method, size, array);
  //env->NewObject(cl, method, size, array);
}

// call java void method without arguments
void TraceLogJNI::callJavaMethod(const char *packageClass, const char *name, const char *signature){
  
  jclass cl; 
  jmethodID method;  
  jobjectArray args;
  JNIEnv *env = getVM();
 
  //JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

  cl = env->FindClass(packageClass); 
  if (!cl){
    std::cout << "Error: Class not found" << std::endl;
  }
  method = env->GetMethodID(cl, name, signature); 
  if (!method){
    std::cout << "Error: Method not found" << std::endl;
  } 
  //ruft Methode name auf
  env->NewObject(cl, method, args);
  //NICHT:  env->CallVoidMethod(obj, method, args); 
}

} }