#pragma once

/// \file

/// Gets the number of arguments of a variadic preprocessor macro.
/// If an empty __VA_ARGS__ is passed in, this will still return 1.
/// There is no perfect way to detect parameter lists with zero elements.
#ifndef WD_VA_NUM_ARGS
#  define WD_VA_NUM_ARGS(...) WD_VA_NUM_ARGS_HELPER(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#  define WD_VA_NUM_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#endif


#define WD_CALL_MACRO(macro, args) macro args


#define WD_EXPAND_ARGS_1(op, a0) op(a0)
#define WD_EXPAND_ARGS_2(op, a0, a1) op(a0) op(a1)
#define WD_EXPAND_ARGS_3(op, a0, a1, a2) op(a0) op(a1) op(a2)
#define WD_EXPAND_ARGS_4(op, a0, a1, a2, a3) op(a0) op(a1) op(a2) op(a3)
#define WD_EXPAND_ARGS_5(op, a0, a1, a2, a3, a4) op(a0) op(a1) op(a2) op(a3) op(a4)
#define WD_EXPAND_ARGS_6(op, a0, a1, a2, a3, a4, a5) op(a0) op(a1) op(a2) op(a3) op(a4) op(a5)
#define WD_EXPAND_ARGS_7(op, a0, a1, a2, a3, a4, a5, a6) op(a0) op(a1) op(a2) op(a3) op(a4) op(a5) op(a6)
#define WD_EXPAND_ARGS_8(op, a0, a1, a2, a3, a4, a5, a6, a7) op(a0) op(a1) op(a2) op(a3) op(a4) op(a5) op(a6) op(a7)
#define WD_EXPAND_ARGS_9(op, a0, a1, a2, a3, a4, a5, a6, a7, a8) op(a0) op(a1) op(a2) op(a3) op(a4) op(a5) op(a6) op(a7) op(a8)
#define WD_EXPAND_ARGS_10(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9) op(a0) op(a1) op(a2) op(a3) op(a4) op(a5) op(a6) op(a7) op(a8) op(a9)
#define WD_EXPAND_ARGS_11(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
  op(a0) op(a1) op(a2) op(a3) op(a4) op(a5) op(a6) op(a7) op(a8) op(a9) op(a10)
#define WD_EXPAND_ARGS_12(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  op(a0) op(a1) op(a2) op(a3) op(a4) op(a5) op(a6) op(a7) op(a8) op(a9) op(a10) op(a11)

/// Variadic macro "dispatching" the arguments to the correct macro.
/// The number of arguments is found by using WD_VA_NUM_ARGS(__VA_ARGS__)
#define WD_EXPAND_ARGS(op, ...) WD_CALL_MACRO(WD_CONCAT(WD_EXPAND_ARGS_, WD_VA_NUM_ARGS(__VA_ARGS__)), (op, __VA_ARGS__))

//////////////////////////////////////////////////////////////////////////

#define WD_EXPAND_ARGS_WITH_INDEX_1(op, a0) op(a0, 0)
#define WD_EXPAND_ARGS_WITH_INDEX_2(op, a0, a1) op(a0, 0) op(a1, 1)
#define WD_EXPAND_ARGS_WITH_INDEX_3(op, a0, a1, a2) op(a0, 0) op(a1, 1) op(a2, 2)
#define WD_EXPAND_ARGS_WITH_INDEX_4(op, a0, a1, a2, a3) op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3)
#define WD_EXPAND_ARGS_WITH_INDEX_5(op, a0, a1, a2, a3, a4) op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4)
#define WD_EXPAND_ARGS_WITH_INDEX_6(op, a0, a1, a2, a3, a4, a5) op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4) op(a5, 5)
#define WD_EXPAND_ARGS_WITH_INDEX_7(op, a0, a1, a2, a3, a4, a5, a6) op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4) op(a5, 5) op(a6, 6)
#define WD_EXPAND_ARGS_WITH_INDEX_8(op, a0, a1, a2, a3, a4, a5, a6, a7) \
  op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4) op(a5, 5) op(a6, 6) op(a7, 7)
#define WD_EXPAND_ARGS_WITH_INDEX_9(op, a0, a1, a2, a3, a4, a5, a6, a7, a8) \
  op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4) op(a5, 5) op(a6, 6) op(a7, 7) op(a8, 8)
#define WD_EXPAND_ARGS_WITH_INDEX_10(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  op(a0, 0) op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4) op(a5, 5) op(a6, 6) op(a7, 7) op(a8, 8) op(a9, 9)

