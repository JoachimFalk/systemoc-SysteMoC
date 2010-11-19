#include <jni.h>

namespace SysteMoC { namespace Detail {

using namespace std;

class TraceLogJNI{

private:
  bool stopped;
public:
  TraceLogJNI(bool b);
  JNIEnv* getVM();
  void callJavaMethod(const char *packageClass, const char *name, const char *signature);
  void callJavaMethod_SetTableItem(int num, std::string items[]);
  void callJavaMethod_SetList(const char *name, int size, std::string items[]);
  void setStopped(bool b);
  bool getStopped();
};

}  }