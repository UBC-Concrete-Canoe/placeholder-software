#include "OcctViewport.h"
#include "core/ControlPoint.h"
#include "render/OcctStyleMapper.h"
#include "render/VisualPoint.h"

// OCCT Core
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_AmbientLight.hxx>
#include <V3d_DirectionalLight.hxx>

// Platform Specific Windows
#ifdef _WIN32
#include <WNT_Window.hxx>
#elif defined(__APPLE__)
#include <Cocoa_Window.hxx>
#else
#include <Xw_Window.hxx>
#endif

namespace
{
std::shared_ptr<std::vector<Handle(VisualPoint)>> gSharedVisualPoints =
	std::make_shared<std::vector<Handle(VisualPoint)>>();
}

OcctViewport::OcctViewport()
  : myVisualPoints(gSharedVisualPoints)
{}

// Initialize 1: Create the main 3D render
void
OcctViewport::initialize(WId windowHandle)
{
	// Create the graphics driver and viewer
	Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
	Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(displayConnection);
	myViewer = new V3d_Viewer(graphicDriver);

	// Set up directional and ambient lighting
	Handle(V3d_DirectionalLight) lightDir =
		new V3d_DirectionalLight(V3d_Zneg, Quantity_Color(Quantity_NOC_GRAY97), 1);
	lightDir->SetDirection(1.0, -2.0, -10.0);
	Handle(V3d_AmbientLight) lightAmb = new V3d_AmbientLight();

	myViewer->AddLight(lightDir);
	myViewer->AddLight(lightAmb);
	myViewer->SetLightOn(lightDir);
	myViewer->SetLightOn(lightAmb);

	// Create the interactive context for rendering objects
	myContext = new AIS_InteractiveContext(myViewer);
	myContext->DefaultDrawer()->SetFaceBoundaryDraw(true);
	myContext->SetPixelTolerance(8);

	// Keep backend highlight in sync with renderer-agnostic style intent.
	const Quantity_Color hoverColor = OcctStyleMapper::toOcctColor(myControlPointStyle.hoverColor);
	const Quantity_Color selectedColor =
		OcctStyleMapper::toOcctColor(myControlPointStyle.selectedColor);
	Handle(Prs3d_PointAspect) hoverPointAspect = OcctStyleMapper::makePointAspect(
		myControlPointStyle.hoverColor, myControlPointStyle.selectedMarkerScale
	);
	Handle(Prs3d_PointAspect) selectedPointAspect = OcctStyleMapper::makePointAspect(
		myControlPointStyle.selectedColor, myControlPointStyle.selectedMarkerScale
	);
	const Handle(Prs3d_Drawer) & dynamicHighlight =
		myContext->HighlightStyle(Prs3d_TypeOfHighlight_Dynamic);
	dynamicHighlight->SetColor(hoverColor);
	dynamicHighlight->SetPointAspect(hoverPointAspect);

	const Handle(Prs3d_Drawer) & selectedHighlight =
		myContext->HighlightStyle(Prs3d_TypeOfHighlight_Selected);
	selectedHighlight->SetColor(selectedColor);
	selectedHighlight->SetPointAspect(selectedPointAspect);

	myView = myViewer->CreateView();

	setupView(windowHandle); // Embed into Qt's window handle
}

// Initialize 2: Create the planar renders
void
OcctViewport::initialize(WId windowHandle, Handle(AIS_InteractiveContext) sharedContext)
{
	myContext = sharedContext; // Instead of creating a new context, it uses the shared one which
							   // allows all views to sync up
	myViewer = myContext->CurrentViewer();
	myView = myViewer->CreateView();

	setupView(windowHandle); // Embed into Qt's window handle
}

// Embed into Qt's window handle, placed here to reduce clutter
void
OcctViewport::setupView(WId windowHandle)
{
	myView->SetImmediateUpdate(false);
	myView->SetShadingModel(V3d_PHONG);
	myView->SetBackgroundColor(Quantity_NOC_BLACK);

// Embed the view into the Qt widget's native window handle
#ifdef _WIN32
	Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)windowHandle);
