// NO #pragma once in this file !

template <typename R NS_COMMA_IF(ARG_COUNT) NS_LIST(typename P, ARG_COUNT)>
class nsConsoleFunction<R(NS_LIST(P, ARG_COUNT))> : public nsConsoleFunctionBase
{
public:
  using FUNC = nsDelegate<R(NS_LIST(P, ARG_COUNT))>;

  FUNC m_Func;

  nsConsoleFunction(nsStringView sFunctionName, nsStringView sDescription, FUNC f)
    : nsConsoleFunctionBase(sFunctionName, sDescription)
  {
    m_Func = f;
  }

  nsUInt32 GetNumParameters() const override { return ARG_COUNT; }

  virtual nsVariant::Type::Enum GetParameterType(nsUInt32 uiParam) const override
  {
    NS_ASSERT_DEV(uiParam < GetNumParameters(), "Invalid Parameter Index {0}", uiParam);

#if (ARG_COUNT > 0)

    switch (uiParam)
    {
      case 0:
        return static_cast<nsVariant::Type::Enum>(nsVariant::TypeDeduction<P0>::value);

#  if (ARG_COUNT > 1)
      case 1:
        return static_cast<nsVariant::Type::Enum>(nsVariant::TypeDeduction<P1>::value);
#  endif
#  if (ARG_COUNT > 2)
      case 2:
        return static_cast<nsVariant::Type::Enum>(nsVariant::TypeDeduction<P2>::value);
#  endif
#  if (ARG_COUNT > 3)
      case 3:
        return static_cast<nsVariant::Type::Enum>(nsVariant::TypeDeduction<P3>::value);
#  endif
#  if (ARG_COUNT > 4)
      case 4:
        return static_cast<nsVariant::Type::Enum>(nsVariant::TypeDeduction<P4>::value);
#  endif
#  if (ARG_COUNT > 5)
      case 5:
        return static_cast<nsVariant::Type::Enum>(nsVariant::TypeDeduction<P5>::value);
#  endif
    }

#endif
    return nsVariant::Type::Invalid;
  }

  virtual nsResult Call(nsArrayPtr<nsVariant> params) override
  {
    nsResult r = NS_FAILURE;
    NS_IGNORE_UNUSED(r);

#if (ARG_COUNT > 0)
    P0 param0 = params[0].ConvertTo<P0>(&r);

    if (r.Failed())
      return NS_FAILURE;
#endif

#if (ARG_COUNT > 1)
    P1 param1 = params[1].ConvertTo<P1>(&r);

    if (r.Failed())
      return NS_FAILURE;
#endif

#if (ARG_COUNT > 2)
    P2 param2 = params[2].ConvertTo<P2>(&r);

    if (r.Failed())
      return NS_FAILURE;
#endif

#if (ARG_COUNT > 3)
    P3 param3 = params[3].ConvertTo<P3>(&r);

    if (r.Failed())
      return NS_FAILURE;
#endif

#if (ARG_COUNT > 4)
    P4 param4 = params[4].ConvertTo<P4>(&r);

    if (r.Failed())
      return NS_FAILURE;
#endif

#if (ARG_COUNT > 5)
    P5 param5 = params[5].ConvertTo<P5>(&r);

    if (r.Failed())
      return NS_FAILURE;
#endif

    m_Func(NS_LIST(param, ARG_COUNT));
    return NS_SUCCESS;
  }
};
