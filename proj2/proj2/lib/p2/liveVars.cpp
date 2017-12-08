//===--------------- liveVars.cpp - Project 1 for CS 701 ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a skeleton of an implementation for the liveVars
// pass of Univ. Wisconsin-Madison's CS 701 Project 1.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "liveVars"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Statistic.h"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include "llvm/Support/CFG.h"
#include <map>
#include "flags.h"
#ifdef PRINTANALRESULTS
    static const bool PRINT_ANAL_RESULTS = true;
#else
    static const bool PRINT_ANAL_RESULTS = false;
#endif
#ifdef PRINTREMOVING
    static const bool PRINT_REMOVING = true;
#else
    static const bool PRINT_REMOVING = false;
#endif

STATISTIC(RemovedInstructionCount, "Number of useless assignments found.");

using namespace llvm;

namespace {
    class liveVars : public FunctionPass {
        std::map <Instruction*, int> mapInst;
        public:
        static char ID; // Pass identification, replacement for typeid
        static int instId;
        liveVars() : FunctionPass(ID) {
        }
        void map_function(Function &F){
            for(Function::iterator iter=F.begin();iter!=F.end();iter++){
                // std::cerr<<"BLOCKS "<<(iter)->getName().str()<<"\n";
                for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++){
                    mapInst[i]=instId++;
                }
            }
        } 
        //**********************************************************************
        // runOnFunction
        //**********************************************************************
        virtual bool runOnFunction(Function &F) {
            // print fn name
            //std::cerr << "FUNCTION " << F.getName().str() << "\n";

            int num_blocks=0;   //number of basic blocks
            int num_passes=0;   //number of passes before solutions of basic blocks converge
            std::map <BasicBlock*, int> block_int;
            //std::cerr <<"Calculating number of blocks ...\n";
            for(Function::iterator iter=F.begin();iter!=F.end();iter++){
                block_int[(BasicBlock*)iter]=num_blocks;
                num_blocks++;
            }
            //std::cerr <<"Number of blocks ="<<num_blocks<<"\n";

            std::vector <std::set<Instruction*> > BB_gen(num_blocks);
            std::vector <std::set<Instruction*> > BB_kill(num_blocks);
            std::vector <std::set<Instruction*> > BB_lbefore(num_blocks);
            std::vector <std::set<Instruction*> > BB_lafter(num_blocks);
            std::vector <std::set<Instruction*> > BB_lbefore_old(num_blocks);
            std::vector <std::set<Instruction*> > BB_lafter_old(num_blocks);
            std::set<Instruction*> temp_set;
            std::set<Instruction*> gen_set;
            std::set<Instruction*> useless_Inst;

            std::map <Instruction*, std::set<Instruction*> > inst_lbefore;
            std::map <Instruction*, std::set<Instruction*> > inst_lafter;

            std::set <Instruction*> inst_remove;

            //Creating global map for the instructions
            map_function(F);    

            //std::cerr << "Generating GEN and KILL sets for the basic blocks\n";

            int block_ind = num_blocks-1;
            Function::iterator iter = F.end();
            while(iter != F.begin()){            
                iter--;    
                //            int num_inst=0;
                int flag;
                for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++){

                    //check if this instruction is already in the GEN set of the block
                    if(BB_gen[block_ind].find((Instruction*)(i))!=BB_gen[block_ind].end()){
                        flag=0;
                    }
                    else{
                        flag=1;
                    }
                    for(int j=0; j< i->getNumOperands(); j++){
                        if(isa<Instruction>(i->getOperand(j))){
                            if(BB_kill[block_ind].find((Instruction*)(i->getOperand(j)))==BB_kill[block_ind].end()){
                                BB_gen[block_ind].insert((Instruction*)(i->getOperand(j)));
                            }
                        }
                    }   

                    //trial
                    if(BB_gen[block_ind].find((Instruction*)(i))!=BB_gen[block_ind].end()){
                        flag=0;
                    }
 
                    //if instruction not already in the GEN set then insert it into KILL set                        
                    if(flag){
                        BB_kill[block_ind].insert(i);
                    }

                    // num_inst++;

                }
                //            std::cerr<<"Number of instructions in block= "<<num_inst<<", in Kill set = "<<BB_kill[block_ind].size()<<"\n";
                //            std::cerr<<"Number of instructions in Gen set = "<<BB_gen[block_ind].size()<<"\n";
                block_ind--;

            }