#elif defined(Q_OS_MAC)
	Handle(Cocoa_Window) wind = new Cocoa_Window((NSView*)windowHandle);
#else
	// Display connection can be found from the graphic driver
	Handle(Aspect_DisplayConnection) displayConnection = myViewer->Driver()->GetDisplayConnection();
	Handle(Xw_Window) wind = new Xw_Window(displayConnection, (Aspect_Drawable)windowHandle);
#endif

	myView->SetWindow(wind);
	if (!wind->IsMapped())
	{
		wind->Map();
	}
}

void
OcctViewport::displayShape(const TopoDS_Shape& shape)
{
	Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
	myContext->Display(aisShape, true);
	myView->FitAll();
}

void
OcctViewport::displayControlPoint(const ControlPoint* point)
{
	if (myContext.IsNull() || point == nullptr)
	{
		return;
	}

	Handle(VisualPoint) visualPoint = new VisualPoint(point, myControlPointStyle);
	myContext->Display(visualPoint, Standard_False);
	myContext->Activate(visualPoint, 0, Standard_False);
	myVisualPoints->push_back(visualPoint);
	myView->FitAll();
	myContext->UpdateCurrentViewer();
}

void
OcctViewport::setControlPointStyle(const ControlPointVisualStyle& style)
{
	myControlPointStyle = style;
}

void
OcctViewport::removeAll()
{
	if (!myContext.IsNull())
	{
		myContext->RemoveAll(true);
		myVisualPoints->clear();
	}
}

void
OcctViewport::fitAll()
{
	if (!myView.IsNull())
	{
		myView->FitAll();
	}
}

void
OcctViewport::fitSelected()
{
	if (!myContext.IsNull())
	{
		myContext->FitSelected(myView);
	}
}

void
OcctViewport::setViewPreset(V3d_TypeOfOrientation orientation)
{
	if (!myView.IsNull())
	{
		myView->SetProj(orientation);
		myView->Redraw();
	}
}

void
OcctViewport::setShadingMode(bool wireframe)
{
	if (myContext.IsNull())
	{
		return;
	}

	// Select the display mode: AIS_WireFrame or AIS_Shaded
	const int dm = wireframe ? AIS_WireFrame : AIS_Shaded;

	// Apply to all objects if nothing is selected, otherwise apply to selection
	if (myContext->NbSelected() == 0)
	{
		myContext->SetDisplayMode(dm, false);
	}
	else
	{
		for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
		{
			myContext->SetDisplayMode(myContext->SelectedInteractive(), dm, false);
		}
	}
	myContext->UpdateCurrentViewer();
}

void
OcctViewport::redraw()
{
	if (!myView.IsNull())
	{
		myView->Redraw();
	}
}

void
OcctViewport::synchronizeVisualPoints()
{
	if (myContext.IsNull() || !myVisualPoints || myVisualPoints->empty())
	{
		return;
	}

	bool anyUpdated = false;
	for (const Handle(VisualPoint) & point : *myVisualPoints)
	{
		if (point.IsNull())
		{
			continue;
		}

		const gp_Pnt before = point->point();
		point->synchronize();
		if (before.Distance(point->point()) > 0.0)
		{
			myContext->Redisplay(point, Standard_False);
			anyUpdated = true;
		}
	}

	if (updateVisualPointSelectionStyles())
	{
		anyUpdated = true;
	}

	if (anyUpdated)
	{
		myContext->UpdateCurrentViewer();
	}
}

bool
OcctViewport::updateVisualPointSelectionStyles()
{
	bool anyUpdated = false;

	for (const Handle(VisualPoint) & point : *myVisualPoints)
	{
		if (point.IsNull())
		{
			continue;
		}

		const bool isSelected = myContext->IsSelected(point);
		if (point->setSelectedStyle(isSelected))
		{
			myContext->Redisplay(point, Standard_False);
			anyUpdated = true;
		}
	}

	return anyUpdated;
}
