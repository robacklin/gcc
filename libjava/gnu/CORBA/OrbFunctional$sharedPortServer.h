
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_OrbFunctional$sharedPortServer__
#define __gnu_CORBA_OrbFunctional$sharedPortServer__

#pragma interface

#include <gnu/CORBA/OrbFunctional$portServer.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class OrbFunctional;
        class OrbFunctional$sharedPortServer;
    }
  }
}

class gnu::CORBA::OrbFunctional$sharedPortServer : public ::gnu::CORBA::OrbFunctional$portServer
{

public: // actually package-private
  OrbFunctional$sharedPortServer(::gnu::CORBA::OrbFunctional *, jint);
  virtual void tick();
  ::gnu::CORBA::OrbFunctional * __attribute__((aligned(__alignof__( ::gnu::CORBA::OrbFunctional$portServer)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_OrbFunctional$sharedPortServer__
