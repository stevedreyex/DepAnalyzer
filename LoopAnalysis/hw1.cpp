#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <math.h>
#include <stdio.h>

using namespace llvm;

namespace {

struct SolEquStru {
  int is_there_solution;
  int X0;
  int XCoeffT;
  int Y0;
  int YCoeffT;
};

struct TriStru {
  int gcd;
  int x;
  int y;
};

/*

The main subroutine, given a, b,
return x, y, g, where a*x+b*y =g;

*/
struct TriStru Extended_Euclid(int a, int b) {
  struct TriStru Tri1, Tri2;
  if (b == 0) {
    Tri1.gcd = a;
    Tri1.x = 1;
    Tri1.y = 0;
    return Tri1;
  }
  Tri2 = Extended_Euclid(b, (a % b));
  Tri1.gcd = Tri2.gcd;
  Tri1.x = Tri2.y;
  Tri1.y = Tri2.x - (a / b) * Tri2.y;
  return Tri1;
}

int normal_gcd(int a, int b) {
  int c;
  while (a != 0) {
    c = a;
    a = b % a;
    b = c;
  }
  return b;
}

/*
  Solve the diophatine equation by given a,b, and c, where
    a*x+b*y =c

  return

    X= X0 + XCoeffT *t;
    y= Y0 + YCoeffT *t;

*/
SolEquStru diophatine_solver(int a, int b, int c) {
  int k;
  struct TriStru Triple;
  struct SolEquStru s;

  Triple = Extended_Euclid(a, b);
  if ((c % Triple.gcd) == 0) {
    k = c / Triple.gcd;
    s.X0 = Triple.x * k;
    s.XCoeffT = (b / Triple.gcd);
    s.Y0 = Triple.y * k;
    s.YCoeffT = ((-a) / Triple.gcd);
    s.is_there_solution = 1;
  } else
    s.is_there_solution = 0;

  return (s);
}

int Min(int a, int b) {
  if (a <= b) {
    return a;
  } else {
    return b;
  }
}

int Max(int a, int b) {
  if (a >= b) {
    return a;
  } else {
    return b;
  }
}

struct HW1Pass : public PassInfoMixin<HW1Pass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

/*
 * Note: Auto-generated
 * TODO: Cannot find register name like %mul
 */
std::pair<std::string, int> getLLvmValueConst(llvm::Value *val1, llvm::Value *val2){
  std::string str = "";
  int val = 0;
  if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(val1)){
    val = CI->getSExtValue();
  }
  else if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(val1)){
    str = I->getOperand(0)->getName();
  }
  if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(val2)){
    val = CI->getSExtValue();
  }
  else if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(val2)){
    str = I->getOperand(0)->getName();
  }
  return std::make_pair(str, val);
}

