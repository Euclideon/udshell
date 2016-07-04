from dumper import *

# Helper methods
def epStringHelper(d, base, chType, size = None):
    charSize = chType.sizeof
    innerType = str(chType)
    strType = "utf8"
    if innerType == "wchar_t":
        if charSize == 2:
            strType = "utf16"
        else:
            strType = "ucs4"
    elif innerType == "char16_t":
        strType = "utf16"
    elif innerType == "char32_t":
        strType = "ucs4"

    if size is None:
        elided, shown, data = d.readToFirstZero(base, 1, d.displayStringLimit)
    else:
        elided, shown = d.computeLimit(int(size), d.displayStringLimit)
        data = d.readMemory(base, shown*charSize)

    return elided, data, strType

def epPrintString(d, base, chType, length):
    elided, data, strType = epStringHelper(d, base, chType, length)
    d.putValue(data, strType, elided=elided)

def epPrintValue(d, itemName, typeName, value):
    with SubItem(d, itemName):
        d.putValue(value)
        d.putType(typeName)
        d.putNumChild(0)


# String debug handlers
def qdump__ep__BaseString(d, value):
    epPrintString(d, value["ptr"], d.templateArgument(value.type, 0), int(value["length"]))
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)

def qdump__ep__MutableString(d, value):
    epPrintString(d, value["ptr"], d.lookupType('char'), int(value["length"]))
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)

def qdump__ep__SharedString(d, value):
    epPrintString(d, value["ptr"], d.lookupType('char'), int(value["length"]))
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


# Variant helper methods
def epVariant_Void(d, value):
    d.putBetterType("ep::Variant (Void)")
    d.putValue("void")
    d.putNumChild(0)

def epVariant_Error(d, value):
    d.putBetterType("ep::Variant (Error)")
    err = value["err"].dereference()
    d.putItem(err)

def epVariant_Null(d, value):
    d.putBetterType("ep::Variant (Null)")
    d.putValue("nullptr")
    d.putNumChild(0)

def epVariant_Bool(d, value):
    d.putBetterType("ep::Variant (Bool)")
    b = bool(int(value["b"]) != 0)
    d.putValue("true" if b else "false")
    d.putNumChild(0)

def epVariant_Int(d, value):
    d.putBetterType("ep::Variant (Int)")
    i = int(value["i"])
    d.putValue(i)
    d.putNumChild(0)

def epVariant_Float(d, value):
    d.putBetterType("ep::Variant (Float)")
    f = value["f"]
    d.putValue(f)
    d.putNumChild(0)

def epVariant_Enum(d, value):
    d.putBetterType("ep::Variant (Enum)")
    i = int(value["length"])
    d.putValue(i)
    d.putNumChild(0)

def epVariant_Bitfield(d, value):
    d.putBetterType("ep::Variant (Bitfield)")
    i = int(value["length"])
    d.putValue(i)
    d.putNumChild(0)

# SharedPtr Variant Types
def epVariant_SharedPtrUnknown(d, value):
    sp = value["sp"].dereference()
    rc = int(sp["rc"])
    d.putValue("RC: %d Addr: 0x%x" % (rc, int(sp.address)))
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            epPrintValue(d, "sharedPtrType", "Variant::SharedPtrType", "Unknown")
            d.putIntItem("rc", rc)
            d.putSubItem("sp", sp)

def epVariant_SharedPtrComponent(d, value):
    c = value["c"].dereference()
    rc = int(c["rc"])
    d.putValue("RC: %d Addr: 0x%x" % (rc, int(c.address)))
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            epPrintValue(d, "sharedPtrType", "Variant::SharedPtrType", "Component")
            d.putIntItem("rc", rc)
            d.putSubItem("c", c)

def epVariant_SharedPtrDelegate(d, value):
    delegate = value["d"].dereference()
    rc = int(delegate["rc"])
    d.putValue("RC: %d Addr: 0x%x" % (rc, int(delegate.address)))
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            epPrintValue(d, "sharedPtrType", "Variant::SharedPtrType", "Delegate")
            d.putIntItem("rc", rc)
            d.putSubItem("d", delegate)

def epVariant_SharedPtrSubscription(d, value):
    sub = value["sub"].dereference()
    rc = int(sub["rc"])
    d.putValue("RC: %d Addr: 0x%x" % (rc, int(sub.address)))
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            epPrintValue(d, "sharedPtrType", "Variant::SharedPtrType", "Subscription")
            d.putIntItem("rc", rc)
            d.putSubItem("sub", sub)

def epVariant_SharedPtrAssocArray(d, value):
    aa = value["aa"].dereference()
    rc = aa["rc"]
    tree = aa["tree"]
    if str(tree.type).startswith("ep::AVLTree"):
        numNodes = tree["numNodes"]
        d.putValue("RC: %d <%d items>" % (rc, numNodes))
    else:
        d.putValue("RC: %d" % rc)

    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            epPrintValue(d, "sharedPtrType", "Variant::SharedPtrType", "AssocArray")
            d.putIntItem("rc", rc)
            d.putSubItem("tree", tree)

def epVariant_SharedPtrRange(d, value):
    r = value["r"].dereference()
    rc = int(r["rc"])
    d.putValue("RC: %d Addr: 0x%x" % (rc, int(r.address)))
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            epPrintValue(d, "sharedPtrType", "Variant::SharedPtrType", "Range")
            d.putIntItem("rc", rc)
            d.putSubItem("r", r)


