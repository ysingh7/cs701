//===--------------- loopInv.cpp - Project 1 for CS 701 ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a skeleton of an implementation for the loopInv
// pass of Univ. Wisconsin-Madison's CS 701 Project 1.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "loopInv"
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
#include <queue>
#include <stack>
#include "flags.h"
/*
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
*/
#ifdef PRINTDOM
    static const bool PRINT_DOM = true;
#else
    static const bool PRINT_DOM = false;
#endif

using namespace llvm;

namespace {

    class natural_loop {
        public:
            BasicBlock* loop_header;
            int size;
            std::set <BasicBlock*> loop_body;
            natural_loop(const natural_loop &obj) {
            loop_header = obj.loop_header;
            loop_body = obj.loop_body;
            std::cerr<<"Dont\n";
            }
            natural_loop(BasicBlock* h, BasicBlock* n){
                std::cerr<<"Created\n";
                loop_header = h;
                loop_body.insert(n);            
                size = 0;
            }   
            void check(){
                std::cerr<<"Check "<<size<<"\n";
            }     
            void natural_loop_add_existing(BasicBlock* n){
                loop_body.insert(n);            
            }    

    };

    class loopInv : public FunctionPass {

        std::map <Instruction*, int> mapInst;
        std::map <BasicBlock*, int> block_int;

        //dominator set
        std::map <BasicBlock*, std::set<BasicBlock*> > dom_set;

        //dfs block visited or not
        std::map <BasicBlock*, bool> dfs_visited;

        std::queue <BasicBlock*> worklist;
        
        //mapping from basic block to natural loop    
        std::map <BasicBlock*, natural_loop*> loop_map;

        public:
        static char ID; // Pass identification, replacement for typeid
        static int instId;
        loopInv() : FunctionPass(ID) {
        }

        //generating dominator set for the BBs
        void gen_dom(void){
            while(!worklist.empty()){
                BasicBlock* Y = worklist.front();
//                std::cerr<<"Popping from worklist: "<<block_int[Y]<<"\n";
                worklist.pop();
                std::set <BasicBlock*> ndom;
                ndom.empty();
                std::set <BasicBlock*> intersection;
                intersection.empty();
                pred_iterator PI = pred_begin(Y);
                if(PI != pred_end(Y)){ //predecessor exists
                    BasicBlock* tempBB = *PI;
                    intersection.insert( dom_set[tempBB].begin(), dom_set[tempBB].end());               
                    PI++;
                    while(PI != pred_end(Y)){
                        tempBB = *PI;
//                        std::cerr<<"intersection: ";
                        for(std::set<BasicBlock*>::iterator k=intersection.begin();k!=intersection.end();k++){
//                            std::cerr<<"    "<< (*k)->getName().str(); //block_int[*k];
                        }
//                        std::cerr<<"\n";
//                        std::cerr<<"dom set: ";
                        for(std::set<BasicBlock*>::iterator k=dom_set[tempBB].begin();k!=dom_set[tempBB].end();k++){
//                            std::cerr<<"    "<< (*k)->getName().str(); //block_int[*k];
                        }
//                        std::cerr<<"\n";
 
                        set_intersection(intersection.begin(),intersection.end(),dom_set[tempBB].begin(),dom_set[tempBB].end(),std::inserter(ndom,ndom.begin()));                      
//                        std::cerr<<"ndom: ";
                        for(std::set<BasicBlock*>::iterator k=ndom.begin();k!=ndom.end();k++){
//                            std::cerr<<"    "<< (*k)->getName().str(); //block_int[*k];
                        }
//                        std::cerr<<"\n";
 

                        intersection = ndom;
                        PI++;
                    }
                    ndom = intersection;

                }
                ndom.insert(Y);
//                        std::cerr<<"Intersection of "<<block_int[Y]<<"  ";
                        for(std::set<BasicBlock*>::iterator k=ndom.begin();k!=ndom.end();k++){
//                            std::cerr<<"    "<< (*k)->getName().str(); //block_int[*k];
                        }
//                        std::cerr<<"\n";
 

                if(dom_set[Y] != ndom){
                    dom_set[Y] = ndom;
                    for (succ_iterator PI = succ_begin(Y), E = succ_end(Y); PI != E; ++PI) {
                        BasicBlock *succ = *PI;
                        worklist.push(succ);
                    }
                }
            }

        }


        void map_function(Function &F){
            for(Function::iterator iter=F.begin();iter!=F.end();iter++){
                // std::cerr<<"BLOCKS "<<(iter)->getName().str()<<"\n";
                for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++){
                    mapInst[i]=instId++;
                }
            }
        } 

        void find_natural_loops(BasicBlock* root){

                dfs_visited[root] = true;
                for (succ_iterator PI = succ_begin(root), E = succ_end(root); PI != E; ++PI) {
                    BasicBlock *succ = *PI;
                    //std::cerr<<(succ)->getName().str()<<"  ";                    

                    //check if any of the successor dominates this basic block
                    if(dom_set[root].find(succ)!=dom_set[root].end()){
                        std::cerr<<"Backedge found: "<<root->getName().str()<<" and "<<succ->getName().str()<<"\n";
                        
                        //check if header already exists
                        if(loop_map.find(succ)==loop_map.end()){
                            natural_loop loop_new(succ, root);                
                            loop_map[succ] = &loop_new;
                            (loop_map[succ])->check();
                        }
                        else{
                            (loop_map[succ])->natural_loop_add_existing(root);
                        }
                    }                    

                }

                for (succ_iterator PI = succ_begin(root), E = succ_end(root); PI != E; ++PI) {
                    BasicBlock *succ = *PI;
                    //check if node has already been visited in the DFS
                    if(dfs_visited[succ]==false)
                        find_natural_loops(succ);        
                }          

        }

