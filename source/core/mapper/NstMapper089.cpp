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

#include "../NstMapper.hpp"
#include "NstMapper089.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper89::SubReset(bool)
		{
			for (dword i=0x8000U; i <= 0xFFFFUL; ++i)
			{
				if ((i & 0xFF00U) == 0xC000U)
					Map( i, &Mapper89::Poke_Prg );
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE(Mapper89,Prg)
		{
			ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_ONE : Ppu::NMT_ZERO );
			prg.SwapBank<SIZE_16K,0x0000U>( data >> 4 );
			chr.SwapBank<SIZE_8K,0x0000U>( (data >> 4 & 0x8) | (data & 0x7) );
		}
	}
}
