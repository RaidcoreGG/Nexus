///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  MblReader.cpp
/// Description  :  Provides Mumble API events and extended data.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "GW2/Mumble/MblReader.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

using namespace Mumble;

#include "Util/CmdLine.h"
#include "Context.h"
#include "MblExtensions.h"

CMumbleReader::CMumbleReader(CDataLinkApi* aDataLink, CEventApi* aEventApi, CLogApi* aLogger)
{
	assert(aDataLink);
	assert(aEventApi);
	assert(aLogger);

	this->DataLinkApi = aDataLink;
	this->EventApi = aEventApi;
	this->Logger = aLogger;

	this->Name = CmdLine::GetArgumentValue("-mumble");

	if (this->Name.empty())
	{
		this->Name = "MumbleLink";
	}

	/* share the linked mem regardless whether it's disabled, for dependant addons */
	this->MumbleLink = (Mumble::Data*)this->DataLinkApi->ShareResource(DL_MUMBLE_LINK, sizeof(Mumble::Data), this->Name.c_str(), true);
	this->MumbleIdentity = (Mumble::Identity*)this->DataLinkApi->ShareResource(DL_MUMBLE_LINK_IDENTITY, sizeof(Mumble::Identity), "", false);
	this->NexusLink = (NexusLinkData_t*)this->DataLinkApi->ShareResource(DL_NEXUS_LINK, sizeof(NexusLinkData_t), "", true);

	if (this->Name != "0")
	{
		this->IsRunning = true;
		this->Thread = std::thread(&CMumbleReader::Advance, this);
	}
}

CMumbleReader::~CMumbleReader()
{
	this->IsRunning = false;

	if (this->Thread.joinable())
	{
		this->Thread.join();
	}
}

std::string CMumbleReader::GetName()
{
	return this->Name;
}

bool CMumbleReader::IsDisabled() const
{
	return !this->IsRunning;
}

void CMumbleReader::Advance()
{
	while (this->IsRunning)
	{
		CContext*        ctx      = CContext::GetContext();
		RenderContext_t* renderer = ctx->GetRendererCtx();

		this->Flip = !this->Flip; // every other tick so it gets refreshed every 100ms // FIXME: this is ugly lol
		if (this->Flip)
		{
			this->NexusLink->IsGameplay = this->PreviousTick != this->MumbleLink->UITick ||
				(this->PreviousFrameCounter == renderer->Metrics.FrameCount && this->NexusLink->IsGameplay);
			this->NexusLink->IsMoving = this->PreviousAvatarPosition != this->MumbleLink->AvatarPosition;
			this->NexusLink->IsCameraMoving = this->PreviousCameraFront != this->MumbleLink->CameraFront;

			this->PreviousFrameCounter = renderer->Metrics.FrameCount;
			this->PreviousTick = this->MumbleLink->UITick;
			this->PreviousAvatarPosition = this->MumbleLink->AvatarPosition;
			this->PreviousCameraFront = this->MumbleLink->CameraFront;
		}

		if (this->MumbleLink->Identity[0])
		{
			/* cache identity */
			this->PreviousIdentity = *this->MumbleIdentity;

			try
			{
				/* parse and assign current identity */
				json j = json::parse(MumbleLink->Identity);
				strcpy(this->MumbleIdentity->Name, j["name"].get<std::string>().c_str());
				j["profession"].get_to(this->MumbleIdentity->Profession);
				j["spec"].get_to(this->MumbleIdentity->Specialization);
				j["race"].get_to(this->MumbleIdentity->Race);
				j["map_id"].get_to(this->MumbleIdentity->MapID);
				j["world_id"].get_to(this->MumbleIdentity->WorldID);
				j["team_color_id"].get_to(this->MumbleIdentity->TeamColorID);
				j["commander"].get_to(this->MumbleIdentity->IsCommander);
				j["fov"].get_to(this->MumbleIdentity->FOV);
				j["uisz"].get_to(this->MumbleIdentity->UISize);
			}
			catch (json::parse_error& ex)
			{
				this->Logger->Trace(CH_MUMBLE_READER, "MumbleLink could not be parsed. Parse Error: %s", ex.what());
			}
			catch (json::type_error& ex)
			{
				this->Logger->Trace(CH_MUMBLE_READER, "MumbleLink could not be parsed. Type Error: %s", ex.what());
			}
			catch (...)
			{
				this->Logger->Trace(CH_MUMBLE_READER, "MumbleLink could not be parsed. Unknown Error.");
			}

			/* notify (also notifies the GUI to update its scaling factor) */
			if (*this->MumbleIdentity != this->PreviousIdentity)
			{
				this->EventApi->Raise(EV_MUMBLE_IDENTITY_UPDATED, this->MumbleIdentity);
			}
		}

		for (size_t i = 0; i < 50; i++)
		{
			if (!this->IsRunning) { return; } // abort if shutdown during sleep

			Sleep(1);
		}
	}
}
