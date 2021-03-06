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

#ifndef NST_SYSTEM_DLL_H
#define NST_SYSTEM_DLL_H

#pragma once

#include "NstMain.hpp"
#include <Windows.h>

namespace Nestopia
{
	namespace System
	{
		class Dll
		{
		public:

			ibool Load(tstring);

		private:

			HMODULE hModule;

		public:

			Dll()
			: hModule(NULL) {}

			explicit Dll(tstring file)
			: hModule(::LoadLibrary(file))
			{
				NST_ASSERT( file && *file );
			}

			~Dll()
			{
				if (hModule)
					::FreeLibrary( hModule );
			}

			void Unload()
			{
				if (HMODULE tmp = hModule)
				{
					hModule = NULL;
					::FreeLibrary( tmp );
				}
			}

			bool Loaded() const
			{
				return hModule != NULL;
			}

			HMODULE GetHandle() const
			{
				return hModule;
			}

			FARPROC operator () (cstring name) const
			{
				NST_ASSERT( hModule && name && *name );
				return ::GetProcAddress( hModule, name );
			}

			template<typename T>
			T Fetch(cstring name) const
			{
				NST_ASSERT( name && *name );
				return hModule ? (T) ::GetProcAddress( hModule, name ) : NULL;
			}
		};
	}
}

#endif
