/*
 *   Copyright (c) 2024-present Mikael K. Aboagye & WD Studios L.L.C.
 *   All rights reserved.
 *   This Project & Code is Licensed under the MIT License.
 */
#pragma once
#include <InspectorPlugin/InspectorPluginDLL.h>
#include <Jolt/Jolt.h>

namespace JPH
{
  class BodyInterface;
  class BodyManager;
  class PhysicsSystem;
} // namespace JPH

namespace JDebug::API
{
  /**
   * @class JPHDebuggerInterface
   * @brief Interface for the JDebugger to interact with the Jolt Physics System.
   */
  class NS_INSPECTORPLUGIN_DLL JPHDebuggerInterface
  {
  public:
    /**
     * @enum JDInstructionLevel
     * @brief Instruction levels for network communication.
     */
    enum class JDInstructionLevel : nsUInt8
    {
      JDIL_All = 0x001,      // Network Everything across the server, including body states, positions, velocities, etc.
      JDIL_Line = 0x002,     // Network only the positions of the body, and the body states.
      JDIL_Function = 0x003, // Network only the body states.
    };

  public:
    /**
     * @brief Default constructor.
     */
    JPHDebuggerInterface() = default;

    /**
     * @brief Constructor.
     * @param in_physicssystem The Jolt Physics System.
     * @param in_manager The Body Manager.
     */
    explicit JPHDebuggerInterface(const JPH::PhysicsSystem& in_physicssystem, const JPH::BodyManager* in_manager = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~JPHDebuggerInterface();

    /**
     * @brief Set the Body Interface.
     * @param in_interface The Body Interface.
     */
    void SetBodyInterface(const JPH::BodyInterface& in_interface);

    /**
     * @brief Set the Body Manager.
     * @param in_manager The Body Manager.
     */
    void SetBodyManager(const JPH::BodyManager& in_manager);

    /**
     * @brief Set the network connection link.
     * @param in_link The network connection link.
     */
    void SetNetworkConnectionLink(const std::string& in_link);

    /**
     * @brief Set the instruction level for network communication.
     * @param in_level The instruction level.
     */
    void SetInstructionLevel(JDInstructionLevel in_level);

    /**
     * @brief This function is called when the JDebugger disconnects.
     *
     * This function is a pure virtual function and must be implemented by derived classes.
     * It is called when the JDebugger disconnects from the debugger interface.
     *
     * @note This function is meant to be overridden by derived classes.
     */
    virtual void OnJDebuggerDisconnect() = 0;

    /**
     * @brief This function is called when the JDebugger connects.
     *
     * This function is a pure virtual function and must be implemented by derived classes.
     * It is called when the JDebugger connects to the debugger interface.
     *
     * @note This function is meant to be overridden by derived classes.
     */
    virtual void OnJDebuggerConnect() = 0;

    /**
     * @brief Pre-update function.
     *
     * This function is a pure virtual function and must be implemented by derived classes.
     * It is called before the physics system is updated.
     *
     * @note This function is meant to be overridden by derived classes.
     */
    virtual void PreUpdate() = 0;

    /**
     * @brief Post-update function.
     *
     * This function is a pure virtual function and must be implemented by derived classes.
     * It is called after the physics system is updated.
     *
     * @note This function is meant to be overridden by derived classes.
     */
    virtual void PostUpdate() = 0;

    /**
     * @brief Pre-frame end function.
     *
     * This function is a pure virtual function and must be implemented by derived classes.
     * It is called before the frame ends.
     *
     * I Recommend to use this function to send the data to the JDebugger, clean up any custom bodies, etc.
     *
     * @note This function is meant to be overridden by derived classes.
     */
    virtual void PreFrameEnd() = 0;

    /**
     * @brief Pre-frame start function.
     *
     * This function is a pure virtual function and must be implemented by derived classes.
     * It is called before the frame starts.
     *
     * Just like PreFrameEnd(), I Recommend to use this function to send the data to the JDebugger, clean up any custom bodies, etc.
     *
     * @note This function is meant to be overridden by derived classes.
     */
    virtual void PreFrameStart() = 0;

  public:
    void FrameEnd();
    void FrameStart();

  private:
    JPH::BodyInterface* m_pInterface;       ///< The body interface.
    JPH::BodyManager* m_pManager = nullptr; ///< The body manager. This can be null, we will just replace those calls with PhysicsSystem calls.
    JPH::PhysicsSystem* m_pPhysicsSystem;   ///< The Jolt Physics System.
    std::string m_sNetworkConnectionLink;   ///< The network connection link.
    JDInstructionLevel m_eInstructionLevel; ///< The instruction level for network communication.
  };
} // namespace JDebug::API