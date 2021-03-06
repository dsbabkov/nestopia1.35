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

#ifndef NST_CORE_H
#define NST_CORE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstTypes.hpp"

#if CHAR_BIT != 8
#error unsupported platform!
#endif

#ifdef __INTEL_COMPILER
#pragma warning( disable : 304 373 383 444 810 869 981 1682 1683 )
#elif defined(_MSC_VER)
#pragma warning( disable : 4100 4244 4511 4512 4800 4996 )
#endif

#ifdef _MSC_VER

 #define NES_IO_CALL __fastcall

 #if defined(NST_PROFILER) && defined(NST_X86)
 inline void __stdcall GetCpuTicks(unsigned __int64* ticks)
 {
     __asm
     {
         cpuid
         rdtsc
         mov esi, ticks
         mov DWORD PTR [esi+0], eax
         mov DWORD PTR [esi+4], edx
     }
 }
 #endif
// #elif defined(__GNUC__) && defined(__i386__)
//  #define NES_IO_CALL __attribute__((regparm(3)))
#else
 #define NES_IO_CALL
#endif

#define NST_PI  3.1415926535897932384626433832795
#define NST_DEG 0.0174532925199432957692369076848

namespace Nes
{
	namespace Core
	{
		enum Mode
		{
			MODE_NTSC,
			MODE_PAL
		};

		enum
		{
			SIZE_1K    = 0x400U,
			SIZE_2K    = 0x800U,
			SIZE_4K    = 0x1000U,
			SIZE_8K    = 0x2000U,
			SIZE_16K   = 0x4000U,
			SIZE_32K   = 0x8000U,
			SIZE_40K   = 0xA000U,
			SIZE_64K   = 0x10000UL,
			SIZE_128K  = 0x20000UL,
			SIZE_256K  = 0x40000UL,
			SIZE_512K  = 0x80000UL,
			SIZE_1024K = 0x100000UL,
			SIZE_2048K = 0x200000UL,
			SIZE_4096K = 0x200000UL
		};

		typedef void* StdStream;
	}

	enum PpuType
	{
		PPU_RP2C02,
		PPU_RP2C03B,
		PPU_RP2C03G,
		PPU_RP2C04_0001,
		PPU_RP2C04_0002,
		PPU_RP2C04_0003,
		PPU_RP2C04_0004,
		PPU_RC2C03B,
		PPU_RC2C03C,
		PPU_RC2C05_01,
		PPU_RC2C05_02,
		PPU_RC2C05_03,
		PPU_RC2C05_04,
		PPU_RC2C05_05
	};

	enum System
	{
		SYSTEM_HOME,
		SYSTEM_VS,
		SYSTEM_PC10
	};

	enum Region
	{
		REGION_NTSC = 1,
		REGION_PAL,
		REGION_BOTH
	};

	enum
	{
		FPS_NTSC = 60,
		FPS_PAL  = 50
	};

	enum Result
	{
		RESULT_ERR_INVALID_MAPPER           = -15,
		RESULT_ERR_MISSING_BIOS             = -14,
		RESULT_ERR_UNSUPPORTED_SOUND_CHIP   = -13,
		RESULT_ERR_UNSUPPORTED_GAME         = -12,
		RESULT_ERR_UNSUPPORTED_MAPPER       = -11,
		RESULT_ERR_UNSUPPORTED_VSSYSTEM     = -10,
		RESULT_ERR_UNSUPPORTED_FILE_VERSION =  -9,
		RESULT_ERR_UNSUPPORTED              =  -8,
		RESULT_ERR_INVALID_CRC              =  -7,
		RESULT_ERR_CORRUPT_FILE             =  -6,
		RESULT_ERR_INVALID_FILE             =  -5,
		RESULT_ERR_INVALID_PARAM            =  -4,
		RESULT_ERR_NOT_READY                =  -3,
		RESULT_ERR_OUT_OF_MEMORY            =  -2,
		RESULT_ERR_GENERIC                  =  -1,
		RESULT_OK                           =   0,
		RESULT_NOP                          =  +1,
		RESULT_WARN_UNSUPPORTED             =  +2,
		RESULT_WARN_BAD_DUMP                =  +3,
		RESULT_WARN_BAD_PROM                =  +4,
		RESULT_WARN_BAD_CROM                =  +5,
		RESULT_WARN_BAD_FILE_HEADER         =  +6,
		RESULT_WARN_BATTERY_NOT_SAVED       =  +7,
		RESULT_WARN_BATTERY_NOT_LOADED      =  +8,
		RESULT_WARN_ENCRYPTED_ROM           =  +9,
		RESULT_WARN_INCORRECT_FILE_HEADER   = +10
	};
}

#define NES_FAILED(x_) ((x_) < Nes::RESULT_OK)
#define NES_SUCCEEDED(x_) ((x_) >= Nes::RESULT_OK)

#endif
