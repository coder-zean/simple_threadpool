/**
 * @file function_wrapper.h
 * @author zean (13071517766@163.com)
 * @brief 可调用对象包装类（这个文件代码仿写的是std::thread源码）
 * @version 0.1
 * @date 2023-02-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>


template <typename T>
struct SuccessType {
  typedef T Type;
};

struct FailureType {};

// ============================================================================
// 用于获取可调用对象的返回类型，以及可调用对象属于哪种可调用类型
// 可调用类型有五种：
// 1. InvokeMemFuncRef：非静态成员函数，且调用该非静态成员函数的是一个对象
// 2. InvokeMemFuncDeref：非静态成员函数，且调用该非静态成员函数的是一个对象指针
// 3. InvokeMemObjRef：非静态成员变量，且调用该非静态成员变量的是一个对象
// 4. InvokeMemObjDeref：非静态成员变量，且调用该非静态成员变量的是一个对象指针
// 5.
// InvokeOther：其他可调用类型，包括lambda、有operator()对象、普通函数、静态函数

// 用于包装可调用对象返回值类型和可调用对象所属的调用类型tag
template <typename T, typename InvokeTag>
struct ResSuccessType : public SuccessType<T> {
  typedef InvokeTag Tag;
};

struct InvokeMemFuncRef {};
struct InvokeMemFuncDeref {};
struct InvokeMemObjRef {};
struct InvokeMemObjDeref {};
struct InvokeOther {};

template <typename MemFunc, typename ClassObj, typename... Args>
struct ResultOfMemFuncRef {
  typedef ResSuccessType<decltype((std::declval<ClassObj>().*
                                   std::declval<MemFunc>())(
                             std::declval<Args>()...)),
                         InvokeMemFuncRef>
      Type;
};

template <typename MemFunc, typename ClassPtr, typename... Args>
struct ResultOfMemFuncDeref {
  typedef ResSuccessType<decltype(((*std::declval<ClassPtr>()).*
                                   std::declval<MemFunc>())(
                             std::declval<Args>()...)),
                         InvokeMemFuncDeref>
      Type;
};

template <typename MemObj, typename ClassObj>
struct ResultOfMemObjRef {
  typedef ResSuccessType<decltype(std::declval<ClassObj>().*
                                  std::declval<MemObj>()),
                         InvokeMemObjRef>
      Type;
};

template <typename MemObj, typename ClassPtr>
struct ResultOfMemObjDeref {
  typedef ResSuccessType<decltype((*std::declval<ClassPtr>()).*
                                  std::declval<MemObj>()),
                         InvokeMemObjDeref>
      Type;
};

template <typename Func, typename... Args>
struct ResultOfOther {
  typedef ResSuccessType<
      decltype(std::declval<Func>()(std::declval<Args>()...)), InvokeOther>
      Type;
};

template <typename MemFunc, typename ClassArg, typename... Args>
struct ResultOfMemFunc {};

template <typename Res, typename Class, typename ClassArg, typename... Args>
struct ResultOfMemFunc<Res Class::*, ClassArg, Args...> {
  typedef typename std::remove_reference<ClassArg>::type ArgVal;
  typedef Res Class::*MemPtr;
  // is_base_of用于判断传入的用于调用成员函数的对象是不是一个指针
  typedef typename std::conditional<
      std::is_base_of<Class, ArgVal>::value,
      ResultOfMemFuncRef<MemPtr, ClassArg, Args...>,
      ResultOfMemFuncDeref<MemPtr, ClassArg, Args...>>::type::Type Type;
};

template <typename MemObj, typename ClassArg, typename... Args>
struct ResultOfMemObj {};

template <typename Res, typename Class, typename ClassArg, typename... Args>
struct ResultOfMemObj<Res Class::*, ClassArg, Args...> {
  using ArgVal = std::__remove_cvref_t<ClassArg>;
  typedef Res Class::*MemPtr;
  typedef typename std::conditional<
      std::is_base_of<Class, ArgVal>::value,
      ResultOfMemObjRef<MemPtr, ClassArg, Args...>,
      ResultOfMemObjDeref<MemPtr, ClassArg, Args...>>::type::Type Type;
};

template <bool, bool, typename Callable, typename... Args>
struct ResultOf {
  typedef FailureType Type;
};

template <typename MemFunc, typename ClassArg, typename... Args>
struct ResultOf<true, false, MemFunc, ClassArg, Args...>
    : public ResultOfMemFunc<typename std::decay<MemFunc>::type,
                             typename std::__inv_unwrap<ClassArg>::type,
                             Args...> {};

template <typename MemObj, typename ClassArg>
struct ResultOf<false, true, MemObj, ClassArg>
    : public ResultOfMemObj<typename std::decay<MemObj>::type,
                            typename std::__inv_unwrap<ClassArg>::type> {};

template <typename Func, typename... Args>
struct ResultOf<false, false, Func, Args...>
    : public ResultOfOther<Func, Args...> {};

//
template <typename Callable, typename... Args>
struct InvokeResult
    : public ResultOf<
          std::is_member_function_pointer<
              typename std::remove_reference<Callable>::type>::value,
          std::is_member_object_pointer<
              typename std::remove_reference<Callable>::type>::value,
          Callable, Args...>::Type {};

// ============================================================================

// ============================================================================
// 实际调用任务函数部分，共有5种调用类型，故有五个InvokeImpl函数的重载
// 具体调用哪个InvokeImpl，由Invoke函数
template <typename Res, typename MemFunc, typename ClassArg, typename... Args>
constexpr Res InvokeImpl(InvokeMemFuncRef, MemFunc&& func, ClassArg&& class_arg,
                         Args&&... args) {
  return (std::__invfwd<ClassArg>(class_arg).*
          func)(std::forward<Args>(args)...);
}

template <typename Res, typename MemFunc, typename ClassArg, typename... Args>
constexpr Res InvokeImpl(InvokeMemFuncDeref, MemFunc&& func,
                         ClassArg&& class_arg, Args&&... args) {
  return ((*std::forward<ClassArg>(class_arg)).*
          func)(std::forward<Args>(args)...);
}

template <typename Res, typename MemObj, typename ClassArg>
constexpr Res InvokeImpl(InvokeMemObjRef, MemObj&& obj, ClassArg&& class_arg) {
  // 这里__invfwd而不是forward，是为了处理出现reference_wrapper的情况
  return std::__invfwd<ClassArg>(class_arg).*obj;
}

template <typename Res, typename MemObj, typename ClassArg>
constexpr Res InvokeImpl(InvokeMemObjDeref, MemObj&& obj,
                         ClassArg&& class_arg) {
  return (*std::forward<ClassArg>(class_arg)).*obj;
}

template <typename Res, typename Func, typename... Args>
constexpr Res InvokeImpl(InvokeOther, Func&& func, Args&&... args) {
  return std::forward<Func>(func)(std::forward<Args>(args)...);
}

template <typename Callable, typename... Args>
typename InvokeResult<Callable, Args...>::Type Invoke(Callable&& func,
                                                      Args&&... args) {
  typedef typename InvokeResult<Callable, Args...>::Tag CallableTag;
  typedef typename InvokeResult<Callable, Args...>::Type ResType;
  // PrintArgsHelper(args...);
  return InvokeImpl<ResType>(CallableTag{}, std::forward<Callable>(func),
                             std::forward<Args>(args)...);
}

// ============================================================================

template <typename Tuple>
struct FunctionWrapper {
  Tuple func_and_args_;

  template <typename>
  struct Result {};

  template <typename Callable, typename... Args>
  struct Result<std::tuple<Callable, Args...>>
      : public InvokeResult<Callable, Args...> {};

  template <std::size_t... Indexs>
  typename Result<Tuple>::Type InvokeHelper(std::_Index_tuple<Indexs...>) {
    return Invoke(std::get<Indexs>(std::move(func_and_args_))...);
  }

  typename Result<Tuple>::Type operator()() {
    // Build_index_tuple返回的是一个Index_tuple类型，这个类型的模板参数是数字，
    // 例：Index_tuple<1, 2, 3, 4, ..., N> t; // 定义了一个Index_tuple
    using Indexs =
        typename std::_Build_index_tuple<std::tuple_size<Tuple>::value>::__type;
    return InvokeHelper(Indexs());
  }
};

template <typename... T>
using DecayTuple = std::tuple<typename std::decay<T>::type...>;

template <typename Callable, typename... Args>
static FunctionWrapper<DecayTuple<Callable, Args...>> MakeFuncWrapper(
    Callable&& callable, Args&&... args) {
  return {DecayTuple<Callable, Args...>{std::forward<Callable>(callable),
                                        std::forward<Args>(args)...}};
}
