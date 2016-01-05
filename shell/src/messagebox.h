#ifndef MESSAGE_BOX_H
#define MESSAGE_BOX_H

#include "ep/epenum.h"

/** Example Usage
 *
 *  spMessageBox->CallMethod("show", Variant::VarMap{
 *     {"title", "Some title"},
 *     {"text", "Some text"},
 *     {"buttons", Array<String>{ "Button1", "Button2", "Button3" }},
 *     {"callback", Delegate<void(Variant::VarMap)>(&MessageReceived)},
 *     {"type", MBType::Edit},
 *     {"icon", MBIconType::Warning},
 *     {"initEditText", "Some default text"}
 *     {"validator", Delegate<bool(String text)>(&MessageValidator)}
 *   });
 *
 *  ...
 *
 *  void MessageReceived(Variant::VarMap retValues)
 *  {
 *    String buttonLabel = retValues.Get("buttonLabel")->asString();
 *    int buttonIndex = retValues.Get("buttonIndex")->asInt();
 *    String editText = retValues.Get("editText")->asString();
 *  }
 */

namespace ep {

// MessageBox types
EP_ENUM(MBType,
  Message,
  Edit
);

// Icons
EP_ENUM(MBIconType,
  None,
  About,
  Error,
  Warning
);

}
#endif // MESSAGE_BOX_H
