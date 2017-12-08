//===--------------- optLoads.cpp - Project 1 for CS 701 ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a skeleton of an implementation for the optLoads
// pass of Univ. Wisconsin-Madison's CS 701 Project 1.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "optLoads"
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
  class optLoads : public FunctionPass {
    std::map<Instruction *,int> Imap;
    static int inst_id;
    public:
    static char ID; // Pass identification, replacement for typeid
    optLoads() : FunctionPass(ID) {}
    
    //**********************************************************************
    // runOnFunction
    //**********************************************************************
    virtual bool runOnFunction(Function &F) {
      std::vector<Instruction *> LdMap;
      // print fn name
      //std::cerr << "FUNCTION " << F.getName().str() << "\n";
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
         //std::cerr<<"\nBASIC  BLOCK "<<(iter)->getName().str()<<"\n";
	 int flag=0;
         std::string operand;  	
	 Value* store_val;
	 for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++)
	 {

		if(!strcmp((i)->getOpcodeName(),"load")){	
			if(flag){
				if(operand.compare(i->getOperand(0)->getName().str())==0){
					//std::cout<<"STORE followed by LOAD on "<<i->getOperand(0)->getName().str()<<"\n";
					LdMap.push_back(i);
					i->replaceAllUsesWith(store_val);
					std::cout<<"%"<<Imap[i]<<" is a useless load"<<"\n";
				}
				flag=0;
				//	Imap.erase(i);
				
			}
		}
		else if(!strcmp((i)->getOpcodeName(),"store")){
		//	std::cout<<"STORE "<<i->getNumOperands()<<i->getOperand(1)->getName().str()<<"\n";
                        store_val = i->getOperand(0);
                        operand = i->getOperand(1)->getName().str();
			flag=1;
		}
		else{
			flag=0;
		}
	}

/*
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
*/	
      }
      int func_flag=0;
      for (std::vector<Instruction *>::iterator it = LdMap.begin(); it != LdMap.end(); ++it){
	(*it)->eraseFromParent();
	//std::cout<< "HI\n";//(*it)->getName().str();
	func_flag=1;
      }
      if(func_flag){
	return true;
      }
      else{
	return false;  // because we have NOT changed this function
     }
    }

    //**********************************************************************
    // print (do not change this method)
    //
    // If this pass is run with -f -analyze, this method will be called
    // after each call to runOnFunction.
    //**********************************************************************
    virtual void print(std::ostream &O, const Module *M) const {
        O << "This is optLoads.\n";
    }

    //**********************************************************************
    // getAnalysisUsage
    //**********************************************************************

    // We don't modify the program, so we preserve all analyses
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      //AU.setPreservesAll();
    };

  };
  char optLoads::ID = 0;
  int optLoads::inst_id = 1;

  // register the optLoads class: 
  //  - give it a command-line argument (optLoads)
  //  - a name ("print code")
  //  - a flag saying that we don't modify the CFG
  //  - a flag saying this is not an analysis pass
  RegisterPass<optLoads> X("optLoads", "optimize unnecessary loads",
			   false, false);
}
