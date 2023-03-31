
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <getopt.h>

enum class DBLogLevel {
  Debug,
  Info,
  Warning,
  Error,
  Fatal
};

static void dbLogPrintf(DBLogLevel level, const char* file, int line, const char* format, ...) 
  __attribute__ ((format (printf, 4, 5)));

static DBLogLevel sLevel = DBLogLevel::Debug;

#define DBLOG(LEVEL, FORMAT, ...) do { dbLogPrintf(LEVEL, __FILE__, __LINE__, FORMAT, ## __VA_ARGS__); } while (0)
#define DBLOG_DEBUG(FORMAT, ...) DBLOG(DBLogLevel::Debug, FORMAT, ## __VA_ARGS__)
#define DBLOG_INFO(FORMAT, ...) DBLOG(DBLogLevel::Info, FORMAT, ## __VA_ARGS__)
#define DBLOG_WARN(FORMAT, ...) DBLOG(DBLogLevel::Warning, FORMAT, ## __VA_ARGS__)
#define DBLOG_ERROR(FORMAT, ...) DBLOG(DBLogLevel::Error, FORMAT, ## __VA_ARGS__)
#define DBLOG_FATAL(FORMAT, ...) DBLOG(DBLogLevel::Fatal, FORMAT, ## __VA_ARGS__)


class DocumentationGeneratorVisitor : public clang::RecursiveASTVisitor<DocumentationGeneratorVisitor> {
public:
  DocumentationGeneratorVisitor(clang::ASTContext *ctx)
    : m_ast_context(ctx)
    {
      DBLOG_DEBUG("DocumentationGeneratorVisitor");
    } 

    virtual bool VisitFunctionDecl(clang::FunctionDecl *func) {
      // DBLOG_DEBUG("VisitFunctionDecl: %s", func->getNameInfo().getAsString().c_str());


      #if 0
      auto range = func->getSourceRange();
      auto start = clang::Lexer::
      printf("%s\n", func->getAsString().c_str());
      
      auto comments = m_ast_context->getRawCommentForDeclNoCache(func);
      if (comments)        
        printf(" %s\n", comments->getRawText(m_ast_context->getSourceManager()).str().c_str());

      printf("%s ", func->getDeclaredReturnType().getAsString().c_str());
      printf("%s", func->getNameInfo().getAsString().c_str());
      printf("(");

      for (auto &params : func->getParameters()) {
      }

      printf(");");
      printf("\n");
      #endif

      printf("%s\n", getFunctionSourceCode(func, m_ast_context->getSourceManager()).c_str());

      return true;
    }

private:
  std::string getFunctionSourceCode(const clang::FunctionDecl *func, const clang::SourceManager &source_manager) {
    clang::SourceRange range = func->getSourceRange();

    if (range.isInvalid()) {
        return "";
    }

    clang::SourceLocation startLoc = clang::Lexer::getLocForEndOfToken(range.getBegin(), 0, source_manager, func->getLangOpts());
    clang::SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(range.getEnd(), 0, source_manager, func->getLangOpts());

    if (startLoc.isInvalid() || endLoc.isInvalid()) {
        return "";
    }

    return std::string(source_manager.getCharacterData(startLoc), 
      source_manager.getCharacterData(endLoc) - source_manager.getCharacterData(startLoc));
  }


private:
  clang::ASTContext *m_ast_context;

};

class DocumentationGeneratorConsumer : public clang::ASTConsumer {
public:
  explicit DocumentationGeneratorConsumer(clang::ASTContext *context)
    : m_visitor(context) 
    {
      DBLOG_DEBUG("DocumentationGeneratorConsumer");
    }

  virtual void HandleTranslationUnit(clang::ASTContext &context) { 
    DBLOG_DEBUG("HandleTranslationUnit\n");  
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  DocumentationGeneratorVisitor m_visitor;
};

class DocumentationGeneratorFrontendAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &compiler,
    llvm::StringRef input_file_name) override
  {
    DBLOG_DEBUG("CreateASTConsumer: %s", input_file_name.str().c_str());
    return std::make_unique<DocumentationGeneratorConsumer>(&compiler.getASTContext());
  }
};

// read entire contents of a file into a string
std::string read_file(const std::string &file_name)
{
  std::ifstream t(file_name);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

//static llvm::cl::OptionCategory documentation_generator_catgory("docbot options");
//static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
//static llvm::cl::extrahelp MoreHelp("\nMode help text..\n");

int main(int argc, char const *argv[])
{
  std::string input_file_name;
  std::vector<std::string> search_paths;  

  // capture the input_file_name from the command line using getopt_long
  while (true) {
    static struct option long_options[] = {
      {"input-file", required_argument, 0, 'i'},
      {"search-path", required_argument, 0, 'I'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, (char**)argv, "i:I:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'i':
        input_file_name = optarg;
        break;
      case 'I':        
        search_paths.push_back(optarg);
        break;
      default:
        break;
    }
  }

  if (input_file_name.empty()) {
    DBLOG_ERROR("Need to supply an input file name");
    return 1;
  }

  //auto options_parser = clang::tooling::CommonOptionsParser::create(argc, argv, documentation_generator_catgory);  
  //clang::tooling::ClangTool tool(options_parser->getCompilations(), options_parser->getSourcePathList());

  std::vector<std::string> args;  
  for (const auto &path : search_paths)
    args.push_back(std::string("-I" + path));

  std::string source_code = read_file(input_file_name);
  return clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<DocumentationGeneratorFrontendAction>(), 
      source_code.c_str(),
      args);
}


void dbLogPrintf(DBLogLevel level, const char* file, int line, const char* format, ...) 
{
  if (level < sLevel)
    return;
  
  // convert level to string
  const char* level_str = nullptr;
  switch (level) {
    case DBLogLevel::Debug:
      level_str = "DEBUG";
      break;
    case DBLogLevel::Info:
      level_str = "INFO";
      break;
    case DBLogLevel::Warning:
      level_str = "WARN";
      break;
    case DBLogLevel::Error:
      level_str = "ERROR";
      break;
    case DBLogLevel::Fatal:
      level_str = "FATAL";
      break;
  }

  // trim the file name up until the last '/'
  const char* file_name = file;
  const char* last_slash = strrchr(file, '/');
  if (last_slash)
    file_name = last_slash + 1;

  char buff[1024];

  va_list args;
  va_start(args, format);
  vsnprintf(buff, sizeof(buff), format, args);
  va_end(args);

  // print the current date and time
  time_t now = time(0);
  struct tm tstruct;
  char time_buff[80];
  tstruct = *localtime(&now);
  strftime(time_buff, sizeof(time_buff), "%Y-%m-%d.%X", &tstruct);

  // print the log message
  fprintf(stderr, "%s %s %s:%d %s\n", time_buff, level_str, file_name, line, buff);  
}
