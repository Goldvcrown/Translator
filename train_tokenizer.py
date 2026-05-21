import os
import sentencepiece as spm
from datasets import load_from_disk
from config import NMTConfig


def save_corpus_to_file(texts, filepath):
    """Lưu corpus vào file text để train sentencepiece"""
    with open(filepath, 'w', encoding='utf-8') as f:
        for text in texts:
            f.write(text.strip() + '\n')


def train_sentencepiece(cfg, is_src=True):
    """
    Huấn luyện SentencePiece tokenizer
    
    Args:
        cfg: NMTConfig object
        is_src: True nếu train cho source language (English), False cho target (Vietnamese)
    """
    print(f"Loading dataset from {cfg.data_dir}")
    dataset = load_from_disk(cfg.data_dir)
    
    train_data = dataset['train']
    
    lang_key = 'en' if is_src else 'vi'
    model_prefix = cfg.src_model_prefix if is_src else cfg.tgt_model_prefix
    
    temp_file = f"./temp_corpus_{lang_key}.txt"
    
    corpus = [item['translation'][lang_key] for item in train_data]
    save_corpus_to_file(corpus, temp_file)
    
    print(f"===> Processing {lang_key.upper()} corpus: {len(corpus)} sentences")
    
    if not os.path.exists(cfg.sp_dir):
        os.makedirs(cfg.sp_dir)
    
    template = (
        "--input={} "
        "--model_prefix={} "
        "--vocab_size={} "
        "--character_coverage={} "
        "--model_type={} "
        "--pad_id={} "
        "--bos_id={} "
        "--eos_id={} "
        "--unk_id={}"
    )
    
    sp_config = template.format(
        temp_file,
        f"{cfg.sp_dir}/{model_prefix}",
        cfg.sp_vocab_size,
        cfg.character_coverage,
        cfg.model_type,
        cfg.pad_id,
        cfg.sos_id,
        cfg.eos_id,
        cfg.unk_id
    )
    
    print(f"Training SentencePiece for {lang_key.upper()}...")
    spm.SentencePieceTrainer.Train(sp_config)
    print(f"Model saved to {cfg.sp_dir}/{model_prefix}.model")
    
    if os.path.exists(temp_file):
        os.remove(temp_file)


def main():
    cfg = NMTConfig()
    
    print("=" * 50)
    print("Training English (source) tokenizer...")
    print("=" * 50)
    train_sentencepiece(cfg, is_src=True)
    
    print("\n" + "=" * 50)
    print("Training Vietnamese (target) tokenizer...")
    print("=" * 50)
    train_sentencepiece(cfg, is_src=False)
    
    print("\n✓ Tokenizer training completed!")


if __name__ == "__main__":
    main()