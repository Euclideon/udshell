#include "textnodeimpl.h"
#include "renderscene.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/metadata.h"
#include "components/resources/modelimpl.h"
#include "components/resources/materialimpl.h"


namespace ep {

ArrayBufferRef TextNodeImpl::spQuadVerts;
ShaderRef TextNodeImpl::spTextVS;
ShaderRef TextNodeImpl::spTextPS;

Array<const PropertyInfo> TextNode::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("text", getText, setText, "The text", nullptr, 0),
    EP_MAKE_PROPERTY("font", getFont, setFont, "The font", nullptr, 0),
    EP_MAKE_PROPERTY("height", getHeight, setHeight, "The text height", nullptr, 0),
    EP_MAKE_PROPERTY("justification", getJustification, setJustification, "The text justification", nullptr, 0),
    EP_MAKE_PROPERTY("outlineWidth", getOutlineWidth, setOutlineWidth, "The outline width (0 for no outline)", nullptr, 0),
    EP_MAKE_PROPERTY("color", getColor, setColor, "The text color", nullptr, 0),
    EP_MAKE_PROPERTY("outlineColor", getOutlineColor, setOutlineColor, "The text outline color", nullptr, 0),
  };
}

TextNodeImpl::TextNodeImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  Variant *v = initParams.get("font");
  if (v)
    setFont(v->as<FontRef>());
  v = initParams.get("text");
  if (v)
    setText(v->asSharedString());
  v = initParams.get("height");
  if (v)
    setHeight(v->as<double>());
  v = initParams.get("justification");
  if (v)
    setJustification(v->as<Justification>());
  v = initParams.get("outlineWidth");
  if (v)
    setOutlineWidth(v->as<double>());
  v = initParams.get("color");
  if (v)
    setColor(v->as<Float3>());
  v = initParams.get("outlineColor");
  if (v)
    setOutlineColor(v->as<Float3>());
}

void TextNodeImpl::createStaticResources()
{
  spQuadVerts = getKernel()->createComponent<ArrayBuffer>();
  spQuadVerts->allocateFromData<float>(Slice<const float>{ 0.f, 1.f, 2.f, 1.f, 3.f, 2.f });
  spQuadVerts->getMetadata()->insertAt("a_vertIndex", "attributeInfo", 0, "name");

  spTextVS = getKernel()->createComponent<Shader>();
  spTextVS->setType(ShaderType::VertexShader);
  spTextVS->setCode(
    "#version 130\n"
    "attribute float a_vertIndex;\n"
    "uniform mat4 u_mfwvp;\n"
    "uniform vec4 u_viewDisplaySize;\n"
    "uniform vec2 u_screenOffset[4];\n"
    "uniform vec2 u_vertUV[4];\n"
    "void main()\n"
    "{\n"
    "  vec4 screenPos = u_mfwvp*vec4(0,0,0,1);\n"
    "  screenPos /= screenPos.w;\n"
    "  ivec2 iScreenPos = ivec2((screenPos.xy + vec2(1,1)) * 0.5 * u_viewDisplaySize.xy);\n"
    "  ivec2 screenOffset = ivec2(u_screenOffset[int(a_vertIndex)]);\n"
    "  screenOffset.y = -screenOffset.y;\n"
    "  iScreenPos += screenOffset;\n"
    "  screenPos.xy = vec2(iScreenPos) * u_viewDisplaySize.zw * 2 - vec2(1,1);\n"
    "  gl_Position = screenPos;\n"
//    "  vec2 halfTexel = 0.5/u_screenOffset[3];\n"
//    "  gl_TexCoord[0].xy = u_vertUV[int(a_vertIndex)] + halfTexel;\n"
    "  gl_TexCoord[0].xy = u_vertUV[int(a_vertIndex)];\n"
    "}\n");

  spTextPS = getKernel()->createComponent<Shader>();
  spTextPS->setType(ShaderType::PixelShader);
  spTextPS->setCode(
    "uniform sampler2D text;\n"
    "void main()\n"
    "{\n"
    "  gl_FragColor = texture2D(text, gl_TexCoord[0].xy);\n"
    "}\n");
}

void TextNodeImpl::render(RenderScene &spScene, const Double4x4 &mat)
{
  if (!spQuadVerts)
    createStaticResources();

  if (!spTextMat)
  {
    spTextMat = getKernel()->createComponent<Material>();
    spTextMat->setVertexShader(spTextVS);
    spTextMat->setPixelShader(spTextPS);
    spTextMat->setMaterialProperty("u_vertUV", Array<Float2>{ { 0.0,0.0 }, { 1.0,0.0 }, { 0.0,1.0 }, { 1.0,1.0 } });
  }

  if (!spTextImage)
  {
    try {
      // render the text to an image
      spTextImage = spFont->rasterizeText(text, (int)height, color, outlineColor);
      spTextMat->setMaterialProperty("text", spTextImage);
    } catch (...) {
      // failed to render text! >_<
      // TODO: make a noise? but don't spam the output!!
      return;
    }
  }

  Slice<const size_t> shape = spTextImage->getShape();
  Array<Float2, 4> quad(Alloc, 4);
  switch (justification)
  {
    case Justification::TopLeft:
    case Justification::CenterLeft:
    case Justification::BottomLeft:
      quad[0].x = 0.f;
      quad[1].x = (float)shape[0];
      quad[2].x = 0.f;
      quad[3].x = (float)shape[0];
      break;
    case Justification::TopCenter:
    case Justification::Center:
    case Justification::BottomCenter:
      quad[0].x = (float)-(int)shape[0]/2;
      quad[1].x = (float)-(int)shape[0]/2 + shape[0];
      quad[2].x = (float)-(int)shape[0]/2;
      quad[3].x = (float)-(int)shape[0]/2 + shape[0];
      break;
    case Justification::TopRight:
    case Justification::CenterRight:
    case Justification::BottomRight:
      quad[0].x = (float)-(int)shape[0];
      quad[1].x = 0.f;
      quad[2].x = (float)-(int)shape[0];
      quad[3].x = 0.f;
      break;
  }
  switch (justification)
  {
    case Justification::TopLeft:
    case Justification::TopCenter:
    case Justification::TopRight:
      quad[0].y = 0.f;
      quad[1].y = 0.f;
      quad[2].y = (float)shape[1];
      quad[3].y = (float)shape[1];
      break;
    case Justification::CenterLeft:
    case Justification::Center:
    case Justification::CenterRight:
      quad[0].y = (float)-(int)shape[1]/2;
      quad[1].y = (float)-(int)shape[1]/2;
      quad[2].y = (float)-(int)shape[1]/2 + shape[1];
      quad[3].y = (float)-(int)shape[1]/2 + shape[1];
      break;
    case Justification::BottomLeft:
    case Justification::BottomCenter:
    case Justification::BottomRight:
      quad[0].y = (float)-(int)shape[1];
      quad[1].y = (float)-(int)shape[1];
      quad[2].y = 0.f;
      quad[3].y = 0.f;
      break;
  }
  spTextMat->setMaterialProperty("u_screenOffset", quad);

  GeomRenderJob &job = spScene.geom.pushBack();
  job.matrix = mat;
  job.spMaterial = spTextMat;

  job.vertexArrays = Array<ArrayBufferRef>({ spQuadVerts });
  job.spIndices = nullptr;
  job.renderList = RenderList{ PrimType::Triangles, 0, 0, 6 };

  job.retainShaderInputConfig = Delegate<void(SharedPtr<RefCounted>)>(this, &TextNodeImpl::RetainShaderInputConfig);
}

} // namespace ep
