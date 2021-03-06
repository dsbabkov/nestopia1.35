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

#ifndef NST_WINDOW_USER_H
#define NST_WINDOW_USER_H

#pragma once

#include "NstMain.hpp"
#include "NstString.hpp"

namespace Nestopia
{
	namespace Window
	{
		namespace User
		{
			enum Type
			{
				FAIL,
				WARN,
				INFORM,
				CONFIRM
			};

			void  Fail    (uint,uint=0);
			void  Fail    (tstring,uint=0);
			void  Fail    (tstring,tstring);
			void  Warn    (uint,uint=0);
			void  Warn    (tstring,uint=0);
			void  Warn    (tstring,tstring);
			void  Inform  (uint,uint=0);
			void  Inform  (tstring,uint=0);
			void  Inform  (tstring,tstring);
			ibool Confirm (uint,uint=0);
			ibool Confirm (tstring,uint=0);
			ibool Confirm (tstring,tstring);
			ibool Input   (HeapString&,tstring,tstring=NULL);
		}
	}
}

#endif
