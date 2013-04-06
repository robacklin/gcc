
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_w3c_dom_bootstrap_DOMImplementationRegistry__
#define __org_w3c_dom_bootstrap_DOMImplementationRegistry__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class DOMImplementation;
          class DOMImplementationList;
          class DOMImplementationSource;
        namespace bootstrap
        {
            class DOMImplementationRegistry;
        }
      }
    }
  }
}

class org::w3c::dom::bootstrap::DOMImplementationRegistry : public ::java::lang::Object
{

  DOMImplementationRegistry(::java::util::Vector *);
public:
  static ::org::w3c::dom::bootstrap::DOMImplementationRegistry * newInstance();
  ::org::w3c::dom::DOMImplementation * getDOMImplementation(::java::lang::String *);
  ::org::w3c::dom::DOMImplementationList * getDOMImplementationList(::java::lang::String *);
  void addSource(::org::w3c::dom::DOMImplementationSource *);
private:
  static ::java::lang::ClassLoader * getClassLoader();
  static ::java::lang::String * getServiceValue(::java::lang::ClassLoader *);
  static jboolean isJRE11();
  static ::java::lang::ClassLoader * getContextClassLoader();
  static ::java::lang::String * getSystemProperty(::java::lang::String *);
  static ::java::io::InputStream * getResourceAsStream(::java::lang::ClassLoader *, ::java::lang::String *);
public:
  static ::java::lang::String * PROPERTY;
private:
  static const jint DEFAULT_LINE_LENGTH = 80;
  ::java::util::Vector * __attribute__((aligned(__alignof__( ::java::lang::Object)))) sources;
public:
  static ::java::lang::Class class$;
};

#endif // __org_w3c_dom_bootstrap_DOMImplementationRegistry__
