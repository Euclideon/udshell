// delegate_inl.h
namespace ep {
namespace internal {

template<typename C, typename R, typename ...Args>
struct function_traits<R(C::*)(Args...)>
{
  static const size_t num_args = sizeof...(Args);

  using result_type = R;

  using args = std::tuple<Args...>;
  using function_signature = R(Args...);

  template <size_t i>
  struct arg
  {
    using type = typename std::tuple_element<i, args>::type;
  };
};
template<typename C, typename R, typename ...Args>
struct function_traits<R(C::*)(Args...) const>
{
  static const size_t num_args = sizeof...(Args);

  using result_type = R;

  using args = std::tuple<Args...>;
  using function_signature = R(Args...);

  template <size_t i>
  struct arg
  {
    using type = typename std::tuple_element<i, args>::type;
  };
};

} // namespace internal

template <typename T, typename R, typename... Args>
class LambdaMemento : public DelegateMemento
{
public:
  template <typename U>
  LambdaMemento(U &&t)
    : DelegateMemento(fastdelegate::MakeDelegate(&instance, &T::operator()).GetMemento())
    , instance(std::forward<U>(t))
  {}

private:
  T instance;
};

template <typename R, typename... Args>
template <typename T, internal::enable_if_valid_functor<T, Delegate<R(Args...)>, R, Args...>*>
Delegate<R(Args...)>::Delegate(T &&lambda)
  : Delegate((DelegateMementoRef)SharedPtr<LambdaMemento<typename std::remove_reference<T>::type, R, Args...>>::create(std::forward<T>(lambda)))
{}

} // namespace ep
