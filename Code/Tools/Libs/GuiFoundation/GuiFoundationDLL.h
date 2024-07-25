#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Uuid.h>
#include <QColor>
#include <QDataStream>
#include <QMetaType>
#include <ToolsFoundation/ToolsFoundationDLL.h>

// Configure the DLL Import/Export Define
#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_GUIFOUNDATION_LIB
#    define NS_GUIFOUNDATION_DLL NS_DECL_EXPORT
#  else
#    define NS_GUIFOUNDATION_DLL NS_DECL_IMPORT
#  endif
#else
#  define NS_GUIFOUNDATION_DLL
#endif

class QWidget;
class QObject;


Q_DECLARE_METATYPE(nsUuid);

/// \brief Calls setUpdatesEnabled(false) on all given QObjects, and the reverse in the destructor. Can be nested.
class NS_GUIFOUNDATION_DLL nsQtScopedUpdatesDisabled
{
public:
  nsQtScopedUpdatesDisabled(QWidget* pWidget1, QWidget* pWidget2 = nullptr, QWidget* pWidget3 = nullptr, QWidget* pWidget4 = nullptr,
    QWidget* pWidget5 = nullptr, QWidget* pWidget6 = nullptr);
  ~nsQtScopedUpdatesDisabled();

private:
  QWidget* m_pWidgets[6];
};


/// \brief Calls blockSignals(true) on all given QObjects, and the reverse in the destructor. Can be nested.
class NS_GUIFOUNDATION_DLL nsQtScopedBlockSignals
{
public:
  nsQtScopedBlockSignals(QObject* pObject1, QObject* pObject2 = nullptr, QObject* pObject3 = nullptr, QObject* pObject4 = nullptr,
    QObject* pObject5 = nullptr, QObject* pObject6 = nullptr);
  ~nsQtScopedBlockSignals();

private:
  QObject* m_pObjects[6];
};

NS_ALWAYS_INLINE QColor nsToQtColor(const nsColorGammaUB& c)
{
  return QColor(c.r, c.g, c.b, c.a);
}

NS_ALWAYS_INLINE nsColorGammaUB qtToNsColor(const QColor& c)
{
  return nsColorGammaUB(c.red(), c.green(), c.blue(), c.alpha());
}

NS_ALWAYS_INLINE nsString qtToNsString(const QString& sString)
{
  QByteArray data = sString.toUtf8();
  return nsString(nsStringView(data.data(), static_cast<nsUInt32>(data.size())));
}

NS_ALWAYS_INLINE QString nsMakeQString(nsStringView sString)
{
  return QString::fromUtf8(sString.GetStartPointer(), sString.GetElementCount());
}

template <typename T>
void operator>>(QDataStream& inout_stream, T*& rhs)
{
  void* p = nullptr;
  uint len = sizeof(void*);
  inout_stream.readRawData((char*)&p, len);
  rhs = (T*)p;
}


template <typename T>
void operator<<(QDataStream& inout_stream, T* rhs)
{
  inout_stream.writeRawData((const char*)&rhs, sizeof(void*));
}

template <typename T>
void operator>>(QDataStream& inout_stream, nsDynamicArray<T>& rhs)
{
  nsUInt32 uiIndices = 0;
  inout_stream >> uiIndices;
  rhs.Clear();
  rhs.Reserve(uiIndices);

  for (int i = 0; i < uiIndices; ++i)
  {
    T obj = {};
    inout_stream >> obj;
    rhs.PushBack(obj);
  }
}

template <typename T>
void operator<<(QDataStream& inout_stream, nsDynamicArray<T>& rhs)
{
  nsUInt32 iIndices = rhs.GetCount();
  inout_stream << iIndices;

  for (nsUInt32 i = 0; i < iIndices; ++i)
  {
    inout_stream << rhs[i];
  }
}
