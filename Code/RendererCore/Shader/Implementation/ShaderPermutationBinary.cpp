#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Shader/ShaderPermutationBinary.h>

struct nsShaderPermutationBinaryVersion
{
  enum Enum : nsUInt32
  {
    Version1 = 1,
    Version2 = 2,
    Version3 = 3,
    Version4 = 4,
    Version5 = 5,
    Version6 = 6, // Fixed DX11 particles vanishing

    // Increase this version number to trigger shader recompilation

    ENUM_COUNT,
    Current = ENUM_COUNT - 1
  };
};

nsShaderPermutationBinary::nsShaderPermutationBinary()
{
  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    m_uiShaderStageHashes[stage] = 0;
}

nsResult nsShaderPermutationBinary::Write(nsStreamWriter& inout_stream)
{
  // write this at the beginning so that the file can be read as an nsDependencyFile
  m_DependencyFile.StoreCurrentTimeStamp();
  NS_SUCCEED_OR_RETURN(m_DependencyFile.WriteDependencyFile(inout_stream));

  const nsUInt8 uiVersion = nsShaderPermutationBinaryVersion::Current;

  if (inout_stream.WriteBytes(&uiVersion, sizeof(nsUInt8)).Failed())
    return NS_FAILURE;

  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (inout_stream.WriteDWordValue(&m_uiShaderStageHashes[stage]).Failed())
      return NS_FAILURE;
  }

  m_StateDescriptor.Save(inout_stream);

  inout_stream << m_PermutationVars.GetCount();

  for (auto& var : m_PermutationVars)
  {
    inout_stream << var.m_sName.GetString();
    inout_stream << var.m_sValue.GetString();
  }

  return NS_SUCCESS;
}

nsResult nsShaderPermutationBinary::Read(nsStreamReader& inout_stream, bool& out_bOldVersion)
{
  NS_SUCCEED_OR_RETURN(m_DependencyFile.ReadDependencyFile(inout_stream));

  nsUInt8 uiVersion = 0;

  if (inout_stream.ReadBytes(&uiVersion, sizeof(nsUInt8)) != sizeof(nsUInt8))
    return NS_FAILURE;

  NS_ASSERT_DEV(uiVersion <= nsShaderPermutationBinaryVersion::Current, "Wrong Version {0}", uiVersion);

  out_bOldVersion = uiVersion != nsShaderPermutationBinaryVersion::Current;

  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (inout_stream.ReadDWordValue(&m_uiShaderStageHashes[stage]).Failed())
      return NS_FAILURE;
  }

  m_StateDescriptor.Load(inout_stream);

  if (uiVersion >= nsShaderPermutationBinaryVersion::Version2)
  {
    nsUInt32 uiPermutationCount;
    inout_stream >> uiPermutationCount;

    m_PermutationVars.SetCount(uiPermutationCount);

    nsStringBuilder tmp;
    for (nsUInt32 i = 0; i < uiPermutationCount; ++i)
    {
      auto& var = m_PermutationVars[i];

      inout_stream >> tmp;
      var.m_sName.Assign(tmp.GetData());
      inout_stream >> tmp;
      var.m_sValue.Assign(tmp.GetData());
    }
  }

  return NS_SUCCESS;
}
