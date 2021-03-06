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
#include "NstMapper215.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper215::Mapper215(Context& c)
		:
		Mmc3     (c,BRD_GENERIC,PROM_MAX_512K|WRAM_NONE),
		ctrl6000 (c.attribute == ATR_6000_CTRL)
		{}

		void Mapper215::SubReset(const bool hard)
		{
			exRegs[0] = 0x00;
			exRegs[1] = 0xFF;
			exRegs[2] = 0x04;
			exRegs[3] = false;

			Mmc3::SubReset( hard );

			Map( 0x5000U, &Mapper215::Poke_5000 );
			Map( 0x5001U, &Mapper215::Poke_5001 );
			Map( 0x5007U, &Mapper215::Poke_5007 );

			if (ctrl6000)
			{
				Map( 0x6000U, &Mapper215::Poke_5000 );
				Map( 0x6001U, &Mapper215::Poke_5001 );
				Map( 0x6007U, &Mapper215::Poke_5007 );
			}

			for (uint i=0x0000U; i < 0x2000U; i += 0x2)
			{
				Map( 0x8000U + i, &Mapper215::Poke_8000 );
				Map( 0x8001U + i, &Mapper215::Poke_8001 );
				Map( 0xA000U + i, &Mapper215::Poke_A000 );
				Map( 0xC000U + i, &Mapper215::Poke_C000 );
				Map( 0xC001U + i, &Mapper215::Poke_C001 );
				Map( 0xE001U + i, &Mapper215::Poke_E001 );
			}
		}

		void Mapper215::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == NES_STATE_CHUNK_ID('R','E','G','\0'))
				{
					const State::Loader::Data<4> data( state );

					exRegs[0] = data[0];
					exRegs[1] = data[1];
					exRegs[2] = data[2];
					exRegs[3] = data[3] & 0x1;
				}

				state.End();
			}
		}

		void Mapper215::SubSave(State::Saver& state) const
		{
			const u8 data[4] =
			{
				exRegs[0],
				exRegs[1],
				exRegs[2],
				exRegs[3]
			};

			state.Begin('R','E','G','\0').Write( data ).End();
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		uint Mapper215::GetPrgBank(const uint bank) const
		{
			if (exRegs[1] & 0x8)
				return (bank & 0x1F) | 0x20;
			else
				return (bank & 0x0F) | (exRegs[1] & 0x10);
		}

		void Mapper215::UpdatePrg()
		{
			if (exRegs[0] & 0x80)
			{
				const uint bank = (exRegs[0] & 0x0F) | (exRegs[1] & 0x10);
				prg.SwapBanks<SIZE_16K,0x0000U>( bank, bank );
			}
			else
			{
				const uint i = (regs.ctrl0 & Regs::CTRL0_XOR_PRG) >> 5;

				prg.SwapBanks<SIZE_8K,0x0000U>
				(
					GetPrgBank( banks.prg[i]   ),
					GetPrgBank( banks.prg[1]   ),
					GetPrgBank( banks.prg[i^2] ),
					GetPrgBank( banks.prg[3]   )
				);
			}
		}

		uint Mapper215::GetChrBank(const uint bank) const
		{
			if (exRegs[1] & 0x4)
				return bank | 0x100;
			else
				return (bank & 0x7F) | (exRegs[1] << 3 & 0x80);
		}

		void Mapper215::UpdateChr() const
		{
			ppu.Update();

			const uint swap = (regs.ctrl0 & Regs::CTRL0_XOR_CHR) << 5;

			chr.SwapBanks<SIZE_1K>
			(
				0x0000U ^ swap,
				GetChrBank( banks.chr[0] << 1 | 0x0 ),
				GetChrBank( banks.chr[0] << 1 | 0x1 ),
				GetChrBank( banks.chr[1] << 1 | 0x0 ),
				GetChrBank( banks.chr[1] << 1 | 0x1 )
			);

			chr.SwapBanks<SIZE_1K>
			(
				0x1000U ^ swap,
				GetChrBank( banks.chr[2] ),
				GetChrBank( banks.chr[3] ),
				GetChrBank( banks.chr[4] ),
				GetChrBank( banks.chr[5] )
			);
		}

		NES_POKE(Mapper215,5000)
		{
			if (exRegs[0] != data)
			{
				exRegs[0] = data;
				Mapper215::UpdatePrg();
			}
		}

		NES_POKE(Mapper215,5001)
		{
			if (exRegs[1] != data)
			{
				exRegs[1] = data;
				Mapper215::UpdateChr();
			}
		}

		NES_POKE(Mapper215,5007)
		{
			exRegs[2] = data;

			regs.ctrl0 = 0;
			Mapper215::UpdatePrg();
			Mapper215::UpdateChr();
		}

		NES_POKE(Mapper215,8000)
		{
			if (!exRegs[2])
				NES_CALL_POKE(Mmc3,8000,0x8000U,data);
		}

		NES_POKE(Mapper215,8001)
		{
			if (exRegs[2])
			{
				if (exRegs[3] && ((exRegs[0] & 0x80) == 0 || (regs.ctrl0 & Regs::CTRL0_MODE) < 6))
				{
					exRegs[3] = false;
					NES_CALL_POKE(Mmc3,8001,0x8001U,data);
				}
			}
			else
			{
				NES_CALL_POKE(Mmc3,8001,0x8001U,data);
			}
		}

		NES_POKE(Mapper215,A000)
		{
			if (exRegs[2])
			{
				static const u8 security[8] = {0,2,5,3,6,1,7,4};

				data = (data & 0xC0) | security[data & 0x07];
				exRegs[3] = true;

				NES_CALL_POKE(Mmc3,8000,0x8000U,data);
			}
			else
			{
				SetMirroringHV( data );
			}
		}

		NES_POKE(Mapper215,C000)
		{
			if (exRegs[2])
				SetMirroringHV( data >> 7 | data );
			else
				NES_CALL_POKE(Mmc3,C000,0xC000U,data);
		}

		NES_POKE(Mapper215,C001)
		{
			if (exRegs[2])
				NES_CALL_POKE(Mmc3,E001,0xE001U,data);
			else
				NES_CALL_POKE(Mmc3,C001,0xC001U,data);
		}

		NES_POKE(Mapper215,E001)
		{
			if (exRegs[2])
			{
				NES_CALL_POKE(Mmc3,C000,0xC000U,data);
				NES_CALL_POKE(Mmc3,C001,0xC001U,data);
			}
			else
			{
				NES_CALL_POKE(Mmc3,E001,0xE001U,data);
			}
		}
	}
}
