#include "text.h"
#include "kernel.h"
#include "components/memstream.h"

#include "rapidxml.hpp"

namespace kernel {

static KeyValuePair ParseXMLNode(rapidxml::xml_node<> *node)
{
  using namespace rapidxml;

  if (node->type() == node_data)
    return KeyValuePair("_text", String(node->value(), node->value_size()));

  Array<KeyValuePair> children;

  // Add the node's attributes as an element _attributes
  Array<KeyValuePair> attributes;
  for (xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
    attributes.pushBack(KeyValuePair(String(attr->name(), attr->name_size()), String(attr->value(), attr->value_size())));
  if (!attributes.empty())
    children.pushBack(KeyValuePair("_attributes", attributes));

  for (xml_node<> *child = node->first_node(); child; child = child->next_sibling())
    children.pushBack(ParseXMLNode(child));

  // If this is a leaf element with no attributes, simplify the value to a String
  if (children.length == 1 && children[0].key.asString().eq("_text"))
    return KeyValuePair(String(node->name(), node->name_size()), children[0].value.asSharedString());

  return KeyValuePair(String(node->name(), node->name_size()), children);
}

Variant Text::ParseXml()
{
  // RapidXML requires buffer to be null terminated
  if (buffer[buffer.length - 1] != '\0')
  {
    Resize(buffer.length + 1);
    buffer[buffer.length - 1] = '\0';
  }

  using namespace rapidxml;

  xml_document<> doc;
  doc.parse<parse_no_string_terminators>((char *)buffer.ptr);

  KeyValuePair kvp = ParseXMLNode(doc.first_node());

  Array<KeyValuePair> rootValues;
  rootValues.pushBack(kvp);

  return rootValues;
}

void Text::FormatXml(Variant root)
{
  Variant::VarMap rootElements = root.asAssocArray();
  if (rootElements.Size() == 0)
  {
    LogWarning(2, "Tree has no nodes!");
    return;
  }
  else if (rootElements.Size() > 1)
  {
    LogWarning(2, "XML may only have a single root node!");
    return;
  }

  StreamRef spOut = GetKernel().CreateComponent<MemStream>({ { "buffer", ComponentRef(this) }, { "flags", OpenFlags::Write } });
  spOut->WriteLn("<?xml version=\"1.0\" encoding=\"utf-8\"?>");

  auto kvp = rootElements.begin();
  FormatXmlElement(spOut, *kvp, 0);
}

void Text::FormatXmlElement(StreamRef spOut, KeyValuePair element, int depth)
{
  String _name = element.key.asString();
  MutableString<1024> str;
  str.format("{'',*0}<{1}", depth * 2, _name);
  spOut->Write(str);

  if (element.value.is(Variant::Type::AssocArray))
  {
    Variant::VarMap children = element.value.asAssocArray();

    for (auto child : children)
    {
      if (!child.key.asString().cmp("_attributes"))
      {
        Variant::VarMap attributes = child.value.asAssocArray();
        for (auto attr : attributes)
        {
          str.format(" {0}=\"{1}\"", attr.key.asString(), attr.value.asSharedString());
          spOut->Write(str);
        }
      }
    }

    spOut->Write(String(">\n"));

    for (auto child : children)
    {
      if (child.key.asString().eq("_text"))
        str.format("{'',*0}{1}\n", (depth + 1) * 2, child.value.asSharedString());
    }

    for (auto child : children)
    {
      String childName = child.key.asString();
      if (!childName.eq("_attributes") && !childName.eq("_text"))
        FormatXmlElement(spOut, child, depth + 1);
    }

    str.format("{'',*0}</{1}>\n", depth * 2, name);
    spOut->Write(str);
  }
  else
  {
    SharedString value = element.value.asSharedString();
    if (!value.empty())
    {
      str.format(">{0}</{1}>\n", value, name);
      spOut->Write(str);
    }
    else
      spOut->Write(String(" />\n"));
  }
}

} // namespace kernel
