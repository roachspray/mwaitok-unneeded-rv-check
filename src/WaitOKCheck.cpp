/*
 * quick hack to find malloc(9) with M_WAITOK and a compare to NULL
 */


#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CallSite.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugInfoMetadata.h"

using namespace llvm;

#include "WaitOKCheck.h"

void
WaitOKCheck::getAnalysisUsage(AnalysisUsage &AU) const
{

	AU.setPreservesCFG(); 
}

/*
 * Another way to do this is to make an InstVisitor to
 * go to call sites and compare sites... but so it goes.
 *
 */
bool
WaitOKCheck::runOnModule(Module &M)
{

	for (auto &F : M) {
		if (F.isIntrinsic()) {
			continue;
		}
		std::string fname = "<noname>";
		if (F.hasName()) {
			fname = F.getName().str();
		}
		if (fname == "malloc" ||
		    fname == "__curthread") {
			continue;
		}

		for (auto &BB : F) {
			for (auto &I : BB) {
				if (!isa<CallInst>(&I)) {
					continue;
				}
				CallInst *ci = cast<CallInst>(&I);
				Function *calledFn = ci->getCalledFunction();
				if (calledFn == NULL) {
					continue;
				}
				if (!calledFn->hasName()) {
					continue;
				}
				StringRef sr = calledFn->getName();
				if (sr.empty()) {
					continue;
				}

				// these 4 can use M_WAITOK
				if (!sr.equals("malloc")
				    && !sr.equals("mallocarray")
				    && !sr.equals("realloc")
				    && !sr.equals("reallocf")) {
				   	
					continue;
				}
				Value *arg = ci->getArgOperand(ci->getNumArgOperands() - 1);
				if (arg == NULL) {
					continue;
				}
				if (!isa<ConstantInt>(arg)) {
					continue;
				}
				ConstantInt *conInt = cast<ConstantInt>(arg);
				APInt a = conInt->getValue();
				a &= 0x0002;
				if (a == 0) {
					continue;
				}
				//errs() << sr.str() << "(M_WAITOK) in '" << fname << "'\n";
				/*
				 * Hack: basic idea is after mem2reg pass,
				 * there would be a bitcast then compare to
				 * do a null check. This is reasonable enough
				 * to find most cases.
				*/
				auto uit = ci->user_begin();
				User *u = *uit;
				if (!isa<BitCastInst>(u)) {
					continue;
				}
				Instruction *bci = cast<Instruction>(u);
				auto uit2 = bci->user_begin();
				User *u2 = *uit2;
				if (!isa<CmpInst>(u2)) {
					continue;
				}
				Instruction *i2 = cast<Instruction>(u2);
				MDNode *md2 = i2->getMetadata(0);
				if (!md2) {
					errs() << "\n" << sr.str() << "(M_WAITOK) with a cmp in "   \
					    << fname << "()\n";
					continue;
				} else {
					DILocation *dLoc2 = dyn_cast<DILocation>(md2);
					if (!dLoc2) {
						errs() << "\nFound malloc-compare!\n";
						continue;
					}
					errs() << "\nmalloc(M_WAITOK)-compare @ " <<   \
					    dLoc2->getFilename() << ":" <<   \
					    dLoc2->getLine() << "\n";
					continue;
				}
			}
		}
	}
	return false;
}

char WaitOKCheck::ID = 0;
static RegisterPass<WaitOKCheck> X("waitok-check", "M_WAITOK return value compare checker", false, false);
