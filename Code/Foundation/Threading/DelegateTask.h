#pragma once

#include <Foundation/Threading/TaskSystem.h>

/// \brief A simple task implementation that calls a delegate function.
template <typename T>
class nsDelegateTask final : public nsTask
{
public:
  using FunctionType = nsDelegate<void(const T&)>;

  nsDelegateTask(const char* szTaskName, nsTaskNesting taskNesting, FunctionType func, const T& param)
  {
    m_Func = func;
    m_param = param;
    ConfigureTask(szTaskName, taskNesting);
  }

private:
  virtual void Execute() override { m_Func(m_param); }

  FunctionType m_Func;
  T m_param;
};

template <>
class nsDelegateTask<void> final : public nsTask
{
public:
  using FunctionType = nsDelegate<void()>;

  nsDelegateTask(const char* szTaskName, nsTaskNesting taskNesting, FunctionType func)
  {
    m_Func = func;
    ConfigureTask(szTaskName, taskNesting);
  }

private:
  virtual void Execute() override { m_Func(); }

  FunctionType m_Func;
};
