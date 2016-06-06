namespace ep {

  inline Variant epToVariant(const UDElementMetadata &e)
  {
    Variant v = epToVariant(*static_cast<const ElementMetadata*>(&e));
    if (e.blend != UDAttributeBlend::None)
    {
      Variant::VarMap r = v.asAssocArray();
      r.insert("blend", e.blend);
    }
    // v is a mutable reference to the VarMap
    return v;
  }

  inline void epFromVariant(const Variant &v, UDElementMetadata *pE)
  {
    epFromVariant(v, static_cast<ElementMetadata*>(pE));
    Variant *pI = v.getItem("blend");
    if (pI)
      pE->blend = pI->as<UDAttributeBlend>();
    else
      pE->blend = UDAttributeBlend::None;
  }

  template <typename T>
  inline void UDModel::SetConstantData(UDConstantDataType type, const T &data)
  {
    static_assert(std::is_trivial<T>::value, "T is not a trivial type");
    BufferRef spBuffer = Kernel::GetInstance()->CreateComponent<Buffer>();
    spBuffer->Allocate(sizeof(T));
    Slice<void> buffer = spBuffer->Map();
    EPASSERT_THROW(buffer, Result::Failure, "Failed to Map spBuffer");
    memcpy(buffer.ptr, &data, sizeof(T));
    spBuffer->Unmap();
    SetConstantData(type, spBuffer);
  }

  template <typename T>
  inline void UDModel::SetConstantData(UDConstantDataType type, Slice<const T> data)
  {
    static_assert(std::is_trivial<T>::value, "T is not a trivial type");
    ArrayBufferRef spBuffer = Kernel::GetInstance()->CreateComponent<ArrayBuffer>();
    spBuffer->AllocateFromData(data);
    SetConstantData(type, spBuffer);
  }

  template <typename T>
  inline T UDModel::GetConstantData(UDConstantDataType type)
  {
    static_assert(std::is_trivial<T>::value, "T is not a trivial type");
    BufferRef spBuffer = GetConstantData(type);
    EPTHROW_IF(!spBuffer, Result::InvalidArgument, "There is no constant data set for {0}", type.StringOf());

    T data;
    Slice<void> buffer = spBuffer->MapForRead();
    EPASSERT_THROW(sizeof(T) == buffer.length, Result::InvalidType, "sizeof(T) differs from the buffer length");
    memcpy(&data, buffer.ptr, buffer.length);
    return data;
  }

  inline const Double4x4& UDRender::GetMatrix() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return *reinterpret_cast<Double4x4*>(pContext->private0[3]);
  }

  inline const UDClipArea& UDRender::GetClipArea() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return *reinterpret_cast<UDClipArea*>(pContext->private0[1]);
  }

  inline UDModelFlags UDRender::GetFlags() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return UDModelFlags(pContext->private1[0]);
  }

  inline uint32_t UDRender::GetStartingRoot() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return pContext->private1[1];
  }

  inline uint32_t UDRender::GetAnimationFrame() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return pContext->private1[2];
  }

  template <typename T>
  inline const T &UDFilterVoxel::GetConstantData() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return *reinterpret_cast<T*>(pContext->private0[5]);
  }

  template <typename T>
  inline const T &UDRenderVoxel::GetConstantData() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return *reinterpret_cast<T*>(pContext->private0[7]);
  }

  template <typename T>
  inline const T &UDRenderPixel::GetConstantData() const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    return *reinterpret_cast<T*>(pContext->private0[9]);
  }

  inline uint32_t UDRenderNode::GetNodeColor(UDNodeIndex nodeIndex) const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    using GetNodeColor = uint32_t(internal::UDPrivateData *pInternal, UDNodeIndex nodeIndex);

    internal::UDPrivateData* pInternal = reinterpret_cast<internal::UDPrivateData*>(pContext->private0[0]);

    GetNodeColor *pGetNodeColor = reinterpret_cast<GetNodeColor*>(pInternal->private3[7]);

    return pGetNodeColor(pInternal, nodeIndex);
  }

  template <typename T>
  inline const T& UDRenderNode::GetAttribute(UDNodeIndex nodeIndex, UDRenderNode::UDStreamType type) const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    using GetAttribute = const void *(internal::UDPrivateData *pInternal, UDNodeIndex nodeIndex, UDRenderNode::UDStreamType type);

    internal::UDPrivateData* pInternal = reinterpret_cast<internal::UDPrivateData*>(pContext->private0[0]);

    GetAttribute *pGetAttribute = reinterpret_cast<GetAttribute*>(pInternal->private3[9]);

    return *reinterpret_cast<T*>(pGetAttribute(pInternal, nodeIndex, type));
  }

  template <typename T>
  inline const T& UDRenderNode::GetAttribute(UDNodeIndex nodeIndex, uint32_t attributeIndex) const
  {
    const UDRenderContext *pContext = reinterpret_cast<const UDRenderContext*>(this);
    using GetAttribute = const void *(internal::UDPrivateData *pInternal, UDNodeIndex nodeIndex, uint32_t index);

    internal::UDPrivateData* pInternal = reinterpret_cast<internal::UDPrivateData*>(pContext->private0[0]);

    GetAttribute *pGetAttribute = reinterpret_cast<GetAttribute*>(pInternal->private3[9]);

    return *reinterpret_cast<T*>(pGetAttribute(pInternal, nodeIndex, LightingComposite + 1 + attributeIndex));
  }
}
