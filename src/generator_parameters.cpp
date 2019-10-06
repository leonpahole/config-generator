//
// Created by leon on 5. 10. 19.
//

#include <stdexcept>
#include <sstream>
#include <getopt.h>
#include <iostream>
#include "generator_parameters.h"

const std::string
        PARAM_ENV = "env",
        PARAM_FILE = "file",
        PARAM_DIR = "dir",
        PARAM_OUT = "out",
        PARAM_STDOUT = "stdout",
        PARAM_DEFINER = "definer",
        PARAM_CASE_SENSITIVE = "case-sensitive",
        PARAM_HELP = "help",

        VALUE_TRUE = "true",
        VALUE_FALSE = "false";

/*
 * Constructor
 */
generator_parameters::generator_parameters() = default;

/*
 * Set name value pair based on string name of argument.
 */
void generator_parameters::set_argument(const std::string &argument_name, const std::string &argument_value) {

    if (argument_value.length() == 0) return;

    this->display_help = false;

    if (argument_name == PARAM_ENV) {
        this->environment_files.push_back(argument_value);
    } else if (argument_name == PARAM_FILE) {
        this->template_files.push_back(argument_value);
    } else if (argument_name == PARAM_DIR) {
        this->template_directory = argument_value;
    } else if (argument_name == PARAM_OUT) {
        this->output_files.push_back(argument_value);
    } else if (argument_name == PARAM_STDOUT) {
        this->output_to_stdout = argument_value != VALUE_FALSE;
    } else if (argument_name == PARAM_DEFINER) {
        this->definer = argument_value;
    } else if (argument_name == PARAM_CASE_SENSITIVE) {
        this->is_case_sensitive = argument_value != VALUE_FALSE;
    } else {
        this->display_help = true;
    }
}

/*
 * Go through all parameters and make sure they are valid
 */
void generator_parameters::validate_params() {

    std::ostringstream error_string_stream;

    if (this->environment_files.empty()) {
        error_string_stream << "No environment file specified. Use --" << PARAM_ENV << "." << std::endl;
    }

    if (this->template_directory.length() > 0) {
        this->uses_directory = true;
    }

    if (this->template_files.empty() && !this->uses_directory) {
        error_string_stream << "No input files or directory specified. Use --" << PARAM_DIR << " or --" << PARAM_FILE
                            << "." << std::endl;
    } else if (!this->template_files.empty() && this->uses_directory) {
        error_string_stream
                << "Directory and files cannot be specified at once. Please only specify either --" << PARAM_FILE
                << " or --" << PARAM_DIR << "." << std::endl;
    }

    // either specify outputs or stdout printout
    if (this->output_files.empty() && !this->output_to_stdout) {
        error_string_stream << "No outputs specified. Use --" << PARAM_OUT << " or alternatively --" << PARAM_STDOUT
                            << "." << std::endl;
    }

    // fail if number of outputs donesn't match with number of files, but only if outputs exist (otherwise stdout is used)
    if (!this->uses_directory && !this->output_files.empty()) {

        if (this->template_files.size() != this->output_files.size()) {

            error_string_stream
                    << "Amount of inputs is not the same as amount of outputs. Please specify same amount of --"
                    << PARAM_FILE << " and --" << PARAM_OUT << " flags." << std::endl;
        }
    }

    // set output directory
    if (this->uses_directory) {
        this->output_directory = this->output_files[0];
    }

    if (!error_string_stream.str().empty()) {

        throw std::runtime_error(error_string_stream.str());
    }
}

/*
 * use getopt_long to get long program paraeters and call set_argument to set configuration.
 */
void generator_parameters::get_params(int argc, char **argv) {

    int c;

    static struct option long_options[] = {
            {PARAM_ENV.c_str(),            required_argument, nullptr, 0},
            {PARAM_FILE.c_str(),           required_argument, nullptr, 0},
            {PARAM_DIR.c_str(),            required_argument, nullptr, 0},
            {PARAM_OUT.c_str(),            required_argument, nullptr, 0},
            {PARAM_STDOUT.c_str(),         no_argument,       nullptr, 0},
            {PARAM_DEFINER.c_str(),        required_argument, nullptr, 0},
            {PARAM_CASE_SENSITIVE.c_str(), no_argument,       nullptr, 0},
            {PARAM_HELP.c_str(),           no_argument,       nullptr, 0}
    };

    while (true) {

        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {

            case 0:

                if (long_options[option_index].name == PARAM_HELP) {
                    generator_parameters::print_help();
                    break;
                }

                if (long_options[option_index].has_arg == no_argument) {

                    this->set_argument(long_options[option_index].name, VALUE_TRUE);

                } else {

                    if (optarg) {

                        this->set_argument(long_options[option_index].name, optarg);

                    } else {
                        std::cerr << "[WARN] Ignoring option with empty value --" << long_options[option_index].name
                                  << ", which requires a value.";
                    }
                }

                break;

            case '?':
            default:
                break;
        }
    }
}

/*
 * Print help of the program
 */
void generator_parameters::print_help() {

    std::cout << "Usage: config-generator" << std::endl <<
              "``--env``: path to environment file. You can specify more files by adding multiple ``-env`` flags. "
              << std::endl <<
              "If variables in files overlap, warnings will be issues, but the variable in latter file will take precedence."
              << std::endl <<
              "``--file``: path to configuration file. You can specify more files by adding multiple ``-file`` flags."
              << std::endl <<
              "``--dir``: substitute all files in a directory." << std::endl
              << std::endl <<
              "``--out``: name of output file or directory, depending on whether you used ``--file`` or ``-dir``."
              << std::endl <<
              "If you specified multiple files, you need to specify multiple outputs as well." << std::endl <<
              "The inputs will be mapped to outputs based on the sequence." << std::endl <<
              std::endl <<
              "``--stdout``: instead of writing to file, output the result to stdout." << std::endl <<
              "Can be used with ``-out`` to combine writing to files and priting to stdout." << std::endl << std::flush;
}

/*
 * First, run get_params, then validate them with validate_params and return success.
 */
bool generator_parameters::configure(int argc, char **argv) {

    try {
        this->get_params(argc, argv);

        if (this->display_help) {
            this->print_help();
            return false;
        }

        this->validate_params();
    }
    catch (std::runtime_error &error) {
        std::cerr << error.what() << std::endl;
        return false;
    }

    return true;
}