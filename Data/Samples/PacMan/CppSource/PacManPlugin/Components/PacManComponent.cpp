#include <PacManPlugin/PacManPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <PacManPlugin/Components/PacManComponent.h>

// clang-format off
WD_BEGIN_COMPONENT_TYPE(PacManComponent, 1 /* version */, wdComponentMode::Dynamic) // 'Dynamic' because we want to change the owner's transform
{
  // if we wanted to show properties in the editor, we would need to register them here
  //WD_BEGIN_PROPERTIES
  //{
  //  WD_MEMBER_PROPERTY("Amplitude", m_fAmplitude)->AddAttributes(new wdDefaultValueAttribute(1), new wdClampValueAttribute(0, 10)),
  //}
  //WD_END_PROPERTIES;

  WD_BEGIN_ATTRIBUTES
  {
    new wdCategoryAttribute("PacMan"), // Component menu group
  }
  WD_END_ATTRIBUTES;

  // declare the message handlers that we have, so that messages can be delivered to us
  WD_BEGIN_MESSAGEHANDLERS
  {
    WD_MESSAGE_HANDLER(wdMsgInputActionTriggered, OnMsgInputActionTriggered),
    WD_MESSAGE_HANDLER(wdMsgTriggerTriggered, OnMsgTriggerTriggered),
  }
  WD_END_MESSAGEHANDLERS;
}
WD_END_COMPONENT_TYPE
// clang-format on

PacManComponent::PacManComponent() = default;
PacManComponent::~PacManComponent() = default;

void PacManComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  // preload prefabs needed later
  {
    m_hCollectCoinEffect = wdResourceManager::LoadResource<wdPrefabResource>("{ 9b006872-70ba-4086-8ce5-244304032851 }"); // GUID of the prefab copied in the editor
    wdResourceManager::PreloadResource(m_hCollectCoinEffect);

    m_hLoseGameEffect = wdResourceManager::LoadResource<wdPrefabResource>("{ 02314d71-f49e-45f3-89e2-ce4b7b1cba09 }"); // GUID copied in the editor
    wdResourceManager::PreloadResource(m_hLoseGameEffect);
  }

  wdHashedString hs;
  hs.Assign("Stats");
  m_pStateBlackboard = wdBlackboard::GetOrCreateGlobal(hs);

  // store the start state of PacMan in the global blackboard
  hs.Assign("PacManState");
  m_pStateBlackboard->RegisterEntry(hs, PacManState::Alive);
  m_pStateBlackboard->SetEntryValue(hs, PacManState::Alive).AssertSuccess();

  hs.Assign("CoinsEaten");
  m_pStateBlackboard->RegisterEntry(hs, 0);
  m_pStateBlackboard->SetEntryValue(hs, 0).AssertSuccess();
}

void PacManComponent::Update()
{
  // this function is called once per frame

  if (auto pBlackboard = wdBlackboard::FindGlobal(wdTempHashedString("Stats")))
  {
    // retrieve the current state of PacMan
    const PacManState state = static_cast<PacManState>(pBlackboard->GetEntryValue(wdTempHashedString("PacManState"), PacManState::Alive).Get<wdInt32>());

    // if it was eaten by a ghost recently, play the lose effect and delete PacMan
    if (state == PacManState::EatenByGhost)
    {
      // access the lose game effect
      // the prefab may still be loading (though very unlikely)
      // only if it is fully loaded, do we instantiate it, in all other cases we just ignore it
      wdPrefabResource::InstantiatePrefab(m_hLoseGameEffect, true, *GetWorld(), GetOwner()->GetGlobalTransform());

      // since we lost, just delete the PacMan object altogether at the end of the frame
      GetWorld()->DeleteObjectDelayed(GetOwner()->GetHandle());

      return;
    }
  }

  // if PacMan is still alive, update his movement

  bool bWall[4] = {false, false, false, false};

  if (wdPhysicsWorldModuleInterface* pPhysics = GetWorld()->GetOrCreateModule<wdPhysicsWorldModuleInterface>())
  {
    // access the physics system for raycasts

    wdPhysicsCastResult res;
    wdPhysicsQueryParameters params;
    params.m_ShapeTypes = wdPhysicsShapeType::Static; // we only want to hit static geometry, and ignore dynamic/kinematic objects like the other ghosts and the coins

    wdVec3 pos = GetOwner()->GetGlobalPosition();
    pos.z += 0.5f;
    const float dist = 1.0f;
    const float off = 0.35f;
    const wdVec3 offx(off, 0.0f, 0.0f);
    const wdVec3 offy(0.0f, off, 0.0f);

    // do two raycasts into each of the 4 directions, and use a slight offset
    // this way, if both raycasts into one direction hit nothing, we know that there is enough free space, that we can now change direction
    // if only one raycast hits nothing, we need to travel further, otherwise we bump into a corner

    //    ^ ^
    //    | |
    // <--+ +-->
    //     P
    // <--+ +-->
    //    | |
    //    V V

    bWall[0] |= pPhysics->Raycast(res, pos - offy, wdVec3(1, 0, 0), dist, params);
    bWall[0] |= pPhysics->Raycast(res, pos + offy, wdVec3(1, 0, 0), dist, params);

    bWall[1] |= pPhysics->Raycast(res, pos - offx, wdVec3(0, 1, 0), dist, params);
    bWall[1] |= pPhysics->Raycast(res, pos + offx, wdVec3(0, 1, 0), dist, params);

    bWall[2] |= pPhysics->Raycast(res, pos - offy, wdVec3(-1, 0, 0), dist, params);
    bWall[2] |= pPhysics->Raycast(res, pos + offy, wdVec3(-1, 0, 0), dist, params);

    bWall[3] |= pPhysics->Raycast(res, pos - offx, wdVec3(0, -1, 0), dist, params);
    bWall[3] |= pPhysics->Raycast(res, pos + offx, wdVec3(0, -1, 0), dist, params);
  }

  // if there is no wall in the direction that the player wants PacMan to go, we can safely switch direction now
  // this makes PacMan much easier to steer around the maze, than if we were to change directly immediately
  // since it prevents getting stuck on some corner
  if (!bWall[m_TargetDirection])
  {
    m_Direction = m_TargetDirection;
  }

  // now just change the rotation of PacMan to point into the current direction
  wdQuat rotation;
  rotation.SetFromAxisAndAngle(wdVec3::UnitZAxis(), wdAngle::Degree(m_Direction * 90));
  GetOwner()->SetGlobalRotation(rotation);

  // and communicate to the character controller component, that it should move forwards at a fixed speed
  wdMsgMoveCharacterController msg;
  msg.m_fMoveForwards = 2.0f;
  GetOwner()->SendMessage(msg);
}

