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

#ifndef NST_INPUT_HORITRACK_H
#define NST_INPUT_HORITRACK_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class HoriTrack : public Device
			{
			public:

				HoriTrack(const Cpu&);

			private:

				enum
				{
					REVERSED  = 0x010000UL,
					LOWSPEED  = 0x020000UL,
					CONNECTED = 0x080000UL
				};

				void Reset();
				void Poke(uint);
				uint Peek(uint);
				void LoadState(State::Loader&,dword);
				void SaveState(State::Saver&,uchar) const;

				uint strobe;
				dword stream;
				dword state;
				uint x;
				uint y;
			};
		}
	}
}

#endif
