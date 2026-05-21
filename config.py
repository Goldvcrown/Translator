class NMTConfig:
    """Configuration for Neural Machine Translation"""
    
    def __init__(self):
        # Dataset paths
        self.data_dir = "./hf_dataset"
        
        # Language settings
        self.src_lang = 'en'
        self.tgt_lang = 'vi'
        
        # Tokenizer settings
        self.sp_dir = './sentencepiece_models'
        self.src_model_prefix = 'sp_en'
        self.tgt_model_prefix = 'sp_vi'
        self.sp_vocab_size = 10000
        self.character_coverage = 1.0
        self.model_type = 'unigram' 
        
        # Special tokens
        self.pad_id = 0
        self.sos_id = 1
        self.eos_id = 2
        self.unk_id = 3
        
        # Model hyperparameters 
        self.num_heads = 4
        self.num_layers = 6
        self.d_model = 128
        self.d_ff = 512
        self.dropout = 0.1
        self.seq_len = 128
        
        # Training settings 
        self.batch_size = 32
        self.learning_rate = 5e-4
        self.num_epochs = 10 
        self.device = 'cuda'
        self.num_workers = 0
        self.gradient_accumulation_steps = 8
        
        # Checkpoint settings
        self.ckpt_dir = './save_model'
        self.ckpt_name = 'best_ckpt.tar'
        self.beam_size = 3

