import torch
import torch.nn as nn
import torch.optim as optim
from tqdm import tqdm
import os
import math
from config import NMTConfig
from model import create_model
from dataset_loader import get_data_loader


class Trainer:
    """Trainer class cho Neural Machine Translation"""
    
    def __init__(self, cfg):
        self.cfg = cfg
        self.device = torch.device(cfg.device if torch.cuda.is_available() else 'cpu')
        
        print(f"Using device: {self.device}")
        
        # Enable cudnn benchmark for faster training
        if torch.cuda.is_available():
            torch.backends.cudnn.benchmark = True
        
        # Load data
        print("\nLoading training data...")
        self.train_dataset, self.train_loader = get_data_loader(cfg, "train")
        
        print("\nLoading validation data...")
        self.val_dataset, self.val_loader = get_data_loader(cfg, "validation")
        
        # Create model
        print("\nInitializing model...")
        self.model = create_model(cfg).to(self.device)
        
        # Use mixed precision training for faster training
        self.use_amp = torch.cuda.is_available()
        self.scaler = torch.cuda.amp.GradScaler() if self.use_amp else None
        
        # Loss and optimizer
        self.criterion = nn.CrossEntropyLoss(ignore_index=cfg.pad_id, label_smoothing=0.1)
        self.optimizer = optim.AdamW(self.model.parameters(), lr=cfg.learning_rate, 
                                      betas=(0.9, 0.98), eps=1e-9, weight_decay=0.01)
        
        # Warmup + Cosine learning rate scheduler
        num_training_steps = len(self.train_loader) * cfg.num_epochs
        num_warmup_steps = num_training_steps // 10
        
        def lr_lambda(current_step):
            if current_step < num_warmup_steps:
                return float(current_step) / float(max(1, num_warmup_steps))
            progress = float(current_step - num_warmup_steps) / float(max(1, num_training_steps - num_warmup_steps))
            return max(0.0, 0.5 * (1.0 + math.cos(math.pi * progress)))
        
        self.scheduler = optim.lr_scheduler.LambdaLR(self.optimizer, lr_lambda)
        
        # Gradient accumulation
        self.gradient_accumulation_steps = getattr(cfg, 'gradient_accumulation_steps', 1)
        
        # Best validation loss tracking
        self.best_val_loss = float('inf')
        
        # Create checkpoint directory
        if not os.path.exists(cfg.ckpt_dir):
            os.makedirs(cfg.ckpt_dir)
        
        print(f"\nModel parameters: {sum(p.numel() for p in self.model.parameters()):,}")
        print(f"Mixed Precision Training: {self.use_amp}")
        print(f"Gradient Accumulation Steps: {self.gradient_accumulation_steps}")
    
    def train_epoch(self, epoch):
        """Train một epoch với mixed precision và gradient accumulation"""
        self.model.train()
        total_loss = 0
        
        pbar = tqdm(self.train_loader, desc=f'Epoch {epoch+1}/{self.cfg.num_epochs}')
        
        self.optimizer.zero_grad()
        
        for batch_idx, (src, tgt_input, tgt_output) in enumerate(pbar):
            src = src.to(self.device, non_blocking=True)
            tgt_input = tgt_input.to(self.device, non_blocking=True)
            tgt_output = tgt_output.to(self.device, non_blocking=True)
            
            # Mixed precision training
            if self.use_amp:
                with torch.cuda.amp.autocast():
                    output = self.model(src, tgt_input)
                    output = output.reshape(-1, output.shape[-1])
                    tgt_output_flat = tgt_output.reshape(-1)
                    loss = self.criterion(output, tgt_output_flat)
                    loss = loss / self.gradient_accumulation_steps
                
                self.scaler.scale(loss).backward()
                
                # Gradient accumulation
                if (batch_idx + 1) % self.gradient_accumulation_steps == 0:
                    self.scaler.unscale_(self.optimizer)
                    torch.nn.utils.clip_grad_norm_(self.model.parameters(), max_norm=1.0)
                    self.scaler.step(self.optimizer)
                    self.scaler.update()
                    self.optimizer.zero_grad()
                    self.scheduler.step()
            else:
                output = self.model(src, tgt_input)
                output = output.reshape(-1, output.shape[-1])
                tgt_output_flat = tgt_output.reshape(-1)
                loss = self.criterion(output, tgt_output_flat)
                loss = loss / self.gradient_accumulation_steps
                
                loss.backward()
                
                if (batch_idx + 1) % self.gradient_accumulation_steps == 0:
                    torch.nn.utils.clip_grad_norm_(self.model.parameters(), max_norm=1.0)
                    self.optimizer.step()
                    self.optimizer.zero_grad()
                    self.scheduler.step()
            
            total_loss += loss.item() * self.gradient_accumulation_steps
            
            # Update progress bar
            pbar.set_postfix({
                'loss': f'{loss.item() * self.gradient_accumulation_steps:.4f}',
                'lr': f'{self.scheduler.get_last_lr()[0]:.2e}'
            })
        
        avg_loss = total_loss / len(self.train_loader)
        return avg_loss
    
    def validate(self):
        """Validation với mixed precision"""
        self.model.eval()
        total_loss = 0
        
        with torch.no_grad():
            for src, tgt_input, tgt_output in tqdm(self.val_loader, desc='Validation'):
                src = src.to(self.device, non_blocking=True)
                tgt_input = tgt_input.to(self.device, non_blocking=True)
                tgt_output = tgt_output.to(self.device, non_blocking=True)
                
                if self.use_amp:
                    with torch.cuda.amp.autocast():
                        output = self.model(src, tgt_input)
                        output = output.reshape(-1, output.shape[-1])
                        tgt_output = tgt_output.reshape(-1)
                        loss = self.criterion(output, tgt_output)
                else:
                    output = self.model(src, tgt_input)
                    output = output.reshape(-1, output.shape[-1])
                    tgt_output = tgt_output.reshape(-1)
                    loss = self.criterion(output, tgt_output)
                
                total_loss += loss.item()
        
        avg_loss = total_loss / len(self.val_loader)
        return avg_loss
    
    def save_checkpoint(self, epoch, val_loss):
        """Lưu checkpoint"""
        checkpoint = {
            'epoch': epoch,
            'model_state_dict': self.model.state_dict(),
            'optimizer_state_dict': self.optimizer.state_dict(),
            'val_loss': val_loss,
            'cfg': self.cfg
        }
        
        checkpoint_path = os.path.join(self.cfg.ckpt_dir, self.cfg.ckpt_name)
        torch.save(checkpoint, checkpoint_path)
        print(f"Checkpoint saved to {checkpoint_path}")
    
    def load_checkpoint(self):
        """Load checkpoint"""
        checkpoint_path = os.path.join(self.cfg.ckpt_dir, self.cfg.ckpt_name)
        
        if os.path.exists(checkpoint_path):
            checkpoint = torch.load(checkpoint_path, map_location=self.device)
            self.model.load_state_dict(checkpoint['model_state_dict'])
            self.optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
            start_epoch = checkpoint['epoch'] + 1
            self.best_val_loss = checkpoint['val_loss']
            print(f"Loaded checkpoint from epoch {checkpoint['epoch']}")
            return start_epoch
        else:
            print("No checkpoint found, starting from scratch")
            return 0
    
    def train(self):
        """Main training loop"""
        start_epoch = 0 
        
        print("\n" + "="*50)
        print("Starting Training")
        print("="*50)
        
        for epoch in range(start_epoch, self.cfg.num_epochs):
            train_loss = self.train_epoch(epoch)
            val_loss = self.validate()
            
            print(f"\nEpoch {epoch+1}/{self.cfg.num_epochs}")
            print(f"Train Loss: {train_loss:.4f}")
            print(f"Val Loss: {val_loss:.4f}")
            print(f"Learning Rate: {self.scheduler.get_last_lr()[0]:.2e}")
            
            if val_loss < self.best_val_loss:
                self.best_val_loss = val_loss
                self.save_checkpoint(epoch, val_loss)
                print(f"✓ New best model saved!")
            
            if epoch > 5 and val_loss > self.best_val_loss * 1.1:
                print(f"Early stopping triggered at epoch {epoch+1}")
                break
            
            print("-"*50)
        
        print("\n" + "="*50)
        print("Training Completed!")
        print(f"Best Validation Loss: {self.best_val_loss:.4f}")
        print("="*50)


def main():
    cfg = NMTConfig()
    trainer = Trainer(cfg)
    trainer.train()


if __name__ == "__main__":
    main()