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

#include <algorithm>
#include "NstIoLog.hpp"
#include "NstWindowUser.hpp"
#include "NstWindowParam.hpp"
#include "NstResourceString.hpp"
#include "NstManagerPaths.hpp"
#include "NstApplicationInstance.hpp"
#include "NstDialogPaletteEditor.hpp"
#include "NstDialogVideoDecoder.hpp"
#include "NstDialogVideo.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( disable : 279 )
#endif

namespace Nestopia
{
	namespace Window
	{
		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_NTSC_TOP    == IDC_VIDEO_NTSC_LEFT + 1 &&
			IDC_VIDEO_NTSC_RIGHT  == IDC_VIDEO_NTSC_LEFT + 2 &&
			IDC_VIDEO_NTSC_BOTTOM == IDC_VIDEO_NTSC_LEFT + 3 &&
			IDC_VIDEO_PAL_LEFT    == IDC_VIDEO_NTSC_LEFT + 4 &&
			IDC_VIDEO_PAL_TOP     == IDC_VIDEO_NTSC_LEFT + 5 &&
			IDC_VIDEO_PAL_RIGHT   == IDC_VIDEO_NTSC_LEFT + 6 &&
			IDC_VIDEO_PAL_BOTTOM  == IDC_VIDEO_NTSC_LEFT + 7
		);

		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_16_BIT == IDC_VIDEO_8_BIT + 1 &&
			IDC_VIDEO_32_BIT == IDC_VIDEO_8_BIT + 2
		);

		NST_COMPILE_ASSERT
		(
			IDC_VIDEO_PALETTE_YUV      == IDC_VIDEO_PALETTE_AUTO + 1 &&
			IDC_VIDEO_PALETTE_RGB      == IDC_VIDEO_PALETTE_AUTO + 2 &&
			IDC_VIDEO_PALETTE_CUSTOM   == IDC_VIDEO_PALETTE_AUTO + 3 &&
			IDC_VIDEO_PALETTE_PATH     == IDC_VIDEO_PALETTE_AUTO + 4 &&
			IDC_VIDEO_PALETTE_BROWSE   == IDC_VIDEO_PALETTE_AUTO + 5 &&
			IDC_VIDEO_PALETTE_CLEAR    == IDC_VIDEO_PALETTE_AUTO + 6 &&
			IDC_VIDEO_PALETTE_EDITOR   == IDC_VIDEO_PALETTE_AUTO + 7
		);

		NST_COMPILE_ASSERT
		(
			Nes::Video::MIN_BRIGHTNESS == -100 && Nes::Video::DEFAULT_BRIGHTNESS == 0 && Nes::Video::MAX_BRIGHTNESS == +100 &&
			Nes::Video::MIN_SATURATION == -100 && Nes::Video::DEFAULT_SATURATION == 0 && Nes::Video::MAX_SATURATION == +100 &&
			Nes::Video::MIN_CONTRAST   == -100 && Nes::Video::DEFAULT_CONTRAST   == 0 && Nes::Video::MAX_CONTRAST   == +100
		);

		struct Video::Handlers
		{
			static const MsgHandler::Entry<Video> messages[];
			static const MsgHandler::Entry<Video> commands[];
		};

		const MsgHandler::Entry<Video> Video::Handlers::messages[] =
		{
			{ WM_INITDIALOG, &Video::OnInitDialog },
			{ WM_DESTROY,    &Video::OnDestroy    },
			{ WM_HSCROLL,    &Video::OnHScroll    }
		};

		const MsgHandler::Entry<Video> Video::Handlers::commands[] =
		{
			{ IDC_VIDEO_DEVICE,            &Video::OnCmdDevice         },
			{ IDC_VIDEO_MODE,              &Video::OnCmdMode           },
			{ IDC_VIDEO_8_BIT,             &Video::OnCmdBitDepth       },
			{ IDC_VIDEO_16_BIT,            &Video::OnCmdBitDepth       },
			{ IDC_VIDEO_32_BIT,            &Video::OnCmdBitDepth       },
			{ IDC_VIDEO_EFFECTS,           &Video::OnCmdFilter         },
			{ IDC_VIDEO_FILTER_SETTINGS,   &Video::OnCmdFilterSettings },
			{ IDC_VIDEO_PALETTE_AUTO,      &Video::OnCmdPalType        },
			{ IDC_VIDEO_PALETTE_YUV,       &Video::OnCmdPalType        },
			{ IDC_VIDEO_PALETTE_RGB,       &Video::OnCmdPalType        },
			{ IDC_VIDEO_PALETTE_CUSTOM,    &Video::OnCmdPalType        },
			{ IDC_VIDEO_COLORS_ADVANCED,   &Video::OnCmdColorsAdvanced },
			{ IDC_VIDEO_COLORS_RESET,      &Video::OnCmdColorsReset    },
			{ IDC_VIDEO_PALETTE_BROWSE,    &Video::OnCmdPalBrowse      },
			{ IDC_VIDEO_PALETTE_CLEAR,     &Video::OnCmdPalClear       },
			{ IDC_VIDEO_PALETTE_EDITOR,    &Video::OnCmdPalEditor      },
			{ IDC_VIDEO_DEFAULT,           &Video::OnCmdDefault        },
			{ IDOK,                        &Video::OnCmdOk             }
		};

		class Video::Settings::Backup
		{
			const Adapters::const_iterator adapter;
			const Modes::const_iterator mode;
			Filter::Settings* const filter;
			Filter::Settings filters[Filter::NUM_TYPES];
			const i16 screenCurvature;
			const bool autoPalette;
			const i8 brightness;
			const i8 saturation;
			const i8 contrast;
			const i8 hue;
			const i8 sharpness;
			const i8 colorResolution;
			const i8 colorBleed;
			const i8 colorArtifacts;
			const i8 colorFringing;
			const Nes::Video::Decoder decoder;
			const Path palettePath;
			const Nes::Video::Palette::Mode paletteMode;
			const Nes::Video::Palette::CustomType paletteCustomType;
			u8 paletteData[Nes::Video::Palette::NUM_ENTRIES_EXT][3];

		public:

			Backup(const Settings& settings,Nes::Video nes)
			:
			adapter           (settings.adapter),
			mode              (settings.mode),
			filter            (settings.filter),
			screenCurvature   (settings.screenCurvature),
			autoPalette       (settings.autoPalette),
			brightness        (nes.GetBrightness()),
			saturation        (nes.GetSaturation()),
			contrast          (nes.GetContrast()),
			hue               (nes.GetHue()),
			sharpness         (nes.GetSharpness()),
			colorResolution   (nes.GetColorResolution()),
			colorBleed        (nes.GetColorBleed()),
			colorArtifacts    (nes.GetColorArtifacts()),
			colorFringing     (nes.GetColorFringing()),
			decoder           (nes.GetDecoder()),
			palettePath       (settings.palette),
			paletteMode       (nes.GetPalette().GetMode()),
			paletteCustomType (nes.GetPalette().GetCustomType())
			{
				for (uint i=0; i < Filter::NUM_TYPES; ++i)
					filters[i] = settings.filters[i];

				if (paletteMode == Nes::Video::Palette::MODE_CUSTOM)
					nes.GetPalette().GetCustom( paletteData, paletteCustomType );
			}

			void Restore(Settings& settings,Nes::Video nes) const
			{
				settings.adapter = adapter;
				settings.mode = mode;
				settings.filter = filter;

				for (uint i=0; i < Filter::NUM_TYPES; ++i)
					settings.filters[i] = filters[i];

				settings.screenCurvature = screenCurvature;
				settings.palette = palettePath;
				settings.autoPalette = autoPalette;

				nes.SetBrightness( brightness );
				nes.SetSaturation( saturation );
				nes.SetContrast( contrast );
				nes.SetHue( hue );
				nes.SetSharpness( sharpness );
				nes.SetColorResolution( colorResolution );
				nes.SetColorBleed( colorBleed );
				nes.SetColorArtifacts( colorArtifacts );
				nes.SetColorFringing( colorFringing );
				nes.SetDecoder( decoder );
				nes.GetPalette().SetMode( paletteMode );

				if (paletteMode == Nes::Video::Palette::MODE_CUSTOM)
					nes.GetPalette().SetCustom( paletteData, paletteCustomType );
			}
		};

		Video::Settings::Settings()
		: fullscreenScale(SCREEN_MATCHED), backup(NULL) {}

		Video::Settings::~Settings()
		{
			delete backup;
		}

		Video::Video
		(
			Managers::Emulator& emulator,
			const Adapters& a,
			const Managers::Paths& p,
			const Configuration& cfg
		)
		:
		adapters ( a ),
		nes      ( emulator ),
		dialog   ( IDD_VIDEO, this, Handlers::messages, Handlers::commands ),
		paths    ( p )
		{
			settings.adapter = std::find
			(
				adapters.begin(),
				adapters.end(),
				System::Guid( cfg["video device"] )
			);

			if (settings.adapter == adapters.end())
				settings.adapter = adapters.begin();

			settings.mode = settings.adapter->modes.find
			(
				Mode
				(
					cfg[ "video fullscreen width"  ],
					cfg[ "video fullscreen height" ],
					cfg[ "video fullscreen bpp"    ]
				)
			);

			if (settings.mode == settings.adapter->modes.end())
				settings.mode = GetDefaultMode();

			if (GenericString(cfg["view size fullscreen"]) == _T("stretched"))
				settings.fullscreenScale = SCREEN_STRETCHED;

			settings.filter = settings.filters + Filter::Load
			(
				cfg,
				settings.filters,
				Nes::Video(nes),
				settings.adapter->maxScreenSize,
				settings.adapter->filters & Adapter::FILTER_BILINEAR,
				GetDesiredPaletteMode()
			);

			{
				const GenericString type( cfg["video texture location"] );

				if (type == _T("sysmem"))
					settings.texMem = Settings::TEXMEM_SYSMEM;
				else
					settings.texMem = Settings::TEXMEM_VIDMEM;
			}

			{
				Rect& ntsc = settings.rects.ntsc;
				Rect& pal = settings.rects.pal;

				ntsc.left   = cfg[ "video ntsc left"   ].Default( 0                  );
				ntsc.top    = cfg[ "video ntsc top"    ].Default( NTSC_CLIP_TOP      );
				ntsc.right  = cfg[ "video ntsc right"  ].Default( NES_WIDTH-1        );
				ntsc.bottom = cfg[ "video ntsc bottom" ].Default( NTSC_CLIP_BOTTOM-1 );
				pal.left    = cfg[ "video pal left"    ].Default( 0                  );
				pal.top     = cfg[ "video pal top"     ].Default( PAL_CLIP_TOP       );
				pal.right   = cfg[ "video pal right"   ].Default( NES_WIDTH-1        );
				pal.bottom  = cfg[ "video pal bottom"  ].Default( PAL_CLIP_BOTTOM-1  );

				ValidateRects();
			}

			{
				uint value;

				if (200 >= (value=cfg["video color brightness"].Default( 100 )))
					Nes::Video(nes).SetBrightness( int(value) - 100 );

				if (200 >= (value=cfg["video color saturation"].Default( 100 )))
					Nes::Video(nes).SetSaturation( int(value) - 100 );

				if (200 >= (value=cfg["video color contrast"].Default( 100 )))
					Nes::Video(nes).SetContrast( int(value) - 100 );
			}

			{
				int hue = (uint) cfg["video color hue"].Default( 0 );

				if (hue > 180 && hue <= 360)
					hue -= 360;

				if (hue >= Nes::Video::MIN_HUE && hue <= Nes::Video::MAX_HUE)
					Nes::Video(nes).SetHue( hue );
			}

			settings.palette = cfg["video palette file"];
			ImportPalette( settings.palette, Managers::Paths::QUIETLY );

			{
				settings.autoPalette = false;
				const GenericString type( cfg["video palette"] );

				Nes::Video::Palette::Mode mode;

				if (type == _T("yuv"))
				{
					mode = Nes::Video::Palette::MODE_YUV;
				}
				else if (type == _T("rgb"))
				{
					mode = Nes::Video::Palette::MODE_RGB;
				}
				else if (type == _T("custom") && settings.palette.Length())
				{
					mode = Nes::Video::Palette::MODE_CUSTOM;
				}
				else
				{
					settings.autoPalette = true;
					mode = Nes::Video::Palette::MODE_YUV;
				}

				Nes::Video(nes).GetPalette().SetMode( mode );
			}

			{
				int value;

				if (MAX_SCREEN_CURVATURE-MIN_SCREEN_CURVATURE >= (value=cfg["video screen curvature"].Default( MAX_SCREEN_CURVATURE )))
					value -= MAX_SCREEN_CURVATURE;

				settings.screenCurvature = value;
			}

			if (settings.screenCurvature > MAX_SCREEN_CURVATURE || settings.screenCurvature < MIN_SCREEN_CURVATURE)
				settings.screenCurvature = 0;

			settings.autoHz = (cfg["video auto display frequency"] != Configuration::NO);
			settings.tvAspect = (cfg["video tv aspect ratio"] == Configuration::YES);

			VideoDecoder::Load( cfg, Nes::Video(nes) );

			UpdateFinalRects();
		}

		void Video::Save(Configuration& cfg) const
		{
			if (settings.adapter != adapters.end())
			{
				cfg[ "video device" ].Quote() = settings.adapter->guid.GetString();

				if (settings.mode != settings.adapter->modes.end())
				{
					cfg[ "video fullscreen width"  ] = settings.mode->width;
					cfg[ "video fullscreen height" ] = settings.mode->height;
					cfg[ "video fullscreen bpp"    ] = settings.mode->bpp;
				}
			}

			Filter::Save
			(
				cfg,
				settings.filters,
				Nes::Video(nes),
				(Filter::Type) (settings.filter - settings.filters)
			);

			cfg[ "video texture location" ] = (settings.texMem == Settings::TEXMEM_SYSMEM ? "sysmem" : "vidmem");

			{
				GenericString name;

				if (settings.autoPalette)
				{
					name = _T("auto");
				}
				else switch (Nes::Video(nes).GetPalette().GetMode())
				{
					case Nes::Video::Palette::MODE_YUV: name = _T("yuv");    break;
					case Nes::Video::Palette::MODE_RGB: name = _T("rgb");    break;
					default:                            name = _T("custom"); break;
				}

				cfg[ "video palette" ] = name;
			}

			cfg[ "video palette file"           ].Quote() = settings.palette;
			cfg[ "video ntsc left"              ] = settings.rects.ntsc.left;
			cfg[ "video ntsc top"               ] = settings.rects.ntsc.top;
			cfg[ "video ntsc right"             ] = settings.rects.ntsc.right - 1;
			cfg[ "video ntsc bottom"            ] = settings.rects.ntsc.bottom - 1;
			cfg[ "video pal left"               ] = settings.rects.pal.left;
			cfg[ "video pal top"                ] = settings.rects.pal.top;
			cfg[ "video pal right"              ] = settings.rects.pal.right - 1;
			cfg[ "video pal bottom"             ] = settings.rects.pal.bottom - 1;
			cfg[ "video color brightness"       ] = uint( 100 + Nes::Video(nes).GetBrightness() );
			cfg[ "video color saturation"       ] = uint( 100 + Nes::Video(nes).GetSaturation() );
			cfg[ "video color contrast"         ] = uint( 100 + Nes::Video(nes).GetContrast()   );
			cfg[ "video color hue"              ] = uint( Nes::Video(nes).GetHue() + (Nes::Video(nes).GetHue() < 0 ? 360 : 0) );
			cfg[ "video screen curvature"       ] = uint( MAX_SCREEN_CURVATURE + settings.screenCurvature );
			cfg[ "video auto display frequency" ].YesNo() = settings.autoHz;
			cfg[ "video tv aspect ratio"        ].YesNo() = settings.tvAspect;
			cfg[ "view size fullscreen"         ] = (settings.fullscreenScale == SCREEN_STRETCHED ? _T("stretched") : _T("matched"));

			VideoDecoder::Save( cfg, Nes::Video(nes) );
		}

		Nes::Video::Palette::Mode Video::GetDesiredPaletteMode() const
		{
			if (Nes::Machine(nes).Is(Nes::Machine::VS) || Nes::Machine(nes).Is(Nes::Machine::PC10))
				return Nes::Video::Palette::MODE_RGB;
			else
				return Nes::Video::Palette::MODE_YUV;
		}

		void Video::UpdateAutoModes() const
		{
			const Nes::Video::Palette::Mode mode = GetDesiredPaletteMode();

			if (settings.autoPalette && settings.lockedPalette.Empty())
				Nes::Video(nes).GetPalette().SetMode( mode );

			VideoFilters::UpdateAutoModes( settings.filters, nes, mode );
		}

		void Video::GetRenderState(Nes::Video::RenderState& state,float rect[4],const Point screen) const
		{
			typedef Nes::Video::RenderState State;

			{
				const Rect& nesRect = (Nes::Machine(nes).GetMode() == Nes::Machine::NTSC ? settings.rects.ntsc : settings.rects.pal);

				rect[0] = nesRect.left;
				rect[1] = nesRect.top;
				rect[2] = nesRect.right;
				rect[3] = nesRect.bottom;
			}

			state.width = NES_WIDTH;
			state.height = NES_HEIGHT;
			state.scanlines = 0;
			state.filter = State::FILTER_NONE;

			uint scale = 1;

			switch (settings.filter - settings.filters)
			{
				case Filter::TYPE_SCANLINES:

					NST_ASSERT( settings.adapter->maxScreenSize >= Filter::MAX_2X_SIZE );

					state.scanlines = settings.filters[Filter::TYPE_SCANLINES].attributes[Filter::ATR_SCANLINES];

					if (state.scanlines && screen.y >= (rect[3]-rect[1]) * 2)
						scale = 2;

					break;

				case Filter::TYPE_NTSC:

					NST_ASSERT( settings.adapter->maxScreenSize >= Filter::MAX_NTSC_SIZE );

					if (rect[0] < 1)
						rect[0] = 1;

					rect[0] *= float( NTSC_WIDTH  ) / NES_WIDTH;
					rect[1] *= float( NTSC_HEIGHT ) / NES_HEIGHT;
					rect[2] *= float( NTSC_WIDTH  ) / NES_WIDTH;
					rect[3] *= float( NTSC_HEIGHT ) / NES_HEIGHT;

					state.width = NTSC_WIDTH;
					state.height = NTSC_HEIGHT;
					state.filter = State::FILTER_NTSC;
					state.scanlines = settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_SCANLINES];
					break;

				case Filter::TYPE_2XSAI:

					NST_ASSERT( settings.adapter->maxScreenSize >= Filter::MAX_2X_SIZE );

					scale = 2;

					switch (settings.filters[Filter::TYPE_2XSAI].attributes[Filter::ATR_TYPE])
					{
						case Filter::ATR_SUPER2XSAI: state.filter = State::FILTER_SUPER_2XSAI; break;
						case Filter::ATR_SUPEREAGLE: state.filter = State::FILTER_SUPER_EAGLE; break;
						default:                     state.filter = State::FILTER_2XSAI;       break;
					}
					break;

				case Filter::TYPE_SCALEX:
				{
					NST_ASSERT( settings.adapter->maxScreenSize >= Filter::MAX_2X_SIZE );

					int attribute = settings.filters[Filter::TYPE_SCALEX].attributes[Filter::ATR_TYPE];

					if (attribute == Filter::ATR_SCALEAX)
					{
						const Point nes( rect[2]-rect[0], rect[3]-rect[1] );

						if (screen.x >= nes.x*3 && screen.y >= nes.y*3)
						{
							attribute = Filter::ATR_SCALE3X;
						}
						else if (screen.x >= nes.x*2 && screen.y >= nes.y*2)
						{
							attribute = Filter::ATR_SCALE2X;
						}
					}

					switch (attribute)
					{
						case Filter::ATR_SCALE3X:

							if (settings.adapter->maxScreenSize >= Filter::MAX_3X_SIZE)
							{
								scale = 3;
								state.filter = State::FILTER_SCALE3X;
								break;
							}

						case Filter::ATR_SCALE2X:

							scale = 2;
							state.filter = State::FILTER_SCALE2X;
							break;
					}
					break;
				}

				case Filter::TYPE_HQX:
				{
					NST_ASSERT( settings.adapter->maxScreenSize >= Filter::MAX_2X_SIZE );

					int attribute = settings.filters[Filter::TYPE_HQX].attributes[Filter::ATR_TYPE];

					if (attribute == Filter::ATR_HQAX)
					{
						const Point nes( rect[2]-rect[0], rect[3]-rect[1] );

						if (screen.x >= nes.x*4 && screen.y >= nes.y*4)
						{
							attribute = Filter::ATR_HQ4X;
						}
						else if (screen.x >= nes.x*3 && screen.y >= nes.y*3)
						{
							attribute = Filter::ATR_HQ3X;
						}
						else if (screen.x >= nes.x*2 && screen.y >= nes.y*2)
						{
							attribute = Filter::ATR_HQ2X;
						}
					}

					switch (attribute)
					{
						case Filter::ATR_HQ4X:

							if (settings.adapter->maxScreenSize >= Filter::MAX_4X_SIZE)
							{
								scale = 4;
								state.filter = State::FILTER_HQ4X;
								break;
							}

						case Filter::ATR_HQ3X:

							if (settings.adapter->maxScreenSize >= Filter::MAX_3X_SIZE)
							{
								scale = 3;
								state.filter = State::FILTER_HQ3X;
								break;
							}

						case Filter::ATR_HQ2X:

							scale = 2;
							state.filter = State::FILTER_HQ2X;
							break;
					}
					break;
				}
			}

			state.width = state.width * scale;
			state.height = state.height * scale;

			for (uint i=0; i < 4; ++i)
				rect[i] *= scale;
		}

		void Video::LoadGamePalette(const Path& path)
		{
			if (path.Length())
			{
				settings.lockedPalette = path;
				settings.lockedMode = Nes::Video(nes).GetPalette().GetMode();
				Nes::Video(nes).GetPalette().SetMode( Nes::Video::Palette::MODE_CUSTOM );
				ImportPalette( settings.lockedPalette, Managers::Paths::QUIETLY );
			}
		}

		void Video::UnloadGamePalette()
		{
			if (settings.lockedPalette.Length())
			{
				settings.lockedPalette.Destroy();
				Nes::Video(nes).GetPalette().SetMode( settings.lockedMode );
				ImportPalette( settings.palette, Managers::Paths::QUIETLY );
			}
		}

		void Video::SavePalette(Path& path) const
		{
			if (Nes::Video(nes).GetPalette().GetMode() == Nes::Video::Palette::MODE_CUSTOM)
			{
				if (settings.lockedPalette.Length())
				{
					path = settings.lockedPalette;
				}
				else if (settings.palette.Length())
				{
					path = settings.palette;
				}
			}
		}

		Video::Modes::const_iterator Video::GetDialogMode() const
		{
			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
			{
				if (it->bpp == settings.mode->bpp && it->width >= DEFAULT_WIDTH && it->height >= DEFAULT_HEIGHT)
					return it;
			}

			return settings.adapter->modes.begin();
		}

		uint Video::GetFullscreenScaleMethod() const
		{
			return settings.filter == settings.filters+Filter::TYPE_NTSC ? settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_RESCALE_PIC] ? 1 : 2 : 0;
		}

		void Video::UpdateFullscreenScaleMethod(uint prev)
		{
			if (settings.fullscreenScale != SCREEN_STRETCHED && prev != GetFullscreenScaleMethod())
				settings.fullscreenScale = SCREEN_MATCHED;
		}

		void Video::UpdateFinalRects()
		{
			for (uint i=0; i < 2; ++i)
			{
				Rect& rect = (i ? settings.rects.outPal : settings.rects.outNtsc);
				rect = (i ? settings.rects.pal : settings.rects.ntsc);

				if (settings.filter == settings.filters + Filter::TYPE_NTSC)
				{
					if (rect.left < 1)
						rect.left = 1;

					if (settings.tvAspect || !settings.filters[Filter::TYPE_NTSC].attributes[Filter::ATR_RESCALE_PIC])
					{
						rect.left   = (rect.left   * NTSC_WIDTH  + NES_WIDTH  / 2) / NES_WIDTH;
						rect.top    = (rect.top    * NTSC_HEIGHT + NES_HEIGHT / 2) / NES_HEIGHT;
						rect.right  = (rect.right  * NTSC_WIDTH  + NES_WIDTH  / 2) / NES_WIDTH;
						rect.bottom = (rect.bottom * NTSC_HEIGHT + NES_HEIGHT / 2) / NES_HEIGHT;
					}
					else
					{
						rect.right += 2;
					}
				}
				else if (settings.tvAspect)
				{
					rect.left  = (rect.left  * TV_WIDTH + NES_WIDTH / 2) / NES_WIDTH;
					rect.right = (rect.right * TV_WIDTH + NES_WIDTH / 2) / NES_WIDTH;
				}
			}
		}

		void Video::ValidateRects()
		{
			Settings::Rects& r = settings.rects;

			r.ntsc.left   = NST_CLAMP( r.ntsc.left,   0,           NES_WIDTH  -1 );
			r.ntsc.top    = NST_CLAMP( r.ntsc.top,    0,           NES_HEIGHT -1 );
			r.ntsc.right  = NST_CLAMP( r.ntsc.right,  r.ntsc.left, NES_WIDTH  -1 ) + 1;
			r.ntsc.bottom = NST_CLAMP( r.ntsc.bottom, r.ntsc.top,  NES_HEIGHT -1 ) + 1;
			r.pal.left    = NST_CLAMP( r.pal.left,    0,           NES_WIDTH  -1 );
			r.pal.top     = NST_CLAMP( r.pal.top,     0,           NES_HEIGHT -1 );
			r.pal.right   = NST_CLAMP( r.pal.right,   r.pal.left,  NES_WIDTH  -1 ) + 1;
			r.pal.bottom  = NST_CLAMP( r.pal.bottom,  r.pal.top,   NES_HEIGHT -1 ) + 1;
		}

		void Video::ResetColors()
		{
			Nes::Video(nes).SetBrightness ( Nes::Video::DEFAULT_BRIGHTNESS );
			Nes::Video(nes).SetSaturation ( Nes::Video::DEFAULT_SATURATION );
			Nes::Video(nes).SetContrast   ( Nes::Video::DEFAULT_CONTRAST   );
			Nes::Video(nes).SetHue        ( Nes::Video::DEFAULT_HUE        );
		}

		Video::Modes::const_iterator Video::GetDefaultMode() const
		{
			for (uint bpp=16; bpp <= 32; bpp += 16)
			{
				const Modes::const_iterator it( settings.adapter->modes.find(Mode(DEFAULT_WIDTH,DEFAULT_HEIGHT,bpp)) );

				if (it != settings.adapter->modes.end())
					return it;
			}

			return settings.adapter->modes.begin();
		}

		ibool Video::OnInitDialog(Param&)
		{
			NST_ASSERT( settings.backup == NULL );
			settings.backup = new Settings::Backup( settings, Nes::Video(nes) );

			{
				const Control::ComboBox comboBox( dialog.ComboBox(IDC_VIDEO_DEVICE) );

				for (Adapters::const_iterator it(adapters.begin()), end(adapters.end()); it != end; ++it)
				{
					comboBox.Add( it->name.Ptr() );

					if (settings.adapter == it)
						comboBox[it - adapters.begin()].Select();
				}
			}

			for (uint i=IDC_VIDEO_NTSC_LEFT; i <= IDC_VIDEO_PAL_BOTTOM; ++i)
				dialog.Edit( i ).Limit( 3 );

			dialog.Slider( IDC_VIDEO_COLORS_BRIGHTNESS ).SetRange( 0, Nes::Video::MAX_BRIGHTNESS-Nes::Video::MIN_BRIGHTNESS );
			dialog.Slider( IDC_VIDEO_COLORS_SATURATION ).SetRange( 0, Nes::Video::MAX_SATURATION-Nes::Video::MIN_SATURATION );
			dialog.Slider( IDC_VIDEO_COLORS_CONTRAST   ).SetRange( 0, Nes::Video::MAX_CONTRAST-Nes::Video::MIN_CONTRAST );
			dialog.Slider( IDC_VIDEO_COLORS_HUE        ).SetRange( 0, Nes::Video::MAX_HUE-Nes::Video::MIN_HUE );

			dialog.Slider( IDC_VIDEO_SC_SLIDER ).SetRange( 0, MAX_SCREEN_CURVATURE-MIN_SCREEN_CURVATURE );

			dialog.CheckBox( IDC_VIDEO_AUTO_HZ ).Check( settings.autoHz );

			dialog.RadioButton( settings.texMem == Settings::TEXMEM_VIDMEM ? IDC_VIDEO_NESTEXTURE_VIDMEM : IDC_VIDEO_NESTEXTURE_SYSMEM ).Check();

			UpdateDevice( *settings.mode );
			UpdateRects( settings.rects.ntsc, settings.rects.pal );
			UpdateColors();
			UpdateScreenCurvature();
			UpdatePalette();

			return true;
		}

		ibool Video::OnDestroy(Param&)
		{
			if (settings.backup)
			{
				settings.backup->Restore( settings, Nes::Video(nes) );

				delete settings.backup;
				settings.backup = NULL;
			}
			else
			{
				settings.autoHz = dialog.CheckBox(IDC_VIDEO_AUTO_HZ).Checked();

				if (dialog.RadioButton(IDC_VIDEO_NESTEXTURE_VIDMEM).Checked())
					settings.texMem = Settings::TEXMEM_VIDMEM;
				else
					settings.texMem = Settings::TEXMEM_SYSMEM;

				dialog.Control( IDC_VIDEO_NTSC_LEFT   ).Text() >> settings.rects.ntsc.left;
				dialog.Control( IDC_VIDEO_NTSC_TOP    ).Text() >> settings.rects.ntsc.top;
				dialog.Control( IDC_VIDEO_NTSC_RIGHT  ).Text() >> settings.rects.ntsc.right;
				dialog.Control( IDC_VIDEO_NTSC_BOTTOM ).Text() >> settings.rects.ntsc.bottom;
				dialog.Control( IDC_VIDEO_PAL_LEFT    ).Text() >> settings.rects.pal.left;
				dialog.Control( IDC_VIDEO_PAL_TOP     ).Text() >> settings.rects.pal.top;
				dialog.Control( IDC_VIDEO_PAL_RIGHT   ).Text() >> settings.rects.pal.right;
				dialog.Control( IDC_VIDEO_PAL_BOTTOM  ).Text() >> settings.rects.pal.bottom;

				ValidateRects();
			}

			UpdateFinalRects();

			return true;
		}

		ibool Video::OnHScroll(Param& param)
		{
			const int value = param.Slider().Scroll();

			switch (param.Slider().GetId())
			{
				case IDC_VIDEO_COLORS_BRIGHTNESS:

					Nes::Video(nes).SetBrightness( value + Nes::Video::MIN_BRIGHTNESS );
					break;

				case IDC_VIDEO_COLORS_SATURATION:

					Nes::Video(nes).SetSaturation( value + Nes::Video::MIN_SATURATION );
					break;

				case IDC_VIDEO_COLORS_CONTRAST:

					Nes::Video(nes).SetContrast( value + Nes::Video::MIN_CONTRAST );
					break;

				case IDC_VIDEO_COLORS_HUE:

					Nes::Video(nes).SetHue( value + Nes::Video::MIN_HUE );
					break;

				case IDC_VIDEO_SC_SLIDER:

					settings.screenCurvature = value + MIN_SCREEN_CURVATURE;
					UpdateScreenCurvature();
					Application::Instance::GetMainWindow().Redraw();

				default:

					return true;
			}

			UpdateColors();
			Application::Instance::GetMainWindow().Redraw();

			return true;
		}

		ibool Video::OnCmdDevice(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				settings.adapter = adapters.begin();

				for (uint i=dialog.ComboBox( IDC_VIDEO_DEVICE ).Selection().GetIndex(); i; --i)
					++settings.adapter;

				UpdateDevice( *settings.mode );
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdBitDepth(Param& param)
		{
			if (param.Button().Clicked())
				UpdateResolutions( Mode(settings.mode->width,settings.mode->height,8U << (param.Button().GetId() - IDC_VIDEO_8_BIT)) );

			return true;
		}

		ibool Video::OnCmdMode(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				settings.mode = settings.adapter->modes.begin();

				for (uint i=dialog.ComboBox(IDC_VIDEO_MODE).Selection().Data(); i; --i)
					++settings.mode;
			}

			return true;
		}

		ibool Video::OnCmdFilter(Param& param)
		{
			if (param.ComboBox().SelectionChanged())
			{
				const uint method = GetFullscreenScaleMethod();
				settings.filter = settings.filters + (Filter::Type) (Control::ComboBox::Value) dialog.ComboBox( IDC_VIDEO_EFFECTS ).Selection().Data();
				UpdateFullscreenScaleMethod( method );

				UpdatePalette();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdFilterSettings(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint method = GetFullscreenScaleMethod();

				static const ushort idd[] =
				{
					IDD_VIDEO_FILTER_NONE,
					IDD_VIDEO_FILTER_SCANLINES,
					IDD_VIDEO_FILTER_NTSC,
					IDD_VIDEO_FILTER_2XSAI,
					IDD_VIDEO_FILTER_SCALEX,
					IDD_VIDEO_FILTER_HQX
				};

				VideoFilters
				(
					Nes::Video(nes),
					idd[settings.filter-settings.filters],
					*settings.filter,
					settings.adapter->maxScreenSize,
					settings.adapter->filters & Adapter::FILTER_BILINEAR,
					GetDesiredPaletteMode()
				).Open();

				UpdateFullscreenScaleMethod( method );
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdPalType(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint cmd = param.Button().GetId();

				settings.autoPalette = (cmd == IDC_VIDEO_PALETTE_AUTO);

				Nes::Video(nes).GetPalette().SetMode
				(
					cmd == IDC_VIDEO_PALETTE_YUV    ? Nes::Video::Palette::MODE_YUV :
					cmd == IDC_VIDEO_PALETTE_RGB    ? Nes::Video::Palette::MODE_RGB :
					cmd == IDC_VIDEO_PALETTE_CUSTOM ? Nes::Video::Palette::MODE_CUSTOM :
                                                      GetDesiredPaletteMode()
				);

				UpdatePalette();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdColorsReset(Param& param)
		{
			if (param.Button().Clicked())
			{
				ResetColors();
				UpdateColors();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdColorsAdvanced(Param& param)
		{
			if (param.Button().Clicked())
			{
				VideoDecoder( nes ).Open();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdPalBrowse(Param& param)
		{
			if (param.Button().Clicked())
			{
				const Path file
				(
					paths.BrowseLoad
					(
						Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE,
						Application::Instance::GetFullPath( settings.palette )
					)
				);

				if (file.Length())
				{
					settings.palette = file;

					ImportPalette( settings.palette, Managers::Paths::NOISY );
					UpdatePalette();
					Application::Instance::GetMainWindow().Redraw();
				}
			}

			return true;
		}

		ibool Video::OnCmdPalClear(Param& param)
		{
			if (param.Button().Clicked())
			{
				if (settings.palette.Length())
				{
					settings.palette.Destroy();

					if (Nes::Video(nes).GetPalette().GetMode() == Nes::Video::Palette::MODE_CUSTOM)
					{
						Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );
						settings.autoPalette = true;
					}

					UpdatePalette();
					Application::Instance::GetMainWindow().Redraw();
				}
			}

			return true;
		}

		ibool Video::OnCmdPalEditor(Param& param)
		{
			if (param.Button().Clicked())
			{
				Nes::Video video( nes );

				if (settings.palette.Empty())
					video.GetPalette().ResetCustom();

				{
					const Path path( PaletteEditor( video, paths, settings.palette ).Open() );

					if (settings.palette.Empty() && path.Length())
						settings.palette = path;
				}

				if (settings.palette.Length())
				{
					ImportPalette( settings.palette, Managers::Paths::QUIETLY );
					UpdatePalette();
				}

				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdDefault(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint method = GetFullscreenScaleMethod();

				settings.adapter = adapters.begin();
				settings.mode = GetDefaultMode();

				for (uint i=0; i < Filter::NUM_TYPES; ++i)
					settings.filters[i].Reset( (Filter::Type) i );

				settings.filter = settings.filters + Filter::TYPE_NONE;

				UpdateDevice( *settings.mode );

				dialog.RadioButton( IDC_VIDEO_NESTEXTURE_VIDMEM ).Check();
				dialog.RadioButton( IDC_VIDEO_NESTEXTURE_SYSMEM ).Uncheck();

				UpdateRects
				(
					Rect( 0, NTSC_CLIP_TOP, NES_WIDTH, NTSC_CLIP_BOTTOM ),
					Rect( 0, PAL_CLIP_TOP, NES_WIDTH, PAL_CLIP_BOTTOM )
				);

				dialog.CheckBox( IDC_VIDEO_AUTO_HZ ).Check();

				ResetColors();
				UpdateColors();

				settings.autoPalette = true;

				settings.screenCurvature = 0;
				UpdateScreenCurvature();

				const Nes::Video::Palette::Mode mode = GetDesiredPaletteMode();
				Nes::Video video(nes);

				if (settings.lockedPalette.Empty())
					video.GetPalette().SetMode( mode );

				VideoFilters::UpdateAutoModes( settings.filters, video, mode );

				video.SetDecoder( Nes::Video::Decoder(Nes::Video::DECODER_CANONICAL) );

				UpdateFullscreenScaleMethod( method );

				UpdatePalette();
				Application::Instance::GetMainWindow().Redraw();
			}

			return true;
		}

		ibool Video::OnCmdOk(Param& param)
		{
			if (param.Button().Clicked())
			{
				delete settings.backup;
				settings.backup = NULL;

				dialog.Close();
			}

			return true;
		}

		void Video::UpdateDevice(Mode mode)
		{
			dialog.RadioButton( IDC_VIDEO_NESTEXTURE_VIDMEM ).Enable( settings.adapter->videoMemScreen );
			dialog.RadioButton( IDC_VIDEO_NESTEXTURE_SYSMEM ).Enable( settings.adapter->videoMemScreen );

			uint available = 0;

			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it)
			{
				switch (it->bpp)
				{
					case 8:  available |=  8; break;
					case 16: available |= 16; break;
					case 32: available |= 32; break;
				}

				if (available == (8|16|32))
					break;
			}

			NST_ASSERT( available & (8|16|32) );

			dialog.Control( IDC_VIDEO_8_BIT  ).Enable( available & 8  );
			dialog.Control( IDC_VIDEO_16_BIT ).Enable( available & 16 );
			dialog.Control( IDC_VIDEO_32_BIT ).Enable( available & 32 );

			switch (mode.bpp)
			{
				case 32: mode.bpp = ((available & 32) ? 32 : (available & 16) ? 16 :  8); break;
				case 16: mode.bpp = ((available & 16) ? 16 : (available & 32) ? 32 :  8); break;
				case  8: mode.bpp = ((available &  8) ?  8 : (available & 16) ? 16 : 32); break;
			}

			UpdateResolutions( mode );
			UpdateFilters();
		}

		void Video::UpdateResolutions(Mode mode)
		{
			settings.mode = settings.adapter->modes.find( mode );

			if (settings.mode == settings.adapter->modes.end())
			{
				settings.mode = settings.adapter->modes.find( Mode(DEFAULT_WIDTH,DEFAULT_HEIGHT,mode.bpp) );

				if (settings.mode == settings.adapter->modes.end())
					settings.mode = settings.adapter->modes.begin();

				mode = *settings.mode;
			}

			dialog.RadioButton( IDC_VIDEO_8_BIT  ).Check( mode.bpp == 8  );
			dialog.RadioButton( IDC_VIDEO_16_BIT ).Check( mode.bpp == 16 );
			dialog.RadioButton( IDC_VIDEO_32_BIT ).Check( mode.bpp == 32 );

			const Control::ComboBox comboBox( dialog.ComboBox(IDC_VIDEO_MODE) );
			comboBox.Clear();

			uint idx=0;
			HeapString string;

			for (Modes::const_iterator it(settings.adapter->modes.begin()), end(settings.adapter->modes.end()); it != end; ++it, ++idx)
			{
				if (mode.bpp == it->bpp)
				{
					string.Clear();
					comboBox.Add( (string << it->width << 'x' << it->height).Ptr() ).Data() = idx;

					if (mode.width == it->width && mode.height == it->height)
						comboBox[comboBox.Size() - 1].Select();
				}
			}
		}

		void Video::UpdateFilters()
		{
			const Control::ComboBox comboBox( dialog.ComboBox(IDC_VIDEO_EFFECTS) );
			comboBox.Clear();

			comboBox.Add( Resource::String(IDS_NONE) ).Data() = Filter::TYPE_NONE;

			if (settings.adapter->maxScreenSize >= Filter::MAX_2X_SIZE)
			{
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCANLINES) ).Data() = Filter::TYPE_SCANLINES;

				if (settings.adapter->maxScreenSize >= Filter::MAX_NTSC_SIZE)
					comboBox.Add( Resource::String(IDS_VIDEO_FILTER_NTSC) ).Data() = Filter::TYPE_NTSC;

				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_2XSAI) ).Data() = Filter::TYPE_2XSAI;
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_SCALEX) ).Data() = Filter::TYPE_SCALEX;
				comboBox.Add( Resource::String(IDS_VIDEO_FILTER_HQX) ).Data() = Filter::TYPE_HQX;
			}

			for (uint i=0, size=comboBox.Size(); i < size; ++i)
			{
				if (settings.filter-settings.filters == (Filter::Type) (Control::ComboBox::Value) comboBox[i].Data())
				{
					comboBox[i].Select();
					return;
				}
			}

			settings.filter = settings.filters + Filter::TYPE_NONE;
			comboBox[0].Select();
		}

		void Video::UpdateColors() const
		{
			const int colors[] =
			{
				Nes::Video(nes).GetBrightness(),
				Nes::Video(nes).GetSaturation(),
				Nes::Video(nes).GetContrast(),
				Nes::Video(nes).GetHue()
			};

			dialog.Control( IDC_VIDEO_COLORS_RESET ).Enable
			(
				colors[0] != Nes::Video::DEFAULT_BRIGHTNESS ||
				colors[1] != Nes::Video::DEFAULT_SATURATION ||
				colors[2] != Nes::Video::DEFAULT_CONTRAST   ||
				colors[3] != Nes::Video::DEFAULT_HUE
			);

			dialog.Slider( IDC_VIDEO_COLORS_BRIGHTNESS ).Position() = colors[0] - Nes::Video::MIN_BRIGHTNESS;
			dialog.Slider( IDC_VIDEO_COLORS_SATURATION ).Position() = colors[1] - Nes::Video::MIN_SATURATION;
			dialog.Slider( IDC_VIDEO_COLORS_CONTRAST   ).Position() = colors[2] - Nes::Video::MIN_CONTRAST;
			dialog.Slider( IDC_VIDEO_COLORS_HUE        ).Position() = colors[3] - Nes::Video::MIN_HUE;

			dialog.Control( IDC_VIDEO_COLORS_BRIGHTNESS_VAL ).Text() << RealString( colors[0] / 100.0, 2 ).Ptr();
			dialog.Control( IDC_VIDEO_COLORS_SATURATION_VAL ).Text() << RealString( (colors[1] + 100) / 100.0, 2 ).Ptr();
			dialog.Control( IDC_VIDEO_COLORS_CONTRAST_VAL   ).Text() << RealString( (colors[2] + 100) / 100.0, 2 ).Ptr();
			dialog.Control( IDC_VIDEO_COLORS_HUE_VAL        ).Text() << colors[3];
		}

		void Video::UpdateScreenCurvature() const
		{
			dialog.Slider( IDC_VIDEO_SC_SLIDER ).Position() = settings.screenCurvature - MIN_SCREEN_CURVATURE;
			dialog.Control( IDC_VIDEO_SC_VALUE ).Text() << RealString( settings.screenCurvature / 10.0 ).Ptr();
		}

		void Video::UpdateRects(const Rect& ntsc,const Rect& pal) const
		{
			dialog.Control( IDC_VIDEO_NTSC_LEFT   ).Text() << uint(ntsc.left);
			dialog.Control( IDC_VIDEO_NTSC_TOP    ).Text() << uint(ntsc.top);
			dialog.Control( IDC_VIDEO_NTSC_RIGHT  ).Text() << uint(ntsc.right - 1);
			dialog.Control( IDC_VIDEO_NTSC_BOTTOM ).Text() << uint(ntsc.bottom - 1);
			dialog.Control( IDC_VIDEO_PAL_LEFT    ).Text() << uint(pal.left);
			dialog.Control( IDC_VIDEO_PAL_TOP     ).Text() << uint(pal.top);
			dialog.Control( IDC_VIDEO_PAL_RIGHT   ).Text() << uint(pal.right - 1);
			dialog.Control( IDC_VIDEO_PAL_BOTTOM  ).Text() << uint(pal.bottom - 1);
		}

		void Video::UpdatePalette() const
		{
			const ibool unlocked = settings.lockedPalette.Empty();

			for (uint i=IDC_VIDEO_PALETTE_AUTO; i <= IDC_VIDEO_PALETTE_EDITOR; ++i)
				dialog.Control( i ).Enable( unlocked );

			const Nes::Video::Palette::Mode mode = Nes::Video(nes).GetPalette().GetMode();

			dialog.Control( IDC_VIDEO_PALETTE_CLEAR  ).Enable( unlocked && settings.palette.Length() );
			dialog.Control( IDC_VIDEO_PALETTE_CUSTOM ).Enable( unlocked && settings.palette.Length() );
			dialog.Control( IDC_VIDEO_PALETTE_PATH   ).Enable( unlocked && mode == Nes::Video::Palette::MODE_CUSTOM );

			dialog.Edit( IDC_VIDEO_PALETTE_PATH ) << (unlocked ? settings.palette.Ptr() : settings.lockedPalette.Ptr());

			dialog.Control( IDC_VIDEO_COLORS_ADVANCED ).Enable( mode == Nes::Video::Palette::MODE_YUV );

			dialog.RadioButton( IDC_VIDEO_PALETTE_AUTO   ).Check( settings.autoPalette );
			dialog.RadioButton( IDC_VIDEO_PALETTE_YUV    ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_YUV    );
			dialog.RadioButton( IDC_VIDEO_PALETTE_RGB    ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_RGB    );
			dialog.RadioButton( IDC_VIDEO_PALETTE_CUSTOM ).Check( !settings.autoPalette && mode == Nes::Video::Palette::MODE_CUSTOM );
		}

		void Video::ImportPalette(Path& palette,Managers::Paths::Alert alert)
		{
			if (palette.Empty())
				return;

			palette.MakePretty();

			Managers::Paths::File file;

			if (paths.Load( file, Managers::Paths::File::PALETTE|Managers::Paths::File::ARCHIVE, Application::Instance::GetFullPath(palette), alert ))
			{
				if (file.data.Size() >= Nes::Video::Palette::NUM_ENTRIES*3)
				{
					Nes::Video::Palette::CustomType type = Nes::Video::Palette::STD_PALETTE;

					if (file.data.Size() >= Nes::Video::Palette::NUM_ENTRIES_EXT*3)
						type = Nes::Video::Palette::EXT_PALETTE;

					if (NES_SUCCEEDED(Nes::Video(nes).GetPalette().SetCustom( reinterpret_cast<Nes::Video::Palette::Colors>(file.data.Begin()), type )))
						return;
				}
			}

			if (alert == Managers::Paths::QUIETLY)
				Io::Log() << "Video: warning, custom palette file: \"" << palette << "\" invalid or not found!\r\n";
			else
				User::Fail( IDS_DIALOG_VIDEO_INVALID_PALETTE );

			palette.Destroy();
			Nes::Video(nes).GetPalette().SetMode( GetDesiredPaletteMode() );
		}
	}
}
