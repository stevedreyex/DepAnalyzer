#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
enum class varType { i32, ptr };

namespace {

std::string extractName(llvm::Value *val){
  std::string str = "";

  // if val->getname is not empty jus ret
  if (!val->getName().empty()){
    return val->getName().str();
  }
  
  if (llvm::ConstantInt *CI = llvm::dyn_cast<llvm::ConstantInt>(val)){
    str = std::to_string(CI->getSExtValue());
  }
  else if (llvm::Instruction *I = llvm::dyn_cast<llvm::Instruction>(val)){
    str = I->getOperand(0)->getName();
  }
  return str;
}




struct HW2Pass : public PassInfoMixin<HW2Pass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


bool isVal(const std::string& str) {
    for (char ch : str) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }
    return true;
}

bool isSubstring(const std::string& str, const std::string& substring) {
    return str.find(substring) != std::string::npos;
}

bool compareByLength(const std::list<std::string>* a, const std::list<std::string>* b) {
    return a->size() > b->size();
}

std::string repeatString(const std::string& str, int n) {
    std::string result;
    for (int i = 0; i < n; ++i) {
        result += str;
    }
    return result;
}

void trefDump(std::set<std::string> &TREF) {
  errs() << "TREF: {";
  int elem = 0;
  for (auto &v : TREF) {
    if (elem != 0) {
      errs() << ", ";
    }
    errs() << v;
    elem++;
  } errs() << "}\n";
}

void tgenDump(std::set<std::string> &TGEN) {
  int elem = 0;
  errs() << "TGEN: {";
  for (auto &v : TGEN) {
    if (elem != 0) {
      errs() << ", ";
    }
    errs() << v;
    elem++;
  } errs() << "}\n";
} 

void tdefDump(std::map<std::string, int> &TDEF) {
  int elem = 0;
  errs() << "TDEF: {";
  // sort by value first
  // std::sort(TDEF.begin(), TDEF.end(), [](const auto &a, const auto &b) {
  //   return a.second < b.second;
  // });
  for (auto &v : TDEF) {
    if (elem != 0) {
      errs() << ", ";
    }
    errs() << "(" << v.first << ", S" << v.second << ")";
    elem++;
  } errs() << "}\n";
}

void star_dump(int num){
  for (int i = 0; i < num; i++){
    errs() << "*";
  }
}

/* if list size > 1, then it has equiv 
 * (n-1) stars the first element with the nth element
 * if element is value do not show
 */
void tequivDump(std::vector<std::list<std::string> *> &forest) {
  int has_equiv = 0;
  errs() << "TEQUIV: {";
  for (auto &t : forest) {
    if (t->size() > 1) {
      int i = 1;
      for (auto &v : *t) {
        if (v != t->front() && !isVal(v) && !isSubstring(v, "add")) {
          has_equiv = 1;
          errs() << "(";
          star_dump(i);
          errs() << t->front() << ", " << v;
          errs() << "), ";
          i++;
        }
      }
    }
  }
  if (has_equiv == 1)
    errs() << "\b\b}\n";
  else
    errs() << "}\n";
}

void depDump(std::vector<std::string> &DEP) {
  if (DEP.size() == 0){
    errs() << "DEP: {}\n";
    return;
  }
  errs() << "DEP: {\n";
  for (auto &v : DEP) {
    errs() << "    " << v << '\n';
  } errs() << "}\n";
}

void treeDump(std::vector<std::list<std::string> *> &forest) {
  for (auto &t : forest) {
    errs() << "Tree: ";
    for (auto &v : *t) {
      errs() << v << " -> ";
    }
    errs() << "NULL\n";
  }
}

void derefCntDump(std::map<std::string, int> &derefCnt){
  if (derefCnt.size() == 0)
    return;
  errs() << "derefCnt: {";
  for (auto &p : derefCnt){
    errs() << "(" << p.first << ", " << p.second << "), ";
  }
  errs() << "\b\b}\n";
}

int find_tree(std::vector<std::list<std::string> *> &forest, std::string &name){
  for (int i = 0; i < forest.size(); i++){
    if (forest[i]->front() == name){
      return i;
    }
  }
  return -1;
}

