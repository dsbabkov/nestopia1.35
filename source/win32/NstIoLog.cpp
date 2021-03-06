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

#include "NstIoLog.hpp"

namespace Nestopia
{
	namespace Io
	{
		struct Log::Callbacker
		{
			void* data;
			void (NST_CALL *code)(void*,tstring,uint);
		};

		Log::Callbacker Log::callbacker = {NULL,NULL};

		void Log::SetCallback(void* data,void (NST_CALL* code)(void*,tstring,uint))
		{
			NST_ASSERT( bool(data) == bool(code) );

			callbacker.code = code;
			callbacker.data = data;
		}

		void Log::UnsetCallback()
		{
			callbacker.code = NULL;
			callbacker.data = NULL;
		}

		Log::~Log()
		{
			if (callbacker.code)
				callbacker.code( callbacker.data, Ptr(), Length() );
		}
	}
}
