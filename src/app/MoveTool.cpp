#include "app/MoveTool.h"

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <Graphic3d_Vec3.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Vec.hxx>


MoveTool::MoveTool(Handle(AIS_InteractiveContext) context, Handle(V3d_View) view, HullModel* model)
     : m_context(context), m_view(view), m_model(model)
{
}

bool MoveTool::onMousePress(int x, int y)
{
    m_context->MoveTo(x, y, m_view, Standard_True);
    m_context->SelectDetected();

    m_context->InitSelected();
    if (!m_context->MoreSelected())
    {
        m_context->ClearSelected(Standard_True);
        m_selectedIndex = -1;
        m_mode = Mode::View;
        return false;
    }

    Handle(AIS_InteractiveObject) picked = m_context->SelectedInteractive();

    for (int i = 0; i < m_model->size(); ++i)
    {
        if (m_model->aisShapeAt(i) == picked)
        {
            m_selectedIndex = i;
            m_mode = Mode::Select;
            m_lastX = x;
            m_lastY = y;

            const ControlPoint& cp = m_model->points()[i];
            m_dragAnchor = gp_Pnt(cp.x, cp.y, cp.z);
            m_accumulated = Graphic3d_Vec3(0.f, 0.f, 0.f);

            return true;  // suppress orbit
        }
    }

    m_selectedIndex = -1;
    m_mode = Mode::View;
    return false;
}

bool MoveTool::onMouseMove(int x, int y)
{
    if (m_mode == Mode::View || m_selectedIndex < 0)
        return false;

    if (m_mode == Mode::Select)
        m_mode = Mode::Move;

    // Project previous and current screen positions onto the camera-facing
    // plane through the drag anchor. Graphic3d_Vec3 carries the world coords.
    Graphic3d_Vec3 prev = projectOntoPlane(m_lastX, m_lastY, m_dragAnchor);
    Graphic3d_Vec3 curr = projectOntoPlane(x, y, m_dragAnchor);

    // Per-frame delta as a Graphic3d_Vec3.
    Graphic3d_Vec3 delta(curr.x() - prev.x(),
                         curr.y() - prev.y(),
                         curr.z() - prev.z());

    // Apply immediately so the point tracks the cursor in real time.
    m_model->movePoint(m_selectedIndex,
                       static_cast<double>(delta.x()),
                       static_cast<double>(delta.y()),
                       static_cast<double>(delta.z()));

    // Accumulate into the drag's running total (committed as one command on release).
    m_accumulated = Graphic3d_Vec3(m_accumulated.x() + delta.x(),
                                   m_accumulated.y() + delta.y(),
                                   m_accumulated.z() + delta.z());

    // Advance the anchor to the new point position for the next frame.
    const ControlPoint& cp = m_model->points()[m_selectedIndex];
    m_dragAnchor = gp_Pnt(cp.x, cp.y, cp.z);

    m_lastX = x;
    m_lastY = y;

    return true;  // suppress orbit
}

void MoveTool::onMouseRelease(int x, int y)
{
    if (m_mode != Mode::Move)
        return;

    // Only commit a command if the point actually moved.
    const float kEpsilon = 1e-6f;
    // if (std::abs(m_accumulated.x()) > kEpsilon ||
    //     std::abs(m_accumulated.y()) > kEpsilon ||
    //     std::abs(m_accumulated.z()) > kEpsilon)
    // {
    //     // The model is already at the final position; store the total
    //     // displacement so MoveCommand::undo() can reverse it in one step.
    //     auto cmd = std::make_unique<MoveCommand>(
    //         m_model,
    //         m_selectedIndex,
    //         static_cast<double>(m_accumulated.x()),
    //         static_cast<double>(m_accumulated.y()),
    //         static_cast<double>(m_accumulated.z()));

    //     m_undoStack.push(std::move(cmd));

    //     // A new drag invalidates the redo history.
    //     while (!m_redoStack.empty())
    //         m_redoStack.pop();
    // }

    m_accumulated = Graphic3d_Vec3(0.f, 0.f, 0.f);
    m_mode = Mode::Select;
}

// void MoveTool::undo()
// {
//     if (m_undoStack.empty())
//         return;

//     auto& cmd = m_undoStack.top();
//     cmd->undo();
//     m_redoStack.push(std::move(m_undoStack.top()));
//     m_undoStack.pop();
// }

// void MoveTool::redo()
// {
//     if (m_redoStack.empty())
//         return;

//     auto& cmd = m_redoStack.top();
//     cmd->execute();
//     m_undoStack.push(std::move(m_redoStack.top()));
//     m_redoStack.pop();
// }

Graphic3d_Vec3 MoveTool::projectOntoPlane(int x, int y, const gp_Pnt& anchor) const
{
    // Shoot a ray from the camera through the screen pixel.
    gp_Pnt rayOrigin;
    gp_Dir rayDir;
    m_view->ConvertWithProj(x, y, rayOrigin, rayDir);

    // Plane through `anchor`, normal = camera direction (faces the viewer).
    gp_Dir camDir = m_view->Camera()->Direction();
    gp_Vec n(camDir);
    gp_Vec orig(rayOrigin.X(), rayOrigin.Y(), rayOrigin.Z());
    gp_Vec anchorVec(anchor.X(), anchor.Y(), anchor.Z());

    // Ray-plane intersection: t = (n · (anchor - origin)) / (n · dir)
    double denom = n.Dot(gp_Vec(rayDir));
    double t = 0.0;
    if (std::abs(denom) > 1e-10)
        t = n.Dot(anchorVec - orig) / denom;

    // Return the hit point as a Graphic3d_Vec3 for use in delta arithmetic.
    return Graphic3d_Vec3(
        static_cast<float>(rayOrigin.X() + t * rayDir.X()),
        static_cast<float>(rayOrigin.Y() + t * rayDir.Y()),
        static_cast<float>(rayOrigin.Z() + t * rayDir.Z()));
}
