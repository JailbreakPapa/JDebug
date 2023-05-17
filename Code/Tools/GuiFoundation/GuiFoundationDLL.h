#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Types/Uuid.h>
#include <QColor>
#include <QMetaType>
#include <ToolsFoundation/ToolsFoundationDLL.h>

// Configure the DLL Import/Export Define
#if WD_ENABLED(WD_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_GUIFOUNDATION_LIB
#    define WD_GUIFOUNDATION_DLL WD_DECL_EXPORT
#  else
#    define WD_GUIFOUNDATION_DLL WD_DECL_IMPORT
#  endif
#else
#  define WD_GUIFOUNDATION_DLL
#endif

class QWidget;
class QObject;


Q_DECLARE_METATYPE(wdUuid);

/// \brief Calls setUpdatesEnabled(false) on all given QObjects, and the reverse in the destructor. Can be nested.
class WD_GUIFOUNDATION_DLL wdQtScopedUpdatesDisabled
{
public:
  wdQtScopedUpdatesDisabled(QWidget* pWidget1, QWidget* pWidget2 = nullptr, QWidget* pWidget3 = nullptr, QWidget* pWidget4 = nullptr,
    QWidget* pWidget5 = nullptr, QWidget* pWidget6 = nullptr);
  ~wdQtScopedUpdatesDisabled();

private:
  QWidget* m_pWidgets[6];
};


/// \brief Calls blockSignals(true) on all given QObjects, and the reverse in the destructor. Can be nested.
class WD_GUIFOUNDATION_DLL wdQtScopedBlockSignals
{
public:
  wdQtScopedBlockSignals(QObject* pObject1, QObject* pObject2 = nullptr, QObject* pObject3 = nullptr, QObject* pObject4 = nullptr,
    QObject* pObject5 = nullptr, QObject* pObject6 = nullptr);
  ~wdQtScopedBlockSignals();

private:
  QObject* m_pObjects[6];
};

WD_ALWAYS_INLINE QColor wdToQtColor(const wdColorGammaUB& c)
{
  return QColor(c.r, c.g, c.b, c.a);
}

WD_ALWAYS_INLINE wdColorGammaUB qtToEzColor(const QColor& c)
{
  return wdColorGammaUB(c.red(), c.green(), c.blue(), c.alpha());
}
