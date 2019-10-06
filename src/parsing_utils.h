//
// Created by leon on 5. 10. 19.
//

#ifndef CONFIG_GENERATOR_PARSING_UTILS_H
#define CONFIG_GENERATOR_PARSING_UTILS_H

#include <utility>
#include <sstream>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <regex>
#include "string_utils.h"

/*
 * Utilities for parsing env files and conditions
 */
namespace parsing_utils {

    const std::string LOGICAL_AND = "AND", LOGICAL_OR = "OR";
    const std::string CONDITIONAL_IS = "IS", CONDITIONAL_IS_NOT = "IS_NOT";
    const std::string IF_STATEMENT = "IF", ENDIF_STATEMENT = "ENDIF";

    /*
     * Take a string, such as A=3 and return pair <name, value>
     * Throws runtime_error
     */
    std::pair<std::string, std::string> get_name_value_pair(const std::string &env_line, char equal_sign = '=') {

        std::ostringstream error_stream;

        int equals_count = string_utils::count_char(env_line, equal_sign);

        // check for amount of equal signs (=), should be exactly one
        if (equals_count == 0) {
            error_stream << "No '" << equal_sign << "' characters found in non-empty environment line " << env_line
                         << ".";
            throw std::runtime_error(error_stream.str());
        } else if (equals_count > 1) {
            error_stream << "Multiple '" << equal_sign << "' characters (" << equals_count
                         << ") found in non-empty environment line " << env_line << ".";
            throw std::runtime_error(error_stream.str());
        }

        std::string env_name, env_value;

        // find equals sign and split the string into two parts; right and left side of expression
        for (unsigned long i = 0; i < env_line.size(); i++) {
            if (env_line[i] == equal_sign) {
                env_name = string_utils::trim(env_line.substr(0, i));
                env_value = string_utils::trim(env_line.substr(i + 1));
                break;
            }
        }

        // check if either left or right side are empty
        if (env_name.empty()) {
            error_stream << "Empty variable name found in non-empty environment line " << env_line << ".";
            throw std::runtime_error(error_stream.str());
        } else if (env_value.empty()) {
            error_stream << "Empty variable value found in non-empty environment line " << env_line << ".";
            throw std::runtime_error(error_stream.str());
        }

        // return pair with 2 values: name and value
        std::pair<std::string, std::string> name_and_value_pair;
        name_and_value_pair.first = env_name;
        name_and_value_pair.second = env_value;

        return name_and_value_pair;
    }

    /*
     * Checks if a line is an if statement.
     * Line is an if statement if it begins with if identifier, such as %IF.
     * Function expects string to be trimmed.
     * Throws runtime_error if line contains if but it doesn't start with it.
     */
    bool is_line_if_statement(const std::string &line, const std::string &definer) {

        std::string if_statement_identifier = definer + IF_STATEMENT;

        bool line_begins_with_if = line.rfind(if_statement_identifier + " ") == 0;

        if (line_begins_with_if) {
            return true;
        } else if (line.find(if_statement_identifier) != std::string::npos) {
            std::ostringstream error_stream;
            error_stream << "If line '" << line << "' doesn't have " << if_statement_identifier << " at the beginning.";
            throw std::runtime_error(error_stream.str());
        }

        return false;
    }

    /*
     * Checks if a line is an endif statement.
     * Line is an endif statement if it only contains and endif identifier, such as %ENDIF.
     * Function expects string to be trimmed.
     * Throws runtime_error if line contains endif but it isn't the only thing in the line.
     */
    bool is_line_endif_statement(const std::string &line, const std::string &definer) {

        std::string endif_statement_identifier = definer + ENDIF_STATEMENT;

        bool line_is_exactly_endif = line == endif_statement_identifier;

        if (line_is_exactly_endif) {
            return true;
        } else if (line.find(endif_statement_identifier) != std::string::npos) {
            std::ostringstream error_stream;
            error_stream << "Endif line '" << line << "' contains additional text. Endif lines should contain only "
                         << endif_statement_identifier << ".";
            throw std::runtime_error(error_stream.str());
        }

        return false;
    }

    /*
     * Returns val right_side logical_operator left_side
     * Throws runtime_error if invalid logical operator
     */
    bool evaluate_logical_operator(bool left_side, const std::string &logical_operator, bool right_side) {

        if (logical_operator == LOGICAL_AND) {
            return left_side && right_side;
        } else if (logical_operator == LOGICAL_OR) {
            return left_side || right_side;
        }

        std::ostringstream error_stream;
        error_stream << "Unknown logical operator '" << logical_operator << "'.";
        throw std::runtime_error(error_stream.str());
    }

