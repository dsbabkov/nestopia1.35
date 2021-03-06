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

#ifndef NST_BOARDS_MMC3_H
#define NST_BOARDS_MMC3_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "../NstClock.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Mmc3 : public Mapper
			{
				class BaseIrq
				{
				public:

					void Reset(bool);
					void LoadState(State::Loader&);
					void SaveState(State::Saver&) const;

				private:

					uint count;
					uint latch;
					ibool reload;
					ibool enabled;
					const ibool persistant;

				public:

					BaseIrq(bool p=false)
					: persistant(p) {}

					ibool Signal()
					{
						uint tmp = count;

						if (!count || reload)
						{
							count = latch;
							reload = false;
						}
						else
						{
							--count;
						}

						return (tmp | persistant) && !count && enabled;
					}

					void SetLatch(uint data)
					{
						latch = data;
					}

					void Reload()
					{
						reload = true;
					}

					void Enable()
					{
						enabled = true;
					}

					void Disable(Cpu& cpu)
					{
						enabled = false;
						cpu.ClearIRQ();
					}
				};

			public:

				struct Irq : Clock::A12<BaseIrq>
				{
					enum
					{
						SIGNAL_DURATION = 16
					};

					Irq(Cpu&,Ppu&,IrqDelay=NO_IRQ_DELAY,bool=false);
				};

			protected:

				enum Board
				{
					BRD_GENERIC,
					BRD_TEROM,
					BRD_TFROM,
					BRD_TGROM,
					BRD_TKROM,
					BRD_TLROM,
					BRD_TL1ROM,
					BRD_TLSROM,
					BRD_TR1ROM,
					BRD_TSROM,
					BRD_TVROM
				};

				enum Revision
				{
					REV_A,
					REV_B,
					REV_C
				};

				Mmc3(Context&,Board=BRD_GENERIC,uint=WRAM_AUTO,Revision=REV_B);

				void SubReset(bool);

				NES_DECL_POKE( 8000 )
				NES_DECL_POKE( 8001 )
				NES_DECL_POKE( A001 )
				NES_DECL_POKE( C000 )
				NES_DECL_POKE( C001 )
				NES_DECL_POKE( E000 )
				NES_DECL_POKE( E001 )

				virtual void UpdatePrg();
				virtual void UpdateChr() const;

				struct Regs
				{
					enum
					{
						CTRL0_MODE          = b00000111,
						CTRL0_XOR_PRG       = b01000000,
						CTRL0_XOR_CHR       = b10000000,
						CTRL1_WRAM_READONLY = b01000000,
						CTRL1_WRAM_ENABLED  = b10000000,
						CTRL1_WRAM          = CTRL1_WRAM_ENABLED|CTRL1_WRAM_READONLY
					};

					void Reset();

					uint ctrl0;
					uint ctrl1;
				};

				struct Banks
				{
					void Reset();

					uint chr[6];
					uint prg[4];
				};

				Regs regs;

				union
				{
					Banks banks;
					uint bankBlock[10];
				};

			private:

				NST_COMPILE_ASSERT( sizeof(Banks) == sizeof(uint) * 10 );

				static uint BoardToWRam(Board,uint);

				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void VSync();

				Irq irq;
			};
		}
	}
}

#endif
