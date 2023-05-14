#include "types.h"
#include "helpers.h"
#include <string>

#pragma once

struct testStruct{
    std::string fen;
    depth_t depthLim;
    uint64 expected;

    testStruct(std::string fen_, depth_t depthLim_, uint64 expected_):
        fen(fen_),
        depthLim(depthLim_),
        expected(expected_)
    {}
};

void testPerft();