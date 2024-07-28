#include <Core/CorePCH.h>

#include <Core/Scripting/LuaWrapper.h>

#ifdef BUILDSYSTEM_ENABLE_LUA_SUPPORT

nsLuaWrapper::nsLuaWrapper()
{
  m_bReleaseOnExit = true;
  m_pState = nullptr;

  Clear();
}

nsLuaWrapper::nsLuaWrapper(lua_State* s)
{
  m_pState = s;
  m_bReleaseOnExit = false;
}

nsLuaWrapper::~nsLuaWrapper()
{
  if (m_bReleaseOnExit)
    lua_close(m_pState);
}

void nsLuaWrapper::Clear()
{
  NS_ASSERT_DEV(m_bReleaseOnExit, "Cannot clear a script that did not create the Lua state itself.");

  if (m_pState)
    lua_close(m_pState);

  m_pState = lua_newstate(lua_allocator, nullptr);

  luaL_openlibs(m_pState);
}

nsResult nsLuaWrapper::ExecuteString(const char* szString, const char* szDebugChunkName, nsLogInterface* pLogInterface) const
{
  NS_ASSERT_DEV(m_States.m_iLuaReturnValues == 0,
    "nsLuaWrapper::ExecuteString: You didn't discard the return-values of the previous script call. {0} Return-values were expected.",
    m_States.m_iLuaReturnValues);

  if (!pLogInterface)
    pLogInterface = nsLog::GetThreadLocalLogSystem();

  int error = luaL_loadbuffer(m_pState, szString, nsStringUtils::GetStringElementCount(szString), szDebugChunkName);

  if (error != LUA_OK)
  {
    NS_LOG_BLOCK("nsLuaWrapper::ExecuteString");

    nsLog::Error(pLogInterface, "[lua]Lua compile error: {0}", lua_tostring(m_pState, -1));
    nsLog::Info(pLogInterface, "[luascript]Script: {0}", szString);

    return NS_FAILURE;
  }

  error = lua_pcall(m_pState, 0, 0, 0);

  if (error != LUA_OK)
  {
    NS_LOG_BLOCK("nsLuaWrapper::ExecuteString");

    nsLog::Error(pLogInterface, "[lua]Lua error: {0}", lua_tostring(m_pState, -1));
    nsLog::Info(pLogInterface, "[luascript]Script: {0}", szString);

    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

void* nsLuaWrapper::lua_allocator(void* ud, void* ptr, size_t osize, size_t nsize)
{
  /// \todo Create optimized allocator.

  if (nsize == 0)
  {
    delete[] (nsUInt8*)ptr;
    return (nullptr);
  }

  nsUInt8* ucPtr = new nsUInt8[nsize];

  if (ptr != nullptr)
  {
    nsMemoryUtils::Copy(ucPtr, (nsUInt8*)ptr, nsUInt32(osize < nsize ? osize : nsize));

    delete[] (nsUInt8*)ptr;
  }

  return ((void*)ucPtr);
}


#endif // BUILDSYSTEM_ENABLE_LUA_SUPPORT
