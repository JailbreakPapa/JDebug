/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

//============================================================================
/// \file   DockSplitter.cpp
/// \author Uwe Kindler
/// \date   24.03.2017
/// \brief  Implementation of CDockSplitter
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include "DockSplitter.h"

#include <QDebug>
#include <QChildEvent>
#include <QVariant>
#include "DockAreaWidget.h"

namespace ads
{
/**
 * Private dock splitter data
 */
struct DockSplitterPrivate
{
	CDockSplitter* _this;
	int VisibleContentCount = 0;

	DockSplitterPrivate(CDockSplitter* _public) : _this(_public) {}
};

//============================================================================
CDockSplitter::CDockSplitter(QWidget *parent)
	: QSplitter(parent),
	  d(new DockSplitterPrivate(this))
{
    setProperty("ads-splitter", QVariant(true));
	setChildrenCollapsible(false);
}


//============================================================================
CDockSplitter::CDockSplitter(Qt::Orientation orientation, QWidget *parent)
	: QSplitter(orientation, parent),
	  d(new DockSplitterPrivate(this))
{

}

//============================================================================
CDockSplitter::~CDockSplitter()
{
    ADS_PRINT("~CDockSplitter");
	delete d;
}


//============================================================================
bool CDockSplitter::hasVisibleContent() const
{
	// TODO Cache or precalculate this to speed up
	for (int i = 0; i < count(); ++i)
	{
		if (!widget(i)->isHidden())
		{
			return true;
		}
	}

	return false;
}


//============================================================================
QWidget* CDockSplitter::firstWidget() const
{
	return (count() > 0) ? widget(0) : nullptr;
}


//============================================================================
QWidget* CDockSplitter::lastWidget() const
{
	return (count() > 0) ? widget(count() - 1) : nullptr;
}

//============================================================================
bool CDockSplitter::isResizingWithContainer() const
{
    for (auto area : findChildren<CDockAreaWidget*>())
    {
        if(area->isCentralWidgetArea())
        {
            return true;
        }
    }

    return false;
}

} // namespace ads

//---------------------------------------------------------------------------
// EOF DockSplitter.cpp
