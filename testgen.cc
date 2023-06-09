#include "testgen.h"

UnitTestGeneratorVisitor::UnitTestGeneratorVisitor(clang::ASTContext *ctx, const docbot::Options &opts)
  : m_ast_context(ctx)
  , m_options(opts) 
{
}

bool UnitTestGeneratorVisitor::VisitFunctionDecl(clang::FunctionDecl *func)
{ 
  const std::string function_name = func->getNameInfo().getAsString();
  DBLOG_INFO("found function:%s", function_name.c_str());
  return true;
}

std::string UnitTestGeneratorVisitor::getFunctionSourceCode(const clang::FunctionDecl *func,
  const clang::SourceManager &source_manager)
{
  const clang::SourceRange range = func->getSourceRange();
  const clang::SourceLocation start_location = range.getBegin();

  const clang::SourceLocation end_location = clang::Lexer::getLocForEndOfToken(
      range.getEnd(), 0, source_manager, func->getLangOpts());

  const char *begin = source_manager.getCharacterData(start_location);
  const char *end = source_manager.getCharacterData(end_location);

  return std::string(begin, (end - begin));
}

void UnitTestGeneratorDiagnosticConsumer::HandleDiagnostic(clang::DiagnosticsEngine::Level level,
  const clang::Diagnostic &info)
{
  clang::SmallString<256> message;
  info.FormatDiagnostic(message);
  // DiagnosticConsumer::HandleDiagnostic(level, info);
  if (level == clang::DiagnosticsEngine::Level::Fatal) {
    DBLOG_FATAL("FATAL:%s", message.c_str());
    exit(1);
  }
}

UnitTestGeneratorConsumer::UnitTestGeneratorConsumer(clang::ASTContext *context,
    const docbot::Options &opts)
  : m_visitor(context, opts)
{

}

void UnitTestGeneratorConsumer::HandleTranslationUnit(clang::ASTContext &context) 
{
  m_visitor.TraverseDecl(context.getTranslationUnitDecl());
}

UnitTestGeneratorFrontendAction::UnitTestGeneratorFrontendAction(const docbot::Options &opts)
  : m_options(opts)
{

}

std::unique_ptr<clang::ASTConsumer> UnitTestGeneratorFrontendAction::CreateASTConsumer(
    clang::CompilerInstance &compiler, llvm::StringRef input_file_name)
{
  compiler.getDiagnostics().setClient(new UnitTestGeneratorDiagnosticConsumer());
  return std::make_unique<UnitTestGeneratorConsumer>(&compiler.getASTContext(), m_options);
}