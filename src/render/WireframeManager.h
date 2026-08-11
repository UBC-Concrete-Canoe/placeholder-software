#pragma once

#include "core/HullModel.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Handle.hxx>

#include <memory>
#include <unordered_map>

/**
 * @brief Renders every control-mesh edge as an independently updateable shape.
 */
class WireframeManager : public IHullModelObserver
{
public:
	WireframeManager(
		const Handle(AIS_InteractiveContext)& context,
		const std::shared_ptr<HullModel>& hullModel
	);
	~WireframeManager() override;

	//! Rebuild all rendered edges from the model.
	void build();

	//! Temporary compatibility name for callers from the lattice prototype.
	void BuildLattice() { build(); }

	void onControlPointMoved(int pointId) override;
	void onEdgeAdded(int edgeId) override;
	void onEdgeRemoved(int edgeId) override;
	void onModelReset() override;

private:
	void updateEdge(int edgeId);
	void removeRenderedEdge(int edgeId, bool updateViewer);

	Handle(AIS_InteractiveContext) m_context;
	std::shared_ptr<HullModel> m_hullModel;
	std::unordered_map<int, Handle(AIS_Shape)> m_edgeShapes;
	bool m_isVisible = true;
};
