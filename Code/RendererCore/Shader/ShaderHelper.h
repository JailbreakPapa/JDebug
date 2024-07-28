#pragma once

#include <Foundation/Strings/String.h>
#include <RendererCore/Declarations.h>

namespace nsShaderHelper
{
  class NS_RENDERERCORE_DLL nsTextSectionizer
  {
  public:
    void Clear();

    void AddSection(const char* szName);

    void Process(const char* szText);

    nsStringView GetSectionContent(nsUInt32 uiSection, nsUInt32& out_uiFirstLine) const;

  private:
    struct nsTextSection
    {
      nsTextSection(const char* szName)
        : m_sName(szName)

      {
      }

      void Reset()
      {
        m_szSectionStart = nullptr;
        m_Content = nsStringView();
        m_uiFirstLine = 0;
      }

      nsString m_sName;
      const char* m_szSectionStart = nullptr;
      nsStringView m_Content;
      nsUInt32 m_uiFirstLine = 0;
    };

    nsStringBuilder m_sText;
    nsHybridArray<nsTextSection, 16> m_Sections;
  };

  struct nsShaderSections
  {
    enum Enum
    {
      PLATFORMS,
      PERMUTATIONS,
      MATERIALPARAMETER,
      MATERIALCONFIG,
      RENDERSTATE,
      SHADER,
      VERTEXSHADER,
      HULLSHADER,
      DOMAINSHADER,
      GEOMETRYSHADER,
      PIXELSHADER,
      COMPUTESHADER,
      TEMPLATE_VARS
    };
  };

  NS_RENDERERCORE_DLL void GetShaderSections(const char* szContent, nsTextSectionizer& out_sections);

  nsUInt32 CalculateHash(const nsArrayPtr<nsPermutationVar>& vars);
} // namespace nsShaderHelper
