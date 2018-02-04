///////////////////////////////////////////////////////////////////////////////////////////////////
//  
//      __   __                              __    
//     |  |_|  |--.-----. .----.--.--.-----.|  |--.
//   __|   _|     |  -__| |   _|  |  |__ --||     |
//  |__|____|__|__|_____| |__| |_____|_____||__|__|
//                                                 
//  Copyright (C) 2007-2013 Cedric Guillemet
//
// This file is part of .the rush//.
//
//    .the rush// is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    .the rush// is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with .the rush//.  If not, see <http://www.gnu.org/licenses/>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CORE_SMARTPOINTER_H__
#define CORE_SMARTPOINTER_H__

#if 0   //  not used for now and restricted to compiler supporting C++11 (formerly C++0x)
#include <memory>


//class to reference ressource with only one owner and multiple weak references
template<typename _T>
class ref_ptr
{
    bool m_bIsOwner;
    std::tr1::shared_ptr<_T> m_Owner;
    std::tr1::weak_ptr<_T> m_Ref;

public:
    typedef ref_ptr<_T> _ref_T;

    ref_ptr() : m_bIsOwner(false), m_Owner(), m_Ref()
    {

    }

    ref_ptr(const _ref_T& _Other) : m_bIsOwner(false), m_Owner(), m_Ref(_Other.m_Owner)
    {
        ASSERT_GAME( m_Ref.use_count() == 1 );
    }

    ref_ptr(_T*& _Ptr) : m_bIsOwner(true), m_Owner(_Ptr), m_Ref()
    {
        ASSERT_GAME( m_Owner.use_count() == 1 );
    }

    ~ref_ptr()
    {
        //DO STHG??
    }

    _ref_T& operator = (_T* _Right)
    {
        ASSERT_GAME( m_bIsOwner == false );
        ASSERT_GAME( m_Owner._Get() == NULL );
        m_Ref.reset();

        if ( _Right != NULL )
        {
            m_bIsOwner = true;
            m_Owner.reset(_Right);
        }

        return (*this);
    }
/*
    _ref_T& operator = (_ref_T& _Right)
    {
        ASSERT_GAME( m_bIsOwner == false );
        ASSERT_GAME( m_Owner._Get() == NULL );
        m_Ref.reset();
        m_Ref = (_Right.m_bIsOwner) ? _Right.m_Owner : _Right.m_Ref;

        return (*this);
    }
*/
    _ref_T& operator = (const _ref_T& _Right)
    {
        ASSERT_GAME( m_bIsOwner == false );
        ASSERT_GAME( m_Owner.use_count() == 0 && m_Owner.get() == NULL );
        ASSERT_GAME( _Right.use_count() == 1 );
        m_Ref.reset();

        //FIXME: for some reason, when assigning a shared_ptr to m_Ref, it constructs a temporary intermediate weak_ptr
        m_Ref = (_Right.m_bIsOwner) ? _Right.m_Owner : _Right.m_Ref;

        return (*this);
    }

    _T *operator->() const
    {
        return get();
    }

private:
    friend bool operator == ( const _ref_T& _R1, const _ref_T& _R2 );

    friend bool operator != ( const _ref_T& _R1, const _ref_T& _R2 );

    _T *get() const
    {
        _T* const ptr = (m_bIsOwner) ? m_Owner._Get() : m_Ref._Get();

        //IMPORTANT: note that if ref owner has been deleted and thus m_Ref has expired data,
        // using m_Ref._Get() returned pointer will most likely result in a crash / memory corruption

        const long ref_use_count = use_count();
        ASSERT_GAME_MSG( ref_use_count != 0, "referenced resource at adress '%p' isn't valid anymore. Ref owner has been deleted.", ptr );
        ASSERT_GAME_MSG( ref_use_count == 1, "referenced resource should have only one owner but has %d instead.", ref_use_count );

        return (ref_use_count != 0) ? ptr : NULL;
    }

    long use_count() const
    {
        return (m_bIsOwner) ? m_Owner.use_count() : m_Ref.use_count();
    }
};

template<typename _T>
bool operator == ( const ref_ptr<_T>& _R1, const ref_ptr<_T>& _R2 )
{
    return (_R1.get() == _R2.get());
}

template<typename _T>
bool operator != ( const ref_ptr<_T>& _R1, const ref_ptr<_T>& _R2 )
{
    return (!(_R1.get() == _R2.get()));
}
#endif  //  0

#endif  //  CORE_SMARTPOINTER_H__
