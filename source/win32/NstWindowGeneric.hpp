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

#ifndef NST_WINDOW_GENERIC_H
#define NST_WINDOW_GENERIC_H

#pragma once

#include "NstWindowRect.hpp"

namespace Nestopia
{
	namespace Window
	{
		class Generic
		{
		protected:

			HWND hWnd;

		public:

			Generic(HWND h=NULL)
			: hWnd(h) {}

			Generic& operator = (HWND h)
			{
				hWnd = h;
				return *this;
			}

			operator HWND () const
			{
				return hWnd;
			}

		private:

			class SizeProxy
			{
				HWND const hWnd;

				void Set(const Point&) const;

			public:

				SizeProxy(HWND h)
				: hWnd(h) {}

				void operator  = (const Point&) const;
				void operator += (const Point&) const;
				void operator -= (const Point&) const;

				Point Coordinates() const;

				operator Point () const
				{
					return Coordinates();
				}
			};

			class PositionProxy
			{
				HWND const hWnd;

				void Set(Point) const;

			public:

				PositionProxy(HWND h)
				: hWnd(h) {}

				void operator  = (const Point&) const;
				void operator += (const Point&) const;
				void operator -= (const Point&) const;

				void BringBehind(HWND) const;

				Point Coordinates() const;

				operator Point () const
				{
					return Coordinates();
				}
			};

			class StyleProxy
			{
				HWND const hWnd;
				const ibool ex;
				const long flags;

			public:

				StyleProxy(HWND h,ibool e,long f)
				: hWnd(h), ex(e), flags(f) {}

				void operator = (bool) const;
				operator long () const;
			};

		public:

			Rect::Window Coordinates() const
			{
				return hWnd;
			}

			Point::Client ClientCoordinates() const
			{
				return hWnd;
			}

			Point::Picture PictureCoordinates() const
			{
				return hWnd;
			}

			Point::NonClient NonClientCoordinates() const
			{
				return hWnd;
			}

			SizeProxy Size() const
			{
				return hWnd;
			}

			PositionProxy Position() const
			{
				return hWnd;
			}

			StyleProxy Style(long flags) const
			{
				return StyleProxy( hWnd, false, flags );
			}

			StyleProxy ExStyle(long flags) const
			{
				return StyleProxy( hWnd, true, flags );
			}

			void Maximize() const;
			void Minimize() const;
			void Restore() const;

			ibool Enabled() const;
			ibool Active() const;
			ibool Focused() const;
			ibool Maximized() const;
			ibool Minimized() const;
			ibool Restored() const;
			ibool Visible() const;

			Rect GetPlacement() const;
			void SetPlacement(const Rect&) const;

			ibool SameThread() const;

			ibool Enable(ibool=true) const;
			void  Show(ibool=true) const;
			ibool Activate() const;
			void  Redraw(ibool=true) const;
			void  Close() const;
			ibool Destroy();

			void MakeTopMost(ibool=true) const;
			void MakeWindowed(long,long) const;
			void MakeFullscreen() const;

			Generic Parent() const;

			LONG_PTR Send(uint) const;
			void     Post(uint) const;

			void SendCommand(uint) const;
			void PostCommand(uint) const;

			template<typename T,typename U>
			LONG_PTR Send(uint msg,T t,U u) const
			{
				NST_COMPILE_ASSERT( sizeof(t) <= sizeof(WPARAM) && sizeof(u) <= sizeof(LPARAM) );
				return ::SendMessage( hWnd, msg, (WPARAM) t, (LPARAM) u );
			}

			template<typename T,typename U>
			void Post(uint msg,T t,U u) const
			{
				NST_COMPILE_ASSERT( sizeof(T) <= sizeof(WPARAM) && sizeof(U) <= sizeof(LPARAM) );
				::PostMessage( hWnd, msg, (WPARAM) t, (LPARAM) u );
			}

			static Generic Find(tstring);

			class Stream
			{
				HWND const hWnd;

				int GetTextLength(const char*) const;
				int GetTextLength(const wchar_t*) const;

				int GetText(char*,uint) const;
				int GetText(wchar_t*,uint) const;

			public:

				Stream(Generic g)
				: hWnd(g.hWnd) {}

				void operator << (const char*) const;
				void operator << (const wchar_t*) const;
				void operator << (int) const;
				void operator << (uint) const;

				template<typename T>
				uint operator >> (T&) const;

				uint operator >> (long&) const;
				uint operator >> (ulong&) const;

				uint operator >> ( schar&  i ) const { long  t; const uint r = operator >> (t); i = ( schar  ) t; return r; }
				uint operator >> ( uchar&  i ) const { ulong t; const uint r = operator >> (t); i = ( uchar  ) t; return r; }
				uint operator >> ( short&  i ) const { long  t; const uint r = operator >> (t); i = ( short  ) t; return r; }
				uint operator >> ( ushort& i ) const { ulong t; const uint r = operator >> (t); i = ( ushort ) t; return r; }
				uint operator >> ( int&    i ) const { long  t; const uint r = operator >> (t); i = ( int    ) t; return r; }
				uint operator >> ( uint&   i ) const { ulong t; const uint r = operator >> (t); i = ( uint   ) t; return r; }

				void Clear() const;
			};

			Stream Text() const
			{
				return *this;
			}

			class LockDraw
			{
				HWND const hWnd;

			public:

				LockDraw(HWND);
				~LockDraw();
			};
		};

		template<typename T>
		uint Generic::Stream::operator >> (T& string) const
		{
			const int size = GetTextLength( string.Ptr() );

			if (size > 0)
			{
				string.Resize( size );

				if (GetText( string.Ptr(), string.Length() ) > 0)
					return string.Validate();
			}

			string.Clear();
			return 0;
		}
	}
}

#endif
