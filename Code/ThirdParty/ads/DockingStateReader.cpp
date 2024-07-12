/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

//============================================================================
/// \file   DockingStateReader.cpp
/// \author Uwe Kindler
/// \date   29.11.2019
/// \brief  Implementation of CDockingStateReader
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "DockingStateReader.h"

namespace ads
{

//============================================================================
void CDockingStateReader::setFileVersion(int FileVersion)
{
	m_FileVersion = FileVersion;
}

//============================================================================
int CDockingStateReader::fileVersion() const
{
	return m_FileVersion;
}
} // namespace ads

//---------------------------------------------------------------------------
// EOF DockingStateReader.cpp