            while(1){
                //std::cerr << "\n\n";
                //std::cerr << "Computing LiveBefore and LiveAfter sets for the basic blocks\n";
                block_ind = num_blocks-1;
                iter = F.end();
                while(iter != F.begin()){            
                    iter--;    
                    BasicBlock *BB;
                    BB = (BasicBlock*)iter;
                    BB_lafter[block_int[BB]].clear();

                    for (succ_iterator PI = succ_begin(BB), E = succ_end(BB); PI != E; ++PI) {
                        BasicBlock *succ = *PI;
                        BB_lafter[block_int[BB]].insert(BB_lbefore[block_int[succ]].begin(),BB_lbefore[block_int[succ]].end());
                        //std::cerr<<"Predecessor is : "<<Pred->getName().str()<<"\n";
                    }
                    BB_lbefore[block_int[BB]].clear();
                    BB_lbefore[block_int[BB]].insert( BB_lafter[block_int[BB]].begin(), BB_lafter[block_int[BB]].end());
                    BB_lbefore[block_int[BB]].insert( BB_gen[block_int[BB]].begin(), BB_gen[block_int[BB]].end());
                    temp_set.clear();
                    std::set_difference(BB_lbefore[block_int[BB]].begin(),BB_lbefore[block_int[BB]].end(),BB_kill[block_int[BB]].begin(),BB_kill[block_int[BB]].end(),std::inserter(temp_set,temp_set.begin()));
                    BB_lbefore[block_int[BB]].clear();
                    BB_lbefore[block_int[BB]].insert(temp_set.begin(),temp_set.end());

                    //std::cerr<<"Number of entries in liveAfter set= "<<BB_lafter[block_int[BB]].size()<<"\n";
                    //std::cerr<<"Number of entries in liveBefore set= "<<BB_lbefore[block_int[BB]].size()<<"\n";
                    block_ind--;

                }
                num_passes++;
                if(BB_lafter_old==BB_lafter){
                    //std::cerr<<"\n\nSame as previous pass\n";
                    //std::cerr<<"Total passes = "<<num_passes<<"\n\n\n";
                    break;
                }
                else{
                    BB_lafter_old = BB_lafter;
                }
            }

            //std::cerr << "Computing LiveBefore and LiveAfter sets for the instructions\n";
            iter = F.end();
            Instruction* succ_inst;
            while(iter != F.begin()){            
                iter--;    
                BasicBlock *BB;
                BB = (BasicBlock*)iter;

                int num_inst=1;

                BasicBlock::iterator i = (*BB).end();
                while(i != (*BB).begin()){
                    i--;
                    if(num_inst==1){ //If this is the last instruction of the block then use the lAfter of the basic block
                        inst_lafter[i].insert(BB_lafter[block_int[BB]].begin(),BB_lafter[block_int[BB]].end());                
                        //std::cerr<<"Content of BB = "<<BB_lafter[block_int[BB]].size();
                    }
                    else{ //Else copy the lBefore of the successor's instruction
                        inst_lafter[i].insert(inst_lbefore[succ_inst].begin(),inst_lbefore[succ_inst].end());                
                    } 
                    
                    int flag = 1;
                    //store the operands of the inst
                    for(int j=0; j< i->getNumOperands(); j++){                                                
                        //check if the operand is Inst before inserting
                        if(isa<Instruction>(i->getOperand(j))){              
                            inst_lbefore[i].insert((Instruction*)(i->getOperand(j)));
                            //random check
                            if(((Instruction*)(i->getOperand(j)))==(i))
                                flag = 0;
                        }
                    }   

                    //merge the entries with the lAfter set
                    inst_lbefore[i].insert(inst_lafter[i].begin(),inst_lafter[i].end());

                    //erase the Kill set element, which is the instruction itself
                    if(flag)
                        inst_lbefore[i].erase(i);                

                    //store the instruction to be used as the successor in the next iteration
                    succ_inst = i;

                    //std::cerr<<"\n"<<num_inst<<" number of elements= "<< inst_lafter[i].size()<<"\n";
                    num_inst++;
                }
            }

