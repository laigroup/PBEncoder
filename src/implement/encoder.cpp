#include "encoder.hpp"

void Encoder::addClause(vector<Int> &clause) {
    // util::printClause(clause);
    clauses.push_back(clause);
    clauseCnt++;
}

Int Encoder::getNewAuxVar() {
    return ++varCnt;
}

void Encoder::printCnf(const string &filePath) const {
    std::ofstream outfile(filePath);
    if(!outfile.is_open()) {
        util::showError(filePath + " can not open");
    }

    string problemType;

    switch (weightFormat) {
        case PBWeightFormat::UNWEIGHTED : problemType = "cnf"; break;
        case PBWeightFormat::WEIGHTED   : problemType = "wcnf"; break; 
    }
    outfile << "p " << problemType << " " << varCnt << " " << clauseCnt << "\n";

    for(Int i = 0; i < clauseCnt; i++) {
        for(Int literal : clauses[i]) {
            outfile << literal << " ";
        }
        outfile << "0\n";
    }

    if(weightFormat == PBWeightFormat::WEIGHTED) {
        for(Int x = 1; x <= varCnt; x++) {
            Float weight;
            weight = literalWeights.find(x)!=literalWeights.end() ? literalWeights.at(x) : 1;
            outfile << "w " << x << " " << weight << std::endl;

            weight = literalWeights.find(-x)!=literalWeights.end() ? literalWeights.at(-x) : 1;
            outfile << "w " << -x << " " << weight << std::endl;
        }
    }

    outfile.close();
}

void Encoder::encodePbf(const Pbf &pbf) {
    const vector<vector<Int> > &variables = pbf.getVariables();
    const vector<vector<Int> > &coefficients = pbf.getCoefficients();
    const vector<Int> &limits = pbf.getLimits();
    varCnt = pbf.getApparentVarCount();
    clauseCnt = 0;

    for(Int i = 0; i < variables.size(); i++) {
        encodeConstraint(variables.at(i), coefficients.at(i), limits.at(i));
    }

    weightFormat = pbf.getWeightFormat();
    literalWeights = pbf.getLiteralWeights();
}

void WarnersEncoder::encodeConstraint(const vector<Int>& variable, const vector<Int> &coefficient, const Int &limit) {
    Int left = 0, right = variable.size() - 1;

    util::printConstraint(variable,  coefficient, limit);

    maxCoefficient = coefficientBit = 0;
    for(auto coef : coefficient) {
        maxCoefficient = std::max(coef, maxCoefficient);
    }

    while((maxCoefficient >> coefficientBit) > 0) {
        coefficientBit++;
    }

    // cout << "maxCoefficient: " << maxCoefficient << "  coefficientBit: " << coefficientBit << std::endl;

    vector<Int> auxVars = intervalEncode(variable, coefficient, left, right);
    limitEncode(limit, auxVars);
}

