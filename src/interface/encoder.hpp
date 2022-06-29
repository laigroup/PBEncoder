#include "pbformula.hpp"

class Encoder {
protected:
    vector<vector<Int> > clauses;
    Int varCnt, clauseCnt;
    PBWeightFormat weightFormat;
    Map<Int, Float> literalWeights;

    void addClause(vector<Int> &clause);
    virtual void encodeConstraint(const vector<Int>& variable, const vector<Int> &coefficient, const Int &limit) = 0;
    Int getNewAuxVar();

public:
    void printCnf(const string &filepath) const;
    void encodePbf(const Pbf &pbf);
    Encoder(){};
};


class WarnersEncoder : public Encoder {
protected:
    Int maxCoefficient, coefficientBit;
    void encodeConstraint(const vector<Int>& variable, const vector<Int> &coefficient, const Int &limit);
    vector<Int> intervalEncode(const vector<Int>& variable, const vector<Int> &coefficient, Int left, Int right);
    void limitEncode(Int limit, vector<Int>& auxVar);
    void weightEncode();

public:
    WarnersEncoder(){};
};