void PacManComponent::OnMsgInputActionTriggered(wdMsgInputActionTriggered& msg)
{
  // this is called every time the input component detects that there is relevant input state
  // see https://wdengine.net/pages/docs/input/input-component.html

  if (msg.m_TriggerState != wdTriggerState::Continuing)
    return;

  if (msg.m_sInputAction.GetString() == "Up")
    m_TargetDirection = WalkDirection::Up;

  if (msg.m_sInputAction.GetString() == "Down")
    m_TargetDirection = WalkDirection::Down;

  if (msg.m_sInputAction.GetString() == "Left")
    m_TargetDirection = WalkDirection::Left;

  if (msg.m_sInputAction.GetString() == "Right")
    m_TargetDirection = WalkDirection::Right;
}

void PacManComponent::OnMsgTriggerTriggered(wdMsgTriggerTriggered& msg)
{
  // this is called every time a trigger component (on this object) detects overlap with another physics object
  // see https://wdengine.net/pages/docs/physics/jolt/actors/jolt-trigger-component.html
  // we have two triggers on our PacMan, one to detect Ghosts, another to detect Coins
  // we could achieve the same thing with just one trigger, though

  if (msg.m_TriggerState != wdTriggerState::Activated)
    return;

  // the "Ghost" trigger had an overlap
  if (msg.m_sMessage.GetString() == "Ghost")
  {
    m_pStateBlackboard->SetEntryValue(wdTempHashedString("PacManState"), PacManState::EatenByGhost).AssertSuccess();
    return;
  }

  // the "Pickup" trigger had an overlap
  if (msg.m_sMessage.GetString() == "Pickup")
  {
    // use the handle to the game object that activated the trigger, to get a pointer to the game object
    wdGameObject* pObject = nullptr;
    if (GetWorld()->TryGetObject(msg.m_hTriggeringObject, pObject))
    {
      // if this was a coin, pick it up
      if (pObject->GetName() == "Coin")
      {
        // just delete the coin object at the end of the frame
        GetWorld()->DeleteObjectDelayed(pObject->GetHandle());

        // we register "CoinsEaten" at the very beginning, so reading and writing this value should always work here
        const wdTempHashedString ceName("CoinsEaten");
        wdVariant ceValue = m_pStateBlackboard->GetEntryValue(ceName, 0).Get<wdInt32>() + 1;
        m_pStateBlackboard->SetEntryValue(ceName, ceValue).AssertSuccess();

        // spawn a collect coin effect
        wdPrefabResource::InstantiatePrefab(m_hCollectCoinEffect, false, *GetWorld(), pObject->GetGlobalTransform());
      }
    }
  }
}

void PacManComponent::SerializeComponent(wdWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  // currently we have nothing to serialize
}

void PacManComponent::DeserializeComponent(wdWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const wdUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  // currently we have nothing to deserialize
}