#define WD_EXPAND_ARGS_WITH_INDEX(op, ...) WD_CALL_MACRO(WD_CONCAT(WD_EXPAND_ARGS_WITH_INDEX_, WD_VA_NUM_ARGS(__VA_ARGS__)), (op, __VA_ARGS__))

//////////////////////////////////////////////////////////////////////////

#define WD_EXPAND_ARGS_PAIR_1(...)
#define WD_EXPAND_ARGS_PAIR_2(op, a0, a1) op(a0, a1)
#define WD_EXPAND_ARGS_PAIR_3(op, a0, a1, ...) op(a0, a1)
#define WD_EXPAND_ARGS_PAIR_4(op, a0, a1, a2, a3) op(a0, a1) op(a2, a3)
#define WD_EXPAND_ARGS_PAIR_6(op, a0, a1, a2, a3, a4, a5) op(a0, a1) op(a2, a3) op(a4, a5)
#define WD_EXPAND_ARGS_PAIR_8(op, a0, a1, a2, a3, a4, a5, a6, a7) op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7)
#define WD_EXPAND_ARGS_PAIR_10(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9) op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7) op(a8, a9)
#define WD_EXPAND_ARGS_PAIR_12(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7) op(a8, a9) op(a10, a11)
#define WD_EXPAND_ARGS_PAIR_14(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7) op(a8, a9) op(a10, a11) op(a12, a13)
#define WD_EXPAND_ARGS_PAIR_16(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
  op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7) op(a8, a9) op(a10, a11) op(a12, a13) op(a14, a15)
#define WD_EXPAND_ARGS_PAIR_18(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) \
  op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7) op(a8, a9) op(a10, a11) op(a12, a13) op(a14, a15) op(a16, a17)
#define WD_EXPAND_ARGS_PAIR_20(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) \
  op(a0, a1) op(a2, a3) op(a4, a5) op(a6, a7) op(a8, a9) op(a10, a11) op(a12, a13) op(a14, a15) op(a16, a17) op(a18, a19)

#define WD_EXPAND_ARGS_PAIR(op, ...) WD_CALL_MACRO(WD_CONCAT(WD_EXPAND_ARGS_PAIR_, WD_VA_NUM_ARGS(__VA_ARGS__)), (op, __VA_ARGS__))

//////////////////////////////////////////////////////////////////////////

#define WD_EXPAND_ARGS_PAIR_COMMA_1(...) /* handles the case of zero parameters (e.g. an empty __VA_ARGS__) */
#define WD_EXPAND_ARGS_PAIR_COMMA_2(op, a0, a1) op(a0, a1)
#define WD_EXPAND_ARGS_PAIR_COMMA_3(op, a0, a1, ...) op(a0, a1)
#define WD_EXPAND_ARGS_PAIR_COMMA_4(op, a0, a1, a2, a3) op(a0, a1), op(a2, a3)
#define WD_EXPAND_ARGS_PAIR_COMMA_6(op, a0, a1, a2, a3, a4, a5) op(a0, a1), op(a2, a3), op(a4, a5)
#define WD_EXPAND_ARGS_PAIR_COMMA_8(op, a0, a1, a2, a3, a4, a5, a6, a7) op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7)
#define WD_EXPAND_ARGS_PAIR_COMMA_10(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9) op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7), op(a8, a9)
#define WD_EXPAND_ARGS_PAIR_COMMA_12(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
  op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7), op(a8, a9), op(a10, a11)
#define WD_EXPAND_ARGS_PAIR_COMMA_14(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
  op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7), op(a8, a9), op(a10, a11), op(a12, a13)
#define WD_EXPAND_ARGS_PAIR_COMMA_16(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
  op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7), op(a8, a9), op(a10, a11), op(a12, a13), op(a14, a15)
#define WD_EXPAND_ARGS_PAIR_COMMA_18(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) \
  op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7), op(a8, a9), op(a10, a11), op(a12, a13), op(a14, a15), op(a16, a17)
#define WD_EXPAND_ARGS_PAIR_COMMA_20(op, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) \
  op(a0, a1), op(a2, a3), op(a4, a5), op(a6, a7), op(a8, a9), op(a10, a11), op(a12, a13), op(a14, a15), op(a16, a17), op(a18, a19)

#define WD_EXPAND_ARGS_PAIR_COMMA(op, ...) WD_CALL_MACRO(WD_CONCAT(WD_EXPAND_ARGS_PAIR_COMMA_, WD_VA_NUM_ARGS(__VA_ARGS__)), (op, __VA_ARGS__))