epVariantSharedPtrTable = [
    epVariant_SharedPtrUnknown,
    epVariant_SharedPtrComponent,
    epVariant_SharedPtrDelegate,
    epVariant_SharedPtrSubscription,
    epVariant_SharedPtrAssocArray,
    epVariant_SharedPtrRange
]

def epVariant_SharedPtr(d, value):
    ownsContent = int(value["ownsContent"]) != 0
    if ownsContent:
        d.putBetterType("ep::Variant (SharedPtr<T>)")
    else:
        d.putBetterType("ep::Variant (T*)")
    subType = int(value["length"])
    epVariantSharedPtrTable[subType](d, value)

def epVariant_String(d, value):
    ownsContent = int(value["ownsContent"]) != 0
    if ownsContent:
        d.putBetterType("ep::Variant (SharedString)")
    else:
        d.putBetterType("ep::Variant (String)")
    length = int(value["length"])
    s = value["s"]
    epPrintString(d, s, d.lookupType('char'), length)
    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            d.putIntItem("length", length)
            epPrintValue(d, "ownsContent", "bool", "true" if ownsContent else "false")
            d.putSubItem("s", s)

def epVariant_Array(d, value):
    ownsContent = int(value["ownsContent"]) != 0
    if ownsContent:
        d.putBetterType("ep::Variant (SharedArray)")
    else:
        d.putBetterType("ep::Variant (Slice)")
    length = value["length"]
    a = value["a"]
    d.putValue("<%d items>" % length)
    d.putAddress(value.address)
    d.putNumChild(length)
    if d.isExpanded():
        d.putArrayData(a, length, a.dereference().type)

def epVariant_SmallString(d, value):
    d.putBetterType("ep::Variant (SmallString)")
    length = int(d.extractByte(d.extractPointer(value.address))) >> 4
    byteType = d.lookupType('char')
    s = d.pointerValue(value.address.cast(d.charPtrType()) + 1)
    epPrintString(d, s, byteType, length)
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putIntItem("length", length)
            with SubItem(d, "string"):
                epPrintString(d, s, byteType, length)
                d.putType("char *")
                d.putNumChild(0)


epVariantTable = [
    epVariant_Void,
    epVariant_Error,
    epVariant_Null,
    epVariant_Bool,
    epVariant_Int,
    epVariant_Float,
    epVariant_Enum,
    epVariant_Bitfield,
    epVariant_SharedPtr,
    epVariant_String,
    epVariant_Array,
    epVariant_SmallString
]


# Variant debug handler
def qdump__ep__Variant(d, value):
    variantType = int(value["t"])
    epVariantTable[variantType](d, value)


# Slice debug handlers
def qdump__ep__Slice(d, value):
    array = value["ptr"]
    size = value["length"]
    innerType = d.templateArgument(value.type, 0)
    addr = array.cast(innerType.pointer())
    d.putValue("<%d items>" % size)
    d.putAddress(value.address)
    d.putNumChild(size)
    if d.isExpanded():
        d.putArrayData(addr, size, innerType)

def qdump__ep__Array(d, value):
    qdump__ep__Slice(d, value)

def qdump__ep__SharedArray(d, value):
    qdump__ep__Slice(d, value)


# AVL Tree debug handlers
def qdump__ep__AVLTree(d, value):
    numNodes = value["numNodes"]
    d.putValue("<%d items>" % numNodes)
    if d.isExpanded():
        keyType = d.templateArgument(value.type, 0)
        valueType = d.templateArgument(value.type, 1)
        node = value["pRoot"].dereference()
        nodeType = node.type

        def helper(d, node, nodeType, i):
            left = node["left"]
            if not d.isNull(left):
                i = helper(d, left.dereference(), nodeType, i)
                if i >= numNodes:
                    return i
            kvp = node.cast(nodeType)["kvp"]
            with SubItem(d, i):
                d.putItem(kvp)
            i += 1
            if i >= numNodes:
                return i
            right = node["right"]
            if not d.isNull(right):
                i = helper(d, right.dereference(), nodeType, i)
            return i

        with PairedChildren(d, numNodes, useKeyAndValue=True, keyType=keyType, valueType=valueType, pairType=nodeType):
            helper(d, node, nodeType, 0)

def qdump__ep__SharedMap(d, value):
    ptr = value["ptr"]
    node = ptr["pInstance"].cast(d.templateArgument(ptr.type, 0).pointer()).dereference()
    rc = node["rc"]
    tree = node["tree"]

    if str(tree.type).startswith("ep::AVLTree"):
        numNodes = tree["numNodes"]
        d.putValue("RC: %d <%d items>" % (rc, numNodes))
    else:
        d.putValue("RC: %d" % rc)

    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putSubItem("tree", tree)


def qdump__ep__Component(d, value):
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__ep__KVP(d, value):
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)

def qdump__ep__KVPRef(d, value):
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putFields(value)


def qdump__ep__SharedPtr(d, value):
    pInstance = value["pInstance"]
    innerType = d.templateArgument(value.type, 0)
    innerObj = pInstance.cast(innerType.pointer()).dereference()
    rc = int(innerObj["rc"])
    d.putValue("RC: %d Addr: 0x%x" % (rc, int(innerObj.address)))
    d.putAddress(value.address)
    d.putNumChild(2)
    with Children(d):
        d.putIntItem("rc", rc)
        d.putSubItem("pInstance", innerObj)
