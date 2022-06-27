#include "encoder.hpp"

int main(int argc, char **argv){
    string inputfile = argv[1];
    string outputfile = argv[2];

    Pbf pbf(inputfile);
    WarnersEncoder encoder;

    encoder.encodePbf(pbf);
    encoder.printCnf(outputfile);

    return 0;
}