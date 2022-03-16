#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

namespace cistem
{
    namespace arguments
    {
        enum class ArgumentType
        {
            BOOLEAN,
            INTEGER,
            FLOAT,
            DOUBLE,
            STRING,
        };
        struct cisTEMArgumentDescription
        {
            ArgumentType type;
            std::string description;
            std::any default_value;
            constexpr char type_char()
            {
                if (type == ArgumentType::INTEGER)
                    return 'i';
                if (type == ArgumentType::DOUBLE)
                    return 'd';
                if (type == ArgumentType::INTEGER)
                    return 't';
                if (type == ArgumentType::BOOLEAN)
                    return 'b';
                if (type == ArgumentType::FLOAT)
                    return 'f';
                return '?';
            };
        };
    }
}

#endif