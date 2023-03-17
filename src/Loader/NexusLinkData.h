#ifndef NEXUSLINKDATA_H
#define NEXUSLINKDATA_H

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
};

#endif