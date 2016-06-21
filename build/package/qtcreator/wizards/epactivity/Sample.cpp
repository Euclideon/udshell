#include "%{ProjectName}.h"

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

Array<const PropertyInfo> %{ProjectName}::GetProperties() const
{
  return { };
}

Array<const ep::MethodInfo> %{ProjectName}::GetMethods() const
{
  return { };
}


Array<const EventInfo> %{ProjectName}::GetEvents() const
{
  return { };
}

Array<const StaticFuncInfo> %{ProjectName}::GetStaticFuncs() const
{
  return { };
}

%{ProjectName}::%{ProjectName}(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Activity(pType, pKernel, uid, initParams)
{
  // Setup the Scene, Camera and View Components
  ViewRef spView = pKernel->CreateComponent<View>();
  spScene = pKernel->CreateComponent<Scene>();
  CameraRef spCamera = pKernel->CreateComponent<SimpleCamera>();
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
  auto spGenerator = pKernel->CreateComponent<PrimitiveGenerator>();

  ArrayBufferRef spVertexBuffer = pKernel->CreateComponent<ArrayBuffer>();
  ArrayBufferRef spIndexBuffer = pKernel->CreateComponent<ArrayBuffer>();

  spGenerator->GenerateCube(spVertexBuffer, spIndexBuffer);

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
  UIComponentRef sp%{ProjectName}UI;
  epscope(fail) { if(!sp%{ProjectName}UI) pKernel->LogError("Error creating %{ProjectName} UI Component"); };
  sp%{ProjectName}UI = component_cast<UIComponent>(pKernel->CreateComponent("%{Namespace}.mainui"));
  sp%{ProjectName}UI->Set("viewport", spViewport);
  
  // Associate the Main UIComponent with our Activity Component
  SetUI(sp%{ProjectName}UI);
}

void %{ProjectName}::Activate()
{
  GetKernel().UpdatePulse.Subscribe(Delegate<void(double)>(this, &%{ProjectName}::Update));
}

void %{ProjectName}::Deactivate()
{
  GetKernel().UpdatePulse.Unsubscribe(Delegate<void(double)>(this, &%{ProjectName}::Update));
}

void %{ProjectName}::Update(double timeStep)
{
  if (spScene)
    spScene->Update(timeStep);
}

Variant %{ProjectName}::Save() const
{
  return nullptr;
}

extern "C" bool epPluginAttach()
{
  // Register the Activity Component
  Kernel::GetInstance()->RegisterComponentType<%{ProjectName}>();

  // Register the plugin's QML Components
  Kernel::GetInstance()->Call("registerqmlcomponents", ":/%{Namespace}");
  return true;
}

} // namespace %{Namespace}
