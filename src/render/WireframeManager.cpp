
#include "WireframeManager.h"
#include "HullModel.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_Polyline.hxx>
#include <TColgp_Array1OfPnt.hxx>

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
    m_uRowLines.assign(uCount, Handle(AIS_Polyline)());
    m_vRowLines.assign(vCount, Handle(AIS_Polyline)());

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
    // TODO: Update AIS_Polyline objects for U row at uIndex

   int vCount = m_hullModel->getVCount(); // Updated getter
    if (vCount < 2) return; 

    TColgp_Array1OfPnt points(1, vCount);
    for (int v = 0; v < vCount; ++v) {
        // UPDATED: Fetch the ControlPoint, then extract the gp_Pnt
        points.SetValue(v + 1, m_hullModel->getPoint(uIndex, v).getPosition());
    }

    if (m_uRowLines[uIndex].IsNull()) {
        m_uRowLines[uIndex] = new AIS_Polyline(points);
        if (m_isVisible) m_context->Display(m_uRowLines[uIndex], Standard_False);
    } else {
        m_uRowLines[uIndex]->SetPoints(points);
        m_context->Redisplay(m_uRowLines[uIndex], Standard_False);
    }
}

void WireframeManager::UpdateVRowPolylines(int vIndex)
{
    // TODO: Update AIS_Polyline objects for V row at vIndex

    int uCount = m_hullModel->getUCount();
    if (uCount < 2) return; 

    TColgp_Array1OfPnt points(1, uCount);
    for (int u = 0; u < uCount; ++u) {
        points.SetValue(u + 1, m_hullModel->getPoint(u, vIndex).getPosition());
    }

    if (m_vColLines[vIndex].IsNull()) {
        m_vColLines[vIndex] = new AIS_Polyline(points);
        m_context->Display(m_vColLines[vIndex], Standard_False);
    } else {
        m_vColLines[vIndex]->SetPoints(points);
        m_context->Redisplay(m_vColLines[vIndex], Standard_False);
    }
}