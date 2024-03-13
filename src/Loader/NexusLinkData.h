#ifndef NEXUSLINKDATA_H
#define NEXUSLINKDATA_H

/* A structure containing various variables for other addons to use. */
struct NexusLinkData
{
	unsigned	Width;
	unsigned	Height;
	float		Scaling;

	bool		IsMoving;
	bool		IsCameraMoving;
	bool		IsGameplay;

	ImFont*		Font;
	ImFont*		FontBig;
	ImFont*		FontUI;

	signed int	QuickAccessIconsCount;
	signed int	QuickAccessMode;
	bool		QuickAccessIsVertical;
};

#endif