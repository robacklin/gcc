
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_InputMap__
#define __javax_swing_InputMap__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace swing
    {
        class InputMap;
        class KeyStroke;
    }
  }
}

class javax::swing::InputMap : public ::java::lang::Object
{

public:
  InputMap();
  virtual ::java::lang::Object * get(::javax::swing::KeyStroke *);
  virtual void put(::javax::swing::KeyStroke *, ::java::lang::Object *);
  virtual void remove(::javax::swing::KeyStroke *);
  virtual ::javax::swing::InputMap * getParent();
  virtual void setParent(::javax::swing::InputMap *);
  virtual jint size();
  virtual void clear();
  virtual JArray< ::javax::swing::KeyStroke * > * keys();
  virtual JArray< ::javax::swing::KeyStroke * > * allKeys();
private:
  static const jlong serialVersionUID = -5429059542008604257LL;
  ::java::util::Map * __attribute__((aligned(__alignof__( ::java::lang::Object)))) inputMap;
  ::javax::swing::InputMap * parent;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_InputMap__
