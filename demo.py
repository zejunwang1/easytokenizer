# coding=utf-8

from easytokenizer import AutoTokenizer

vocab_path = "vocab.txt"
tokenizer = AutoTokenizer(vocab_path, do_lower_case = True)

# encode single text
text = "计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。"
input_ids, offsets = tokenizer.encode(text, add_cls_sep = True, truncation = True, max_length = 512)
print("encode single text:")
print("input_ids:")
print(input_ids)
print("offsets:")
print(offsets)
print("\n")


# encode batch texts
texts = ["计算机科学与技术（Computer Science and Technology）是一门普通高等学校本科专业。",
         "清华大学的[MASK]算机科学与技术专业实力全国第一。"]
input_ids, token_type_ids, attention_mask, offsets = tokenizer.encode(
    texts, num_threads = 1, add_cls_sep = True, padding = True, padding_to_max_length = False,
    truncation = True, max_length = 512)
print("encode batch texts:")
print("input_ids:")
print(input_ids)
print("attention_mask:")
print(attention_mask)
print("offsets:")
print(offsets)


