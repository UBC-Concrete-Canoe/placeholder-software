#include "HullModel.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace
{
bool
edgeMatches(const ControlEdge& edge, int firstPointId, int secondPointId)
{
	return (edge.firstPointId == firstPointId && edge.secondPointId == secondPointId)
		|| (edge.firstPointId == secondPointId && edge.secondPointId == firstPointId);
}

template<typename T>
void
eraseId(std::vector<T>& values, T value)
{
	values.erase(std::remove(values.begin(), values.end(), value), values.end());
}
} // namespace

int
HullModel::addPoint(const gp_Pnt& position, double weight)
{
	const int id = m_nextPointId++;
	m_points.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(id),
		std::forward_as_tuple(id, position, weight)
	);
	m_pointOrder.push_back(id);
	m_pointToEdges.emplace(id, std::unordered_set<int>{});

	for (IHullModelObserver* observer : m_observers)
	{
		observer->onPointAdded(id);
	}
	return id;
}

int
HullModel::addEdge(int firstPointId, int secondPointId)
{
	if (firstPointId == secondPointId)
	{
		throw std::invalid_argument("An edge requires two distinct control points");
	}
	if (m_points.count(firstPointId) == 0 || m_points.count(secondPointId) == 0)
	{
		throw std::out_of_range("An edge references an unknown control point");
	}
	if (findEdgeId(firstPointId, secondPointId) >= 0)
	{
		throw std::invalid_argument("A control edge already connects these points");
	}

	const int id = m_nextEdgeId++;
	m_edges.emplace(id, ControlEdge{id, firstPointId, secondPointId});
	m_edgeOrder.push_back(id);
	m_pointToEdges[firstPointId].insert(id);
	m_pointToEdges[secondPointId].insert(id);
	m_edgeToFaces.emplace(id, std::unordered_set<int>{});

	for (IHullModelObserver* observer : m_observers)
	{
		observer->onEdgeAdded(id);
	}
	return id;
}

int
HullModel::addFace(const std::vector<int>& pointIds)
{
	if (pointIds.size() < 3)
	{
		throw std::invalid_argument("A face requires at least three control points");
	}

	std::unordered_set<int> uniquePoints;
	for (int pointId : pointIds)
	{
		if (m_points.count(pointId) == 0)
		{
			throw std::out_of_range("A face references an unknown control point");
		}
		if (!uniquePoints.insert(pointId).second)
		{
			throw std::invalid_argument("A face loop cannot repeat a control point");
		}
	}
	if (faceLoopExists(pointIds))
	{
		throw std::invalid_argument("An equivalent control face already exists");
	}

	for (std::size_t index = 0; index < pointIds.size(); ++index)
	{
		const int edgeId =
			findEdgeId(pointIds[index], pointIds[(index + 1) % pointIds.size()]);
		if (edgeId >= 0 && m_edgeToFaces.at(edgeId).size() >= 2)
		{
			throw std::invalid_argument("Adding this face would create a non-manifold edge");
		}
	}

	std::vector<int> edgeIds;
	edgeIds.reserve(pointIds.size());
	for (std::size_t index = 0; index < pointIds.size(); ++index)
	{
		const int firstPointId = pointIds[index];
		const int secondPointId = pointIds[(index + 1) % pointIds.size()];
		int edgeId = findEdgeId(firstPointId, secondPointId);
		if (edgeId < 0)
		{
			edgeId = addEdge(firstPointId, secondPointId);
		}
		edgeIds.push_back(edgeId);
	}

	const int id = m_nextFaceId++;
	m_faces.emplace(id, ControlFace{id, pointIds, edgeIds});
	m_faceOrder.push_back(id);
	for (int edgeId : edgeIds)
	{
		m_edgeToFaces[edgeId].insert(id);
	}

	for (IHullModelObserver* observer : m_observers)
	{
		observer->onFaceAdded(id);
	}
	return id;
}

void
HullModel::updatePoint(int pointId, const gp_Pnt& newPosition)
{
	point(pointId).setPosition(newPosition);
	for (IHullModelObserver* observer : m_observers)
	{
		observer->onControlPointMoved(pointId);
	}
}

