/*
 *   Copyright (c) 2023 WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <AniDriveProtoPlugin/AniDriveProtoPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Core/System/Window.h>
#include <Core/World/World.h>
#include <AniDriveProtoPlugin/GameState/AniDriveProtoGameState.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Logging/Log.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponent.h>

nsCVarBool cvar_DebugDisplay("AniDriveProto.DebugDisplay", false, nsCVarFlags::Default, "Whether the game should display debug geometry.");

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(AniDriveProtoGameState, 1, nsRTTIDefaultAllocator<AniDriveProtoGameState>)
NS_END_DYNAMIC_REFLECTED_TYPE;

AniDriveProtoGameState::AniDriveProtoGameState() = default;
AniDriveProtoGameState::~AniDriveProtoGameState() = default;

void AniDriveProtoGameState::OnActivation(nsWorld* pWorld, const nsTransform* pStartPosition)
{
  NS_LOG_BLOCK("GameState::Activate");

  SUPER::OnActivation(pWorld, pStartPosition);
}

void AniDriveProtoGameState::OnDeactivation()
{
  NS_LOG_BLOCK("GameState::Deactivate");

  SUPER::OnDeactivation();
}

void AniDriveProtoGameState::AfterWorldUpdate()
{
  SUPER::AfterWorldUpdate();

  if (cvar_DebugDisplay)
  {
    nsDebugRenderer::DrawLineSphere(m_pMainWorld, nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), 1.0f), nsColor::Orange);
  }

  nsDebugRenderer::Draw2DText(m_pMainWorld, "Press 'O' to spawn objects", nsVec2I32(10, 10), nsColor::White);
  nsDebugRenderer::Draw2DText(m_pMainWorld, "Press 'P' to remove objects", nsVec2I32(10, 30), nsColor::White);
}

void AniDriveProtoGameState::BeforeWorldUpdate()
{
  NS_LOCK(m_pMainWorld->GetWriteMarker());
}

nsGameStatePriority AniDriveProtoGameState::DeterminePriority(nsWorld* pWorld) const
{
  return nsGameStatePriority::Default;
}

void AniDriveProtoGameState::ConfigureMainWindowInputDevices(nsWindow* pWindow)
{
  SUPER::ConfigureMainWindowInputDevices(pWindow);

  // setup devices here
}

static void RegisterInputAction(const char* szInputSet, const char* szInputAction, const char* szKey1, const char* szKey2 = nullptr, const char* szKey3 = nullptr)
{
  nsInputActionConfig cfg;
  cfg.m_bApplyTimeScaling = true;
  cfg.m_sInputSlotTrigger[0] = szKey1;
  cfg.m_sInputSlotTrigger[1] = szKey2;
  cfg.m_sInputSlotTrigger[2] = szKey3;

  nsInputManager::SetInputActionConfig(szInputSet, szInputAction, cfg, true);
}

void AniDriveProtoGameState::ConfigureInputActions()
{
  SUPER::ConfigureInputActions();

  RegisterInputAction("AniDriveProtoPlugin", "SpawnObject", nsInputSlot_KeyO, nsInputSlot_Controller0_ButtonA, nsInputSlot_MouseButton2);
  RegisterInputAction("AniDriveProtoPlugin", "DeleteObject", nsInputSlot_KeyP, nsInputSlot_Controller0_ButtonB);
}

void AniDriveProtoGameState::ProcessInput()
{
  SUPER::ProcessInput();

  nsWorld* pWorld = m_pMainWorld;

  if (nsInputManager::GetInputActionState("AniDriveProtoPlugin", "SpawnObject") == nsKeyState::Pressed)
  {
    const nsVec3 pos = GetMainCamera()->GetCenterPosition() + GetMainCamera()->GetCenterDirForwards();

    // make sure we are allowed to modify the world
    NS_LOCK(pWorld->GetWriteMarker());

    // create a game object at the desired position
    nsGameObjectDesc desc;
    desc.m_LocalPosition = pos;

    nsGameObject* pObject = nullptr;
    nsGameObjectHandle hObject = pWorld->CreateObject(desc, pObject);

    m_SpawnedObjects.PushBack(hObject);

    // attach a mesh component to the object
    nsMeshComponent* pMesh;
    pWorld->GetOrCreateComponentManager<nsMeshComponentManager>()->CreateComponent(pObject, pMesh);

    // Set the mesh to use.
    // Here we use a path relative to the project directory.
    // We have to reference the 'transformed' file, not the source file.
    // This would break if the source asset is moved or renamed.
    pMesh->SetMeshFile("AssetCache/Common/Meshes/Sphere.nsMesh");

    // here we use the asset GUID to reference the transformed asset
    // we can copy the GUID from the asset browser
    // the GUID is stable even if the source asset gets moved or renamed
    // using asset collections we could also give a nice name like 'Blue Material' to this asset
    nsMaterialResourceHandle hMaterial = nsResourceManager::LoadResource<nsMaterialResource>("{ aa1c5601-bc43-fbf8-4e07-6a3df3af51e7 }");

    // override the mesh material in the first slot with something different
    pMesh->SetMaterial(0, hMaterial);
  }

  if (nsInputManager::GetInputActionState("AniDriveProtoPlugin", "DeleteObject") == nsKeyState::Pressed)
  {
    if (!m_SpawnedObjects.IsEmpty())
    {
      // make sure we are allowed to modify the world
      NS_LOCK(pWorld->GetWriteMarker());

      nsGameObjectHandle hObject = m_SpawnedObjects.PeekBack();
      m_SpawnedObjects.PopBack();

      // this is only for demonstration purposes, removing the object will delete all attached components as well
      nsGameObject* pObject = nullptr;
      if (pWorld->TryGetObject(hObject, pObject))
      {
        nsMeshComponent* pMesh = nullptr;
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

void AniDriveProtoGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
