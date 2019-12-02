#include "ir/module-utils.h"
#include "ir/utils.h"
#include <ir/local-graph.h>
#include "pass.h"
#include "cfg/cfg-traversal.h"
#include "support/sorted_vector.h"
#include "wasm.h"

namespace wasm {

struct Taint : public Pass { // Performs intraprocedural taint analysis on the start function.

    bool modifiesBinaryenIR() override { return false; }

    void run(PassRunner* runner, Module* module) override {

        struct TaintBlockInfo {
            std::unordered_set<Expression*> exprTaints; // Tainted expressions
            std::unordered_set<Index> localTaints; // Tainted locals
            std::unordered_set<Name> globalTaints; // Tainted globals
            std::unordered_set<Address::address_t> memoryTaints; // Tainted memory
        };

        struct TaintWalker : CFGWalker<TaintWalker, Visitor<TaintWalker>, TaintBlockInfo> {

            std::map<BasicBlock*, size_t> blockIds;

            Module* module;

            std::unordered_set<Name> taintSources; // Name of function that returns a taint (is a source of taint)
            std::unordered_set<Name> taintSinks; // Name of function which is a sink for taints

            TaintWalker(Module* module) : module(module) {
                // Mark taint sources and sinks
                for (auto& exprt : module->exports) {
                    if (exprt->kind == ExternalKind::Function) {
                        if (!strcmp(exprt->name.str, "taint_source")) {
                            taintSources.insert(exprt->value);
                        }
                    } 
                    if (exprt->kind == ExternalKind::Function) {
                        if (!strcmp(exprt->name.str, "taint_sink")) {
                            taintSinks.insert(exprt->value);
                        }
                    }
                }
                if (taintSources.empty()) {
                    std::cout << "WARNING: No taint sources" << std::endl;
                }
                if (taintSinks.empty()) {
                    std::cout << "WARNING: No taint sinks" << std::endl;
                }

                // Walk start function.
                if (module->start.str) {
                    Function* func = module->getFunction(module->start);
                    std::cout << "Walking " << module->start << std::endl;
                    walkFunction(func);
                }

                // // Walk all exported function bodies.
                // for (auto& export_ptr : module->exports) {
                //     Export* e = export_ptr.get();
                //     if (e->kind == ExternalKind::Function) {
                //         Function* func = module->getFunction(e->value);
                //         std::cout << "Walking " << e->name << std::endl;
                //         walkFunction(func);
                //     }
                // }

                // // Wall all functions in table.
                // for (auto& segment : module->table.segments) {
                //     for (auto& curr : segment.data) {
                //         Function* func = module->getFunction(curr);
                //         std::cout << "Walking " << func->name << std::endl;
                //         walkFunction(func);
                //     }
                // }
            }


            void startBasicBlock() {
                CFGWalker::startBasicBlock();

                blockIds[currBasicBlock] = blockIds.size();
                std::cout << "Block: " << blockIds[currBasicBlock] << std::endl;
            }

            void link(BasicBlock* from, BasicBlock* to) {
                CFGWalker::link(from, to);
                if (from && to) {
                    std::cout << "> Joining Block " << blockIds[from] << " (out) to Block " << blockIds[to] << " (in) " << std::endl;
                    to->contents.exprTaints.insert(from->contents.exprTaints.begin(), from->contents.exprTaints.end());
                    to->contents.localTaints.insert(from->contents.localTaints.begin(), from->contents.localTaints.end());
                    to->contents.globalTaints.insert(from->contents.globalTaints.begin(), from->contents.globalTaints.end());
                    to->contents.memoryTaints.insert(from->contents.memoryTaints.begin(), from->contents.memoryTaints.end());
                }
            }

            void visitLocalGet(LocalGet* curr) {
                // std::cout<<"local get"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.localTaints.count(curr->index)) {
                        currBasicBlock->contents.exprTaints.insert(curr);
                        std::cout << "  Loading tainted Local " << curr->index << std::endl;
                    }
                }
            }

