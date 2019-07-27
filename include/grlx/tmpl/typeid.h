#pragma once

#ifndef GRLX_TYPEID
#define GRLX_TYPEID

namespace grlx
{


typedef unsigned long TypeId;

template<class T>
struct TypeInfo
{
    static TypeId id() { return reinterpret_cast<TypeId>( &inst() ); }
private:
    struct Impl{};
    static Impl const & inst() { static Impl sImpl; return sImpl; }
};

template<class T>
TypeId typeId()
{
    return TypeInfo<T>::id();
}

template<class T>
TypeId typeId(T)
{
    return TypeInfo<T>::id();
}

}
#endif
