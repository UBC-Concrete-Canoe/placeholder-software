#pragma once

#include "core/HullModel.h"

/**
 * @brief Represents a single point-move operation as a reversible command.
 *
 * Stores the point index and the world-space displacement so the move can
 * be re-applied or reversed at any time.
 *
 * Usage:
 *   MoveCommand cmd(model, index, dx, dy, dz);
 *   cmd.execute();   // apply the move
 *   cmd.undo();      // reverse it
 */
class MoveCommand
{
public:
    /**
     * @param model   The hull model to operate on
     * @param index   Index of the ControlPoint being moved
     * @param dx      World-space X displacement
     * @param dy      World-space Y displacement
     * @param dz      World-space Z displacement
     */
    MoveCommand(HullModel* model, int index, double dx, double dy, double dz);

    //! Apply the displacement to the point.
    void execute();

    //! Reverse the displacement (apply the negation).
    void undo();

private:
    HullModel* m_model;
    int        m_index;
    double     m_dx, m_dy, m_dz;
};