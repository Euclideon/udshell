#include "text.h"
#include "kernel.h"
#include "components/memstream.h"

#include "rapidxml.hpp"

namespace ep {

Variant Text::ComponentParamsToXMLMap(Variant map)
{
  Variant::VarMap node;
  Array<Variant> childNodes;

  if (map.is(Variant::Type::AssocArray))
  {
    auto children = map.asAssocArray();
    for (auto child : children)
    {
      if (child.key.asString().eq("_text"))
        node.Insert("text", child.value.asString());
      else if (child.key.asString().eq("_attributes"))
        node.Insert("attributes", child.value.asAssocArray());
      else
      {
        Variant::VarMap childNode = ComponentParamsToXMLMap(child.value).asAssocArray();
        childNode.Insert("name", child.key);
        childNodes.pushBack(childNode);
      }
    }

    node.Insert("children", childNodes);
  }
  else
  {
    SharedString str = map.asSharedString();
    if (!str.empty())
      node.Insert("text", str);
  }

  return node;
}

Variant Text::XMLMapToComponentParams(Variant node)
{
  bool hasAttributes = false, hasChildren = false;
  Variant::VarMap map;

  if (!node.is(Variant::Type::AssocArray))
    return map;

  Variant::VarMap element = node.asAssocArray();

  Variant *pAttributes = element.Get("attributes");
  if (pAttributes && pAttributes->is(Variant::Type::AssocArray))
  {
    Variant::VarMap attributes = pAttributes->asAssocArray();
    for (auto attr : attributes)
    {
      map.Insert(attr);
      hasAttributes = true;
    }
  }

  Variant *pChildren = element.Get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Slice<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::Type::AssocArray))
      {
        Variant::VarMap childMap = child.asAssocArray();
        map.Insert(*childMap.Get("name"), XMLMapToComponentParams(child));

        hasChildren = true;
      }
    }
  }

  Variant *pText = element.Get("text");
  if (pText && pText->is(Variant::Type::String))
  {
    if (hasChildren || hasAttributes)
      map.Insert("_text", pText->asString());
    else
      return pText->asString();
  }

  return map;
}

static Variant ParseXMLNode(rapidxml::xml_node<> *node)
{
  using namespace rapidxml;

  Variant::VarMap attributes;
  Array<Variant> children;
  Variant::VarMap outNode;
  String text = "";

  if (!node)
    return outNode;

  // Add the node's attributes as an element _attributes

  for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
    attributes.Insert(KeyValuePair(String(attr->name(), attr->name_size()), String(attr->value(), attr->value_size())));
  for (xml_node<> *child = node->first_node(); child; child = child->next_sibling())
  {
    if (child->type() == node_data)
      text = String(child->value(), child->value_size());
    else
      children.pushBack(ParseXMLNode(child));
  }

  outNode.Insert("name", String(node->name(), node->name_size()));
  if(!text.empty())
    outNode.Insert("text", std::move(text));
  if (!attributes.Empty())
    outNode.Insert("attributes", std::move(attributes));
  if(!children.empty())
    outNode.Insert("children", std::move(children));

  return std::move(outNode);
}

Variant Text::ParseXml()
{
  Slice<const void> buffer = MapForRead();

  // RapidXML requires buffer to be null terminated
  if (buffer[buffer.length - 1] != '\0')
  {
    Unmap();
    Resize(buffer.length + 1);
    Slice<void> write = Map();
    write[buffer.length] = '\0';
    buffer = write;
  }
  epscope(exit) { Unmap(); };

  using namespace rapidxml;

  xml_document<> doc;
  doc.parse<parse_no_string_terminators>((char *)buffer.ptr);

  return ParseXMLNode(doc.first_node());
}

void Text::FormatXml(Variant root)
{
  if (!root.is(Variant::Type::AssocArray))
  {
    LogWarning(2, "FormatXML -- parameter is not a Map");
    return;
  }
  Variant::VarMap rootElement = root.asAssocArray();

  StreamRef spOut = GetKernel().CreateComponent<MemStream>({ { "buffer", ComponentRef(this) }, { "flags", OpenFlags::Write } });
  spOut->WriteLn("<?xml version=\"1.0\" encoding=\"utf-8\"?>");

  FormatXmlElement(spOut, rootElement, 0);
}

void Text::FormatXmlElement(StreamRef spOut, Variant::VarMap element, int depth)
{
  bool hasAttributes = false, hasChildren = false, hasText = false;

  String elemName = element.Get("name")->asString();
  MutableString<1024> str;
  str.format("{'',*0}<{1}", depth * 2, elemName);
  spOut->Write(str);

  Variant *pAttributes = element.Get("attributes");
  if (pAttributes && pAttributes->is(Variant::Type::AssocArray))
  {
    Variant::VarMap attributes = pAttributes->asAssocArray();

    for (auto attr : attributes)
    {
      str.format(" {0}=\"{1}\"", attr.key.asString(), attr.value.asSharedString());
      spOut->Write(str);
    }

    if (attributes.Size() > 0)
      hasAttributes = true;
  }

  Variant *pChildren = element.Get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Slice<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::Type::AssocArray))
      {
        if (!hasChildren)
        {
          spOut->Write(String(">\n"));
          hasChildren = true;
        }

        FormatXmlElement(spOut, child.asAssocArray(), depth + 1);
      }
    }
  }

  if(hasAttributes && !hasChildren)
    spOut->Write(String(">\n"));

  Variant *pText = element.Get("text");
  if (pText && pText->is(Variant::Type::String))
  {
    hasText = true;

    if (hasChildren || hasAttributes)
      str.format("{'',*0}{1}\n", (depth + 1) * 2, pText->asSharedString());
    else
      str.format(">{0}</{1}>\n", pText->asSharedString(), elemName);

    spOut->Write(str);
  }

  if (!hasAttributes && !hasChildren)
  {
    if(!hasText)
      spOut->Write(String(" />\n"));
  }
  else
  {
    str.format("{'',*0}</{1}>\n", depth * 2, elemName);
    spOut->Write(str);
  }
}

uint32_t Text::GetLineNumberFromByteIndex(String buffer, size_t index)
{
  int lineNumber = 0;

  for (size_t i = 0; i < buffer.length && i <= index; i++)
  {
    if (buffer[i] == '\n')
      lineNumber++;
  }

  return lineNumber;
}

} // namespace ep
