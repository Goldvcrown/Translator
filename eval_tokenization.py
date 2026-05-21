import sentencepiece as spm
import numpy as np
import matplotlib.pyplot as plt
from datasets import load_from_disk
import os
from config import NMTConfig
from tabulate import tabulate


class TokenizationEvaluator:
    """Evaluate tokenization quality metrics"""
    
    def __init__(self, cfg):
        self.cfg = cfg
        
        # Load SentencePiece models
        self.sp_src = spm.SentencePieceProcessor()
        self.sp_src.Load(f"{cfg.sp_dir}/{cfg.src_model_prefix}.model")
        
        self.sp_tgt = spm.SentencePieceProcessor()
        self.sp_tgt.Load(f"{cfg.sp_dir}/{cfg.tgt_model_prefix}.model")
        
        print(f"   Loaded SentencePiece models")
        print(f"   Source vocab size: {self.sp_src.vocab_size()}")
        print(f"   Target vocab size: {self.sp_tgt.vocab_size()}")
    
    def load_dataset(self, split="test"):
        """Load dataset split"""
        print(f"\nLoading {split} dataset...")
        try:
            dataset = load_from_disk(self.cfg.data_dir)
            data_split = dataset[split]
        except Exception:
            split_path = os.path.join(self.cfg.data_dir, split)
            data_split = load_from_disk(split_path)
        
        src_texts = [item['translation'][self.cfg.src_lang] for item in data_split]
        tgt_texts = [item['translation'][self.cfg.tgt_lang] for item in data_split]
        
        print(f"Loaded {len(src_texts)} sentence pairs")
        return src_texts, tgt_texts
    
    def compute_subword_fertility(self, texts, is_src=True):
        """
        Compute subword fertility (avg tokens per word)
        
        Args:
            texts: List of sentences
            is_src: True for source language, False for target
        
        Returns:
            fertility_scores: List of fertility values per sentence
            avg_fertility: Average fertility across all sentences
            word_counts: List of word counts per sentence
        """
        sp = self.sp_src if is_src else self.sp_tgt
        
        fertility_scores = []
        word_counts = []
        
        for text in texts:
            words = text.strip().split()
            if len(words) == 0:
                continue
            
            tokens = sp.EncodeAsIds(text.strip())
            
            fertility = len(tokens) / len(words)
            fertility_scores.append(fertility)
            word_counts.append(len(words))
        
        avg_fertility = np.mean(fertility_scores)
        
        return fertility_scores, avg_fertility, word_counts
    
    def compute_unk_token_ratio(self, texts, is_src=True):
        """
        Compute unknown token ratio
        
        Args:
            texts: List of sentences
            is_src: True for source language, False for target
        
        Returns:
            unk_ratios: List of unknown token ratios per sentence
            overall_unk_ratio: Overall unknown token ratio
            total_tokens: Total number of tokens processed
            unk_tokens: Total number of unknown tokens
        """
        sp = self.sp_src if is_src else self.sp_tgt
        unk_id = sp.unk_id()  # Usually 0 or 3
        
        unk_ratios = []
        total_tokens = 0
        total_unks = 0
        
        for text in texts:
            tokens = sp.EncodeAsIds(text.strip())
            if len(tokens) == 0:
                continue
            
            num_unks = sum(1 for t in tokens if t == unk_id)
            unk_ratio = num_unks / len(tokens) if len(tokens) > 0 else 0
            
            unk_ratios.append(unk_ratio)
            total_tokens += len(tokens)
            total_unks += num_unks
        
        overall_unk_ratio = (total_unks / total_tokens * 100) if total_tokens > 0 else 0
        
        return unk_ratios, overall_unk_ratio, total_tokens, total_unks
    
    def visualize_metrics(self, src_fertility, src_avg_fert, src_words,
                         tgt_fertility, tgt_avg_fert, tgt_words,
                         src_unk_ratios, src_unk_overall,
                         tgt_unk_ratios, tgt_unk_overall, split="test"):
        """Create visualizations for tokenization metrics"""
        
        fig, axes = plt.subplots(2, 3, figsize=(18, 10))
        fig.suptitle(f'Tokenization Quality Analysis - {split.upper()} Set', 
                     fontsize=16, fontweight='bold')
        
        axes[0, 0].hist(src_fertility, bins=50, color='steelblue', edgecolor='black', alpha=0.7)
        axes[0, 0].axvline(src_avg_fert, color='red', linestyle='--', linewidth=2, 
                          label=f'Mean: {src_avg_fert:.2f}')
        axes[0, 0].set_xlabel('Subword Fertility', fontsize=11)
        axes[0, 0].set_ylabel('Frequency', fontsize=11)
        axes[0, 0].set_title(f'Source Language - Subword Fertility\n(Avg: {src_avg_fert:.2f} tokens/word)', 
                            fontsize=12)
        axes[0, 0].legend()
        axes[0, 0].grid(axis='y', alpha=0.3)
        
        axes[0, 1].hist(tgt_fertility, bins=50, color='lightcoral', edgecolor='black', alpha=0.7)
        axes[0, 1].axvline(tgt_avg_fert, color='red', linestyle='--', linewidth=2, 
                          label=f'Mean: {tgt_avg_fert:.2f}')
        axes[0, 1].set_xlabel('Subword Fertility', fontsize=11)
        axes[0, 1].set_ylabel('Frequency', fontsize=11)
        axes[0, 1].set_title(f'Target Language - Subword Fertility\n(Avg: {tgt_avg_fert:.2f} tokens/word)', 
                            fontsize=12)
        axes[0, 1].legend()
        axes[0, 1].grid(axis='y', alpha=0.3)
        
        axes[0, 2].boxplot([src_fertility, tgt_fertility], labels=['Source', 'Target'],
                          patch_artist=True,
                          boxprops=dict(facecolor='lightblue', alpha=0.7),
                          medianprops=dict(color='red', linewidth=2))
        axes[0, 2].set_ylabel('Subword Fertility', fontsize=11)
        axes[0, 2].set_title('Fertility Comparison\n(Box Plot)', fontsize=12)
        axes[0, 2].grid(axis='y', alpha=0.3)
        
        axes[1, 0].hist([r * 100 for r in src_unk_ratios], bins=50, 
                       color='steelblue', edgecolor='black', alpha=0.7)
        axes[1, 0].axvline(src_unk_overall, color='red', linestyle='--', linewidth=2,
                          label=f'Overall: {src_unk_overall:.2f}%')
        axes[1, 0].set_xlabel('Unknown Token Ratio (%)', fontsize=11)
        axes[1, 0].set_ylabel('Frequency', fontsize=11)
        axes[1, 0].set_title(f'Source Language - Unknown Token Ratio\n(Overall: {src_unk_overall:.2f}%)', 
                            fontsize=12)
        axes[1, 0].legend()
        axes[1, 0].grid(axis='y', alpha=0.3)
        
        axes[1, 1].hist([r * 100 for r in tgt_unk_ratios], bins=50,
                       color='lightcoral', edgecolor='black', alpha=0.7)
        axes[1, 1].axvline(tgt_unk_overall, color='red', linestyle='--', linewidth=2,
                          label=f'Overall: {tgt_unk_overall:.2f}%')
        axes[1, 1].set_xlabel('Unknown Token Ratio (%)', fontsize=11)
        axes[1, 1].set_ylabel('Frequency', fontsize=11)
        axes[1, 1].set_title(f'Target Language - Unknown Token Ratio\n(Overall: {tgt_unk_overall:.2f}%)', 
                            fontsize=12)
        axes[1, 1].legend()
        axes[1, 1].grid(axis='y', alpha=0.3)
        
        axes[1, 2].boxplot([[r * 100 for r in src_unk_ratios], 
                           [r * 100 for r in tgt_unk_ratios]], 
                          labels=['Source', 'Target'],
                          patch_artist=True,
                          boxprops=dict(facecolor='lightblue', alpha=0.7),
                          medianprops=dict(color='red', linewidth=2))
        axes[1, 2].set_ylabel('Unknown Token Ratio (%)', fontsize=11)
        axes[1, 2].set_title('Unknown Token Ratio Comparison\n(Box Plot)', fontsize=12)
        axes[1, 2].grid(axis='y', alpha=0.3)
        
        plt.tight_layout()
        
        import datetime
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        plot_path = f"tokenization_analysis_{split}_{timestamp}.png"
        plt.savefig(plot_path, dpi=150, bbox_inches='tight')
        print(f"\nVisualization saved to: {plot_path}")
        plt.close()
    
    def print_statistics(self, src_fertility, src_avg_fert, src_words, src_unk_ratios, src_unk_overall,
                        tgt_fertility, tgt_avg_fert, tgt_words, tgt_unk_ratios, tgt_unk_overall):
        """Print detailed statistics"""
        
        print("\n" + "=" * 90)
        print("TOKENIZATION QUALITY ANALYSIS")
        print("=" * 90)
        
        print("\n" + "-" * 90)
        print("SUBWORD FERTILITY (tokens per word)")
        print("-" * 90)
        
        stats_fertility = [
            ["Metric", "Source (EN)", "Target (VI)"],
            ["Mean Fertility", f"{src_avg_fert:.3f}", f"{tgt_avg_fert:.3f}"],
            ["Median Fertility", f"{np.median(src_fertility):.3f}", f"{np.median(tgt_fertility):.3f}"],
            ["Min Fertility", f"{np.min(src_fertility):.3f}", f"{np.min(tgt_fertility):.3f}"],
            ["Max Fertility", f"{np.max(src_fertility):.3f}", f"{np.max(tgt_fertility):.3f}"],
            ["Std Dev", f"{np.std(src_fertility):.3f}", f"{np.std(tgt_fertility):.3f}"],
            ["Total Sentences", f"{len(src_fertility)}", f"{len(tgt_fertility)}"],
        ]
        
        print(tabulate(stats_fertility, headers="firstrow", tablefmt="grid"))
        
        print("\n" + "-" * 90)
        print("UNKNOWN TOKEN RATIO")
        print("-" * 90)
        
        src_unk_array = np.array(src_unk_ratios) * 100
        tgt_unk_array = np.array(tgt_unk_ratios) * 100
        
        stats_unk = [
            ["Metric", "Source (EN)", "Target (VI)"],
            ["Overall UNK Ratio (%)", f"{src_unk_overall:.3f}%", f"{tgt_unk_overall:.3f}%"],
            ["Mean UNK Ratio (%)", f"{np.mean(src_unk_array):.3f}%", f"{np.mean(tgt_unk_array):.3f}%"],
            ["Median UNK Ratio (%)", f"{np.median(src_unk_array):.3f}%", f"{np.median(tgt_unk_array):.3f}%"],
            ["Max UNK Ratio (%)", f"{np.max(src_unk_array):.3f}%", f"{np.max(tgt_unk_array):.3f}%"],
            ["Sentences with 0% UNK", f"{sum(1 for r in src_unk_ratios if r == 0)}", 
             f"{sum(1 for r in tgt_unk_ratios if r == 0)}"],
            ["Sentences with >5% UNK", f"{sum(1 for r in src_unk_ratios if r > 0.05)}", 
             f"{sum(1 for r in tgt_unk_ratios if r > 0.05)}"],
        ]
        
        print(tabulate(stats_unk, headers="firstrow", tablefmt="grid"))
        
        print("\n" + "=" * 90)
        print("📌 SUMMARY & INTERPRETATION")
        print("=" * 90)
        
        print(f"""
✓ Subword Fertility:
  - Source language: {src_avg_fert:.2f} tokens/word (lower is better - less fragmentation)
  - Target language: {tgt_avg_fert:.2f} tokens/word
  - Interpretation: Measures how much words are split into subwords.
    Typical range: 1.0-2.0 (1.0 = each word is one token, >2.0 = heavy fragmentation)

✓ Unknown Token Ratio:
  - Source language: {src_unk_overall:.3f}% (very low is excellent)
  - Target language: {tgt_unk_overall:.3f}%
  - Interpretation: Percentage of tokens that are unknown (<unk>).
    Target: <1% or even 0% for good coverage
    If >5%: vocabulary might be too small or data quality issues
""")
        
        print("=" * 90)


