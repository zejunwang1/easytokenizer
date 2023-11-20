#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "args.h"
#include "tokenizer.h"

int main(int argc, char* argv[])
{
  args::ArgumentParser parser("easytokenizer-cpp speed testing.");
  args::HelpFlag help(parser, "help", "Show help information", {'h', "help"});
  args::ValueFlag<std::string> vocabPath(
      parser, "", "Tokenizer vocabulary file.", {"vocab_path"});
  args::Flag doLowerCase(
      parser, "", "Whether to convert upper case letters to lower case.", {"do_lower_case"});
  args::Flag codepointLevel(
      parser, "", "Whether to return character position in offsets.", {"codepoint_level"});
  args::ValueFlag<std::string> sentPath(
      parser, "", "Sentence data path to be processed.", {"sent_path"});
  args::ValueFlag<int> numThreads(
      parser, "", "Number of parallel threads.", {"num_threads"});
  args::ValueFlag<int> batchSize(
      parser, "", "Batch size.", {"batch_size"});
  
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
  
  std::string vocab_path, sent_path;
  bool do_lower_case = false;
  bool codepoint_level = false;
  int num_threads = 1;
  int batch_size = 1;
  if (vocabPath)
    vocab_path = args::get(vocabPath);
  if (doLowerCase)
    do_lower_case = true;
  if (codepointLevel)
    codepoint_level = true;
  if (sentPath)
    sent_path = args::get(sentPath);
  if (numThreads)
    num_threads = args::get(numThreads);
  if (batchSize)
    batch_size = args::get(batchSize);
  if (vocab_path.empty() || sent_path.empty())
  {
    std::cerr << parser;
    throw std::invalid_argument("Get empty vocabulary/sentence file!");
  }
  

  tokenizer::Tokenizer AutoTokenizer(vocab_path, do_lower_case, codepoint_level);
  
  std::string sentence;
  std::vector<std::string> sent_list;
  std::ifstream ifs(sent_path);
  if (!ifs.is_open())
    throw std::invalid_argument(sent_path + " can not be opened for loading!");
  while (std::getline(ifs, sentence))
    if (sentence.size())
      sent_list.emplace_back(sentence);
  
  int start = 0, end = 0, n = sent_list.size();
  int num_batches = n > 0 ? ((n - 1) / batch_size + 1) : 0;
  
  std::vector<std::vector<int>> input_ids;
  std::vector<std::vector<int>> attention_mask;
  std::vector<std::vector<int>> offsets;
  input_ids.reserve(n);
  attention_mask.reserve(n);
  offsets.reserve(n);
  
  bool add_cls_sep = true;
  bool padding = true;
  bool padding_to_max_length = false;
  bool truncation = true;
  int max_length = 512;

  std::vector<std::string> batch_sent_list;
  batch_sent_list.reserve(batch_size);
  std::chrono::steady_clock::time_point time_start = std::chrono::steady_clock::now();
  for (int i = 0; i < num_batches; i++)
  {
    batch_sent_list.clear();
    start = i * batch_size;
    end = std::min((i + 1) * batch_size, n);
    for (int j = start; j < end; j++)
      batch_sent_list.emplace_back(sent_list[j]);
    
    std::vector<std::vector<int>> batch_input_ids;
    std::vector<std::vector<int>> batch_attention_mask;
    std::vector<std::vector<int>> batch_offsets;
    AutoTokenizer.encode(batch_sent_list, batch_input_ids, batch_attention_mask, batch_offsets, 
      num_threads, add_cls_sep, padding, padding_to_max_length, truncation, max_length);
    
    for (int j = 0; j < batch_input_ids.size(); j++)
    {
      input_ids.emplace_back(batch_input_ids[j]);
      attention_mask.emplace_back(batch_attention_mask[j]);
      offsets.emplace_back(batch_offsets[j]);
    }
  }
  std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_used = std::chrono::duration_cast<std::chrono::duration<double>>(
      time_end-time_start);
  std::cout << "Number of sentences: " << n << "  num_threads: " << num_threads << 
      "  batch_size: " << batch_size << std::endl;
  std::cout << "Time usage: " << time_used.count() << "s." << std::endl;
   
  return 0;
}

