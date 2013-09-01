#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace clang;
using namespace oclint;


// Wrapper around ParmVarDecl to allow implementing == for name based matching.
class ParamType
{
private:
    clang::ParmVarDecl *decl;
public:
    ParamType(clang::ParmVarDecl *param) : decl(param) {}

    string getNameAsString() const
    {
        return decl->getNameAsString();
    }

    bool operator == (const ParamType &other) const
    {
        return this->getNameAsString() == other.getNameAsString();
    }

    clang::Decl * getDecl() const
    {
        return decl;
    }
};

typedef unordered_set<ParamType> CandidateClump;
typedef unordered_map<CandidateClump, long> ClumpCounts;

namespace std
{
    template<>
    struct hash<ParamType>
    {
        size_t operator () (const ParamType decl) const
        {
            hash<string> hasher;
            return hasher(decl.getNameAsString());
        }
    };

    template<>
    struct hash<CandidateClump>
    {
        size_t operator () (const CandidateClump& clump) const
        {
            size_t result = 0;
            hash<ParamType> hasher;

            for (auto &param : clump)
            {
                result = result ^ hasher(param);
            }

            return result;
        }
    };
}

class DataClumpRule : public AbstractASTVisitorRule<DataClumpRule>
{
private:
    static RuleSet rules;
    vector<vector<ParmVarDecl *>> _params;

    int numParamThreshold = 2;
    int numClumpOccurancesThreshold = 3;

    vector<CandidateClump> groups_of(int group_size, vector<ParmVarDecl *>::iterator begin,
                                    vector<ParmVarDecl *>::iterator end)
    {
        vector<CandidateClump> allSets;

        if (group_size == 1) {
            for (auto& param = begin; param != end; param++)
            {
                CandidateClump subset;
                subset.insert(ParamType(*param));
                allSets.push_back(subset);
            }
        }
        else
        {
            for (auto& param = begin; param != end  && end - param >= group_size; param++)
            {
                vector<CandidateClump> groups = groups_of(group_size - 1, param + 1, end);
                for (auto &subset: groups)
                {
                    subset.insert(ParamType(*param));
                    allSets.push_back(subset);
                }
            }
        }
        return allSets;
    }
    
    vector<CandidateClump> build_candidate_clumps(vector<ParmVarDecl *> params)
    {
        vector<CandidateClump> allGroups;

        for (int size = numParamThreshold; size <= params.size(); ++size)
        {
            vector<CandidateClump> newGroups = groups_of(size, params.begin(), params.end());
            allGroups.insert( allGroups.end(), newGroups.begin(), newGroups.end() );
        }

        return allGroups;
    }
    
    void count_candidate_clumps(vector<CandidateClump> candidates,
                                ClumpCounts &counts)
    {
        for (auto &candidate : candidates)
        {
            counts[candidate]++;
        }
    }
    
    void report_clumps(ClumpCounts &counts)
    {
        for (auto &count : counts) {
            if (count.second >= numClumpOccurancesThreshold)
            {
                CandidateClump clump = count.first;
                addViolation(clump.begin()->getDecl(), this, description(clump, count.second));
            }
        }
    }

    string description(CandidateClump &clump, long count)
    {
        ostringstream stream;
        stream << "Data clump: Parameters ( ";
        for (auto &param : clump)
        {
            stream << param.getNameAsString() << " ";
        }
        stream << ") used together in " << count << " methods.";
        return stream.str();
    }

    void findAndReportClumps()
    {
        // Currently Reek looks for a group of two or more parameters with
        // the same names that are expected by three or more methods of a class.
        
        // for each parameter list
        //   build cantidates clumps for all groups of <numParamThreshold> parameters
        //     order doesn't matter so use a set
        //     use the sets as a key into an unordered_map
        //     count the occurance of each cantidates clump
        // report all cantidates clumps with occurance higher than <numClumpOccurancesThreshold>
        
        // XXX Need to limit to a single class
        //   XXX Including categories on that class would be nice

        // XXX Need to check that types match
        
        // XXX Need to eliminate reducndancy in subclumps
        //   i.e right now I get reports for:
        //     ( sport position user ) used together in 3 methods.
        //     ( position sport ) used together in 3 methods.
        //     ( position user ) used together in 3 methods.
        //     ( sport user ) used together in 3 methods.

        ClumpCounts counts;
        for (auto &param_group : _params) {
            vector<CandidateClump> new_candidate_clumps(build_candidate_clumps(param_group));
            count_candidate_clumps(new_candidate_clumps, counts);
        }
        report_clumps(counts);
    }

public:
    virtual const string name() const
    {
        return "data clump";
    }

    virtual int priority() const
    {
        return 3;
    }

    virtual void setUp()
    {
        _params.clear();
    }

    virtual void tearDown()
    {
        findAndReportClumps();
    }

    bool VisitFunctionDecl(FunctionDecl *decl)
    {
        if (decl->param_size() >= numParamThreshold)
        {
            vector<ParmVarDecl *> args(decl->param_begin(), decl->param_end());
            _params.push_back(args);
        }
        return true;
    }

    bool VisitCXXMethodDecl(CXXMethodDecl *decl)
    {
        if (decl->param_size() >= numParamThreshold)
        {
            vector<ParmVarDecl *> args(decl->param_begin(), decl->param_end());
            _params.push_back(args);
        }
        return true;
    }

    bool VisitObjCMethodDecl(ObjCMethodDecl *decl)
    {
        if (decl->param_size() >= numParamThreshold)
        {
            vector<ParmVarDecl *> args(decl->param_begin(), decl->param_end());
            _params.push_back(args);
        }
        return true;
    }
};

RuleSet DataClumpRule::rules(new DataClumpRule());
