/* This is a heavily modified version of the file used in RetroArch */

/*  RetroArch - A frontend for libretro.
*  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
*  Copyright (C) 2010-2014 - Daniel De Matteis
*
*  RetroArch is free software: you can redistribute it and/or modify it under the terms
*  of the GNU General Public License as published by the Free Software Found-
*  ation, either version 3 of the License, or (at your option) any later version.
*
*  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
*  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE.  See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with RetroArch.
*  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../pch.h"

#define supertwoxsai_interpolate_xrgb8888(A, B) ((((A) & 0xFEFEFEFE) >> 1) + (((B) & 0xFEFEFEFE) >> 1) + ((A) & (B) & 0x01010101))

#define supertwoxsai_interpolate2_xrgb8888(A, B, C, D) ((((A) & 0xFCFCFCFC) >> 2) + (((B) & 0xFCFCFCFC) >> 2) + (((C) & 0xFCFCFCFC) >> 2) + (((D) & 0xFCFCFCFC) >> 2) + (((((A) & 0x03030303) + ((B) & 0x03030303) + ((C) & 0x03030303) + ((D) & 0x03030303)) >> 2) & 0x03030303))

#define supertwoxsai_result(A, B, C, D) (((A) != (C) || (A) != (D)) - ((B) != (C) || (B) != (D)))

#ifndef supertwoxsai_declare_variables
#define supertwoxsai_declare_variables(typename_t, in) \
         typename_t product1a, product1b, product2a, product2b; \
         const typename_t colorB0 = *(in - prevline - prevcolumn); \
         const typename_t colorB1 = *(in - prevline + 0); \
         const typename_t colorB2 = *(in - prevline + nextcolumn); \
         const typename_t colorB3 = *(in - prevline + nextcolumn2); \
         const typename_t color4  = *(in - prevcolumn); \
         const typename_t color5  = *(in + 0); \
         const typename_t color6  = *(in + nextcolumn); \
         const typename_t colorS2 = *(in + nextcolumn2); \
         const typename_t color1  = *(in + nextline - prevcolumn); \
         const typename_t color2  = *(in + nextline + 0); \
         const typename_t color3  = *(in + nextline + nextcolumn); \
         const typename_t colorS1 = *(in + nextline + nextcolumn2); \
         const typename_t colorA0 = *(in + nextline2 - prevcolumn); \
         const typename_t colorA1 = *(in + nextline2 + 0); \
         const typename_t colorA2 = *(in + nextline2 + nextcolumn); \
         const typename_t colorA3 = *(in + nextline2 + nextcolumn2)
#endif

#ifndef supertwoxsai_function
#define supertwoxsai_function(result_cb, interpolate_cb, interpolate2_cb) \
         if (color2 == color6 && color5 != color3) \
            product2b = product1b = color2; \
         else if (color5 == color3 && color2 != color6) \
            product2b = product1b = color5; \
         else if (color5 == color3 && color2 == color6) \
         { \
            int r = 0; \
            r += result_cb(color6, color5, color1, colorA1); \
            r += result_cb(color6, color5, color4, colorB1); \
            r += result_cb(color6, color5, colorA2, colorS1); \
            r += result_cb(color6, color5, colorB2, colorS2); \
            if (r > 0) \
               product2b = product1b = color6; \
            else if (r < 0) \
               product2b = product1b = color5; \
            else \
               product2b = product1b = interpolate_cb(color5, color6); \
         } \
         else \
         { \
            if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0) \
               product2b = interpolate2_cb(color3, color3, color3, color2); \
            else if ((color5 == color2 && color2 == colorA2) & (colorA1 != color3 && color2 != colorA3)) \
               product2b = interpolate2_cb(color2, color2, color2, color3); \
            else \
               product2b = interpolate_cb(color2, color3); \
            if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0) \
               product1b = interpolate2_cb(color6, color6, color6, color5); \
            else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3) \
               product1b = interpolate2_cb(color6, color5, color5, color5); \
            else \
               product1b = interpolate_cb(color5, color6); \
         } \
         if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2) \
         { \
            product2a = interpolate_cb(color2, color5); \
         } \
         else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0) \
         { \
            product2a = interpolate_cb(color2, color5); \
         } \
         else \
            product2a = color2; \
         if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2) \
         { \
            product1a = interpolate_cb(color2, color5); \
         } \
         else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0) \
         { \
            product1a = interpolate_cb(color2, color5); \
         } \
         else \
            product1a = color5; \
         out[0] = product1a; \
         out[1] = product1b; \
         out[dst_stride] = product2a; \
         out[dst_stride + 1] = product2b; \
         ++in; \
         out += 2
#endif

void supertwoxsai_generic_xrgb8888(unsigned width, unsigned height, uint32_t *src, unsigned src_stride, uint32_t *dst, unsigned dst_stride)
{
	unsigned finish;
	int y = 0;
	int x = 0;
	for(; height; height--) {
		uint32_t *in = (uint32_t*)src;
		uint32_t *out = (uint32_t*)dst;

		int prevline = (y > 0 ? src_stride : 0);
		int nextline = (height > 1 ? src_stride : 0);
		int nextline2 = (height > 2 ? src_stride * 2 : nextline);

		for(finish = width; finish; finish -= 1) {
			int prevcolumn = (x > 0 ? 1 : 0);
			int nextcolumn = (finish > 1 ? 1 : 0);
			int nextcolumn2 = (finish > 2 ? 2 : nextcolumn);
			supertwoxsai_declare_variables(uint32_t, in);

			//---------------------------    B1 B2
			//                             4  5  6 S2
			//                             1  2  3 S1
			//                               A1 A2
			//--------------------------------------

			supertwoxsai_function(supertwoxsai_result, supertwoxsai_interpolate_xrgb8888, supertwoxsai_interpolate2_xrgb8888);
			x++;
		}

		src += src_stride;
		dst += 2 * dst_stride;
		y++;
		x = 0;
	}
}