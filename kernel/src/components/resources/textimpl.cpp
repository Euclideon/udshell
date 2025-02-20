#include "textimpl.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/memstream.h"

#include "rapidxml.hpp"
#include "rapidjson/document.h"

namespace ep {

Array<const MethodInfo> Text::getMethods() const
{
  return{
    EP_MAKE_METHOD(parseXml, "Parse XML formatted text in buffer into a hierarchical structure of KeyValuePairs"),
    EP_MAKE_METHOD(formatXml, "Format a hierarchical structure of KeyValuePairs as XML text"),
    EP_MAKE_METHOD(parseJson, "Parse Json formatted text in buffer into a hierarchical structure of KeyValuePairs"),
    EP_MAKE_METHOD(formatJson, "Format a hierarchical structure of KeyValuePairs as Json text"),
  };
}
Array<const StaticFuncInfo> Text::getStaticFuncs() const
{
  return{
    EP_MAKE_STATICFUNC(xmlMapToComponentParams, "Convert a map representing a hierarchy of XML elements into 'Component Params' format, i.e. suitable as InitParams and for returning from Save()"),
    EP_MAKE_STATICFUNC(componentParamsToXmlMap, "Convert a map in 'Component Params' format into a map representing a hierarchy of XML elements"),
  };
}

Variant TextImplStatic::ComponentParamsToXMLMap(Variant map)
{
  Variant::VarMap::MapType node;
  Array<Variant> childNodes;

  if (map.is(Variant::SharedPtrType::AssocArray))
  {
    auto children = map.asAssocArray();
    for (auto child : children)
    {
      if (child.key.asString().eq("_text"))
        node.insert("text", child.value.asString());
      else if (child.key.asString().eq("_attributes"))
        node.insert("attributes", child.value.asAssocArray());
      else
      {
        Variant::VarMap::MapType childNode = ComponentParamsToXMLMap(child.value).claimMap();
        childNode.insert("name", child.key);
        childNodes.pushBack(std::move(childNode));
      }
    }

    node.insert("children", childNodes);
  }
  else
  {
    SharedString str = map.asSharedString();
    if (!str.empty())
      node.insert("text", str);
  }

  return std::move(node);
}

Variant TextImplStatic::XMLMapToComponentParams(Variant node)
{
  bool hasAttributes = false, hasChildren = false;
  Variant::VarMap::MapType map;

  if (!node.is(Variant::SharedPtrType::AssocArray))
    return std::move(map);

  Variant::VarMap element = node.asAssocArray();

  Variant *pAttributes = element.get("attributes");
  if (pAttributes && pAttributes->is(Variant::SharedPtrType::AssocArray))
  {
    Variant::VarMap attributes = pAttributes->asAssocArray();
    for (auto attr : attributes)
    {
      map.insert(attr);
      hasAttributes = true;
    }
  }

  Variant *pChildren = element.get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Slice<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::SharedPtrType::AssocArray))
      {
        Variant::VarMap childMap = child.asAssocArray();
        map.insert(*childMap.get("name"), XMLMapToComponentParams(child));

        hasChildren = true;
      }
    }
  }

  Variant *pText = element.get("text");
  if (pText && pText->is(Variant::Type::String))
  {
    if (hasChildren || hasAttributes)
      map.insert("_text", pText->asString());
    else
      return pText->asString();
  }

  return std::move(map);
}

static Variant ParseXMLNode(rapidxml::xml_node<> *node)
{
  using namespace rapidxml;

  Variant::VarMap::MapType attributes;
  Array<Variant> children;
  Variant::VarMap::MapType outNode;
  String text = "";

  if (!node)
    return std::move(outNode);

  // Add the node's attributes as an element _attributes

  for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
    attributes.insert(KeyValuePair(String(attr->name(), attr->name_size()), String(attr->value(), attr->value_size())));
  for (xml_node<> *child = node->first_node(); child; child = child->next_sibling())
  {
    if (child->type() == node_data)
      text = String(child->value(), child->value_size());
    else
      children.pushBack(ParseXMLNode(child));
  }

  outNode.insert("name", String(node->name(), node->name_size()));
  if(!text.empty())
    outNode.insert("text", std::move(text));
  if (!attributes.empty())
    outNode.insert("attributes", std::move(attributes));
  if(!children.empty())
    outNode.insert("children", std::move(children));

  return std::move(outNode);
}

Variant TextImpl::ParseXml()
{
  char *pNew = nullptr;
  Slice<const void> buffer = pInstance->mapForRead();
  epscope(exit)
  {
    pInstance->unmap();
    epFree(pNew);
  };

  // RapidXML requires buffer to be null terminated
  if (buffer[buffer.length - 1] != '\0')
  {
    pNew = (char*)epAlloc(buffer.length + 1);
    memcpy(pNew, buffer.ptr, buffer.length);
    pNew[buffer.length] = 0;
    buffer.ptr = pNew;
  }

  using namespace rapidxml;

  xml_document<> doc;
  doc.parse<parse_no_string_terminators>((char *)buffer.ptr);
  Variant r(ParseXMLNode(doc.first_node()));

  return r;
}