//////////////////////////////////////////////////////////////////////////

#define WD_TO_BOOL_0 0
#define WD_TO_BOOL_1 1
#define WD_TO_BOOL_2 1
#define WD_TO_BOOL_3 1
#define WD_TO_BOOL_4 1
#define WD_TO_BOOL_5 1
#define WD_TO_BOOL_6 1
#define WD_TO_BOOL_7 1
#define WD_TO_BOOL_8 1
#define WD_TO_BOOL_9 1

#define WD_TO_BOOL(x) WD_CONCAT(WD_TO_BOOL_, x)

//////////////////////////////////////////////////////////////////////////

#define WD_IF_0(x)
#define WD_IF_1(x) x
#define WD_IF(cond, x)                \
  WD_CONCAT(WD_IF_, WD_TO_BOOL(cond)) \
  (x)

#define WD_IF_ELSE_0(x, y) y
#define WD_IF_ELSE_1(x, y) x
#define WD_IF_ELSE(cond, x, y)             \
  WD_CONCAT(WD_IF_ELSE_, WD_TO_BOOL(cond)) \
  (x, y)

//////////////////////////////////////////////////////////////////////////

#define WD_COMMA_MARK_0
#define WD_COMMA_MARK_1 ,
#define WD_COMMA_IF(cond) WD_CONCAT(WD_COMMA_MARK_, WD_TO_BOOL(cond))

//////////////////////////////////////////////////////////////////////////

#define WD_LIST_0(x)
#define WD_LIST_1(x) WD_CONCAT(x, 0)
#define WD_LIST_2(x) WD_LIST_1(x), WD_CONCAT(x, 1)
#define WD_LIST_3(x) WD_LIST_2(x), WD_CONCAT(x, 2)
#define WD_LIST_4(x) WD_LIST_3(x), WD_CONCAT(x, 3)
#define WD_LIST_5(x) WD_LIST_4(x), WD_CONCAT(x, 4)
#define WD_LIST_6(x) WD_LIST_5(x), WD_CONCAT(x, 5)
#define WD_LIST_7(x) WD_LIST_6(x), WD_CONCAT(x, 6)
#define WD_LIST_8(x) WD_LIST_7(x), WD_CONCAT(x, 7)
#define WD_LIST_9(x) WD_LIST_8(x), WD_CONCAT(x, 8)
#define WD_LIST_10(x) WD_LIST_9(x), WD_CONCAT(x, 9)

#define WD_LIST(x, count)    \
  WD_CONCAT(WD_LIST_, count) \
  (x)

//////////////////////////////////////////////////////////////////////////

#define WD_PAIR_LIST_0(x, y)
#define WD_PAIR_LIST_1(x, y) \
  WD_CONCAT(x, 0)            \
  WD_CONCAT(y, 0)
#define WD_PAIR_LIST_2(x, y) WD_PAIR_LIST_1(x, y), WD_CONCAT(x, 1) WD_CONCAT(y, 1)
#define WD_PAIR_LIST_3(x, y) WD_PAIR_LIST_2(x, y), WD_CONCAT(x, 2) WD_CONCAT(y, 2)
#define WD_PAIR_LIST_4(x, y) WD_PAIR_LIST_3(x, y), WD_CONCAT(x, 3) WD_CONCAT(y, 3)
#define WD_PAIR_LIST_5(x, y) WD_PAIR_LIST_4(x, y), WD_CONCAT(x, 4) WD_CONCAT(y, 4)
#define WD_PAIR_LIST_6(x, y) WD_PAIR_LIST_5(x, y), WD_CONCAT(x, 5) WD_CONCAT(y, 5)
#define WD_PAIR_LIST_7(x, y) WD_PAIR_LIST_6(x, y), WD_CONCAT(x, 6) WD_CONCAT(y, 6)
#define WD_PAIR_LIST_8(x, y) WD_PAIR_LIST_7(x, y), WD_CONCAT(x, 7) WD_CONCAT(y, 7)
#define WD_PAIR_LIST_9(x, y) WD_PAIR_LIST_8(x, y), WD_CONCAT(x, 8) WD_CONCAT(y, 8)
#define WD_PAIR_LIST_10(x, y) WD_PAIR_LIST_9(x, y), WD_CONCAT(x, 9) WD_CONCAT(y, 9)

#define WD_PAIR_LIST(x, y, count) \
  WD_CONCAT(WD_PAIR_LIST_, count) \
  (x, y)
