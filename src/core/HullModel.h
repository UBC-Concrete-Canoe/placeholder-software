#ifndef HULLMODEL_H
#define HULLMODEL_H

#include <unordered_map>
#include <vector>
#include "ControlPoint.h"

#include <TColgp_Array2OfPnt.hxx>

/**
 * @brief Container managing a fixed 2D lattice of hull control points.
 *
 * This model assumes a FreeShip-like workflow where U/V dimensions are defined
 * before modeling starts. The topology is fixed after construction: control
 * points are modified in place rather than inserted or removed.
 *
 * Internally, points are stored in row-major order for cache-friendly access.
 */
class HullModel
{
public:
	/**
	 * @brief Construct a fixed-size hull control lattice.
	 * @param uCount Number of control points in the U direction.
	 * @param vCount Number of control points in the V direction.
	 */
	HullModel(int uCount, int vCount);

	/**
	 * @brief Get a mutable control point at (u, v).
	 * @param u Zero-based U index.
	 * @param v Zero-based V index.
	 * @return Reference to the requested control point.
	 * @throws std::out_of_range If (u, v) is outside the configured lattice.
	 */
	ControlPoint& getPoint(int u, int v);
	/**
	 * @brief Get a read-only control point at (u, v).
	 * @param u Zero-based U index.
	 * @param v Zero-based V index.
	 * @return Const reference to the requested control point.
	 * @throws std::out_of_range If (u, v) is outside the configured lattice.
	 */
	const ControlPoint& getPoint(int u, int v) const;

	/**
	 * @brief Update an existing control point by stable ID.
	 * @param id Control point ID assigned during construction.
	 * @param newPos New 3D position value.
	 * @note This model does not add points at runtime; only existing points are updated.
	 */
	void updatePoint(int id, const gp_Pnt& newPos);

	/**
	 * @brief Export control points as OCCT poles.
	 * @return 1-based `TColgp_Array2OfPnt` suitable for OCCT surface APIs.
	 */
	TColgp_Array2OfPnt toOcctPoles() const;

	/** @brief Get the lattice size along U. */
	int getUCount() const { return m_uCount; }
	/** @brief Get the lattice size along V. */
	int getVCount() const { return m_vCount; }

private:
	/**
	 * @brief Convert 2D lattice coordinates to row-major 1D index.
	 * @param u Zero-based U index.
	 * @param v Zero-based V index.
	 * @return Flat vector index for `m_points`.
	 * @throws std::out_of_range If (u, v) is outside the configured lattice.
	 */
	size_t getIndex(int u, int v) const;

	int m_uCount;
	int m_vCount;

	//! Row-major data layer: U rows by V columns.
	std::vector<ControlPoint> m_points;

	//! Maps stable point IDs to row-major vector indices.
	std::unordered_map<int, size_t> m_idToIndexMap;
};

#endif // HULLMODEL_H