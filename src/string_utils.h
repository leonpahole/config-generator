//
// Created by leon on 5. 10. 19.
//

#ifndef CONFIG_GENERATOR_STRING_UTILS_H
#define CONFIG_GENERATOR_STRING_UTILS_H

#include <string>

/*
 * Utilities for working with strings
 */
namespace string_utils {

    /*
     * Trim string from left side
     */
    std::string left_trim(const std::string &str, const std::string &chars = "\t\n\v\f\r ") {

        if (str.empty()) return str;

        unsigned long i;
        for (i = 0; i < str.length(); i++) {

            // keep checking until one character is not a trimming character
            if (chars.find(str[i]) == std::string::npos) break;
        }

        if (i > 0) {
            return str.substr(i);
        }

        return str;
    }

    /*
     * Trim string from right side
     */
    std::string right_trim(const std::string &str, const std::string &chars = "\t\n\v\f\r ") {

        if (str.empty()) return str;

        unsigned long i;
        for (i = str.length() - 1; i >= 0; i--) {

            // keep checking until one character is not a trimming character
            if (chars.find(str[i]) == std::string::npos) break;
        }

        if (i < str.length() - 1) {
            return str.substr(0, i);
        }

        return str;
    }

    /*
     * Trim string from both sides
     */
    std::string trim(const std::string &str, const std::string &chars = "\t\n\v\f\r ") {
        std::string right_trimmed = right_trim(str, chars);
        return left_trim(right_trimmed, chars);
    }

    /*
     * Searches string for a value.
     * If it finds a character not exactly for exact_count amount, returns false
     * Otherwise, returns true
     * char_count_max_exact is used to return the information about
     */
    int count_char(const std::string &str, char search_char) {

        int char_count = 0;

        for (char str_char : str) {

            if (str_char == search_char) {
                char_count++;
            }
        }

        return char_count;
    }

    /*
     * Replace str from search with replace (only first occurence)
     */
    std::string replace(std::string &str, const std::string &search, const std::string &replace) {

        size_t start_pos = str.find(search);
        if (start_pos == std::string::npos)
            return str;

        std::string replaced_string = str;
        replaced_string.replace(start_pos, search.length(), replace);
        return replaced_string;
    }

    /*
     * Compare two strings without looking at case of characters.
     */
    bool compare_case_insensitive(const std::string &str1, const std::string &str2) {
        return std::equal(str1.begin(), str1.end(), str2.begin(),
                          [](const char &a, const char &b) {
                              return (std::tolower(a) == std::tolower(b));
                          });
    }
}

#endif //CONFIG_GENERATOR_STRING_UTILS_H
