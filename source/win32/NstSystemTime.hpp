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

#ifndef NST_SYSTEM_TIME_H
#define NST_SYSTEM_TIME_H

#pragma once

#include "NstString.hpp"

namespace Nestopia
{
	namespace System
	{
		class Time
		{
			void SetLocal(const void*);

		public:

			Time();

			void Set();
			ibool Set(tstring);
			void Clear();
			HeapString ToString(ibool=false) const;

			ibool Almost(const Time&) const;
			bool operator <  (const Time&) const;
			bool operator == (const Time&) const;

			u16 year;
			u8 month;
			u8 day;
			u8 hour;
			u8 minute;
			u8 second;
			u16 milli;
		};
	}
}

#endif
