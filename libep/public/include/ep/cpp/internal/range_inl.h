namespace ep {
namespace internal {

  // ***** HORRIBLE META CRUFT!!! *****
  template <bool> struct FeaturesImpl {
    template <typename T> static RangeFeatures call(const T &r) { return r.features(); }
  };
  template <> struct FeaturesImpl<false> {
    template <typename T> static RangeFeatures call(const T &)
    {
      return (Range<T>::HasLength ? RangeFeatures::HasLength    : 0) |
             (Range<T>::HasFront  ? RangeFeatures::ForwardRange : 0) |
             (Range<T>::HasBack   ? RangeFeatures::ReverseRange : 0) |
             (Range<T>::HasIndex  ? RangeFeatures::RandomRange  : 0);
    }
  };

  template <bool> struct EmptyImpl {
    template <typename T> static bool call(const T &r) { return r.empty(); }
  };
  template <> struct EmptyImpl<false> {
    template <typename T> static bool call(const T &) { return false; }
  };
  template <bool> struct LengthImpl {
    template <typename T> static size_t call(const T &r) { return r.length(); }
  };
  template <> struct LengthImpl<false> {
    template <typename T> static size_t call(const T &) { EPTHROW(Result::InvalidType, "Range does not have a length"); }
  };

  template <bool> struct GetFrontImpl {
    template <typename T> static auto call(const T &r) -> typename Range<T>::ElementType { return r.front(); }
  };
  template <> struct GetFrontImpl<false> {
    template <typename T> static auto call(const T &) -> typename Range<T>::ElementType { EPTHROW(Result::InvalidType, "Not a forward range!"); }
  };
  template <bool> struct PopFrontImpl {
    template <typename T> static auto call(T &r) -> typename Range<T>::ElementType { return r.popFront(); }
  };
  template <> struct PopFrontImpl<false> {
    template <typename T> static auto call(T &) -> typename Range<T>::ElementType { EPTHROW(Result::InvalidType, "Not a forward range!"); }
  };

  template <bool> struct GetBackImpl {
    template <typename T> static auto call(const T &r) -> typename Range<T>::ElementType { return r.back(); }
  };
  template <> struct GetBackImpl<false> {
    template <typename T> static auto call(const T &) -> typename Range<T>::ElementType { EPTHROW(Result::InvalidType, "Not a reverse range!"); }
  };
  template <bool> struct PopBackImpl {
    template <typename T> static auto call(T &r) -> typename Range<T>::ElementType { return r.popBack(); }
  };
  template <> struct PopBackImpl<false> {
    template <typename T> static auto call(T &) -> typename Range<T>::ElementType { EPTHROW(Result::InvalidType, "Not a reverse range!"); }
  };

  template <bool> struct IndexImpl {
    template <typename T> static auto call(const T &r, size_t n) -> typename Range<T>::ElementType { return r[n]; }
  };
  template <> struct IndexImpl<false> {
    template <typename T> static auto call(const T &, size_t) -> typename Range<T>::ElementType { EPTHROW(Result::InvalidType, "Not a random range!"); }
  };
  // ***** PHEW, WE SURVIVED! *****

} // namespace internal

template <typename T> inline RangeFeatures Range<T>::features() const { return internal::FeaturesImpl<HasFeatures>::template call<T>(range); }

template <typename T> inline bool Range<T>::empty() const { return internal::EmptyImpl<HasEmpty>::template call<T>(range); }
template <typename T> inline size_t Range<T>::length() const { return internal::LengthImpl<HasLength>::template call<T>(range); }

template <typename T> inline auto Range<T>::front() const -> typename Range<T>::ElementType { return internal::GetFrontImpl<HasFront>::template call<T>(range); }
template <typename T> inline auto Range<T>::popFront() -> typename Range<T>::ElementType { return internal::PopFrontImpl<HasFront>::template call<T>(range); }

template <typename T> inline auto Range<T>::back() const -> typename Range<T>::ElementType { return internal::GetBackImpl<HasBack>::template call<T>(range); }
template <typename T> inline auto Range<T>::popBack() -> typename Range<T>::ElementType { return internal::PopBackImpl<HasBack>::template call<T>(range); }

template <typename T> inline auto Range<T>::operator[](size_t index) const -> typename Range<T>::ElementType { return internal::IndexImpl<HasIndex>::template call<T>(range, index); }

// TODO: sfinae for these...
template <typename T> inline auto Range<T>::begin() const -> typename Range<T>::Iterator { return range.begin(); }
template <typename T> inline auto Range<T>::end() const -> typename Range<T>::Iterator { return range.end(); }

} // namespace ep
