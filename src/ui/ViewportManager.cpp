#include <memory>
#include "OcctWidget.h"
#include "ViewportManager.h"

ViewportManager::ViewportManager() {
    //Create viewports
    persp_viewport = std::make_unique<OcctViewport>();
	plan_viewport = std::make_unique<OcctViewport>();
	profile_viewport = std::make_unique<OcctViewport>();
	bodyplan_viewport = std::make_unique<OcctViewport>();

    // Create the controllers to handle user input
	persp_controller = std::make_unique<ViewportController>(persp_viewport.get());
	plan_controller = std::make_unique<ViewportController>(plan_viewport.get());
	profile_controller = std::make_unique<ViewportController>(profile_viewport.get());
	bodyplan_controller = std::make_unique<ViewportController>(bodyplan_viewport.get());
}

void ViewportManager::initializeViewport(MainWindow* window) {
    //Locate the widgets from Qt
    OcctWidget* persp_widget = window->findChild<OcctWidget*>("renderWidget"); //This must change if the promoted widget in Qt changes name.
	OcctWidget* plan_widget = window->findChild<OcctWidget*>("planWidget");
	OcctWidget* profile_widget = window->findChild<OcctWidget*>("profileWidget");
	OcctWidget* bodyplan_widget = window->findChild<OcctWidget*>("bodyplanWidget");

	//Change perspective view from OCCT's default Orthographic to Perspective
	//NOTE: Having issues setting this up. 
	Handle(V3d_View) int_persp_view = persp_viewport->getView();
	int_persp_view->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);

    // Ensure Qt creates a native handle; required for OCCT embedding
	(void)persp_widget->winId();
	(void)plan_widget->winId();
	(void)profile_widget->winId();
	(void)bodyplan_widget->winId();

    //Connect widgets to input controllers
    persp_widget->setController(persp_controller.get());
	plan_widget->setController(plan_controller.get());
	profile_widget->setController(profile_controller.get());
	bodyplan_widget->setController(bodyplan_controller.get());

    // Initialize OCCT within the widget's native window
	persp_viewport->initialize(persp_widget->winId());

	//Obtain context so that the other three viewing planes can be dependent on the main 3D view
	Handle(AIS_InteractiveContext) sharedCtx = persp_viewport->getContext();

	//Initialize three planar views based on main 3D view
	plan_viewport->initialize(plan_widget->winId(), sharedCtx);
	profile_viewport->initialize(profile_widget->winId(), sharedCtx);
	bodyplan_viewport->initialize(bodyplan_widget->winId(), sharedCtx);
}

void ViewportManager::create_shape(TopoDS_Shape shape) {
    persp_viewport->displayShape(shape);
}

void ViewportManager::set_planars() {
    //Set viewing angles
	plan_viewport->setViewPreset(V3d_Zpos); //down Z-axis
	profile_viewport->setViewPreset(V3d_Yneg); //down Y-axis
	bodyplan_viewport->setViewPreset(V3d_Xpos); //down X-axis

	//Fit planar views to shape
	plan_viewport->fitAll();
	profile_viewport->fitAll();
	bodyplan_viewport->fitAll();
}