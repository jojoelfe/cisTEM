#include <arguments.h>

namespace cistem
{
    namespace refine_template_arguments
    {
        using namespace cistem::arguments;
        enum
        {
            save_movies = 0,
            max_threads,
            argument_length // Must stay on the bottom
        };

        arguments::cisTEMArgumentDescription RefineTemplateArgumentsDescription[argument_length] = {
            [save_movies] = {
                arguments::ArgumentType::BOOLEAN,
                "Print this help message",
                false},
            [max_threads] = {arguments::ArgumentType::INTEGER, "Maximum number of threads to use", 1}};
        class RefineTemplateArguments
        {
            std::array<std::any, argument_length> values;

        public:
            RefineTemplateArguments()
            {
                // Iterate over ArgumentDescription and set default values
                for (int i = 0; i < argument_length; i++)
                {
                    values[i] = RefineTemplateArgumentsDescription[i].default_value;
                }
            }

            template <typename T>
            T get(int x)
            {
                return std::any_cast<T>(values[x]);
            }

            void GetInteractiveInputs()
            {
                for (int i = 0; i < argument_length; i++)
                {
                    if (RefineTemplateArgumentsDescription[i].type == ArgumentType::BOOLEAN)
                    {
                        std::cout << RefineTemplateArgumentsDescription[i].description << " [y/n] ";
                        std::string input;
                        std::cin >> input;
                        if (input == "y" || input == "Y")
                        {
                            values[i] = true;
                        }
                        else
                        {
                            values[i] = false;
                        }
                    }
                    else if (RefineTemplateArgumentsDescription[i].type == ArgumentType::INTEGER)
                    {
                        std::cout << RefineTemplateArgumentsDescription[i].description << " [integer] ";
                        std::string input;
                        std::cin >> input;
                        values[i] = std::stoi(input);
                    }
                    else if (RefineTemplateArgumentsDescription[i].type == ArgumentType::FLOAT)
                    {
                        std::cout << RefineTemplateArgumentsDescription[i].description << " [float] ";
                        std::string input;
                        std::cin >> input;
                        values[i] = std::stof(input);
                    }
                    else if (RefineTemplateArgumentsDescription[i].type == ArgumentType::DOUBLE)
                    {
                        std::cout << RefineTemplateArgumentsDescription[i].description << " [double] ";
                        std::string input;
                        std::cin >> input;
                        values[i] = std::stod(input);
                    }
                    else if (RefineTemplateArgumentsDescription[i].type == ArgumentType::STRING)
                    {
                        std::cout << RefineTemplateArgumentsDescription[i].description << " [string] ";
                        std::string input;
                        std::cin >> input;
                        values[i] = input;
                    }
                }
            }
        };

    }
}
