/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iomanip>
#include <memory>

#include "ir/module-utils.h"
#include "ir/utils.h"
#include "pass.h"
#include "wasm.h"

namespace wasm {

struct PrintCallGraphEdges : public Pass {
  bool modifiesBinaryenIR() override { return false; }

  void run(PassRunner* runner, Module* module) override {

    struct CallPrinter : public PostWalker<CallPrinter> {
      Module* module;
      Function* currFunction;
      std::map<Function*,Index> funcMap;
      std::set<Name> visitedTargets; // Used to avoid printing duplicate edges.
      std::vector<Function*> allIndirectTargets;
      CallPrinter(Module* module) : module(module) {

        Index i = 0;
        for (auto& func : module->functions) {
          funcMap.insert({func.get(), i++});
        }        

        // Walk function bodies.
        ModuleUtils::iterDefinedFunctions(*module, [&](Function* curr) {
          currFunction = curr;
          visitedTargets.clear();
          walk(curr->body);
        });

      }
      void visitCall(Call* curr) {
        auto* target = module->getFunction(curr->target);
        if (visitedTargets.count(target->name) > 0) {
          return;
        }
        visitedTargets.insert(target->name);
        // std::cout << currFunction->name << "," << target->name << "\n";
        std::cout << funcMap[currFunction] << "," << funcMap[target] << "\n";
      }

      void visitCallIndirect(CallIndirect* curr) {
        for (auto& segment : module->table.segments) {
          for (auto& curr : segment.data) {
            auto* func = module->getFunction(curr);
            if (visitedTargets.count(func->name) > 0) {
              return;
            }
            visitedTargets.insert(func->name);
            // std::cout << currFunction->name << "," << func->name << "\n";
            std::cout << funcMap[currFunction] << "," << funcMap[func] << "\n";
          }
        }
      }
    };
    CallPrinter printer(module);

  }
};

Pass* createPrintCallGraphEdgesPass() { return new PrintCallGraphEdges(); }

} // namespace wasm
