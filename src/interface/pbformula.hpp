/* PB Constraint */
#pragma once

/* inclusions *****************************************************************/

#include "util.hpp"

/* constants ******************************************************************/

extern const string &WEIGHT_WORD;

/* classes ********************************************************************/

class PbLabel : public vector<Int> {
public: 
    void addNumber(Int i);
};

class Pbf {
protected:
    Int declaredVarCount = DUMMY_MAX_INT;
    Int apparentVarCount = DUMMY_MIN_INT;
    PBWeightFormat weightFormat;
    Map<Int, Float> literalWeights;
    vector<vector<Int> > variables;
    vector<vector<Int> > coefficients;
    vector<Int> limits;
    vector<Int> apparentVars; // vars appearing in clauses, ordered by 1st appearance
    
    void updateApparentVars(Int literal); // adds var to apparentVars
    void addConstraint(const vector<Int> &variables, const vector<Int> &coefficent, const Int &limit); // writes: variables, apparentVars

public:
    Int getDeclaredVarCount() const;
    Int getApparentVarCount() const;
    PBWeightFormat getWeightFormat() const;
    Map<Int, Float> getLiteralWeights() const;
    Int getEmptyConstraintIndex() const; // first (nonnegative) index if found else DUMMY_MIN_INT
    const vector<vector<Int>> &getVariables() const;
    const vector<vector<Int>> &getCoefficients() const;
    const vector<Int> &getLimits() const;
    const vector<Int> &getApparentVars() const;
    void printConstraints() const;
    void sortConstraintsByOrdering();
    Pbf(const string &filePath, PBWeightFormat weightFormat);
    Pbf(const vector<vector<Int>> &variables, const vector<vector<Int>> &coefficients, const vector<Int> &limits);
};