            void visitLocalSet(LocalSet* curr) {
                // std::cout<<"local set"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.exprTaints.count(curr->value)) {
                        currBasicBlock->contents.localTaints.insert(curr->index);
                        std::cout << "  Setting taint to Local " << curr->index << std::endl;
                    } else {
                        if (currBasicBlock->contents.localTaints.count(curr->index)) {
                            std::cout << "  Overwriting taint at Local " << curr->index << std::endl;
                        }
                        currBasicBlock->contents.localTaints.erase(curr->index);
                    }
                }
            }

            void visitGlobalGet(GlobalGet* curr) {
                // std::cout<<"global get"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.globalTaints.count(curr->name)) {
                        currBasicBlock->contents.exprTaints.insert(curr);
                        std::cout << "  Loading tainted Global " << curr->name << std::endl;
                    }
                }
            }

            void visitGlobalSet(GlobalSet* curr) {
                // std::cout<<"global set"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.exprTaints.count(curr->value)) {
                        currBasicBlock->contents.globalTaints.insert(curr->name);
                        std::cout << "  Setting taint to Global " << curr->name << std::endl;
                    } else {
                        if (currBasicBlock->contents.globalTaints.count(curr->name)) {
                            std::cout << "  Overwriting taint at Global " << curr->name << std::endl;
                        }
                        currBasicBlock->contents.globalTaints.erase(curr->name);
                    }
                }
            }

            void visitLoad(Load* curr) {
                // std::cout<<"load"<<std::endl;
                if (currBasicBlock) {
                    Address::address_t addr = curr->offset.addr + curr->align.addr;
                    if (currBasicBlock->contents.memoryTaints.count(addr)) {
                        currBasicBlock->contents.exprTaints.insert(curr);
                        std::cout << "  Loading tainted memory at address " << addr << std::endl;
                    }
                }
            }

            void visitStore(Store* curr) {
                // std::cout<<"store"<<std::endl;
                if (currBasicBlock) {
                    Address::address_t addr = curr->offset.addr + curr->align.addr;
                    if (currBasicBlock->contents.exprTaints.count(curr->value)) {
                        currBasicBlock->contents.memoryTaints.insert(addr);
                        std::cout << "  Storing taint at memory address " << addr << std::endl;
                    } else {
                        if (currBasicBlock->contents.memoryTaints.count(addr)) {
                            std::cout << "  Overwriting taint at memory address " << addr << std::endl;
                        }
                        currBasicBlock->contents.memoryTaints.erase(addr);
                    }
                }
            }

            void visitSelect(Select* curr) {
                // std::cout<<"select"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.exprTaints.count(curr->ifTrue) || currBasicBlock->contents.exprTaints.count(curr->ifFalse)) {
                        currBasicBlock->contents.exprTaints.insert(curr);
                        std::cout << "  Tainted Select" << std::endl;
                    }
                }
            }

            void visitCall(Call* curr) {
                // std::cout<<"call"<<std::endl;
                if (currBasicBlock) {
                    if (taintSources.count(curr->target)) {
                        std::cout << "  [Taint source detected]" << std::endl;
                        currBasicBlock->contents.exprTaints.insert(curr);
                    }
                    if (taintSinks.count(curr->target)) {
                        for (auto op : curr->operands) {
                            if (currBasicBlock->contents.exprTaints.count(op)) {
                                std::cout << "  [Taint has reached a sink]" << std::endl;
                            }
                        }
                    }

                    // if any operand is tainted, the return is assumed tainted.
                    for (auto op : curr->operands) {
                        if (currBasicBlock->contents.exprTaints.count(op)) {
                            currBasicBlock->contents.exprTaints.insert(curr);
                            std::cout << "  Tainted value passed to function " << curr->target << std::endl;
                        }
                    }
                }
            }

            void visitCallIndirect(CallIndirect* curr) {
                // std::cout<<"call indirect"<<std::endl;
                if (currBasicBlock) {
                    // Can't determine if the target of the call is a sink without more information. This is room for improvement.

                    // if any operand is tainted, the return is assumed tainted.
                    for (auto op : curr->operands) {
                        if (currBasicBlock->contents.exprTaints.count(op)) {
                            currBasicBlock->contents.exprTaints.insert(curr);
                            std::cout << "  Tainted value passed to an indirect function call" << std::endl;
                        }
                    }
                }
            }

            void visitBinary(Binary* curr) {
                // std::cout<<"binary"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.exprTaints.count(curr->left) || currBasicBlock->contents.exprTaints.count(curr->right)) {
                        currBasicBlock->contents.exprTaints.insert(curr);
                        std::cout << "  Tainted value in Binary operation " << curr->_id << std::endl;
                    }
                }
            }

            void visitUnary(Unary* curr) {
                // std::cout<<"unary"<<std::endl;
                if (currBasicBlock) {
                    if (currBasicBlock->contents.exprTaints.count(curr->value)) {
                        currBasicBlock->contents.exprTaints.insert(curr);
                        std::cout << "  Tainted value in Unary operation " << curr->_id << std::endl;
                    }
                }
            }

        };
        TaintWalker walker(module);
    }

};

Pass* createTaintPass() { return new Taint(); }

} // namespace wasm
