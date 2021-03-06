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

#ifndef NST_CARTRIDGE_UNIF_H
#define NST_CARTRIDGE_UNIF_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Cartridge::Unif
		{
		public:

			Unif
			(
				StdStream,
				Ram&,
				Ram&,
				Ram&,
				Api::Cartridge::Info&,
				const ImageDatabase*,
				ImageDatabase::Handle&
			);

		private:

			void  Import();
			dword ComputeCrc() const;
			void  CopyRom();
			bool  NewChunk(bool&);

			ulong ReadName       ();
			ulong ReadComment    ();
			ulong ReadString     (cstring,std::string&);
			ulong ReadDumper     ();
			ulong ReadSystem     ();
			ulong ReadRomCrc     (uint,uint);
			ulong ReadRomData    (uint,uint,ulong);
			ulong ReadBattery    ();
			ulong ReadMapper     ();
			ulong ReadMirroring  ();
			ulong ReadController ();
			ulong ReadChrRam     ();

			bool CheckMapper();
			void CheckImageDatabase();

			enum
			{
				HEADER_RESERVED_LENGTH = 24,
				NO_MAPPER = SHRT_MAX
			};

			struct Dump
			{
				enum
				{
					NAME_LENGTH = 100,
					AGENT_LENGTH = 100,
					LENGTH = NAME_LENGTH + 4 + AGENT_LENGTH
				};

				char name[NAME_LENGTH];
				uint day;
				uint month;
				uint year;
				char agent[AGENT_LENGTH];
			};

			Stream::In stream;
			Log log;

			Ram& prg;
			Ram& chr;
			Ram& wrk;

			Api::Cartridge::Info& info;

			const ImageDatabase* const database;
			ImageDatabase::Handle& databaseHandle;

			dword crc;

			Result result;

			struct Rom
			{
				Rom();

				dword crc;
				Ram rom;

				static const char id[16];
			};

			Rom roms[2][16];

			struct Board
			{
				bool operator < (const Board&) const;

				cstring name;
				u16 mapper;
				u16 wrkRam;
			};

			static bool sorted;
			static Board boards[];

		public:

			Result GetResult() const
			{
				return result;
			}
		};
	}
}

#endif
