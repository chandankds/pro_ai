//===- PostDominanceFrontier.cpp - Post-Dominanance Frontier Calculation --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the post-dominator construction algorithms.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetOperations.h"

#include "PostDominanceFrontier.h"
#include "DominatorInternals.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
//  PostDominanceFrontier Implementation
//===----------------------------------------------------------------------===//


//namespace {
/// PostDominanceFrontier Class - Concrete subclass of DominanceFrontier that is
/// used to compute the a post-dominance frontier.
	

char PostDominanceFrontier::ID = 0;

const DominanceFrontier::DomSetType &
PostDominanceFrontier::calculate(const PostDominatorTree &DT,
		const DomTreeNode *Node) {
	// Loop over CFG successors to calculate DFlocal[Node]
	BasicBlock *BB = Node->getBlock();
	DomSetType &S = Frontiers[BB];       // The new set to fill in...
	if (getRoots().empty()) return S;

	if (BB)
		for (pred_iterator SI = pred_begin(BB), SE = pred_end(BB);
				SI != SE; ++SI) {
			BasicBlock *P = *SI;
			// Does Node immediately dominate this predecessor?
			DomTreeNode *SINode = DT[P];
			if (SINode && SINode->getIDom() != Node)
				S.insert(P);
		}

	// At this point, S is DFlocal.  Now we union in DFup's of our children...
	// Loop through and visit the nodes that Node immediately dominates (Node's
	// children in the IDomTree)
	//
	for (DomTreeNode::const_iterator
			NI = Node->begin(), NE = Node->end(); NI != NE; ++NI) {
		DomTreeNode *IDominee = *NI;
		const DomSetType &ChildDF = calculate(DT, IDominee);

		DomSetType::const_iterator CDFI = ChildDF.begin(), CDFE = ChildDF.end();
		for (; CDFI != CDFE; ++CDFI) {
			if (!DT.properlyDominates(Node, DT[*CDFI]))
				S.insert(*CDFI);
		}
	}

	return S;
}

#if 0
INITIALIZE_PASS_BEGIN(PostDominanceFrontier, "postdomfrontier",
                "Post-Dominance Frontier Construction", true, true);
INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
INITIALIZE_PASS_END(PostDominanceFrontier, "postdomfrontier",
                "Post-Dominance Frontier Construction", true, true);
#endif

static RegisterPass<PostDominanceFrontier> X("-postdomfrontier", "Post Dominance Frontier.",
	  true /* Only looks at CFG? */,
	  true /* Analysis Pass? */);

//FunctionPass* createPostDomFrontier();

//} // End anon namespace
























/// OLD, WTF CODE


//FunctionPass* llvm::createPostDomFrontier() {
 // return new PostDominanceFrontier();
//}
