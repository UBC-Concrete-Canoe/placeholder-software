#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <gp_Pnt.hxx> // OCCT 3D Point primitive

/**
 * @brief Lightweight object representing a single vertex in the hull lattice.
 *
 * Stores a stable identifier, 3D position, and NURBS weight for a control point.
 * Any mutation to position or weight marks the point as dirty so dependent systems
 * can detect and react to changes.
 */
class ControlPoint
{
public:
	/**
	 * @brief Construct a control point.
	 * @param id Unique integer identifier for this point.
	 * @param pos Initial 3D position in model space.
	 * @param weight Rational weight used by NURBS operations (defaults to 1.0).
	 */
	ControlPoint(int id, const gp_Pnt& pos, double weight = 1.0)
	  : m_id(id)
	  , m_position(pos)
	  , m_weight(weight)
	  , m_isDirty(false)
	{
	}

	/**
	 * @brief Get the current 3D position.
	 * @return Const reference to the point position.
	 */
	const gp_Pnt& getPosition() const { return m_position; }
	/**
	 * @brief Update the 3D position.
	 * @param pos New position value.
	 * @note Marks the control point as dirty.
	 */
	void setPosition(const gp_Pnt& pos)
	{
		m_position = pos;
		m_isDirty = true;
	}

	/**
	 * @brief Get the NURBS weight.
	 * @return Current rational weight.
	 */
	double getWeight() const { return m_weight; }
	/**
	 * @brief Set the NURBS weight.
	 * @param w New rational weight.
	 * @note Marks the control point as dirty.
	 */
	void setWeight(double w)
	{
		m_weight = w;
		m_isDirty = true;
	}

	/**
	 * @brief Get the immutable point identifier.
	 * @return Integer point ID.
	 */
	int getId() const { return m_id; }
	/**
	 * @brief Check whether the point has been modified.
	 * @return True if position or weight changed since last clear.
	 */
	bool isDirty() const { return m_isDirty; }
	/**
	 * @brief Reset the dirty flag after change processing.
	 */
	void clearDirty() { m_isDirty = false; }

private:
	int m_id;
	gp_Pnt m_position;
	double m_weight;
	bool m_isDirty; // Observer flag for change tracking
};

#endif // CONTROLPOINT_H