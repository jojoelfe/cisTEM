#include <any>
#include <array>
#include <string>
#include <climits>
#include <cfloat>
#include <iostream>
#include <wx/wx.h>
#include <wx/socket.h>
#include <sstream>
#include "../../core/defines.h"
#include "../../core/userinput.h"
#include "../../core/run_profiles.h"
#include "../../core/job_packager.h"
#include "../../core/arguments.h"
#include "../../../include/catch2/catch.hpp"
#include <string>
#include <cstring>

using namespace cistem;

TEST_CASE("Arguments class", "[Arguments]") {
    enum {
        input_starfile = 0,
        max_threads,
        whiten_image,
        arguments_length // Must stay on the bottom
    };

    arguments::ArgumentDescription TestArgumentsDescription[arguments_length] = {
            {arguments::FilenameArgument,
             "Input starfile",
             std::string("input.star")},
            {arguments::IntegerArgument,
             "Maximum number of threads to use",
             1},
            {arguments::BooleanArgument,
             "Should I whiten the image?",
             false}};

    arguments::Arguments test_args = arguments::Arguments<arguments_length>(TestArgumentsDescription, "TestProgram", 1.00);

    char arg_string[arguments_length];
    test_args.generate_argument_string(arg_string);
    CHECK_THAT(arg_string, Catch::Matchers::Equals("tib"));
    REQUIRE(test_args.get<std::string>(input_starfile) == "input.star");
    REQUIRE(test_args.get<int>(max_threads) == 1);
    REQUIRE(test_args.get<bool>(whiten_image) == false);
    test_args.set<bool>(whiten_image, true);
    REQUIRE(test_args.get<bool>(whiten_image) == true);
}