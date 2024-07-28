#pragma once

#include <Foundation/Strings/HashedString.h>
#include <RendererCore/Pipeline/RenderData.h>

class nsStreamWriter;

class NS_RENDERERCORE_DLL nsExtractor : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsExtractor, nsReflectedClass);
  NS_DISALLOW_COPY_AND_ASSIGN(nsExtractor);

public:
  nsExtractor(const char* szName);
  virtual ~nsExtractor();

  /// \brief Sets the name of the extractor.
  void SetName(const char* szName);

  /// \brief returns the name of the extractor.
  const char* GetName() const;

  virtual void Extract(const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData);

  virtual void PostSortAndBatch(const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData);

  virtual nsResult Serialize(nsStreamWriter& inout_stream) const;
  virtual nsResult Deserialize(nsStreamReader& inout_stream);

protected:
  /// \brief returns true if the given object should be filtered by view tags.
  bool FilterByViewTags(const nsView& view, const nsGameObject* pObject) const;

  /// \brief extracts the render data for the given object.
  void ExtractRenderData(const nsView& view, const nsGameObject* pObject, nsMsgExtractRenderData& msg, nsExtractedRenderData& extractedRenderData) const;

private:
  friend class nsRenderPipeline;

  bool m_bActive;

  nsHashedString m_sName;

protected:
  nsHybridArray<nsHashedString, 4> m_DependsOn;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  mutable nsUInt32 m_uiNumCachedRenderData;
  mutable nsUInt32 m_uiNumUncachedRenderData;
#endif
};


class NS_RENDERERCORE_DLL nsVisibleObjectsExtractor : public nsExtractor
{
  NS_ADD_DYNAMIC_REFLECTION(nsVisibleObjectsExtractor, nsExtractor);

public:
  nsVisibleObjectsExtractor(const char* szName = "VisibleObjectsExtractor");
  ~nsVisibleObjectsExtractor();

  virtual void Extract(const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;
};

class NS_RENDERERCORE_DLL nsSelectedObjectsExtractorBase : public nsExtractor
{
  NS_ADD_DYNAMIC_REFLECTION(nsSelectedObjectsExtractorBase, nsExtractor);

public:
  nsSelectedObjectsExtractorBase(const char* szName = "SelectedObjectsExtractor");
  ~nsSelectedObjectsExtractorBase();

  virtual void Extract(const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData) override;
  virtual const nsDeque<nsGameObjectHandle>* GetSelection() = 0;

  nsRenderData::Category m_OverrideCategory;
};

/// \brief Stores a list of game objects that should get highlighted by the renderer.
///
/// Store an instance somewhere in your game code:
/// nsSelectedObjectsContext m_SelectedObjects;
/// Add handles to game object that should be get the highlighting outline (as the editor uses for selected objects).
/// On an nsView call:
/// nsView::SetExtractorProperty("HighlightObjects", "SelectionContext", &m_SelectedObjects);
/// The first name must be the name of an nsSelectedObjectsExtractor that is instantiated by the render pipeline.
///
/// As long as there is also an nsSelectionHighlightPass in the render pipeline, all objects in this selection will be rendered
/// with an outline.
class NS_RENDERERCORE_DLL nsSelectedObjectsContext : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSelectedObjectsContext, nsReflectedClass);

public:
  nsSelectedObjectsContext();
  ~nsSelectedObjectsContext();

  void RemoveDeadObjects(const nsWorld& world);
  void AddObjectAndChildren(const nsWorld& world, const nsGameObjectHandle& hObject);
  void AddObjectAndChildren(const nsWorld& world, const nsGameObject* pObject);

  nsDeque<nsGameObjectHandle> m_Objects;
};

/// \brief An extractor that can be instantiated in a render pipeline, to define manually which objects should be rendered with a selection outline.
///
/// \sa nsSelectedObjectsContext
class NS_RENDERERCORE_DLL nsSelectedObjectsExtractor : public nsSelectedObjectsExtractorBase
{
  NS_ADD_DYNAMIC_REFLECTION(nsSelectedObjectsExtractor, nsSelectedObjectsExtractorBase);

public:
  nsSelectedObjectsExtractor(const char* szName = "ExplicitlySelectedObjectsExtractor");
  ~nsSelectedObjectsExtractor();

  virtual const nsDeque<nsGameObjectHandle>* GetSelection() override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  /// \brief The context is typically set through an nsView, through nsView::SetExtractorProperty("<name>", "SelectionContext", pointer);
  void SetSelectionContext(nsSelectedObjectsContext* pSelectionContext) { m_pSelectionContext = pSelectionContext; } // [ property ]
  nsSelectedObjectsContext* GetSelectionContext() const { return m_pSelectionContext; }                              // [ property ]

private:
  nsSelectedObjectsContext* m_pSelectionContext = nullptr;
};
