
#include "WireframeManager.h"
#include "core/HullModel.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

// NEW: Required to build a continuous wireframe line out of points
#include <BRepBuilderAPI_MakePolygon.hxx>


WireframeManager::WireframeManager(const Handle(AIS_InteractiveContext)& context, const std::shared_ptr<HullModel>& hullModel)
    : m_context(context), m_hullModel(hullModel)
{
    // TODO: Subscribe to the model so onControlPointMoved gets called
}

WireframeManager::~WireframeManager()
{
    // Clean up memory from the OCCT viewer
    for (auto& line : m_uRowLines) {
        if (!line.IsNull()) m_context->Remove(line, Standard_False);
    }
    for (auto& line : m_vRowLines) {
        if (!line.IsNull()) m_context->Remove(line, Standard_False);
    }
    m_context->UpdateCurrentViewer();
}

void WireframeManager::BuildLattice()
{
    // TODO: Implement lattice building from hull model

    if (!m_hullModel) return;

    int uCount = m_hullModel->getUCount();
    int vCount = m_hullModel->getVCount();

    // Size the vectors to match the grid
    m_uRowLines.assign(uCount, Handle(AIS_Shape)());
    m_vRowLines.assign(vCount, Handle(AIS_Shape)());

    // Generate the initial grid
    for (int u = 0; u < uCount; ++u) UpdateURowPolylines(u);
    for (int v = 0; v < vCount; ++v) UpdateVRowPolylines(v);
    
    m_context->UpdateCurrentViewer();
}

void WireframeManager::onControlPointMoved(int uIndex, int vIndex)
{
    UpdateURowPolylines(uIndex);
    UpdateVRowPolylines(vIndex);
}

void WireframeManager::UpdateURowPolylines(int uIndex)
{
    int vCount = m_hullModel->getVCount(); 
    if (vCount < 2) return; 

    // 1. Build a Polygon Wire from the grid points
    BRepBuilderAPI_MakePolygon polyMaker;
    for (int v = 0; v < vCount; ++v) {
        polyMaker.Add(m_hullModel->getPoint(uIndex, v).getPosition());
    }
    polyMaker.Build();

    // 2. Display or Update the AIS_Shape
    if (m_uRowLines[uIndex].IsNull()) {
        m_uRowLines[uIndex] = new AIS_Shape(polyMaker.Shape());
        if (m_isVisible) m_context->Display(m_uRowLines[uIndex], Standard_False);
    } else {
        // PERFORMANCE REQUIREMENT MET: Update existing geometry without deleting
        // Note: If m_uRowLines[uIndex]->Set() throws an error in your specific 
        // OCCT version, change it to m_uRowLines[uIndex]->SetShape()
        m_uRowLines[uIndex]->SetShape(polyMaker.Shape()); 
        m_context->Redisplay(m_uRowLines[uIndex], Standard_False);
    }
}

void WireframeManager::UpdateVRowPolylines(int vIndex)
{
   int uCount = m_hullModel->getUCount();
    if (uCount < 2) return; 

    BRepBuilderAPI_MakePolygon polyMaker;
    for (int u = 0; u < uCount; ++u) {
        polyMaker.Add(m_hullModel->getPoint(u, vIndex).getPosition());
    }
    polyMaker.Build();

    if (m_vRowLines[vIndex].IsNull()) {
        m_vRowLines[vIndex] = new AIS_Shape(polyMaker.Shape());
        if (m_isVisible) m_context->Display(m_vRowLines[vIndex], Standard_False);
    } else {
        m_vRowLines[vIndex]->SetShape(polyMaker.Shape()); 
        m_context->Redisplay(m_vRowLines[vIndex], Standard_False);
    }

}