EdgeSplitResult
HullModel::splitEdge(int edgeId)
{
	const ControlEdge oldEdge = edge(edgeId);
	const ControlPoint& firstPoint = point(oldEdge.firstPointId);
	const ControlPoint& secondPoint = point(oldEdge.secondPointId);
	const gp_Pnt midpoint(
		(firstPoint.getPosition().X() + secondPoint.getPosition().X()) * 0.5,
		(firstPoint.getPosition().Y() + secondPoint.getPosition().Y()) * 0.5,
		(firstPoint.getPosition().Z() + secondPoint.getPosition().Z()) * 0.5
	);
	const double midpointWeight = (firstPoint.getWeight() + secondPoint.getWeight()) * 0.5;

	std::vector<std::vector<int>> adjacentLoops;
	for (int faceId : adjacentFaceIds(edgeId))
	{
		adjacentLoops.push_back(face(faceId).pointIds);
		removeFace(faceId);
	}
	removeEdge(edgeId);

	const int midpointId = addPoint(midpoint, midpointWeight);
	const int firstEdgeId = addEdge(oldEdge.firstPointId, midpointId);
	const int secondEdgeId = addEdge(midpointId, oldEdge.secondPointId);

	std::vector<int> replacementFaceIds;
	for (std::vector<int>& loop : adjacentLoops)
	{
		std::vector<int> replacement;
		replacement.reserve(loop.size() + 1);
		bool inserted = false;
		for (std::size_t index = 0; index < loop.size(); ++index)
		{
			const int current = loop[index];
			const int next = loop[(index + 1) % loop.size()];
			replacement.push_back(current);
			if (edgeMatches(oldEdge, current, next))
			{
				replacement.push_back(midpointId);
				inserted = true;
			}
		}
		if (!inserted)
		{
			throw std::logic_error("Adjacent face does not contain the split edge");
		}
		replacementFaceIds.push_back(addFace(replacement));
	}

	return {midpointId, firstEdgeId, secondEdgeId, replacementFaceIds};
}

std::pair<int, int>
HullModel::insertEdge(int faceId, int firstPointId, int secondPointId)
{
	const std::vector<int> originalLoop = face(faceId).pointIds;
	const auto firstIt = std::find(originalLoop.begin(), originalLoop.end(), firstPointId);
	const auto secondIt = std::find(originalLoop.begin(), originalLoop.end(), secondPointId);
	if (firstIt == originalLoop.end() || secondIt == originalLoop.end())
	{
		throw std::invalid_argument("Inserted edge points must belong to the selected face");
	}
	if (firstPointId == secondPointId || findEdgeId(firstPointId, secondPointId) >= 0)
	{
		throw std::invalid_argument("Inserted edge points must be non-adjacent");
	}

	const std::size_t firstIndex =
		static_cast<std::size_t>(std::distance(originalLoop.begin(), firstIt));
	const std::size_t secondIndex =
		static_cast<std::size_t>(std::distance(originalLoop.begin(), secondIt));
	const std::vector<int> firstLoop = pathAlongFace(originalLoop, firstIndex, secondIndex);
	const std::vector<int> secondLoop = pathAlongFace(originalLoop, secondIndex, firstIndex);
	if (firstLoop.size() < 3 || secondLoop.size() < 3)
	{
		throw std::invalid_argument("Inserted edge would create a degenerate face");
	}

	removeFace(faceId);
	addEdge(firstPointId, secondPointId);
	const int firstFaceId = addFace(firstLoop);
	const int secondFaceId = addFace(secondLoop);
	return {firstFaceId, secondFaceId};
}

ControlPoint&
HullModel::point(int pointId)
{
	auto found = m_points.find(pointId);
	if (found == m_points.end())
	{
		throw std::out_of_range("Unknown control point ID");
	}
	return found->second;
}

const ControlPoint&
HullModel::point(int pointId) const
{
	auto found = m_points.find(pointId);
	if (found == m_points.end())
	{
		throw std::out_of_range("Unknown control point ID");
	}
	return found->second;
}

