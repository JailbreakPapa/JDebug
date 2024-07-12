/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

//============================================================================
/// \file   IconProvider.cpp
/// \author Uwe Kindler
/// \date   18.10.2019
/// \brief  Implementation of CIconProvider
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "IconProvider.h"
#include <QVector>

namespace ads
{
/**
 * Private data class (pimpl)
 */
struct IconProviderPrivate
{
	CIconProvider *_this;
	QVector<QIcon> UserIcons{IconCount, QIcon()};

	/**
	 * Private data constructor
	 */
	IconProviderPrivate(CIconProvider *_public);
};
// struct LedArrayPanelPrivate

//============================================================================
IconProviderPrivate::IconProviderPrivate(CIconProvider *_public) :
	_this(_public)
{

}

//============================================================================
CIconProvider::CIconProvider() :
	d(new IconProviderPrivate(this))
{

}

//============================================================================
CIconProvider::~CIconProvider()
{
	delete d;
}


//============================================================================
QIcon CIconProvider::customIcon(eIcon IconId) const
{
	Q_ASSERT(IconId < d->UserIcons.size());
	return d->UserIcons[IconId];
}


//============================================================================
void CIconProvider::registerCustomIcon(eIcon IconId, const QIcon &icon)
{
	Q_ASSERT(IconId < d->UserIcons.size());
	d->UserIcons[IconId] = icon;
}

} // namespace ads




//---------------------------------------------------------------------------
// EOF IconProvider.cpp
