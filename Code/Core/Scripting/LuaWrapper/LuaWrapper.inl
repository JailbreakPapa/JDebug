#ifdef BUILDSYSTEM_ENABLE_LUA_SUPPORT

#  pragma once

inline lua_State* nsLuaWrapper::GetLuaState()
{
  return m_pState;
}

inline nsInt32 nsLuaWrapper::ReturnToScript() const
{
  return (m_States.m_iParametersPushed);
}

inline nsUInt32 nsLuaWrapper::GetNumberOfFunctionParameters() const
{
  return ((int)lua_gettop(m_pState));
}

inline bool nsLuaWrapper::IsParameterBool(nsUInt32 uiParameter) const
{
  return (lua_type(m_pState, uiParameter + s_iParamOffset) == LUA_TBOOLEAN);
}

inline bool nsLuaWrapper::IsParameterFloat(nsUInt32 uiParameter) const
{
  return (lua_type(m_pState, uiParameter + s_iParamOffset) == LUA_TNUMBER);
}

inline bool nsLuaWrapper::IsParameterInt(nsUInt32 uiParameter) const
{
  return (lua_type(m_pState, uiParameter + s_iParamOffset) == LUA_TNUMBER);
}

inline bool nsLuaWrapper::IsParameterString(nsUInt32 uiParameter) const
{
  return (lua_type(m_pState, uiParameter + s_iParamOffset) == LUA_TSTRING);
}

inline bool nsLuaWrapper::IsParameterNil(nsUInt32 uiParameter) const
{
  return (lua_type(m_pState, uiParameter + s_iParamOffset) == LUA_TNIL);
}

inline bool nsLuaWrapper::IsParameterTable(nsUInt32 uiParameter) const
{
  return (lua_type(m_pState, uiParameter + s_iParamOffset) == LUA_TTABLE);
}

inline void nsLuaWrapper::PushParameter(nsInt32 iParameter)
{
  lua_pushinteger(m_pState, iParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushParameter(bool bParameter)
{
  lua_pushboolean(m_pState, bParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushParameter(float fParameter)
{
  lua_pushnumber(m_pState, fParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushParameter(const char* szParameter)
{
  lua_pushstring(m_pState, szParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushParameter(const char* szParameter, nsUInt32 uiLength)
{
  lua_pushlstring(m_pState, szParameter, uiLength);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushParameterNil()
{
  lua_pushnil(m_pState);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushReturnValue(nsInt32 iParameter)
{
  lua_pushinteger(m_pState, iParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushReturnValue(bool bParameter)
{
  lua_pushboolean(m_pState, bParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushReturnValue(float fParameter)
{
  lua_pushnumber(m_pState, fParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushReturnValue(const char* szParameter)
{
  lua_pushstring(m_pState, szParameter);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushReturnValue(const char* szParameter, nsUInt32 uiLength)
{
  lua_pushlstring(m_pState, szParameter, uiLength);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::PushReturnValueNil()
{
  lua_pushnil(m_pState);
  m_States.m_iParametersPushed++;
}

inline void nsLuaWrapper::SetVariableNil(const char* szName) const
{
  lua_pushnil(m_pState);

  if (m_States.m_iOpenTables == 0)
    lua_setglobal(m_pState, szName);
  else
    lua_setfield(m_pState, -2, szName);
}

inline void nsLuaWrapper::SetVariable(const char* szName, nsInt32 iValue) const
{
  lua_pushinteger(m_pState, iValue);

  if (m_States.m_iOpenTables == 0)
    lua_setglobal(m_pState, szName);
  else
    lua_setfield(m_pState, -2, szName);
}

inline void nsLuaWrapper::SetVariable(const char* szName, float fValue) const
{
  lua_pushnumber(m_pState, fValue);

  if (m_States.m_iOpenTables == 0)
    lua_setglobal(m_pState, szName);
  else
    lua_setfield(m_pState, -2, szName);
}

inline void nsLuaWrapper::SetVariable(const char* szName, bool bValue) const
{
  lua_pushboolean(m_pState, bValue);

  if (m_States.m_iOpenTables == 0)
    lua_setglobal(m_pState, szName);
  else
    lua_setfield(m_pState, -2, szName);
}

inline void nsLuaWrapper::SetVariable(const char* szName, const char* szValue) const
{
  lua_pushstring(m_pState, szValue);

  if (m_States.m_iOpenTables == 0)
    lua_setglobal(m_pState, szName);
  else
    lua_setfield(m_pState, -2, szName);
}

inline void nsLuaWrapper::SetVariable(const char* szName, const char* szValue, nsUInt32 uiLen) const
{
  lua_pushlstring(m_pState, szValue, uiLen);

  if (m_States.m_iOpenTables == 0)
    lua_setglobal(m_pState, szName);
  else
    lua_setfield(m_pState, -2, szName);
}

inline void nsLuaWrapper::PushTable(const char* szTableName, bool bGlobalTable)
{
  if (bGlobalTable || m_States.m_iOpenTables == 0)
    lua_getglobal(m_pState, szTableName);
  else
  {
    lua_pushstring(m_pState, szTableName);
    lua_gettable(m_pState, -2);
  }

  m_States.m_iParametersPushed++;
}

inline int nsLuaWrapper::GetIntParameter(nsUInt32 uiParameter) const
{
  return ((int)(lua_tointeger(m_pState, uiParameter + s_iParamOffset)));
}

inline bool nsLuaWrapper::GetBoolParameter(nsUInt32 uiParameter) const
{
  return (lua_toboolean(m_pState, uiParameter + s_iParamOffset) != 0);
}

inline float nsLuaWrapper::GetFloatParameter(nsUInt32 uiParameter) const
{
  return ((float)(lua_tonumber(m_pState, uiParameter + s_iParamOffset)));
}

inline const char* nsLuaWrapper::GetStringParameter(nsUInt32 uiParameter) const
{
  return (lua_tostring(m_pState, uiParameter + s_iParamOffset));
}

#endif // BUILDSYSTEM_ENABLE_LUA_SUPPORT
