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

#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstResourceString.hpp"
#include "NstIoFile.hpp"
#include "NstManagerPaths.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include "NstDialogInesHeader.hpp"

namespace Nestopia
{
	namespace Window
	{
		class InesHeader::CustomSize
		{
		public:

			CustomSize(uint&);

		private:

			struct Handlers;

			ibool OnInitDialog  (Param&);
			ibool OnCmdOk       (Param&);

			Dialog dialog;
			uint& size;

		public:

			void Open()
			{
				dialog.Open();
			}
		};

		struct InesHeader::Handlers
		{
			static const MsgHandler::Entry<InesHeader> messages[];
			static const MsgHandler::Entry<InesHeader> commands[];
		};

		const MsgHandler::Entry<InesHeader> InesHeader::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &InesHeader::OnInitDialog },
		};

		const MsgHandler::Entry<InesHeader> InesHeader::Handlers::commands[] =
		{
			{ IDC_INES_HEADER_TYPE_STD,    &InesHeader::OnCmdFileType  },
			{ IDC_INES_HEADER_TYPE_EXT,    &InesHeader::OnCmdFileType  },
			{ IDC_INES_HEADER_PRGROM_LIST, &InesHeader::OnCmdSizeOther },
			{ IDC_INES_HEADER_WRKRAM_LIST, &InesHeader::OnCmdSizeOther },
			{ IDC_INES_HEADER_CHRROM_LIST, &InesHeader::OnCmdSizeOther },
			{ IDC_INES_HEADER_SYSTEM_HOME, &InesHeader::OnCmdSystem    },
			{ IDC_INES_HEADER_SYSTEM_VS,   &InesHeader::OnCmdSystem    },
			{ IDC_INES_HEADER_SYSTEM_PC10, &InesHeader::OnCmdSystem    },
			{ IDC_INES_HEADER_ORIGINAL,    &InesHeader::OnCmdOriginal  },
			{ IDC_INES_HEADER_DETECT,      &InesHeader::OnCmdDetect    },
			{ IDC_INES_HEADER_SAVE,        &InesHeader::OnCmdSave      }
		};

		InesHeader::InesHeader(const Nes::Cartridge::Database& db,const Managers::Paths& p)
		: dialog(IDD_INES_HEADER,this,Handlers::messages,Handlers::commands), database(db), paths(p) {}

		uint InesHeader::Import(const Path& loadPath,Collection::Buffer& buffer)
		{
			NST_ASSERT( buffer.Empty() );

			try
			{
				Io::File file( loadPath, Io::File::COLLECT );

				if (file.Size() >= HEADER_SIZE && file.Peek<u32>() == HEADER_ID)
				{
					file.Stream() >> buffer;
					return 0;
				}
				else
				{
					return IDS_FILE_ERR_INVALID;
				}
			}
			catch (Io::File::Exception ids)
			{
				return ids;
			}
			catch (const std::bad_alloc&)
			{
				return IDS_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return IDS_ERR_GENERIC;
			}
		}

		uint InesHeader::Export(const Path& savePath,const Collection::Buffer& buffer)
		{
			NST_ASSERT( buffer.Size() >= HEADER_SIZE );

			try
			{
				Io::File file( savePath, Io::File::READ|Io::File::WRITE );

				if (file.Size() == 0 || file.Peek<u32>() == HEADER_ID)
				{
					file.Stream() << buffer;
					return 0;
				}
				else
				{
					return IDS_FILE_ERR_INVALID;
				}
			}
			catch (Io::File::Exception i)
			{
				return i;
			}
			catch (const std::bad_alloc&)
			{
				return IDS_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				return IDS_ERR_GENERIC;
			}
		}

		void InesHeader::Open(const Path& p)
		{
			if (p.Length())
			{
				{
					Collection::Buffer buffer;

					if (const uint result = Import( p, buffer ))
					{
						User::Fail( result );
						return;
					}

					std::memcpy( header, buffer.Ptr(), HEADER_SIZE );
					dbEntry = database.FindEntry( buffer.Ptr(), buffer.Size() );
				}

				path = &p;
				dialog.Open();
			}
		}

		ibool InesHeader::OnInitDialog(Param&)
		{
			static const tstring vsPPU[] =
			{
				_T( "RP2C03B"     ),
				_T( "RP2C03G"     ),
				_T( "PR2C04-0001" ),
				_T( "RP2C04-0002" ),
				_T( "RP2C04-0003" ),
				_T( "RP2C04-0004" ),
				_T( "RC2C03B"     ),
				_T( "RC2C03C"     ),
				_T( "RC2C05-01"   ),
				_T( "RC2C05-02"   ),
				_T( "RC2C05-03"   ),
				_T( "RC2C05-04"   ),
				_T( "RC2C05-05"   )
			};

			Control::ComboBox combo( dialog.ComboBox( IDC_INES_HEADER_VS_PPU_LIST ) );
			combo.Add( vsPPU, NST_COUNT(vsPPU) );
			combo[0].Select();

			static const tstring vsModes[] =
			{
				_T( "Standard"      ),
				_T( "RBI Baseball"  ),
				_T( "TKO Boxing"    ),
				_T( "Super Xevious" )
			};

			combo = dialog.ComboBox( IDC_INES_HEADER_VS_MODE_LIST );
			combo.Add( vsModes, NST_COUNT(vsModes) );
			combo[0].Select();

			dialog.Edit( IDC_INES_HEADER_MAPPER_BASE_VALUE ).Limit( 3 );
			dialog.Edit( IDC_INES_HEADER_MAPPER_SUB_VALUE ).Limit( 2 );

			dialog.Control( IDC_INES_HEADER_DETECT ).Enable( dbEntry != NULL );

			Nes::Api::Cartridge::Setup setup;
			Nes::Api::Cartridge::ReadNesHeader( setup, header, HEADER_SIZE );
			UpdateHeader( setup );

			return true;
		}

		ibool InesHeader::OkToSave(const uint fileSize) const
		{
			const uint saveSize =
			(
				HEADER_SIZE +
				(dialog.CheckBox( IDC_INES_HEADER_TRAINER ).Checked() ? 512 : 0) +
				(dialog.ComboBox( IDC_INES_HEADER_PRGROM_LIST ).Selection().Data()) +
				(dialog.ComboBox( IDC_INES_HEADER_CHRROM_LIST ).Selection().Data())
			);

			return saveSize <= fileSize || User::Confirm( IDS_INES_HEADER_UNSAFE );
		}

		ibool InesHeader::SaveHeader(Header& save) const
		{
			Nes::Api::Cartridge::Setup setup;

			setup.version = (dialog.RadioButton( IDC_INES_HEADER_TYPE_EXT ).Checked() ? 2 : 0);

			if (dialog.RadioButton( IDC_INES_HEADER_SYSTEM_VS ).Checked())
			{
				setup.system = Nes::SYSTEM_VS;
			}
			else if (dialog.RadioButton( IDC_INES_HEADER_SYSTEM_PC10 ).Checked())
			{
				setup.system = Nes::SYSTEM_PC10;
			}
			else
			{
				setup.system = Nes::SYSTEM_HOME;
			}

			if (dialog.RadioButton( IDC_INES_HEADER_REGION_BOTH ).Checked())
			{
				setup.region = Nes::REGION_BOTH;
			}
			else if (dialog.RadioButton( IDC_INES_HEADER_REGION_PAL ).Checked())
			{
				setup.region = Nes::REGION_PAL;
			}
			else
			{
				setup.region = Nes::REGION_NTSC;
			}

			setup.prgRom       = dialog.ComboBox( IDC_INES_HEADER_PRGROM_LIST ).Selection().Data();
			setup.wrkRam       = dialog.ComboBox( IDC_INES_HEADER_WRKRAM_LIST ).Selection().Data();
			setup.wrkRamBacked = dialog.ComboBox( IDC_INES_HEADER_WRKRAM_BACKED_LIST ).Selection().Data();
			setup.chrRom       = dialog.ComboBox( IDC_INES_HEADER_CHRROM_LIST ).Selection().Data();
			setup.chrRam       = dialog.ComboBox( IDC_INES_HEADER_CHRRAM_LIST ).Selection().Data();
			setup.chrRamBacked = dialog.ComboBox( IDC_INES_HEADER_CHRRAM_BACKED_LIST ).Selection().Data();

			if (setup.system == Nes::SYSTEM_VS)
			{
				setup.ppu = (Nes::PpuType) (dialog.ComboBox( IDC_INES_HEADER_VS_PPU_LIST ).Selection().GetIndex() + 1);
				setup.security = dialog.ComboBox( IDC_INES_HEADER_VS_MODE_LIST ).Selection().GetIndex();
			}
			else
			{
				setup.ppu = Nes::PPU_RP2C02;
				setup.security = 0;
			}

			if (dialog.RadioButton( IDC_INES_HEADER_FOURSCREEN ).Checked())
			{
				setup.mirroring = Nes::Api::Cartridge::MIRROR_FOURSCREEN;
			}
			else if (dialog.RadioButton( IDC_INES_HEADER_HORIZONTAL ).Checked())
			{
				setup.mirroring = Nes::Api::Cartridge::MIRROR_HORIZONTAL;
			}
			else
			{
				setup.mirroring = Nes::Api::Cartridge::MIRROR_VERTICAL;
			}

			if (!(dialog.Edit( IDC_INES_HEADER_MAPPER_BASE_VALUE ) >> setup.mapper))
				return false;

			setup.subMapper = 0;

			if (setup.version && !(dialog.Edit( IDC_INES_HEADER_MAPPER_SUB_VALUE ) >> setup.subMapper))
				return false;

			setup.trainer = bool(dialog.CheckBox( IDC_INES_HEADER_TRAINER ).Checked());

			if (NES_FAILED(Nes::Api::Cartridge::WriteNesHeader( setup, save, HEADER_SIZE )))
				return false;

			u8 reserved[HEADER_SIZE];
			std::memset( reserved, 0, sizeof(reserved) );

			if (setup.version)
			{
				reserved[8] = header[8] & 0x0E;
				reserved[12] = header[12] & 0xFC;

				std::memcpy( reserved+14, header+14, HEADER_SIZE-14 );
			}
			else
			{
				reserved[7] = header[7] & 0x02;
				reserved[9] = header[9] & 0xFE;

				std::memcpy( reserved+10, header+10, HEADER_SIZE-10 );
			}

			for (uint i=4; i < HEADER_SIZE-4; ++i)
			{
				if (reserved[i])
				{
					if (!User::Confirm( IDS_INES_HEADER_REMOVE_RESERVED ))
					{
						for (uint j=4; j < HEADER_SIZE-4; ++j)
							save[j] |= reserved[j];
					}
					break;
				}
			}

			return true;
		}

		void InesHeader::UpdateHeader(const Nes::Api::Cartridge::Setup& setup) const
		{
			dialog.RadioButton( IDC_INES_HEADER_TYPE_STD ).Check( !setup.version );
			dialog.RadioButton( IDC_INES_HEADER_TYPE_EXT ).Check(  setup.version );

			dialog.Edit( IDC_INES_HEADER_MAPPER_BASE_VALUE ).Text() << setup.mapper;
			dialog.Edit( IDC_INES_HEADER_MAPPER_SUB_VALUE ).Text() << setup.subMapper;

			dialog.RadioButton( IDC_INES_HEADER_FOURSCREEN ).Check( setup.mirroring == Nes::Api::Cartridge::MIRROR_FOURSCREEN );
			dialog.RadioButton( IDC_INES_HEADER_VERTICAL   ).Check( setup.mirroring == Nes::Api::Cartridge::MIRROR_VERTICAL   );
			dialog.RadioButton( IDC_INES_HEADER_HORIZONTAL ).Check( setup.mirroring != Nes::Api::Cartridge::MIRROR_VERTICAL && setup.mirroring != Nes::Api::Cartridge::MIRROR_FOURSCREEN );

			dialog.RadioButton( IDC_INES_HEADER_SYSTEM_VS   ).Check( setup.system == Nes::SYSTEM_VS   );
			dialog.RadioButton( IDC_INES_HEADER_SYSTEM_PC10 ).Check( setup.system == Nes::SYSTEM_PC10 );
			dialog.RadioButton( IDC_INES_HEADER_SYSTEM_HOME ).Check( setup.system == Nes::SYSTEM_HOME );

			dialog.RadioButton( IDC_INES_HEADER_REGION_BOTH ).Check( setup.region == Nes::REGION_BOTH  );
			dialog.RadioButton( IDC_INES_HEADER_REGION_PAL  ).Check( setup.region == Nes::REGION_PAL   );
			dialog.RadioButton( IDC_INES_HEADER_REGION_NTSC ).Check( setup.region == Nes::REGION_NTSC  );

			dialog.CheckBox( IDC_INES_HEADER_TRAINER ).Check( setup.trainer );

			dialog.ComboBox( IDC_INES_HEADER_VS_PPU_LIST  )[setup.ppu ? setup.ppu-1 : 0].Select();
			dialog.ComboBox( IDC_INES_HEADER_VS_MODE_LIST )[setup.security].Select();

			UpdateVersion();

			UpdateSizes( IDC_INES_HEADER_PRGROM_LIST, SIZETYPE_STD_16K, setup.prgRom );
			UpdateSizes( IDC_INES_HEADER_WRKRAM_LIST, setup.version ? SIZETYPE_EXT : SIZETYPE_STD_8K, setup.wrkRam );
			UpdateSizes( IDC_INES_HEADER_WRKRAM_BACKED_LIST, setup.version ? SIZETYPE_EXT : SIZETYPE_STATE, setup.wrkRamBacked );
			UpdateSizes( IDC_INES_HEADER_CHRROM_LIST, SIZETYPE_STD_8K, setup.chrRom );
			UpdateSizes( IDC_INES_HEADER_CHRRAM_LIST, SIZETYPE_EXT, setup.chrRam );
			UpdateSizes( IDC_INES_HEADER_CHRRAM_BACKED_LIST, SIZETYPE_EXT, setup.chrRamBacked );
		}

		void InesHeader::DetectHeader(Nes::Api::Cartridge::Setup& setup) const
		{
			NST_ASSERT( dbEntry );

			setup.system = database.GetSystem(dbEntry);
			setup.region = database.GetRegion(dbEntry);
			setup.prgRom = database.GetPrgRom(dbEntry);
			setup.wrkRam = database.GetWrkRam(dbEntry);
			setup.wrkRamBacked = database.GetWrkRamBacked(dbEntry);
			setup.chrRom = database.GetChrRom(dbEntry);
			setup.chrRam = database.GetChrRam(dbEntry);
			setup.chrRamBacked = database.GetChrRamBacked(dbEntry);
			setup.mirroring = database.GetMirroring(dbEntry);
			setup.mapper = database.GetMapper(dbEntry);
			setup.trainer = bool(database.HasTrainer(dbEntry));
		}

		void InesHeader::UpdateVersion() const
		{
			const ibool v2 = dialog.RadioButton( IDC_INES_HEADER_TYPE_EXT ).Checked();

			if (!v2 && dialog.RadioButton( IDC_INES_HEADER_SYSTEM_PC10 ).Checked())
			{
				dialog.RadioButton( IDC_INES_HEADER_SYSTEM_PC10 ).Uncheck();
				dialog.RadioButton( IDC_INES_HEADER_SYSTEM_HOME ).Check();
			}

			dialog.Control( IDC_INES_HEADER_SYSTEM_PC10 ).Enable( v2 );

			if (!v2 && dialog.RadioButton( IDC_INES_HEADER_REGION_BOTH ).Checked())
			{
				dialog.RadioButton( IDC_INES_HEADER_REGION_BOTH ).Uncheck();
				dialog.RadioButton( IDC_INES_HEADER_REGION_NTSC ).Check();
			}

			dialog.Control( IDC_INES_HEADER_REGION_BOTH ).Enable( v2 );

			dialog.Control( IDC_INES_HEADER_MAPPER_SUB_TEXT ).Enable( v2 );
			dialog.Control( IDC_INES_HEADER_MAPPER_SUB_VALUE ).Enable( v2 );

			dialog.Control( IDC_INES_HEADER_CHRRAM_LIST ).Enable( v2 );
			dialog.Control( IDC_INES_HEADER_CHRRAM_TEXT ).Enable( v2 );
			dialog.Control( IDC_INES_HEADER_CHRRAM_BACKED_LIST ).Enable( v2 );
			dialog.Control( IDC_INES_HEADER_CHRRAM_BACKED_TEXT ).Enable( v2 );

			UpdateSystem();
		}

		void InesHeader::UpdateSystem() const
		{
			const ibool vsExt =
			(
				dialog.RadioButton( IDC_INES_HEADER_SYSTEM_VS ).Checked() &&
				dialog.RadioButton( IDC_INES_HEADER_TYPE_EXT ).Checked()
			);

			dialog.Control( IDC_INES_HEADER_VS_PPU_LIST  ).Enable( vsExt );
			dialog.Control( IDC_INES_HEADER_VS_PPU_TEXT  ).Enable( vsExt );
			dialog.Control( IDC_INES_HEADER_VS_MODE_LIST ).Enable( vsExt );
			dialog.Control( IDC_INES_HEADER_VS_MODE_TEXT ).Enable( vsExt );
		}

		uint InesHeader::GetMaxSize(uint block) const
		{
			return (dialog.RadioButton(IDC_INES_HEADER_TYPE_EXT).Checked() ? 0xFFF : 0xFF) * block;
		}

		void InesHeader::UpdateSizes(const uint idc,const SizeType sizeType,uint sizeSelect) const
		{
			const Control::ComboBox combo( dialog.ComboBox(idc) );
			combo.Clear();

			if (sizeType == SIZETYPE_STD_8K || sizeType == SIZETYPE_STD_16K)
			{
				combo.Reserve( 1+8+1, 8 );
				combo.Add( Resource::String(IDS_NONE).Ptr() ).Data() = 0;

				uint selection = 0;
				uint match = 0;

				for (uint i=0; i < 8; ++i)
				{
					const uint size = (uint(sizeType) << i) * Nes::Core::SIZE_1K;

					if (sizeSelect == size)
					{
						selection = i+1;
						match = size;
					}

					combo.Add( (String::Stack<7>() << (size / Nes::Core::SIZE_1K)  << 'k').Ptr() ).Data() = size;
				}

				combo.Add( Resource::String(IDS_TEXT_OTHER) ).Data() = OTHER_SIZE | sizeType;

				if (sizeSelect != match)
				{
					const uint block = uint(sizeType) * Nes::Core::SIZE_1K;
					sizeSelect = (sizeSelect / block + (sizeSelect % block > 0)) * block;

					if (sizeSelect > GetMaxSize( block ))
					{
						selection = 1+7;
					}
					else
					{
						combo.Add( (String::Stack<7>() << (sizeSelect / Nes::Core::SIZE_1K) << 'k').Ptr() ).Data() = sizeSelect;
						selection = 1+8+1;
					}
				}

				combo[selection].Select();
			}
			else if (sizeType == SIZETYPE_EXT)
			{
				static const struct { tstring name; uint size; } sizes[14] =
				{
					{ _T( "128"   ), 128                   },
					{ _T( "256"   ), 256                   },
					{ _T( "512"   ), 512                   },
					{ _T( "1k"    ), Nes::Core::SIZE_1K    },
					{ _T( "2k"    ), Nes::Core::SIZE_2K    },
					{ _T( "4k"    ), Nes::Core::SIZE_4K    },
					{ _T( "8k"    ), Nes::Core::SIZE_8K    },
					{ _T( "16k"   ), Nes::Core::SIZE_16K   },
					{ _T( "32k"   ), Nes::Core::SIZE_32K   },
					{ _T( "64k"   ), Nes::Core::SIZE_64K   },
					{ _T( "128k"  ), Nes::Core::SIZE_128K  },
					{ _T( "256k"  ), Nes::Core::SIZE_256K  },
					{ _T( "512k"  ), Nes::Core::SIZE_512K  },
					{ _T( "1024k" ), Nes::Core::SIZE_1024K }
				};

				combo.Reserve( 1+14, 16 );
				combo.Add( Resource::String(IDS_NONE).Ptr() ).Data() = 0;

				uint selection = sizeSelect ? UINT_MAX : 0;

				for (uint i=0; i < 14; ++i)
				{
					if (selection == UINT_MAX && sizeSelect <= sizes[i].size)
						selection = 1+i;

					combo.Add( sizes[i].name ).Data() = sizes[i].size;
				}

				combo[selection == UINT_MAX ? 1+14-1 : selection].Select();
			}
			else
			{
				NST_ASSERT( sizeType == SIZETYPE_STATE );

				combo.Reserve( 2, 16 );
				combo.Add( Resource::String(IDS_TEXT_DISABLED) ).Data() = false;
				combo.Add( Resource::String(IDS_TEXT_ENABLED)  ).Data() = true;

				combo[sizeSelect >= Nes::Core::SIZE_1K].Select();
			}
		}

		ibool InesHeader::OnCmdFileType(Param& param)
		{
			if (param.Button().Clicked())
			{
				UpdateVersion();

				uint size = dialog.ComboBox(IDC_INES_HEADER_PRGROM_LIST).Selection().Data();
				UpdateSizes( IDC_INES_HEADER_PRGROM_LIST, SIZETYPE_STD_16K, size );

				size = dialog.ComboBox(IDC_INES_HEADER_WRKRAM_LIST).Selection().Data();
				uint save = dialog.ComboBox(IDC_INES_HEADER_WRKRAM_BACKED_LIST).Selection().Data();

				const ibool v2 = dialog.RadioButton( IDC_INES_HEADER_TYPE_EXT ).Checked();

				if (dialog.ComboBox(IDC_INES_HEADER_WRKRAM_BACKED_LIST).Size() == 2)
				{
					if (v2 && save)
					{
						save = NST_MAX(size,Nes::Core::SIZE_8K);
						size = 0;
					}
				}
				else if (!v2)
				{
					size += save;

					if (size < Nes::Core::SIZE_8K)
						size = 0;
				}

				UpdateSizes( IDC_INES_HEADER_WRKRAM_LIST, v2 ? SIZETYPE_EXT : SIZETYPE_STD_8K, size );
				UpdateSizes( IDC_INES_HEADER_WRKRAM_BACKED_LIST, v2 ? SIZETYPE_EXT : SIZETYPE_STATE, save );

				size = dialog.ComboBox(IDC_INES_HEADER_CHRROM_LIST).Selection().Data();
				UpdateSizes( IDC_INES_HEADER_CHRROM_LIST, SIZETYPE_STD_8K, size );
			}

			return true;
		}

		ibool InesHeader::OnCmdSystem(Param& param)
		{
			if (param.Button().Clicked())
				UpdateSystem();

			return true;
		}

		ibool InesHeader::OnCmdSizeOther(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				const Control::ComboBox combo( dialog.ComboBox(param.ComboBox().GetId()) );
				const uint otherSize = combo.Selection().Data();

				if (otherSize & OTHER_SIZE)
				{
					const uint last = combo.Back().Data();
					uint size = last;

					if (last & OTHER_SIZE)
						size &= OTHER_SIZE_DIV;
					else
						size /= Nes::Core::SIZE_1K;

					CustomSize( size ).Open();

					if (size % (otherSize & OTHER_SIZE_DIV) || size > GetMaxSize(otherSize & OTHER_SIZE_DIV))
					{
						size = (last & OTHER_SIZE) ? (otherSize & OTHER_SIZE_DIV) : (last / Nes::Core::SIZE_1K);
						User::Warn( IDS_ERR_INVALID_SIZE, IDS_TITLE_ERROR );
					}

					if (!(last & OTHER_SIZE))
						combo.Back().Erase();

					combo.Add( (String::Stack<7>() << size << 'k').Ptr() ).Data() = size * Nes::Core::SIZE_1K;
					combo.Back().Select();
				}
			}

			return true;
		}

		ibool InesHeader::OnCmdOriginal(Param& param)
		{
			if (param.Button().Clicked())
			{
				Nes::Api::Cartridge::Setup setup;
				Nes::Api::Cartridge::ReadNesHeader( setup, header, HEADER_SIZE );
				UpdateHeader( setup );
			}

			return true;
		}

		ibool InesHeader::OnCmdDetect(Param& param)
		{
			if (param.Button().Clicked())
			{
				Nes::Api::Cartridge::Setup setup;
				DetectHeader( setup );
				UpdateHeader( setup );
			}

			return true;
		}

		ibool InesHeader::OnCmdSave(Param& param)
		{
			if (param.Button().Clicked())
			{
				Header header;

				if (!SaveHeader( header ))
				{
					User::Warn( IDS_INES_HEADER_INVALID, IDS_TITLE_ERROR );
					return true;
				}

				const Path savePath( paths.BrowseSave( Managers::Paths::File::INES, Managers::Paths::DONT_SUGGEST, *path ) );

				if (savePath.Empty())
					return true;

				Collection::Buffer buffer;
				uint result = Import( *path, buffer );

				if (result == 0)
				{
					NST_ASSERT( buffer.Size() >= HEADER_SIZE );

					if (!OkToSave( buffer.Size() ))
						return true;

					std::memcpy( buffer.Ptr(), header, HEADER_SIZE );
					result = Export( savePath, buffer );

					if (result == 0)
					{
						dialog.Close();
						return true;
					}
				}

				User::Fail( result );
			}

			return true;
		}

		struct InesHeader::CustomSize::Handlers
		{
			static const MsgHandler::Entry<CustomSize> messages[];
			static const MsgHandler::Entry<CustomSize> commands[];
		};

		const MsgHandler::Entry<InesHeader::CustomSize> InesHeader::CustomSize::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &CustomSize::OnInitDialog }
		};

		const MsgHandler::Entry<InesHeader::CustomSize> InesHeader::CustomSize::Handlers::commands[] =
		{
			{ IDOK, &CustomSize::OnCmdOk }
		};

		InesHeader::CustomSize::CustomSize(uint& s)
		:
		size   (s),
		dialog (IDD_INES_HEADER_CUSTOM,this,Handlers::messages,Handlers::commands)
		{
		}

		ibool InesHeader::CustomSize::OnInitDialog(Param&)
		{
			const Control::Edit edit( dialog.Edit(IDC_INES_HEADER_CUSTOM_VALUE) );

			edit.Limit( 5 );
			edit.Text() << size;

			return true;
		}

		ibool InesHeader::CustomSize::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				dialog.Edit( IDC_INES_HEADER_CUSTOM_VALUE ) >> size;
				dialog.Close();
			}

			return true;
		}
	}
}
