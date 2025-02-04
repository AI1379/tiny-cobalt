//
// Created by Renatus Madrigal on 02/04/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_ASSET_H_
#define TINY_COBALT_INCLUDE_COMMON_ASSET_H_

#include <exception>
#include <iostream>

#define TINY_COBALT_CONCEPT_ASSERT(Concept, Class) static_assert(Concept<Class>, #Class " must satisfy " #Concept)

// TODO: Implement a more detailed assert
#define TINY_COBALT_ASSERT(Expr, Msg)                                                                                 \
    do {                                                                                                               \
        if (!Expr) {                                                                                                   \
            std::cerr << Msg << std::endl;                                                                             \
            std::terminate();                                                                                          \
        }                                                                                                              \
    } while (0)

#endif // TINY_COBALT_INCLUDE_COMMON_ASSET_H_