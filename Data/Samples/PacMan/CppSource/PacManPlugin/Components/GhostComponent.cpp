#include <PacManPlugin/PacManPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <PacManPlugin/Components/GhostComponent.h>

// clang-format off
WD_BEGIN_COMPONENT_TYPE(GhostComponent, 1 /* version */, wdComponentMode::Dynamic) // 'Dynamic' because we want to change the owner's transform
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Speed", m_fSpeed)->AddAttributes(new wdDefaultValueAttribute(2.0f), new wdClampValueAttribute(0.1f, 10.0f)),
  }
  WD_END_PROPERTIES;

  WD_BEGIN_ATTRIBUTES
  {
    new wdCategoryAttribute("PacMan"), // Component menu group
  }
  WD_END_ATTRIBUTES;
}
WD_END_COMPONENT_TYPE
// clang-format on

GhostComponent::GhostComponent() = default;
GhostComponent::~GhostComponent() = default;

void GhostComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  wdHashedString hs;
  hs.Assign("Stats");
  m_pStateBlackboard = wdBlackboard::GetOrCreateGlobal(hs);

  // preload our disappear effect for when the player wins
  m_hDisappear = wdResourceManager::LoadResource<wdPrefabResource>("{ bad55bab-9701-484c-b3f2-90caeb206716 }");
  wdResourceManager::PreloadResource(m_hDisappear);
}

void GhostComponent::Update()
{
  // check the blackboard for whether the player just won
  {
    const PacManState state = static_cast<PacManState>(m_pStateBlackboard->GetEntryValue(wdTempHashedString("PacManState"), PacManState::Alive).Get<wdInt32>());

    if (state == PacManState::WonGame)
    {
      // create the 'disappear' effect
      wdPrefabResource::InstantiatePrefab(m_hDisappear, true, *GetWorld(), GetOwner()->GetGlobalTransform());

      // and delete yourself at the end of the frame
      GetWorld()->DeleteObjectDelayed(GetOwner()->GetHandle());
      return;
    }
  }

  bool bWall[4] = {false, false, false, false};

  if (wdPhysicsWorldModuleInterface* pPhysics = GetWorld()->GetOrCreateModule<wdPhysicsWorldModuleInterface>())
  {
    // do four raycasts into each direction, to detect which direction would be free to walk into
    // if the forwards direction is blocked, we want the ghost to turn

    wdPhysicsCastResult res;
    wdPhysicsQueryParameters params;
    params.m_ShapeTypes = wdPhysicsShapeType::Static;

    wdVec3 pos = GetOwner()->GetGlobalPosition();
    pos.z += 0.5f;

    wdVec3 dir[4] =
      {
        wdVec3(1, 0, 0),
        wdVec3(0, 1, 0),
        wdVec3(-1, 0, 0),
        wdVec3(0, -1, 0),
      };

    wdHybridArray<wdDebugRenderer::Line, 4> lines;

    for (wdUInt32 i = 0; i < 4; ++i)
    {
      bWall[i] = pPhysics->Raycast(res, pos, dir[i], 0.55f, params);

      auto& l = lines.ExpandAndGetRef();
      l.m_start = pos;
      l.m_end = pos + dir[i] * 0.55f;
      l.m_startColor = l.m_endColor = bWall[i] ? wdColor::Red : wdColor::Green;
    }

    // could be used to visualize the raycasts
    // wdDebugRenderer::DrawLines(GetWorld(), lines, wdColor::White);
  }

  wdRandom& rng = GetWorld()->GetRandomNumberGenerator();

  // if the direction into which the ghost currently walks is occluded, randomly turn left or right and check again
  while (bWall[m_Direction])
  {
    wdUInt8 uiDir = (wdUInt8)m_Direction;

    if (bWall[(uiDir + 1) % 4] && bWall[(uiDir + 3) % 4]) // both left and right are blocked -> turn around
    {
      uiDir = (uiDir + 2) % 4;
    }
    else if (bWall[(uiDir + 1) % 4]) // right side is blocked -> turn left
    {
      uiDir = (uiDir + 3) % 4;
    }
    else if (bWall[(uiDir + 3) % 4]) // left side is blocked -> turn right
    {
      uiDir = (uiDir + 1) % 4;
    }
    else
    {
      // otherwise randomly turn left or right

      if (rng.Bool())
        uiDir = (uiDir + 1) % 4;
      else
        uiDir = (uiDir + 3) % 4;
    }

    m_Direction = static_cast<WalkDirection>(uiDir);
  }

  // now just change the rotation of the ghost to point into the current direction
  wdQuat rotation;
  rotation.SetFromAxisAndAngle(wdVec3::UnitZAxis(), wdAngle::Degree(m_Direction * 90));
  GetOwner()->SetGlobalRotation(rotation);

  // and communicate to the character controller component, that it should move forwards at a fixed speed
  wdMsgMoveCharacterController msg;
  msg.m_fMoveForwards = m_fSpeed;
  GetOwner()->SendMessage(msg);
}

void GhostComponent::SerializeComponent(wdWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  if (OWNTYPE::GetStaticRTTI()->GetTypeVersion() == 1)
  {
    wdReflectionSerializer::WriteObjectToBinary(s, GetDynamicRTTI(), this);
  }
  else
  {
    // do custom serialization
  }
}

void GhostComponent::DeserializeComponent(wdWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const wdUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  if (uiVersion == 1)
  {
    wdReflectionSerializer::ReadObjectPropertiesFromBinary(s, *GetDynamicRTTI(), this);
  }
  else
  {
    // do custom serialization
  }
}
