#ifndef	__WAITOKCHECK_H
#define	__WAITOKCHECK_H

struct WaitOKCheck : public ModulePass {
	static char ID;

	WaitOKCheck() : ModulePass(ID) {}
	virtual bool runOnModule(Module &);
	virtual void getAnalysisUsage(AnalysisUsage &) const;

};

#endif
