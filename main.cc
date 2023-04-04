#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include <getopt.h>

#include "docbot.h"
#include "openai.h"

class DocumentationGeneratorVisitor : public clang::RecursiveASTVisitor<DocumentationGeneratorVisitor> {
public:
  struct Options {
    std::regex FunctionNameRegex;
    std::string ApiKey;
  };

  DocumentationGeneratorVisitor(clang::ASTContext *ctx, const Options &opts)
    : m_ast_context(ctx)
    , m_options(opts) { }

  virtual bool VisitFunctionDecl(clang::FunctionDecl *func)
  {    
    if (!func->hasBody())
      return true;

    const std::string function_name = func->getNameInfo().getAsString();
    if (!std::regex_match(function_name, m_options.FunctionNameRegex))
      return true;
        
    DBLOG_INFO("found match:%s", function_name.c_str());

    // TODO: upload to chat_gpt and get documentation    
    printf("%s\n", getFunctionSourceCode(func, m_ast_context->getSourceManager()).c_str());

    // and then step2 (draw sequence diagram)

    // and then step3 (generate unit tests)
    return true;    
  }

private:
  std::string getFunctionSourceCode(const clang::FunctionDecl *func, const clang::SourceManager &source_manager) {        
    const clang::SourceRange range = func->getSourceRange();
    const clang::SourceLocation start_location = range.getBegin();

    const clang::SourceLocation end_location = clang::Lexer::getLocForEndOfToken(range.getEnd(), 0, 
      source_manager, func->getLangOpts());

    const char *begin = source_manager.getCharacterData(start_location);
    const char *end = source_manager.getCharacterData(end_location);

    return std::string(begin, (end - begin));      
  }

private:
  clang::ASTContext *m_ast_context;  
  Options m_options;
};

class DocumentationGeneratorDiagnosticConsumer : public clang::DiagnosticConsumer {
public:
  virtual void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic &info) override {       
    clang::SmallString<256> message;
    info.FormatDiagnostic(message);
    // DiagnosticConsumer::HandleDiagnostic(level, info);
    if (level == clang::DiagnosticsEngine::Level::Fatal) {
      DBLOG_FATAL("FATAL:%s", message.c_str());    
      exit(1);
    }
  }
};

class DocumentationGeneratorConsumer : public clang::ASTConsumer {
public:
  explicit DocumentationGeneratorConsumer(clang::ASTContext *context, const DocumentationGeneratorVisitor::Options &opts)
    : m_visitor(context, opts) { }    

  virtual void HandleTranslationUnit(clang::ASTContext &context) {   
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  DocumentationGeneratorVisitor m_visitor;
};

class DocumentationGeneratorFrontendAction : public clang::ASTFrontendAction {
public:
  DocumentationGeneratorFrontendAction(const DocumentationGeneratorVisitor::Options &opts)
    : m_options(opts) { }

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &compiler, llvm::StringRef input_file_name) override
  {
    compiler.getDiagnostics().setClient(new DocumentationGeneratorDiagnosticConsumer());
    return std::make_unique<DocumentationGeneratorConsumer>(&compiler.getASTContext(), m_options);
  }
private:
  DocumentationGeneratorVisitor::Options m_options;
};

// read entire contents of a file into a string
std::string read_file(const std::string &file_name)
{
  std::ifstream t(file_name);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

void print_help();

//static llvm::cl::OptionCategory documentation_generator_catgory("docbot options");
//static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
//static llvm::cl::extrahelp MoreHelp("\nMode help text..\n");

int main(int argc, char const *argv[])
{  
  std::string input_file_name;
  std::vector<std::string> search_paths;  
  
  DocumentationGeneratorVisitor::Options options;

  // capture the input_file_name from the command line using getopt_long
  while (true) {
    static struct option long_options[] = {
      {"input-file", required_argument, 0, 'f'},
      {"search-path", required_argument, 0, 'I'},
      {"help", no_argument, 0, 'h'},
      {"regex", required_argument, 0, 'r'},
      {"api-key", required_argument, 0, 'k'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, (char**)argv, "f:I:hr:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'f':
        input_file_name = optarg;
        break;
      case 'I':        
        search_paths.push_back(optarg);
        break;
      case 'h':
        print_help();
        exit(0);
      case 'r':
        options.FunctionNameRegex = optarg;
        break;
      case 'k':
        options.ApiKey = optarg;
        break;
      default:
        break;
    }
  }

  if (input_file_name.empty()) {
    DBLOG_ERROR("Need to supply an input file name");
    print_help();
    return 1;
  }
  

  std::vector<std::string> args;  
  for (const auto &path : search_paths)
    args.push_back(std::string("-I" + path));

  std::string source_code = read_file(input_file_name);
  source_code = R"(int8_t     queue_push      (queue_t *q, void *data)
{
    element_t *e, *tmp;
    e = (element_t *)malloc(sizeof(element_t));
    if (e == NULL) {
        return -1;
    }
    memset(e, 0, sizeof(element_t));
    e->data = data;
    if (q->head == NULL) {
        q->head = e;
    } else {
        tmp = q->head;
        q->head = e;
        e->next = tmp;
    }
    return 0;
})";

  openai::Client clnt("...");
  //clnt.getModels();
  clnt.documentCode(source_code);
  return 0;

  return clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<DocumentationGeneratorFrontendAction>(options), 
      source_code.c_str(),
      args);
}

void print_help()
{
  printf("Usage: docbot [options] -f <input-file>\n");
  printf("Options:\n");
  printf("  -f, --input-file <file>  Input file name\n");
  printf("  -I, --search-path <path> Search path\n");
  printf("  -h, --help               Display this help message\n");
  printf("  -r, --regex <regex>      Function name regex\n");
}
