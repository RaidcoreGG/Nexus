///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - Licensed under the MIT license.
///
/// Name         :  ImAnimate.cpp
/// Description  :  Immediate mode animations.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "ImAnimate.h"

#include <cmath>
#include <imgui_internal.h>

#define PI 3.14159f

namespace ImAnimate
{
	float CurveFunc_Linear(float aProgress)
	{
		return aProgress;
	}

	float CurveFunc_InSine(float aProgress)
	{
		return 1 - cos((aProgress * PI) / 2);
	}
	float CurveFunc_OutSine(float aProgress)
	{
		return sin((aProgress * PI) / 2);
	}
	float CurveFunc_InOutSine(float aProgress)
	{
		return -(cos(PI * aProgress) - 1) / 2;
	}

	float CurveFunc_InQuad(float aProgress)
	{
		return aProgress * aProgress;
	}
	float CurveFunc_OutQuad(float aProgress)
	{
		return 1 - (1 - aProgress) * (1 - aProgress);
	}
	float CurveFunc_InOutQuad(float aProgress)
	{
		return aProgress < 0.5f ? 2 * aProgress * aProgress : 1 - powf(-2 * aProgress + 2, 2) / 2.0f;
	}

	float CurveFunc_InCubic(float aProgress)
	{
		return aProgress * aProgress * aProgress;
	}
	float CurveFunc_OutCubic(float aProgress)
	{
		return 1 - powf(1 - aProgress, 3);
	}
	float CurveFunc_InOutCubic(float aProgress)
	{
		return aProgress < 0.5f ? 4 * aProgress * aProgress * aProgress : 1 - powf(-2 * aProgress + 2, 3) / 2.0f;
	}

	float CurveFunc_InQuart(float aProgress)
	{
		return aProgress * aProgress * aProgress * aProgress;
	}
	float CurveFunc_OutQuart(float aProgress)
	{
		return 1 - powf(1 - aProgress, 4);
	}
	float CurveFunc_InOutQuart(float aProgress)
	{
		return aProgress < 0.5f ? 8 * aProgress * aProgress * aProgress * aProgress : 1 - powf(-2 * aProgress + 2, 4) / 2.0f;
	}

	float CurveFunc_InQuint(float aProgress)
	{
		return aProgress * aProgress * aProgress * aProgress * aProgress;
	}
	float CurveFunc_OutQuint(float aProgress)
	{
		return 1 - powf(1 - aProgress, 5);
	}
	float CurveFunc_InOutQuint(float aProgress)
	{
		return aProgress < 0.5f ? 16 * aProgress * aProgress * aProgress * aProgress * aProgress : 1 - powf(-2 * aProgress + 2, 5) / 2.0f;
	}

	float CurveFunc_InExpo(float aProgress)
	{
		return aProgress == 0 ? 0 : powf(2, 10 * aProgress - 10);
	}
	float CurveFunc_OutExpo(float aProgress)
	{
		return aProgress == 1 ? 1 : 1 - powf(2, -10 * aProgress);
	}
	float CurveFunc_InOutExpo(float aProgress)
	{
		return aProgress == 0 ? 0 : aProgress == 1 ? 1 : aProgress < 0.5 ? powf(2, 20 * aProgress - 10) / 2.0f : (2 - powf(2, -20 * aProgress + 10)) / 2.0f;
	}

	float CurveFunc_InCirc(float aProgress)
	{
		return 1 - sqrt(1 - powf(aProgress, 2));
	}
	float CurveFunc_OutCirc(float aProgress)
	{
		return sqrt(1 - powf(aProgress - 1, 2));
	}
	float CurveFunc_InOutCirc(float aProgress)
	{
		return aProgress < 0.5f ? (1 - sqrt(1 - powf(2 * aProgress, 2))) / 2.0f : (sqrt(1 - powf(-2 * aProgress + 2, 2)) + 1) / 2.0f;
	}

	typedef float(*PFN_CURVEFUNC)(float);