            int num_inst=1;
            //std::cerr << "Identifying useless assignments\n";
            for(Function::iterator iter=F.begin();iter!=F.end();iter++){

                //std::cerr<<"\nBASIC  BLOCK "<<(iter)->getName().str()<<"\n";
                for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++){   
                    //Identify the useless assignment
                    if(inst_lafter[i].find(i)==inst_lafter[i].end()){
                        if((*i).isBinaryOp()||(*i).isShift()||(*i).isCast()){
                            inst_remove.insert(i);
                            //std::cerr<<"removing useless assignment %"<<mapInst[i]<<"\n";                    
                            RemovedInstructionCount++;
                        }
                        else{
                            int Op = (*i).getOpcode();
                            if((Op==26)||(Op==27)||(Op==29)||(Op==49)||(Op==53)||(Op==56)||(Op==45)||(Op==46)){
                                inst_remove.insert(i);
                                RemovedInstructionCount++;
                                //std::cerr<<"removing useless assignment %"<<mapInst[i]<<"\n";
                            }                
                        }
                    }
                    num_inst++;   
                }
            }

            //Debug output if Flag passes
            if(PRINT_ANAL_RESULTS){
                std::cerr<<"\nFUNCTION "<<F.getName().str()<<"\n";
                for(Function::iterator iter=F.begin();iter!=F.end();iter++){

                    std::set<int> print_inst;

                    std::cerr<<"\nBASIC BLOCK "<<(iter)->getName().str();//<<" L-Before: { }  L-After: { }\n";
                    std::cerr<<"  L-Before: {";
                    print_inst.clear();
                    for(std::set<Instruction*>::iterator k=BB_lbefore[block_int[(BasicBlock*)iter]].begin();k!=BB_lbefore[block_int[(BasicBlock*)iter]].end();k++){
                        //std::cerr<<" %"<<mapInst[*k]<<" ";
                        print_inst.insert(mapInst[*k]);
                    }
                    for(std::set<int>::iterator k=print_inst.begin();k!=print_inst.end();k++){
                        std::cerr<<" %"<<*k;
                    }
                    std::cerr<<" } L-After: { ";

                    print_inst.clear();
                    for(std::set<Instruction*>::iterator k=BB_lafter[block_int[(BasicBlock*)iter]].begin();k!=BB_lafter[block_int[(BasicBlock*)iter]].end();k++){
                        //std::cerr<<"%"<<mapInst[*k]<<" ";
                        print_inst.insert(mapInst[*k]);
                    }
                    for(std::set<int>::iterator k=print_inst.begin();k!=print_inst.end();k++){
                        std::cerr<<" %"<<*k;
                    }
                    std::cerr<<" }\n";

                    for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++){   
                        std::cerr<<"%"<<mapInst[(Instruction*)i]<<":   ";
                        std::cerr<<"L-Before: {";
            
                        print_inst.clear();
                        for(std::set<Instruction*>::iterator k=inst_lbefore[(Instruction*)i].begin();k!=inst_lbefore[(Instruction*)i].end();k++){
                            //std::cerr<<" %"<<mapInst[*k]<<" ";
                            print_inst.insert(mapInst[*k]);
                        }
                        for(std::set<int>::iterator k=print_inst.begin();k!=print_inst.end();k++){
                            std::cerr<<" %"<<*k;
                        }
                        std::cerr<<" } L-After: { ";
                        print_inst.clear();
                        for(std::set<Instruction*>::iterator k=inst_lafter[(Instruction*)i].begin();k!=inst_lafter[(Instruction*)i].end();k++){
                            //std::cerr<<"%"<<mapInst[*k]<<" ";
                            print_inst.insert(mapInst[*k]);
                        }
                        for(std::set<int>::iterator k=print_inst.begin();k!=print_inst.end();k++){
                            std::cerr<<" %"<<*k;
                        }
 
                        std::cerr<<" }\n";
                    }

                }
            }
            //print useless assignments if Flag passed 
            if(PRINT_REMOVING){
                for(std::set<Instruction*>::iterator iter=inst_remove.begin();iter!=inst_remove.end();iter++){
                    std::cerr<<"removing useless assignment %"<<mapInst[*iter]<<"\n";                    
                }
            }

            int changed=0;
            //remove useless assignments by erasing the nodes from the parent
            for(std::set<Instruction*>::iterator iter=inst_remove.begin();iter!=inst_remove.end();iter++){
                  	(*iter)->eraseFromParent();
                    changed = 1;
            }
 
            /*

               for(Function::iterator iter=F.end();iter!=F.begin();iter--){
               BasicBlock *BB;
               BB = (BasicBlock*)iter;

               std::cerr<<"\n";
               std::cerr<<(iter)->getName().str()<<" : "<<"\n";
               for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) {
               BasicBlock *Pred = *PI;
               std::cerr<<"Predecessor is : "<<Pred->getName().str()<<"\n";
               }
               std::cerr<<"\n";
               }
               */

            if(changed)
                return true;   // if we have erased one or more nodes
            else
                return false;  // if we have NOT changed this function
        }

        //**********************************************************************
        // print (do not change this method)
        //
        // If this pass is run with -f -analyze, this method will be called
        // after each call to runOnFunction.
        //**********************************************************************
        virtual void print(std::ostream &O, const Module *M) const {
            O << "This is liveVars.\n";
        }

        //**********************************************************************
        // getAnalysisUsage
        //**********************************************************************

        // We don't modify the program, so we preserve all analyses
        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            //      AU.setPreservesAll();
        };

    };
            char liveVars::ID = 0;
            int liveVars::instId = 1;

            // register the liveVars class: 
            //  - give it a command-line argument (liveVars)
            //  - a name ("print code")
            //  - a flag saying that we don't modify the CFG
            //  - a flag saying this is not an analysis pass
            RegisterPass<liveVars> X("liveVars", "Live vars analysis",
                    false, true);
    }
