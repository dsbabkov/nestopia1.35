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

#ifndef NST_MAPPER_14_H
#define NST_MAPPER_14_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper14 : public Boards::Mmc3
		{
		public:

			Mapper14(Context& c)
			: Mmc3(c,BRD_GENERIC,CROM_MAX_512K|PROM_MAX_512K) {}

		private:

			void SubReset(bool);
			void SubLoad(State::Loader&);
			void SubSave(State::Saver&) const;

			void UpdatePrg();
			void UpdateChr() const;

			NES_DECL_POKE( Prg )

			u8 exMode;
			u8 exPrg[2];
			u8 exChr[8];
			u8 exNmt;
		};
	}
}

#endif
