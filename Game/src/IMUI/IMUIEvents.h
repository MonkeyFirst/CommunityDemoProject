
#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D
{

	/// IMUI Render Event.
	URHO3D_EVENT(E_IMUIRENDER, IMUIRender)
	{
		URHO3D_PARAM(P_IMUI, imui);              // IMUI pointer
	}

}