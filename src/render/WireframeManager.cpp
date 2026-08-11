#include "WireframeManager.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

WireframeManager::WireframeManager(
	const Handle(AIS_InteractiveContext)& context,
	const std::shared_ptr<HullModel>& hullModel
)
  : m_context(context)
  , m_hullModel(hullModel)
{
	if (m_hullModel)
	{
		m_hullModel->addObserver(this);
	}
}

WireframeManager::~WireframeManager()
{
	if (m_hullModel)
	{
		m_hullModel->removeObserver(this);
	}
	if (!m_context.IsNull())
	{
		for (const auto& [edgeId, shape] : m_edgeShapes)
		{
			(void)edgeId;
			if (!shape.IsNull())
			{
				m_context->Remove(shape, Standard_False);
			}
		}
		m_context->UpdateCurrentViewer();
	}
}

void
WireframeManager::build()
{
	if (!m_hullModel || m_context.IsNull())
	{
		return;
	}

	for (const auto& [edgeId, shape] : m_edgeShapes)
	{
		(void)edgeId;
		if (!shape.IsNull())
		{
			m_context->Remove(shape, Standard_False);
		}
	}
	m_edgeShapes.clear();

	for (std::size_t index = 0; index < m_hullModel->edgeCount(); ++index)
	{
		const ControlEdge* edge = m_hullModel->edgeAt(index);
		if (edge != nullptr)
		{
			updateEdge(edge->id);
		}
	}
	m_context->UpdateCurrentViewer();
}

void
WireframeManager::onControlPointMoved(int pointId)
{
	if (!m_hullModel)
	{
		return;
	}
	for (int edgeId : m_hullModel->incidentEdgeIds(pointId))
	{
		updateEdge(edgeId);
	}
	if (!m_context.IsNull())
	{
		m_context->UpdateCurrentViewer();
	}
}

void
WireframeManager::onEdgeAdded(int edgeId)
{
	updateEdge(edgeId);
}

void
WireframeManager::onEdgeRemoved(int edgeId)
{
	removeRenderedEdge(edgeId, true);
}

void
WireframeManager::onModelReset()
{
	build();
}

void
WireframeManager::updateEdge(int edgeId)
{
	if (!m_hullModel || m_context.IsNull())
	{
		return;
	}

	const ControlEdge& controlEdge = m_hullModel->edge(edgeId);
	const gp_Pnt& first = m_hullModel->point(controlEdge.firstPointId).getPosition();
	const gp_Pnt& second = m_hullModel->point(controlEdge.secondPointId).getPosition();
	const TopoDS_Edge renderedEdge = BRepBuilderAPI_MakeEdge(first, second);

	auto found = m_edgeShapes.find(edgeId);
	if (found == m_edgeShapes.end())
	{
		Handle(AIS_Shape) shape = new AIS_Shape(renderedEdge);
		m_edgeShapes.emplace(edgeId, shape);
		if (m_isVisible)
		{
			m_context->Display(shape, Standard_False);
		}
		return;
	}

	found->second->SetShape(renderedEdge);
	m_context->Redisplay(found->second, Standard_False);
}

void
WireframeManager::removeRenderedEdge(int edgeId, bool updateViewer)
{
	auto found = m_edgeShapes.find(edgeId);
	if (found == m_edgeShapes.end())
	{
		return;
	}
	if (!m_context.IsNull() && !found->second.IsNull())
	{
		m_context->Remove(found->second, Standard_False);
		if (updateViewer)
		{
			m_context->UpdateCurrentViewer();
		}
	}
	m_edgeShapes.erase(found);
}