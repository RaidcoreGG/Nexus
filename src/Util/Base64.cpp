///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  Base64.cpp
/// Description  :  Contains a variety of utility for Base64.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "Base64.h"

namespace Base64
{
	static const unsigned char Base64Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::string Encode(const unsigned char* aData, size_t aSize)
	{
		unsigned char* out;
		unsigned char* pos;
		const unsigned char* end;
		const unsigned char* in;

		size_t outLength;

		outLength = 4 * ((aSize + 2) / 3);

		if (outLength < aSize)
		{
			return "";
		}

		std::string outStr;
		outStr.resize(outLength);
		out = (unsigned char*)&outStr[0];

		end = aData + aSize;
		in = aData;
		pos = out;
		while (end - in >= 3)
		{
			*pos++ = Base64Table[in[0] >> 2];
			*pos++ = Base64Table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
			*pos++ = Base64Table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
			*pos++ = Base64Table[in[2] & 0x3f];
			in += 3;
		}

		if (end - in)
		{
			*pos++ = Base64Table[in[0] >> 2];

			if (end - in == 1)
			{
				*pos++ = Base64Table[(in[0] & 0x03) << 4];
				*pos++ = '=';
			}
			else
			{
				*pos++ = Base64Table[((in[0] & 0x03) << 4) |
					(in[1] >> 4)];
				*pos++ = Base64Table[(in[1] & 0x0f) << 2];
			}

			*pos++ = '=';
		}

		return outStr;
	}

	static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
	7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
	0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

	std::string Decode(std::string aData, size_t aSize)
	{
		unsigned char* p = (unsigned char*)aData.c_str();
		int pad = aSize > 0 && (aSize % 4 || p[aSize - 1] == '=');
		const size_t L = ((aSize + 3) / 4 - pad) * 4;
		std::string str(L / 4 * 3 + pad, '\0');

		for (size_t i = 0, j = 0; i < L; i += 4)
		{
			int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
			str[j++] = n >> 16;
			str[j++] = n >> 8 & 0xFF;
			str[j++] = n & 0xFF;
		}

		if (pad)
		{
			int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
			str[str.size() - 1] = n >> 16;

			if (aSize > L + 2 && p[L + 2] != '=')
			{
				n |= B64index[p[L + 2]] << 6;
				str.push_back(n >> 8 & 0xFF);
			}
		}

		return str;
	}
}
