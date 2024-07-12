#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Strings/String.h>

class NS_FOUNDATION_DLL nsDataTransfer;

/// \brief A small wrapper class around an nsTelemetryMessage for sending a 'data transfer'. See nsDataTransfer for more details.
class NS_FOUNDATION_DLL nsDataTransferObject
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsDataTransferObject);

public:
  /// \brief nsDataTransferObject instances should always be created on the stack and should be very short lived.
  ///
  /// \param BelongsTo
  ///   The nsDataTransfer through which the data is going to be sent shortly.
  /// \param szObjectName
  ///   The name of the data blob. Since several data transfers can be made through the same nsDataTransfer, this is used to identify
  ///   the different pieces of information. For example when sending each texture of a G-Buffer, szObjectName could be 'diffuse' when
  ///   sending the diffuse channel, 'depth' for the depth buffer content, etc.
  /// \param szMimeType
  ///   The mime type for the data blob. Tools such as nsInspector can use this information to determine whether they can display it
  ///   directly or in some other editor.
  /// \param szFileExtension
  ///   The file extension that should be used when the user wants to store the data on disk. This is separate from the mime type,
  ///   as you might want to send data with mime type 'text/json' but store it on disk as '.savegame'.
  nsDataTransferObject(nsDataTransfer& ref_belongsTo, nsStringView sObjectName, nsStringView sMimeType, nsStringView sFileExtension);

  /// \brief The destructor will assert if the data has not been transmitted.
  ~nsDataTransferObject();

  /// \brief Returns the stream writer that you need to use to write the data into the object.
  ///
  /// When finished writing all data to the object, you should call Transmit().
  nsStreamWriter& GetWriter() { return m_Msg.GetWriter(); }

  /// \brief Will initiate the data transfer.
  void Transmit();

private:
  friend class nsDataTransfer;

  bool m_bHasBeenTransferred;
  nsDataTransfer& m_BelongsTo;
  nsTelemetryMessage m_Msg;
};

/// \brief A 'data transfer' is a blob of data that an application can send to connected tools such as nsInspector upon request.
///
/// Data transfers can be used to allow an application to send large amounts of data to tools such as nsInspector, which can then
/// display or process them somehow. An example usage is to send a screenshot or the different textures of the G-Buffer for analysis.
/// Or an application could send the current game state as one large text or binary document.
/// The data transfer can contain any kind of data, however depending on its type, the connected tools may be able to display the data
/// directly, or not.
/// The nsDataTransfer object represents one logical group of data that will be sent all together. Each piece of data is represented
/// by an nsDataTransferObject. E.g. each texture of a G-Buffer (diffuse, normals, depth) should be sent via one instance of nsDataTransferObject.
/// Those instances of nsDataTransferObject are just created on demand and destroyed directly after, they do not need to be kept around.
///
/// The nsDataTransfer instance, however, needs to be created once and then kept around as long as the data transfer should be possible.
/// It is additionally necessary to enable the data transfer by calling EnableDataTransfer() once, which also specifies under which name
/// the transfer appears in the connected tools.
///
/// At runtime the application needs to check every nsDataTransfer object regularly whether IsTransferRequested() returns true. If so,
/// the application should prepare all data by putting it into instances of nsDataTransferObject and then transferring them through the
/// nsDataTransfer object via nsDataTransfer::Transfer().
///
/// The tools can request a data transfer at any time, however they will not block for the result. Thus whether an application 'answers'
/// or not, is not a problem, the application may just ignore the request. Similarly, the application may also 'push' out a data transfer,
/// even when no data was requested. However, this can overload the network bandwidth or the computer that runs nsInspector, if not done
/// carefully.
class NS_FOUNDATION_DLL nsDataTransfer
{
public:
  /// \brief By default the data transfer is deactivated.
  nsDataTransfer();

  /// \brief Deactivates the data transfer (sends this info via nsTelemetry).
  virtual ~nsDataTransfer();

  /// \brief Disables the data transfer. It will not show up in nsInspector anymore and calling Transfer() on it will be ignored.
  void DisableDataTransfer();

  /// \brief Enables the data transfer. It will show up with the given name in nsInspector.
  void EnableDataTransfer(nsStringView sDataName);

  /// \brief Sets the IsTransferRequested() state to true. Ignored if the data transfer is disabled.
  void RequestDataTransfer();

  /// \brief Returns whether the data transfer has been requested. Always returns false while the data transfer is disabled.
  ///
  /// If bReset is set to false, the request state will not be reset, which can be used to 'peek' at the current state.
  /// By default the request state is reset to false afterwards, the application should answer the request.
  bool IsTransferRequested(bool bReset = true);

private:
  virtual void OnTransferRequest() {}

  void SendStatus();

  /// \brief The data in the given nsDataTransferObject is sent via nsTelemetry to all connected tools, which can then display or process it.
  void Transfer(nsDataTransferObject& Object);

private:
  friend class nsDataTransferObject;

  static void TelemetryMessage(void* pPassThrough);
  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e);
  static void Initialize();
  static void SendAllDataTransfers();

  static bool s_bInitialized;

  bool m_bEnabled;
  bool m_bTransferRequested;
  nsString m_sDataName;
  static nsSet<nsDataTransfer*> s_AllTransfers;
};
