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

#include "NstWindowCustom.hpp"
#include "NstCtrlEdit.hpp"
#include <Richedit.h>

namespace Nestopia
{
	namespace Window
	{
		namespace Control
		{
			void Edit::SetNumberOnly(const ibool numOnly) const
			{
				control.Style(ES_NUMBER) = numOnly;
			}

			void Edit::Limit(const uint length) const
			{
				control.Send( EM_SETLIMITTEXT, length, 0 );
			}
		}
	}
}
