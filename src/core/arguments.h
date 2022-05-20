#ifndef __CORE_ARGUMENTS_H__
#define __CORE_ARGUMENTS_H__

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

/** 
 * @brief Argument description
 * This struct is used to describe one argument that can be passed to a cisTEM program.
*/
struct ArgumentDescription {
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

/** 
 * This template is used to generate an ArgumentClass for every program. The template is passed an enum
 * that defines the names and number of arguments and an array of ArgumentDescription structs that describe the arguments.
*/

template <int num_arguments>
class Arguments {
  public:
    Arguments(ArgumentDescription (&argument_descriptions)[num_arguments], std::string program_name, float version) {
        descriptions = argument_descriptions;
        // Iterate over ArgumentDescription and set default values
        for ( int i = 0; i < num_arguments; i++ ) {
            values[i] = descriptions[i].default_value;
        }

        program_name_ = program_name;
        version_      = version;
    }

    template <typename T>
    T get(int x) {
        return std::any_cast<T>(values[x]);
    }

    template <typename T>
    void set(int x, T value) {
        values[x] = value;
    }

    void generate_argument_string(char* arg_string) {
        for ( int i = 0; i < num_arguments; i++ ) {
            arg_string[i] = descriptions[i].type_char( );
        }
    }

    void GetInteractiveInputs( ) {
        UserInput* my_input = new UserInput(program_name_.c_str( ), version_);
        for ( int i = 0; i < num_arguments; i++ ) {
            if ( descriptions[i].type == FilenameArgument ) {
                values[i] = my_input->GetFilenameFromUser(descriptions[i].description.c_str( ),
                                                          descriptions[i].description.c_str( ),
                                                          std::any_cast<std::string>(descriptions[i].default_value).c_str( ),
                                                          true);
            }
            else if ( descriptions[i].type == BooleanArgument ) {
                std::stringstream converter;
                converter << std::boolalpha << std::any_cast<bool>(descriptions[i].default_value);
                values[i] = my_input->GetYesNoFromUser(descriptions[i].description.c_str( ),
                                                       descriptions[i].description.c_str( ),
                                                       converter.str( ).c_str( ));
            }
            else if ( descriptions[i].type == IntegerArgument ) {
                std::stringstream converter;
                converter << std::any_cast<int>(descriptions[i].default_value);
                values[i] = my_input->GetIntFromUser(descriptions[i].description.c_str( ),
                                                     descriptions[i].description.c_str( ),
                                                     converter.str( ).c_str( ));
            }
        }
        delete my_input;
    }

    void get_from_current_job(RunJob my_current_job) {
    }

    void set_manual_arguments(RunJob my_current_job) {
        if ( num_arguments == 1 )
            my_current_job.arguments = new RunArgument;
        else
            my_current_job.arguments = new RunArgument[num_arguments];

        for ( int i = 0; i < num_arguments; i++ ) {
            if ( descriptions[i].type == FilenameArgument || descriptions[i].type == FilenameArgument ) {
                {
                    arguments[counter].SetStringArgument(std::any_cast<std::string>(values[i]).c_str( ););
                }
                else if ( *format == 'f' ) // float
                {
                    arguments[counter].SetFloatArgument(va_arg(args, double));
                }
                else if ( *format == 'i' ) // integer
                {
                    arguments[counter].SetIntArgument(va_arg(args, int));
                }
                else if ( *format == 'b' ) // bool
                {
                    arguments[counter].SetBoolArgument(va_arg(args, int));
                }
                else {
                    MyPrintWithDetails("Error: Unknown format character!\n");
                }
            }

          private:
            std::array<std::any, num_arguments> values;
            ArgumentDescription*                descriptions;
            std::string                         program_name_;
            float                               version_;
        };
    } // namespace arguments
} // namespace cistem

#endif