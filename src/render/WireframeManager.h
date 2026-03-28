#pragma once

//OCCT
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Standard_Handle.hxx>
#include <TColgp_Array1OfPnt.hxx>

//not sure if we need these
#include <vector>
#include <memory>

/**
 * @brief Manages wireframe rendering of control points and hull lines.
 *
 * Provides an interface for adding control points and hull lines as
 * AIS_Polyline objects to the OCCT interactive context. Handles creation,
 * display, and removal of these wireframe elements.
 */

class HullModel; //to avoid dependancy

//interface for observer pattern

class IHullModelObserver {
    public:
        virtual ~IHullModelObserver() = default;

        virtual void onControlPointMoved(int uIndex, int vIndex) = 0; //called when the hull model is updated
};

class WireframeManager : public IHullModelObserver {
    public:
        WireframeManager(const Handle(AIS_InteractiveContext)& context, const std::shared_ptr<HullModel>& hullModel);
        ~WireframeManager() override;

        void BuildLattice(); //builds the initial lattice based on the hull model

        void onControlPointMoved(int uIndex, int vIndex) override; //called when the hull model is updated

    private:

        void UpdateURowPolylines(int uIndex); //updates the AIS_Polyline objects for a specific U row of control points
        void UpdateVRowPolylines(int vIndex); //updates the AIS_Polyline objects for a specific V row of control points

        // core data
        Handle(AIS_InteractiveContext) m_context; //OCCT interactive context for rendering
        std::shared_ptr<HullModel> m_hullModel; //reference to the hull model for accessing control points and hull lines

        //occt object storage
        std::vector<Handle(AIS_Shape)> m_uRowLines; //AIS_Polyline objects representing U rows of control points
        std::vector<Handle(AIS_Shape)> m_vRowLines; //AIS_Polyline objects representing V rows of control points

        bool m_isVisible = true;

};


