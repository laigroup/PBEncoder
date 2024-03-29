#include "encoder.hpp"
#include "../../lib/cxxopts.hpp"

class OptionDict {
public:
  /* optional: */
    bool helpFlag;

    string input_file;
    string output_file;
    PBWeightFormat weightFormat;
    OutputFormat outputFormat;
    EncoderType encoderType;

    cxxopts::Options *options;

    void printOptions() const;
    void printHelp() const;
    void printWelcome() const;
    OptionDict(int argc, char *argv[]);
};