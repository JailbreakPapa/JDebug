#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/ShaderCompiler/PermutationGenerator.h>

void nsPermutationGenerator::Clear()
{
  m_Permutations.Clear();
}


void nsPermutationGenerator::RemovePermutations(const nsHashedString& sPermVarName)
{
  m_Permutations.Remove(sPermVarName);
}

void nsPermutationGenerator::AddPermutation(const nsHashedString& sName, const nsHashedString& sValue)
{
  NS_ASSERT_DEV(!sName.IsEmpty(), "");
  NS_ASSERT_DEV(!sValue.IsEmpty(), "");

  m_Permutations[sName].Insert(sValue);
}

nsUInt32 nsPermutationGenerator::GetPermutationCount() const
{
  nsUInt32 uiPermutations = 1;

  for (auto it = m_Permutations.GetIterator(); it.IsValid(); ++it)
  {
    uiPermutations *= it.Value().GetCount();
  }

  return uiPermutations;
}

void nsPermutationGenerator::GetPermutation(nsUInt32 uiPerm, nsHybridArray<nsPermutationVar, 16>& out_permVars) const
{
  out_permVars.Clear();

  for (auto itVariable = m_Permutations.GetIterator(); itVariable.IsValid(); ++itVariable)
  {
    const nsUInt32 uiValues = itVariable.Value().GetCount();
    nsUInt32 uiUseValue = uiPerm % uiValues;

    uiPerm /= uiValues;

    auto itValue = itVariable.Value().GetIterator();

    for (; uiUseValue > 0; --uiUseValue)
    {
      ++itValue;
    }

    nsPermutationVar& pv = out_permVars.ExpandAndGetRef();
    pv.m_sName = itVariable.Key();
    pv.m_sValue = itValue.Key();
  }
}
