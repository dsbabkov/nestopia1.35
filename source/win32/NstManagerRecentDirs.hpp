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

#ifndef NST_MANAGER_RECENTDIRS_H
#define NST_MANAGER_RECENTDIRS_H

#pragma once

namespace Nestopia
{
	namespace Managers
	{
		class RecentDirs
		{
		public:

			RecentDirs(Emulator&,const Configuration&,Window::Menu&);
			~RecentDirs();

			void Save(Configuration&) const;

		private:

			enum
			{
				MAX_DIRS = 5
			};

			void OnMenu(uint);
			void OnLock(uint);
			void OnClear(uint);
			void OnLoad(Emulator::Event);
			void Add(uint,const HeapString&) const;

			Emulator& emulator;
			const Window::Menu& menu;
		};
	}
}

#endif