const ControlEdge&
HullModel::edge(int edgeId) const
{
	auto found = m_edges.find(edgeId);
	if (found == m_edges.end())
	{
		throw std::out_of_range("Unknown control edge ID");
	}
	return found->second;
}

const ControlFace&
HullModel::face(int faceId) const
{
	auto found = m_faces.find(faceId);
	if (found == m_faces.end())
	{
		throw std::out_of_range("Unknown control face ID");
	}
	return found->second;
}

ControlPoint*
HullModel::pointAt(std::size_t index)
{
	return index < m_pointOrder.size() ? &point(m_pointOrder[index]) : nullptr;
}

const ControlPoint*
HullModel::pointAt(std::size_t index) const
{
	return index < m_pointOrder.size() ? &point(m_pointOrder[index]) : nullptr;
}

const ControlEdge*
HullModel::edgeAt(std::size_t index) const
{
	return index < m_edgeOrder.size() ? &edge(m_edgeOrder[index]) : nullptr;
}

const ControlFace*
HullModel::faceAt(std::size_t index) const
{
	return index < m_faceOrder.size() ? &face(m_faceOrder[index]) : nullptr;
}

std::vector<int>
HullModel::incidentEdgeIds(int pointId) const
{
	if (m_points.count(pointId) == 0)
	{
		throw std::out_of_range("Unknown control point ID");
	}
	const auto& edgeIds = m_pointToEdges.at(pointId);
	return {edgeIds.begin(), edgeIds.end()};
}

std::vector<int>
HullModel::adjacentFaceIds(int edgeId) const
{
	if (m_edges.count(edgeId) == 0)
	{
		throw std::out_of_range("Unknown control edge ID");
	}
	const auto& faceIds = m_edgeToFaces.at(edgeId);
	return {faceIds.begin(), faceIds.end()};
}

std::vector<std::string>
HullModel::validateTopology() const
{
	std::vector<std::string> errors;
	std::unordered_set<int> endpointPairs;

	for (const auto& [edgeId, controlEdge] : m_edges)
	{
		if (m_points.count(controlEdge.firstPointId) == 0
			|| m_points.count(controlEdge.secondPointId) == 0)
		{
			errors.push_back("Edge " + std::to_string(edgeId) + " references a missing point");
			continue;
		}
		if (controlEdge.firstPointId == controlEdge.secondPointId)
		{
			errors.push_back("Edge " + std::to_string(edgeId) + " has identical endpoints");
		}
		const int low = std::min(controlEdge.firstPointId, controlEdge.secondPointId);
		const int high = std::max(controlEdge.firstPointId, controlEdge.secondPointId);
		const int pairHash = low * 73856093 ^ high * 19349663;
		if (!endpointPairs.insert(pairHash).second)
		{
			errors.push_back("Duplicate edge endpoints at edge " + std::to_string(edgeId));
		}
		if (m_pointToEdges.count(controlEdge.firstPointId) == 0
			|| m_pointToEdges.at(controlEdge.firstPointId).count(edgeId) == 0
			|| m_pointToEdges.count(controlEdge.secondPointId) == 0
			|| m_pointToEdges.at(controlEdge.secondPointId).count(edgeId) == 0)
		{
			errors.push_back("Edge " + std::to_string(edgeId) + " is missing point adjacency");
		}
		if (m_edgeToFaces.count(edgeId) == 0 || m_edgeToFaces.at(edgeId).size() > 2)
		{
			errors.push_back("Edge " + std::to_string(edgeId) + " has invalid face adjacency");
		}
	}

	for (const auto& [faceId, controlFace] : m_faces)
	{
		if (controlFace.pointIds.size() < 3
			|| controlFace.edgeIds.size() != controlFace.pointIds.size())
		{
			errors.push_back("Face " + std::to_string(faceId) + " has an invalid loop size");
			continue;
		}

		std::unordered_set<int> uniquePoints(
			controlFace.pointIds.begin(), controlFace.pointIds.end()
		);
		if (uniquePoints.size() != controlFace.pointIds.size())
		{
			errors.push_back("Face " + std::to_string(faceId) + " repeats a point");
		}

		for (std::size_t index = 0; index < controlFace.pointIds.size(); ++index)
		{
			const int edgeId = controlFace.edgeIds[index];
			auto edgeIt = m_edges.find(edgeId);
			if (edgeIt == m_edges.end()
				|| !edgeMatches(
					edgeIt->second,
					controlFace.pointIds[index],
					controlFace.pointIds[(index + 1) % controlFace.pointIds.size()]
				))
			{
				errors.push_back("Face " + std::to_string(faceId) + " has a broken edge loop");
				continue;
			}
			if (m_edgeToFaces.at(edgeId).count(faceId) == 0)
			{
				errors.push_back("Face " + std::to_string(faceId) + " is missing edge adjacency");
			}
		}
	}

	return errors;
}

