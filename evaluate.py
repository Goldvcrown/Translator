import torch
import sentencepiece as spm
from sacrebleu.metrics import BLEU
from tqdm import tqdm
import os
import json
import csv
from datetime import datetime
import matplotlib.pyplot as plt
import numpy as np
from tabulate import tabulate

from config import NMTConfig
from model import create_model
from dataset_loader import get_data_loader
from torch.serialization import add_safe_globals


class Evaluator:
    def __init__(self, cfg, checkpoint_path):
        self.cfg = cfg
        self.device = torch.device(
            cfg.device if torch.cuda.is_available() else "cpu"
        )
        print(f"Using device: {self.device}")

        add_safe_globals([NMTConfig])

        self.sp_src = spm.SentencePieceProcessor()
        self.sp_src.Load(f"{cfg.sp_dir}/{cfg.src_model_prefix}.model")

        self.sp_tgt = spm.SentencePieceProcessor()
        self.sp_tgt.Load(f"{cfg.sp_dir}/{cfg.tgt_model_prefix}.model")

        self.model = create_model(cfg).to(self.device)

        if not os.path.exists(checkpoint_path):
            raise FileNotFoundError(
                f"❌ Checkpoint not found: {checkpoint_path}"
            )

        try:
            checkpoint = torch.load(
                checkpoint_path,
                map_location=self.device,
                weights_only=True
            )
            print("Loaded checkpoint with weights_only=True (safe mode)")
        except Exception as e:
            print("Safe load failed, falling back to weights_only=False (trusted)")
            print(f"Reason: {e}")
            checkpoint = torch.load(
                checkpoint_path,
                map_location=self.device,
                weights_only=False
            )

        state_dict = checkpoint.get("model_state_dict", checkpoint)
        self.model.load_state_dict(state_dict)
        self.model.eval()

        print(f"  Loaded checkpoint from {checkpoint_path}")

    def greedy_translate(self, sentence):
        """Greedy decoding for one sentence"""

        src_ids = (
            [self.cfg.sos_id]
            + self.sp_src.EncodeAsIds(sentence.strip())
            + [self.cfg.eos_id]
        )

        src_ids = src_ids[:self.cfg.seq_len]
        src_ids += [self.cfg.pad_id] * (self.cfg.seq_len - len(src_ids))

        src = torch.LongTensor(src_ids).unsqueeze(0).to(self.device)

        tgt_ids = [self.cfg.sos_id]

        for _ in range(self.cfg.seq_len - 1):
            tgt_input = tgt_ids + [self.cfg.pad_id] * (
                self.cfg.seq_len - len(tgt_ids)
            )
            tgt = torch.LongTensor(tgt_input).unsqueeze(0).to(self.device)

            with torch.no_grad():
                output = self.model(src, tgt)

            next_token = output[0, len(tgt_ids) - 1].argmax(dim=-1).item()

            if next_token == self.cfg.eos_id:
                break

            tgt_ids.append(next_token)

        return self.sp_tgt.DecodeIds(tgt_ids[1:])

    def compute_sentence_bleu(self, pred, ref):
        """Tính BLEU score cho một câu"""
        bleu = BLEU()
        score = bleu.sentence_score(pred, [ref])
        return score.score

    def visualize_results(self, predictions, references, src_texts, bleu_score, split="test"):
        """Trực quan hóa kết quả đánh giá"""
        
        sentence_bleus = []
        for pred, ref in zip(predictions, references):
            bleu = self.compute_sentence_bleu(pred, ref[0])
            sentence_bleus.append(bleu)
        
        sentence_bleus = np.array(sentence_bleus)
        
        print("\n" + "=" * 80)
        print("📊 EVALUATION STATISTICS")
        print("=" * 80)
        
        stats = [
            ["Metric", "Value"],
            ["Corpus BLEU Score", f"{bleu_score.score:.2f}"],
            ["Average Sentence BLEU", f"{sentence_bleus.mean():.2f}"],
            ["Min Sentence BLEU", f"{sentence_bleus.min():.2f}"],
            ["Max Sentence BLEU", f"{sentence_bleus.max():.2f}"],
            ["Std Dev", f"{sentence_bleus.std():.2f}"],
            ["Total Sentences", f"{len(predictions)}"],
        ]
        
        print(tabulate(stats, headers="firstrow", tablefmt="grid"))
        
        print("\n" + "=" * 80)
        print("📝 SAMPLE TRANSLATIONS (Best & Worst)")
        print("=" * 80)
        
        best_idx = np.argsort(sentence_bleus)[-3:][::-1]
        worst_idx = np.argsort(sentence_bleus)[:3]
        
        print("\n✅ BEST TRANSLATIONS:")
        for idx in best_idx:
            print(f"\n[BLEU: {sentence_bleus[idx]:.2f}]")
            print(f"  Source:      {src_texts[idx]}")
            print(f"  Reference:   {references[idx][0]}")
            print(f"  Predicted:   {predictions[idx]}")
        
        print("\n\n❌ WORST TRANSLATIONS:")
        for idx in worst_idx:
            print(f"\n[BLEU: {sentence_bleus[idx]:.2f}]")
            print(f"  Source:      {src_texts[idx]}")
            print(f"  Reference:   {references[idx][0]}")
            print(f"  Predicted:   {predictions[idx]}")
        
        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        fig.suptitle(f"Evaluation Results - {split.upper()} Set", fontsize=16, fontweight='bold')
        
        axes[0, 0].hist(sentence_bleus, bins=30, color='skyblue', edgecolor='black', alpha=0.7)
        axes[0, 0].axvline(sentence_bleus.mean(), color='red', linestyle='--', linewidth=2, label=f'Mean: {sentence_bleus.mean():.2f}')
        axes[0, 0].axvline(bleu_score.score, color='green', linestyle='--', linewidth=2, label=f'Corpus: {bleu_score.score:.2f}')
        axes[0, 0].set_xlabel('BLEU Score', fontsize=11)
        axes[0, 0].set_ylabel('Frequency', fontsize=11)
        axes[0, 0].set_title('Distribution of Sentence BLEU Scores', fontsize=12)
        axes[0, 0].legend()
        axes[0, 0].grid(axis='y', alpha=0.3)
        
        num_show = min(200, len(sentence_bleus))
        axes[0, 1].plot(sentence_bleus[:num_show], color='steelblue', linewidth=1.5, alpha=0.7)
        axes[0, 1].fill_between(range(num_show), sentence_bleus[:num_show], alpha=0.3)
        axes[0, 1].axhline(sentence_bleus.mean(), color='red', linestyle='--', label=f'Mean: {sentence_bleus.mean():.2f}')
        axes[0, 1].set_xlabel('Sentence Index', fontsize=11)
        axes[0, 1].set_ylabel('BLEU Score', fontsize=11)
        axes[0, 1].set_title(f'BLEU Scores per Sentence (First {num_show})', fontsize=12)
        axes[0, 1].legend()
        axes[0, 1].grid(axis='y', alpha=0.3)
        
        src_lengths = [len(src.split()) for src in src_texts]
        ref_lengths = [len(ref[0].split()) for ref in references]
        pred_lengths = [len(pred.split()) for pred in predictions]
        
        x = np.arange(3)
        width = 0.25
        axes[1, 0].bar(x - width, [np.mean(src_lengths), np.mean(ref_lengths), np.mean(pred_lengths)], 
                       width, label='Mean', color='lightblue', edgecolor='black')
        axes[1, 0].bar(x, [np.median(src_lengths), np.median(ref_lengths), np.median(pred_lengths)], 
                       width, label='Median', color='lightcoral', edgecolor='black')
        axes[1, 0].set_ylabel('Length (tokens)', fontsize=11)
        axes[1, 0].set_title('Sentence Length Comparison', fontsize=12)
        axes[1, 0].set_xticks(x)
        axes[1, 0].set_xticklabels(['Source', 'Reference', 'Predicted'])
        axes[1, 0].legend()
        axes[1, 0].grid(axis='y', alpha=0.3)
        
        length_buckets = [(0, 10), (10, 20), (20, 30), (30, 50), (50, 100), (100, 1000)]
        bleu_by_length = []
        bucket_labels = []
        
        for start, end in length_buckets:
            mask = (np.array(src_lengths) >= start) & (np.array(src_lengths) < end)
            if mask.sum() > 0:
                bleu_by_length.append(sentence_bleus[mask].mean())
                bucket_labels.append(f'{start}-{end}')
        
        axes[1, 1].bar(bucket_labels, bleu_by_length, color='lightgreen', edgecolor='black', alpha=0.7)
        axes[1, 1].set_xlabel('Source Sentence Length (tokens)', fontsize=11)
        axes[1, 1].set_ylabel('Average BLEU Score', fontsize=11)
        axes[1, 1].set_title('BLEU Score by Sentence Length', fontsize=12)
        axes[1, 1].tick_params(axis='x', rotation=45)
        axes[1, 1].grid(axis='y', alpha=0.3)
        
        plt.tight_layout()
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        plot_path = f"evaluation_results_{split}_{timestamp}.png"
        plt.savefig(plot_path, dpi=150, bbox_inches='tight')
        print(f"\n✅ Visualization saved to: {plot_path}")
        plt.close()
        
        return sentence_bleus

    def export_results(self, predictions, references, src_texts, sentence_bleus, bleu_score, split="test"):
        """Export kết quả thành JSON và CSV"""
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        results = {
            "split": split,
            "timestamp": timestamp,
            "corpus_bleu": float(bleu_score.score),
            "statistics": {
                "mean_bleu": float(sentence_bleus.mean()),
                "min_bleu": float(sentence_bleus.min()),
                "max_bleu": float(sentence_bleus.max()),
                "std_bleu": float(sentence_bleus.std()),
                "total_sentences": int(len(predictions))
            },
            "samples": []
        }
        
        for src, ref, pred, bleu in zip(src_texts, references, predictions, sentence_bleus):
            results["samples"].append({
                "source": src,
                "reference": ref[0],
                "predicted": pred,
                "bleu_score": float(bleu)
            })
        
        json_path = f"evaluation_results_{split}_{timestamp}.json"
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(results, f, ensure_ascii=False, indent=2)
        
        print(f"  Results exported to: {json_path}")
        
        csv_path = f"evaluation_results_{split}_{timestamp}.csv"
        with open(csv_path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(['Source', 'Reference', 'Predicted', 'BLEU Score'])
            for src, ref, pred, bleu in zip(src_texts, references, predictions, sentence_bleus):
                writer.writerow([src, ref[0], pred, f"{bleu:.2f}"])
        
        print(f"  Results exported to: {csv_path}")

    def evaluate_bleu(self, split="test"):
        """Evaluate BLEU score"""

        dataset, _ = get_data_loader(self.cfg, split)

        predictions = []
        references = []
        src_texts = []

        for i in tqdm(range(len(dataset)), desc="Evaluating"):
            pred = self.greedy_translate(dataset.src_texts[i])
            predictions.append(pred)
            references.append([dataset.tgt_texts[i]])
            src_texts.append(dataset.src_texts[i])

        bleu = BLEU()
        score = bleu.corpus_score(predictions, references)

        return score, predictions, references, src_texts


def main():
    cfg = NMTConfig()

    checkpoint_path = r"./save_model/best_ckpt.tar"

    evaluator = Evaluator(
        cfg,
        checkpoint_path=checkpoint_path
    )

    bleu_score, predictions, references, src_texts = evaluator.evaluate_bleu("test")

    print("\n" + "=" * 80)
    print(f"BLEU score (test set): {bleu_score.score:.2f}")
    print("=" * 80)
    
    sentence_bleus = evaluator.visualize_results(predictions, references, src_texts, bleu_score, split="test")
    
    evaluator.export_results(predictions, references, src_texts, sentence_bleus, bleu_score, split="test")


if __name__ == "__main__":
    main()

