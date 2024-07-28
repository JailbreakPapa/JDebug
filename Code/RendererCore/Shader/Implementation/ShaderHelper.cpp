#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Shader/ShaderHelper.h>

namespace nsShaderHelper
{
  void nsTextSectionizer::Clear()
  {
    m_Sections.Clear();
    m_sText.Clear();
  }

  void nsTextSectionizer::AddSection(const char* szName)
  {
    m_Sections.PushBack(nsTextSection(szName));
  }

  void nsTextSectionizer::Process(const char* szText)
  {
    for (nsUInt32 i = 0; i < m_Sections.GetCount(); ++i)
      m_Sections[i].Reset();

    m_sText = szText;


    for (nsUInt32 s = 0; s < m_Sections.GetCount(); ++s)
    {
      m_Sections[s].m_szSectionStart = m_sText.FindSubString_NoCase(m_Sections[s].m_sName.GetData());

      if (m_Sections[s].m_szSectionStart != nullptr)
        m_Sections[s].m_Content = nsStringView(m_Sections[s].m_szSectionStart + m_Sections[s].m_sName.GetElementCount());
    }

    for (nsUInt32 s = 0; s < m_Sections.GetCount(); ++s)
    {
      if (m_Sections[s].m_szSectionStart == nullptr)
        continue;

      nsUInt32 uiLine = 1;

      const char* sz = m_sText.GetData();
      while (sz < m_Sections[s].m_szSectionStart)
      {
        if (*sz == '\n')
          ++uiLine;

        ++sz;
      }

      m_Sections[s].m_uiFirstLine = uiLine;

      for (nsUInt32 s2 = 0; s2 < m_Sections.GetCount(); ++s2)
      {
        if (s == s2)
          continue;

        if (m_Sections[s2].m_szSectionStart > m_Sections[s].m_szSectionStart)
        {
          const char* szContentStart = m_Sections[s].m_Content.GetStartPointer();
          const char* szSectionEnd = nsMath::Min(m_Sections[s].m_Content.GetEndPointer(), m_Sections[s2].m_szSectionStart);

          m_Sections[s].m_Content = nsStringView(szContentStart, szSectionEnd);
        }
      }
    }
  }

  nsStringView nsTextSectionizer::GetSectionContent(nsUInt32 uiSection, nsUInt32& out_uiFirstLine) const
  {
    out_uiFirstLine = m_Sections[uiSection].m_uiFirstLine;
    return m_Sections[uiSection].m_Content;
  }

  void GetShaderSections(const char* szContent, nsTextSectionizer& out_sections)
  {
    out_sections.Clear();

    out_sections.AddSection("[PLATFORMS]");
    out_sections.AddSection("[PERMUTATIONS]");
    out_sections.AddSection("[MATERIALPARAMETER]");
    out_sections.AddSection("[MATERIALCONFIG]");
    out_sections.AddSection("[RENDERSTATE]");
    out_sections.AddSection("[SHADER]");
    out_sections.AddSection("[VERTEXSHADER]");
    out_sections.AddSection("[HULLSHADER]");
    out_sections.AddSection("[DOMAINSHADER]");
    out_sections.AddSection("[GEOMETRYSHADER]");
    out_sections.AddSection("[PIXELSHADER]");
    out_sections.AddSection("[COMPUTESHADER]");
    out_sections.AddSection("[TEMPLATE_VARS]");

    out_sections.Process(szContent);
  }

  nsUInt32 CalculateHash(const nsArrayPtr<nsPermutationVar>& vars)
  {
    nsHybridArray<nsUInt64, 128> buffer;
    buffer.SetCountUninitialized(vars.GetCount() * 2);

    for (nsUInt32 i = 0; i < vars.GetCount(); ++i)
    {
      auto& var = vars[i];
      buffer[i * 2 + 0] = var.m_sName.GetHash();
      buffer[i * 2 + 1] = var.m_sValue.GetHash();
    }

    auto bytes = buffer.GetByteArrayPtr();
    return nsHashingUtils::xxHash32(bytes.GetPtr(), bytes.GetCount());
  }
} // namespace nsShaderHelper
