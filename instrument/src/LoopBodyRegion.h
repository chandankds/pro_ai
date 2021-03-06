#ifndef LOOP_BODY_REGION_H
#define LOOP_BODY_REGION_H

#include "Region.h"
#include <llvm/Analysis/LoopInfo.h>

class LoopBodyRegion : public Region
{
  private:
	llvm::Loop* loop;
	RegionId id;

	std::string fileName;
	std::string funcName;
	unsigned int startLine;
	unsigned int endLine;

  public:
	static const std::string REGION_NAME;

	LoopBodyRegion(RegionId id, llvm::Loop* func);
	virtual ~LoopBodyRegion();

	virtual RegionId getId() const;
	virtual const std::string& getRegionType() const;
	virtual const std::string& getFileName() const;
	virtual const std::string& getFuncName() const;
	virtual unsigned int getEndLine() const;
	virtual unsigned int getStartLine() const;
};


#endif // LOOP_BODY_REGION_H
