//
// Created by leon on 5. 10. 19.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include "config_generator.h"
#include "string_utils.h"
#include "parsing_utils.h"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

config_generator::config_generator(generator_parameters &parameters) : parameters(&parameters) {}

/*
 * Read one specific environment file into the class dictionary.
 * Overrides overlapping variables in previous environment files.
 * Skips file if env file doesn't exist.
 */
void config_generator::read_env_file(const std::string &file_path) {

    std::ifstream env_file(file_path);

    if (!env_file.good()) {
        std::cerr << "[WARN] Environment file " << file_path << " doesn't exist, skipping." << std::endl;
        return;
    }

    std::string line;
    int line_count = 1;
    while (std::getline(env_file, line)) {

        std::string trimmed_line = string_utils::trim(line);

        // ignore empty lines
        if (trimmed_line.length() == 0) continue;

        std::pair<std::string, std::string> name_value_pair;

        try {
            name_value_pair = parsing_utils::get_name_value_pair(trimmed_line, '=');
        }
        catch (std::runtime_error &error) {

            // print error, but continue
            std::cerr << "[ERROR] File: " << file_path << ", line: " << line_count << ": " << error.what() << std::endl;
            continue;
        }

        // check if value already exists and display override warning if it does
        if (this->env_var_dictionary.find(name_value_pair.first) != this->env_var_dictionary.end()) {
            std::cout << "[WARN] File: " << file_path << ", line: " << line_count << ": overriding value of variable '"
                      << name_value_pair.first << "' from " << this->env_var_dictionary[name_value_pair.first] << " to "
                      << name_value_pair.second << std::endl;
        }

        this->env_var_dictionary[name_value_pair.first] = name_value_pair.second;

        line_count++;
    }

    env_file.close();
}

/*
 * Iterate env files list and read them into env_var_dictionary.
 */
void config_generator::read_env_files() {

    for (const auto &environment_file : this->parameters->environment_files) {
        this->read_env_file(environment_file);
    }
}

/*
 * Read one specific template file and perform configuration generation.
 * At the end, write it to output and to stdout, if specified.
 */
void config_generator::generate_file(const std::string &file_path, const std::string &out_file_path) {

    std::ostringstream generated_file_stream;

    std::ifstream template_file(file_path);

    if (!template_file.good()) {
        std::cout << "[WARN] Template file" << file_path << "doesn't exist, skipping." << std::endl;
        return;
    }

    std::string line;
    int line_count = 1;

    /*
     * To support nested if statements, stack is introduced.
     * When we enter if statement, the evaluation is pushed on stack.
     * If we enter another if statement, the evaluation is again pushed on stack.
     * When we reach endif, value is popped and next evaluation is taken for previous if statement (if it exists)
     */
    std::stack<bool> if_statement_evaluations_stack;

    while (std::getline(template_file, line)) {

        std::string trimmed_line = string_utils::trim(line);

        // ignore empty lines
        // todo: keep whitespace in empty lines?
        if (trimmed_line.length() == 0) {
            generated_file_stream << std::endl;
            continue;
        }

        try {

            // first, substitute everything in the file
            // this will also substitute variables in if statements
            // this line is not trimmed, so that indents are kept
            std::string substituted_line = parsing_utils::substitute_vars(line, this->parameters->definer,
                                                                          this->env_var_dictionary);

            // trim this line now, to use it for further analysis
            std::string substituted_line_trimmed = string_utils::trim(substituted_line);

            if (parsing_utils::is_line_if_statement(substituted_line_trimmed, this->parameters->definer)) {
                // is line if?

                // evaluate it
                bool if_statement_evaluated = parsing_utils::evaluate_if_statement_line(substituted_line_trimmed,
                                                                                        this->parameters->definer,
                                                                                        this->parameters->is_case_sensitive);

                // put it on stack
                if_statement_evaluations_stack.push(if_statement_evaluated);

            } else if (parsing_utils::is_line_endif_statement(substituted_line_trimmed, this->parameters->definer)) {
                // is line endif?

                // if no other if statements have been introduced prior, this is an error
                if (if_statement_evaluations_stack.empty()) {

                    throw std::runtime_error("No endif expected here.");
                }

                // take away the current if value, because the block has ended here
                if_statement_evaluations_stack.pop();

            } else {
                // is normal line?

                // check if there is currently an if statement active
                if (!if_statement_evaluations_stack.empty()) {

                    // check if currently activated if statement is evaluated as true
                    if (if_statement_evaluations_stack.top()) {

                        // if yes, add it to file. Otherwise don't.
                        generated_file_stream << substituted_line << std::endl;
                    }
                } else {

                    // no if statement, add normally to the file
                    generated_file_stream << substituted_line << std::endl;
                }
            }

            line_count++;
        }
        catch (std::runtime_error &error) {
            std::ostringstream error_stream;
            error_stream << "[ERROR] File: " << file_path << ", line: " << line_count
                         << ": " << error.what();
            throw std::runtime_error(error_stream.str());
        }
    }

    template_file.close();

    if (!if_statement_evaluations_stack.empty()) {
        throw std::runtime_error("Expected endif (check if every if statement has a corresponding endif)");
    }

    std::string generated_file = generated_file_stream.str();

    if (!out_file_path.empty()) {
        std::ofstream output_file(out_file_path);
        output_file << generated_file;
        std::cout << "Wrote: " << out_file_path << std::endl;
        output_file.close();
    }

    if (this->parameters->output_to_stdout) {

        std::cout << "<<< " << out_file_path << " >>>" << std::endl
                  << generated_file << std::endl << "<<< " << out_file_path << " end >>>" << std::endl << std::endl;
    }
}

/*
 * Iterate template files list and generate configurations from them.
 */
void config_generator::generate_files() {

    for (unsigned long i = 0; i < this->parameters->template_files.size(); i++) {

        try {
            this->generate_file(this->parameters->template_files[i],
                                this->parameters->output_files.size() > i ? this->parameters->output_files[i] : "");
        }
        catch (std::runtime_error &error) {
            std::cerr << error.what() << std::endl;
        }
    }
}

void config_generator::generate_directory(const std::string &name, int indent, const std::string &base_name) {

    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name.c_str())))
        return;

    while ((entry = readdir(dir)) != NULL) {

        // if is directory
        if (entry->d_type == DT_DIR) {

            char path[1024];
            char base_path[1024];

            // ignore . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            // get name
            snprintf(path, sizeof(path), "%s/%s", name.c_str(), entry->d_name);
            snprintf(base_path, sizeof(base_path), "%s/%s", base_name.c_str(), entry->d_name);

            mkdir(base_path, 0777);

            this->generate_directory(path, indent + 2, base_path);
        } else {

            char path[1024];
            char base_path[1024];

            snprintf(path, sizeof(path), "%s/%s", name.c_str(), entry->d_name);
            snprintf(base_path, sizeof(base_path), "%s/%s", base_name.c_str(), entry->d_name);

            this->generate_file(path, base_path);
        }
    }

    closedir(dir);
}

void config_generator::generate_directories() {

    try {
        mkdir(this->parameters->output_directory.c_str(), 0777);
        this->generate_directory(this->parameters->template_directory, 0, this->parameters->output_directory);
    }
    catch (std::runtime_error &error) {
        std::cerr << error.what() << std::endl;
    }
}

void config_generator::run() {
    this->read_env_files();

    if (this->parameters->uses_directory) {

        this->generate_directories();

    } else {

        this->generate_files();
    }
}