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
#include "../board/NstBrdMmc3.hpp"
#include "NstMapper254.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		void Mapper254::SubReset(const bool hard)
		{
			security = ~0U;

			Mmc3::SubReset( hard );

			for (uint i=0x8000U; i < 0xA000U; i += 0x2)
				Map( i, &Mapper254::Poke_8000 );

			Map( WRK_POKE );
			Map( 0x6000U, 0x7FFFU, &Mapper254::Peek_Wrk );
		}

		void Mapper254::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
					security = (state.Read8() & 0x1) ? ~0U : 0U;

				state.End();
			}
		}

		void Mapper254::SubSave(State::Saver& state) const
		{
			state.Begin('R','E','G','\0').Write8( security & 0x1 ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper254,Wrk)
		{
			return wrk[0][address - 0x6000U] ^ (regs.ctrl1 & security);
		}

		NES_POKE(Mapper254,8000)
		{
			security = 0U;
			NES_CALL_POKE(Mmc3,8000,address,data);
		}
	}
}