std::string find_father(std::vector<std::list<std::string> *> &forest, std::string &name){
  for (auto &t : forest){
    // if the second elem of list is name, then the first elem is its father
    if (t->size() > 1 && *(std::next(t->begin(), 1)) == name){
      return t->front();
    }
  }
  return "";
}

std::string find_child(std::vector<std::list<std::string> *> &forest, std::string &name){
  for (auto &t : forest){
    // if the first elem of list is name, then the second elem is its child
    if (t->front() == name){
      return *(std::next(t->begin(), 1));
    }
  }
  return "";
}

/*
 * Find the dereference count of the name
 * If not in derefCnt, return name
 */
std::string find_derefence(std::map<std::string, int> &derefCnt, std::vector<std::list<std::string> *> &forest, std::string &name, int is_rhs){
  for (auto &p : derefCnt){
    if (p.first == name){
      for (auto &t : forest){
        if (t->front() == name){
          if (t->size() > 1){
            // errs() << "p.second: " << p.second << '\n';
            return *(std::next(t->begin(), p.second - is_rhs));
          }
        }
      }
    }
  }
  return name;
}

PreservedAnalyses HW2Pass::run(Function &F, FunctionAnalysisManager &FAM) {
  // errs() << "[HW2]: " << F.getName() << '\n';
  // Create Set TREF, TGEN, DEP, TDEF and TEQUIV
  std::set<std::string> TREF, TGEN;
  std::vector<std::string> DEP;
  std::map<std::string, int> TDEF, TDEF_prev;
  // Map for variable and its type
  std::map<std::string, varType> varMap;
  std::pair<std::string, std::string> valPair;
  std::vector<std::string> IN;
  std::vector<std::list<std::string> *> forest; // is TEQUIV
  std::list<std::string> *tree;
  // std::vector<std::pair<std::string, int>> derefCnt;
  std::map<std::string, int> derefCnt;
  std::string tempFirstName = "", tempSecondName = "", lastName = "";
  int proceedLHS = 0;
  int currentStmt = 0;

  for (auto &B : F) {
    for (auto &I : B) {

      /* Since TGENi is the TKILL, operate the TGENi with TREF
        */
      std::vector<std::string> TKILL;
      for (auto &v : TGEN) {
        for (auto & w : TREF) {
          if (isSubstring(v, w)) {
            TKILL.push_back(w);
          }
        }
      } 
      for (auto &v : TKILL) {
        TREF.erase(v);
      }

      TGEN.clear();
      // is alloca
      if (auto AI = dyn_cast<AllocaInst>(&I)) {
        // errs() << "alloca: " << *AI << '\n';
        // if is ptr type
        if (AI->getAllocatedType()->isPointerTy()) {
          varMap[AI->getName().str()] = varType::ptr;
          // errs() << "ptr: " << AI->getName() << '\n';
          /* Create the tree structure such as in powerpoint p.343
           * But is kind of like a list, and the first elem is the frontire (leaf)
           */
          tree = new std::list<std::string>();
          tree->push_back(AI->getName().str());
          forest.push_back(tree);
        } else {
          varMap[AI->getName().str()] = varType::i32;
          // errs() << "i32: " << AI->getName() << '\n';
        }
      }
      // Load Instruction
      if (auto LI = dyn_cast<LoadInst>(&I)) {
        // load from which var to which reg
        // errs() << "load " << extractName(LI->getOperand(0)) << " to "
        //        << LI->getValueName() << '\n';
        std::string loadName = extractName(LI->getOperand(0));
        IN.push_back(loadName);
        TREF.insert(loadName); 
        std::string setElem = "*" + find_father(forest, loadName);
        auto it = TREF.find(setElem);
        if (it != TREF.end()){
          TREF.insert("**" + find_father(forest, loadName));
          TREF.erase(setElem);
        } else {
          auto it = TREF.find("**" + find_father(forest, loadName));
          if (setElem != "*" && it == TREF.end()) {
          TREF.insert(setElem);
          }
        }
        
        varType loadType = varMap[loadName];
        if (loadType == varType::i32 || isVal(loadName)) {
          // end of dereference
        } 
        else { // loadType == varType::ptr
          // keep dereferencing
          // find in derefCnt, pair.first == loadName
          // std::vector<std::pair<std::string, int>>::iterator it = find_if(derefCnt.begin(), derefCnt.end(), [loadName](const std::pair<std::string, int> &p) { return p.first == loadName; });
          std::map<std::string, int>::iterator it = derefCnt.find(loadName);
          if (it != derefCnt.end()) {
            // it->second++;
            derefCnt[loadName]++;

            // errs() << "last: " << lastName << " loadName: " << loadName << '\n';
            if(tempFirstName == "" && lastName == loadName && proceedLHS == 0){
              // errs() << "tempFirstName: " << loadName << '\n';
              tempFirstName = loadName;
            } else if (tempFirstName != "" && lastName == loadName){
              proceedLHS = 1;
              // errs() << "Change to LHS\n" << '\n';
            }
            if(tempSecondName == "" && lastName == loadName && proceedLHS == 1){
              // errs() << "tempSecondName: " << loadName << '\n';
              tempSecondName = loadName;
            }
          } else{
            derefCnt[loadName] = 1;
          }
          lastName = loadName;
        }
      }
      // Store Instruction
      if (auto SI = dyn_cast<StoreInst>(&I)) {
        // valPair = getLLvmValueConst(SI->getPointerOperand(), SI->getOperand(0)); -> (p,3)

        // A Statement, dump the current Set info
        currentStmt++;
        
        /*
         * At CS5404_CH3_2, there are 2 cases but i only see p=&x case
         * in .ll which is  `store ptr %x, ptr %p, align 8`
         * So I assumed RHS is always i32 type
         */
        std::string lhsName, rhsName;
        if (tempSecondName != ""){

        // *p is not the frontier of p, and is the proper subtree of p
          lhsName = tempSecondName;
          TREF.insert("*"+lhsName);
        }
        else
          lhsName = extractName(SI->getPointerOperand());
        
        if (tempFirstName != "" && tempSecondName != ""){
          // load a ptr case like p in x = *p, generate ref of p and *p
          rhsName = tempFirstName;
          TREF.insert("*"+rhsName);
          TREF.insert(find_child(forest, rhsName));
          TREF.insert(rhsName);  
        }
        else if ( tempFirstName != "" && tempSecondName == ""){
          lhsName = tempFirstName;
          rhsName = extractName(SI->getOperand(0));
          TREF.insert("*"+lhsName);
          TREF.insert(find_child(forest, lhsName));
          TREF.insert(lhsName);
        }
        else
          rhsName = extractName(SI->getOperand(0)); 


        
        /* Here update the TGEN by derefCnt and forest
         */
        // Find if the lhsName is in derefCnt
        int derefenceAdded = 0;
        for (auto &p : derefCnt){
          if (p.first == lhsName){
            std::string setElem = repeatString("*", p.second) + lhsName;
            TGEN.insert(setElem);
            TDEF[setElem] = currentStmt;
            setElem = find_derefence(derefCnt, forest, lhsName, 0);
            TGEN.insert(setElem);
            TDEF[setElem] = currentStmt;
            derefenceAdded = 1;
          }
        }
        if (derefenceAdded == 0){
          TDEF[lhsName] = currentStmt;
          TGEN.insert(extractName(SI->getPointerOperand()));
        }

        

        /* End of update TGEN */

        /* After the computation of TGEN and TREF and TDEF_prev
         * We could finally calculate the DEP
         */
        for (auto &v : TDEF_prev){
          for (auto &w : TREF){
            if (isSubstring(w, "[*") && w.substr(1, w.size() - 2) == v.first){
              std::string setElem = "[" +w.substr(1, w.size() - 2) + ": ";
              setElem += "S" + std::to_string(v.second) + "--->S" + std::to_string(currentStmt) + "]";
              DEP.push_back(setElem);
            } else if (w == v.first) {
              std::string setElem =  w + ": ";
              setElem += "S" + std::to_string(v.second) + "--->S" + std::to_string(currentStmt);
              DEP.push_back(setElem);
            }
          }
          for (auto &w : TGEN){
            if (w == v.first) {
              std::string setElem =  w + ": ";
              setElem += "S" + std::to_string(v.second) + "-O->S" + std::to_string(currentStmt);
              DEP.push_back(setElem);
            }
          }
        }

        TDEF_prev = TDEF;
        
        varType rhsType = varMap[rhsName];
        varType lhsType = varMap[lhsName];
        if (rhsType == varType::i32) {
          // since write val to ptr has ptr access, create/update tree
          // std::list<std::string> *tree = new std::list<std::string>();
          // tree->push_back(lhsName);
          // tree->push_back(rhsName);

          /* Find if there is a tree that its first elem is rhsName
           * If so, update the tree
           * If not, create a new tree
           */
          
        } 
        else  { // rhsType == varType::ptr
          /*
          * CS5404_CH3_2 p.9, If any element of TGEN(S) is a proper subtree
          * of the element in EQUIV_IN(S), the pair is removed
          * NOTE: Ptr and Ptr forms a transitive tree structure
          * Also, since it's transitive, should add the son and grandparent
          * WARNING: This is not a good impl, can only handel 2 level ptr 2 ptr
          * TODO: impl the tree structure (maybe we should regard all ptrs as tree?) 
          * could use pair<string ROOT, list CHILD>
          */
          
          // errs() << "erase " << rhsName << '\n';
          TGEN.erase(rhsName);

        }
          /* Tree update phase
           */
          auto derefRHS = find_derefence(derefCnt, forest, rhsName, 1);
          int tree_idx = find_tree(forest, derefRHS);
          if (tree_idx == -1){
            tree = new std::list<std::string>();
            tree->push_back(rhsName);
            forest.push_back(tree);
            tree_idx = forest.size() - 1;
          } 
          // errs() << "tree_idx: " << tree_idx << '\n';
          // else we find the update tree


        // errs() << "store " << rhsName << " to "
              //  << lhsName << '\n';

          // SHOULD FIRST CHECK WHERE IS THE DESTINATION!!!!

          

          std::list<std::string> *updateTree = forest[tree_idx];
          std::string his_father = find_derefence(derefCnt, forest, lhsName, 0);
          int has_update = 1;
          if (derefRHS == his_father)
            goto updated;
          // errs() << "rhs write to: " << his_father << '\n';
          for (int i = 0; i < 4; i++){
            has_update = 0;
            for (auto curTree : forest){
              if (curTree->front() == his_father){
                if (curTree->size() != 1) {
                  curTree->clear();
                  curTree->push_back(his_father);
                  // push back the rest of update tree
                  for (auto &leaf : *updateTree){
                    curTree->push_back(leaf);
                  }
                } else{
                  // is not the real value then perform update
                  if (!isVal(his_father)){
                    for (auto &leaf : *updateTree){
                      curTree->push_back(leaf);
                    }
                  }
                }
                his_father = find_father(forest, curTree->front());
                updateTree = curTree;
                has_update = 1;
              } 
            } 
          }

updated:

        errs() << "S" << currentStmt << ":--------------------\n";
        trefDump(TREF);
        tgenDump(TGEN);
        // May impl Later
        depDump(DEP);
        tdefDump(TDEF);
        // The tree visualizer:
        std::sort(forest.begin(), forest.end(), compareByLength);

        tequivDump(forest);
        // treeDump(forest);

        IN.clear();
        DEP.clear();
        TREF.clear();
        derefCnt.clear();
        tempSecondName = ""; 
        tempFirstName = "";
        // errs() << "Sep--------------------" << '\n';
        errs() <<  '\n';
      }
    }
  }
  return PreservedAnalyses::all();
}

} // end anonymous namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HW2Pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "hw2") {
                    FPM.addPass(HW2Pass());
                    return true;
                  }
                  return false;
                });
          }};
}
