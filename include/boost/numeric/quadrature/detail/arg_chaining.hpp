#ifndef BOOST_NUMERIC_QUADRATURE_ARG_CHAINING_H
#define BOOST_NUMERIC_QUADRATURE_ARG_CHAINING_H

#if defined(MSC_VER) && MSC_VER>1000
#pragma once
#endif

// Copyright (C) 2007 by Hugo Duncan

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)

/*! arg_chaining  routines
  @file   arg_chaining.hpp
  @brief  arg_chaining routines
*/


namespace boost
{
  namespace numeric
  {
    namespace quadrature
    {
      namespace detail
      {
        // default argument tag
        struct defarg{};

        // struct for holding an argument by const reference
        template <typename Arg>
        struct arg_cref
        {
          arg_cref(Arg const&  r) : m_ref(r) {}
          Arg const&  m_ref;
        };

        // struct for holding an argument by reference
        template <typename Arg>
        struct arg_ref
        {
          arg_ref(Arg& r) : m_ref(r) {}
          Arg& m_ref;
        };

        // struct for holding an argument by value
        template <typename Arg>
        struct arg_value
        {
          arg_value(Arg v) : m_value(v) {}
          Arg m_value;
        };

        // structs for recognising a default argument
        template <>
        struct arg_cref<defarg>
        {};

        template <>
        struct arg_ref<defarg>
        {};

        template <>
        struct arg_value<defarg>
        {};

      }// namespace
    }// namespace
  }// namespace
}// namespace


// macro to define a template type for an optional argument
#define BOOST_QUADRATURE_ARG_TEMPLATE(template_arg_name) \
  typename template_arg_name=detail::defarg

// macro to define storage and default values for optional arguments
#define BOOST_QUADRATURE_CREF_ARG(var_name, template_arg_name, \
                                  default_type, default_expr)           \
  detail::arg_cref<template_arg_name> BOOST_PP_CAT(m_,var_name);        \
  default_type BOOST_PP_CAT(var_name,_)(detail::arg_cref<detail::defarg>)const \
  {                                                                     \
    return default_expr;                                                \
  }                                                                     \
  template <typename T>                                                 \
  T const& BOOST_PP_CAT(var_name,_)(const detail::arg_cref<T>& v)const  \
  {                                                                     \
    return v.m_ref;                                                     \
  }

#define BOOST_QUADRATURE_REF_ARG(var_name, template_arg_name, default_type, default_expr) \
  mutable detail::arg_ref<template_arg_name> BOOST_PP_CAT(m_,var_name); \
  default_type& BOOST_PP_CAT(var_name,_)(detail::arg_ref<detail::defarg>) const\
  {                                                                     \
    return default_expr;                                                \
  }                                                                     \
  template <typename T>                                                 \
  T& BOOST_PP_CAT(var_name,_)(detail::arg_ref<T>& v) const              \
  {                                                                     \
    return v.m_ref;                                                     \
  }

#define BOOST_QUADRATURE_VALUE_ARG(var_name, template_arg_name, default_type, default_expr) \
  detail::arg_value<template_arg_name> BOOST_PP_CAT(m_,var_name);       \
  default_type BOOST_PP_CAT(var_name,_)(detail::arg_value<detail::defarg>)const \
  {                                                                     \
    return default_expr;                                                \
  }                                                                     \
  template <typename T>                                                 \
  T BOOST_PP_CAT(var_name,_)(const detail::arg_value<T>& v) const       \
  {                                                                     \
    return v.m_value;                                                   \
  }


#define BOOST_QUADRATURE_CONSTRUCTOR_CREF_ARG(var_name, template_arg_name) \
  detail::arg_cref<template_arg_name> var_name=\
    detail::arg_cref<template_arg_name>()
#define BOOST_QUADRATURE_CONSTRUCTOR_REF_ARG(var_name, template_arg_name) \
  detail::arg_ref<template_arg_name> var_name=\
    detail::arg_ref<template_arg_name>()
#define BOOST_QUADRATURE_CONSTRUCTOR_VALUE_ARG(var_name, template_arg_name) \
  detail::arg_value<template_arg_name> var_name=\
    detail::arg_value<template_arg_name>()


#endif