void TextImpl::FormatXml(Variant root)
{
  if (!root.is(Variant::SharedPtrType::AssocArray))
  {
    logWarning(2, "FormatXml -- parameter is not a Map");
    return;
  }
  Variant::VarMap rootElement = root.asAssocArray();

  StreamRef spOut = getKernel()->createComponent<MemStream>({ { "buffer", ComponentRef(pInstance) }, { "flags", OpenFlags::Write } });
  spOut->writeLn("<?xml version=\"1.0\" encoding=\"utf-8\"?>");

  FormatXmlElement(spOut, rootElement, 0);
}

void TextImpl::FormatXmlElement(StreamRef spOut, Variant::VarMap element, int depth)
{
  bool hasAttributes = false, hasChildren = false, hasText = false;

  String elemName = element.get("name")->asString();
  MutableString<1024> str;
  str.format("{'',*0}<{1}", depth * 2, elemName);
  spOut->write(str);

  Variant *pAttributes = element.get("attributes");
  if (pAttributes && pAttributes->is(Variant::SharedPtrType::AssocArray))
  {
    Variant::VarMap attributes = pAttributes->asAssocArray();

    for (auto attr : attributes)
    {
      str.format(" {0}=\"{1}\"", attr.key.asString(), attr.value.asSharedString());
      spOut->write(str);
    }

    if (attributes.size() > 0)
      hasAttributes = true;
  }

  Variant *pChildren = element.get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Slice<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::SharedPtrType::AssocArray))
      {
        if (!hasChildren)
        {
          spOut->write(String(">\n"));
          hasChildren = true;
        }

        FormatXmlElement(spOut, child.asAssocArray(), depth + 1);
      }
    }
  }

  if(hasAttributes && !hasChildren)
    spOut->write(String(">\n"));

  Variant *pText = element.get("text");
  if (pText && pText->is(Variant::Type::String))
  {
    hasText = true;

    if (hasChildren || hasAttributes)
      str.format("{'',*0}{1}\n", (depth + 1) * 2, pText->asSharedString());
    else
      str.format(">{0}</{1}>\n", pText->asSharedString(), elemName);

    spOut->write(str);
  }

  if (!hasAttributes && !hasChildren)
  {
    if(!hasText)
      spOut->write(String(" />\n"));
  }
  else
  {
    str.format("{'',*0}</{1}>\n", depth * 2, elemName);
    spOut->write(str);
  }
}

uint32_t TextImplStatic::GetLineNumberFromByteIndex(String buffer, size_t index)
{
  int lineNumber = 0;

  for (size_t i = 0; i < buffer.length && i <= index; i++)
  {
    if (buffer[i] == '\n')
      lineNumber++;
  }

  return lineNumber;
}


static Variant ParseJsonNode(const rapidjson::Value& val)
{
  using namespace rapidjson;
  switch (val.GetType())
  {
    case Type::kNullType:
      return nullptr;
    case Type::kTrueType:
      return true;
    case Type::kFalseType:
      return false;
    case Type::kNumberType:
      if (val.IsUint())
        return val.GetUint();
      else if (val.IsInt())
        return val.GetInt();
      else if (val.IsUint64())
        return val.GetUint64();
      else if (val.IsInt64())
        return val.GetInt64();
      else
        return val.GetDouble();
    case Type::kStringType:
      return String(val.GetString(), val.GetStringLength());
    case Type::kArrayType:
    {
      SizeType len = val.Size();
      Array<Variant, 0> arr(Reserve, len);
      for (SizeType i = 0; i < len; ++i)
        arr.pushBack(ParseJsonNode(val[i]));
      return arr;
    }
    case Type::kObjectType:
    {
      auto j = val.MemberEnd();
      Variant::VarMap::MapType map;
      for (auto i = val.MemberBegin(); i != j; ++i)
        map.insert(ParseJsonNode(i->name), ParseJsonNode(i->value));
      return std::move(map);
    }
    default:
      EPASSERT_THROW(false, Result::Failure, "Unknown error!");
  }
}

Variant TextImpl::ParseJson()
{
  Slice<const void> buffer = pInstance->mapForRead();

  // RapidXML requires buffer to be null terminated
  if (buffer[buffer.length - 1] != '\0')
  {
    pInstance->unmap();
    pInstance->resize(buffer.length + 1);
    Slice<void> write = pInstance->map();
    write[buffer.length] = '\0';
    buffer = write;
  }
  epscope(exit) { pInstance->unmap(); };

  using namespace rapidjson;

  Document document;
  document.Parse<ParseFlag::kParseDefaultFlags>((const char*)buffer.ptr);

  return ParseJsonNode(document);
}

void TextImpl::FormatJson(Variant root)
{
  EPASSERT(false, "TODO!");
}

} // namespace ep
