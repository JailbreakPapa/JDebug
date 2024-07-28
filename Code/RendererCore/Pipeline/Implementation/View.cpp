#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Math/Frustum.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererCore/Pipeline/Extractor.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsCameraUsageHint, 1)
  NS_ENUM_CONSTANT(nsCameraUsageHint::None),
  NS_ENUM_CONSTANT(nsCameraUsageHint::MainView),
  NS_ENUM_CONSTANT(nsCameraUsageHint::EditorView),
  NS_ENUM_CONSTANT(nsCameraUsageHint::RenderTarget),
  NS_ENUM_CONSTANT(nsCameraUsageHint::Culling),
  NS_ENUM_CONSTANT(nsCameraUsageHint::Thumbnail),
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsView, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("RenderTarget0", m_PinRenderTarget0),
    NS_MEMBER_PROPERTY("RenderTarget1", m_PinRenderTarget1),
    NS_MEMBER_PROPERTY("RenderTarget2", m_PinRenderTarget2),
    NS_MEMBER_PROPERTY("RenderTarget3", m_PinRenderTarget3),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsView::nsView()
{
  m_pExtractTask = NS_DEFAULT_NEW(nsDelegateTask<void>, "", nsTaskNesting::Never, nsMakeDelegate(&nsView::ExtractData, this));
}

nsView::~nsView() = default;

void nsView::SetName(nsStringView sName)
{
  m_sName.Assign(sName);

  nsStringBuilder sb = sName;
  sb.Append(".ExtractData");
  m_pExtractTask->ConfigureTask(sb, nsTaskNesting::Maybe);
}

void nsView::SetWorld(nsWorld* pWorld)
{
  if (m_pWorld != pWorld)
  {
    m_pWorld = pWorld;

    nsRenderWorld::ResetRenderDataCache(*this);
  }
}

void nsView::SetSwapChain(nsGALSwapChainHandle hSwapChain)
{
  if (m_Data.m_hSwapChain != hSwapChain)
  {
    // Swap chain and render target setup are mutually exclusive.
    m_Data.m_hSwapChain = hSwapChain;
    m_Data.m_renderTargets = nsGALRenderTargets();
    if (m_pRenderPipeline)
    {
      nsRenderWorld::AddRenderPipelineToRebuild(m_pRenderPipeline, GetHandle());
    }
  }
}

void nsView::SetRenderTargets(const nsGALRenderTargets& renderTargets)
{
  if (m_Data.m_renderTargets != renderTargets)
  {
    // Swap chain and render target setup are mutually exclusive.
    m_Data.m_hSwapChain = nsGALSwapChainHandle();
    m_Data.m_renderTargets = renderTargets;
    if (m_pRenderPipeline)
    {
      nsRenderWorld::AddRenderPipelineToRebuild(m_pRenderPipeline, GetHandle());
    }
  }
}

const nsGALRenderTargets& nsView::GetActiveRenderTargets() const
{
  if (const nsGALSwapChain* pSwapChain = nsGALDevice::GetDefaultDevice()->GetSwapChain(m_Data.m_hSwapChain))
  {
    return pSwapChain->GetRenderTargets();
  }
  return m_Data.m_renderTargets;
}

void nsView::SetRenderPipelineResource(nsRenderPipelineResourceHandle hPipeline)
{
  if (hPipeline == m_hRenderPipeline)
  {
    return;
  }

  m_uiRenderPipelineResourceDescriptionCounter = 0;
  m_hRenderPipeline = hPipeline;

  if (m_pRenderPipeline == nullptr)
  {
    EnsureUpToDate();
  }
}

nsRenderPipelineResourceHandle nsView::GetRenderPipelineResource() const
{
  return m_hRenderPipeline;
}

void nsView::SetCameraUsageHint(nsEnum<nsCameraUsageHint> val)
{
  m_Data.m_CameraUsageHint = val;
}

void nsView::SetViewRenderMode(nsEnum<nsViewRenderMode> value)
{
  m_Data.m_ViewRenderMode = value;
}

void nsView::SetViewport(const nsRectFloat& viewport)
{
  m_Data.m_ViewPortRect = viewport;

  UpdateViewData(nsRenderWorld::GetDataIndexForExtraction());
}

