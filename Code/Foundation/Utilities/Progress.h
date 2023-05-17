#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>

class wdProgress;
class wdProgressRange;

/// \brief Through these events the state of an wdProgress instance is communicated.
///
/// Other code can use this to visualize the progress in different ways.
/// For instance a GUI application can show a progress bar dialog and a game
/// could show a loading screen.
struct WD_FOUNDATION_DLL wdProgressEvent
{
  enum class Type
  {
    ProgressStarted, ///< Sent when the the first progress starts
    ProgressEnded,   ///< Sent when progress finishes or is canceled
    ProgressChanged, ///< Sent whenever the progress value changes. Not necessarily in every update step.
    CancelClicked,   ///< The user just clicked cancel (for the first time).
  };

  Type m_Type;
  wdProgress* m_pProgressbar;
};

/// \brief Manages the way a progress bar is subdivided and advanced.
///
/// wdProgress represents a single progress bar. It can be sub-divided into groups and sub-groups using wdProgressbarRange.
/// From the ranges and the current advancement, a final progress percentage is computed. Every time a significant change
/// takes place, an event is broadcast. This allows other code to display the progress, either in a GUI application
/// or in a fullscreen loading screen or in any other way appropriate.
class WD_FOUNDATION_DLL wdProgress
{
public:
  wdProgress();
  ~wdProgress();

  /// \brief Returns the current overall progress in [0; 1] range.
  float GetCompletion() const;

  /// \brief Sets the current overall progress in [0; 1] range. Should not be called directly, typically called by wdProgreesRange.
  void SetCompletion(float fCompletion);

  /// \brief Returns the current 'headline' text for the progress bar
  wdStringView GetMainDisplayText() const;

  /// \brief Returns the current detail text for the progress bar
  wdStringView GetStepDisplayText() const;

  /// \brief Used to inform wdProgress of outside user input. May have an effect or not.
  void UserClickedCancel();

  /// \brief Whether the user requested to cancel the operation.
  bool WasCanceled() const;

  /// \brief Returns whether the current operations may be canceled or not.
  bool AllowUserCancel() const;

  /// \brief Returns the currently set default wdProgress instance. This will always be valid.
  static wdProgress* GetGlobalProgressbar();

  /// \brief Allows to set a custom wdProgress instance as the global default instance.
  static void SetGlobalProgressbar(wdProgress* pProgress);

  /// \brief Events are sent when the progress changes
  wdEvent<const wdProgressEvent&> m_Events;

  /// \brief Custom user data.
  void* m_pUserData = nullptr;

private:
  friend class wdProgressRange;
  void SetActiveRange(wdProgressRange* pRange);

  wdProgressRange* m_pActiveRange = nullptr;

  wdString m_sCurrentDisplayText;
  bool m_bCancelClicked = false;
  bool m_bEnableCancel = true;

  float m_fLastReportedCompletion = 0.0f;
  float m_fCurrentCompletion = 0.0f;
};

/// \brief wdProgressRange is the preferred method to inform the system of progress.
///
/// wdProgressRange is a scoped class, ie. upon creation it adds a range to the current progress
/// and upon destruction the entire range is considered to be completed.
/// Ranges can be nested. For instance when a top level range consists of three 'steps',
/// then opening a nested range will sub-divide that first step. When the nested range is closed,
/// the first top-level step is finished and BeginNextStep() should be called on the top-level range.
/// Subsequently the second step is active and can again be further subdivided with another nested wdProgressRange.
class WD_FOUNDATION_DLL wdProgressRange
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdProgressRange);

public:
  /// \brief Creates a progress range scope.
  ///
  /// If any other progress range is currently active, it will become the parent range and the currently active step will be subdivided.
  /// \param szDisplayText is the main display text for this range.
  /// \param uiSteps is the number of steps that this range will be subdivided into
  /// \param bAllowCancel specifies whether the user can cancel this operation
  /// \param pProgressbar can be specified, if available, otherwise the currently active wdProgress instance is used.
  wdProgressRange(wdStringView sDisplayText, wdUInt32 uiSteps, bool bAllowCancel, wdProgress* pProgressbar = nullptr);

  /// \brief Creates a progress range scope without steps. Use SetCompletion to manually set the completion value.
  wdProgressRange(wdStringView sDisplayText, bool bAllowCancel, wdProgress* pProgressbar = nullptr);

  /// \brief The destructor closes the current range. All progress in this range is assumed to have completed,
  /// even if BeginNextStep() has not been called once for every subdivision step.
  ~wdProgressRange();

  /// \brief Returns the wdProgress instance that this range uses.
  wdProgress* GetProgressbar() const;

  /// \brief Allows to weigh each step differently.
  ///
  /// This makes it possible to divide an operation into two steps, but have one part take up 90% and the other 10%.
  /// \param uiStep The index for the step to set the weight
  /// \param fWeight The weighting in [0; 1] range
  void SetStepWeighting(wdUInt32 uiStep, float fWeight);

  /// \brief Should be called whenever a new sub-step is started to advance the progress.
  ///
  /// \param szStepDisplayText The sub-text for the next step to be displayed.
  /// \param uiNumSteps How many steps have been completed.
  /// \return Returns false if the user clicked cancel.
  bool BeginNextStep(wdStringView sStepDisplayText, wdUInt32 uiNumSteps = 1);

  /// \brief Manually set the completion value between 0..1.
  bool SetCompletion(double fCompletionFactor);

  /// \brief Whether the user requested to cancel the operation.
  bool WasCanceled() const;

private:
  friend class wdProgress;

  void Init(wdStringView sDisplayText, bool bAllowCancel, wdProgress* pProgressbar);
  float GetStepWeight(wdUInt32 uiStep) const;
  void ComputeCurStepBaseAndRange(double& out_base, double& out_range);

  wdProgressRange* m_pParentRange = nullptr;
  wdProgress* m_pProgressbar = nullptr;

  wdInt32 m_iCurrentStep = 0;
  wdString m_sDisplayText;
  wdString m_sStepDisplayText;
  wdHashTable<wdUInt32, float> m_StepWeights;

  bool m_bAllowCancel = false;
  double m_fPercentageBase = 0.0;
  double m_fPercentageRange = 0.0;
  double m_fWeightedCompletion = 0.0;
  double m_fSummedWeight = 0.0;
};
