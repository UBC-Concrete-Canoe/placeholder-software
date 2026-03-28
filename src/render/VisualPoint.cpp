#include "VisualPoint.h"

#include <Graphic3d_ArrayOfPoints.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Quantity_Color.hxx>
#include <Select3D_SensitivePoint.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VisualPoint, AIS_InteractiveObject)

namespace
{
constexpr Standard_Real kSyncEpsilon = 1.0e-7;
constexpr Standard_Real kDefaultMarkerScale = 30.0;
constexpr Standard_Real kSelectedMarkerScale = 40.0;
} // namespace

VisualPoint::VisualPoint(const ControlPoint* controlPoint)
  : m_controlPoint(controlPoint)
  , m_cachedPoint(pointFromModel())
{
	SetInfiniteState(Standard_False);
	SetMutable(Standard_True);
	SetHilightMode(1);
}

void
VisualPoint::synchronize()
{
	const gp_Pnt current = pointFromModel();
	if (!hasMoved(current))
	{
		return;
	}

	m_cachedPoint = current;
	SetToUpdate();
	UpdateSelection();
}

bool
VisualPoint::setSelectedStyle(bool selected)
{
	if (m_isSelected == selected)
	{
		return false;
	}

	m_isSelected = selected;
	SetToUpdate();
	return true;
}

void
VisualPoint::Compute(
	const Handle(PrsMgr_PresentationManager) &,
	const Handle(Prs3d_Presentation) & thePrs,
	const Standard_Integer theMode
)
{
	thePrs->Clear();

	Handle(Prs3d_PointAspect) pointAspect;
	if (m_isSelected || theMode == 1)
	{
		pointAspect =
			new Prs3d_PointAspect(Aspect_TOM_POINT, Quantity_NOC_ORANGE, kSelectedMarkerScale);
	}
	else
	{
		pointAspect =
			new Prs3d_PointAspect(Aspect_TOM_POINT, Quantity_NOC_WHITE, kDefaultMarkerScale);
	}

	myDrawer->SetPointAspect(pointAspect);
	thePrs->CurrentGroup()->SetGroupPrimitivesAspect(pointAspect->Aspect());

	Handle(Graphic3d_ArrayOfPoints) array = new Graphic3d_ArrayOfPoints(1);
	array->AddVertex(m_cachedPoint);
	thePrs->CurrentGroup()->AddPrimitiveArray(array);
}

void
VisualPoint::ComputeSelection(
	const Handle(SelectMgr_Selection) & theSelection,
	const Standard_Integer
)
{
	Handle(SelectMgr_EntityOwner) owner = new SelectMgr_EntityOwner(this, 10);
	Handle(Select3D_SensitivePoint) sensitivePoint =
		new Select3D_SensitivePoint(owner, m_cachedPoint);
	theSelection->Add(sensitivePoint);
}

gp_Pnt
VisualPoint::pointFromModel() const
{
	if (m_controlPoint == nullptr)
	{
		return gp_Pnt(0.0, 0.0, 0.0);
	}

	return m_controlPoint->getPosition();
}

bool
VisualPoint::hasMoved(const gp_Pnt& p) const
{
	return p.Distance(m_cachedPoint) > kSyncEpsilon;
}
