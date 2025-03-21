#ifndef TEST_H
#define TEST_H
#include <iostream>
#include <rainy/meta/reflection/function.hpp>
#include <string>
#include <vector>

#if RAINY_USING_WINDOWS
#if RAINY_USING_DYNAMIC
#ifdef RAINY_DYNAMIC_EXPORTS
#define TEST_DLL_API __declspec(dllexport)
#else
#define TEST_DLL_API __declspec(dllimport)
#endif
#else
#define TEST_DLL_API
#endif
#else
#define TEST_DLL_API
#endif

TEST_DLL_API rainy::meta::reflection::function get_fn();
TEST_DLL_API rainy::meta::reflection::function get_fn1();
TEST_DLL_API rainy::meta::reflection::instance get_obj();
TEST_DLL_API rainy::meta::reflection::instance get_param();

#endif