PreservedAnalyses HW1Pass::run(Function &F, FunctionAnalysisManager &FAM) {
  // Get loop information by perform LoopAnalysis
  auto &LA = FAM.getResult<LoopAnalysis>(F);
  auto &LAA = FAM.getResult<LoopAccessAnalysis>(F);
  auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  // If there's more than 2 loops then i'm doomed :P
  auto &LL = *LA.begin();
  // Initial string ref store loopBody
  std::string loopBody;
  std::string currentBBName = "";
  // Assume induction variable is i
  int isArrayAccess = 0;
  int cntInductionVar = 0;
  int ub = 0;
  int lb = 0;
  int curLineInfo = 1;
  std::string curInductionVarName = "";
  // Structure store read and write access: spair<array name, vector of coefficient>
  std::vector<std::pair<std::string, std::vector<int>>> readAccess, writeAccess;
  std::pair<std::string, std::vector<int>> curAccess;
  for (auto &BB : F) {
    
    for (auto &I : BB) {
      // Find Loop body name
      currentBBName = BB.getName();

      if (currentBBName == LL->getLoopPreheader()->getName()){
        if (auto *SI = dyn_cast<StoreInst>(&I)) {
          // get induction variable i and its initial value
          if(SI->getPointerOperand()->hasName() && SI->getPointerOperand()->getName() == "i"){
            //get the store value
            if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(SI->getValueOperand())){
              lb = CI->getSExtValue();
            }
          }
        }
      }

      // for cond get upper bound and lower bound
      if (currentBBName == LL->getHeader()->getName()){
        if (auto *BI = dyn_cast<BranchInst>(&I)) {
          // get the successor of the branch, assume there's only one conditional branch that is the loop latch
          if (BI->isConditional()){
            if (BI->getSuccessor(0)->getName() == LL->getLoopLatch()->getName())
              loopBody = BI->getSuccessor(0)->getName();
            else 
              loopBody = BI->getSuccessor(1)->getName();
          }
        }
        // if is slt instruction
        if (auto *CI = dyn_cast<ICmpInst>(&I)) {
          std::pair<std::string, int> indVarAndUb = getLLvmValueConst(CI->getOperand(0), CI->getOperand(1));
          ub = indVarAndUb.second - 1;
        }
      }

      // if this is a load instruction
      if (auto *LI = dyn_cast<LoadInst>(&I)) {
        // get which register instruction load from
        if(LI->getPointerOperand()->hasName() && LI->getPointerOperand()->getName() == "i"){
          cntInductionVar = 1;
          curAccess = std::make_pair(LI->getPointerOperand()->getName(), std::vector<int>{0, 1});
        } else if (isArrayAccess){
          curAccess.second.push_back(curLineInfo);
          readAccess.push_back(curAccess);
          isArrayAccess = 0;
        }
      }

      // else if this is a store instruction
      else if (auto *SI = dyn_cast<StoreInst>(&I)) {
        if (isArrayAccess){
          curAccess.second.push_back(curLineInfo);
          curLineInfo++;
          writeAccess.push_back(curAccess);
          isArrayAccess = 0;
        }
      }
      // else if this is a array access instruction
      else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
        // get the array name
        if (GEP->getPointerOperand()->hasName()){
          curAccess.first = GEP->getPointerOperand()->getName();
          isArrayAccess = 1;
        }
      }
      //else if this is add /sub /mul /div instruction
      else if (auto *BI = dyn_cast<BinaryOperator>(&I)) {
        // if this is an add
        if (BI->getOpcode() == Instruction::Add) {
          std::pair<std::string, int> curInductionVarNamePair = getLLvmValueConst(BI->getOperand(0), BI->getOperand(1));
          curAccess.second[0] += curInductionVarNamePair.second;
        }
        // if this is a sub
        else if (BI->getOpcode() == Instruction::Sub) {
          std::pair<std::string, int> curInductionVarNamePair = getLLvmValueConst(BI->getOperand(0), BI->getOperand(1));
          curAccess.second[0] -= curInductionVarNamePair.second;
        }
        // if this is a mul
        else if (BI->getOpcode() == Instruction::Mul) {
          std::pair<std::string, int> curInductionVarNamePair = getLLvmValueConst(BI->getOperand(0), BI->getOperand(1));
          curAccess.second[0] *= curInductionVarNamePair.second;
          curAccess.second[1] *= curInductionVarNamePair.second;
        }
      }
    }
  }
  /* End of Parse */

  // Start to solve the diophatine equation
  // Assume there's only one induction variable
  // Assume the induction variable is i
  // Assume the induction variable is in the form of i = a + b*t
  std::vector<std::string> flowOutput;
  std::vector<std::string> antiOutput;

  // W & R
  for (auto write : writeAccess){
    for (auto read : readAccess){
      if (read.first == write.first) {
        SolEquStru f = diophatine_solver(write.second[1], -1*read.second[1], read.second[0] - write.second[0]);
        if (f.is_there_solution){
          int rtub, rtlb, wtub, wtlb;
          if (f.XCoeffT > 0){
            rtub = (ub - f.X0) / f.XCoeffT;
            rtlb = (lb - f.X0) / f.XCoeffT;
          }
          else {
            rtub = (lb - f.X0) / f.XCoeffT;
            rtlb = (ub - f.X0) / f.XCoeffT;
          }
          if (f.YCoeffT > 0){
            wtub = (ub - f.Y0) / f.YCoeffT;
            wtlb = (lb - f.Y0) / f.YCoeffT;
          }
          else {
            wtub = (lb - f.Y0) / f.YCoeffT;
            wtlb = (ub - f.Y0) / f.YCoeffT;
          }
          // intersect the range
          int tub_intersect = Min(rtub, wtub);
          int tlb_intersect = Max(rtlb, wtlb);
          for (int a = tlb_intersect; a <= tub_intersect; a++){
            int read_index = f.YCoeffT * a + f.Y0;
            int write_index = f.XCoeffT * a + f.X0;
            if (write_index < read_index || (write_index==read_index && write.second[2] < read.second[2])){
              flowOutput.push_back("(i=" + std::to_string(write_index) + ",i="+std::to_string(read_index)+")\n" + write.first.c_str() + ":S" + std::to_string(write.second[2]) + " -----> S" + std::to_string(read.second[2]) + "\n");
            } else {
              antiOutput.push_back("(i=" + std::to_string(read_index) + ",i="+std::to_string(write_index)+")\n" + write.first.c_str() + ":S" + std::to_string(read.second[2]) + " --A--> S" + std::to_string(write.second[2]) + "\n");
            }
            // errs() << "X: " << f.XCoeffT * a + f.X0 << " Y: "<< f.YCoeffT * a + f.Y0 <<'\n';
          }
        }
      }
    }
  }
  errs() << "====Flow Dependency====\n";
  for(auto &str : flowOutput){
    errs() << str;
  }
  std::reverse(antiOutput.begin(), antiOutput.end());
  errs() << "====Anti-Dependency====\n";
  for(auto &str : antiOutput){
    errs() << str;
  }

  std::vector<std::string> outputOutput;
  // Write and Write, use the above pattern
  for (auto write1 = writeAccess.begin(); write1 != writeAccess.end(); write1++){
    for (auto write2 = std::next(write1); write2 != writeAccess.end(); write2++){
      if (write1->first == write2->first){
        SolEquStru f = diophatine_solver(write1->second[1], -1 * write2->second[1], write2->second[0] - write1->second[0]);
        if (f.is_there_solution){
          int rtub, rtlb, wtub, wtlb;
          if (f.XCoeffT > 0){
            rtub = (ub - f.X0) / f.XCoeffT;
            rtlb = (lb - f.X0) / f.XCoeffT;
          }
          else {
            rtub = (lb - f.X0) / f.XCoeffT;
            rtlb = (ub - f.X0) / f.XCoeffT;
          }
          if (f.YCoeffT > 0){
            wtub = (ub - f.Y0) / f.YCoeffT;
            wtlb = (lb - f.Y0) / f.YCoeffT;
          }
          else {
            wtub = (lb - f.Y0) / f.YCoeffT;
            wtlb = (ub - f.Y0) / f.YCoeffT;
          }
          // intersect the range
          int tub_intersect = Min(rtub, wtub);
          int tlb_intersect = Max(rtlb, wtlb);
          
          for (int a = tlb_intersect; a <= tub_intersect; a++){
            int write1_index = f.YCoeffT * a + f.Y0;
            int write2_index = f.XCoeffT * a + f.X0;
            if (write1_index > write2_index){
              outputOutput.push_back("(i=" + std::to_string(write2_index) + ",i="+std::to_string(write1_index)+")\n" + write2->first.c_str() + ":S" + std::to_string(write1->second[2]) + " --O--> S" + std::to_string(write2->second[2]) + "\n");
            }

          }
        }
      }
    }
  }
  errs() << "====Output Dependency====\n";
  for (auto &str : outputOutput){
    errs() << str;
  }

  return PreservedAnalyses::all();
  }

} // end anonymous namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HW1Pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "hw1") {
                    FPM.addPass(HW1Pass());
                    return true;
                  }
                  return false;
                });
          }};
}
