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

#include <cstring>
#include <new>
#include <algorithm>
#include "../NstMachine.hpp"
#include "../NstCartridge.hpp"
#include "../input/NstInpDevice.hpp"
#include "../input/NstInpAdapter.hpp"
#include "../input/NstInpPad.hpp"
#include "../input/NstInpLightGun.hpp"
#include "../input/NstInpPaddle.hpp"
#include "../input/NstInpPowerPad.hpp"
#include "../input/NstInpMouse.hpp"
#include "../input/NstInpFamilyTrainer.hpp"
#include "../input/NstInpFamilyKeyboard.hpp"
#include "../input/NstInpSuborKeyboard.hpp"
#include "../input/NstInpDoremikkoKeyboard.hpp"
#include "../input/NstInpHoriTrack.hpp"
#include "../input/NstInpPachinko.hpp"
#include "../input/NstInpOekaKidsTablet.hpp"
#include "../input/NstInpHyperShot.hpp"
#include "../input/NstInpCrazyClimber.hpp"
#include "../input/NstInpMahjong.hpp"
#include "../input/NstInpExcitingBoxing.hpp"
#include "../input/NstInpTopRider.hpp"
#include "../input/NstInpPokkunMoguraa.hpp"
#include "../input/NstInpPartyTap.hpp"
#include "../input/NstInpRob.hpp"
#include "../NstMapper.hpp"
#include "../NstClock.hpp"
#include "../board/NstBrdBandai.hpp"
#include "NstApiMachine.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			Controllers::PollCaller2< Controllers::Pad               > Controllers::Pad::callback;
			Controllers::PollCaller1< Controllers::Zapper            > Controllers::Zapper::callback;
			Controllers::PollCaller1< Controllers::Paddle            > Controllers::Paddle::callback;
			Controllers::PollCaller1< Controllers::PowerPad          > Controllers::PowerPad::callback;
			Controllers::PollCaller1< Controllers::Mouse             > Controllers::Mouse::callback;
			Controllers::PollCaller1< Controllers::FamilyTrainer     > Controllers::FamilyTrainer::callback;
			Controllers::PollCaller3< Controllers::FamilyKeyboard    > Controllers::FamilyKeyboard::callback;
			Controllers::PollCaller3< Controllers::SuborKeyboard     > Controllers::SuborKeyboard::callback;
			Controllers::PollCaller3< Controllers::DoremikkoKeyboard > Controllers::DoremikkoKeyboard::callback;
			Controllers::PollCaller1< Controllers::HoriTrack         > Controllers::HoriTrack::callback;
			Controllers::PollCaller1< Controllers::Pachinko          > Controllers::Pachinko::callback;
			Controllers::PollCaller1< Controllers::VsSystem          > Controllers::VsSystem::callback;
			Controllers::PollCaller1< Controllers::OekaKidsTablet    > Controllers::OekaKidsTablet::callback;
			Controllers::PollCaller1< Controllers::HyperShot         > Controllers::HyperShot::callback;
			Controllers::PollCaller1< Controllers::CrazyClimber      > Controllers::CrazyClimber::callback;
			Controllers::PollCaller2< Controllers::Mahjong           > Controllers::Mahjong::callback;
			Controllers::PollCaller2< Controllers::ExcitingBoxing    > Controllers::ExcitingBoxing::callback;
			Controllers::PollCaller1< Controllers::TopRider          > Controllers::TopRider::callback;
			Controllers::PollCaller2< Controllers::PokkunMoguraa     > Controllers::PokkunMoguraa::callback;
			Controllers::PollCaller1< Controllers::PartyTap          > Controllers::PartyTap::callback;
			Controllers::PollCaller1< Controllers::KaraokeStudio     > Controllers::KaraokeStudio::callback;

			Controllers::PowerPad::PowerPad() throw()
			{
				std::fill( sideA, sideA + NUM_SIDE_A_BUTTONS, false );
				std::fill( sideB, sideB + NUM_SIDE_B_BUTTONS, false );
			}

			Controllers::FamilyTrainer::FamilyTrainer() throw()
			{
				std::fill( sideA, sideA + NUM_SIDE_A_BUTTONS, false );
				std::fill( sideB, sideB + NUM_SIDE_B_BUTTONS, false );
			}

			Controllers::FamilyKeyboard::FamilyKeyboard() throw()
			{
				std::memset( parts, 0x00, sizeof(parts) );
			}

			Controllers::SuborKeyboard::SuborKeyboard() throw()
			{
				std::memset( parts, 0x00, sizeof(parts) );
			}

			Controllers::Controllers() throw()
			{
			}
		}
	}

	namespace Api
	{
		Result Input::ConnectController(const uint port,const Type type) throw()
		{
			Core::Input::Device* old = NULL;

			try
			{
				switch (port)
				{
					case PORT_1:
					case PORT_2:
					{
						if (emulator.extPort->GetDevice( port )->GetType() == type)
							return RESULT_NOP;

						Core::Input::Device* device;

						switch (type)
						{
							case UNCONNECTED:

								device = new Core::Input::Device( emulator.cpu );
								break;

							case PAD1:
							case PAD2:
							case PAD3:
							case PAD4:

								device = new Core::Input::Pad(  emulator.cpu, uint(type) - PAD1 );
								break;

							case ZAPPER:

								device = new Core::Input::LightGun( emulator.cpu, emulator.ppu );
								break;

							case PADDLE:

								device = new Core::Input::Paddle( emulator.cpu, false );
								break;

							case POWERPAD:

								device = new Core::Input::PowerPad( emulator.cpu );
								break;

							case MOUSE:

								device = new Core::Input::Mouse( emulator.cpu );
								break;

							case ROB:

								device = new Core::Input::Rob( emulator.cpu, emulator.ppu );
								break;

							default: return RESULT_ERR_INVALID_PARAM;
						}

						old = emulator.extPort->Connect( port, device );
						break;
					}

					case PORT_3:
					case PORT_4:

						if (emulator.extPort->NumPorts() > 2)
						{
							if (emulator.extPort->GetDevice( port )->GetType() == type)
								return RESULT_NOP;

							switch (type)
							{
								case UNCONNECTED:

									if (emulator.extPort->GetDevice( port == PORT_3 ? PORT_4 : PORT_3 )->GetType() == UNCONNECTED)
									{
										Core::Input::Adapter* const adapter = new Core::Input::AdapterTwo
										(
											emulator.extPort->GetDevice(0),
											emulator.extPort->GetDevice(1),
											emulator.extPort->GetType()
										);

										for (uint i=2; i < 4; ++i)
											delete emulator.extPort->GetDevice(i);

										delete emulator.extPort;
										emulator.extPort = adapter;
									}
									else
									{
										old = emulator.extPort->Connect( port, new Core::Input::Device( emulator.cpu ) );
									}
									break;

								case PAD1:
								case PAD2:
								case PAD3:
								case PAD4:

									old = emulator.extPort->Connect( port, new Core::Input::Pad( emulator.cpu, uint(type) - PAD1 ) );
									break;

								default: return RESULT_ERR_INVALID_PARAM;
							}
						}
						else
						{
							switch (type)
							{
								case UNCONNECTED:

									return RESULT_NOP;

								case PAD1:
								case PAD2:
								case PAD3:
								case PAD4:
								{
									Core::Input::Device* const devices[2] =
									{
										new (std::nothrow) Core::Input::Device( emulator.cpu ),
										new (std::nothrow) Core::Input::Pad( emulator.cpu, uint(type) - PAD1 )
									};

									Core::Input::Adapter* adapter;

									if
									(
										devices[0] && devices[1] && NULL !=
										(
											adapter = new (std::nothrow) Core::Input::AdapterFour
											(
												emulator.extPort->GetDevice(0),
												emulator.extPort->GetDevice(1),
												devices[port == PORT_3],
												devices[port != PORT_3],
												emulator.extPort->GetType()
											)
										)
									)
									{
										delete emulator.extPort;
										emulator.extPort = adapter;
									}
									else
									{
										delete devices[0];
										delete devices[1];

										return RESULT_ERR_OUT_OF_MEMORY;
									}
									break;
								}

								default: return RESULT_ERR_INVALID_PARAM;
							}
						}
						break;

					case EXPANSION_PORT:

						if (emulator.expPort->GetType() == type)
							return RESULT_NOP;

						old = emulator.expPort;

						switch (type)
						{
							case UNCONNECTED:       emulator.expPort = new Core::Input::Device( emulator.cpu );            break;
							case PADDLE:            emulator.expPort = new Core::Input::Paddle( emulator.cpu, true );      break;
							case FAMILYTRAINER:     emulator.expPort = new Core::Input::FamilyTrainer( emulator.cpu );     break;
							case FAMILYKEYBOARD:    emulator.expPort = new Core::Input::FamilyKeyboard( emulator.cpu );    break;
							case SUBORKEYBOARD:     emulator.expPort = new Core::Input::SuborKeyboard( emulator.cpu );     break;
							case DOREMIKKOKEYBOARD: emulator.expPort = new Core::Input::DoremikkoKeyboard( emulator.cpu ); break;
							case HORITRACK:         emulator.expPort = new Core::Input::HoriTrack( emulator.cpu );         break;
							case PACHINKO:          emulator.expPort = new Core::Input::Pachinko( emulator.cpu );          break;
							case OEKAKIDSTABLET:    emulator.expPort = new Core::Input::OekaKidsTablet( emulator.cpu );    break;
							case HYPERSHOT:         emulator.expPort = new Core::Input::HyperShot( emulator.cpu );         break;
							case CRAZYCLIMBER:      emulator.expPort = new Core::Input::CrazyClimber( emulator.cpu );      break;
							case MAHJONG:           emulator.expPort = new Core::Input::Mahjong( emulator.cpu );           break;
							case EXCITINGBOXING:    emulator.expPort = new Core::Input::ExcitingBoxing( emulator.cpu );    break;
							case TOPRIDER:          emulator.expPort = new Core::Input::TopRider( emulator.cpu );          break;
							case POKKUNMOGURAA:     emulator.expPort = new Core::Input::PokkunMoguraa( emulator.cpu );     break;
							case PARTYTAP:          emulator.expPort = new Core::Input::PartyTap( emulator.cpu );          break;
							default: return RESULT_ERR_INVALID_PARAM;
						}
						break;

					default: return RESULT_ERR_INVALID_PARAM;
				}
			}
			catch (const std::bad_alloc&)
			{
				return RESULT_ERR_OUT_OF_MEMORY;
			}

			delete old;
			emulator.InitializeInputDevices();

			return RESULT_OK;
		}

		void Input::ConnectAdapter(Adapter adapter) throw()
		{
			emulator.extPort->SetType( adapter );
		}

		Result Input::AutoSelectController(uint port) throw()
		{
			if (port >= NUM_PORTS)
				return RESULT_ERR_INVALID_PARAM;

			Type type;

			if (emulator.image)
			{
				type = (Type) emulator.image->GetDesiredController( port );
			}
			else switch (port)
			{
				case PORT_1: type = PAD1;        break;
				case PORT_2: type = PAD2;        break;
				default:     type = UNCONNECTED; break;
			}

			return ConnectController( port, type );
		}

		void Input::AutoSelectAdapter() throw()
		{
			ConnectAdapter( emulator.image ? (Adapter) emulator.image->GetDesiredAdapter() : ADAPTER_NES );
		}

		Input::Type Input::GetConnectedController(uint port) const throw()
		{
			if (port == EXPANSION_PORT)
				return emulator.expPort->GetType();

			if (port < emulator.extPort->NumPorts())
				return emulator.extPort->GetDevice( port )->GetType();

			return UNCONNECTED;
		}

		Input::Adapter Input::GetConnectedAdapter() const throw()
		{
			return emulator.extPort->GetType();
		}

		bool Input::IsControllerConnected(Type type) const throw()
		{
			if (emulator.expPort->GetType() == type)
				return true;

			for (uint ports=emulator.extPort->NumPorts(), i=0; i < ports; ++i)
			{
				if (emulator.extPort->GetDevice(i)->GetType() == type)
					return true;
			}

			return false;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
