#include "%{ActivityHdr}"

#include "ep/cpp/component/view.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/component/node/geomnode.h"
#include "ep/cpp/component/primitivegenerator.h"

using namespace ep;

namespace %{Namespace} {

Array<const PropertyInfo> %{ActivityName}::getProperties() const
{
  return { };
}

Array<const ep::MethodInfo> %{ActivityName}::getMethods() const
{
  return { };
}


Array<const EventInfo> %{ActivityName}::getEvents() const
{
  return { };
}

Array<const StaticFuncInfo> %{ActivityName}::getStaticFuncs() const
{
  return { };
}

%{ActivityName}::%{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  // Setup the Scene, Camera and View Components
  ViewRef spView = pKernel->createComponent<View>();
  spScene = pKernel->createComponent<Scene>();
  CameraRef spCamera = pKernel->createComponent<SimpleCamera>();
  spCamera->setPerspective(1.0471975512);
  spCamera->setMatrix({ 1, 0, 0, 0, -0, 1, 0, 0, 0, -0, 1, 0, 0.59999999999999998, -2.553059046268463, 0.51000000000000001, 1 });
  spView->setScene(spScene);
  spView->setCamera(spCamera);

  // Add a test cube to the scene
  // Vertex Shader
  String vertexShader = "attribute vec3 a_position;\\n"
                        "attribute vec4 a_color;\\n"
                        "varying vec4 v_color;\\n"
                        "uniform mat4 u_mfwvp;\\n"
                        "void main()\\n"
                        "{\\n"
                        "  v_color = a_color;\\n"
                        "  gl_Position = u_mfwvp * vec4(a_position, 1.0);\\n"
                        "}\\n";

  // Pixel Shader
  String pixelShader = "varying vec4 v_color;\\n"
                       "void main()\\n"
                       "{\\n"
                       "  gl_FragColor = v_color;\\n"
                       "}\\n";

  // Material
  MaterialRef spMaterial = pKernel->createComponent<Material>();
  spMaterial->setShader(ShaderType::VertexShader, vertexShader);
  spMaterial->setShader(ShaderType::PixelShader, pixelShader);

  // Generate Cube
  ArrayBufferRef spVertexBuffer = pKernel->createComponent<ArrayBuffer>();
  ArrayBufferRef spIndexBuffer = pKernel->createComponent<ArrayBuffer>();

  PrimitiveGenerator::generateCube(spVertexBuffer, spIndexBuffer);

  MetadataRef spMetadata = spVertexBuffer->getMetadata();
  spMetadata->get("attributeInfo")[0].insertItem("name", "a_position");

  // Colour Buffer
  ArrayBufferRef spColourBuffer = pKernel->createComponent<ArrayBuffer>();
  spColourBuffer->allocateFromData(Slice<const Float4>{
    Float4{ 1.0f, 1.0f, 1.0f, 1.0f },
    Float4{ 1.0f, 0.0f, 0.0f, 1.0f },
    Float4{ 0.0f, 1.0f, 0.0f, 1.0f },
    Float4{ 0.0f, 0.0f, 1.0f, 1.0f },

    Float4{ 1.0f, 1.0f, 0.0f, 1.0f },
    Float4{ 1.0f, 0.0f, 1.0f, 1.0f },
    Float4{ 0.0f, 1.0f, 1.0f, 1.0f },
    Float4{ 0.5f, 0.5f, 1.0f, 1.0f }
  });
  spMetadata = spColourBuffer->getMetadata();
  spMetadata->get("attributeInfo")[0].insertItem("name", "a_color");

  ModelRef spModel = pKernel->createComponent<Model>();
  spModel->addVertexArray(spVertexBuffer);
  spModel->addVertexArray(spColourBuffer);
  spModel->setIndexArray(spIndexBuffer);
  spModel->setMaterial(spMaterial);
  spModel->setRenderList(RenderList { PrimType::Triangles, size_t(0), size_t(0), spIndexBuffer->getLength() });

  GeomNodeRef spTestCube = pKernel->createComponent<GeomNode>();
  spTestCube->setModel(spModel);

  spScene->getRootNode()->addChild(spTestCube);

  // Create the Viewport UIComponent - this provides a renderable frame buffer object in the UI of our view component
  UIComponentRef spViewport;
  epscope(fail) { if (!spViewport) pKernel->logError("Error creating Viewport Component"); };
  spViewport = component_cast<UIComponent>(pKernel->createComponent("ui.Viewport", Variant::VarMap{ { "view", spView } }));

  // Create the Main UIComponent for the activity and attach the viewport ui component
  UIComponentRef sp%{ActivityName}UI;
  epscope(fail) { if(!sp%{ActivityName}UI) pKernel->logError("Error creating %{ActivityName} UI Component"); };
  sp%{ActivityName}UI = component_cast<UIComponent>(pKernel->createComponent("%{Namespace}.MainUI"));
  sp%{ActivityName}UI->set("viewport", spViewport);

  // Associate the Main UIComponent with our Activity Component
  setUI(sp%{ActivityName}UI);
}

void %{ActivityName}::activate()
{
  getKernel().updatePulse.subscribe(Delegate<void(double)>(this, &%{ActivityName}::update));
}

void %{ActivityName}::deactivate()
{
  getKernel().updatePulse.unsubscribe(Delegate<void(double)>(this, &%{ActivityName}::update));
}

void %{ActivityName}::update(double timeStep)
{
  if (spScene)
    spScene->update(timeStep);
}

Variant %{ActivityName}::save() const
{
  return nullptr;
}

extern "C" bool epPluginAttach()
{
  // Register the Activity Component
  Kernel::getInstance()->registerComponentType<%{ActivityName}>();

  // Register the plugin's QML Components
  Kernel::getInstance()->call("registerQmlComponents", ":/%{Namespace}");
  return true;
}

} // namespace %{Namespace}
