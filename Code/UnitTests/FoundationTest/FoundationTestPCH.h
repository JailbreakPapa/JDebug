#pragma once

#include <TestFramework/Framework/TestFramework.h>
#include <TestFramework/Utilities/ConstructionCounter.h>

#include <Foundation/Basics.h>
#include <Foundation/Basics/Assert.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/TypeTraits.h>
#include <Foundation/Types/Types.h>
#include <Foundation/Types/Variant.h>

#include <Foundation/Math/Declarations.h>

using nsMathTestType = float;

using nsVec2T = nsVec2Template<nsMathTestType>;                           ///< This is only for testing purposes
using nsVec3T = nsVec3Template<nsMathTestType>;                           ///< This is only for testing purposes
using nsVec4T = nsVec4Template<nsMathTestType>;                           ///< This is only for testing purposes
using nsMat3T = nsMat3Template<nsMathTestType>;                           ///< This is only for testing purposes
using nsMat4T = nsMat4Template<nsMathTestType>;                           ///< This is only for testing purposes
using nsQuatT = nsQuatTemplate<nsMathTestType>;                           ///< This is only for testing purposes
using nsPlaneT = nsPlaneTemplate<nsMathTestType>;                         ///< This is only for testing purposes
using nsBoundingBoxT = nsBoundingBoxTemplate<nsMathTestType>;             ///< This is only for testing purposes
using nsBoundingBoxSphereT = nsBoundingBoxSphereTemplate<nsMathTestType>; ///< This is only for testing purposes
using nsBoundingSphereT = nsBoundingSphereTemplate<nsMathTestType>;       ///< This is only for testing purposes
using nsTransformT = nsTransformTemplate<nsMathTestType>;

#define nsFoundationTest_Plugin1 "nsFoundationTest_Plugin1"
#define nsFoundationTest_Plugin2 "nsFoundationTest_Plugin2"
