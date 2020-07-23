#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DataLayout.h"
#include <utility>
#include <string>

using namespace llvm;

/*
 * class for the ArithmeticIntensity pass
 */
class ArithmeticIntensity : public FunctionPass 
{
    public:
        static char ID;
        std::string results;
        ArithmeticIntensity() : FunctionPass(ID)
        {
            results = "";
        }
        bool runOnFunction(Function &F) override; 
        std::pair<int,int> count_in_loop(Loop *L,int, DataLayout*); // to handle counting in loops
        void getAnalysisUsage(AnalysisUsage &AU) const;
};