vector<Int> WarnersEncoder::intervalEncode(const vector<Int>& variable, const vector<Int> &coefficient, Int left, Int right) {
    // cout << "In intervalEnode left-right : " << left << "-" << right << std::endl;
    vector<Int> auxVars;
    vector<Int> tmpClause;
    if(left == right) { // leaf
        // cout << "In intervalEnode left-right : " << left << "-" << right << std::endl;
        Int xi = variable[left];
        Int ai = coefficient[left];
        auxVars.resize(coefficientBit);
        for(Int i = 0; i < auxVars.size(); i++) {
            auxVars[i] = getNewAuxVar();        // p_k^{left}
            if((ai >> i) & 1) {                 // i \in B_{a_i}
                tmpClause.push_back(-auxVars[i]); tmpClause.push_back(xi);
                addClause(tmpClause); tmpClause.clear();

                tmpClause.push_back(auxVars[i]); tmpClause.push_back(-xi);
                addClause(tmpClause); tmpClause.clear();
            } else {                            // i \notin B_{a_i}
                tmpClause.push_back(-auxVars[i]);
                addClause(tmpClause); tmpClause.clear();
            }
        }
    } else {            // subtree root
        Int mid = (left + right) >> 1;
        vector<Int> auxVarsL = intervalEncode(variable, coefficient, left, mid);
        vector<Int> auxVarsR = intervalEncode(variable, coefficient, mid + 1, right);
        // cout << "In intervalEnode left-right : " << left << "-" << right << std::endl;
        if(auxVarsL.size() != auxVarsR.size()) {
            // cout << "* different size between " << auxVarsL.size() << " " << auxVarsR.size() << std::endl;
            auxVarsR.push_back(getNewAuxVar());
            tmpClause.push_back(-auxVarsR[auxVarsR.size()-1]);          // ???? is this correct???
            addClause(tmpClause); tmpClause.clear();
        }
        auxVars.resize(auxVarsL.size() + 1);
        vector<Int> carryVar(auxVarsL.size());
        for(Int i = 0; i < auxVars.size(); i++) auxVars[i] = getNewAuxVar();
        for(Int i = 0; i < carryVar.size(); i++) carryVar[i] = getNewAuxVar();
        
        // mathcal{T}^+ (subroot, lsubtree, rsubtree)
        // formula (4)
        tmpClause.push_back(-auxVars[0]); tmpClause.push_back(-auxVarsL[0]); tmpClause.push_back(-auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(-auxVars[0]); tmpClause.push_back(auxVarsL[0]); tmpClause.push_back(auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(auxVars[0]); tmpClause.push_back(auxVarsL[0]); tmpClause.push_back(-auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(auxVars[0]); tmpClause.push_back(-auxVarsL[0]); tmpClause.push_back(auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        // formula (5)
        tmpClause.push_back(-carryVar[0]); tmpClause.push_back(auxVarsL[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(-carryVar[0]); tmpClause.push_back(auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        // tmpClause.push_back(carryVar[0]); tmpClause.push_back(-auxVarsL[0]);
        // addClause(tmpClause); tmpClause.clear();

        // tmpClause.push_back(carryVar[0]); tmpClause.push_back(-auxVarsR[0]);
        // addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(carryVar[0]); tmpClause.push_back(-auxVarsL[0]); tmpClause.push_back(-auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        // formula (6)
        for(Int i = 1; i < auxVarsL.size(); i++) {      // [1, Mu - 1]
            tmpClause.push_back(auxVars[i]); tmpClause.push_back(-auxVarsL[i]); 
            tmpClause.push_back(-auxVarsR[i]); tmpClause.push_back(-carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(auxVars[i]); tmpClause.push_back(-auxVarsL[i]); 
            tmpClause.push_back(auxVarsR[i]); tmpClause.push_back(carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(auxVars[i]); tmpClause.push_back(auxVarsL[i]); 
            tmpClause.push_back(-auxVarsR[i]); tmpClause.push_back(carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(auxVars[i]); tmpClause.push_back(auxVarsL[i]); 
            tmpClause.push_back(auxVarsR[i]); tmpClause.push_back(-carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-auxVars[i]); tmpClause.push_back(auxVarsL[i]); 
            tmpClause.push_back(auxVarsR[i]); tmpClause.push_back(carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-auxVars[i]); tmpClause.push_back(auxVarsL[i]); 
            tmpClause.push_back(-auxVarsR[i]); tmpClause.push_back(-carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-auxVars[i]); tmpClause.push_back(-auxVarsL[i]); 
            tmpClause.push_back(auxVarsR[i]); tmpClause.push_back(-carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-auxVars[i]); tmpClause.push_back(-auxVarsL[i]); 
            tmpClause.push_back(-auxVarsR[i]); tmpClause.push_back(carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();
        }

        // formula (7)
        for(Int i = 1; i < auxVarsL.size(); i++) {
            tmpClause.push_back(carryVar[i]); tmpClause.push_back(-auxVarsL[i]); tmpClause.push_back(-auxVarsR[i]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(carryVar[i]); tmpClause.push_back(-auxVarsL[i]); tmpClause.push_back(-carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(carryVar[i]); tmpClause.push_back(-auxVarsR[i]); tmpClause.push_back(-carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-carryVar[i]); tmpClause.push_back(auxVarsL[i]); tmpClause.push_back(auxVarsR[i]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-carryVar[i]); tmpClause.push_back(auxVarsL[i]); tmpClause.push_back(carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();

            tmpClause.push_back(-carryVar[i]); tmpClause.push_back(auxVarsR[i]); tmpClause.push_back(carryVar[i-1]);
            addClause(tmpClause); tmpClause.clear();
        }

        // formula (8)
        tmpClause.push_back(carryVar[carryVar.size()-1]); tmpClause.push_back(-auxVars[auxVars.size()-1]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(-carryVar[carryVar.size()-1]); tmpClause.push_back(auxVars[auxVars.size()-1]);
        addClause(tmpClause); tmpClause.clear();
    }
    return auxVars;
}

void WarnersEncoder::limitEncode(Int limit, vector<Int> &auxVars) {
    vector<Int> tmpClause;
    for(Int i = 0; i < auxVars.size(); i++) {
        if((limit >> i) & 1) continue;
        tmpClause.push_back(-auxVars[i]);
        for(Int j = i + 1; j < auxVars.size(); j++) {
            if((limit >> j) & 1) {
                tmpClause.push_back(-auxVars[j]);
            }
        }
        addClause(tmpClause); tmpClause.clear();
    }
}
