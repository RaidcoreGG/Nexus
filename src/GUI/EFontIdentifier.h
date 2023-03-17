#ifndef GUI_FONT_H
#define GUI_FONT_H

namespace GUI
{
	typedef enum class EFont
	{
		None = 0,

		User,

		Menomonia_Small,
		MenomoniaBig_Small,
		Trebuchet_Small,

		Menomonia_Normal,
		MenomoniaBig_Normal,
		Trebuchet_Normal,

		Menomonia_Large,
		MenomoniaBig_Large,
		Trebuchet_Large,

		Menomonia_Larger,
		MenomoniaBig_Larger,
		Trebuchet_Larger
	} EFontIdentifier;
}

#endif