void nsView::ForceUpdate()
{
  if (m_pRenderPipeline)
  {
    nsRenderWorld::AddRenderPipelineToRebuild(m_pRenderPipeline, GetHandle());
  }
}

void nsView::ExtractData()
{
  NS_ASSERT_DEV(IsValid(), "Cannot extract data from an invalid view");

  nsRenderWorldExtractionEvent extractionEvent;
  extractionEvent.m_Type = nsRenderWorldExtractionEvent::Type::BeforeViewExtraction;
  extractionEvent.m_pView = this;
  extractionEvent.m_uiFrameCounter = nsRenderWorld::GetFrameCounter();
  nsRenderWorld::s_ExtractionEvent.Broadcast(extractionEvent);


  m_pRenderPipeline->m_sName = m_sName;
  m_pRenderPipeline->ExtractData(*this);

  extractionEvent.m_Type = nsRenderWorldExtractionEvent::Type::AfterViewExtraction;
  nsRenderWorld::s_ExtractionEvent.Broadcast(extractionEvent);
}

void nsView::ComputeCullingFrustum(nsFrustum& out_frustum) const
{
  const nsCamera* pCamera = GetCullingCamera();
  const float fViewportAspectRatio = m_Data.m_ViewPortRect.width / m_Data.m_ViewPortRect.height;

  nsMat4 viewMatrix = pCamera->GetViewMatrix();

  nsMat4 projectionMatrix;
  pCamera->GetProjectionMatrix(fViewportAspectRatio, projectionMatrix);

  out_frustum = nsFrustum::MakeFromMVP(projectionMatrix * viewMatrix);
}

void nsView::SetShaderPermutationVariable(const char* szName, const char* szValue)
{
  nsHashedString sName;
  sName.Assign(szName);

  for (auto& var : m_PermutationVars)
  {
    if (var.m_sName == sName)
    {
      if (var.m_sValue != szValue)
      {
        var.m_sValue.Assign(szValue);
        m_bPermutationVarsDirty = true;
      }
      return;
    }
  }

  auto& var = m_PermutationVars.ExpandAndGetRef();
  var.m_sName = sName;
  var.m_sValue.Assign(szValue);
  m_bPermutationVarsDirty = true;
}

void nsView::SetRenderPassProperty(const char* szPassName, const char* szPropertyName, const nsVariant& value)
{
  SetProperty(m_PassProperties, szPassName, szPropertyName, value);
}

void nsView::SetExtractorProperty(const char* szPassName, const char* szPropertyName, const nsVariant& value)
{
  SetProperty(m_ExtractorProperties, szPassName, szPropertyName, value);
}

void nsView::ResetRenderPassProperties()
{
  for (auto it : m_PassProperties)
  {
    auto& prop = it.Value();
    if (prop.m_bIsValid)
    {
      prop.m_CurrentValue = prop.m_DefaultValue;
      prop.m_bIsDirty = true;
    }
  }
}

void nsView::ResetExtractorProperties()
{
  for (auto it : m_ExtractorProperties)
  {
    auto& prop = it.Value();
    if (prop.m_bIsValid)
    {
      prop.m_CurrentValue = prop.m_DefaultValue;
      prop.m_bIsDirty = true;
    }
  }
}

void nsView::SetRenderPassReadBackProperty(const char* szPassName, const char* szPropertyName, const nsVariant& value)
{
  SetReadBackProperty(m_PassReadBackProperties, szPassName, szPropertyName, value);
}

nsVariant nsView::GetRenderPassReadBackProperty(const char* szPassName, const char* szPropertyName)
{
  nsStringBuilder sKey(szPassName, "::", szPropertyName);

  auto it = m_PassReadBackProperties.Find(sKey);
  if (it.IsValid())
  {
    return it.Value().m_CurrentValue;
  }

  nsLog::Warning("Unknown read-back property '{0}::{1}'", szPassName, szPropertyName);
  return nsVariant();
}


bool nsView::IsRenderPassReadBackPropertyExisting(const char* szPassName, const char* szPropertyName) const
{
  nsStringBuilder sKey(szPassName, "::", szPropertyName);

  auto it = m_PassReadBackProperties.Find(sKey);
  return it.IsValid();
}

void nsView::UpdateViewData(nsUInt32 uiDataIndex)
{
  if (m_pRenderPipeline != nullptr)
  {
    m_pRenderPipeline->UpdateViewData(*this, uiDataIndex);
  }
}

