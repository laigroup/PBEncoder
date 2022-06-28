#include "main.hpp"

void OptionDict::printHelp() const {
    cout << "Usage: \n\tEncoder [OPTION...]\n";
    printOptions(); 
}

void OptionDict::printOptions() const {
	cout << "\t  -h, --help      	help information\t\tOptional\n";
	cout << "\t  -" << INPUT_OPTION << " arg  \t\targ: input file path \t\tRequired\n";
	cout << "\t  -" << OUTPUT_OPTION << " arg  \t\targ: output file path \t\tRequired\n";
}

void OptionDict::printWelcome() const {
    bool commented = !helpFlag;

    util::printThickLine(commented);

    util::printComment("Encoder: Encoding Pseudo-Boolean constraints to CNF with counting safe (help: 'Encoder -h')", 0, 1, commented);

    const string &version = "1.0";
    const string &date = "2022/06/28";
    util::printComment("Version " + version + ", released on " + date, 0, 1, commented);

    util::printThickLine(commented);
}

OptionDict::OptionDict(int argc, char *argv[]) {
    options = new cxxopts::Options("Encoder", "");

    options->add_options(REQUIRED_OPTION_GROUP)
        (INPUT_OPTION, "", cxxopts::value<string>()->default_value(""))
        (OUTPUT_OPTION, "", cxxopts::value<string>()->default_value(""))
        ;

    options->add_options(OPTIONAL_OPTION_GROUP)
        (HELP_OPTION, "help")
        ;

    cxxopts::ParseResult result = options->parse(argc, argv);

    helpFlag = result["h"].as<bool>();

    printWelcome();

    input_file = result[INPUT_OPTION].as<string>();
    output_file = result[OUTPUT_OPTION].as<string>();
}

int main(int argc, char **argv){

    OptionDict optionDict(argc, argv);

    if(optionDict.helpFlag) {
        optionDict.printHelp();
    } else {
        util::printComment("Process ID of this main program:", 1);
        util::printComment("pid " + to_string(getpid()));
        Pbf pbf(optionDict.input_file);
        WarnersEncoder encoder;

        encoder.encodePbf(pbf);
        encoder.printCnf(optionDict.output_file);
    }

    return 0;
}