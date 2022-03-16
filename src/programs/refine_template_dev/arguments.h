#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

#include <any>

namespace cistem {
namespace arguments {
enum {
    BooleanArgument,
    IntegerArgument,
    FloatArgument,
    DoubleArgument,
    StringArgument,
    FilenameArgument,
};

struct cisTEMArgumentDescription {
    int         type;
    std::string description;
    std::any    default_value;

    constexpr char type_char( ) {
        if ( type == IntegerArgument )
            return 'i';
        if ( type == DoubleArgument )
            return 'd';
        if ( type == StringArgument )
            return 't';
        if ( type == FilenameArgument )
            return 't';
        if ( type == BooleanArgument )
            return 'b';
        if ( type == FloatArgument )
            return 'f';
        return '?';
    };
};
} // namespace arguments
} // namespace cistem

#endif