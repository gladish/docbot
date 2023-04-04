
#pragma once

#include "docbot.h"
#include "log.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>

class DocumentationGeneratorVisitor : public clang::RecursiveASTVisitor<DocumentationGeneratorVisitor> {
public: 
  DocumentationGeneratorVisitor(clang::ASTContext *ctx, const docbot::Options &opts);
  virtual bool VisitFunctionDecl(clang::FunctionDecl *func);

private:
  std::string getFunctionSourceCode(const clang::FunctionDecl *func,
    const clang::SourceManager &source_manager);

private:
  clang::ASTContext *m_ast_context;
  docbot::Options m_options;
};

class DocumentationGeneratorDiagnosticConsumer : public clang::DiagnosticConsumer {
public:
  virtual void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
    const clang::Diagnostic &info) override;
};

class DocumentationGeneratorConsumer : public clang::ASTConsumer {
public:
  explicit DocumentationGeneratorConsumer(clang::ASTContext *context, const docbot::Options &opts);
  virtual void HandleTranslationUnit(clang::ASTContext &context);
private:
  DocumentationGeneratorVisitor m_visitor;
};

class DocumentationGeneratorFrontendAction : public clang::ASTFrontendAction {
public:
  DocumentationGeneratorFrontendAction(const docbot::Options &opts);
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &compiler,llvm::StringRef input_file_name) override;

private:
  docbot::Options m_options;
};