

#include "docgen.h"

DocumentationGeneratorVisitor::DocumentationGeneratorVisitor(clang::ASTContext *ctx, const docbot::Options &opts)
  : m_ast_context(ctx)
  , m_options(opts) 
{
}

bool DocumentationGeneratorVisitor::VisitFunctionDecl(clang::FunctionDecl *func)
{
  if (!func->hasBody())
    return true;

  const std::string function_name = func->getNameInfo().getAsString();
  if (!std::regex_match(function_name, m_options.FunctionNameRegex))
    return true;

  DBLOG_INFO("found match:%s", function_name.c_str());

  // TODO: upload to chat_gpt and get documentation
  printf(
      "%s\n",
      getFunctionSourceCode(func, m_ast_context->getSourceManager()).c_str());

  // and then step2 (draw sequence diagram)

  // and then step3 (generate unit tests)
  return true;
}

std::string DocumentationGeneratorVisitor::getFunctionSourceCode(
    const clang::FunctionDecl *func,
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

void DocumentationGeneratorDiagnosticConsumer::HandleDiagnostic(clang::DiagnosticsEngine::Level level,
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

DocumentationGeneratorConsumer::DocumentationGeneratorConsumer(clang::ASTContext *context,
    const docbot::Options &opts)
  : m_visitor(context, opts)
{

}

void DocumentationGeneratorConsumer::HandleTranslationUnit(clang::ASTContext &context) 
{
  m_visitor.TraverseDecl(context.getTranslationUnitDecl());
}

DocumentationGeneratorFrontendAction::DocumentationGeneratorFrontendAction(const docbot::Options &opts)
  : m_options(opts)
{

}

std::unique_ptr<clang::ASTConsumer> DocumentationGeneratorFrontendAction::CreateASTConsumer(
    clang::CompilerInstance &compiler, llvm::StringRef input_file_name)
{
  compiler.getDiagnostics().setClient(new DocumentationGeneratorDiagnosticConsumer());
  return std::make_unique<DocumentationGeneratorConsumer>(&compiler.getASTContext(), m_options);
}
