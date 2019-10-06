//
// Created by leon on 5. 10. 19.
//

#ifndef CONFIG_GENERATOR_GENERATOR_PARAMETERS_H
#define CONFIG_GENERATOR_GENERATOR_PARAMETERS_H

#include <vector>
#include <string>

class generator_parameters {
private:
    std::vector<std::string> environment_files;
    std::vector<std::string> template_files;
    std::string template_directory;
    std::vector<std::string> output_files;

    std::string output_directory;
    bool uses_directory = false;

    bool output_to_stdout = false;

    std::string definer = "%";
    bool is_case_sensitive = false;

    bool display_help = true;

    friend class config_generator;

    void get_params(int argc, char **argv);

    void set_argument(const std::string &argument_name, const std::string &argument_value);

    static void print_help();

    void validate_params();

public:
    generator_parameters();

    bool configure(int argc, char **argv);
};


#endif //CONFIG_GENERATOR_GENERATOR_PARAMETERS_H
