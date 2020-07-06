#include <tildac/codegen.hpp>
#include <tildac/types.hpp>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace tildac {
    struct Compiler_Context {
        llvm::LLVMContext handle;
        llvm::IRBuilder<> builder;
        llvm::Module module;
        llvm::TargetMachine* target_cpu;
        llvm::Reloc::Model reloc_model;
        std::unordered_map<std::string, llvm::Type*> builtin_types;
        std::vector<std::unordered_map<std::string, llvm::AllocaInst*>> symbol_table;

        Compiler_Context(): handle(), builder(handle), module("", handle) {
            auto triple = llvm::sys::getDefaultTargetTriple();
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();

            module.setTargetTriple(triple);
            std::string error{};
            auto target = llvm::TargetRegistry::lookupTarget(triple, error);

            llvm::TargetOptions target_options;
            reloc_model = llvm::Reloc::Model::PIC_;
            target_cpu = target->createTargetMachine(triple, "generic", "", target_options, reloc_model);
            module.setDataLayout(target_cpu->createDataLayout());

            builtin_types = {
                { "void", llvm::Type::getVoidTy(handle) },
                { "bool", llvm::Type::getInt1Ty(handle) },
                { "i8", llvm::Type::getInt8Ty(handle) },
                { "i16", llvm::Type::getInt16Ty(handle) },
                { "i32", llvm::Type::getInt32Ty(handle) },
                { "i64", llvm::Type::getInt64Ty(handle) },
                { "u8", llvm::Type::getInt8Ty(handle) },
                { "u16", llvm::Type::getInt16Ty(handle) },
                { "u32", llvm::Type::getInt32Ty(handle) },
                { "u64", llvm::Type::getInt64Ty(handle) },
                { "c8", llvm::Type::getInt8Ty(handle) },
                { "c16", llvm::Type::getInt16Ty(handle) },
                { "c32", llvm::Type::getInt32Ty(handle) },
                { "f32", llvm::Type::getFloatTy(handle) },
                { "f64", llvm::Type::getDoubleTy(handle) },
                { "c8**", llvm::PointerType::get(llvm::PointerType::get(llvm::Type::getInt8Ty(handle), 0), 0) },
            };
        }
    };

    static void emit_compile_error(const std::string& msg) {
        llvm::errs() << msg;
    }

    static bool is_block_terminated(llvm::BasicBlock* block) {
        return block->getInstList().back().isTerminator();
    }

    static llvm::Type* acquire_llvm_type(Compiler_Context& context, const Qualified_Type& type) {
        return context.builtin_types[type.name];
    }

    static llvm::Type* acquire_llvm_type(Compiler_Context& context, const Type& type) {
        switch(type.node_type) {
            case AST_Node_Type::qualified_type: {
                return acquire_llvm_type(context, static_cast<const Qualified_Type&>(type));
            }

            default:
                return nullptr;
        }
    }

    static llvm::AllocaInst* make_variable_alloca(Compiler_Context& context, const Variable_Declaration& variable) {
        const std::string& name = variable.identifier->name;
        llvm::Type* type = acquire_llvm_type(context, *variable.type);
        llvm::AllocaInst* alloca = context.builder.CreateAlloca(type, nullptr, name);
        context.symbol_table.back()[name] = alloca;
        return alloca;
    }

    static llvm::Value* generate_expression(Compiler_Context& context, const Expression& expression);

    static llvm::Value* generate_literal_expression(Compiler_Context& context, const Integer_Literal& expression) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.handle), std::stoull(expression.value));
    }

    static llvm::Value* generate_identifier_expression(Compiler_Context& context, const Identifier& identifier) {
        const auto variable = context.symbol_table.back().at(identifier.name);
        return context.builder.CreateLoad(variable->getAllocatedType(), variable);
    }

    static llvm::Value* generate_binary_expression(Compiler_Context& context, const Binary_Expression& expression) {
        auto lhs = generate_expression(context, *expression.lhs);
        auto rhs = generate_expression(context, *expression.rhs);
        switch(expression.op) {
            case Operator::binary_add: {
                return context.builder.CreateAdd(lhs, rhs);
            }

            case Operator::binary_sub: {
                return context.builder.CreateSub(lhs, rhs);
            }

            case Operator::binary_mul: {
                return context.builder.CreateMul(lhs, rhs);
            }

            case Operator::binary_div: {
                return context.builder.CreateSDiv(lhs, rhs);
            }

            case Operator::binary_eq: {
                return context.builder.CreateICmpEQ(lhs, rhs);
            }

            default:
                return nullptr;
        }
    }

    static llvm::Value* generate_function_call_expression(Compiler_Context& context, const Function_Call_Expression& expression) {
        llvm::Function* function = context.module.getFunction(expression.identifier->name);
        if(!function) {
            emit_compile_error("Undefined function: \"" + expression.identifier->name + "\" referenced");
        }

        std::vector<llvm::Value*> arguments{};
        for(const auto& argument: expression.arg_list->arguments) {
            arguments.emplace_back(generate_expression(context, *argument));
        }

        return context.builder.CreateCall(function, arguments);
    }

    static llvm::Value* generate_expression(Compiler_Context& context, const Expression& expression) {
        switch(expression.node_type) {
            case AST_Node_Type::integer_literal: {
                return generate_literal_expression(context, static_cast<const Integer_Literal&>(expression));
            }

            case AST_Node_Type::binary_expression: {
                return generate_binary_expression(context, static_cast<const Binary_Expression&>(expression));
            }

            case AST_Node_Type::identifier_expression: {
                return generate_identifier_expression(context, *static_cast<const Identifier_Expression&>(expression).identifier);
            }

            case AST_Node_Type::function_call_expression: {
                return generate_function_call_expression(context, static_cast<const Function_Call_Expression&>(expression));
            }
            default:
                return nullptr;
        }
    }

    static void generate_statement(Compiler_Context& context, const Statement& statement);

    static void generate_if_statement(Compiler_Context& context, const If_Statement& statement) {
        auto condition = generate_expression(context, *statement.condition);
        auto function = context.builder.GetInsertBlock()->getParent();

        llvm::BasicBlock* true_block = llvm::BasicBlock::Create(context.handle, "", function);
        llvm::BasicBlock* false_block = llvm::BasicBlock::Create(context.handle, "", function);
        llvm::BasicBlock* merge_block = llvm::BasicBlock::Create(context.handle, "", function);

        context.builder.CreateCondBr(condition, true_block, merge_block);
        context.builder.SetInsertPoint(true_block);
        generate_statement(context, *statement.block);
        if(!is_block_terminated(true_block)) {
            context.builder.CreateBr(merge_block);
        }

        context.builder.SetInsertPoint(false_block);
        if(statement.else_block) {
            generate_statement(context, *statement.else_block);
            return;
        }
        if(!is_block_terminated(false_block)) {
            context.builder.CreateBr(merge_block);
        }
        context.builder.SetInsertPoint(merge_block);
        if(statement.else_if) {
            generate_if_statement(context, *statement.else_if);
        }
    }

    static void generate_return_statement(Compiler_Context& context, const Return_Statement& statement) {
        if(!statement.expression) {
            context.builder.CreateRetVoid();
        } else {
            context.builder.CreateRet(generate_expression(context, *statement.expression));
        }
    }

    static void generate_variable_declaration(Compiler_Context& context, const Variable_Declaration& declaration) {
        auto variable = make_variable_alloca(context, declaration);
        if(declaration.initializer) {
            context.builder.CreateStore(generate_expression(context, *declaration.initializer), variable);
        }
    }

    static void generate_statement_list(Compiler_Context& context, const Statement_List& node) {
        for(const auto& statement: node.statements) {
            generate_statement(context, *statement);
        }
    }

    static void generate_statement(Compiler_Context& context, const Statement& statement) {
        switch(statement.node_type) {
            case AST_Node_Type::if_statement: {
                return generate_if_statement(context, static_cast<const If_Statement&>(statement));
            }

            case AST_Node_Type::return_statement: {
                return generate_return_statement(context, static_cast<const Return_Statement&>(statement));
            }

            case AST_Node_Type::declaration_statement: {
                return generate_variable_declaration(context, *static_cast<const Declaration_Statement&>(statement).var_decl);
            }

            case AST_Node_Type::block_statement: {
                return generate_statement_list(context, *static_cast<const Block_Statement&>(statement).statements);
            }

            default:
                return;
        }
    }

    static void generate_function(Compiler_Context& context, const Function_Declaration& node) {
        std::vector<llvm::Type*> arguments{};
        for(const auto& parameter: node.parameter_list->params) {
            arguments.emplace_back(acquire_llvm_type(context, *parameter->type));
        }
        auto function_type = llvm::FunctionType::get(acquire_llvm_type(context, *node.return_type), arguments, false);
        auto function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, node.name->name, context.module);
        auto block = llvm::BasicBlock::Create(context.handle, "", function);
        context.builder.SetInsertPoint(block);
        context.symbol_table.emplace_back();
        u64 arg_idx = 0;
        for(auto& arg: function->args()) {
            arg.setName(node.parameter_list->params[arg_idx++]->identifier->name);
            llvm::IRBuilder<> param_builder(&function->getEntryBlock(), function->getEntryBlock().begin());
            auto param_alloca = param_builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            context.symbol_table.back()[arg.getName()] = param_alloca;
            param_builder.CreateStore(&arg, param_alloca);
        }
        generate_statement_list(context, *node.body->statements);
        context.symbol_table.pop_back();
    }

    static void generate_node(Compiler_Context& context, const AST_Node& node) {
        switch(node.node_type) {
            case AST_Node_Type::function_declaration: {
                generate_function(context, static_cast<const Function_Declaration&>(node));
            } break;

            default:
                return;
        }
    }

    void generate(const std::vector<Owning_Ptr<AST_Node>>& nodes, const bool optimize) {
        Compiler_Context context{};

        for(const auto& node: nodes) {
            generate_node(context, *node);
        }

        llvm::legacy::PassManager pass_manager{};
        if(optimize) {
            llvm::PassBuilder pass_builder;
            llvm::LoopAnalysisManager loop_analysis_manager(false);
            llvm::FunctionAnalysisManager function_analysis_manager(false);
            llvm::CGSCCAnalysisManager CGSCC_analysis_manager(false);
            llvm::ModuleAnalysisManager module_analysis_manager(false);
            pass_builder.registerModuleAnalyses(module_analysis_manager);
            pass_builder.registerCGSCCAnalyses(CGSCC_analysis_manager);
            pass_builder.registerFunctionAnalyses(function_analysis_manager);
            pass_builder.registerLoopAnalyses(loop_analysis_manager);
            pass_builder.crossRegisterProxies(loop_analysis_manager, function_analysis_manager, CGSCC_analysis_manager, module_analysis_manager);
            llvm::ModulePassManager module_pass_manager = pass_builder.buildPerModuleDefaultPipeline(llvm::PassBuilder::O3, false);
            module_pass_manager.run(context.module, module_analysis_manager);

            pass_manager.add(llvm::createInstructionCombiningPass());
            pass_manager.add(llvm::createReassociatePass());
            pass_manager.add(llvm::createGVNPass());
            pass_manager.add(llvm::createCFGSimplificationPass());
        }
        std::error_code file_error_code;
        llvm::raw_fd_ostream output("output.o", file_error_code, llvm::sys::fs::OF_None);
        if(context.target_cpu->addPassesToEmitFile(pass_manager, output, nullptr, llvm::CGFT_ObjectFile)) {
            throw std::runtime_error("Target platform doesn't support object files.");
        }
        pass_manager.run(context.module);
        output.flush();

        context.module.print(llvm::outs(), nullptr);
    }
} // namespace tildac