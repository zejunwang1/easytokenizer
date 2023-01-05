# coding=utf-8
# author: wangzejun (wangzejunscut@126.com)

import argparse
import os
import time
from transformers import BertTokenizer, BertTokenizerFast
from easytokenizer import AutoTokenizer as OurTokenizer

from paddlenlp.transformers import AutoTokenizer

def main(args):
    sent_list = []
    with open(args.data_path, mode="r", encoding="utf-8") as data_handle:
        for line in data_handle:
            line = line.strip()
            if line:
                sent_list.append(line)
    
    # huggingface tokenizer
    hug_tokenizer = BertTokenizer.from_pretrained(
        "bert-base-chinese", do_lower_case=args.do_lower_case)
    hug_fast_tokenizer = BertTokenizerFast.from_pretrained(
        "bert-base-chinese", do_lower_case=args.do_lower_case)
    
    # easytokenizer
    our_tokenizer = OurTokenizer(args.vocab_path, do_lower_case=args.do_lower_case)
    
    # paddlenlp faster tokenizer
    paddlenlp_tokenizer = AutoTokenizer.from_pretrained(
        "bert-base-chinese", use_faster=True, do_lower_case=args.do_lower_case)
    
    num_batches = int((len(sent_list) - 1) / args.batch_size) + 1
    '''
    hug_input_ids = []
    hug_time_usage = 0
    for i in range(num_batches):
        start = i * args.batch_size
        end = min((i + 1) * args.batch_size, len(sent_list))
        batch_sent_list = sent_list[start: end]
        
        t_s = time.time()
        batch_output = hug_tokenizer(batch_sent_list, padding=True)
        batch_input_ids = batch_output["input_ids"]
        t_e = time.time()
        hug_time_usage += (t_e - t_s)
        hug_input_ids.extend(batch_input_ids)
    '''
    hug_fast_input_ids = []
    hug_fast_time_usage = 0
    for i in range(num_batches):
        start = i * args.batch_size
        end = min((i + 1) * args.batch_size, len(sent_list))
        batch_sent_list = sent_list[start: end]
        
        t_s = time.time()
        batch_output = hug_fast_tokenizer(batch_sent_list, padding=True)
        batch_input_ids = batch_output["input_ids"]
        t_e = time.time()
        hug_fast_time_usage += (t_e - t_s)
        hug_fast_input_ids.extend(batch_input_ids)
    
    our_input_ids = []
    our_time_usage = 0
    for i in range(num_batches):
        start = i * args.batch_size
        end = min((i + 1) * args.batch_size, len(sent_list))
        batch_sent_list = sent_list[start: end]
        
        t_s = time.time()   
        batch_output = our_tokenizer.encode(batch_sent_list, num_threads=args.num_threads)
        batch_input_ids = batch_output["input_ids"]
        t_e = time.time()
        our_time_usage += (t_e - t_s)
        our_input_ids.extend(batch_input_ids)
    
    paddlenlp_input_ids = []
    paddle_time_usage = 0
    for i in range(num_batches):
        start = i * args.batch_size
        end = min((i + 1) * args.batch_size, len(sent_list))
        batch_sent_list = sent_list[start: end]
        
        t_s = time.time()        
        batch_output = paddlenlp_tokenizer(batch_sent_list, padding=True)
        batch_input_ids = batch_output["input_ids"]
        t_e = time.time()
        paddle_time_usage += (t_e - t_s)
        paddlenlp_input_ids.extend(batch_input_ids)
    
    '''
    # Correctness verification
    count = 0
    for i in range(len(hug_fast_input_ids)):
        if our_input_ids[i] != hug_fast_input_ids[i]:
            print(i)
            count += 1
    
    print("")
    print(count)
    '''
   
    #print("huggingface tokenizer time usage: {}s".format(hug_time_usage))
    print("huggingface fast tokenizer time usage: {}s".format(hug_fast_time_usage))
    print("our tokenizer time usage: {}s".format(our_time_usage))
    print("paddlenlp faster tokenizer time usage: {}s".format(paddle_time_usage))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--vocab_path", required=True, type=str)
    parser.add_argument("--data_path", required=True, type=str)
    parser.add_argument("--num_threads", type=int, default=1)
    parser.add_argument("--batch_size", type=int, default=1)
    parser.add_argument("--do_lower_case", action="store_true")
    args = parser.parse_args()

    main(args)
