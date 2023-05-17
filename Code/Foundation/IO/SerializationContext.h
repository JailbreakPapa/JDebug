
#pragma once

/// \brief Base class for serialization contexts. A serialization context can be used to add high level logic to serialization, e.g.
/// de-duplicating objects.
///
/// Typically a context is created before any serialization happens and can then be accessed anywhere through the GetContext method.
template <typename Derived>
class wdSerializationContext
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdSerializationContext);

public:
  wdSerializationContext() { Derived::SetContext(this); }
  ~wdSerializationContext() { Derived::SetContext(nullptr); }

  /// \brief Set the context as active which means it can be accessed via GetContext in serialization methods.
  ///
  /// It can be useful to manually set a context as active if a serialization process is spread across multiple scopes
  /// and other serialization can happen in between.
  void SetActive(bool bActive) { Derived::SetContext(bActive ? this : nullptr); }
};

/// \brief Declares the necessary functions to access a serialization context
#define WD_DECLARE_SERIALIZATION_CONTEXT(type) \
public:                                        \
  static type* GetContext();                   \
                                               \
protected:                                     \
  friend class wdSerializationContext<type>;   \
  static void SetContext(wdSerializationContext* pContext);


/// \brief Implements the necessary functions to access a serialization context through GetContext.
#define WD_IMPLEMENT_SERIALIZATION_CONTEXT(type)                                                                                     \
  thread_local type* WD_CONCAT(s_pActiveContext, type);                                                                              \
  type* type::GetContext() { return WD_CONCAT(s_pActiveContext, type); }                                                             \
  void type::SetContext(wdSerializationContext* pContext)                                                                            \
  {                                                                                                                                  \
    WD_ASSERT_DEV(pContext == nullptr || WD_CONCAT(s_pActiveContext, type) == nullptr, "Only one context can be active at a time."); \
    WD_CONCAT(s_pActiveContext, type) = static_cast<type*>(pContext);                                                                \
  }
