#include "encoder.hpp"

void Encoder::addClause(vector<Int> &clause) {
    if(DEBUG) util::printClause(clause);
    clauses.push_back(clause);
    clauseCnt++;
}

Int Encoder::getNewAuxVar() {
    return ++varCnt;
}

void Encoder::printWeightClause(std::ofstream &outfile) const {
    for(Int x = 1; x <= varCnt; x++) {
        Float weight;
        weight = literalWeights.find(x)!=literalWeights.end() ? literalWeights.at(x) : 1;
        outfile << "w " << x << " " << weight << std::endl;

        weight = literalWeights.find(-x)!=literalWeights.end() ? literalWeights.at(-x) : 1;
        outfile << "w " << -x << " " << weight << std::endl;
    }
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
        printWeightClause(outfile);
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
    if(DEBUG) cout << std::endl << "In intervalEnode left-right : " << left << "-" << right << std::endl;
    vector<Int> auxVars;
    vector<Int> tmpClause;
    if(left == right) { // leaf
        // cout << "In intervalEnode left-right : " << left << "-" << right << std::endl;
        Int xi = variable[left];
        Int ai = coefficient[left];
        auxVars.resize(coefficientBit);
        // formula (10)
        if(DEBUG) cout << "For ai = " << ai << "   xi = " << xi << std::endl;
        for(Int i = 0; i < auxVars.size(); i++) {
            auxVars[i] = getNewAuxVar();        // p_k^{left}
            if(DEBUG) {
                cout << "auxVars[" << i << "]  = " << auxVars[i] << std::endl;
            }
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
        if(DEBUG) cout << "Back to intervalEnode left-right : " << left << "-" << right << std::endl;
        if(auxVarsL.size() != auxVarsR.size()) {
            if(DEBUG) cout << "* different size between " << auxVarsL.size() << " " << auxVarsR.size() << std::endl;
            auxVarsR.push_back(getNewAuxVar());
            if(DEBUG) cout << "auxVarsR add " << auxVarsR[auxVarsR.size()-1] << std::endl;
            tmpClause.push_back(-auxVarsR[auxVarsR.size()-1]);
            addClause(tmpClause); tmpClause.clear();
        }
        auxVars.resize(auxVarsL.size() + 1);
        vector<Int> carryVar(auxVarsL.size());
        for(Int i = 0; i < auxVars.size(); i++) auxVars[i] = getNewAuxVar();
        for(Int i = 0; i < carryVar.size(); i++) carryVar[i] = getNewAuxVar();
        if(DEBUG) {
            cout << "For auxVars: ";
            for(Int i = 0; i < auxVars.size(); i++) cout << " " << auxVars[i];
            cout << std::endl;
            cout << "For carryVars: ";
            for(Int i = 0; i < carryVar.size(); i++) cout << " " << carryVar[i];
            cout << std::endl;
        }
        
        // mathcal{T}^+ (subroot, lsubtree, rsubtree)
        // formula (4)
        if(DEBUG) cout << "formula 4" << std::endl;
        tmpClause.push_back(-auxVars[0]); tmpClause.push_back(-auxVarsL[0]); tmpClause.push_back(-auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(-auxVars[0]); tmpClause.push_back(auxVarsL[0]); tmpClause.push_back(auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(auxVars[0]); tmpClause.push_back(auxVarsL[0]); tmpClause.push_back(-auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(auxVars[0]); tmpClause.push_back(-auxVarsL[0]); tmpClause.push_back(auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        // formula (5)
        if(DEBUG) cout << "formula 5" << std::endl;
        tmpClause.push_back(-carryVar[0]); tmpClause.push_back(auxVarsL[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(-carryVar[0]); tmpClause.push_back(auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        tmpClause.push_back(carryVar[0]); tmpClause.push_back(-auxVarsL[0]); tmpClause.push_back(-auxVarsR[0]);
        addClause(tmpClause); tmpClause.clear();

        // formula (6)
        if(DEBUG) cout << "formula 6" << std::endl;
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
        if(DEBUG) cout << "formula 7" << std::endl;
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
        if(DEBUG) cout << "formula 8" << std::endl;
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

string GenArcEncoder::pair2Str(Int id, Int w) {
    return to_string(id) + "_" + to_string(w); 
}

Int GenArcEncoder::getPair2AuxVar(Map<string, Int> &str2AuxVar, Int id, Int w) {
    string str = pair2Str(id, w);
    if(str2AuxVar.find(str) == str2AuxVar.end()) {
        str2AuxVar[str] = getNewAuxVar();
    }
    return str2AuxVar.at(str);
}

void GenArcEncoder::encodeConstraint(const vector<Int>& variable, const vector<Int> &coefficient, const Int &limit) {
    Int consSize = variable.size();
    Map<string, Int> str2AuxVar;
    vector<Int> preSum(consSize + 1);       // preSum[i] = Sum[a_1, a_i] <--> sum coefficient[0, i) i >= 1
    std::queue<Pair<Int, Int> > unmarked;
    vector<Int> tmpClause;

    if(DEBUG) util::printConstraint(variable, coefficient, limit);

    preSum[0] = 0;
    for(Int i = 0; i < consSize; i++) {
        preSum[i + 1] = preSum.at(i) + coefficient.at(i);
    }

    // str2AuxVar[pair2Str(consSize, limit)] = getNewAuxVar();
    tmpClause.clear();
    tmpClause.push_back(getPair2AuxVar(str2AuxVar, consSize, limit));
    addClause(tmpClause);

    unmarked.push({consSize, limit});

    while(!unmarked.empty()) {
        Int id = unmarked.front().first, w = unmarked.front().second; unmarked.pop(); 

        if(w == 0) {
            for(Int i = 0; i < id; i++) {
                tmpClause.clear();
                tmpClause.push_back(-variable.at(i)); tmpClause.push_back(-getPair2AuxVar(str2AuxVar, id, w));
                addClause(tmpClause);
            }

            tmpClause.clear();
            for(Int i = 0; i < id; i++) {
                tmpClause.push_back(variable.at(i));
            }
            tmpClause.push_back(getPair2AuxVar(str2AuxVar, id, w));
            addClause(tmpClause);
        } else if (w < 0) {
            tmpClause.clear();
            tmpClause.push_back(-getPair2AuxVar(str2AuxVar, id, w));
            addClause(tmpClause);
        } else if (w >= preSum.at(id)) {
            tmpClause.clear();
            tmpClause.push_back(getPair2AuxVar(str2AuxVar, id, w));
            addClause(tmpClause);
        } else {                    // D_{id, w} is not terminal node
            if(str2AuxVar.find(pair2Str(id - 1, w - coefficient.at(id - 1))) == str2AuxVar.end()) {
                unmarked.push({id - 1, w - coefficient.at(id - 1)});
            }
            if(str2AuxVar.find(pair2Str(id - 1, w)) == str2AuxVar.end()) {
                unmarked.push({id - 1, w});
            }


            tmpClause.clear();
            tmpClause.push_back(-getPair2AuxVar(str2AuxVar, id - 1, w - coefficient.at(id - 1)));
            tmpClause.push_back(getPair2AuxVar(str2AuxVar, id, w));
            addClause(tmpClause);

            tmpClause.clear();
            tmpClause.push_back(-getPair2AuxVar(str2AuxVar, id, w));
            tmpClause.push_back(getPair2AuxVar(str2AuxVar, id - 1, w));
            addClause(tmpClause);

            tmpClause.clear();
            tmpClause.push_back(-getPair2AuxVar(str2AuxVar, id, w));
            tmpClause.push_back(-variable.at(id - 1));                   // x_i
            tmpClause.push_back(getPair2AuxVar(str2AuxVar, id - 1, w - coefficient.at(id - 1)));
            addClause(tmpClause);

            tmpClause.clear();
            tmpClause.push_back(-getPair2AuxVar(str2AuxVar, id - 1, w));
            tmpClause.push_back(variable.at(id - 1));
            tmpClause.push_back(getPair2AuxVar(str2AuxVar, id, w));
            addClause(tmpClause);
        }
    }
}