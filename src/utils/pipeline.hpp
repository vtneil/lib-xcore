#ifndef LIB_XCORE_UTILS_PIPELINE_HPP
#define LIB_XCORE_UTILS_PIPELINE_HPP

// Idea:
// pipeline_t pipe;
// auto result = pipe.run(func1)
//                   .run(func2)
//                   .run(func3);
//
// func1()   -> T1
// func2(T1) -> T2
// func3(T2) -> T3
//
// const T &, T &, T &&

namespace xcore {

}

#endif  //LIB_XCORE_UTILS_PIPELINE_HPP
