
#pragma once

/// \brief Base class for serialization contexts. A serialization context can be used to add high level logic to serialization, e.g.
/// de-duplicating objects.
///
/// Typically a context is created before any serialization happens and can then be accessed anywhere through the GetContext method.
template <typename Derived>
class nsSerializationContext
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsSerializationContext);

public:
  nsSerializationContext() { Derived::SetContext(this); }
  ~nsSerializationContext() { Derived::SetContext(nullptr); }

  /// \brief Set the context as active which means it can be accessed via GetContext in serialization methods.
  ///
  /// It can be useful to manually set a context as active if a serialization process is spread across multiple scopes
  /// and other serialization can happen in between.
  void SetActive(bool bActive) { Derived::SetContext(bActive ? this : nullptr); }
};

/// \brief Declares the necessary functions to access a serialization context
#define NS_DECLARE_SERIALIZATION_CONTEXT(type) \
public:                                        \
  static type* GetContext();                   \
                                               \
protected:                                     \
  friend class nsSerializationContext<type>;   \
  static void SetContext(nsSerializationContext* pContext);


/// \brief Implements the necessary functions to access a serialization context through GetContext.
#define NS_IMPLEMENT_SERIALIZATION_CONTEXT(type)                                                                                     \
  thread_local type* NS_CONCAT(s_pActiveContext, type);                                                                              \
  type* type::GetContext()                                                                                                           \
  {                                                                                                                                  \
    return NS_CONCAT(s_pActiveContext, type);                                                                                        \
  }                                                                                                                                  \
  void type::SetContext(nsSerializationContext* pContext)                                                                            \
  {                                                                                                                                  \
    NS_ASSERT_DEV(pContext == nullptr || NS_CONCAT(s_pActiveContext, type) == nullptr, "Only one context can be active at a time."); \
    NS_CONCAT(s_pActiveContext, type) = static_cast<type*>(pContext);                                                                \
  }
