#pragma once

#include <AIS_InteractiveObject.hxx>
#include <gp_Pnt.hxx>

#include "core/ControlPoint.h"

/**
 * @brief OCCT visual for a single selectable control point.
 *
 * VisualPoint is an AIS interactive object bound to one model ControlPoint.
 * It renders as a marker in the 3D viewport and exposes OCCT selection
 * through a sensitive point.
 *
 * Lifetime note:
 * - The ControlPoint pointer is non-owning and must remain valid while this
 *   VisualPoint is displayed.
 */
class VisualPoint : public AIS_InteractiveObject
{
	DEFINE_STANDARD_RTTIEXT(VisualPoint, AIS_InteractiveObject)

public:
	/**
	 * @brief Construct a visual bound to a model control point.
	 * @param controlPoint Non-owning pointer to model data.
	 */
	explicit VisualPoint(const ControlPoint* controlPoint);

	/**
	 * @brief Synchronize cached position with the model point.
	 *
	 * Compares the model coordinate to the cached rendered coordinate.
	 * If changed, marks this AIS object for update and refreshes selection.
	 */
	void synchronize();

	/**
	 * @brief Update selected-state style flag.
	 * @param selected True when this point is selected.
	 * @return True if state changed and visual redisplay is needed.
	 */
	bool setSelectedStyle(bool selected);

	/**
	 * @brief Get current cached point in OCCT coordinates.
	 */
	const gp_Pnt& point() const { return m_cachedPoint; }

protected:
	/**
	 * @brief Build point presentation for display/highlight.
	 */
	void Compute(const Handle(PrsMgr_PresentationManager)& thePrsMgr,
	             const Handle(Prs3d_Presentation)& thePrs,
	             const Standard_Integer theMode) override;

	/**
	 * @brief Build selectable entity for ray picking.
	 */
	void ComputeSelection(const Handle(SelectMgr_Selection)& theSelection,
	                      const Standard_Integer theMode) override;

private:
	/**
	 * @brief Convert model coordinate to OCCT gp_Pnt.
	 */
	gp_Pnt pointFromModel() const;

	/**
	 * @brief Compare with epsilon to detect movement.
	 */
	bool hasMoved(const gp_Pnt& p) const;

	const ControlPoint* m_controlPoint = nullptr;
	gp_Pnt m_cachedPoint;
	bool m_isSelected = false;
};

DEFINE_STANDARD_HANDLE(VisualPoint, AIS_InteractiveObject)
