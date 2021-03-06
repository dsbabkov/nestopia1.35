////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef NST_OBJECT_HEAP_H
#define NST_OBJECT_HEAP_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace Object
	{
		template<typename T,uint N=0U> class Heap
		{
			T* const array;

		public:

			enum
			{
				SIZE = N
			};

			Heap()
			: array(new T [SIZE]) {}

			~Heap()
			{
				delete [] array;
			}

			operator T* () const
			{
				return array;
			}
		};

		template<typename T> class Heap<T,0U>
		{
			T& ref;

			Heap(const Heap&);
			void operator = (const Heap&);

		public:

			Heap()
			: ref(*(new T)) {}

			explicit Heap(T* t)
			: ref(*t)
			{
				NST_ASSERT(t);
			}

			~Heap()
			{
				delete &ref;
			}

			T& operator * () const
			{
				return ref;
			}

			T* operator -> () const
			{
				return &ref;
			}
		};

		template<>
		class Heap<void,0U>
		{
			void* const ref;

			Heap(const Heap&);
			void operator = (const Heap&);

		public:

			explicit Heap(void* t)
			: ref(t)
			{
				NST_ASSERT( t );
			}

			explicit Heap(uint size)
			: ref(operator new (size))
			{
				NST_ASSERT( size );
			}

			~Heap()
			{
				operator delete (ref);
			}

			operator void* () const
			{
				return ref;
			}
		};
	}
}

#endif