void nsView::UpdateCachedMatrices() const
{
  const nsCamera* pCamera = GetCamera();

  bool bUpdateVP = false;

  if (m_uiLastCameraOrientationModification != pCamera->GetOrientationModificationCounter())
  {
    bUpdateVP = true;
    m_uiLastCameraOrientationModification = pCamera->GetOrientationModificationCounter();

    m_Data.m_ViewMatrix[0] = pCamera->GetViewMatrix(nsCameraEye::Left);
    m_Data.m_ViewMatrix[1] = pCamera->GetViewMatrix(nsCameraEye::Right);

    // Some of our matrices contain very small values so that the matrix inversion will fall below the default epsilon.
    // We pass zero as epsilon here since all view and projection matrices are invertible.
    m_Data.m_InverseViewMatrix[0] = m_Data.m_ViewMatrix[0].GetInverse(0.0f);
    m_Data.m_InverseViewMatrix[1] = m_Data.m_ViewMatrix[1].GetInverse(0.0f);
  }

  const float fViewportAspectRatio = m_Data.m_ViewPortRect.HasNonZeroArea() ? m_Data.m_ViewPortRect.width / m_Data.m_ViewPortRect.height : 1.0f;
  if (m_uiLastCameraSettingsModification != pCamera->GetSettingsModificationCounter() || m_fLastViewportAspectRatio != fViewportAspectRatio)
  {
    bUpdateVP = true;
    m_uiLastCameraSettingsModification = pCamera->GetSettingsModificationCounter();
    m_fLastViewportAspectRatio = fViewportAspectRatio;


    pCamera->GetProjectionMatrix(m_fLastViewportAspectRatio, m_Data.m_ProjectionMatrix[0], nsCameraEye::Left);
    m_Data.m_InverseProjectionMatrix[0] = m_Data.m_ProjectionMatrix[0].GetInverse(0.0f);

    pCamera->GetProjectionMatrix(m_fLastViewportAspectRatio, m_Data.m_ProjectionMatrix[1], nsCameraEye::Right);
    m_Data.m_InverseProjectionMatrix[1] = m_Data.m_ProjectionMatrix[1].GetInverse(0.0f);
  }

  if (bUpdateVP)
  {
    for (int i = 0; i < 2; ++i)
    {
      m_Data.m_ViewProjectionMatrix[i] = m_Data.m_ProjectionMatrix[i] * m_Data.m_ViewMatrix[i];
      m_Data.m_InverseViewProjectionMatrix[i] = m_Data.m_ViewProjectionMatrix[i].GetInverse(0.0f);
    }
  }
}

void nsView::EnsureUpToDate()
{
  if (m_hRenderPipeline.IsValid())
  {
    nsResourceLock<nsRenderPipelineResource> pPipeline(m_hRenderPipeline, nsResourceAcquireMode::BlockTillLoaded);

    nsUInt32 uiCounter = pPipeline->GetCurrentResourceChangeCounter();

    if (m_uiRenderPipelineResourceDescriptionCounter != uiCounter)
    {
      m_uiRenderPipelineResourceDescriptionCounter = uiCounter;

      m_pRenderPipeline = pPipeline->CreateRenderPipeline();
      nsRenderWorld::AddRenderPipelineToRebuild(m_pRenderPipeline, GetHandle());

      m_bPermutationVarsDirty = true;
      ResetAllPropertyStates(m_PassProperties);
      ResetAllPropertyStates(m_ExtractorProperties);
    }

    ApplyPermutationVars();
    ApplyRenderPassProperties();
    ApplyExtractorProperties();
  }
}

void nsView::ApplyPermutationVars()
{
  if (!m_bPermutationVarsDirty)
    return;

  m_pRenderPipeline->m_PermutationVars = m_PermutationVars;
  m_bPermutationVarsDirty = false;
}

void nsView::SetProperty(nsMap<nsString, PropertyValue>& map, const char* szPassName, const char* szPropertyName, const nsVariant& value)
{
  nsStringBuilder sKey(szPassName, "::", szPropertyName);

  bool bExisted = false;
  auto& prop = map.FindOrAdd(sKey, &bExisted).Value();

  if (!bExisted)
  {
    prop.m_sObjectName = szPassName;
    prop.m_sPropertyName = szPropertyName;
    prop.m_bIsValid = true;
  }

  prop.m_bIsDirty = true;
  prop.m_CurrentValue = value;
}


