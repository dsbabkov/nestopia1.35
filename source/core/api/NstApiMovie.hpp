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

#ifndef NST_API_MOVIE_H
#define NST_API_MOVIE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <iosfwd>
#include "NstApi.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Movie : public Base
		{
			struct StateCaller;

		public:

			template<typename T>
			Movie(T& e)
			: Base(e) {}

			enum How
			{
				CLEAN,
				APPEND
			};

			enum CallbackMode
			{
				DISABLE_CALLBACK,
				ENABLE_CALLBACK
			};

			Result Play(std::istream&,CallbackMode=ENABLE_CALLBACK) throw();
			Result Record(std::ostream&,How=CLEAN,CallbackMode=ENABLE_CALLBACK) throw();

			void Stop() throw();
			void Eject() throw();
			void Cut() throw();

			bool IsPlaying() const throw();
			bool IsRecording() const throw();
			bool IsInserted() const throw();
			bool IsStopped() const throw();

			enum State
			{
				ERR_CORRUPT_FILE = -4,
				ERR_OUT_OF_MEMORY,
				ERR_UNSUPPORTED_IMAGE,
				ERR_GENERIC,
				STOPPED_PLAYING = 1,
				STOPPED_RECORDING,
				RECORDING,
				PLAYING
			};

			typedef void (NST_CALLBACK *StateCallback) (UserData,State);

			static StateCaller stateCallback;
		};

		struct Movie::StateCaller : Core::UserCallback<Movie::StateCallback>
		{
			void operator () (State state) const
			{
				if (function)
					function( userdata, state );
			}
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
