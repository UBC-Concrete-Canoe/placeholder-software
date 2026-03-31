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
        m_selectedIndex = nullptr;
        m_mode = Mode::View;
        return false;
    }

    Handle(AIS_InteractiveObject) picked = m_context->SelectedInteractive();
    Handle(VisualPoint) vp = Handle(VisualPoint)::DownCast(picked);

    if (vp.IsNull())
    {
        // Clicked something that isn't a VisualPoint - ignore.
        m_selectedPoint = nullptr;
        m_mode = Mode::View;
        return false;
    }
 
    // Find the matching ControlPoint in the model by comparing positions.
    m_selectedPoint = nullptr;
    for (int u = 0; u < m_model->getUCount(); ++u)
    {
        for (int v = 0; v < m_model->getVCount(); ++v)
        {
            ControlPoint& cp = m_model->getPoint(u, v);
            if (cp.getPosition().Distance(vp->point()) < 1e-6)
            {
                m_selectedPoint = &cp;
                break;
            }
        }
        if (m_selectedPoint) break;
    }
 
    if (!m_selectedPoint)
    {
        m_mode = Mode::View;
        return false;
    }
 
    m_mode = Mode::Select;
    m_lastX = x;
    m_lastY = y;
    m_dragAnchor = m_selectedPoint->getPosition();
 
    return true;  // suppress orbit
}

bool MoveTool::onMouseMove(int x, int y)
{
    if (m_mode == Mode::View || !m_selectedPoint)
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

    const gp_Pnt& current = m_selectedPoint->getPosition();
    gp_Pnt newPos(
        current.X() + static_cast<double>(delta.x()),
        current.Y() + static_cast<double>(delta.y()),
        current.Z() + static_cast<double>(delta.z()));
 
    m_selectedPoint->setPosition(newPos);
 
    // Advance the anchor to the new position for the next frame.
    m_dragAnchor = newPos;
    m_lastX      = x;
    m_lastY      = y;
 
    return true;  // suppress orbit
}

void MoveTool::onMouseRelease(int x, int y)
{
    if (m_mode == Mode::Move)
        m_mode = Mode::Select;
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
