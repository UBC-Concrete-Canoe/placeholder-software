#pragma once

#include "core/ControlPointVisualStyle.h"

#include <Prs3d_Drawer.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Quantity_Color.hxx>

namespace OcctStyleMapper
{
inline Quantity_Color
toOcctColor(const RgbColor& color)
{
	return Quantity_Color(color.r, color.g, color.b, Quantity_TOC_RGB);
}

inline Handle(Prs3d_PointAspect) makePointAspect(const RgbColor& color, const Standard_Real scale)
{
	return new Prs3d_PointAspect(Aspect_TOM_POINT, toOcctColor(color), scale);
}

inline Handle(Prs3d_Drawer) makePointDrawer(const RgbColor& color, const Standard_Real scale)
{
	Handle(Prs3d_Drawer) drawer = new Prs3d_Drawer();
	Handle(Prs3d_PointAspect) pointAspect = makePointAspect(color, scale);
	drawer->SetPointAspect(pointAspect);
	drawer->SetColor(toOcctColor(color));
	return drawer;
}
} // namespace OcctStyleMapper
