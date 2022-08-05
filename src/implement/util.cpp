/* inclusions *****************************************************************/

#include "util.hpp"

/* global variables ***********************************************************/

Int randomSeed = DEFAULT_RANDOM_SEED;
TimePoint startTime;

const bool DEBUG = true;
/* constants ******************************************************************/

// const string &COMMENT_WORD = "c";    // cnf comment word
const string& COMMENT_WORD = "*";  // pbf comment word
const string& PROBLEM_WORD = "p";
const char& VARIABLE_WORD = 'x';

const string& COMMENT_VARIABLE_WORD = "#variable=";
const string& COMMENT_CONSTRAINT_WORD = "#constraint=";

const string& EQUAL_WORD = "=";
const string& GEQUAL_WORD = ">=";
const string& LEQUAL_WORD = "<=";

const string& END_LINE_WORD = ";";

const string& STDIN_CONVENTION = "-";

const string& REQUIRED_OPTION_GROUP = "Required";
const string& OPTIONAL_OPTION_GROUP = "Optional";

const string& HELP_OPTION = "h, help";
const string& INPUT_OPTION = "I";
const string& OUTPUT_OPTION = "O";
const string& WEIGHT_FORMAT_OPTION = "wf";
const string& ENCODER_OPTION = "ed";
const string& OUTPUT_FORMAT_OPTION = "of";

const std::map<Int, PBWeightFormat> PBWEIGHT_FORMAT_CHOICES = {
    {1, PBWeightFormat::UNWEIGHTED},
    {2, PBWeightFormat::WEIGHTED}};
const Int DEFAULT_PBWEIGHT_FORMAT_CHOICE = 1;

const std::map<Int, OutputFormat> OUTPUT_FORMAT_CHOICES = {
    {1, OutputFormat::MC20},
    {2, OutputFormat::MC21}};
const Int DEFAULT_OUTPUT_FORMAT_CHOICE = 1;

const std::map<Int, EncoderType> ENCODER_CHOICES = {
    {1, EncoderType::Warners},
    {2, EncoderType::GenArc}};
const Int DEFAULT_ENCODER_CHOICE = 1;

const Int DEFAULT_RANDOM_SEED = 10;

const Float NEGATIVE_INFINITY = -std::numeric_limits<Float>::infinity();

const Int DUMMY_MIN_INT = std::numeric_limits<Int>::min();
const Int DUMMY_MAX_INT = std::numeric_limits<Int>::max();

const string& DUMMY_STR = "";

/* namespaces *****************************************************************/

/* namespace util *************************************************************/

bool util::isInt(Float d) {
    Float intPart;
    Float fractionalPart = modf(d, &intPart);
    return fractionalPart == 0.0;
}

/* functions: printing ********************************************************/

void util::printComment(const string& message, Int preceedingNewLines, Int followingNewLines, bool commented) {
    for (Int i = 0; i < preceedingNewLines; i++)
        cout << "\n";
    cout << (commented ? COMMENT_WORD + " " : "") << message;
    for (Int i = 0; i < followingNewLines; i++)
        cout << "\n";
}

void util::printSolutionLine(Float modelCount, Int preceedingThinLines, Int followingThinLines) {
    for (Int i = 0; i < preceedingThinLines; i++)
        printThinLine();
    cout << "s "
         << " " << modelCount << "\n";
    for (Int i = 0; i < followingThinLines; i++)
        printThinLine();
}

void util::printBoldLine(bool commented) {
    printComment("******************************************************************", 0, 1, commented);
}

void util::printThickLine(bool commented) {
    printComment("==================================================================", 0, 1, commented);
}

void util::printThinLine() {
    printComment("------------------------------------------------------------------");
}

void util::printHelpOption() {
    cout << "  -h, --hi      help information\n";
}

/* functions: argument parsing ************************************************/

vector<string> util::getArgV(int argc, char* argv[]) {
    vector<string> argV;
    for (Int i = 0; i < argc; i++)
        argV.push_back(string(argv[i]));
    return argV;
}

/* functions: CNF *************************************************************/

Int util::getCnfVar(Int literal) {
    if (literal == 0) {
        showError("literal is 0");
    }
    return std::abs(literal);
}

Set<Int> util::getClauseCnfVars(const vector<Int>& clause) {
    Set<Int> cnfVars;
    for (Int literal : clause)
        cnfVars.insert(getCnfVar(literal));
    return cnfVars;
}

Set<Int> util::getClusterCnfVars(const vector<Int>& cluster, const vector<vector<Int>>& clauses) {
    Set<Int> cnfVars;
    for (Int clauseIndex : cluster)
        unionize(cnfVars, getClauseCnfVars(clauses.at(clauseIndex)));
    return cnfVars;
}

bool util::appearsIn(Int cnfVar, const vector<Int>& clause) {
    for (Int literal : clause)
        if (getCnfVar(literal) == cnfVar)
            return true;
    return false;
}

bool util::isPositiveLiteral(Int literal) {
    if (literal == 0)
        showError("literal is 0");
    return literal > 0;
}

Int util::getLiteralRank(Int literal, const vector<Int>& cnfVarOrdering) {
    Int cnfVar = getCnfVar(literal);
    auto it = std::find(cnfVarOrdering.begin(), cnfVarOrdering.end(), cnfVar);
    if (it == cnfVarOrdering.end())
        showError("cnfVar not found in cnfVarOrdering");
    return it - cnfVarOrdering.begin();
}

