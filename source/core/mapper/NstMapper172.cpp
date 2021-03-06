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
#include "NstMapper172.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper172::SubReset(const bool hard)
		{
			if (hard)
				regs[3] = regs[2] = regs[1] = regs[0] = 0;

			Map( 0x4100U,          &Mapper172::Peek_4100 );
			Map( 0x4100U, 0x4103U, &Mapper172::Poke_4100 );
			Map( 0x8000U, 0xFFFFU, &Mapper172::Poke_8000 );
		}

		void Mapper172::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					state.Read( regs );

				state.End();
			}
		}

		void Mapper172::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write( regs ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper172,4100)
		{
			return (regs[1] ^ regs[2]) | 0x40;
		}

		NES_POKE(Mapper172,4100)
		{
			regs[address & 0x3] = data;
		}

		NES_POKE(Mapper172,8000)
		{
			ppu.Update();

			chr.SwapBank<SIZE_8K,0x0000U>
			(
				((data^regs[2]) >> 3 & 0x2) |
				((data^regs[2]) >> 5 & 0x1)
			);
		}
	}
}
