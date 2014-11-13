#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <functional>

#include <iod/tags.hh>
#include <iod/sio.hh>
#include <iod/grammar.hh>

namespace iod
{
  template <typename ...T>
  struct sio;

  template <typename ...T>
  inline auto D(T&&... args);

  template <typename T, typename ...Tail>
  inline std::vector<T> iod_array(const T& t, Tail... args);


  template <typename E>
  struct variable;

  template <typename V>
  const V& exp_to_variable(const variable<V>& v)
  {
    return *static_cast<const V*>(&v);
  }

  template <typename S>
  const S& exp_to_variable(const symbol<S>& s)
  {
    return *static_cast<const S*>(&s);
  }

  template <typename S, typename T>
  auto exp_to_variable(const function_call_exp<S, T>& c)
  {
    return typename S::template variable_type<T>(std::get<0>(c.args));
  }

  template <typename S, typename V>
  auto exp_to_variable(const assign_exp<S, V>& e)
  {
    typedef V vtype;
    return typename S::template variable_type<vtype>(e.right);
  }

  template <typename S, typename V, typename... ARGS>
  auto exp_to_variable(const assign_exp<function_call_exp<S, ARGS...>, V>& e)
  {
    typedef V vtype;
    return typename S::template variable_type<vtype, decltype(D(std::declval<ARGS>()...))>(e.right);
  }

  template <typename ...T>
  inline auto D(T&&... args)
  {
    typedef
      sio<std::remove_const_t<std::remove_reference_t<decltype(exp_to_variable(args))>>...>
      result_type;

    return result_type(exp_to_variable(args)...);
  }

  template <int N>
  struct transform_runner
  {
    template <typename F, typename O>
    static inline auto run(O o, F f)
    {
      return cat(transform_runner<N-1>::run(o, f),
                 f(o.template get_nth_attribute<N>()));
    }
  };

  template <typename F, typename... T>
  void apply_variables(F f, const sio<T...>& o)
  {
    return f(*static_cast<const T*>(&o)...);
  }

  template <typename F, typename... T>
  void apply_values(F f, const sio<T...>& o)
  {
    return f(static_cast<const T*>(&o)->value()...);
  }

  template <unsigned I, unsigned S, unsigned O>
  struct paste
  {
    template <typename ...T, typename ...U>
    static void run(sio<T...>& a,
                    const sio<U...>& b)
    {
      a.template get_nth<I + O>() = b.template get_nth<I>();
      paste<I+1, S, O>::run(a, b);
    }
  };

  template <unsigned S, unsigned O>
  struct paste<S, S, O>
  {
    template <typename ...T, typename ...U>
    static void run(sio<T...>& a,
                    const sio<U...>& b)
    {
    }
  };

  template <typename ...T, typename ...U>
  inline auto cat(const sio<T...>& a,
                  const sio<U...>& b)
  {
    return sio<T..., U...>(*static_cast<const T*>(&a)...,
                                  *static_cast<const U*>(&b)...);
  }

  template <typename ...T, typename V>
  inline auto cat(const sio<T...>& a,
                  const V& variable)
  {
    return sio<T..., decltype(exp_to_variable(variable))>
      (*static_cast<const T*>(&a)..., exp_to_variable(variable));
  }

  template <typename T, typename ...Tail>
  inline std::vector<T> iod_array(const T& t, Tail... args)
  {
    std::vector<T> res;
    res.reserve(1 + sizeof...(args));
    iod_internals::array_fill<T, Tail...>(t, args..., res);
    return res;
  }

} // end of namespace iod.

#include <iod/symbol.hh>