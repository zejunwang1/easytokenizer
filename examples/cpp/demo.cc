#include <iostream>
#include <string>
#include <vector>

#include "args.h"
#include "tokenizer.h"

using tokenizer::SizeT;

int main(int argc, char* argv[]) {
  args::ArgumentParser parser("easytokenizer-cpp usage demo.");
  args::HelpFlag help(parser, "help", "Show help information", {'h', "help"});
  args::ValueFlag<std::string> vocabPath(
      parser, "", "Tokenizer vocabulary file.", {"vocab_path"});
  args::Flag doLowerCase(
      parser, "", "Whether to convert upper case letters to lower case.", {"do_lower_case"});
  args::Flag codepointLevel(
      parser, "", "Whether to return character position in offsets.", {"codepoint_level"});
  
  // parse arguments
  try
  { 
    parser.ParseCLI(argc, argv);
  }
  catch (args::Help) 
  {
    std::cerr << parser;
    return 0;
  } 
  catch (args::ParseError e) 
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    std::exit(EXIT_FAILURE);
  } 
  catch (args::ValidationError e) 
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    std::exit(EXIT_FAILURE);
  }
  
  std::string vocab_path;
  bool do_lower_case = false;
  bool codepoint_level = false;
  if (vocabPath)
    vocab_path = args::get(vocabPath);
  if (doLowerCase)
    do_lower_case = true;
  if (codepointLevel)
    codepoint_level = true;
  if (vocab_path.empty())
  {
    std::cerr << parser;
    throw std::invalid_argument("Get empty vocabulary file!");
  }
    

  tokenizer::Tokenizer AutoTokenizer(vocab_path, do_lower_case, codepoint_level);

  /***********************************
   * encode single text
   **********************************/
  std::string text = "计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。";
  std::cout << "encode single text:" << std::endl;
  std::cout << text << std::endl;
  
  // auto& tokens = AutoTokenizer.wordpiece_tokenize(text);
  std::vector<std::string> tokens;
  std::vector<SizeT> offsets;
  AutoTokenizer.wordpiece_tokenize(text, tokens, offsets);

  std::cout << "tokenize result:" << std::endl;
  for (size_t i = 0; i < tokens.size(); i++) 
  {
    std::cout << tokens[i] << " start:" << offsets[2 * i] << " end:" << offsets[2 * i + 1];
    std::cout << std::endl;
  }
  
  bool add_cls_sep = true;
  bool truncation = true;
  SizeT max_length = 512;
  
  // auto& input_ids = AutoTokenizer.encode(text, add_cls_sep, truncation, max_length);
  std::vector<SizeT> input_ids;
  std::vector<SizeT> token_type_ids;
  std::vector<SizeT> attention_mask;
  AutoTokenizer.encode(text, input_ids, token_type_ids, attention_mask, offsets, 
    add_cls_sep, truncation, max_length);

  std::cout << "encode result:" << std::endl;
  for (size_t i = 0; i < input_ids.size(); i++)
    std::cout << input_ids[i] << " ";
  std::cout << std::endl << std::endl;
  
  
  /***********************************
   * encode batch texts 
   **********************************/
  std::vector<std::string> texts;
  texts.emplace_back("计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。");
  texts.emplace_back("清华大学的[MASK]算机科学与技术专业实力全国第一。");
  std::cout << "encode batch texts:" << std::endl;
  for (size_t i = 0; i < texts.size(); i++)
    std::cout << texts[i] << std::endl;
  
  int num_threads = 1;
  bool padding = true;
  bool padding_to_max_length = false;

  std::vector<std::vector<SizeT>> batch_input_ids;
  std::vector<std::vector<SizeT>> batch_token_type_ids;
  std::vector<std::vector<SizeT>> batch_attention_mask;
  std::vector<std::vector<SizeT>> batch_offsets;
  AutoTokenizer.encode(texts, batch_input_ids, batch_token_type_ids, batch_attention_mask,
      batch_offsets, num_threads, add_cls_sep, padding, padding_to_max_length, truncation, max_length);
  
  std::cout << "encode result:" << std::endl;
  std::cout << "input_ids:" << std::endl;
  for (size_t i = 0; i < batch_input_ids.size(); i++) 
  {
    for (size_t j = 0; j < batch_input_ids[i].size(); j++)
      std::cout << batch_input_ids[i][j] << " ";
    std::cout << std::endl;
  }
  
  std::cout << "attention_mask:" << std::endl;
  for (size_t i = 0; i < batch_attention_mask.size(); i++)
  {
    for (size_t j = 0; j < batch_attention_mask[i].size(); j++)
      std::cout << batch_attention_mask[i][j] << " ";
    std::cout << std::endl;
  }
  
  std::cout << "offsets:" << std::endl;
  for (size_t i = 0; i < batch_offsets.size(); i++)
  {
    for (size_t j = 0; j < batch_offsets[i].size(); j++)
      std::cout << batch_offsets[i][j] << " ";
    std::cout << std::endl;
  }

  return 0;
}
