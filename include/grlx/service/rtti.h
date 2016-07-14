/*
 * 
 * Copyright (c) 2011 - ${year}, 
 * David Tito de Lencastre Salvador Pinheiro (david.pinheiro@gorilux.org)
 * All rights reserved.
 *
 * The intellectual and technical concepts contained
 * herein are proprietary to David Tito de Lencastre Salvador Pinheiro
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from David Tito de Lencastre Salvador Pinheiro.
 * 
 */

#ifndef RTTI_H_
#define RTTI_H_

#include <typeinfo>
#include <cstdlib>
#include <string>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

namespace grlx {

namespace core{


typedef int TypeId;


//template<class T>
//struct TypeInfo
//{
//    static TypeId id() { return reinterpret_cast<TypeId>( &inst() ); }
//private:
//    struct Impl{};
//    static Impl const & inst() { static Impl sImpl; return sImpl; }
//};




//struct Rtti {

//	class IdType {
//	public:
//		////////////////////////////////////////////////////////////////////////
//		explicit IdType(const std::type_info & id) :
//				id_(id) {
//		}

//		bool operator==(IdType right) const {
//			return (id_ == right.id_) != 0;
//		}
//		bool operator!=(IdType right) const {
//			return !(*this == right);
//		}

//		bool operator<(IdType right) const {
//			return id_.before(right.id_) != 0;
//		}
//		bool operator>(IdType right) const {
//			return right < *this;
//		}
//		bool operator>=(IdType right) const {
//			return !(*this < right);
//		}
//		bool operator<=(IdType right) const {
//			return !(right < *this);
//		}
//		const char* MangledName() const {
//			return id_.name();
//		}
//		std::string Name() const
//		{
//#ifndef _MSC_VER
//			int status;
//			char *realname = abi::__cxa_demangle(id_.name(), 0,0,&status);
//			std::string name(realname);
//			::free(realname);
//            return name;
//#else
//            std::string result(MangledName());
//            int pos = result.find("class ");
//            if(pos != -1 ){
//                result.replace(pos,6,std::string());
//                return result;
//            }
//            pos = result.find("struct ");
//            if(pos != -1 ){
//                result.replace(pos,7,std::string());
//                return result;
//            }
//            return result;
//#endif

//		}


//	private:
//		////////////////////////////////////////////////////////////////////////
//		const std::type_info & id_;
//	};

//	template<class Base>
//	class RttiBaseType: public Base {
//	public:
//		////////////////////////////////////////////////////////////////////////
//		typedef Rtti::IdType id_type;

//		id_type dynamic_type() const {
//			return IdType( typeid( *this ) );
//		}

//	protected:

//		RttiBaseType( bool ) {}

//		virtual ~RttiBaseType() {}


//	private:

//		virtual void dummy() {}

//	};

//	////////////////////////////////////////////////////////////////////////////
//	template<class MostDerived, class Base>
//	class RttiDerivedType: public Base {
//	public:
//		////////////////////////////////////////////////////////////////////////
//		static IdType static_type() {
//			return IdType( typeid( const MostDerived ) );
//		}



//	protected:
//		////////////////////////////////////////////////////////////////////////
//		~RttiDerivedType() {}

//		RttiDerivedType() {}
//	};

//};

//template< typename T >
//struct Convert {

//	static std::string ToString(){
//		return Rtti::IdType (typeid(T)).Name();
//	}

//};
////template<typename T>
////struct ToString
////{
////    template<typename U>
////    ToString(void(*)(U), const std::string& p_str)
////    {
////        std::string str(p_str.begin() + 1, p_str.end() - 1);
////        std::cout << str << " => ";
////        std::string name;
////        int status;
////        name = abi::__cxa_demangle(typeid(U).name(), 0, 0, &status);
////
////        std::cout << name << std::endl;
////    }
////};





}
}

#endif /* RTTI_H_ */
