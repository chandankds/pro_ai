//Usage:
//$LLVM_BUILD/bin/opt -load $LLVM_BUILD/lib/ArithmeticIntensity.so -arith-intensity file.ll

//It is assumed that the loop is of the form for(i=0;i<trip_count;i++).
//It is checked if the value of trip_count can be found as an integer constant.

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "ArithmeticIntensity.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

/*
 * Function to be run when the pass is called. This computes the Arithmetic
 * Intensity Arguments : F - the function Return value : always false for an
 * analysis pass
 */
bool ArithmeticIntensity::runOnFunction(Function &F) {
	results = "funcname=" + F.getName().str() +
		", arithmetic-intensity="; // add to the result
	DataLayout *d = new DataLayout(
			F.getParent()); // DataLayout object to get the size of data types
	// initialize iterators to iterate over the function
	inst_iterator current = inst_begin(F);
	inst_iterator end = inst_end(F);
	int floating_point_count = 0; // number of FLOPs found
	int bit_count = 0;            // bytes of load/store found
	std::vector<int> floating_ops = {
		Instruction::FAdd, Instruction::FSub, Instruction::FMul,
		Instruction::FDiv, Instruction::FRem }; // vector of all applicable FLOPs
	// iterating over all instructions in the function
	for (; current != end; current++) {
		int op_code = (*current).getOpcode(); // get the Op Code
/*
		std::vector<Loop*> sub_loops = loop->getSubLoops();
		
		// check all subloops for canon var increments
		if(sub_loops.size() > 0) {
		for(std::vector<Loop*>::iterator sub_loop = sub_loops.begin(), sl_end = sub_loops.end(); sub_loop != sl_end; ++sub_loop) 
		gatherInductionVarIncrements(*sub_loop,ind_vars,ind_var_increment_ops);
		}
		*/

		if (std::find(floating_ops.begin(), floating_ops.end(), op_code) !=
				floating_ops.end()) // if it is a FLOP
		{
			floating_point_count++;
		} else if (op_code == Instruction::Load ||
				op_code == Instruction::Store) // if it is a load/store
		{
			Value *casted_instruction;
			// cast based on load/store and get pointer operand
			if (op_code == Instruction::Load) {
				casted_instruction = cast<LoadInst>(&(*current))->getPointerOperand();
			} else {
				casted_instruction = cast<StoreInst>(&(*current))->getPointerOperand();
			}
			PointerType *pointer_operand_type =
				cast<PointerType>(casted_instruction->getType()); // get the type
			int number_of_bits = d->getTypeStoreSize(
					pointer_operand_type
					->getPointerElementType());   // get the size of the type
			bit_count += number_of_bits;          // add to the total count

			/*      bool has_reduction_var_use_pattern =  uses_in_loop.size() == 2
				&& isa<LoadInst>(uses_in_loop[1])
				&& isa<StoreInst>(uses_in_loop[0])
				&& uses_in_loop[1]->hasOneUse();

				if (has_reduction_var_use_pattern) {
				Instruction* load_user = cast<Instruction>(*uses_in_loop[1]->use_begin());
			//LOG_DEBUG() << "\tuser of load: " << PRINT_VALUE(*load_user);

			if( load_user->hasOneUse()
			&& uses_in_loop[0] == *load_user->use_begin()
			&& isReductionOpType(load_user)
			) {
			//LOG_DEBUG() << "\t\thot diggity dawg, that is it!\n";	
			*/

		} else if (op_code == Instruction::PHI) // if it is a PHI node
		{
			/*
			   bool ReductionVars::isReductionOpType(Instruction* inst)
			   {
			   if( inst->getOpcode() == Instruction::Add
			   || inst->getOpcode() == Instruction::FAdd
			   || inst->getOpcode() == Instruction::Mul
			   || inst->getOpcode() == Instruction::FMul
			   ) { return true; }
			   else { return false; }
			   }
			   */
			int type_size = 0;
			// iterate over all the operands
			for (unsigned int i = 0; i < (*current).getNumOperands(); i++) {
				Value *v = (*current).getOperand(i); // get the current operand
				if (dyn_cast<Constant>(v)) // for a constant operand, I count one store
				{
					bit_count += d->getTypeStoreSize(v->getType());
				} else // for a non-constant, I count total three operations - 1 load,
					// 1 store
				{
					bit_count += 2 * d->getTypeStoreSize(v->getType());
				}
				type_size = d->getTypeStoreSize(v->getType());
			}
			bit_count += type_size; // one more load for the result of the PHI
		}
		}
		float arithmetic_intensity; // to store the arithmetic intensity
		if (bit_count == 0)         // if the denominator is 0
		{
			results += "undef\n"; // add to the result
			dbgs() << results;
			return false;
		}
		LoopInfo &LI = getAnalysis<LoopInfo>(); // to handle the case of loops
		std::pair<int, int> loop_result;   // to store the result for loops, first is
		// the FLOP count, second is byte count
		std::pair<int, int> ind_result;    // to store result for individual loops

		/*   for(unsigned j = 0; j < loop_blocks.size(); ++j) {
		     BasicBlock* bb = loop_blocks[j];

		     std::map<Value*,std::vector<GetElementPtrInst*> > ptr_val_to_geps;

		// create map from pointer values to GEPs that use them
		for(BasicBlock::iterator inst = bb->begin(), inst_end = bb->end(); inst != inst_end; ++inst) {
		if(GetElementPtrInst* gep = dyn_cast<GetElementPtrInst>(inst)) {
		ptr_val_to_geps[gep->getPointerOperand()].push_back(gep);
		}
		}

		for(std::map<Value*,std::vector<GetElementPtrInst*> >::iterator gp_it = ptr_val_to_geps.begin(), gp_end = ptr_val_to_geps.end(); gp_it != gp_end; ++gp_it) {
		Instruction* red_var_op = NULL;

		std::vector<GetElementPtrInst*> gep_vector = (*gp_it).second;

		// If this mapped to only one GEP, we'll see if that GEP is used
		// by load and store as part of reduction op sequence.
		if(gep_vector.size() == 1) {
		red_var_op = getReductionVarOp(li,loop,(*gp_it).second.front());
		}    std::vector<BasicBlock*> loop_blocks = loop->getBlocks();
		*/
		int loop_no = 1;
		for (Loop *L : LI) {
			std::cout << "For loop no " << loop_no++ << std::endl;
			ind_result = count_in_loop(L, 1, d); // current iteration level is 1, and passing d
			// for using getTypeStoreSize()
			std::cout << "The FLOPS are :: " << ind_result.first << " Bits L/S :: " << ind_result.second << std::endl;

			/*
			// Create map between incoming values and the block(s) they come from.
			unsigned int num_incoming = phi.getNumIncomingValues();
			for(unsigned int i = 0; i < num_incoming; i++) {
			Value* incoming_value = phi.getIncomingValue(i);
			equivelent_values[incoming_value].insert(phi.getIncomingBlock(i));
			}

			// For each incoming value, we find the nearest common dominator of all incoming
			// blocks that contain that value, adding it to the incoming_blocks set.
			std::set<BasicBlock*> incoming_blocks;
			for(std::map<Value*, std::set<BasicBlock*> >::iterator it = equivelent_values.begin(), end = equivelent_values.end(); it != end; it++)
			incoming_blocks.insert(findNearestCommonDominator(it->second));

			// Add all the controlling blocks of the incoming blocks.
			foreach(BasicBlock* bb, incoming_blocks)
			getPhiControllingBlocks(bb, idom[bb_containing_phi], dt, result);
			*/	

			if (ind_result.first == -1 && ind_result.second == -1) // if any loop returned undef
			{
				loop_result = std::make_pair(-1, -1); // the result is undef
				results += "undef\n"; // result is undef
				dbgs() << results;
				continue;
			} else {
				// add loop result to earlier results
				//    floating_point_count += loop_result.first;
				//    bit_count += loop_result.second;
				floating_point_count = ind_result.first;
				bit_count = ind_result.second;
				arithmetic_intensity = (floating_point_count * 1.0) / bit_count; // compute arithmetic intensity
				//      results += std::to_string(arithmetic_intensity) + "\n"; // store result
				dbgs() << results << arithmetic_intensity;
				std::cout << std::endl ;
				std::cout << std::endl ;
			}


		}
		/*  if (loop_result.first == -1 &&
		    loop_result.second == -1) // if any undef received
		    {
		    results += "undef\n"; // result is undef
		    dbgs() << results;
		    return false;
		    } else {
		// add loop result to earlier results
		//    floating_point_count += loop_result.first;
		//    bit_count += loop_result.second;
		floating_point_count = loop_result.first;
		bit_count = loop_result.second;
		arithmetic_intensity = (floating_point_count * 1.0) /
		bit_count; // compute arithmetic intensity
		results += std::to_string(arithmetic_intensity) + "\n"; // store result
		}
		dbgs() << results;
		*/
		return false;
		}

		/*
		 * Function to recursively count the number of FLOP and load/store operations in
		 * loops It is assumed that the trip count of the loop is statically known, and
		 * that the header contains the icmp instruction. It is also assumed that if the
		 * above is found, the loop is of the form for(int i = 0; i < trip_count; i++).
		 * In all other cases, undef is returned Arguments : L - pointer to the current
		 * loop iter_level - number of times this loop will be iterated due to other
		 * loops outside d - DataLayout pointer for getTypeStoreSize() Return value : a
		 * pair of ints, where the first is the number of FLOPs in the loop, and the
		 * second is the byte count of load/store operations. For undef, (-1,-1) is
		 * returned
		 */
		std::pair<int, int> ArithmeticIntensity::count_in_loop(Loop *L, int iter_level,
				DataLayout *d) {
			int floating_point_count = 0; // to store the FLOP count
			int bit_count = 0;            // to store the number of bytes of load/store
			std::vector<int> floating_ops = {
				Instruction::FAdd, Instruction::FSub, Instruction::FMul,
				Instruction::FDiv,
				Instruction::FRem};             // vector containing all applicable FLOPs
			auto *loop_header = L->getHeader(); // the header of the loop. It is assumed
			// that the icmp instruction is here
			auto *terminator_of_header =
				loop_header->getTerminator(); // the terminator, br, of the header
			int const_value_found = 0; // to store number of constants found for icmp
			int trip_count = 0;        // to store the trip count
			for (Use &U1 :
					terminator_of_header
					->operands()) // with usedef chain, get the operands of the br
			{
				Value *v = U1.get();                              // this should be the icmp
				auto *operand_to_inst = dyn_cast<Instruction>(v); // cast it to Instruction
				if (operand_to_inst &&
						operand_to_inst->getOpcode() ==
						Instruction::ICmp) // if cast successful, and instruction is icmp
				{
					for (Use &U2 : operand_to_inst->operands()) // traverse over its operands
					{
						Value *operands_of_cmp = U2.get();
						if (ConstantInt *CI = dyn_cast<ConstantInt>(
									operands_of_cmp)) // if the operand is a constant int
						{
							trip_count = CI->getLimitedValue(); // that will be the trip count
							const_value_found++; // to check that only one constant int is taken.
							// if I have two, I count it as undef
						}
					}
				}
				break;
			}
			// if no constant, more than one constant, or negative trip count, the result
			// is undef
			//  if (const_value_found == 0 || const_value_found > 1 || trip_count < 0) {
			//    return std::make_pair(-1, -1);
			// }
			int h_floating_point_count = 0,
			    h_bit_count =
				    0; // separately track for the header because the condition block
			// statements are executed one more than the trip count
			for (auto *bb : L->getBlocks()) // iterate over the blocks in the loop
			{
				// iterate over the instructions in the block
				for (BasicBlock::iterator current = bb->begin(); current != bb->end();
						current++) {

					/*    std::vector<Instruction*> uses_in_loop = getNonPhiUsesInLoop(li,loop,val);

					// should have two uses: load then store
					if(uses_in_loop.size() != 2) return NULL;

					// By construction, we know that the two users are GEP insts Note: order
					// is reversed when doing getNonPhiUsesInLoop, so user0 is actually entry
					// 1 and vice versa
					GetElementPtrInst* user0_gep = dyn_cast<GetElementPtrInst>(uses_in_loop[1]);
					GetElementPtrInst* user1_gep = dyn_cast<GetElementPtrInst>(uses_in_loop[0]);

					if(!user0_gep->hasOneUse() || !user1_gep->hasOneUse()) return NULL;

					Instruction* should_be_load = cast<Instruction>(*user0_gep->use_begin());

					if(!isa<LoadInst>(should_be_load)) { return NULL; }
					else if(!isa<StoreInst>(*user1_gep->use_begin())) { return NULL; }

					Instruction* load_user = cast<Instruction>(*should_be_load->use_begin());

					if( load_user->hasOneUse()
					make sure load_user is stored?
					&& isReductionOpType(load_user)
					) {
					return load_user;
					}
					*/


					int op_code = (*current).getOpcode();
					if (std::find(floating_ops.begin(), floating_ops.end(), op_code) !=
							floating_ops.end()) {
						floating_point_count++;
						if (bb == L->getHeader()) // for the header, add separately
						{
							h_floating_point_count++;
						}
					} else if (op_code == Instruction::Load ||
							op_code == Instruction::Store) {
						Value *casted_instruction;
						if (op_code == Instruction::Load) {
							casted_instruction = cast<LoadInst>(&(*current))->getPointerOperand();
						} else {
							casted_instruction =
								cast<StoreInst>(&(*current))->getPointerOperand();
						}

						/*    BasicBlock* header = loop->getHeader();

						// Check all subloops for reduction vars
						std::vector<Loop*> sub_loops = loop->getSubLoops();
						for(unsigned i = 0; i < sub_loops.size(); ++i) {
						getReductionVars(li,sub_loops[i]);
						}


						// If there weren't any subloops, we'll examine all the global variables
						// We also need to check for a special type of "array" reduction that
						// requires a different type of identification.
						if(sub_loops.empty()) {
						Module* mod = header->getParent()->getParent();

						// Examine each global variable to see if it's a reduction var.
						for(Module::global_iterator gi = mod->global_begin(), ge = mod->global_end(); gi != ge; ++gi) {
						if(Instruction* red_var_op = getReductionVarOp(li,loop,gi)) {
						LOG_INFO() << "identified reduction variable operator (global, used in function: "
						<< red_var_op->getParent()->getParent()->getName() 
						<< "): " << *red_var_op << "\n"
						<< "\treduction var: " << *gi << "\n";
						red_var_ops.insert(red_var_op);
						}
						}

						// Check for "array" reduction.
						getArrayReductionVars(li,loop,red_var_ops);
						}

						PHINode* civ = loop->getCanonicalInductionVariable();
						*/
						PointerType *pointer_operand_type =
							cast<PointerType>(casted_instruction->getType());
						int number_of_bits =
							d->getTypeStoreSize(pointer_operand_type->getPointerElementType());
						bit_count += number_of_bits;
						if (bb == L->getHeader()) {
							h_bit_count += number_of_bits;
						}
					} else if (op_code == Instruction::PHI) {
						int type_size = 0;
						int number_of_bits = 0;
						for (unsigned int i = 0; i < (*current).getNumOperands(); i++) {
							Value *v = (*current).getOperand(i);
							if (dyn_cast<Constant>(v)) {
								number_of_bits = d->getTypeStoreSize(v->getType());
								bit_count += number_of_bits;
								if (bb == L->getHeader()) {
									h_bit_count += number_of_bits;
								}
							} else {
								number_of_bits = 2 * d->getTypeStoreSize(v->getType());
								bit_count += number_of_bits;
								if (bb == L->getHeader()) {
									h_bit_count += number_of_bits;
								}
							}
							type_size = d->getTypeStoreSize(v->getType());
						}
						/*
						   for(User::op_iterator gepi_op = gepi->idx_begin(), gepi_ops_end = gepi->idx_end(); 
						   gepi_op != gepi_ops_end; 
						   gepi_op++) 
						   {
						// We are only looking for non-consts that aren't induction variables.
						// We avoid induction variables because they add unnecessary dependencies
						// (i.e. we don't care that i is a dep for a[i])
						PHINode* gepi_op_phi = dyn_cast<PHINode>(gepi_op);
						Instruction* gepi_op_inst = dyn_cast<Instruction>(gepi_op);

						if(!isa<ConstantInt>(gepi_op) &&
						!(gepi_op_phi && induc_vars.isInductionVariable(*gepi_op_phi)) &&
						!(gepi_op_inst && induc_vars.isInductionIncrement(*gepi_op_inst))) 
						{
						//LOG_DEBUG() << "getelementptr " << gepi->getName() << " depends on " << gepi_op->get()->getName() << "\n";

#if 0
push_int(ts_placer.getId(*gepi_op->get()));
#endif
args.push_back(ConstantInt::get(types.i32(), ts_placer.getId(*gepi_op->get()), false));
num_conds++;
}
}
*/        bit_count += type_size;
if (bb == L->getHeader()) {
	h_bit_count += type_size;
}
}
}
}
std::pair<int, int> result; // to store the result
// I counted the instructions in the loop previously iter_level times. so, to
// avoid double counting, I subtract iter_level from the trip_count. However,
// I add iter_level for loop condition blocks as their statements are
// executed one time more than the trip count
//  result.first = floating_point_count * iter_level * (trip_count - 1) +
//                 h_floating_point_count * iter_level;
//  result.second =
//      bit_count * iter_level * (trip_count - 1) + h_bit_count * iter_level;

result.first = floating_point_count + h_floating_point_count;
result.second = bit_count + h_bit_count;
//  result.first = floating_point_count;
// result.second = bit_count;
//  std::cout << std::endl ;


/*  std::pair<int, int> recurse_result; // to store the result of inner loops
    for (auto *NL : *L) {
    recurse_result = count_in_loop(NL, trip_count * iter_level, d);
    if (recurse_result.first == -1 &&
    recurse_result.second == -1) // if any inner loop gave undef
    {
    return std::make_pair(-1, -1);
    }
    result.first += recurse_result.first;
    result.second += recurse_result.second;
    }
    */
return result;
}

void ArithmeticIntensity::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.addRequired<LoopInfo>();
	AU.addPreserved<LoopInfo>();
	AU.setPreservesAll();
}

// register the pass
char ArithmeticIntensity::ID = 0;
static RegisterPass<ArithmeticIntensity>
X("ai",
		"Pass to calculate the Arithmetic Intensity of a function");
