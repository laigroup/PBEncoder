/* pb formula */

/* inclusions *****************************************************************/

#include "pbformula.hpp"

/* constants ******************************************************************/

const string &WEIGHT_WORD = "w";

/* classes ********************************************************************/

/* class Label ****************************************************************/

void PbLabel::addNumber(Int i) {
    push_back(i);
    std::sort(begin(), end(), std::greater<Int>());
}

/* class Pbf ******************************************************************/

void Pbf::updateApparentVars(Int literal) {
    Int var = util::getPbfVar(literal);
    apparentVarCount = var > apparentVarCount ? var : apparentVarCount;
    if (!util::isFound(var, apparentVars))
        apparentVars.push_back(var);
}

void Pbf::addConstraint(const vector<Int>& variable, const vector<Int>& coefficient, const Int& limit) {
    variables.push_back(variable);
    coefficients.push_back(coefficient);
    limits.push_back(limit);

    for (Int literal : variable) {
        updateApparentVars(literal);
    }
}

Int Pbf::getApparentVarCount() const {
    return apparentVarCount;
}

/* same as Cnf class */

Int Pbf::getDeclaredVarCount() const {
    return declaredVarCount;
}

Map<Int, Float> Pbf::getLiteralWeights() const {
    return literalWeights; 
}

PBWeightFormat Pbf::getWeightFormat() const {
    return weightFormat;
}

Int Pbf::getEmptyConstraintIndex() const {
    for (Int clauseIndex = 0; clauseIndex < variables.size(); clauseIndex++) {
        if (variables.at(clauseIndex).empty()) {
            return clauseIndex;
        }
    }
    return DUMMY_MIN_INT;
}

const vector<vector<Int>>& Pbf::getVariables() const {
    return variables;
}

const vector<vector<Int>>& Pbf::getCoefficients() const {
    return coefficients;
}

const vector<Int>& Pbf::getLimits() const {
    return limits;
}

const vector<Int>& Pbf::getApparentVars() const {
    return apparentVars;
}

void Pbf::printConstraints() const {
    util::printPbf(variables, coefficients, limits);
}

Pbf::Pbf(const string& filePath, PBWeightFormat weightFormat) {
    util::printComment("Reading PBF formula...", 1);

    std::ifstream inputFileStream(filePath);  // variable will be destroyed if it goes out of scope
    std::istream* inputStream;
    if (filePath == STDIN_CONVENTION) {
        inputStream = &std::cin;

        util::printThickLine();
        util::printComment("Getting cnf from stdin... (end input with 'Enter' then 'Ctrl d')");
    } else {
        if (!inputFileStream.is_open()) {
            util::showError("unable to open file '" + filePath + "'");
        }
        inputStream = &inputFileStream;
    }
    Int declaredConstraintCount = DUMMY_MIN_INT;
    Int processedConstraintCount = 0;
    this->weightFormat = weightFormat;

    Int lineIndex = 0;

    string line;
    while (std::getline(*inputStream, line)) {
        lineIndex++;
        std::istringstream inputStringStream(line);

        if (verbosityLevel >= 4) util::printComment("Line " + to_string(lineIndex) + "\t" + line);

        vector<string> words;
        std::copy(std::istream_iterator<string>(inputStringStream), std::istream_iterator<string>(), std::back_inserter(words));

        Int wordCount = words.size();
        if (wordCount < 1) continue;

        bool endLineFlag = false;
        const string& startWord = words.at(0);

        if (startWord == COMMENT_WORD || startWord.at(0) == '*') {  // "*"
            if(wordCount < 5) continue;
            const string &declareV = words.at(1);
            if (declareV == COMMENT_VARIABLE_WORD) declaredVarCount = std::stoll(words.at(2));
            const string &declareC = words.at(3);
            if (declareC == COMMENT_CONSTRAINT_WORD) declaredConstraintCount = std::stoll(words.at(4));
        } else if(startWord == WEIGHT_WORD) { // weight line
            if(weightFormat == PBWeightFormat::UNWEIGHTED) util::showError("Wrong weighted option");
            if(wordCount == 3) {
                string var = words.at(1);                 // now i = i+1
                if (var.at(0) != VARIABLE_WORD) util::showError("Wrong Variable format");
                Int literal = std::stoll(var.substr(1));
                Float weight = std::stod(words.at(2));
                literalWeights[literal] = weight;
            } else {
                util::showWarning("Wrong weight format");
            }
        } else {  // clause line
            vector<Int> clause;
            vector<Int> coefficient;
            Int limit;
            for (Int i = 0; i < wordCount; i++) {
                if(endLineFlag && i != wordCount - 1) util::showError("External words after relation limit " + words.at(i));

                const string &nowWord = words.at(i);
                if(nowWord == EQUAL_WORD) {                     // == need format & inverse
                    limit = std::stoll(words.at(++i));          // now i = i+1
                    util::formatConstraint(clause, coefficient, limit);
                    addConstraint(clause, coefficient, limit);
                    util::inverseConstraint(clause, coefficient, limit);
                    addConstraint(clause, coefficient, limit);
                    endLineFlag = true;
                } else if(nowWord == GEQUAL_WORD) {             // >= need inverse
                    limit = std::stoll(words.at(++i));          // now i = i+1
                    util::inverseConstraint(clause, coefficient, limit);
                    addConstraint(clause, coefficient, limit);
                    endLineFlag = true;
                } else if(nowWord == LEQUAL_WORD) {             // <= need format
                    limit = std::stoll(words.at(++i));          // now i = i+1
                    util::formatConstraint(clause, coefficient, limit);
                    addConstraint(clause, coefficient, limit);
                    endLineFlag = true;
                } else if(nowWord == END_LINE_WORD){
                    if(!endLineFlag) util::showError("end line without completion constraint");
                    processedConstraintCount++;
                } else {                                        // coefficient & variable
                    Int coef = std::stoll(words.at(i));
                    string var = words.at(++i);                 // now i = i+1
                    if (var.at(0) != VARIABLE_WORD) util::showError("Wrong Variable format");
                    Int literal = std::stoll(var.substr(1));
                    // cout << "Var " << var << " substr " << var.substr(1) << " literal " << literal << std::endl;
                    if (literal > declaredVarCount || literal < -declaredVarCount) {
                        util::showError("literal '" + to_string(literal) + "' is inconsistent with declared var count '" + to_string(declaredVarCount) + "' -- line " + to_string(lineIndex));
                    }
                    clause.push_back(literal);
                    coefficient.push_back(coef);
                }
            }
        }
    }

    if (weightFormat == PBWeightFormat::UNWEIGHTED) { // populates literalWeights with 1s
        for (Int var = 1; var <= declaredVarCount; var++) {
            literalWeights[var] = 1;
            literalWeights[-var] = 1;
        }
    }

    if (filePath == STDIN_CONVENTION) {
        util::printComment("Getting cnf from stdin: done");
        util::printThickLine();
    }

    if (verbosityLevel >= 1) {
        util::printRow("declaredVarCount", declaredVarCount);
        util::printRow("apparentVarCount", apparentVars.size());
        util::printRow("declaredClauseCount", declaredConstraintCount);
        util::printRow("apparentClauseCount", processedConstraintCount);
    }

    if (verbosityLevel >= 3) {
        printConstraints();
    }
}

Pbf::Pbf(const vector<vector<Int>>& variables, const vector<vector<Int>> &coefficients, const vector<Int> &limits) {
    this->variables = variables;
    this->coefficients = coefficients;
    this->limits = limits;

    for (const vector<Int>& clause : variables) {
        for (Int literal : clause) {
            updateApparentVars(literal);
        }
    }
}
