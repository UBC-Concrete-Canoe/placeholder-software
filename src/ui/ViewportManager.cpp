#include "ViewportManager.h"
#include <gp_Pnt.hxx>
#include "MainWindow.h"
#include "OcctWidget.h"
#include "app/ViewportController.h"

ViewportManager::ViewportManager()
{
	// Create viewports
	persp_viewport = std::make_unique<OcctViewport>();
	plan_viewport = std::make_unique<OcctViewport>();
	profile_viewport = std::make_unique<OcctViewport>();
	bodyplan_viewport = std::make_unique<OcctViewport>();

	// Create the controllers to handle user input
	persp_controller = std::make_unique<ViewportController>(persp_viewport.get());
	plan_controller = std::make_unique<ViewportController>(plan_viewport.get());
	profile_controller = std::make_unique<ViewportController>(profile_viewport.get());
	bodyplan_controller = std::make_unique<ViewportController>(bodyplan_viewport.get());

	// Keep planar views movable (pan/zoom), but prevent orbit rotation.
	plan_controller->setRotationEnabled(false);
	profile_controller->setRotationEnabled(false);
	bodyplan_controller->setRotationEnabled(false);
}

ViewportManager::~ViewportManager() = default;

void
ViewportManager::initializeViewport(MainWindow* window)
{
	// Locate the widgets from Qt
	OcctWidget* persp_widget = window->findChild<OcctWidget*>(
		"renderWidget"
	); // This must change if the promoted widget in Qt changes name.
	OcctWidget* plan_widget = window->findChild<OcctWidget*>("planWidget");
	OcctWidget* profile_widget = window->findChild<OcctWidget*>("profileWidget");
	OcctWidget* bodyplan_widget = window->findChild<OcctWidget*>("bodyplanWidget");

	if (!persp_widget || !plan_widget || !profile_widget || !bodyplan_widget)
	{
		return;
	}

	// Ensure Qt creates a native handle; required for OCCT embedding
	(void)persp_widget->winId();
	(void)plan_widget->winId();
	(void)profile_widget->winId();
	(void)bodyplan_widget->winId();

	// Connect widgets to input controllers
	persp_widget->setController(persp_controller.get());
	plan_widget->setController(plan_controller.get());
	profile_widget->setController(profile_controller.get());
	bodyplan_widget->setController(bodyplan_controller.get());

	// Initialize OCCT within the widget's native window
	persp_viewport->initialize(persp_widget->winId());

	// Obtain context so that the other three viewing planes can be dependent on the main 3D view
	Handle(AIS_InteractiveContext) sharedCtx = persp_viewport->getContext();

	// Change perspective view from OCCT's default Orthographic to Perspective
	// NOTE: Having issues setting this up.
	Handle(V3d_View) int_persp_view = persp_viewport->getView();
	int_persp_view->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);

	// Initialize three planar views based on main 3D view
	plan_viewport->initialize(plan_widget->winId(), sharedCtx);
	profile_viewport->initialize(profile_widget->winId(), sharedCtx);
	bodyplan_viewport->initialize(bodyplan_widget->winId(), sharedCtx);
}

void
ViewportManager::set_planars()
{
	// Set viewing angles
	plan_viewport->setViewPreset(V3d_Zpos);     // down Z-axis
	profile_viewport->setViewPreset(V3d_Yneg);  // down Y-axis
	bodyplan_viewport->setViewPreset(V3d_Xpos); // down X-axis

	// Fit planar views to shape
	plan_viewport->fitAll();
	profile_viewport->fitAll();
	bodyplan_viewport->fitAll();
}

void
ViewportManager::create_demo_hull_model()
{
	if (!persp_viewport || !persp_viewport->getContext())
	{
		return;
	}

	// Build a small arched lattice so both points and wireframe are easy to verify.
	const int uCount = 4;
	const int vCount = 4;
	const double spacing = 30.0;
	demo_hull_model = std::make_shared<HullModel>(uCount, vCount);

	int pointId = 0;
	for (int u = 0; u < uCount; ++u)
	{
		for (int v = 0; v < vCount; ++v)
		{
			const bool interior = (u > 0 && u < (uCount - 1) && v > 0 && v < (vCount - 1));
			const double zHeight = interior ? 10.0 : 0.0;
			demo_hull_model->updatePoint(pointId++, gp_Pnt(u * spacing, v * spacing, zHeight));
		}
	}

	for (std::size_t i = 0; i < demo_hull_model->pointCount(); ++i)
	{
		persp_viewport->displayControlPoint(demo_hull_model->pointAt(i));
	}

	demo_wireframe_manager =
		std::make_unique<WireframeManager>(persp_viewport->getContext(), demo_hull_model);
	demo_wireframe_manager->BuildLattice();
	persp_viewport->fitAll();
}