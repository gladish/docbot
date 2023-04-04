
#include "docbot.h"
#include "log.h"
#include "docgen.h"
#include "testgen.h"

#include <clang/Tooling/Tooling.h>

#include <fstream>
#include <sstream>
#include <getopt.h>

static void print_help();
static std::string read_file(const std::string &file_name);

int main(int argc, char const *argv[])
{      
  docbot::Options options;
  options.Personality = "testbot";   

  // capture the input_file_name from the command line using getopt_long
  while (true) {
    static struct option long_options[] = {
      {"input-file", required_argument, 0, 'f'},
      {"search-path", required_argument, 0, 'I'},
      {"help", no_argument, 0, 'h'},
      {"regex", required_argument, 0, 'r'},
      {"api-key", required_argument, 0, 'k'},
      {"debug", no_argument, 0, 'd'}, 
      {"personality", required_argument, 0, 'p'},   
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, (char**)argv, "f:I:hr:p:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {      
      case 'f':
        options.InputFilename = optarg;
        break;
      case 'I':        
        options.CompilerArgs.push_back(std::string("-I" + std::string(optarg)));
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
      case 'p':
        options.Personality = optarg;
      default:
        break;
    }
  }

  if (options.InputFilename.empty()) {
    DBLOG_ERROR("Need to supply an input file name");
    print_help();
    return 1;
  }

  if (options.ApiKey.empty()) {
    const char *s = getenv("OPENAI_API_KEY");
    if (s)
      options.ApiKey = s;    
  }

  if (options.ApiKey.empty()) {
    DBLOG_ERROR("Need to supply an OpenAI API key");
    print_help();
    return 1;
  }

  std::string source_code = read_file(options.InputFilename);

  if (options.Personality == "docbot") {
    return clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<DocumentationGeneratorFrontendAction>(options), 
      source_code.c_str(),
      options.CompilerArgs);
  }
  else if (options.Personality == "testbot") {
    return clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<UnitTestGeneratorFrontendAction>(options), 
      source_code.c_str(),
      options.CompilerArgs);
  }
  else
    throw std::runtime_error("Unknown personality: " + options.Personality);
}

void print_help()
{
  printf("Usage: docbot [options] -f <input-file>\n");
  printf("Options:\n");
  printf("  -f, --input-file <file>  Input file name\n");
  printf("  -I, --search-path <path> Search path\n");
  printf("  -h, --help               Display this help message\n");
  printf("  -r, --regex <regex>      Function name regex\n");
  printf("  -p, --personality <name> Personality name. One of [\"docbot\", \"testbot\"]\n");
  printf("  -k, --api-key <key>      OpenAI API key\n");
  printf("    or set env OPENAI_API_KEY\n");
}

std::string read_file(const std::string &file_name) 
{
  std::ifstream t(file_name);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}
