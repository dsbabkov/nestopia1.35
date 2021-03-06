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

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <new>
#include "NstCore.hpp"
#include "api/NstApiVideo.hpp"
#include "NstFpuPrecision.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilterNone.hpp"
#include "NstVideoFilterScanlines.hpp"
#include "NstVideoFilterNtsc.hpp"
#ifndef NST_NO_2XSAI
#include "NstVideoFilter2xSaI.hpp"
#endif
#ifndef NST_NO_SCALE2X
#include "NstVideoFilterScaleX.hpp"
#endif
#ifndef NST_NO_HQ2X
#include "NstVideoFilterHqX.hpp"
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			const u8 Renderer::Palette::pc10Palette[64][3] =
			{
				{0x6D,0x6D,0x6D}, {0x00,0x24,0x92}, {0x00,0x00,0xDB}, {0x6D,0x49,0xDB},
				{0x92,0x00,0x6D}, {0xB6,0x00,0x6D}, {0xB6,0x24,0x00}, {0x92,0x49,0x00},
				{0x6D,0x49,0x00}, {0x24,0x49,0x00}, {0x00,0x6D,0x24}, {0x00,0x92,0x00},
				{0x00,0x49,0x49}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xB6,0xB6,0xB6}, {0x00,0x6D,0xDB}, {0x00,0x49,0xFF}, {0x92,0x00,0xFF},
				{0xB6,0x00,0xFF}, {0xFF,0x00,0x92}, {0xFF,0x00,0x00}, {0xDB,0x6D,0x00},
				{0x92,0x6D,0x00}, {0x24,0x92,0x00}, {0x00,0x92,0x00}, {0x00,0xB6,0x6D},
				{0x00,0x92,0x92}, {0x24,0x24,0x24}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0x6D,0xB6,0xFF}, {0x92,0x92,0xFF}, {0xDB,0x6D,0xFF},
				{0xFF,0x00,0xFF}, {0xFF,0x6D,0xFF}, {0xFF,0x92,0x00}, {0xFF,0xB6,0x00},
				{0xDB,0xDB,0x00}, {0x6D,0xDB,0x00}, {0x00,0xFF,0x00}, {0x49,0xFF,0xDB},
				{0x00,0xFF,0xFF}, {0x49,0x49,0x49}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
				{0xFF,0xFF,0xFF}, {0xB6,0xDB,0xFF}, {0xDB,0xB6,0xFF}, {0xFF,0xB6,0xFF},
				{0xFF,0x92,0xFF}, {0xFF,0xB6,0xB6}, {0xFF,0xDB,0x92}, {0xFF,0xFF,0x49},
				{0xFF,0xFF,0x6D}, {0xB6,0xFF,0x49}, {0x92,0xFF,0x6D}, {0x49,0xFF,0xDB},
				{0x92,0xDB,0xFF}, {0x92,0x92,0x92}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
			};

			const u8 Renderer::Palette::vsPalette[4][64][3] =
			{
				{
					{0xFF,0xB6,0xB6}, {0xDB,0x6D,0xFF}, {0xFF,0x00,0x00}, {0x92,0x92,0xFF},
					{0x00,0x92,0x92}, {0x24,0x49,0x00}, {0x49,0x49,0x49}, {0xFF,0x00,0x92},
					{0xFF,0xFF,0xFF}, {0x6D,0x6D,0x6D}, {0xFF,0xB6,0x00}, {0xB6,0x00,0x6D},
					{0x92,0x00,0x6D}, {0xDB,0xDB,0x00}, {0x6D,0x49,0x00}, {0xFF,0xFF,0xFF},
					{0x6D,0xB6,0xFF}, {0xDB,0xB6,0x6D}, {0x6D,0x24,0x00}, {0x6D,0xDB,0x00},
					{0x92,0xDB,0xFF}, {0xDB,0xB6,0xFF}, {0xFF,0xDB,0x92}, {0x00,0x49,0xFF},
					{0xFF,0xDB,0x00}, {0x49,0xFF,0xDB}, {0x00,0x00,0x00}, {0x49,0x00,0x00},
					{0xDB,0xDB,0xDB}, {0x92,0x92,0x92}, {0xFF,0x00,0xFF}, {0x00,0x24,0x92},
					{0x00,0x00,0x6D}, {0xB6,0xDB,0xFF}, {0xFF,0xB6,0xFF}, {0x00,0xFF,0x00},
					{0x00,0xFF,0xFF}, {0x00,0x49,0x49}, {0x00,0xB6,0x6D}, {0xB6,0x00,0xFF},
					{0x00,0x00,0x00}, {0x92,0x49,0x00}, {0xFF,0x92,0xFF}, {0xB6,0x24,0x00},
					{0x92,0x00,0xFF}, {0x00,0x00,0xDB}, {0xFF,0x92,0x00}, {0x00,0x00,0x00},
					{0x00,0x00,0x00}, {0x24,0x92,0x00}, {0xB6,0xB6,0xB6}, {0x00,0x6D,0x24},
					{0xB6,0xFF,0x49}, {0x6D,0x49,0xDB}, {0xFF,0xFF,0x00}, {0xDB,0x6D,0x00},
					{0x00,0x49,0x00}, {0x00,0x6D,0xDB}, {0x00,0x92,0x00}, {0x24,0x24,0x24},
					{0xFF,0xFF,0x6D}, {0xFF,0x6D,0xFF}, {0x92,0x6D,0x00}, {0x92,0xFF,0x6D}
				},
				{
					{0x00,0x00,0x00}, {0xFF,0xB6,0x00}, {0x92,0x6D,0x00}, {0xB6,0xFF,0x49},
					{0x92,0xFF,0x6D}, {0xFF,0x6D,0xFF}, {0x00,0x92,0x92}, {0xB6,0xDB,0xFF},
					{0xFF,0x00,0x00}, {0x92,0x00,0xFF}, {0xFF,0xFF,0x6D}, {0xFF,0x92,0xFF},
					{0xFF,0xFF,0xFF}, {0xDB,0x6D,0xFF}, {0x92,0xDB,0xFF}, {0x00,0x92,0x00},
					{0x00,0x49,0x00}, {0x6D,0xB6,0xFF}, {0xB6,0x24,0x00}, {0xDB,0xDB,0xDB},
					{0x00,0xB6,0x6D}, {0x6D,0xDB,0x00}, {0x49,0x00,0x00}, {0x92,0x92,0xFF},
					{0x49,0x49,0x49}, {0xFF,0x00,0xFF}, {0x00,0x00,0x6D}, {0x49,0xFF,0xDB},
					{0xDB,0xB6,0xFF}, {0x6D,0x49,0x00}, {0x00,0x00,0x00}, {0x6D,0x49,0xDB},
					{0x92,0x00,0x6D}, {0xFF,0xDB,0x92}, {0xFF,0x92,0x00}, {0xFF,0xB6,0xFF},
					{0x00,0x6D,0xDB}, {0x6D,0x24,0x00}, {0xB6,0xB6,0xB6}, {0x00,0x00,0xDB},
					{0xB6,0x00,0xFF}, {0xFF,0xDB,0x00}, {0x6D,0x6D,0x6D}, {0x24,0x49,0x00},
					{0x00,0x49,0xFF}, {0x00,0x00,0x00}, {0xDB,0xDB,0x00}, {0xFF,0xFF,0xFF},
					{0xDB,0xB6,0x6D}, {0x24,0x24,0x24}, {0x00,0xFF,0x00}, {0xDB,0x6D,0x00},
					{0x00,0x49,0x49}, {0x00,0x24,0x92}, {0xFF,0x00,0x92}, {0x24,0x92,0x00},
					{0x00,0x00,0x00}, {0x00,0xFF,0xFF}, {0x92,0x49,0x00}, {0xFF,0xFF,0x00},
					{0xFF,0xB6,0xB6}, {0xB6,0x00,0x6D}, {0x00,0x6D,0x24}, {0x92,0x92,0x92}
				},
				{
					{0xB6,0x00,0xFF}, {0xFF,0x6D,0xFF}, {0x92,0xFF,0x6D}, {0xB6,0xB6,0xB6},
					{0x00,0x92,0x00}, {0xFF,0xFF,0xFF}, {0xB6,0xDB,0xFF}, {0x24,0x49,0x00},
					{0x00,0x24,0x92}, {0x00,0x00,0x00}, {0xFF,0xDB,0x92}, {0x6D,0x49,0x00},
					{0xFF,0x00,0x92}, {0xDB,0xDB,0xDB}, {0xDB,0xB6,0x6D}, {0x92,0xDB,0xFF},
					{0x92,0x92,0xFF}, {0x00,0x92,0x92}, {0xB6,0x00,0x6D}, {0x00,0x49,0xFF},
					{0x24,0x92,0x00}, {0x92,0x6D,0x00}, {0xDB,0x6D,0x00}, {0x00,0xB6,0x6D},
					{0x6D,0x6D,0x6D}, {0x6D,0x49,0xDB}, {0x00,0x00,0x00}, {0x00,0x00,0xDB},
					{0xFF,0x00,0x00}, {0xB6,0x24,0x00}, {0xFF,0x92,0xFF}, {0xFF,0xB6,0xB6},
					{0xDB,0x6D,0xFF}, {0x00,0x49,0x00}, {0x00,0x00,0x6D}, {0xFF,0xFF,0x00},
					{0x24,0x24,0x24}, {0xFF,0xB6,0x00}, {0xFF,0x92,0x00}, {0xFF,0xFF,0xFF},
					{0x6D,0xDB,0x00}, {0x92,0x00,0x6D}, {0x6D,0xB6,0xFF}, {0xFF,0x00,0xFF},
					{0x00,0x6D,0xDB}, {0x92,0x92,0x92}, {0x00,0x00,0x00}, {0x6D,0x24,0x00},
					{0x00,0xFF,0xFF}, {0x49,0x00,0x00}, {0xB6,0xFF,0x49}, {0xFF,0xB6,0xFF},
					{0x92,0x49,0x00}, {0x00,0xFF,0x00}, {0xDB,0xDB,0x00}, {0x49,0x49,0x49},
					{0x00,0x6D,0x24}, {0x00,0x00,0x00}, {0xDB,0xB6,0xFF}, {0xFF,0xFF,0x6D},
					{0x92,0x00,0xFF}, {0x49,0xFF,0xDB}, {0xFF,0xDB,0x00}, {0x00,0x49,0x49}
				},
				{
					{0x92,0x6D,0x00}, {0x6D,0x49,0xDB}, {0x00,0x92,0x92}, {0xDB,0xDB,0x00},
					{0x00,0x00,0x00}, {0xFF,0xB6,0xB6}, {0x00,0x24,0x92}, {0xDB,0x6D,0x00},
					{0xB6,0xB6,0xB6}, {0x6D,0x24,0x00}, {0x00,0xFF,0x00}, {0x00,0x00,0x6D},
					{0xFF,0xDB,0x92}, {0xFF,0xFF,0x00}, {0x00,0x92,0x00}, {0xB6,0xFF,0x49},
					{0xFF,0x6D,0xFF}, {0x49,0x00,0x00}, {0x00,0x49,0xFF}, {0xFF,0x92,0xFF},
					{0x00,0x00,0x00}, {0x49,0x49,0x49}, {0xB6,0x24,0x00}, {0xFF,0x92,0x00},
					{0xDB,0xB6,0x6D}, {0x00,0xB6,0x6D}, {0x92,0x92,0xFF}, {0x24,0x92,0x00},
					{0x92,0x00,0x6D}, {0x00,0x00,0x00}, {0x92,0xFF,0x6D}, {0x6D,0xB6,0xFF},
					{0xB6,0x00,0x6D}, {0x00,0x6D,0x24}, {0x92,0x49,0x00}, {0x00,0x00,0xDB},
					{0x92,0x00,0xFF}, {0xB6,0x00,0xFF}, {0x6D,0x6D,0x6D}, {0xFF,0x00,0x92},
					{0x00,0x49,0x49}, {0xDB,0xDB,0xDB}, {0x00,0x6D,0xDB}, {0x00,0x49,0x00},
					{0x24,0x24,0x24}, {0xFF,0xFF,0x6D}, {0x92,0x92,0x92}, {0xFF,0x00,0xFF},
					{0xFF,0xB6,0xFF}, {0xFF,0xFF,0xFF}, {0x6D,0x49,0x00}, {0xFF,0x00,0x00},
					{0xFF,0xDB,0x00}, {0x49,0xFF,0xDB}, {0xFF,0xFF,0xFF}, {0x92,0xDB,0xFF},
					{0x00,0x00,0x00}, {0xFF,0xB6,0x00}, {0xDB,0x6D,0xFF}, {0xB6,0xDB,0xFF},
					{0x6D,0xDB,0x00}, {0xDB,0xB6,0xFF}, {0x00,0xFF,0xFF}, {0x24,0x49,0x00}
				}
			};

			const uchar Renderer::Palette::Constants::tints[8] =
			{
				0, 6, 10, 8, 2, 4, 0, 0
			};

			const double Renderer::Palette::Constants::levels[2][4] =
			{
				{-0.12, 0.00, 0.31, 0.72 },
				{ 0.40, 0.68, 1.00, 1.00 }
			};

			const double Renderer::Palette::Constants::attenMul = 0.79399;
			const double Renderer::Palette::Constants::attenSub = 0.0782838;

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			inline Renderer::Palette::Custom::Custom()
			: emphasis(NULL) {}

			inline Renderer::Palette::Custom::~Custom()
			{
				delete [] emphasis;
			}

			bool Renderer::Palette::Custom::EnableEmphasis(bool enable)
			{
				if (!enable)
				{
					delete [] emphasis;
					emphasis = NULL;
				}
				else if (!emphasis)
				{
					emphasis = new (std::nothrow) u8 [7][64][3];
				}

				return bool(emphasis) == enable;
			}

			Renderer::Palette::Palette()
			: type(PALETTE_YUV), custom(NULL)
			{
				SetDecoder( Api::Video::DECODER_CANONICAL );
			}

			Renderer::Palette::~Palette()
			{
				delete custom;
			}

			Result Renderer::Palette::SetDecoder(const Decoder& d)
			{
				if (decoder == d)
					return RESULT_NOP;

				for (uint i=0; i < 3; ++i)
				{
					if (d.axes[i].angle >= 360 || d.axes[i].gain > 2.0)
						return RESULT_ERR_INVALID_PARAM;
				}

				decoder = d;
				return RESULT_OK;
			}

			void Renderer::Palette::ToHSV(double r,double g,double b,double& h,double& s,double& v)
			{
				const double min = NST_MIN( r, NST_MIN( g, b ));
				const double max = NST_MAX( r, NST_MAX( g, b ));

				v = max;

				if (max > +FLT_EPSILON || max < -FLT_EPSILON)
				{
					const double delta = max - min;

					s = delta / max;

					if      (r == max) h = 0.0 + (g - b) / delta;
					else if (g == max) h = 2.0 + (b - r) / delta;
					else               h = 4.0 + (r - g) / delta;

					h *= 60.0;

					if (h < 0.0)
						h += 360.0;
				}
				else
				{
					s =  0.0;
					h = -1.0;
				}
			}

			void Renderer::Palette::ToRGB(double h,double s,double v,double& r,double& g,double& b)
			{
				if (s <= +FLT_EPSILON && s >= -FLT_EPSILON)
				{
					r = g = b = v;
				}
				else
				{
					h /= 60.0;

					const int i = std::floor( h );

					const double f = h - i;
					const double p = v * ( 1.0 - s );
					const double q = v * ( 1.0 - s * f );
					const double t = v * ( 1.0 - s * ( 1.0 - f ) );

					switch (i)
					{
						case 0:  r = v; g = t; b = p; break;
						case 1:  r = q; g = v; b = p; break;
						case 2:  r = p; g = v; b = t; break;
						case 3:  r = p; g = q; b = v; break;
						case 4:  r = t; g = p; b = v; break;
						default: r = v; g = p; b = q; break;
					}
				}
			}

			void Renderer::Palette::ToPAL(const double (&src)[3],u8 (&dst)[3])
			{
				for (uint i=0; i < 3; ++i)
					dst[i] = (src[i] >= 1.0 ? 255 : src[i] <= 0.0 ? 0 : src[i] * 255.0 + 0.5);
			}

			Result Renderer::Palette::LoadCustom(const u8 (*colors)[3],const bool emphasis)
			{
				if (!colors)
					return RESULT_ERR_INVALID_PARAM;

				if ((custom == NULL && NULL == (custom = new (std::nothrow) Custom)) || !custom->EnableEmphasis( emphasis ))
					return RESULT_ERR_OUT_OF_MEMORY;

				std::memcpy( custom->palette, colors, 64*3 );

				if (emphasis)
					std::memcpy( custom->emphasis, colors + 64, 7*64*3 );

				return RESULT_OK;
			}

			uint Renderer::Palette::SaveCustom(u8 (*colors)[3],const bool emphasis) const
			{
				if (!colors)
					return 0;

				std::memcpy( colors, custom ? custom->palette : pc10Palette, 64*3 );

				if (!emphasis || !custom || !custom->emphasis)
					return 64;

				std::memcpy( colors + 64, custom->emphasis, 7*64*3 );

				return 7*64;
			}

			bool Renderer::Palette::ResetCustom()
			{
				if (custom)
				{
					custom->EnableEmphasis( false );
					std::memcpy( custom->palette, pc10Palette, 64*3 );
					return true;
				}

				return false;
			}

			Result Renderer::Palette::SetType(PaletteType t)
			{
				if (t == type)
					return RESULT_NOP;

				if (t == PALETTE_CUSTOM && !custom)
				{
					if (NULL == (custom = new (std::nothrow) Custom))
						return RESULT_ERR_OUT_OF_MEMORY;

					ResetCustom();
				}

				type = t;

				return RESULT_OK;
			}

			void Renderer::Palette::Build(const int b,const int s,const int c,const int hue)
			{
				NST_ASSERT( type != PALETTE_YUV );

				const double brightness = b / 200.0;
				const double saturation = (s + 100) / 100.0;
				const double contrast = (c + 100) / 100.0;

				const u8 (*from)[3] =
				(
					type == PALETTE_CUSTOM ? custom->palette :
					type == PALETTE_VS1    ? vsPalette[0] :
					type == PALETTE_VS2    ? vsPalette[1] :
					type == PALETTE_VS3    ? vsPalette[2] :
					type == PALETTE_VS4    ? vsPalette[3] :
                                             pc10Palette
				);

				NST_ASSERT( from );

				for (uint i=0; i < 8; ++i)
				{
					if (i && type == PALETTE_CUSTOM && custom->emphasis)
						from = custom->emphasis[i-1];

					for (uint j=0; j < 64; ++j)
					{
						double rgb[3] =
						{
							from[j][0] / 255.0,
							from[j][1] / 255.0,
							from[j][2] / 255.0
						};

						double h,s,v;

						ToHSV( rgb[0], rgb[1], rgb[2], h, s, v );

						s *= saturation;
						v  = v * contrast + brightness;
						h += hue;

						if (h >= 360.0)
						{
							h -= 360.0;
						}
						else if (h < 0.0)
						{
							h += 360.0;
						}

						ToRGB( h, s, v, rgb[0], rgb[1], rgb[2] );

						if (i)
						{
							if (type == PALETTE_CUSTOM)
							{
								if (!custom->emphasis && (j & 0xF) <= 0xD)
								{
									double yiq[3] =
									{
										0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2],
										0.596 * rgb[0] - 0.275 * rgb[1] - 0.321 * rgb[2],
										0.212 * rgb[0] - 0.523 * rgb[1] + 0.311 * rgb[2]
									};

									if (i == 7)
									{
										yiq[0] = yiq[0] * (Constants::attenMul * 1.13) - (Constants::attenSub * 1.13);
									}
									else
									{
										double s = Constants::levels[(j & 0xF) != 0xD][j >> 4 & 0x3] * (0.5 - Constants::attenMul * 0.5) + Constants::attenSub * 0.5;

										yiq[0] -= s * 0.5;

										if (i >= 3 && i != 4)
										{
											s *= 0.6;
											yiq[0] -= s;
										}

										const double angle = NST_PI / 12 * (Constants::tints[i] * 2 - 7);

										yiq[1] += std::sin( angle ) * s;
										yiq[2] += std::cos( angle ) * s;
									}

									rgb[0] = yiq[0] + 0.956 * yiq[1] + 0.621 * yiq[2];
									rgb[1] = yiq[0] - 0.272 * yiq[1] - 0.647 * yiq[2];
									rgb[2] = yiq[0] - 1.105 * yiq[1] + 1.702 * yiq[2];
								}
							}
							else switch (i)
							{
								case 1: rgb[0] = 0xFF;                   break;
								case 2: rgb[1] = 0xFF;                   break;
								case 3: rgb[1] = rgb[0] = 0xFF;          break;
								case 4: rgb[2] = 0xFF;                   break;
								case 5: rgb[2] = rgb[0] = 0xFF;          break;
								case 6: rgb[2] = rgb[1] = 0xFF;          break;
								case 7: rgb[2] = rgb[1] = rgb[0] = 0xFF; break;
							}
						}

						ToPAL( rgb, palette[(i * 64) + j] );
					}
				}
			}

			void Renderer::Palette::Generate(const int b,const int s,const int c,int hue)
			{
				NST_ASSERT( type == PALETTE_YUV );

				const double brightness = b / 200.0;
				const double saturation = (s + 100) / 100.0;
				const double contrast = (c + 100) / 100.0;
				hue += 33;

				const double matrix[6] =
				{
					std::sin( (int(decoder.axes[0].angle) - hue) * NST_DEG ) * decoder.axes[0].gain * 2,
					std::cos( (int(decoder.axes[0].angle) - hue) * NST_DEG ) * decoder.axes[0].gain * 2,
					std::sin( (int(decoder.axes[1].angle) - hue) * NST_DEG ) * decoder.axes[1].gain * 2,
					std::cos( (int(decoder.axes[1].angle) - hue) * NST_DEG ) * decoder.axes[1].gain * 2,
					std::sin( (int(decoder.axes[2].angle) - hue) * NST_DEG ) * decoder.axes[2].gain * 2,
					std::cos( (int(decoder.axes[2].angle) - hue) * NST_DEG ) * decoder.axes[2].gain * 2
				};

				for (uint n=0; n < PALETTE; ++n)
				{
					double level[2] =
					{
						Constants::levels[0][n >> 4 & 3],
						Constants::levels[1][n >> 4 & 3]
					};

					const int color = n & 0x0F;

					if (color == 0x00)
					{
						level[0] = level[1];
					}
					else if (color == 0x0D)
					{
						level[1] = level[0];
					}
					else if (color > 0x0D)
					{
						level[1] = level[0] = 0.0;
					}

					double y = (level[1] + level[0]) * 0.5;
					double s = (level[1] - level[0]) * 0.5;
					double h = NST_PI / 6 * (color - 3);

					double i = std::sin( h ) * s;
					double q = std::cos( h ) * s;

					const uint tint = n >> 6 & 7;

					if (tint && color <= 0x0D)
					{
						if (tint == 7)
						{
							y = y * (Constants::attenMul * 1.13) - (Constants::attenSub * 1.13);
						}
						else
						{
							const double angle = NST_PI / 12 * (Constants::tints[tint] * 2 - 7);

							s = level[1] * (0.5 - Constants::attenMul * 0.5) + Constants::attenSub * 0.5;
							y -= s * 0.5;

							if (tint >= 3 && tint != 4)
							{
								s *= 0.6;
								y -= s;
							}

							i += std::sin( angle ) * s;
							q += std::cos( angle ) * s;
						}
					}

					if (decoder.boostYellow)
					{
						const double yellowness = i - q;

						if (yellowness > 0.0)
						{
							i = i + yellowness * ((n >> 4 & 3) / 4.0);
							q = q - yellowness * ((n >> 4 & 3) / 4.0);
						}
					}

					i *= saturation;
					q *= saturation;
					y = y * contrast + brightness;

					const double rgb[3] =
					{
						y + matrix[0] * i + matrix[1] * q,
						y + matrix[2] * i + matrix[3] * q,
						y + matrix[4] * i + matrix[5] * q
					};

					ToPAL( rgb, palette[n] );
				}
			}

			void Renderer::Palette::Update(int brightness,int saturation,int contrast,int hue)
			{
				FpuPrecision precision;
				(*this.*(type == PALETTE_YUV ? &Palette::Generate : &Palette::Build))( brightness, saturation, contrast, hue );
			}

			inline const Renderer::PaletteEntries& Renderer::Palette::Get() const
			{
				return palette;
			}

			Renderer::Filter::Format::Format(const RenderState::Bits::Mask& m)
			{
				const dword mask[3] = {m.r,m.g,m.b};

				for (uint i=0; i < 3; ++i)
				{
					left[i] = 0;

					if (mask[i])
					{
						while (!(mask[i] & (0x1UL << left[i])))
							++left[i];
					}

					for
					(
						right[i] = 0;
						right[i] < 8 && (mask[i] & (0x1UL << (left[i] + right[i])));
						right[i] += 1
					);

					right[i] = 8 - right[i];
				}
			}

			Renderer::Filter::Filter(const RenderState& state)
			: bpp(state.bits.count), format(state.bits.mask) {}

			void Renderer::Filter::Transform(const u8 (&src)[PALETTE][3],u32 (&dst)[PALETTE]) const
			{
				if (bpp >= 16)
				{
					for (uint i=0; i < PALETTE; ++i)
					{
						dst[i] =
						(
							((src[i][0] >> format.right[0]) << format.left[0]) |
							((src[i][1] >> format.right[1]) << format.left[1]) |
							((src[i][2] >> format.right[2]) << format.left[2])
						);
					}
				}
			}

			Renderer::State::State()
			:
			update       (UPDATE_PALETTE),
			brightness   (0),
			saturation   (0),
			hue          (0),
			contrast     (0),
			sharpness    (0),
			resolution   (0),
			bleed        (0),
			artifacts    (0),
			fringing     (0),
			scanlines    (0),
			fieldMerging (0)
			{}

			Renderer::Renderer()
			: filter(NULL) {}

			Renderer::~Renderer()
			{
				delete filter;
			}

			Result Renderer::SetState(const RenderState& renderState)
			{
				if (filter)
				{
					if
					(
						state.filter == renderState.filter &&
						state.width == renderState.width &&
						state.height == renderState.height &&
						filter->bpp == renderState.bits.count &&
						state.mask.r == renderState.bits.mask.r &&
						state.mask.g == renderState.bits.mask.g &&
						state.mask.b == renderState.bits.mask.b &&
						state.scanlines == renderState.scanlines &&
						(filter->bpp != 8 || static_cast<const FilterNone*>(filter)->paletteOffset == renderState.paletteOffset)
					)
						return RESULT_NOP;

					delete filter;
					filter = NULL;
				}

				switch (renderState.filter)
				{
					case RenderState::FILTER_NONE:

						if (renderState.scanlines)
						{
							if (FilterScanlines::Check( renderState ))
								filter = new (std::nothrow) FilterScanlines( renderState );
						}
						else
						{
							if (FilterNone::Check( renderState ))
								filter = new (std::nothrow) FilterNone( renderState );
						}
						break;

				#ifndef NST_NO_2XSAI

					case RenderState::FILTER_2XSAI:
					case RenderState::FILTER_SUPER_2XSAI:
					case RenderState::FILTER_SUPER_EAGLE:

						if (Filter2xSaI::Check( renderState ))
							filter = new (std::nothrow) Filter2xSaI( renderState );

						break;

				#endif
				#ifndef NST_NO_SCALE2X

					case RenderState::FILTER_SCALE2X:
					case RenderState::FILTER_SCALE3X:

						if (FilterScaleX::Check( renderState ))
							filter = new (std::nothrow) FilterScaleX( renderState );

						break;
				#endif
				#ifndef NST_NO_HQ2X

					case RenderState::FILTER_HQ2X:
					case RenderState::FILTER_HQ3X:
					case RenderState::FILTER_HQ4X:

						if (FilterHqX::Check( renderState ))
							filter = new (std::nothrow) FilterHqX( renderState );

						break;

				#endif

					case RenderState::FILTER_NTSC:
					{
						const FilterNtscState ntscState
						(
							renderState,
							GetPalette(),
							state.sharpness,
							state.resolution,
							state.bleed,
							state.artifacts,
							state.fringing,
							state.fieldMerging
						);

						if (FilterNtsc<32>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<32>( ntscState );
						}
						else if (FilterNtsc<16>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<16>( ntscState );
						}
						else if (FilterNtsc<15>::Check( renderState ))
						{
							filter = new (std::nothrow) FilterNtsc<15>( ntscState );
						}
						break;
					}
				}

				if (filter)
				{
					state.scanlines = renderState.scanlines;
					state.filter = renderState.filter;
					state.width = renderState.width;
					state.height = renderState.height;
					state.mask = renderState.bits.mask;

					if (state.filter == RenderState::FILTER_NTSC)
						state.update = 0;
					else
						state.update |= State::UPDATE_FILTER;

					return RESULT_OK;
				}
				else
				{
					return RESULT_ERR_UNSUPPORTED;
				}
			}

			Result Renderer::GetState(RenderState& output) const
			{
				if (filter)
				{
					output.filter = state.filter;
					output.width = state.width;
					output.height = state.height;
					output.scanlines = state.scanlines;
					output.bits.count = filter->bpp;

					if (output.bits.count == 8)
					{
						output.bits.mask.b = output.bits.mask.g = output.bits.mask.r = 0;
						output.paletteOffset = static_cast<const FilterNone*>(filter)->paletteOffset;
					}
					else
					{
						output.bits.mask = state.mask;
						output.paletteOffset = 0;
					}

					return RESULT_OK;
				}

				return RESULT_ERR_NOT_READY;
			}

			void Renderer::EnableFieldMerging(bool fieldMerging)
			{
				const bool old = state.fieldMerging;
				state.fieldMerging &= State::FIELD_MERGING_PAL;

				if (fieldMerging)
					state.fieldMerging |= State::FIELD_MERGING_USER;

				if (bool(state.fieldMerging) != old)
					state.update |= State::UPDATE_NTSC;
			}

			void Renderer::SetMode(Mode mode)
			{
				const bool old = state.fieldMerging;
				state.fieldMerging &= State::FIELD_MERGING_USER;

				if (mode == MODE_PAL)
					state.fieldMerging |= State::FIELD_MERGING_PAL;

				if (bool(state.fieldMerging) != old)
					state.update |= State::UPDATE_NTSC;
			}

			Result Renderer::SetHue(int hue)
			{
				if (hue < -45 || hue > 45)
					return RESULT_ERR_INVALID_PARAM;

				if (state.hue == hue)
					return RESULT_NOP;

				state.hue = hue;
				state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;

				return RESULT_OK;
			}

			Result Renderer::SetLevel(i8& type,int value,uint update)
			{
				if (value < -100 || value > 100)
					return RESULT_ERR_INVALID_PARAM;

				if (type == value)
					return RESULT_NOP;

				type = value;
				state.update |= update;

				return RESULT_OK;
			}

			Result Renderer::SetDecoder(const Decoder& decoder)
			{
				const Result result = palette.SetDecoder( decoder );

				if (result == RESULT_OK && palette.GetType() == PALETTE_YUV)
					state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;

				return result;
			}

			Result Renderer::SetPaletteType(PaletteType type)
			{
				const Result result = palette.SetType( type );

				if (result == RESULT_OK)
					state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;

				return result;
			}

			Result Renderer::LoadCustomPalette(const u8 (*colors)[3],const bool emphasis)
			{
				const Result result = palette.LoadCustom( colors, emphasis );

				if (result == RESULT_OK && palette.GetType() == PALETTE_CUSTOM)
					state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;

				return result;
			}

			void Renderer::ResetCustomPalette()
			{
				if (palette.ResetCustom() && palette.GetType() == PALETTE_CUSTOM)
					state.update |= State::UPDATE_PALETTE|State::UPDATE_FILTER;
			}

			const Renderer::PaletteEntries& Renderer::GetPalette()
			{
				if (state.update & State::UPDATE_PALETTE)
				{
					state.update &= ~u8(State::UPDATE_PALETTE);
					palette.Update( state.brightness, state.saturation, state.contrast, state.hue );
				}

				return palette.Get();
			}

			void Renderer::UpdateFilter(Input& input)
			{
				NST_VERIFY( state.update );

				if (state.filter == RenderState::FILTER_NTSC)
				{
					RenderState renderState;
					GetState( renderState );

					delete filter;
					filter = NULL;

					SetState( renderState );
				}
				else if (state.update & State::UPDATE_FILTER)
				{
					const PaletteEntries& entries = GetPalette();
					filter->Transform( entries, input.palette );
					Api::Video::Palette::updateCallback( entries );
				}

				state.update = 0;
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Renderer::Blit(Output& output,Input& input,uint burstPhase)
			{
				if (filter)
				{
					if (state.update)
						UpdateFilter( input );

					if (Output::lockCallback( output ))
					{
						NST_ASSERT( output.pixels && output.pitch );

						if (ulong(std::labs( output.pitch )) >= filter->bpp * (WIDTH / 8U))
							filter->Blit( input, output, burstPhase );

						Output::unlockCallback( output );
					}
				}
			}
		}
	}
}