void nsView::SetReadBackProperty(nsMap<nsString, PropertyValue>& map, const char* szPassName, const char* szPropertyName, const nsVariant& value)
{
  nsStringBuilder sKey(szPassName, "::", szPropertyName);

  bool bExisted = false;
  auto& prop = map.FindOrAdd(sKey, &bExisted).Value();

  if (!bExisted)
  {
    prop.m_sObjectName = szPassName;
    prop.m_sPropertyName = szPropertyName;
    prop.m_bIsValid = true;
  }

  prop.m_bIsDirty = false;
  prop.m_CurrentValue = value;
}

void nsView::ReadBackPassProperties()
{
  nsHybridArray<nsRenderPipelinePass*, 16> passes;
  m_pRenderPipeline->GetPasses(passes);

  for (auto pPass : passes)
  {
    pPass->ReadBackProperties(this);
  }
}

void nsView::ResetAllPropertyStates(nsMap<nsString, PropertyValue>& map)
{
  for (auto it = map.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_bIsDirty = true;
    it.Value().m_bIsValid = true;
  }
}

void nsView::ApplyRenderPassProperties()
{
  for (auto it = m_PassProperties.GetIterator(); it.IsValid(); ++it)
  {
    auto& propertyValue = it.Value();

    if (!propertyValue.m_bIsValid || !propertyValue.m_bIsDirty)
      continue;

    propertyValue.m_bIsDirty = false;

    nsReflectedClass* pObject = nullptr;
    const char* szDot = propertyValue.m_sObjectName.FindSubString(".");
    if (szDot != nullptr)
    {
      NS_REPORT_FAILURE("Setting renderer properties is not possible anymore");
    }
    else
    {
      pObject = m_pRenderPipeline->GetPassByName(propertyValue.m_sObjectName);
    }

    if (pObject == nullptr)
    {
      nsLog::Error("The render pass '{0}' does not exist. Property '{1}' cannot be applied.", propertyValue.m_sObjectName, propertyValue.m_sPropertyName);

      propertyValue.m_bIsValid = false;
      continue;
    }

    ApplyProperty(pObject, propertyValue, "render pass");
  }
}

void nsView::ApplyExtractorProperties()
{
  for (auto it = m_ExtractorProperties.GetIterator(); it.IsValid(); ++it)
  {
    if (!it.Value().m_bIsValid || !it.Value().m_bIsDirty)
      continue;

    it.Value().m_bIsDirty = false;

    nsExtractor* pExtractor = m_pRenderPipeline->GetExtractorByName(it.Value().m_sObjectName);
    if (pExtractor == nullptr)
    {
      nsLog::Error("The extractor '{0}' does not exist. Property '{1}' cannot be applied.", it.Value().m_sObjectName, it.Value().m_sPropertyName);

      it.Value().m_bIsValid = false;
      continue;
    }

    ApplyProperty(pExtractor, it.Value(), "extractor");
  }
}

void nsView::ApplyProperty(nsReflectedClass* pObject, PropertyValue& data, const char* szTypeName)
{
  const nsAbstractProperty* pAbstractProperty = pObject->GetDynamicRTTI()->FindPropertyByName(data.m_sPropertyName);
  if (pAbstractProperty == nullptr)
  {
    nsLog::Error("The {0} '{1}' does not have a property called '{2}', it cannot be applied.", szTypeName, data.m_sObjectName, data.m_sPropertyName);

    data.m_bIsValid = false;
    return;
  }

  if (pAbstractProperty->GetCategory() != nsPropertyCategory::Member)
  {
    nsLog::Error("The {0} property '{1}::{2}' is not a member property, it cannot be applied.", szTypeName, data.m_sObjectName, data.m_sPropertyName);

    data.m_bIsValid = false;
    return;
  }

  auto pMemberProperty = static_cast<const nsAbstractMemberProperty*>(pAbstractProperty);
  if (data.m_DefaultValue.IsValid() == false)
  {
    data.m_DefaultValue = nsReflectionUtils::GetMemberPropertyValue(pMemberProperty, pObject);
  }

  nsReflectionUtils::SetMemberPropertyValue(pMemberProperty, pObject, data.m_CurrentValue);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_View);
