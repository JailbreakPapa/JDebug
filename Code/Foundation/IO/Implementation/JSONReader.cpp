#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONReader.h>


wdJSONReader::wdJSONReader()
{
  m_bParsingError = false;
}

wdResult wdJSONReader::Parse(wdStreamReader& ref_inputStream, wdUInt32 uiFirstLineOffset)
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

    return WD_FAILURE;
  }

  // make sure there is one top level element
  if (m_Stack.IsEmpty())
    m_Stack.PushBack(Element());

  return WD_SUCCESS;
}

bool wdJSONReader::OnVariable(const char* szVarName)
{
  m_sLastName = szVarName;

  return true;
}

void wdJSONReader::OnReadValue(const char* szValue)
{
  if (m_Stack.PeekBack().m_Mode == ElementMode::Array)
    m_Stack.PeekBack().m_Array.PushBack(wdVariant(szValue));
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = wdVariant(szValue);

  m_sLastName.Clear();
}

void wdJSONReader::OnReadValue(double fValue)
{
  if (m_Stack.PeekBack().m_Mode == ElementMode::Array)
    m_Stack.PeekBack().m_Array.PushBack(wdVariant(fValue));
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = wdVariant(fValue);

  m_sLastName.Clear();
}

void wdJSONReader::OnReadValue(bool bValue)
{
  if (m_Stack.PeekBack().m_Mode == ElementMode::Array)
    m_Stack.PeekBack().m_Array.PushBack(wdVariant(bValue));
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = wdVariant(bValue);

  m_sLastName.Clear();
}

void wdJSONReader::OnReadValueNULL()
{
  if (m_Stack.PeekBack().m_Mode == ElementMode::Array)
    m_Stack.PeekBack().m_Array.PushBack(wdVariant());
  else
    m_Stack.PeekBack().m_Dictionary[m_sLastName] = wdVariant();

  m_sLastName.Clear();
}

void wdJSONReader::OnBeginObject()
{
  m_Stack.PushBack(Element());
  m_Stack.PeekBack().m_Mode = ElementMode::Dictionary;
  m_Stack.PeekBack().m_sName = m_sLastName;

  m_sLastName.Clear();
}

void wdJSONReader::OnEndObject()
{
  Element& Child = m_Stack[m_Stack.GetCount() - 1];

  if (m_Stack.GetCount() > 1)
  {
    Element& Parent = m_Stack[m_Stack.GetCount() - 2];

    if (Parent.m_Mode == ElementMode::Array)
    {
      Parent.m_Array.PushBack(Child.m_Dictionary);
    }
    else
    {
      Parent.m_Dictionary[Child.m_sName] = Child.m_Dictionary;
    }

    m_Stack.PopBack();
  }
  else
  {
    // do nothing, keep the top-level dictionary
  }
}

void wdJSONReader::OnBeginArray()
{
  m_Stack.PushBack(Element());
  m_Stack.PeekBack().m_Mode = ElementMode::Array;
  m_Stack.PeekBack().m_sName = m_sLastName;

  m_sLastName.Clear();
}

void wdJSONReader::OnEndArray()
{
  Element& Child = m_Stack[m_Stack.GetCount() - 1];
  Element& Parent = m_Stack[m_Stack.GetCount() - 2];

  if (Parent.m_Mode == ElementMode::Array)
  {
    Parent.m_Array.PushBack(Child.m_Array);
  }
  else
  {
    Parent.m_Dictionary[Child.m_sName] = Child.m_Array;
  }

  m_Stack.PopBack();
}



void wdJSONReader::OnParsingError(const char* szMessage, bool bFatal, wdUInt32 uiLine, wdUInt32 uiColumn)
{
  m_bParsingError = true;
}

WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_JSONReader);
