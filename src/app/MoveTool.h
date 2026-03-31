#pragma once

#include <stack>
#include <memory>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_Vec3.hxx>
#include <V3d_View.hxx>
#include <gp_Pnt.hxx>

#include "core/HullModel.h"
#include "render/VisualPoint.h"
#include "app/MoveCommand.h"

/**
 * @brief Manages point selection and dragging in the 3D viewport.
 *
 * Tool modes:
 *   View   — default; no point selected; mouse events control camera orbit
 *   Select — a point has been picked but the mouse hasn't moved yet
 *   Move   — an active drag is in progress
 */
class MoveTool
{
public:
    enum class Mode { View, Select, Move };

    /**
     * @param context  OCCT interactive context used for hit-testing
     * @param view     3D view used for coordinate projection
     * @param model    Hull model whose points will be mutated
     */
    MoveTool(Handle(AIS_InteractiveContext) context, Handle(V3d_View) view, HullModel* model);

    /**
     * @brief Handle a left-button press at screen position (x, y).
     * @return true if a point was selected (caller should suppress camera orbiting)
     */
    bool onMousePress(int x, int y);

    /**
     * @brief Handle mouse movement.
     *
     * Computes the 3D displacement using Graphic3d_Vec3 and the camera
     * projection plane, then applies it to the selected point.
     *
     * @return true if a drag was applied (caller should suppress camera orbiting)
     */
    bool onMouseMove(int x, int y);

    /**
     * @brief Handle left-button release.
     *
     * Commits the accumulated drag as a single MoveCommand onto the
     * undo stack so the entire drag can be reversed in one Ctrl+Z.
     */
    void onMouseRelease(int x, int y);



    // Undo / Redo - Ask if this should be moved up to a bigger structure and not specific to this
 
    // //! Reverse the most recent completed drag. No-op if stack is empty.
    // void undo();

    // //! Re-apply the most recently undone drag. No-op if redo stack is empty.
    // void redo();

    // bool canUndo() const { return !m_undoStack.empty(); }
    // bool canRedo() const { return !m_redoStack.empty(); }


    Mode mode()          const { return m_mode; }
    bool hasSelection()  const { return m_selectedIndex != nullptr; }

private:
    /**
     * @brief Project a screen pixel onto the plane through `anchor`
     *        perpendicular to the camera, returning the world-space
     *        hit point as a Graphic3d_Vec3.
     */
    Graphic3d_Vec3 projectOntoPlane(int x, int y, const gp_Pnt& anchor) const;

    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_View) m_view;
    HullModel* m_model;

    Mode m_mode = Mode::View;

    ControlPoint* m_selectedPoint = nullptr;

    // Screen position of the last move event
    int m_lastX = 0;
    int m_lastY = 0;

    // World-space anchor at the start of the current drag
    gp_Pnt m_dragAnchor;

    // // Accumulated total displacement for the drag in progress (committed on release)
    // Graphic3d_Vec3 m_accumulated { 0.f, 0.f, 0.f };

    // // Command stacks for undo/redo
    // std::stack<std::unique_ptr<MoveCommand>> m_undoStack;
    // std::stack<std::unique_ptr<MoveCommand>> m_redoStack;
};