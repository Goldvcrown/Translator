import torch
import os
import numpy as np
import sentencepiece as spm
from torch.utils.data import Dataset, DataLoader
from datasets import load_from_disk
from tqdm import tqdm
from config import NMTConfig


def pad_or_truncate(tokenized_sequence, seq_len, pad_id):
    """Padding hoặc truncate sequence về độ dài cố định"""
    if len(tokenized_sequence) < seq_len:
        left = seq_len - len(tokenized_sequence)
        padding = [pad_id] * left
        tokenized_sequence = tokenized_sequence + padding
    else:
        tokenized_sequence = tokenized_sequence[:seq_len]
    return tokenized_sequence


class NMTDataset(Dataset):
    """Dataset cho Neural Machine Translation"""
    
    def __init__(self, cfg, data_type="train"):
        super().__init__()
        self.cfg = cfg
        
        # Load SentencePiece tokenizers
        self.sp_src, self.sp_tgt = self.load_sp_tokenizer()
        
        # Load data từ Hugging Face dataset
        self.src_texts, self.tgt_texts = self.read_data(data_type)
        
        # Tokenize sequences
        print(f"Tokenizing {data_type} data...")
        src_tokenized_sequences = self.texts_to_sequences(self.src_texts, True)
        tgt_input_tokenized_sequences, tgt_output_tokenized_sequences = \
            self.texts_to_sequences(self.tgt_texts, False)
        
        # Convert to tensors
        self.src_data = torch.LongTensor(src_tokenized_sequences)
        self.input_tgt_data = torch.LongTensor(tgt_input_tokenized_sequences)
        self.output_tgt_data = torch.LongTensor(tgt_output_tokenized_sequences)
        
        print(f"Dataset loaded: {len(self)} samples")
    
    def read_data(self, data_type):
        """
        Load dữ liệu từ Hugging Face dataset

        Args:
            data_type: 'train', 'test', hoặc 'validation'
        """

        print(f"Loading {data_type} data from {self.cfg.data_dir}")

        # Try loading a DatasetDict root first; fallback to split subdir
        try:
            dataset = load_from_disk(self.cfg.data_dir)
            data_split = dataset[data_type]

        except Exception:

            split_path = os.path.join(self.cfg.data_dir, data_type)

            if not os.path.exists(split_path):
                raise FileNotFoundError(
                    f"Split path not found: {split_path}. "
                    f"Make sure hf_dataset has '{data_type}' subfolder "
                    f"or is a DatasetDict root."
                )

            print(f"Root load failed; loading split from {split_path}")

            data_split = load_from_disk(split_path)

        # DEBUG MODE
        if data_type == "train":
            data_split = data_split.select(range(5000))

        elif data_type == "validation":
            data_split = data_split.select(range(300))

        elif data_type == "test":
            data_split = data_split.select(range(1000))

        # Extract texts
        src_texts = [
            item['translation'][self.cfg.src_lang]
            for item in data_split
        ]

        tgt_texts = [
            item['translation'][self.cfg.tgt_lang]
            for item in data_split
        ]

        print(f"Loaded {len(src_texts)} sentence pairs")

        return src_texts, tgt_texts
    
    def load_sp_tokenizer(self):
        """Load trained SentencePiece models"""
        sp_src = spm.SentencePieceProcessor()
        sp_src.Load(f"{self.cfg.sp_dir}/{self.cfg.src_model_prefix}.model")
        
        sp_tgt = spm.SentencePieceProcessor()
        sp_tgt.Load(f"{self.cfg.sp_dir}/{self.cfg.tgt_model_prefix}.model")
        
        return sp_src, sp_tgt
    
    def texts_to_sequences(self, texts, is_src=True):
        """
        Chuyển đổi text thành token sequences
        
        Args:
            texts: List of text strings
            is_src: True cho source (encoder input), False cho target (decoder input/output)
        """
        if is_src:
            src_tokenized_sequences = []
            for text in tqdm(texts, desc="Tokenizing source"):
                tokenized = self.sp_src.EncodeAsIds(text.strip())
                sequence = pad_or_truncate(
                    [self.cfg.sos_id] + tokenized + [self.cfg.eos_id],
                    self.cfg.seq_len,
                    self.cfg.pad_id
                )
                src_tokenized_sequences.append(sequence)
            return src_tokenized_sequences
        else:
            tgt_input_tokenized_sequences = []
            tgt_output_tokenized_sequences = []
            
            for text in tqdm(texts, desc="Tokenizing target"):
                tokenized = self.sp_tgt.EncodeAsIds(text.strip())
                
                tgt_input = [self.cfg.sos_id] + tokenized
                tgt_output = tokenized + [self.cfg.eos_id]
                
                tgt_input = pad_or_truncate(tgt_input, self.cfg.seq_len, self.cfg.pad_id)
                tgt_output = pad_or_truncate(tgt_output, self.cfg.seq_len, self.cfg.pad_id)
                
                tgt_input_tokenized_sequences.append(tgt_input)
                tgt_output_tokenized_sequences.append(tgt_output)
            
            return tgt_input_tokenized_sequences, tgt_output_tokenized_sequences
    
    def __getitem__(self, idx):
        """Trả về (src, tgt_input, tgt_output) cho mỗi sample"""
        return self.src_data[idx], self.input_tgt_data[idx], self.output_tgt_data[idx]
    
    def __len__(self):
        return self.src_data.shape[0]


def get_data_loader(cfg, data_type="train"):
    """
    Tạo DataLoader cho training/validation/testing
    
    Args:
        cfg: NMTConfig object
        data_type: 'train', 'validation', hoặc 'test'
    
    Returns:
        dataset, dataloader
    """
    dataset = NMTDataset(cfg, data_type)
    
    shuffle = True if data_type == "train" else False
    
    dataloader = DataLoader(
        dataset,
        batch_size=cfg.batch_size,
        shuffle=shuffle,
        num_workers=cfg.num_workers if hasattr(cfg, 'num_workers') else 4,
        pin_memory=True,  # Faster data transfer to GPU
        persistent_workers=True if (hasattr(cfg, 'num_workers') and cfg.num_workers > 0) else False
    )
    
    return dataset, dataloader


if __name__ == "__main__":
    cfg = NMTConfig()
    
    print("Testing DataLoader...")
    train_dataset, train_loader = get_data_loader(cfg, "train")
    
    for batch in train_loader:
        src, tgt_input, tgt_output = batch
        print(f"\nBatch shapes:")
        print(f"Source: {src.shape}")
        print(f"Target Input: {tgt_input.shape}")
        print(f"Target Output: {tgt_output.shape}")
        break