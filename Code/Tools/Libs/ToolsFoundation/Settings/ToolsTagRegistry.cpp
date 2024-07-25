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
  NS_ALWAYS_INLINE bool Less(const nsToolsTag* a, const nsToolsTag* b) const
  {
    if (a->m_sCategory != b->m_sCategory)
      return a->m_sCategory < b->m_sCategory;

    return a->m_sName < b->m_sName;
    ;
  }
};
////////////////////////////////////////////////////////////////////////
// nsToolsTagRegistry public functions
////////////////////////////////////////////////////////////////////////

nsMap<nsString, nsToolsTag> nsToolsTagRegistry::s_NameToTags;

void nsToolsTagRegistry::Clear()
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

void nsToolsTagRegistry::WriteToDDL(nsStreamWriter& inout_stream)
{
  nsOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(false);
  writer.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);

  for (auto it = s_NameToTags.GetIterator(); it.IsValid(); ++it)
  {
    writer.BeginObject("Tag");

    writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::String, "Name");
    writer.WriteString(it.Value().m_sName);
    writer.EndPrimitiveList();

    writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::String, "Category");
    writer.WriteString(it.Value().m_sCategory);
    writer.EndPrimitiveList();

    writer.EndObject();
  }
}

nsStatus nsToolsTagRegistry::ReadFromDDL(nsStreamReader& inout_stream)
{
  nsOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream).Failed())
  {
    return nsStatus("Failed to read data from ToolsTagRegistry stream!");
  }

  // Makes sure not to remove the built-in tags
  Clear();

  const nsOpenDdlReaderElement* pRoot = reader.GetRootElement();

  for (const nsOpenDdlReaderElement* pTags = pRoot->GetFirstChild(); pTags != nullptr; pTags = pTags->GetSibling())
  {
    if (!pTags->IsCustomType("Tag"))
      continue;

    const nsOpenDdlReaderElement* pName = pTags->FindChildOfType(nsOpenDdlPrimitiveType::String, "Name");
    const nsOpenDdlReaderElement* pCategory = pTags->FindChildOfType(nsOpenDdlPrimitiveType::String, "Category");

    if (!pName || !pCategory)
    {
      nsLog::Error("Incomplete tag declaration!");
      continue;
    }

    nsToolsTag tag;
    tag.m_sName = pName->GetPrimitivesString()[0];
    tag.m_sCategory = pCategory->GetPrimitivesString()[0];

    if (!nsToolsTagRegistry::AddTag(tag))
    {
      nsLog::Error("Failed to add tag '{0}'", tag.m_sName);
    }
  }

  return nsStatus(NS_SUCCESS);
}

bool nsToolsTagRegistry::AddTag(const nsToolsTag& tag)
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

bool nsToolsTagRegistry::RemoveTag(nsStringView sName)
{
  auto it = s_NameToTags.Find(sName);
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

void nsToolsTagRegistry::GetAllTags(nsHybridArray<const nsToolsTag*, 16>& out_tags)
{
  out_tags.Clear();
  for (auto it = s_NameToTags.GetIterator(); it.IsValid(); ++it)
  {
    out_tags.PushBack(&it.Value());
  }

  out_tags.Sort(TagComparer());
}

void nsToolsTagRegistry::GetTagsByCategory(const nsArrayPtr<nsStringView>& categories, nsHybridArray<const nsToolsTag*, 16>& out_tags)
{
  out_tags.Clear();
  for (auto it = s_NameToTags.GetIterator(); it.IsValid(); ++it)
  {
    if (std::any_of(cbegin(categories), cend(categories), [&it](const nsStringView& sCat)
          { return it.Value().m_sCategory == sCat; }))
    {
      out_tags.PushBack(&it.Value());
    }
  }
  out_tags.Sort(TagComparer());
}
