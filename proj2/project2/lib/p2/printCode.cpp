//===--------------- printCode.cpp - Project 1 for CS 701 ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a skeleton of an implementation for the printCode
// pass of Univ. Wisconsin-Madison's CS 701 Project 1.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "printCode"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Statistic.h"
#include <iostream>
#include <vector>
#include <map>
using namespace llvm;

namespace {
  class printCode : public FunctionPass {
    std::map<Instruction *,int> Imap;
    static int inst_id;
    public:
    static char ID; // Pass identification, replacement for typeid
    printCode() : FunctionPass(ID) {}
    
    //**********************************************************************
    // runOnFunction
    //**********************************************************************
    virtual bool runOnFunction(Function &F) {
      // print fn name
      std::cerr << "FUNCTION " << F.getName().str() << "\n";
      for(Function::iterator iter=F.begin();iter!=F.end();iter++)
      {
        // std::cerr<<"BLOCKS "<<(iter)->getName().str()<<"\n";
	 for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++,inst_id++)
	 {
	  //  std::cerr<<"INSTS "<<(i)->getOpcodeName()<<"\n";
	      Imap[i]=inst_id;
         }
      }
      for(Function::iterator iter=F.begin();iter!=F.end();iter++)
      {
         std::cerr<<"\nBASIC  BLOCK "<<(iter)->getName().str()<<"\n";
	 for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++)
	 {
	    std::cerr<<"%"<<Imap[i]<<":"<<"    "<<(i)->getOpcodeName()<<"    ";
	    int numOperands = i->getNumOperands();
	    //std::cerr<<"Value of operands is : "<<numOperands<<"\n";
	    for(int j=0;j<numOperands;j++)
	    {

	       if(isa<Instruction>(i->getOperand(j)))
	       {
		std::cerr<<"%"<<Imap[(Instruction *)i->getOperand(j)]<<"    ";
	       }
		else
		{
	       if(i->getOperand(j)->hasName())
	       std::cerr<<i->getOperand(j)->getName().str()<<"    ";
		else
	       std::cerr<<"XXX"<<"    ";
		}

		}
		std::cerr<<"\n";	     
         }
      }

      // MISSING: Add code here to do the following:
      //          1. Iterate over the instructions in F, creating a
      //             map from instruction address to unique integer.
      //             (It is probably a good idea to put this code in
      //             a separate, private method.)
      //          2. Iterate over the basic blocks in the function and
      //             print each instruction in that block using the format
      //             given in the assignment.

      return false;  // because we have NOT changed this function
    }

    //**********************************************************************
    // print (do not change this method)
    //
    // If this pass is run with -f -analyze, this method will be called
    // after each call to runOnFunction.
    //**********************************************************************
    virtual void print(std::ostream &O, const Module *M) const {
        O << "This is printCode.\n";
    }

    //**********************************************************************
    // getAnalysisUsage
    //**********************************************************************

    // We don't modify the program, so we preserve all analyses
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
    };

  };
  char printCode::ID = 0;
  int printCode::inst_id = 1;

  // register the printCode class: 
  //  - give it a command-line argument (printCode)
  //  - a name ("print code")
  //  - a flag saying that we don't modify the CFG
  //  - a flag saying this is not an analysis pass
  RegisterPass<printCode> X("printCode", "print code",
			   true, false);
}
