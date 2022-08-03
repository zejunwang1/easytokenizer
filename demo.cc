#include <iostream>
#include <string>
#include <vector>
#include <tokenizer.h>

using tokenizer::Offset;

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() != 3 || args[1] != "-vocab") {
    std::cout << "Usage: ./demo -vocab [vocab_path]" << std::endl;
    throw std::invalid_argument("Invalid command line argument!");
  }

  std::string vocab_path = args[2];
  bool do_lower_case = true;    
  
  tokenizer::Tokenizer AutoTokenizer(vocab_path, do_lower_case);

  /***********************************
   * encode single text
   **********************************/
  std::string text = "计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。";
  std::cout << "encode single text:" << std::endl;
  std::cout << text << std::endl;

  std::vector<std::string> tokens;
  std::vector<Offset> offsets;
  // tokens = AutoTokenizer.wordpiece_tokenize(text);
  AutoTokenizer.wordpiece_tokenize(text, tokens, offsets);

  std::cout << "tokenize result:" << std::endl;
  for (size_t i = 0; i < tokens.size(); i++) {
    std::cout << tokens[i] << " position:" << offsets[i].first << " length:" << offsets[i].second;
    std::cout << std::endl;
  }
  
  bool add_cls_sep = true;
  bool truncation = true;
  size_t max_length = 512;

  std::vector<int64_t> input_ids;
  // input_ids = AutoTokenizer.encode(text, add_cls_sep, truncation, max_length);
  AutoTokenizer.encode(text, input_ids, offsets, add_cls_sep, truncation, max_length);

  std::cout << "encode result:" << std::endl;
  for (size_t i = 0; i < input_ids.size(); i++) {
    std::cout << input_ids[i] << " ";
  }
  std::cout << std::endl << std::endl;
  
  
  /***********************************
   * encode batch texts 
   **********************************/
  std::vector<std::string> texts;
  texts.emplace_back("计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。");
  texts.emplace_back("清华大学的[MASK]算机科学与技术专业实力全国第一。");
  std::cout << "encode batch texts:" << std::endl;
  for (size_t i = 0; i < texts.size(); i++) {
    std::cout << texts[i] << std::endl;
  }
  
  int num_threads = 1;
  bool padding = true;
  bool padding_to_max_length = false;

  std::vector<std::vector<int64_t>> batch_input_ids;
  std::vector<std::vector<int64_t>> batch_token_type_ids;
  std::vector<std::vector<int64_t>> batch_attention_mask;
  std::vector<std::vector<Offset>> batch_offsets;
  AutoTokenizer.encode(texts, batch_input_ids, batch_token_type_ids, batch_attention_mask,
      batch_offsets, num_threads, add_cls_sep, padding, padding_to_max_length, truncation, max_length);
  
  std::cout << "encode result:" << std::endl;
  std::cout << "input_ids:" << std::endl;
  for (size_t i = 0; i < batch_input_ids.size(); i++) {
    for (size_t j = 0; j < batch_input_ids[i].size(); j++) {
      std::cout << batch_input_ids[i][j] << " ";
    }
    std::cout << std::endl;
  }
  
  std::cout << "attention_mask:" << std::endl;
  for (size_t i = 0; i < batch_attention_mask.size(); i++) {
    for (size_t j = 0; j < batch_attention_mask[i].size(); j++) {
      std::cout << batch_attention_mask[i][j] << " ";
    }
    std::cout << std::endl;
  }
  return 0;
}
