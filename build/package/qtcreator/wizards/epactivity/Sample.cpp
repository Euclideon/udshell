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

Array<const PropertyInfo> %{ActivityName}::GetProperties() const
{
  return { };
}

Array<const ep::MethodInfo> %{ActivityName}::GetMethods() const
{
  return { };
}


Array<const EventInfo> %{ActivityName}::GetEvents() const
{
  return { };
}

Array<const StaticFuncInfo> %{ActivityName}::GetStaticFuncs() const
{
  return { };
}

%{ActivityName}::%{ActivityName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  // Setup the Scene, Camera and View Components
  ViewRef spView = pKernel->CreateComponent<View>();
  spScene = pKernel->CreateComponent<Scene>();
  CameraRef spCamera = pKernel->CreateComponent<SimpleCamera>();
  spCamera->SetPerspective(1.0471975512);
  spCamera->SetMatrix({ 1, 0, 0, 0, -0, 1, 0, 0, 0, -0, 1, 0, 0.59999999999999998, -2.553059046268463, 0.51000000000000001, 1 });
  spView->SetScene(spScene);
  spView->SetCamera(spCamera);

  // Add a test cube to the scene
  // Vertex Shader
  ShaderRef spVertexShader = pKernel->CreateComponent<Shader>();
  {
    spVertexShader->SetType(ShaderType::VertexShader);

    const char shaderText[] = "attribute vec3 a_position;\\n"
                              "attribute vec4 a_color;\\n"
                              "varying vec4 v_color;\\n"
                              "uniform mat4 u_mfwvp;\\n"
                              "void main()\\n"
                              "{\\n"
                              "  v_color = a_color;\\n"
                              "  gl_Position = u_mfwvp * vec4(a_position, 1.0);\\n"
                              "}\\n";
    spVertexShader->SetCode(shaderText);
  }

  // Pixel Shader
  ShaderRef spPixelShader = pKernel->CreateComponent<Shader>();
  {
    spPixelShader->SetType(ShaderType::PixelShader);

    const char shaderText[] = "varying vec4 v_color;\\n"
                              "void main()\\n"
                              "{\\n"
                              "  gl_FragColor = v_color;\\n"
                              "}\\n";
    spPixelShader->SetCode(shaderText);
  }

  // Material
  MaterialRef spMaterial = pKernel->CreateComponent<Material>();
  spMaterial->SetShader(ShaderType::VertexShader, spVertexShader);
  spMaterial->SetShader(ShaderType::PixelShader, spPixelShader);

  // Generate Cube
  ArrayBufferRef spVertexBuffer = pKernel->CreateComponent<ArrayBuffer>();
  ArrayBufferRef spIndexBuffer = pKernel->CreateComponent<ArrayBuffer>();

  PrimitiveGenerator::GenerateCube(spVertexBuffer, spIndexBuffer);

  MetadataRef spMetadata = spVertexBuffer->GetMetadata();
  spMetadata->Get("attributeinfo")[0].insertItem("name", "a_position");

  // Colour Buffer
  ArrayBufferRef spColourBuffer = pKernel->CreateComponent<ArrayBuffer>();
  {
    spColourBuffer->AllocateFromData(Slice<const Float4>{
      Float4{ 1.0f, 1.0f, 1.0f, 1.0f },
      Float4{ 1.0f, 0.0f, 0.0f, 1.0f },
      Float4{ 0.0f, 1.0f, 0.0f, 1.0f },
      Float4{ 0.0f, 0.0f, 1.0f, 1.0f },

      Float4{ 1.0f, 1.0f, 0.0f, 1.0f },
      Float4{ 1.0f, 0.0f, 1.0f, 1.0f },
      Float4{ 0.0f, 1.0f, 1.0f, 1.0f },
      Float4{ 0.5f, 0.5f, 1.0f, 1.0f }
    });
    spMetadata = spColourBuffer->GetMetadata();
    spMetadata->Get("attributeinfo")[0].insertItem("name", "a_color");
  }

  ModelRef spModel = pKernel->CreateComponent<Model>();
  spModel->AddVertexArray(spVertexBuffer);
  spModel->AddVertexArray(spColourBuffer);
  spModel->SetIndexArray(spIndexBuffer);
  spModel->SetMaterial(spMaterial);
  spModel->SetRenderList(RenderList { PrimType::Triangles, size_t(0), size_t(0), spIndexBuffer->GetLength() });

  GeomNodeRef spTestCube = pKernel->CreateComponent<GeomNode>();
  spTestCube->SetModel(spModel);

  spScene->GetRootNode()->AddChild(spTestCube);

  // Create the Viewport UIComponent - this provides a renderable frame buffer object in the UI of our view component
  UIComponentRef spViewport;
  epscope(fail) { if (!spViewport) pKernel->LogError("Error creating Viewport Component"); };
  spViewport = component_cast<UIComponent>(pKernel->CreateComponent("ui.viewport", Variant::VarMap{ { "view", spView } }));

  // Create the Main UIComponent for the activity and attach the viewport ui component
  UIComponentRef sp%{ActivityName}UI;
  epscope(fail) { if(!sp%{ActivityName}UI) pKernel->LogError("Error creating %{ActivityName} UI Component"); };
  sp%{ActivityName}UI = component_cast<UIComponent>(pKernel->CreateComponent("%{Namespace}.mainui"));
  sp%{ActivityName}UI->Set("viewport", spViewport);

  // Associate the Main UIComponent with our Activity Component
  SetUI(sp%{ActivityName}UI);
}

void %{ActivityName}::Activate()
{
  GetKernel().UpdatePulse.Subscribe(Delegate<void(double)>(this, &%{ActivityName}::Update));
}

void %{ActivityName}::Deactivate()
{
  GetKernel().UpdatePulse.Unsubscribe(Delegate<void(double)>(this, &%{ActivityName}::Update));
}

void %{ActivityName}::Update(double timeStep)
{
  if (spScene)
    spScene->Update(timeStep);
}

Variant %{ActivityName}::Save() const
{
  return nullptr;
}

extern "C" bool epPluginAttach()
{
  // Register the Activity Component
  Kernel::GetInstance()->RegisterComponentType<%{ActivityName}>();

  // Register the plugin's QML Components
  Kernel::GetInstance()->Call("registerqmlcomponents", ":/%{Namespace}");
  return true;
}

} // namespace %{Namespace}
