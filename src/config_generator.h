//
// Created by leon on 5. 10. 19.
//

#ifndef CONFIG_GENERATOR_CONFIG_GENERATOR_H
#define CONFIG_GENERATOR_CONFIG_GENERATOR_H

#include "generator_parameters.h"
#include <unordered_map>

class config_generator {

private:
    generator_parameters *parameters;
    std::unordered_map<std::string, std::string> env_var_dictionary;

    void read_env_files();

    void read_env_file(const std::string &file_path);

    void generate_directory(const std::string &name, int indent, const std::string &base_name);

    void generate_directories();

    void generate_file(const std::string &file_name, const std::string &out_file_path);

    void generate_files();

public:
    explicit config_generator(generator_parameters &parameters);

    void run();
};


#endif //CONFIG_GENERATOR_CONFIG_GENERATOR_H
