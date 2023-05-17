#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <ToolsFoundation/Settings/ToolsTagRegistry.h>

struct TagComparer
{
  WD_ALWAYS_INLINE bool Less(const wdToolsTag* a, const wdToolsTag* b) const
  {
    if (a->m_sCategory != b->m_sCategory)
      return a->m_sCategory < b->m_sCategory;

    return a->m_sName < b->m_sName;
    ;
  }
};
////////////////////////////////////////////////////////////////////////
// wdToolsTagRegistry public functions
////////////////////////////////////////////////////////////////////////

wdMap<wdString, wdToolsTag> wdToolsTagRegistry::s_NameToTags;

void wdToolsTagRegistry::Clear()
{
  for (auto it = s_NameToTags.GetIterator(); it.IsValid();)
  {
    if (!it.Value().m_bBuiltInTag)
    {
      it = s_NameToTags.Remove(it);
    }
    else
    {
      ++it;
    }
  }
}

void wdToolsTagRegistry::WriteToDDL(wdStreamWriter& inout_stream)
{
  wdOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(wdOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);

  for (auto it = s_NameToTags.GetIterator(); it.IsValid(); ++it)
  {
    writer.BeginObject("Tag");

    writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::String, "Name");
    writer.WriteString(it.Value().m_sName);
    writer.EndPrimitiveList();

    writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::String, "Category");
    writer.WriteString(it.Value().m_sCategory);
    writer.EndPrimitiveList();

    writer.EndObject();
  }
}

wdStatus wdToolsTagRegistry::ReadFromDDL(wdStreamReader& inout_stream)
{
  wdOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream).Failed())
  {
    return wdStatus("Failed to read data from ToolsTagRegistry stream!");
  }

  // Makes sure not to remove the built-in tags
  Clear();

  const wdOpenDdlReaderElement* pRoot = reader.GetRootElement();

  for (const wdOpenDdlReaderElement* pTags = pRoot->GetFirstChild(); pTags != nullptr; pTags = pTags->GetSibling())
  {
    if (!pTags->IsCustomType("Tag"))
      continue;

    const wdOpenDdlReaderElement* pName = pTags->FindChildOfType(wdOpenDdlPrimitiveType::String, "Name");
    const wdOpenDdlReaderElement* pCategory = pTags->FindChildOfType(wdOpenDdlPrimitiveType::String, "Category");

    if (!pName || !pCategory)
    {
      wdLog::Error("Incomplete tag declaration!");
      continue;
    }

    wdToolsTag tag;
    tag.m_sName = pName->GetPrimitivesString()[0];
    tag.m_sCategory = pCategory->GetPrimitivesString()[0];

    if (!wdToolsTagRegistry::AddTag(tag))
    {
      wdLog::Error("Failed to add tag '{0}'", tag.m_sName);
    }
  }

  return wdStatus(WD_SUCCESS);
}

bool wdToolsTagRegistry::AddTag(const wdToolsTag& tag)
{
  if (tag.m_sName.IsEmpty())
    return false;

  auto it = s_NameToTags.Find(tag.m_sName);
  if (it.IsValid())
  {
    if (tag.m_bBuiltInTag)
    {
      // Make sure to pass this on, as it is not stored in the DDL file (because we don't want to rely on that)
      it.Value().m_bBuiltInTag = true;
    }

    return true;
  }
  else
  {
    s_NameToTags[tag.m_sName] = tag;
    return true;
  }
}

bool wdToolsTagRegistry::RemoveTag(const char* szName)
{
  auto it = s_NameToTags.Find(szName);
  if (it.IsValid())
  {
    s_NameToTags.Remove(it);
    return true;
  }
  else
  {
    return false;
  }
}

void wdToolsTagRegistry::GetAllTags(wdHybridArray<const wdToolsTag*, 16>& out_tags)
{
  out_tags.Clear();
  for (auto it = s_NameToTags.GetIterator(); it.IsValid(); ++it)
  {
    out_tags.PushBack(&it.Value());
  }

  out_tags.Sort(TagComparer());
}

void wdToolsTagRegistry::GetTagsByCategory(const wdArrayPtr<wdStringView>& categories, wdHybridArray<const wdToolsTag*, 16>& out_tags)
{
  out_tags.Clear();
  for (auto it = s_NameToTags.GetIterator(); it.IsValid(); ++it)
  {
    if (std::any_of(cbegin(categories), cend(categories), [&it](const wdStringView& sCat) { return it.Value().m_sCategory == sCat; }))
    {
      out_tags.PushBack(&it.Value());
    }
  }
  out_tags.Sort(TagComparer());
}
