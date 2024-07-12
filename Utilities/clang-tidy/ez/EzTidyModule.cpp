//===--- MiscTidyModule.cpp - clang-tidy ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "NameCheck.h"

namespace clang
{
  namespace tidy
  {
    namespace ns
    {

      class NsModule : public ClangTidyModule
      {
      public:
        void addCheckFactories(ClangTidyCheckFactories& CheckFactories) override
        {
          CheckFactories.registerCheck<NameCheck>(
            "ns-name-check");
        }
      };

    } // namespace ns

    // Register the MiscTidyModule using this statically initialized variable.
    static ClangTidyModuleRegistry::Add<ns::NsModule>
      X("ns-module", "Adds ns engine specific lint checks.");

    // This anchor is used to force the linker to link in the generated object file
    // and thus register the NsModule.
    volatile int NsModuleAnchorSource = 0;

  } // namespace tidy
} // namespace clang
