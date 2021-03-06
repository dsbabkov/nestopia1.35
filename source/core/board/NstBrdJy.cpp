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
#include "../NstClock.hpp"
#include "NstBrdJy.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Jy::CartSwitches::CartSwitches(uint d,bool l)
			: data(d), ppuLatched(l) {}

			inline ibool Jy::CartSwitches::IsPpuLatched() const
			{
				return ppuLatched;
			}

			Jy::Irq::A12::A12(Irq& irq)
			: base(irq) {}

			Jy::Irq::M2::M2(Irq& irq)
			: base(irq) {}

			#ifdef _MSC_VER
			#pragma warning( push )
			#pragma warning( disable : 4355 )
			#endif

			Jy::Irq::Irq(Cpu& cpu,Ppu& ppu)
			: a12(cpu,ppu,0,Clock::A12<A12>::NO_IRQ_DELAY,*this), m2(cpu,*this) {}

			#ifdef _MSC_VER
			#pragma warning( pop )
			#endif

			Jy::Jy(Context& c,const DefaultDipSwitch d,bool l)
			:
			Mapper       (c,WRAM_NONE),
			irq          (c.cpu,c.ppu),
			cartSwitches (d,l)
			{}

			void Jy::Regs::Reset()
			{
				mul[0] = 0;
				mul[1] = 0;
				tmp = 0;
				ctrl[0] = 0;
				ctrl[1] = 0;
				ctrl[2] = 0;
				ctrl[3] = 0;
			}

			void Jy::Banks::Reset()
			{
				for (uint i=0; i < 4; ++i)
					prg[i] = 0xFF;

				for (uint i=0; i < 8; ++i)
					chr[i] = 0xFFFFU;

				for (uint i=0; i < 4; ++i)
					nmt[i] = 0x00;

				prg6 = NULL;

				chrLatch[0] = 0;
				chrLatch[1] = 4;
			}

			void Jy::Irq::A12::Reset(bool)
			{
			}

			void Jy::Irq::M2::Reset(bool)
			{
			}

			void Jy::Irq::Reset()
			{
				enabled = false;
				mode = 0;
				prescaler = 0;
				scale = 0xFF;
				count = 0;
				flip = 0;

				a12.Reset( true, true );
				m2.Reset( true, true );
			}

			void Jy::SubReset(bool)
			{
				for (uint i=0x5000U; i < 0x5800U; i += 0x4)
					Map( i, &Jy::Peek_5000 );

				for (uint i=0x5800U; i < 0x6000U; i += 0x4)
				{
					cpu.Map( i + 0x0 ).Set( &regs, &Jy::Regs::Peek_5800, &Jy::Regs::Poke_5800 );
					cpu.Map( i + 0x1 ).Set( &regs, &Jy::Regs::Peek_5801, &Jy::Regs::Poke_5801 );
					cpu.Map( i + 0x3 ).Set( &regs, &Jy::Regs::Peek_5803, &Jy::Regs::Poke_5803 );
				}

				Map( 0x6000U, 0x7FFFU, &Jy::Peek_6000 );
				Map( 0x8000U, 0x8FFFU, &Jy::Poke_8000 );
				Map( 0x9000U, 0x9FFFU, &Jy::Poke_9000 );
				Map( 0xA000U, 0xAFFFU, &Jy::Poke_A000 );

				for (uint i=0x0000U; i < 0x1000U; i += 0x8)
				{
					Map( 0xB000U + i, 0xB003U + i, &Jy::Poke_B000 );
					Map( 0xB004U + i, 0xB007U + i, &Jy::Poke_B004 );

					Map( 0xC000U + i, &Jy::Poke_C000 );
					Map( 0xC001U + i, &Jy::Poke_C001 );
					Map( 0xC002U + i, &Jy::Poke_C002 );
					Map( 0xC003U + i, &Jy::Poke_C003 );
					Map( 0xC004U + i, &Jy::Poke_C004 );
					Map( 0xC005U + i, &Jy::Poke_C005 );
					Map( 0xC006U + i, &Jy::Poke_C006 );
				}

				for (uint i=0x0000U; i < 0x1000U; i += 0x4)
				{
					Map( 0xD000U + i, &Jy::Poke_D000 );
					Map( 0xD001U + i, &Jy::Poke_D001 );
					Map( 0xD002U + i, &Jy::Poke_D002 );
					Map( 0xD003U + i, &Jy::Poke_D003 );
				}

				regs.Reset();
				banks.Reset();
				irq.Reset();

				ppu.SetBgHook( Hook(this,&Jy::Hook_PpuBg) );
				ppu.SetSpHook( Hook(this,&Jy::Hook_PpuSp) );

				if (cartSwitches.IsPpuLatched())
				{
					chr.SetAccessor( 0, this, &Jy::Access_Chr_0000 );
					chr.SetAccessor( 1, this, &Jy::Access_Chr_1000 );
				}

				UpdatePrg();
				UpdateExChr();
				UpdateChr();
				UpdateNmt();
			}

			void Jy::BaseLoad(State::Loader& state,const dword id)
			{
				NST_VERIFY( id == NES_STATE_CHUNK_ID('B','T','K','\0') );

				if (id == NES_STATE_CHUNK_ID('B','T','K','\0'))
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case NES_STATE_CHUNK_ID('R','E','G','\0'):
							{
								const State::Loader::Data<35> data( state );

								regs.ctrl[0] = data[0];
								regs.ctrl[1] = data[1];
								regs.ctrl[2] = data[2];
								regs.ctrl[3] = data[3];
								regs.mul[0]  = data[4];
								regs.mul[1]  = data[5];
								regs.tmp     = data[6];
								banks.prg[0] = data[7];
								banks.prg[1] = data[8];
								banks.prg[2] = data[9];
								banks.prg[3] = data[10];
								banks.chr[0] = data[11] | (data[12] << 8);
								banks.chr[1] = data[13] | (data[14] << 8);
								banks.chr[2] = data[15] | (data[16] << 8);
								banks.chr[3] = data[17] | (data[18] << 8);
								banks.chr[4] = data[19] | (data[20] << 8);
								banks.chr[5] = data[21] | (data[22] << 8);
								banks.chr[6] = data[23] | (data[24] << 8);
								banks.chr[7] = data[25] | (data[26] << 8);
								banks.nmt[0] = data[27] | (data[28] << 8);
								banks.nmt[1] = data[29] | (data[30] << 8);
								banks.nmt[2] = data[31] | (data[32] << 8);
								banks.nmt[3] = data[33] | (data[34] << 8);

								UpdatePrg();
								UpdateExChr();
								UpdateChr();
								UpdateNmt();

								break;
							}

							case NES_STATE_CHUNK_ID('L','A','T','\0'):

								NST_VERIFY( cartSwitches.IsPpuLatched() );

								if (cartSwitches.IsPpuLatched())
								{
									banks.chrLatch[0] = state.Read8();
									banks.chrLatch[1] = banks.chrLatch[0] >> 3 & 0x7;
									banks.chrLatch[0] &= 0x7;

									UpdateChr();
								}
								break;

							case NES_STATE_CHUNK_ID('I','R','Q','\0'):
							{
								u8 data[5];
								state.Read( data );

								irq.enabled   = data[0] & 0x1;
								irq.mode      = data[1];
								irq.prescaler = data[2];
								irq.count     = data[3];
								irq.flip      = data[4];

								irq.scale = (irq.mode & Irq::MODE_SCALE_3BIT) ? 0x7 : 0xFF;

								break;
							}
						}

						state.End();
					}
				}
			}

			void Jy::BaseSave(State::Saver& state) const
			{
				state.Begin('B','T','K','\0');

				{
					const u8 data[35] =
					{
						regs.ctrl[0],
						regs.ctrl[1],
						regs.ctrl[2],
						regs.ctrl[3],
						regs.mul[0],
						regs.mul[1],
						regs.tmp,
						banks.prg[0],
						banks.prg[1],
						banks.prg[2],
						banks.prg[3],
						banks.chr[0] & 0xFF,
						banks.chr[0] >> 8,
						banks.chr[1] & 0xFF,
						banks.chr[1] >> 8,
						banks.chr[2] & 0xFF,
						banks.chr[2] >> 8,
						banks.chr[3] & 0xFF,
						banks.chr[3] >> 8,
						banks.chr[4] & 0xFF,
						banks.chr[4] >> 8,
						banks.chr[5] & 0xFF,
						banks.chr[5] >> 8,
						banks.chr[6] & 0xFF,
						banks.chr[6] >> 8,
						banks.chr[7] & 0xFF,
						banks.chr[7] >> 8,
						banks.nmt[0] & 0xFF,
						banks.nmt[0] >> 8,
						banks.nmt[1] & 0xFF,
						banks.nmt[1] >> 8,
						banks.nmt[2] & 0xFF,
						banks.nmt[2] >> 8,
						banks.nmt[3] & 0xFF,
						banks.nmt[3] >> 8
					};

					state.Begin('R','E','G','\0').Write( data ).End();
				}

				if (cartSwitches.IsPpuLatched())
					state.Begin('L','A','T','\0').Write8( banks.chrLatch[0] | banks.chrLatch[1] << 3 ).End();

				{
					const u8 data[5] =
					{
						irq.enabled != 0,
						irq.mode,
						irq.prescaler & 0xFF,
						irq.count,
						irq.flip
					};

					state.Begin('I','R','Q','\0').Write( data ).End();
				}

				state.End();
			}

			uint Jy::CartSwitches::NumDips() const
			{
				return 2;
			}

			uint Jy::CartSwitches::NumValues(uint i) const
			{
				NST_ASSERT( i < 2 );
				return (i == 0) ? 4 : 3;
			}

			cstring Jy::CartSwitches::GetDipName(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? "Game Select" : "Extended Mirroring";
			}

			cstring Jy::CartSwitches::GetValueName(uint dip,uint value) const
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 4 );
					return (value == 0) ? "1" : (value == 1) ? "2" : (value == 2) ? "3" : "4";
				}
				else
				{
					NST_ASSERT( value < 3 );
					return (value == 0) ? "Off" : (value == 1) ? "Controlled" : "On";
				}
			}

			uint Jy::CartSwitches::GetValue(uint dip) const
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
					return data >> 6;
				else
					return data & DIPSWITCH_NMT;
			}

			bool Jy::CartSwitches::SetValue(uint dip,uint value)
			{
				NST_ASSERT( dip < 2 );

				const uint prev = data;

				if (dip == 0)
				{
					NST_ASSERT( value < 4 );
					data = (data & ~uint(DIPSWITCH_GAME)) | (value << 6);
				}
				else
				{
					NST_ASSERT( value < 3 );
					data = (data & ~uint(DIPSWITCH_NMT)) | value;
				}

				return prev != data;
			}

			Jy::Device Jy::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return &cartSwitches;
				else
					return Mapper::QueryDevice( type );
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			inline uint Jy::CartSwitches::GetSetting() const
			{
				return data;
			}

			inline void Jy::Irq::Update()
			{
				a12.Update();
				m2.Update();
			}

			ibool Jy::Irq::IsEnabled() const
			{
				return enabled &&
				(
					(mode & MODE_COUNT_ENABLE) == MODE_COUNT_DOWN ||
					(mode & MODE_COUNT_ENABLE) == MODE_COUNT_UP
				);
			}

			ibool Jy::Irq::IsEnabled(uint checkMode) const
			{
				return (mode & MODE_SOURCE) == checkMode && IsEnabled();
			}

			ibool Jy::Irq::Signal()
			{
				NST_ASSERT( IsEnabled() );

				if (mode & MODE_COUNT_DOWN)
					return (--prescaler & scale) == scale && (count-- & 0xFF) == 0x00;
				else
					return (++prescaler & scale) == 0x00 && (++count & 0xFF) == 0x00;
			}

			ibool Jy::Irq::A12::Signal()
			{
				return base.IsEnabled(MODE_PPU_A12) && base.Signal();
			}

			ibool Jy::Irq::M2::Signal()
			{
				return base.IsEnabled(MODE_M2) && base.Signal();
			}

			uint Jy::Banks::Unscramble(const uint bank)
			{
				return
				(
					((bank & 0x01) << 6) |
					((bank & 0x02) << 4) |
					((bank & 0x04) << 2) |
					((bank & 0x10) >> 2) |
					((bank & 0x20) >> 4) |
					((bank & 0x40) >> 6)
				);
			}

			NES_HOOK(Jy,PpuBg)
			{
				if (irq.IsEnabled(Irq::MODE_PPU_READ) && ppu.IsEnabled())
				{
					for (uint i=0, hit=false; i < (32*4) * 2; i += 2)
					{
						if (irq.Signal() && !hit)
						{
							hit = true;
							cpu.DoIRQ( Cpu::IRQ_EXT, cpu.GetMasterClockCycles() + ppu.GetOneCycle() * i );
						}
					}
				}
			}

			NES_HOOK(Jy,PpuSp)
			{
				if (irq.IsEnabled(Irq::MODE_PPU_READ) && ppu.IsEnabled())
				{
					for (uint i=0, hit=false; i < (8*4+2*4+2) * 2; i += 2)
					{
						if (irq.Signal() && !hit)
						{
							hit = true;
							cpu.DoIRQ( Cpu::IRQ_EXT, cpu.GetMasterClockCycles() + ppu.GetOneCycle() * i );
						}
					}
				}
			}

			NES_ACCESSOR(Jy,Chr_0000)
			{
				const uint data = chr.Peek( address );
				address &= 0xFF8;

				if (address == 0xFD8 || address == 0xFE8)
				{
					banks.chrLatch[0] = address >> 4 & 0x2;

					if ((regs.ctrl[0] & Regs::CTRL0_CHR_MODE) == Regs::CTRL0_CHR_SWAP_4K)
						UpdateChrLatch();
				}

				return data;
			}

			NES_ACCESSOR(Jy,Chr_1000)
			{
				const uint data = chr.Peek( address );
				address &= 0xFF8;

				if (address == 0xFD8 || address == 0xFE8)
				{
					banks.chrLatch[1] = address >> 4 & (0x2|0x4);

					if ((regs.ctrl[0] & Regs::CTRL0_CHR_MODE) == Regs::CTRL0_CHR_SWAP_4K)
						UpdateChrLatch();
				}

				return data;
			}

			NES_PEEK(Jy,5000)
			{
				return (cartSwitches.GetSetting() & DIPSWITCH_GAME) | (address >> 8 & ~uint(DIPSWITCH_GAME));
			}

			NES_POKE(Jy::Regs,5800) { mul[0] = data; }
			NES_POKE(Jy::Regs,5801) { mul[1] = data; }
			NES_POKE(Jy::Regs,5803) { tmp = data;  }

			NES_PEEK(Jy::Regs,5800) { return (mul[0] * mul[1]) & 0xFF; }
			NES_PEEK(Jy::Regs,5801) { return (mul[0] * mul[1]) >> 8;   }
			NES_PEEK(Jy::Regs,5803) { return tmp; }

			NES_PEEK(Jy,6000)
			{
				NST_VERIFY( banks.prg6 );
				return banks.prg6 ? banks.prg6[address - 0x6000U] : (address >> 8);
			}

			NES_POKE(Jy,8000)
			{
				address &= 0x3;
				data &= 0x3F;

				if (banks.prg[address] != data)
				{
					banks.prg[address] = data;
					UpdatePrg();
				}
			}

			NES_POKE(Jy,9000)
			{
				address &= 0x7;
				data |= banks.chr[address] & 0xFF00U;

				if (banks.chr[address] != data)
				{
					banks.chr[address] = data;
					UpdateChr();
				}
			}

			NES_POKE(Jy,A000)
			{
				address &= 0x7;
				data = (data << 8) | (banks.chr[address] & 0x00FFU);

				if (banks.chr[address] != data)
				{
					banks.chr[address] = data;
					UpdateChr();
				}
			}

			NES_POKE(Jy,B000)
			{
				address &= 0x3;
				data |= banks.nmt[address] & 0xFF00U;

				if (banks.nmt[address] != data)
				{
					banks.nmt[address] = data;
					UpdateNmt();
				}
			}

			NES_POKE(Jy,B004)
			{
				address &= 0x3;
				data = (data << 8) | (banks.nmt[address] & 0x00FFU);

				if (banks.nmt[address] != data)
				{
					banks.nmt[address] = data;
					UpdateNmt();
				}
			}

			NES_POKE(Jy,C000)
			{
				data &= Irq::TOGGLE;

				if (irq.enabled != data)
				{
					irq.Update();
					irq.enabled = data;

					if (!data)
						cpu.ClearIRQ();
				}
			}

			NES_POKE(Jy,C001)
			{
				if (irq.mode != data)
				{
					irq.Update();

					NST_VERIFY
					(
						(data & Irq::MODE_SCALE_ADJUST) == 0 &&
						(
							(data & Irq::MODE_SOURCE) == Irq::MODE_M2 ||
							(data & Irq::MODE_SOURCE) == Irq::MODE_PPU_A12 ||
							(data & Irq::MODE_SOURCE) == Irq::MODE_PPU_READ
						)
					);

					irq.mode = data;
					irq.scale = (data & Irq::MODE_SCALE_3BIT) ? 0x7 : 0xFF;
				}
			}

			NES_POKE(Jy,C002)
			{
				if (irq.enabled)
				{
					irq.Update();
					irq.enabled = false;
					cpu.ClearIRQ();
				}
			}

			NES_POKE(Jy,C003)
			{
				if (!irq.enabled)
				{
					irq.Update();
					irq.enabled = true;
				}
			}

			NES_POKE(Jy,C004)
			{
				irq.Update();
				irq.prescaler = data ^ irq.flip;
			}

			NES_POKE(Jy,C005)
			{
				irq.Update();
				irq.count = data ^ irq.flip;
			}

			NES_POKE(Jy,C006)
			{
				irq.flip = data;
			}

			NES_POKE(Jy,D000)
			{
				if (regs.ctrl[0] != data)
				{
					regs.ctrl[0] = data;
					UpdatePrg();
					UpdateExChr();
					UpdateChr();
					UpdateNmt();
				}
			}

			NES_POKE(Jy,D001)
			{
				if (regs.ctrl[1] != data)
				{
					regs.ctrl[1] = data;
					UpdateNmt();
				}
			}

			NES_POKE(Jy,D002)
			{
				if (regs.ctrl[2] != data)
				{
					regs.ctrl[2] = data;
					UpdateNmt();
				}
			}

			NES_POKE(Jy,D003)
			{
				if (regs.ctrl[3] != data)
				{
					regs.ctrl[3] = data;
					UpdatePrg();
					UpdateExChr();
					UpdateChr();
				}
			}

			void Jy::UpdatePrg()
			{
				NST_VERIFY( (regs.ctrl[0] & Regs::CTRL0_PRG_MODE) != Regs::CTRL0_PRG_SWAP_8K_R );

				const uint exPrg = (regs.ctrl[3] & Regs::CTRL3_EX_PRG) << 5;

				if (!(regs.ctrl[0] & Regs::CTRL0_PRG6_ENABLE))
				{
					banks.prg6 = NULL;
				}
				else
				{
					uint bank = banks.prg[3];

					switch (regs.ctrl[0] & Regs::CTRL0_PRG_MODE)
					{
						case Regs::CTRL0_PRG_SWAP_32K:  bank = (bank << 2) | 0x3; break;
						case Regs::CTRL0_PRG_SWAP_16K:  bank = (bank << 1) | 0x1; break;
						case Regs::CTRL0_PRG_SWAP_8K_R: bank = banks.Unscramble( bank ); break;
					}

					banks.prg6 = prg.Source().Mem( ((bank & 0x3F) | exPrg) * SIZE_8K );
				}

				const uint last = (regs.ctrl[0] & Regs::CTRL0_PRG_NOT_LAST) ? banks.prg[3] : 0x3F;

				switch (regs.ctrl[0] & Regs::CTRL0_PRG_MODE)
				{
					case Regs::CTRL0_PRG_SWAP_32K:

						prg.SwapBank<SIZE_32K,0x0000U>
						(
							(last & 0xF) | (exPrg >> 2)
						);
						break;

					case Regs::CTRL0_PRG_SWAP_16K:

						prg.SwapBanks<SIZE_16K,0x0000U>
						(
							(banks.prg[1] & 0x1F) | (exPrg >> 1),
							(last         & 0x1F) | (exPrg >> 1)
						);
						break;

					case Regs::CTRL0_PRG_SWAP_8K:

						prg.SwapBanks<SIZE_8K,0x0000U>
						(
							(banks.prg[0] & 0x3F) | exPrg,
							(banks.prg[1] & 0x3F) | exPrg,
							(banks.prg[2] & 0x3F) | exPrg,
							(last         & 0x3F) | exPrg
						);
						break;

					case Regs::CTRL0_PRG_SWAP_8K_R:

						prg.SwapBanks<SIZE_8K,0x0000U>
						(
							(banks.Unscramble( banks.prg[0] ) & 0x3F) | exPrg,
							(banks.Unscramble( banks.prg[1] ) & 0x3F) | exPrg,
							(banks.Unscramble( banks.prg[2] ) & 0x3F) | exPrg,
							(banks.Unscramble( last         ) & 0x3F) | exPrg
						);
						break;
				}
			}

			void Jy::UpdateExChr()
			{
				if (regs.ctrl[3] & Regs::CTRL3_NO_EX_CHR)
				{
					banks.exChr.mask = 0xFFFFU;
					banks.exChr.bank = 0x0000U;
				}
				else
				{
					const uint mode = (regs.ctrl[0] & Regs::CTRL0_CHR_MODE) >> 3;

					banks.exChr.mask = 0x00FFU >> (mode ^ 0x3);
					banks.exChr.bank = ((regs.ctrl[3] & Regs::CTRL3_EX_CHR_0) | ((regs.ctrl[3] & Regs::CTRL3_EX_CHR_1) >> 2)) << (mode + 5);
				}
			}

			void Jy::UpdateChr() const
			{
				ppu.Update();

				switch (regs.ctrl[0] & Regs::CTRL0_CHR_MODE)
				{
					case Regs::CTRL0_CHR_SWAP_8K:

						chr.SwapBank<SIZE_8K,0x0000U>
						(
							(banks.chr[0] & banks.exChr.mask) | banks.exChr.bank
						);
						break;

					case Regs::CTRL0_CHR_SWAP_4K:

						UpdateChrLatch();
						break;

					case Regs::CTRL0_CHR_SWAP_2K:

						chr.SwapBanks<SIZE_2K,0x0000U>
						(
							(banks.chr[0] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[2] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[4] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[6] & banks.exChr.mask) | banks.exChr.bank
						);
						break;

					case Regs::CTRL0_CHR_SWAP_1K:

						chr.SwapBanks<SIZE_1K,0x0000U>
						(
							(banks.chr[0] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[1] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[2] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[3] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[4] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[5] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[6] & banks.exChr.mask) | banks.exChr.bank,
							(banks.chr[7] & banks.exChr.mask) | banks.exChr.bank
						);
						break;
				}
			}

			void Jy::UpdateChrLatch() const
			{
				NST_ASSERT( (regs.ctrl[0] & Regs::CTRL0_CHR_MODE) == Regs::CTRL0_CHR_SWAP_4K );

				chr.SwapBanks<SIZE_4K,0x0000U>
				(
					(banks.chr[banks.chrLatch[0]] & banks.exChr.mask) | banks.exChr.bank,
					(banks.chr[banks.chrLatch[1]] & banks.exChr.mask) | banks.exChr.bank
				);
			}

			void Jy::UpdateNmt()
			{
				if ((regs.ctrl[0] >> 5 & cartSwitches.GetSetting() & 0x1) | (cartSwitches.GetSetting() & 0x2))
				{
					ppu.Update();

					for (uint i=0; i < 4; ++i)
						nmt.Source( (regs.ctrl[0] & Regs::CTRL0_NMT_CHR_ROM) || ((banks.nmt[i] ^ regs.ctrl[2]) & Regs::CTRL2_NMT_USE_RAM) ).SwapBank<SIZE_1K>( i * 0x0400, banks.nmt[i] );
				}
				else
				{
					SetMirroringVH01( regs.ctrl[1] );
				}
			}

			void Jy::VSync()
			{
				irq.a12.VSync();
				irq.m2.VSync();
			}
		}
	}
}
