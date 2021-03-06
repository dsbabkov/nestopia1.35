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

#ifndef NST_MANAGER_NETPLAY_H
#define NST_MANAGER_NETPLAY_H

#pragma once

namespace Nestopia
{
	namespace Managers
	{
		class Netplay
		{
		public:

			Netplay(Emulator&,const Configuration&,Window::Menu&,const Paths&,Window::Custom&);
			~Netplay();

			void Save(Configuration&,ibool) const;
			ibool Close() const;

		private:

			class Dll;
			class Kaillera;

			void OnCmdStart      (uint);
			void OnCmdDisconnect (uint);
			void OnCmdChat       (uint);
			void OnAppEvent      (Application::Instance::Event,const void*);

			Kaillera* kaillera;
			Emulator& emulator;
			Window::Menu& menu;
			Window::Custom& window;
			const Paths& paths;
			const ibool doFullscreen;
		};
	}
}

#endif
