
#ifdef WD_GCC_WARNING_NAME

#  if WD_ENABLED(WD_COMPILER_GCC)

#    pragma GCC diagnostic push
_Pragma(WD_STRINGIZE(GCC diagnostic ignored WD_GCC_WARNING_NAME))

#  endif

#  undef WD_GCC_WARNING_NAME

#endif
