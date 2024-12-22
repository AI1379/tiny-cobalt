//
// Created by Renatus Madrigal on 12/18/2024
//

#include <gtest/gtest.h>

#include "Common/Dumper.h"

using TinyCobalt::Utility::Dumper;

TEST(Common, Dumper1) {
    std::stringstream ss;
    Dumper dumper(ss);
    dumper << Dumper::Controller::StartBlock << Dumper::Controller::EndBlock << ';';
    EXPECT_EQ(ss.str(), "{\n};");
}

TEST(Common, Dumper2) {
    std::stringstream ss;
    Dumper dumper(ss, 4);
    dumper << Dumper::Controller::StartBlock << Dumper::Controller::EndLine << Dumper::Controller::EndBlock << ';';
    EXPECT_EQ(ss.str(), "{\n    \n};");
}

TEST(Common, Dumper3) {
    std::stringstream ss;
    Dumper dumper(ss, 4);
    dumper.setEndLine(';').setIntendChar('`').setIntend(8);
    dumper.dump(Dumper::Controller::StartBlock)
            .dump(Dumper::Controller::EndLine)
            .dump("Dumper Test")
            .endl()
            .dump(Dumper::Controller::EndBlock);
    EXPECT_EQ(ss.str(), "{;````````Dumper Test;````````;}");
}