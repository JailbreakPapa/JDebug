#pragma once

#include <Foundation/Threading/TaskSystem.h>

/// \brief A simple task implementation that calls a delegate function.
template <typename T>
class wdDelegateTask final : public wdTask
{
public:
  typedef wdDelegate<void(const T&)> FunctionType;

  wdDelegateTask(const char* szTaskName, FunctionType func, const T& param)
  {
    m_Func = func;
    m_param = param;
    ConfigureTask(szTaskName, wdTaskNesting::Never);
  }

private:
  virtual void Execute() override { m_Func(m_param); }

  FunctionType m_Func;
  T m_param;
};

template <>
class wdDelegateTask<void> final : public wdTask
{
public:
  typedef wdDelegate<void()> FunctionType;

  wdDelegateTask(const char* szTaskName, FunctionType func)
  {
    m_Func = func;
    ConfigureTask(szTaskName, wdTaskNesting::Never);
  }

private:
  virtual void Execute() override { m_Func(); }

  FunctionType m_Func;
};