void
HullModel::addObserver(IHullModelObserver* observer)
{
	if (observer != nullptr
		&& std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
	{
		m_observers.push_back(observer);
	}
}

void
HullModel::removeObserver(IHullModelObserver* observer)
{
	eraseId(m_observers, observer);
}

int
HullModel::findEdgeId(int firstPointId, int secondPointId) const
{
	auto incidentIt = m_pointToEdges.find(firstPointId);
	if (incidentIt == m_pointToEdges.end())
	{
		return -1;
	}
	for (int edgeId : incidentIt->second)
	{
		if (edgeMatches(m_edges.at(edgeId), firstPointId, secondPointId))
		{
			return edgeId;
		}
	}
	return -1;
}

void
HullModel::removeEdge(int edgeId)
{
	const ControlEdge oldEdge = edge(edgeId);
	if (!m_edgeToFaces.at(edgeId).empty())
	{
		throw std::logic_error("Cannot remove an edge while faces still reference it");
	}
	m_pointToEdges[oldEdge.firstPointId].erase(edgeId);
	m_pointToEdges[oldEdge.secondPointId].erase(edgeId);
	m_edgeToFaces.erase(edgeId);
	m_edges.erase(edgeId);
	eraseId(m_edgeOrder, edgeId);
	for (IHullModelObserver* observer : m_observers)
	{
		observer->onEdgeRemoved(edgeId);
	}
}

void
HullModel::removeFace(int faceId)
{
	const ControlFace oldFace = face(faceId);
	for (int edgeId : oldFace.edgeIds)
	{
		m_edgeToFaces[edgeId].erase(faceId);
	}
	m_faces.erase(faceId);
	eraseId(m_faceOrder, faceId);
	for (IHullModelObserver* observer : m_observers)
	{
		observer->onFaceRemoved(faceId);
	}
}

bool
HullModel::faceLoopExists(const std::vector<int>& pointIds) const
{
	for (const auto& [faceId, controlFace] : m_faces)
	{
		(void)faceId;
		if (controlFace.pointIds.size() != pointIds.size())
		{
			continue;
		}
		const auto startIt =
			std::find(controlFace.pointIds.begin(), controlFace.pointIds.end(), pointIds.front());
		if (startIt == controlFace.pointIds.end())
		{
			continue;
		}
		const std::size_t start =
			static_cast<std::size_t>(std::distance(controlFace.pointIds.begin(), startIt));
		bool forwardMatches = true;
		bool reverseMatches = true;
		for (std::size_t offset = 0; offset < pointIds.size(); ++offset)
		{
			forwardMatches = forwardMatches
				&& controlFace.pointIds[(start + offset) % pointIds.size()] == pointIds[offset];
			reverseMatches = reverseMatches
				&& controlFace.pointIds[
					(start + pointIds.size() - offset) % pointIds.size()
				] == pointIds[offset];
		}
		if (forwardMatches || reverseMatches)
		{
			return true;
		}
	}
	return false;
}

std::vector<int>
HullModel::pathAlongFace(
	const std::vector<int>& loop,
	std::size_t start,
	std::size_t finish
)
{
	std::vector<int> path;
	for (std::size_t index = start;; index = (index + 1) % loop.size())
	{
		path.push_back(loop[index]);
		if (index == finish)
		{
			break;
		}
	}
	return path;
}