def main():
    cfg = NMTConfig()
    evaluator = TokenizationEvaluator(cfg)
    
    src_texts, tgt_texts = evaluator.load_dataset("test")
    
    print("\n" + "=" * 90)
    print("Computing Subword Fertility...")
    print("=" * 90)
    src_fertility, src_avg_fert, src_words = evaluator.compute_subword_fertility(src_texts, is_src=True)
    tgt_fertility, tgt_avg_fert, tgt_words = evaluator.compute_subword_fertility(tgt_texts, is_src=False)
    
    print("\n" + "=" * 90)
    print("Computing Unknown Token Ratio...")
    print("=" * 90)
    src_unk_ratios, src_unk_overall, src_total_tokens, src_total_unks = \
        evaluator.compute_unk_token_ratio(src_texts, is_src=True)
    tgt_unk_ratios, tgt_unk_overall, tgt_total_tokens, tgt_total_unks = \
        evaluator.compute_unk_token_ratio(tgt_texts, is_src=False)
    
    print(f"\nSource language tokenization:")
    print(f"   Total tokens: {src_total_tokens:,} | Unknown tokens: {src_total_unks}")
    print(f"Target language tokenization:")
    print(f"   Total tokens: {tgt_total_tokens:,} | Unknown tokens: {tgt_total_unks}")
    
    evaluator.print_statistics(src_fertility, src_avg_fert, src_words, 
                              src_unk_ratios, src_unk_overall,
                              tgt_fertility, tgt_avg_fert, tgt_words,
                              tgt_unk_ratios, tgt_unk_overall)
    
    evaluator.visualize_metrics(src_fertility, src_avg_fert, src_words,
                               tgt_fertility, tgt_avg_fert, tgt_words,
                               src_unk_ratios, src_unk_overall,
                               tgt_unk_ratios, tgt_unk_overall, split="test")


if __name__ == "__main__":
    main()
