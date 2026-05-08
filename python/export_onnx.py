import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

import torch
import torch.onnx
import onnx
from model import ChessNet

def export_to_onnx(model_path='chess_model_v1.pt', output_path='chess_model.onnx'):
    model = ChessNet(num_blocks=5, channels=128)
    
    try:
        model.load_state_dict(torch.load(model_path, weights_only=True))
        print(f"Loaded weights from {model_path}")
    except FileNotFoundError:
        print(f"Warning: {model_path} not found. Exporting uninitialized model.")
    
    model.eval()
    # Dummy input matching the expected shape: [batch_size, planes, ranks, files]
    dummy_input = torch.randn(1, 119, 8, 8)
    
    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        input_names=['board_state'],
        output_names=['policy', 'value'],
        opset_version=14,
        do_constant_folding=True
    )
    
    # Downgrade IR version for ONNX Runtime 1.17.1 compatibility
    onnx_model = onnx.load(output_path)
    onnx_model.ir_version = 9
    onnx.save(onnx_model, output_path)
    
    print(f"Model exported successfully to {output_path} with IR version 9")

if __name__ == "__main__":
    export_to_onnx()
