///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  AddonDefinition.cpp
/// Description  :  Contains the definition for aaddons.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "AddonDefinition.h"

AddonDefinition::~AddonDefinition()
{
	if (this->Name)        { free((char*)this->Name);        }
	if (this->Author)      { free((char*)this->Author);      }
	if (this->Description) { free((char*)this->Description); }
	if (this->UpdateLink)  { free((char*)this->UpdateLink);  }
}

bool AddonDefinition::IsValid()
{
	if (this->Signature != 0 &&
		this->Name &&
		this->Author &&
		this->Description &&
		this->Load)
	{
		return true;
	}

	return false;
}

AddonDefinition& AddonDefinition::operator=(const AddonDefinition& rhs)
{
	if (this->Name)        { free((char*)this->Name);        }
	if (this->Author)      { free((char*)this->Author);      }
	if (this->Description) { free((char*)this->Description); }
	if (this->UpdateLink)  { free((char*)this->UpdateLink);  }

	/* copy all */
	memcpy_s(this, sizeof(AddonDefinition), &rhs, sizeof(AddonDefinition));

	/* deep copy strings */
	this->Name        = rhs.Name        ? _strdup(rhs.Name)        : nullptr;
	this->Author      = rhs.Author      ? _strdup(rhs.Author)      : nullptr;
	this->Description = rhs.Description ? _strdup(rhs.Description) : nullptr;
	this->UpdateLink  = rhs.UpdateLink  ? _strdup(rhs.UpdateLink)  : nullptr;

	return *this;
}
