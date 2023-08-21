#include <CppProjectPlugin/CppProjectPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <CppProjectPlugin/GameState/CppProjectGameState.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>

wdCVarBool cvar_DebugDisplay("CppProject.DebugDisplay", false, wdCVarFlags::Default, "Whether the game should display debug geometry.");

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(CppProjectGameState, 1, wdRTTIDefaultAllocator<CppProjectGameState>)
WD_END_DYNAMIC_REFLECTED_TYPE;

CppProjectGameState::CppProjectGameState() = default;
CppProjectGameState::~CppProjectGameState() = default;

void CppProjectGameState::OnActivation(wdWorld* pWorld, const wdTransform* pStartPosition)
{
  WD_LOG_BLOCK("GameState::Activate");

  SUPER::OnActivation(pWorld, pStartPosition);
}

void CppProjectGameState::OnDeactivation()
{
  WD_LOG_BLOCK("GameState::Deactivate");

  SUPER::OnDeactivation();
}

void CppProjectGameState::AfterWorldUpdate()
{
  SUPER::AfterWorldUpdate();

  if (cvar_DebugDisplay)
  {
    wdDebugRenderer::DrawLineSphere(m_pMainWorld, wdBoundingSphere(wdVec3::ZeroVector(), 1.0f), wdColor::Orange);
  }

  wdDebugRenderer::Draw2DText(m_pMainWorld, "Press 'O' to spawn objects", wdVec2I32(10, 10), wdColor::White);
  wdDebugRenderer::Draw2DText(m_pMainWorld, "Press 'P' to remove objects", wdVec2I32(10, 30), wdColor::White);
}

void CppProjectGameState::BeforeWorldUpdate()
{
  WD_LOCK(m_pMainWorld->GetWriteMarker());
}

wdGameStatePriority CppProjectGameState::DeterminePriority(wdWorld* pWorld) const
{
  return wdGameStatePriority::Default;
}

void CppProjectGameState::ConfigureMainWindowInputDevices(wdWindow* pWindow)
{
  SUPER::ConfigureMainWindowInputDevices(pWindow);

  // setup devices here
}

static void RegisterInputAction(const char* szInputSet, const char* szInputAction, const char* szKey1, const char* szKey2 = nullptr, const char* szKey3 = nullptr)
{
  wdInputActionConfig cfg;
  cfg.m_bApplyTimeScaling = true;
  cfg.m_sInputSlotTrigger[0] = szKey1;
  cfg.m_sInputSlotTrigger[1] = szKey2;
  cfg.m_sInputSlotTrigger[2] = szKey3;

  wdInputManager::SetInputActionConfig(szInputSet, szInputAction, cfg, true);
}

void CppProjectGameState::ConfigureInputActions()
{
  SUPER::ConfigureInputActions();

  RegisterInputAction("CppProjectPlugin", "SpawnObject", wdInputSlot_KeyO, wdInputSlot_Controller0_ButtonA, wdInputSlot_MouseButton2);
  RegisterInputAction("CppProjectPlugin", "DeleteObject", wdInputSlot_KeyP, wdInputSlot_Controller0_ButtonB);
}

void CppProjectGameState::ProcessInput()
{
  SUPER::ProcessInput();

  wdWorld* pWorld = m_pMainWorld;

  if (wdInputManager::GetInputActionState("CppProjectPlugin", "SpawnObject") == wdKeyState::Pressed)
  {
    const wdVec3 pos = GetMainCamera()->GetCenterPosition() + GetMainCamera()->GetCenterDirForwards();

    // make sure we are allowed to modify the world
    WD_LOCK(pWorld->GetWriteMarker());

    // create a game object at the desired position
    wdGameObjectDesc desc;
    desc.m_LocalPosition = pos;

    wdGameObject* pObject = nullptr;
    wdGameObjectHandle hObject = pWorld->CreateObject(desc, pObject);

    m_SpawnedObjects.PushBack(hObject);

    // attach a mesh component to the object
    wdMeshComponent* pMesh;
    pWorld->GetOrCreateComponentManager<wdMeshComponentManager>()->CreateComponent(pObject, pMesh);

    // Set the mesh to use.
    // Here we use a path relative to the project directory.
    // We have to reference the 'transformed' file, not the source file.
    // This would break if the source asset is moved or renamed.
    pMesh->SetMeshFile("AssetCache/Common/Meshes/Sphere.wdMesh");

    // here we use the asset GUID to reference the transformed asset
    // we can copy the GUID from the asset browser
    // the GUID is stable even if the source asset gets moved or renamed
    // using asset collections we could also give a nice name like 'Blue Material' to this asset
    wdMaterialResourceHandle hMaterial = wdResourceManager::LoadResource<wdMaterialResource>("{ aa1c5601-bc43-fbf8-4e07-6a3df3af51e7 }");

    // override the mesh material in the first slot with something different
    pMesh->SetMaterial(0, hMaterial);
  }

  if (wdInputManager::GetInputActionState("CppProjectPlugin", "DeleteObject") == wdKeyState::Pressed)
  {
    if (!m_SpawnedObjects.IsEmpty())
    {
      // make sure we are allowed to modify the world
      WD_LOCK(pWorld->GetWriteMarker());

      wdGameObjectHandle hObject = m_SpawnedObjects.PeekBack();
      m_SpawnedObjects.PopBack();

      // this is only for demonstration purposes, removing the object will delete all attached components as well
      wdGameObject* pObject = nullptr;
      if (pWorld->TryGetObject(hObject, pObject))
      {
        wdMeshComponent* pMesh = nullptr;
        if (pObject->TryGetComponentOfBaseType(pMesh))
        {
          pMesh->DeleteComponent();
        }
      }

      // delete the object, all its children and attached components
      pWorld->DeleteObjectDelayed(hObject);
    }
  }
}

void CppProjectGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
