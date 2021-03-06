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

#include "../NstMachine.hpp"
#include "../NstCartridge.hpp"
#include "../NstImage.hpp"
#include "NstApiMachine.hpp"
#include "NstApiMovie.hpp"
#include "NstApiRewinder.hpp"

namespace Nes
{
	namespace Api
	{
		uint Machine::Is(uint what) const throw()
		{
			return emulator.state & what;
		}

		uint Machine::Is(uint what,uint that) const throw()
		{
			return (emulator.state & what) && (emulator.state & that);
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Result Machine::Load(std::istream& stream,uint type)
		{
			const ibool power = emulator.state & ON;

			Unload();

			Result result = emulator.Load( &stream, type );

			if (NES_SUCCEEDED(result))
			{
				switch (emulator.image->GetType())
				{
					case Core::Image::DISK:

						emulator.state |= DISK;
						break;

					case Core::Image::SOUND:

						emulator.state |= SOUND;
						break;

					default:

						emulator.state |= CARTRIDGE;

						if (static_cast<const Core::Cartridge*>(emulator.image)->IsVS())
						{
							emulator.state |= VS;
						}
						else if (static_cast<const Core::Cartridge*>(emulator.image)->GetInfo().setup.system == SYSTEM_PC10)
						{
							emulator.state |= PC10;
						}
						break;
				}

				if (power)
				{
					Result tmp = Power( true );

					if (result > tmp)
						result = tmp;
				}
			}

			return result;
		}

		Result Machine::Load(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::UNKNOWN );
		}

		Result Machine::LoadCartridge(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::CARTRIDGE );
		}

		Result Machine::LoadDisk(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::DISK );
		}

		Result Machine::LoadSound(std::istream& stream) throw()
		{
			return Load( stream, Core::Image::SOUND );
		}

		Result Machine::Unload() throw()
		{
			if (emulator.state & IMAGE)
			{
				if (emulator.state & ON)
					emulator.PowerOff();

				if (emulator.image)
					emulator.Unload();

				emulator.state &= (NTSC|PAL);

				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		Result Machine::Power(const bool on) throw()
		{
			if (on)
			{
				if (emulator.state & IMAGE)
				{
					if (!(emulator.state & ON))
					{
						Result result = emulator.PowerOn();

						if (NES_SUCCEEDED(result))
							emulator.state |= ON;

						return result;
					}
				}
				else
				{
					return RESULT_ERR_NOT_READY;
				}
			}
			else
			{
				if (emulator.state & ON)
				{
					emulator.state &= ~uint(ON);
					emulator.PowerOff();
					return RESULT_OK;
				}
			}

			return RESULT_NOP;
		}

		Result Machine::Reset(const bool hard) throw()
		{
			if ((emulator.state & ON) && !emulator.tracker.IsLocked())
			{
				Result result = emulator.Reset( hard );

				if (NES_FAILED(result))
					Unload();

				return result;
			}

			return RESULT_ERR_NOT_READY;
		}

		Machine::Mode Machine::GetMode() const throw()
		{
			return (Mode) (emulator.state & (NTSC|PAL));
		}

		Machine::Mode Machine::GetDesiredMode() const throw()
		{
			return (!emulator.image || emulator.image->GetMode() == Core::MODE_NTSC) ? NTSC : PAL;
		}

		Result Machine::SetMode(Mode mode) throw()
		{
			if (mode != NTSC && mode != PAL)
				return RESULT_ERR_INVALID_PARAM;

			const Core::Mode m = (mode == NTSC ? Core::MODE_NTSC : Core::MODE_PAL);

			if (m != emulator.cpu.GetMode())
			{
				emulator.state = (emulator.state & ~uint(PAL|NTSC)) | mode;
				emulator.SetMode( m );
				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		Result Machine::LoadState(std::istream& stream) throw()
		{
			if (!emulator.tracker.MovieIsInserted() && !emulator.tracker.IsRewinding())
			{
				Api::Rewinder(emulator).Reset();
				return emulator.LoadState( &stream );
			}

			return RESULT_ERR_NOT_READY;
		}

		Result Machine::SaveState(std::ostream& stream,Compression compression) const throw()
		{
			return emulator.SaveState( &stream, compression != NO_COMPRESSION, false );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
