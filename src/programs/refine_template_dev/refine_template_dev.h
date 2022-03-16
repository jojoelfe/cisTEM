#include "./arguments.h"

namespace cistem {
namespace refine_template_arguments {
using namespace cistem::arguments;

enum {
    input_starfile = 0,
    max_threads,
    whiten_image,
    arguments_length // Must stay on the bottom
};

arguments::cisTEMArgumentDescription RefineTemplateArgumentsDescription[arguments_length] = {
        {FilenameArgument,
         "Input starfile",
         std::string("input.star")},
        {IntegerArgument,
         "Maximum number of threads to use",
         1},
        {BooleanArgument,
         "Should I whiten the image?",
         false}};

class RefineTemplateArguments {
    std::array<std::any, arguments_length> values;

  public:
    RefineTemplateArguments( ) {
        // Iterate over ArgumentDescription and set default values
        for ( int i = 0; i < arguments_length; i++ ) {
            values[i] = RefineTemplateArgumentsDescription[i].default_value;
        }
    }

    template <typename T>
    T get(int x) {
        return std::any_cast<T>(values[x]);
    }

    template <typename T>
    void set(int x, T value) {
        values[x] = value;
    }

    void get_from_current_job(RunJob my_current_job) {
    }

    void set_manual_arguments(RunJob my_current_job) {
    }

    void GetInteractiveInputs( ) {
        UserInput* my_input = new UserInput("RefineTemplateDev", 1.00);
        for ( int i = 0; i < arguments_length; i++ ) {
            if ( RefineTemplateArgumentsDescription[i].type == FilenameArgument ) {
                values[i] = my_input->GetFilenameFromUser(RefineTemplateArgumentsDescription[i].description.c_str( ),
                                                          RefineTemplateArgumentsDescription[i].description.c_str( ),
                                                          std::any_cast<std::string>(RefineTemplateArgumentsDescription[i].default_value).c_str( ),
                                                          true);
            }
            else if ( RefineTemplateArgumentsDescription[i].type == BooleanArgument ) {
                std::stringstream converter;
                converter << std::boolalpha << std::any_cast<bool>(RefineTemplateArgumentsDescription[i].default_value);
                values[i] = my_input->GetYesNoFromUser(RefineTemplateArgumentsDescription[i].description.c_str( ),
                                                       RefineTemplateArgumentsDescription[i].description.c_str( ),
                                                       converter.str( ).c_str( ));
            }
            else if ( RefineTemplateArgumentsDescription[i].type == IntegerArgument ) {
                std::stringstream converter;
                converter << std::any_cast<int>(RefineTemplateArgumentsDescription[i].default_value);
                values[i] = my_input->GetIntFromUser(RefineTemplateArgumentsDescription[i].description.c_str( ),
                                                     RefineTemplateArgumentsDescription[i].description.c_str( ),
                                                     converter.str( ).c_str( ));
            }
        }
        delete my_input;
    }
};

} // namespace refine_template_arguments
} // namespace cistem