	///----------------------------------------------------------------------------------------------------
	/// GetCurveFunc:
	///     Internal function to return the function correlating to the curve.
	///----------------------------------------------------------------------------------------------------
	PFN_CURVEFUNC GetCurveFunc(ECurve aCurve)
	{
		switch (aCurve)
		{
			default:
			case ImAnimate::ECurve::Linear:     return CurveFunc_Linear;
			case ImAnimate::ECurve::InSine:     return CurveFunc_InSine;
			case ImAnimate::ECurve::OutSine:    return CurveFunc_OutSine;
			case ImAnimate::ECurve::InOutSine:  return CurveFunc_InOutSine;
			case ImAnimate::ECurve::InQuad:     return CurveFunc_InQuad;
			case ImAnimate::ECurve::OutQuad:    return CurveFunc_OutQuad;
			case ImAnimate::ECurve::InOutQuad:  return CurveFunc_InOutQuad;
			case ImAnimate::ECurve::InCubic:    return CurveFunc_InCubic;
			case ImAnimate::ECurve::OutCubic:   return CurveFunc_OutCubic;
			case ImAnimate::ECurve::InOutCubic: return CurveFunc_InOutCubic;
			case ImAnimate::ECurve::InQuart:    return CurveFunc_InQuart;
			case ImAnimate::ECurve::OutQuart:   return CurveFunc_OutQuart;
			case ImAnimate::ECurve::InOutQuart: return CurveFunc_InOutQuart;
			case ImAnimate::ECurve::InQuint:    return CurveFunc_InQuint;
			case ImAnimate::ECurve::OutQuint:   return CurveFunc_OutQuint;
			case ImAnimate::ECurve::InOutQuint: return CurveFunc_InOutQuint;
			case ImAnimate::ECurve::InExpo:     return CurveFunc_InExpo;
			case ImAnimate::ECurve::OutExpo:    return CurveFunc_OutExpo;
			case ImAnimate::ECurve::InOutExpo:  return CurveFunc_InOutExpo;
			case ImAnimate::ECurve::InCirc:     return CurveFunc_InCirc;
			case ImAnimate::ECurve::OutCirc:    return CurveFunc_OutCirc;
			case ImAnimate::ECurve::InOutCirc:  return CurveFunc_InOutCirc;
		}
	}

	struct Animation
	{
		double StartTime;
		double ChangeTime;
		float StartValue;
		float EndValue;
	};

	static std::map<void*, Animation> Animations;

	///----------------------------------------------------------------------------------------------------
	/// Clamp:
	///     Internal function to ensure a value is within range.
	///----------------------------------------------------------------------------------------------------
	float Clamp(const float& aValue, const float& aLimit, const float& aLimit2)
	{
		if (aLimit > aLimit2)
		{
			if (aValue > aLimit) { return aLimit; }
			else if (aValue < aLimit2) { return aLimit2; }
		}
		else if (aLimit < aLimit2)
		{
			if (aValue < aLimit) { return aLimit; }
			else if (aValue > aLimit2) { return aLimit2; }
		}
		else /* if (aLimit == aLimit2) */
		{
			return aLimit;
		}

		/* if this is reached, then the value is within range */
		return aValue;
	}
}

namespace ImGui
{
	void Animate(float aStartValue, float aEndValue, float aDurationMs, float* aValue, ImAnimate::ECurve aCurve)
	{
		auto& anim = ImAnimate::Animations[aValue];

		if (*aValue == aEndValue)
		{
			anim.StartTime = 0;
			return;
		}

		float clampedVal = ImAnimate::Clamp(*aValue, aStartValue, aEndValue);

		if (clampedVal != *aValue)
		{
			anim.StartTime = 0;
			*aValue = clampedVal;
			return;
		}

		float range = abs(aEndValue - aStartValue);

		float currProgressVal = abs(*aValue - aStartValue) / range;

		ImGuiContext* ctx = ImGui::GetCurrentContext();

		double now = ctx->Time * 1000.0f;

		if (anim.StartTime == 0 && *aValue != anim.StartValue)
		{
			anim.StartTime = now - (aDurationMs * currProgressVal);
			anim.StartValue = aStartValue;
			anim.EndValue = aEndValue;
		}
		else if (anim.StartTime != 0 && (anim.StartValue != aStartValue || anim.EndValue != aEndValue))
		{
			anim.StartTime = now - (aDurationMs - (now - anim.StartTime));
			anim.StartValue = aStartValue;
			anim.EndValue = aEndValue;
		}
		else if (anim.StartTime == 0)
		{
			anim.StartTime = now;
			anim.StartValue = aStartValue;
			anim.EndValue = aEndValue;
		}

		anim.ChangeTime = now;

		float progress = (float)((anim.ChangeTime - anim.StartTime) / aDurationMs);
		if (progress > 1.0f) { progress = 1.0f; }
		else if (progress < 0.0f) { progress = 0.0f; }

		ImAnimate::PFN_CURVEFUNC curveFunc = ImAnimate::GetCurveFunc(aCurve);

		float curveResult = curveFunc(progress);
		float result = 0;
		if (anim.StartValue > anim.EndValue)
		{
			result = anim.StartValue - (curveResult * range);
		}
		else
		{
			result = (curveResult * range) + anim.StartValue;
		}

		float clampedValPost = ImAnimate::Clamp(result, aStartValue, aEndValue);

		if (clampedValPost != result || clampedValPost == aEndValue)
		{
			anim.StartTime = 0;
			*aValue = clampedValPost;
			return;
		}

		*aValue = result;
	}
}
