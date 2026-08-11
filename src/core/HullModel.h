#pragma once

#include "ControlPoint.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct ControlEdge
{
	int id;
	int firstPointId;
	int secondPointId;
};

struct ControlFace
{
	int id;
	std::vector<int> pointIds;
	std::vector<int> edgeIds;
};

struct EdgeSplitResult
{
	int pointId;
	int firstEdgeId;
	int secondEdgeId;
	std::vector<int> replacementFaceIds;
};

class IHullModelObserver
{
public:
	virtual ~IHullModelObserver() = default;
	virtual void onPointAdded(int) {}
	virtual void onControlPointMoved(int) {}
	virtual void onEdgeAdded(int) {}
	virtual void onEdgeRemoved(int) {}
	virtual void onFaceAdded(int) {}
	virtual void onFaceRemoved(int) {}
	virtual void onModelReset() {}
};

/**
 * @brief Editable, non-grid control mesh for a hull.
 *
 * Vertices, edges, and polygon faces have stable IDs. Adjacency maps make
 * topology edits independent of any rectangular U/V layout.
 */
class HullModel
{
public:
	HullModel() = default;

	int addPoint(const gp_Pnt& position, double weight = 1.0);
	int addEdge(int firstPointId, int secondPointId);
	int addFace(const std::vector<int>& pointIds);

	void updatePoint(int pointId, const gp_Pnt& newPosition);

	/**
	 * @brief Replace an edge by two edges meeting at a new midpoint vertex.
	 *
	 * Every adjacent face loop is rewritten to include the new vertex.
	 */
	EdgeSplitResult splitEdge(int edgeId);

	/**
	 * @brief Insert an edge between two non-adjacent vertices of one face.
	 *
	 * The original face is replaced by the two resulting polygon faces.
	 * @return IDs of the two replacement faces.
	 */
	std::pair<int, int> insertEdge(
		int faceId,
		int firstPointId,
		int secondPointId
	);

	ControlPoint& point(int pointId);
	const ControlPoint& point(int pointId) const;
	const ControlEdge& edge(int edgeId) const;
	const ControlFace& face(int faceId) const;

	std::size_t pointCount() const { return m_pointOrder.size(); }
	std::size_t edgeCount() const { return m_edgeOrder.size(); }
	std::size_t faceCount() const { return m_faceOrder.size(); }

	ControlPoint* pointAt(std::size_t index);
	const ControlPoint* pointAt(std::size_t index) const;
	const ControlEdge* edgeAt(std::size_t index) const;
	const ControlFace* faceAt(std::size_t index) const;

	std::vector<int> incidentEdgeIds(int pointId) const;
	std::vector<int> adjacentFaceIds(int edgeId) const;

	/**
	 * @return Empty when all references, loops, and manifold constraints hold.
	 */
	std::vector<std::string> validateTopology() const;

	void addObserver(IHullModelObserver* observer);
	void removeObserver(IHullModelObserver* observer);

private:
	int findEdgeId(int firstPointId, int secondPointId) const;
	void removeEdge(int edgeId);
	void removeFace(int faceId);
	bool faceLoopExists(const std::vector<int>& pointIds) const;

	static std::vector<int> pathAlongFace(
		const std::vector<int>& loop,
		std::size_t start,
		std::size_t finish
	);

	int m_nextPointId = 0;
	int m_nextEdgeId = 0;
	int m_nextFaceId = 0;

	// unordered_map references remain valid across rehash, keeping VisualPoint
	// bindings stable while new control points are inserted.
	std::unordered_map<int, ControlPoint> m_points;
	std::unordered_map<int, ControlEdge> m_edges;
	std::unordered_map<int, ControlFace> m_faces;
	std::vector<int> m_pointOrder;
	std::vector<int> m_edgeOrder;
	std::vector<int> m_faceOrder;

	std::unordered_map<int, std::unordered_set<int>> m_pointToEdges;
	std::unordered_map<int, std::unordered_set<int>> m_edgeToFaces;
	std::vector<IHullModelObserver*> m_observers;
};
