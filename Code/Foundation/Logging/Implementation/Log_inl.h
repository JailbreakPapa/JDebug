#pragma once

#if WD_DISABLED(WD_COMPILE_FOR_DEVELOPMENT)

inline void wdLog::Dev(wdLogInterface* /*pInterface*/, const wdFormatString& /*string*/) {}

#endif

#if WD_DISABLED(WD_COMPILE_FOR_DEBUG)

inline void wdLog::Debug(wdLogInterface* /*pInterface*/, const wdFormatString& /*string*/) {}

#endif
