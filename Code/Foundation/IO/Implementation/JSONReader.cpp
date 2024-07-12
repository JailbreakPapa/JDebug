#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONReader.h>


nsJSONReader::nsJSONReader()
{
  m_bParsingError = false;
}

nsResult nsJSONReader::Parse(nsStreamReader& ref_inputStream, nsUInt32 uiFirstLineOffset)
{
  m_bParsingError = false;
  m_Stack.Clear();
  m_sLastName.Clear();

  SetInputStream(ref_inputStream, uiFirstLineOffset);

  while (!m_bParsingError && ContinueParsing())
  {
  }

  if (m_bParsingError)
  {
    m_Stack.Clear();
    m_Stack.PushBack(Element());

    return NS_FAILURE;
  }

  // make sure there is one top level element
  if (m_Stack.IsEmpty())
  {
    Element& e = m_Stack.ExpandAndGetRef();
    e.m_Mode = ElementType::None;
  }

  return NS_SUCCESS;
}

bool nsJSONReader::OnVariable(nsStringView sVarName)
{
  m_sLastName = sVarName;

  return true;
}

void nsJSONReader::OnReadValue(nsStringView sValue)
{
  if (m_Stack.PeekBack().m_Mode == ElementType::Array)
    m_Stack.PeekBack().m_Array.PushBack(std::move(nsString(sValue)));
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = std::move(nsString(sValue));

  m_sLastName.Clear();
}

void nsJSONReader::OnReadValue(double fValue)
{
  if (m_Stack.PeekBack().m_Mode == ElementType::Array)
    m_Stack.PeekBack().m_Array.PushBack(nsVariant(fValue));
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = nsVariant(fValue);

  m_sLastName.Clear();
}

void nsJSONReader::OnReadValue(bool bValue)
{
  if (m_Stack.PeekBack().m_Mode == ElementType::Array)
    m_Stack.PeekBack().m_Array.PushBack(nsVariant(bValue));
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = nsVariant(bValue);

  m_sLastName.Clear();
}

void nsJSONReader::OnReadValueNULL()
{
  if (m_Stack.PeekBack().m_Mode == ElementType::Array)
    m_Stack.PeekBack().m_Array.PushBack(nsVariant());
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = nsVariant();

  m_sLastName.Clear();
}

void nsJSONReader::OnBeginObject()
{
  m_Stack.PushBack(Element());
  m_Stack.PeekBack().m_Mode = ElementType::Dictionary;
  m_Stack.PeekBack().m_sName = m_sLastName;

  m_sLastName.Clear();
}

void nsJSONReader::OnEndObject()
{
  Element& Child = m_Stack[m_Stack.GetCount() - 1];

  if (m_Stack.GetCount() > 1)
  {
    Element& Parent = m_Stack[m_Stack.GetCount() - 2];

    if (Parent.m_Mode == ElementType::Array)
    {
      Parent.m_Array.PushBack(Child.m_Dictionary);
    }
    else
    {
      Parent.m_Dictionary[Child.m_sName] = std::move(Child.m_Dictionary);
    }

    m_Stack.PopBack();
  }
  else
  {
    // do nothing, keep the top-level dictionary
  }
}

void nsJSONReader::OnBeginArray()
{
  m_Stack.PushBack(Element());
  m_Stack.PeekBack().m_Mode = ElementType::Array;
  m_Stack.PeekBack().m_sName = m_sLastName;

  m_sLastName.Clear();
}

void nsJSONReader::OnEndArray()
{
  Element& Child = m_Stack[m_Stack.GetCount() - 1];

  if (m_Stack.GetCount() > 1)
  {
    Element& Parent = m_Stack[m_Stack.GetCount() - 2];

    if (Parent.m_Mode == ElementType::Array)
    {
      Parent.m_Array.PushBack(Child.m_Array);
    }
    else
    {
      Parent.m_Dictionary[Child.m_sName] = std::move(Child.m_Array);
    }

    m_Stack.PopBack();
  }
  else
  {
    // do nothing, keep the top-level array
  }
}



void nsJSONReader::OnParsingError(nsStringView sMessage, bool bFatal, nsUInt32 uiLine, nsUInt32 uiColumn)
{
  m_bParsingError = true;
}