    /*
     * Returns val right_side logical_operator left_side
     * Throws runtime_error if invalid conditional operator
     */
    bool evaluate_conditional_operator(const std::string &left_side, const std::string &conditional_operator,
                                       const std::string &right_side, bool case_sensitive = false) {

        if (conditional_operator == CONDITIONAL_IS) {

            if (case_sensitive) {
                return string_utils::compare_case_insensitive(left_side, right_side);
            } else {
                return left_side == right_side;
            }

        } else if (conditional_operator == CONDITIONAL_IS_NOT) {

            if (case_sensitive) {
                return !string_utils::compare_case_insensitive(left_side, right_side);
            } else {
                return left_side != right_side;
            }
        }

        std::ostringstream error_stream;
        error_stream << "Unknown conditional operator '" << conditional_operator << "'.";
        throw std::runtime_error(error_stream.str());
    }

    /*
     * Evaluate line that is considered an if statement. If line isn't an if statement, return true.
     * Function assumes variables are already replaced and that string is trimmed!
     * Eg. %IF DEVELOPMENT IS DEVELOPMENT will work, but %IF %{MODE} IS DEVELOPMENT wont!
     * Throws runtime_error for syntax errors
     */
    bool evaluate_if_statement_line(const std::string &line, const std::string &definer, bool case_sensitive) {

        // evaluate non if statements as "true"
        if (!is_line_if_statement(line, definer)) {
            return true;
        }

        // split whole line by spaces
        std::vector<std::string> line_split_by_spaces;

        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ' ')) {

            line_split_by_spaces.push_back(token);
        }

        // should not happen
        if (line_split_by_spaces.empty()) {
            return true;
        }

        // subcondition -> one sub condition in whole if statement (separated by logical operators)
        // eg. IF A IS B -> 4 words
        // or  AND B IS C -> 4 words
        const int WORDS_IN_IF_SUBCONDITION = 4;

        if (line_split_by_spaces.size() % WORDS_IN_IF_SUBCONDITION != 0) {
            std::ostringstream error_stream;
            error_stream << "If line '" << line
                         << "' has invalid amount of words (expected 4, 8, 12, ..., but got "
                         << line_split_by_spaces.size() << ".";
            throw std::runtime_error(error_stream.str());
        }

        bool final_if_statement_value = false;

        // iterate vector as 2D array, so i will jump by one, but it actually jumps by WORDS_IN_IF_LINE.
        for (unsigned long i = 0; i < line_split_by_spaces.size() / WORDS_IN_IF_SUBCONDITION; i++) {

            std::string current_logical_operator;

            // if we are in second if substatement, because first one contains IF word
            if (i > 0) {

                current_logical_operator = line_split_by_spaces[i * WORDS_IN_IF_SUBCONDITION];
            }

            // get other values in this if statement by pseudo 2D array indexing
            std::string left_val = line_split_by_spaces[i * WORDS_IN_IF_SUBCONDITION + 1];
            std::string conditional_operator = line_split_by_spaces[i * WORDS_IN_IF_SUBCONDITION + 2];
            std::string right_val = line_split_by_spaces[i * WORDS_IN_IF_SUBCONDITION + 3];

            // evaluate this statement
            bool current_statement_value = evaluate_conditional_operator(left_val, conditional_operator, right_val,
                                                                         case_sensitive);

            if (i == 0) {

                // first line: just assign the result to final value
                final_if_statement_value = current_statement_value;

            } else {

                // subsequent lines: add boolean to accumulated final value
                final_if_statement_value = evaluate_logical_operator(final_if_statement_value,
                                                                     current_logical_operator,
                                                                     current_statement_value);
            }
        }

        return final_if_statement_value;
    }

    /*
     * Substitutes any variables in the line, using the dictionary provided (env_var_dictionary).
     * Uses definer with curly braces to replace variables.
     * For example %{HELLO} will be replaces with whatever HELLO points to in env_var_dictionary.
     * Throws runtime_error if HELLO doesn't exist in env_var_dictionary.
     * If variable is empty (%{}), then throw error as well.
     */
    std::string substitute_vars(std::string line, const std::string &definer,
                                std::unordered_map<std::string, std::string> &env_var_dictionary) {

        std::ostringstream error_stream;

        // define regex for finding variables with given pattern %{...}
        std::regex var_pattern(definer + "\\{(.*?)\\}");

        std::smatch match_result;
        std::string substituted_line = line;

        while (std::regex_search(line, match_result, var_pattern)) {

            // there should be exactly 2 results, but check anyway
            if (match_result.size() >= 2) {

                std::string variable_name = match_result[1].str();

                if (variable_name.empty()) {
                    error_stream << "Empty variable.";
                    throw std::runtime_error(error_stream.str());
                } else if (env_var_dictionary.find(variable_name) == env_var_dictionary.end()) {
                    error_stream << "Undefined variable " << variable_name << ".";
                    throw std::runtime_error(error_stream.str());
                }

                std::string unsubstituted_variable = match_result[0].str();

                // in line that will be returned, replace the original variable (%{...}) with value, found in dictionary
                substituted_line = string_utils::replace(substituted_line, unsubstituted_variable,
                                                         env_var_dictionary[variable_name]);
            }

            // update line
            line = match_result.suffix().str();
        }

        return substituted_line;
    }
}

#endif //CONFIG_GENERATOR_PARSING_UTILS_H
