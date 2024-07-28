#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/Query.h>

nsGALQuery::nsGALQuery(const nsGALQueryCreationDescription& Description)
  : nsGALResource<nsGALQueryCreationDescription>(Description)

{
}

nsGALQuery::~nsGALQuery() = default;
