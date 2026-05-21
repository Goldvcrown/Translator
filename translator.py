import torch
import sentencepiece as spm

from config import NMTConfig
from model import create_model


class Translator:

    def __init__(self):

        self.cfg = NMTConfig()

        self.device = torch.device(
            self.cfg.device if torch.cuda.is_available() else "cpu"
        )

        print("Using device:", self.device)

        self.sp_src = spm.SentencePieceProcessor()
        self.sp_src.load(
            f"{self.cfg.sp_dir}/{self.cfg.src_model_prefix}.model"
        )

        self.sp_tgt = spm.SentencePieceProcessor()
        self.sp_tgt.load(
            f"{self.cfg.sp_dir}/{self.cfg.tgt_model_prefix}.model"
        )

        self.model = create_model(self.cfg).to(self.device)

        ckpt_path = f"{self.cfg.ckpt_dir}/{self.cfg.ckpt_name}"

        checkpoint = torch.load(
            ckpt_path,
            map_location=self.device,
            weights_only=False
        )

        self.model.load_state_dict(
            checkpoint["model_state_dict"]
        )

        self.model.eval()

        print("Model loaded successfully!")

    def translate(self, text, max_len=128):

        src_ids = self.sp_src.encode(text)

        src_ids.append(self.cfg.eos_id)

        src = torch.tensor(
            [src_ids],
            dtype=torch.long
        ).to(self.device)

        tgt_ids = [self.cfg.sos_id]

        with torch.no_grad():

            for _ in range(max_len):

                tgt_tensor = torch.tensor(
                    [tgt_ids],
                    dtype=torch.long
                ).to(self.device)

                output = self.model(src, tgt_tensor)

                next_token = output[0, -1].argmax(-1).item()
                print("Predicted:", next_token)
                tgt_ids.append(next_token)

                if next_token == self.cfg.eos_id:
                    break

        clean_ids = []

        for token in tgt_ids:

            if token in [
                self.cfg.sos_id,
                self.cfg.eos_id,
                self.cfg.pad_id
            ]:
                continue

            clean_ids.append(token)

        translated = self.sp_tgt.decode(clean_ids)

        return translated


if __name__ == "__main__":

    translator = Translator()

    print("\nEnglish → Vietnamese Translator")
    print("Type 'quit' to exit.\n")

    while True:

        text = input("English: ")

        if text.lower() == "quit":
            break

        result = translator.translate(text)

        print("Vietnamese:", result)
        print()