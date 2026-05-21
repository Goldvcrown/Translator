import torch
import os
from config import NMTConfig
from model import create_model
from torch.serialization import add_safe_globals


def inspect_checkpoint():
    add_safe_globals([NMTConfig])

    cfg = NMTConfig()
    device = torch.device(cfg.device if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")

    ckpt_path = os.path.join(cfg.ckpt_dir, cfg.ckpt_name)

    if not os.path.exists(ckpt_path):
        print(f"❌ Checkpoint not found: {ckpt_path}")
        return

    try:
        checkpoint = torch.load(ckpt_path, map_location=device, weights_only=True)
        print("\nLoaded with weights_only=True (safe mode)")
    except Exception as e:
        print("\nSafe load failed, falling back to weights_only=False (trusted)")
        print(f"Reason: {e}")
        checkpoint = torch.load(ckpt_path, map_location=device, weights_only=False)

    print("\n" + "=" * 70)
    print("CHECKPOINT LOADED SUCCESSFULLY (PyTorch ≥ 2.6)")
    print("=" * 70)

    print(f"Path       : {ckpt_path}")
    if 'epoch' in checkpoint:
        print(f"Epoch      : {checkpoint['epoch']}")
    if 'val_loss' in checkpoint:
        try:
            print(f"Val loss   : {checkpoint['val_loss']:.6f}")
        except Exception:
            print(f"Val loss   : {checkpoint['val_loss']}")

    print("\nKeys:")
    for k in checkpoint.keys():
        print(f" - {k}")

    # Load model
    model = create_model(cfg).to(device)
    state_dict = checkpoint.get("model_state_dict", checkpoint)
    model.load_state_dict(state_dict)
    model.eval()

    total_params = sum(p.numel() for p in model.parameters())
    print(f"\nModel loaded successfully")
    print(f"Total params: {total_params:,}")

    print("\nCHECKPOINT IS VALID AND READY")


if __name__ == "__main__":
    inspect_checkpoint()



