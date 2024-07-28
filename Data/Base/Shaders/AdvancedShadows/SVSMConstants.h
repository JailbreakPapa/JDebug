#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"


CONSTANT_BUFFER(nsSVSMConstants,3)
{
    FLOAT1(ScreenRayLength);
    UINT1(SVSMRayCount); ///< Number of rays to cast. PC: ANY, PS5: Keep Relative Towards 8k by 8k Resolution. DevKits can go higher, but it should be locked on test-kits. See ECLIPSE-SVSMS-CONSOLE-NOTES on WD Studios Internal Net for more information. XBSX: Keep Relative Towards 8k by 8k Resolution. 
    UINT1(SVSMRaySamplesPFrame);
    FLOAT1(SVSMRayLengthScale);
    FLOAT1(SVSMAngleFromLight);
    FLOAT1(SVSMTexelDitherScale);
    FLOAT1(SVSMExtrapolateSlope);
    FLOAT1(SVSMMaxSlopeBias);
    UINT1(SVSMAdaptedRayCount);
};