Int util::getMinClauseRank(const vector<Int>& clause, const vector<Int>& cnfVarOrdering) {
    Int minRank = DUMMY_MAX_INT;
    for (Int literal : clause) {
        Int rank = getLiteralRank(literal, cnfVarOrdering);
        if (rank < minRank)
            minRank = rank;
    }
    return minRank;
}

Int util::getMaxClauseRank(const vector<Int>& clause, const vector<Int>& cnfVarOrdering) {
    Int maxRank = DUMMY_MIN_INT;
    for (Int literal : clause) {
        Int rank = getLiteralRank(literal, cnfVarOrdering);
        if (rank > maxRank)
            maxRank = rank;
    }
    return maxRank;
}

void util::printClause(const vector<Int>& clause) {
    for (Int literal : clause) {
        cout << std::right << std::setw(5) << literal << " ";
    }
    cout << "\n";
}

void util::printCnf(const vector<vector<Int>>& clauses) {
    printThinLine();
    printComment("cnf {");
    for (Int i = 0; i < clauses.size(); i++) {
        cout << COMMENT_WORD << "\t"
                                "clause ";
        cout << std::right << std::setw(5) << i + 1 << " : ";
        printClause(clauses.at(i));
    }
    printComment("}");
    printThinLine();
}

void util::printLiteralWeights(const Map<Int, Float>& literalWeights) {
    Int maxCnfVar = DUMMY_MIN_INT;
    for (const std::pair<Int, Float>& kv : literalWeights) {
        Int cnfVar = kv.first;
        if (cnfVar > maxCnfVar) {
            maxCnfVar = cnfVar;
        }
    }

    printThinLine();
    printComment("literalWeights {");
    cout << std::right;
    for (Int cnfVar = 1; cnfVar <= maxCnfVar; cnfVar++) {
        cout << COMMENT_WORD << " " << std::right << std::setw(10) << cnfVar << "\t" << std::left << std::setw(15) << literalWeights.at(cnfVar) << "\n";
        cout << COMMENT_WORD << " " << std::right << std::setw(10) << -cnfVar << "\t" << std::left << std::setw(15) << literalWeights.at(-cnfVar) << "\n";
    }
    printComment("}");
    printThinLine();
}

/* functions: PBF *************************************************************/
Int util::getPbfVar(Int literal) {
    if (literal == 0) {
        showError("literal is 0");
    }
    return std::abs(literal);
}

// now relation word is <= but may have negative coefficient
void util::formatConstraint(vector<Int>& clause, vector<Int>& coefficient, Int& limit) {
    for (int i = 0; i < coefficient.size(); i++) {
        const Int& coef = coefficient.at(i);
        if (coef > 0)
            continue;
        else if (coef < 0) {
            coefficient.at(i) = -coefficient.at(i);
            clause.at(i) = -clause.at(i);
            limit += coefficient.at(i);  // + positive number
        } else
            showError("Coefficient is 0");
    }
    if (limit < 0)
        showError("Formula <= negative limit");
}

// now relation word is >= need to inverse to <=
void util::inverseConstraint(vector<Int>& clause, vector<Int>& coefficient, Int& limit) {
    limit = -limit;
    for (int i = 0; i < coefficient.size(); i++) {
        const Int& coef = coefficient.at(i);
        if (coef > 0) {  // -coef < 0;
            clause.at(i) = -clause.at(i);
            limit += coefficient.at(i);
        } else if (coef < 0) {  // -coef > 0
            coefficient.at(i) = -coefficient.at(i);
        } else {
            showError("Coefficient is 0");
        }
    }
    if (limit < 0)
        showError("Formula <= negative limit");
}

void util::printConstraint(const vector<Int>& clause, const vector<Int>& coefficent, const Int& limit) {
    for (int i = 0; i < clause.size(); i++) {
        cout << std::right << std::setw(5) << coefficent.at(i) << " x" << clause.at(i) << " ";
    }
    cout << std::right << std::setw(10) << " <= " << limit;
    cout << "\n";
}

void util::printPbf(const vector<vector<Int>>& clauses, const vector<vector<Int>>& coefficents, const vector<Int>& limits) {
    Int clausesSize = clauses.size();
    Int coefficentsSize = coefficents.size();
    Int limitsSize = limits.size();

    if (clausesSize != coefficentsSize || clausesSize != limitsSize) {
        showError("Unpair Constraints Size");
    }

    printThinLine();
    printComment("pbf {");
    for (Int i = 0; i < clauses.size(); i++) {
        cout << COMMENT_WORD << "\t"
                                "Constraint ";
        cout << std::right << std::setw(5) << i + 1 << " : ";
        printConstraint(clauses.at(i), coefficents.at(i), limits.at(i));
    }
    printComment("}");
    printThinLine();

    std::cout << "Not implement" << std::endl;
}
/* functions: timing **********************************************************/

TimePoint util::getTimePoint() {
    return std::chrono::steady_clock::now();
}

Float util::getSeconds(TimePoint startTime) {
    TimePoint endTime = getTimePoint();
    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
}

void util::printDuration(TimePoint startTime) {
    printThickLine();
    printRow("seconds", getSeconds(startTime));
    printThickLine();
}

/* functions: error handling **************************************************/

void util::showWarning(const string& message, bool commented) {
    printBoldLine(commented);
    printComment("MY_WARNING: " + message, 0, 1, commented);
    printBoldLine(commented);
}

void util::showError(const string& message, bool commented) {
    throw MyError(message, commented);
}

/* classes ********************************************************************/

/* class MyError **************************************************************/

MyError::MyError(const string& message, bool commented) {
    util::printBoldLine(commented);
    util::printComment("MY_ERROR: " + message, 0, 1, commented);
    util::printBoldLine(commented);
}
