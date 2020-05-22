// Copyright 2020 <mmeshcher>

#pragma once

#include <string>

struct Globals
{
    static std::string logLevel;
    static size_t threadAmount;
    static std::string output;
    static std::string input;
    static bool writeOnly;
};

std::string Globals::logLevel{};
size_t Globals::threadAmount{0};
std::string Globals::output{};
std::string Globals::input{};
bool Globals::writeOnly{false};
static const size_t BAREV = 1;
static const size_t CHKORES = 5;
static const size_t MINCH = 25;
static const size_t CHARACTERS = 62;
