#pragma once

#if NS_DISABLED(NS_COMPILE_FOR_DEVELOPMENT)

inline void nsLog::Dev(nsLogInterface* /*pInterface*/, const nsFormatString& /*string*/) {}

#endif

#if NS_DISABLED(NS_COMPILE_FOR_DEBUG)

inline void nsLog::Debug(nsLogInterface* /*pInterface*/, const nsFormatString& /*string*/)
{
}

#endif
