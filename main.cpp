#include "src/generator_parameters.h"
#include "src/config_generator.h"

int main(int argc, char **argv) {

    generator_parameters parameters;

    bool configuration_success = parameters.configure(argc, argv);

    if (!configuration_success) {
        return -1;
    }

    config_generator generator = config_generator(parameters);
    generator.run();

    return 0;
}