        void sort_loops(std::map<BasicBlock*, natural_loop* > loop_set ){

            for(std::map<BasicBlock*, natural_loop* >::iterator iter = loop_map.begin(), E = loop_map.end(); iter != E ; iter ++){
                std::cerr<<"Value = "<<(iter->second)->size<<"\n";
                (iter->second)->check();
                //iterate through each N of the loop and store the maximum size    
//                for(std::set<BasicBlock*>::iterator i = (loop_set[iter->first])->loop_body.begin(); i!= (loop_set[iter->first])->loop_body.end();i++){
/*
                    int size = 0;
                    std::set <BasicBlock*> loop_body;
                    std::stack <BasicBlock*> temp_stack;
                    loop_body.empty();
                    loop_body.insert(*iter); //body = {H}
                    size++;
                    temp_stack.push(i);
                    while(!temp_stack.empty()){
                        BasicBlock* D = temp_stack.pop();
                        if(loop_body.find(D)==loop_body.end()){
                            loop_body.insert(D);
                            size++;
                            for (pred_iterator PI = pred_begin(D), E = pred_end(D); PI != E; ++PI) {
                                BasicBlock *Pred = *PI;
                                temp_stack.push(Pred);
                            }                           
                        }

                    }
                    std::cerr<<"Loop size = "<<size<<"\n";*/
  //            }

            }


        }
        

        //**********************************************************************
        // runOnFunction
        //**********************************************************************
        virtual bool runOnFunction(Function &F) {
            // print fn name
            std::cerr << "\nFUNCTION " << F.getName().str() << "\n";

            int num_blocks=0;   //number of basic blocks
            int num_passes=0;   //number of passes before solutions of basic blocks converge
            BasicBlock* entryBB = NULL;
            std::set<BasicBlock*> all_nodes;

            //print the successors to debug
            for(Function::iterator iter=F.begin();iter!=F.end();iter++){
                BasicBlock* BB = (BasicBlock*)iter;
                all_nodes.insert(BB);
                std::cerr<<"Basic block  "<<(iter)->getName().str()<<"\n";
                //std::cerr<<"Basic block  "<<(iter)->getName().str()<<" successors: ";
                for (succ_iterator PI = succ_begin(BB), E = succ_end(BB); PI != E; ++PI) {
                    BasicBlock *succ = *PI;
                    //std::cerr<<(succ)->getName().str()<<"  ";                    
                }
                //std::cerr<<"\n";
            }
                        
            for(Function::iterator iter=F.begin();iter!=F.end();iter++){
                //create indices for each basic block
                block_int[(BasicBlock*)iter]=num_blocks;
                BasicBlock *BB;
                BB = (BasicBlock*)iter;

                //initialize the DFS map to fals
                dfs_visited[BB] = false;

                //store the entry Basic Block
                if(entryBB==NULL)
                    entryBB = BB;

                //initialize dom set for each BB with the successors
                for (succ_iterator PI = succ_begin(BB), E = succ_end(BB); PI != E; ++PI) {
                    BasicBlock *succ = *PI;
                    dom_set[BB] = all_nodes;
                }

                num_blocks++;
            }

            //Creating global map for the instructions
            map_function(F);    

            //generating the DOM sets for all the BBs
            worklist.push(entryBB);           
            gen_dom();

            //identifying the backedges
            find_natural_loops(entryBB);    

                for(std::map<BasicBlock*, natural_loop* >::iterator iter = loop_map.begin(), E = loop_map.end(); iter != E ; iter ++){
                std::cerr<<"Value = "<<(iter->second)->size<<"\n";
                (iter->second)->check();
                 }    
            //sort the loops by their sizes
            sort_loops(loop_map);


            std::cout<<"Number of blocks = "<<num_blocks<<"\n";
            
            if(PRINT_DOM){
                for(Function::iterator iter=F.begin();iter!=F.end();iter++)
                {
                    std::cerr<<"BASIC  BLOCK ("<<block_int[(BasicBlock*)iter]<<")"<<(iter)->getName().str()<<"  ";
                    std::cerr<<"Dom Set =";
                    for(std::set<BasicBlock*>::iterator k=dom_set[((BasicBlock*)iter)].begin();k!=dom_set[((BasicBlock*)iter)].end();k++){
                        std::cerr<<"    "<< (*k)->getName().str();//block_int[*k];
                    }
                    std::cerr<<"\n";
                }
            }


            //           for(BasicBlock::iterator i=(*iter).begin();i!=(*iter).end();i++){

            if(1)
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
            O << "This is loopInv.\n";
        }

        //**********************************************************************
        // getAnalysisUsage
        //**********************************************************************

        // We don't modify the program, so we preserve all analyses
        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            //      AU.setPreservesAll();
        };

        };
        char loopInv::ID = 0;
        int loopInv::instId = 1;

        // register the loopInv class: 
        //  - give it a command-line argument (loopInv)
        //  - a name ("print code")
        //  - a flag saying that we don't modify the CFG
        //  - a flag saying this is not an analysis pass
        RegisterPass<loopInv> X("loopInv", "Live vars analysis",
                false, true);
    }
