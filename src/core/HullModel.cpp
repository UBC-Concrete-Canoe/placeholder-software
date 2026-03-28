#include "HullModel.h"

#include <stdexcept>

/**
 * @brief Construct a fixed-size control lattice and initialize ID lookup.
 * @param uCount Number of points in U direction.
 * @param vCount Number of points in V direction.
 * @throws std::invalid_argument If either dimension is not positive.
 */
HullModel::HullModel(int uCount, int vCount)
  : m_uCount(uCount)
  , m_vCount(vCount)
{
	if (uCount <= 0 || vCount <= 0)
	{
		throw std::invalid_argument("HullModel dimensions must be greater than zero");
	}

	// Pre-allocate a fixed-size lattice of default control points.
	m_points.reserve(uCount * vCount);
	int idCounter = 0;
	for (int u = 0; u < uCount; ++u)
	{
		for (int v = 0; v < vCount; ++v)
		{
			m_points.emplace_back(idCounter++, gp_Pnt(0, 0, 0));
		}
	}

	// Build ID-to-index map for fast updates by stable ID.
	for (size_t i = 0; i < m_points.size(); ++i)
	{
		m_idToIndexMap[m_points[i].getId()] = i;
	}
}

/** @copydoc HullModel::getPoint(int, int) */
ControlPoint&
HullModel::getPoint(int u, int v)
{
	return m_points[getIndex(u, v)];
}

/** @copydoc HullModel::getPoint(int, int) const */
const ControlPoint&
HullModel::getPoint(int u, int v) const
{
	return m_points[getIndex(u, v)];
}

/** @copydoc HullModel::updatePoint */
void
HullModel::updatePoint(int id, const gp_Pnt& newPos)
{
	auto it = m_idToIndexMap.find(id);
	if (it != m_idToIndexMap.end())
	{
		m_points[it->second].setPosition(newPos);
	}
}

/** @copydoc HullModel::toOcctPoles */
TColgp_Array2OfPnt
HullModel::toOcctPoles() const
{
	TColgp_Array2OfPnt occtPoles(1, m_uCount, 1, m_vCount);

	for (int u = 0; u < m_uCount; ++u)
	{
		for (int v = 0; v < m_vCount; ++v)
		{
			const gp_Pnt& pos = getPoint(u, v).getPosition();
			occtPoles.SetValue(u + 1, v + 1, pos);
		}
	}

	return occtPoles;
}

/** @copydoc HullModel::getIndex */
size_t
HullModel::getIndex(int u, int v) const
{
	if (u < 0 || v < 0 || u >= m_uCount || v >= m_vCount)
	{
		throw std::out_of_range("HullModel index is out of range");
	}

	return (u * m_vCount) + v;
}
