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

#ifndef NST_IO_PORT_H
#define NST_IO_PORT_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Io
		{
		#ifndef NST_FPTR_MEM_MAP

			class Port
			{
				class Component {};

				typedef Data (NES_IO_CALL Component::*Reader)(Address);
				typedef void (NES_IO_CALL Component::*Writer)(Address,Data);

				Component* component;
				Reader reader;
				Writer writer;

			public:

				Port() {}

				template<typename T>
				Port(T* c,Data (NES_IO_CALL T::*r)(Address),void (NES_IO_CALL T::*w)(Address,Data))
				:
				component ( reinterpret_cast<Component*>(c) ),
				reader    ( reinterpret_cast<Reader>(r)    ),
				writer    ( reinterpret_cast<Writer>(w)    )
				{
					NST_COMPILE_ASSERT( sizeof(reader) == sizeof(r) && sizeof(writer) == sizeof(w) );
				}

				template<typename T>
				void Set(T* c,Data (NES_IO_CALL T::*r)(Address),void (NES_IO_CALL T::*w)(Address,Data))
				{
					NST_COMPILE_ASSERT( sizeof(reader) == sizeof(r) && sizeof(writer) == sizeof(w) );

					component = reinterpret_cast<Component*>(c);
					reader    = reinterpret_cast<Reader>(r);
					writer    = reinterpret_cast<Writer>(w);
				}

				template<typename T>
				void Set(Data (NES_IO_CALL T::*r)(Address))
				{
					NST_COMPILE_ASSERT( sizeof(reader) == sizeof(r) );

					reader = reinterpret_cast<Reader>(r);
				}

				template<typename T>
				void Set(void (NES_IO_CALL T::*w)(Address,Data))
				{
					NST_COMPILE_ASSERT( sizeof(writer) == sizeof(w) );

					writer = reinterpret_cast<Writer>(w);
				}

				template<typename T>
				void Set(Data (NES_IO_CALL T::*r)(Address),void (NES_IO_CALL T::*w)(Address,Data))
				{
					NST_COMPILE_ASSERT( sizeof(reader) == sizeof(r) && sizeof(writer) == sizeof(w) );

					reader = reinterpret_cast<Reader>(r);
					writer = reinterpret_cast<Writer>(w);
				}

				uint Peek(Address address) const
				{
					return (*component.*reader)( address );
				}

				void Poke(Address address,Data data) const
				{
					(*component.*writer)( address, data );
				}

				bool SameComponent(const void* ptr) const
				{
					return static_cast<const void*>(component) == ptr;
				}

				bool operator == (const Port& p) const
				{
					return component == p.component && reader == p.reader && writer == p.writer;
				}
			};

			#define NES_DECL_PEEK(a_) Data NES_IO_CALL Peek_##a_(Address);
			#define NES_DECL_POKE(a_) void NES_IO_CALL Poke_##a_(Address,Data);

			#define NES_PEEK(o_,a_) Data NES_IO_CALL o_::Peek_##a_(Address address)
			#define NES_POKE(o_,a_) void NES_IO_CALL o_::Poke_##a_(Address address,Data data)

			#define NES_CALL_POKE(o_,a_,p_,d_) o_::Poke_##a_(p_,d_)
			#define NES_CALL_PEEK(o_,a_,p_)    o_::Peek_##a_(p_)

		#else

			class Port
			{
				typedef void* Component;
				typedef Data (NES_IO_CALL *Reader)(Component,Address);
				typedef void (NES_IO_CALL *Writer)(Component,Address,Data);

				Component component;
				Reader reader;
				Writer writer;

			public:

				Port() {}

				Port(void* c,Reader r,Writer w)
				:
				component ( c ),
				reader    ( r ),
				writer    ( w )
				{}

				void Set(void* c,Reader r,Writer w)
				{
					component = c;
					reader    = r;
					writer    = w;
				}

				void Set(Reader r)
				{
					reader = r;
				}

				void Set(Writer w)
				{
					writer = w;
				}

				void Set(Reader r,Writer w)
				{
					reader = r;
					writer = w;
				}

				Data Peek(Address address) const
				{
					return reader( component, address );
				}

				void Poke(Address address,Data data) const
				{
					writer( component, address, data );
				}

				bool SameComponent(const void* ptr) const
				{
					return component == ptr;
				}

				bool operator == (const Port& p) const
				{
					return component == p.component && reader == p.reader && writer == p.writer;
				}
			};

			#define NES_DECL_PEEK(a_)                                                                       \
																											\
				Data NES_IO_CALL Peek_Member_##a_(Address);                                                 \
																											\
				template<typename T>                                                                        \
				static Data Peek_Type_##a_(void* instance,Address address,Data (NES_IO_CALL T::*)(Address)) \
				{                                                                                           \
					return static_cast<T*>(instance)->Peek_Member_##a_( address );                          \
				}                                                                                           \
																											\
				static Data NES_IO_CALL Peek_##a_(void* instance,Address address)                           \
				{                                                                                           \
					return Peek_Type_##a_( instance, address, &Peek_Member_##a_ );                          \
				}

			#define NES_DECL_POKE(a_)                                                                                      \
                                                                                                                           \
				void NES_IO_CALL Poke_Member_##a_(Address,Data);                                                           \
                                                                                                                           \
				template<typename T>                                                                                       \
				static void Poke_Type_##a_(void* instance,Address address,Data data,void (NES_IO_CALL T::*)(Address,Data)) \
				{                                                                                                          \
					static_cast<T*>(instance)->Poke_Member_##a_( address, data );                                          \
				}                                                                                                          \
                                                                                                                           \
				static void NES_IO_CALL Poke_##a_(void* instance,Address address,Data data)                                \
				{                                                                                                          \
					Poke_Type_##a_( instance, address, data, &Poke_Member_##a_ );                                          \
				}

			#define NES_PEEK(o_,a_) Data NES_IO_CALL o_::Peek_Member_##a_(Address address)
			#define NES_POKE(o_,a_) void NES_IO_CALL o_::Poke_Member_##a_(Address address,Data data)

			#define NES_CALL_POKE(o_,a_,p_,d_) o_::Poke_Member_##a_( p_, d_ )
			#define NES_CALL_PEEK(o_,a_,p_)    o_::Peek_Member_##a_( p_)

		#endif
		}
	}
}

#endif
