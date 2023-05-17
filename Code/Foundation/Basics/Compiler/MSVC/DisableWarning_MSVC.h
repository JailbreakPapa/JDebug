
#ifdef WD_MSVC_WARNING_NUMBER

#  if WD_ENABLED(WD_COMPILER_MSVC)

#    pragma warning(push)
#    pragma warning(disable \
                    : WD_MSVC_WARNING_NUMBER)

#  endif

#  undef WD_MSVC_WARNING_NUMBER

#endif
