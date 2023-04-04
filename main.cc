
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
  std::string input_file_name;
  std::string personality = "testbot";
  std::vector<std::string> search_paths; 
  docbot::Options options;   

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
      case 'p':
        personality = optarg;
      default:
        break;
    }
  }

  if (input_file_name.empty()) {
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

  std::vector<std::string> args;  
  for (const auto &path : search_paths)
    args.push_back(std::string("-I" + path));
  args.push_back("-Wall");

  std::string source_code = read_file(input_file_name);

  if (personality == "docbot") {
    return clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<DocumentationGeneratorFrontendAction>(options), 
      source_code.c_str(),
      args);
  }
  else if (personality == "testbot") {
    return clang::tooling::runToolOnCodeWithArgs(
      std::make_unique<UnitTestGeneratorFrontendAction>(options), 
      source_code.c_str(),
      args);
  }
  else
    throw std::runtime_error("Unknown personality: " + personality);
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
