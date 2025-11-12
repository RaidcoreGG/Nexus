///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - Licensed under the MIT license.
///
/// Name         :  ImAnimate.h
/// Description  :  Immediate mode animations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#ifndef IMANIMATE_H
#define IMANIMATE_H

#include <imgui.h>
#include <map>

///----------------------------------------------------------------------------------------------------
/// ImAnimate Namespace
///----------------------------------------------------------------------------------------------------
namespace ImAnimate
{
	///----------------------------------------------------------------------------------------------------
	/// ECurve Enumeration
	///----------------------------------------------------------------------------------------------------
	enum class ECurve
	{
		Linear,

		InSine,
		OutSine,
		InOutSine,

		InQuad,
		OutQuad,
		InOutQuad,

		InCubic,
		OutCubic,
		InOutCubic,

		InQuart,
		OutQuart,
		InOutQuart,

		InQuint,
		OutQuint,
		InOutQuint,

		InExpo,
		OutExpo,
		InOutExpo,

		InCirc,
		OutCirc,
		InOutCirc
	};
}

///----------------------------------------------------------------------------------------------------
/// ImGui Namespace
///----------------------------------------------------------------------------------------------------
namespace ImGui
{
	///----------------------------------------------------------------------------------------------------
	/// Animate:
	///     Animates a value.
	///     - aStartValue: Is the initial value.
	///     - aEndValue: Is the final value.
	///     - aDurationMs: Is the duration in milliseconds.
	///     - aValue: Is the out/current value.
	///     - aCurve: Is of type ImAnimate::ECurve and is the easing curve used for this animation.
	///----------------------------------------------------------------------------------------------------
	void Animate(float aStartValue, float aEndValue, float aDurationMs, float* aValue, ImAnimate::ECurve aCurve = ImAnimate::ECurve::Linear);
}

#endif
