import torch
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.data import DataLoader
from dataset import ChessGameDataset
from model import ChessNet
from export_onnx import export_to_onnx

class ChessTrainer:
    def __init__(self, model, device='cuda'):
        self.model = model.to(device)
        self.device = device
        self.optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9, weight_decay=1e-4)
        self.scheduler = optim.lr_scheduler.StepLR(self.optimizer, step_size=10, gamma=0.1)
    
    def train_epoch(self, dataloader, epochs=1):
        self.model.train()
        
        for epoch in range(epochs):
            total_loss = 0
            policy_loss_total = 0
            value_loss_total = 0
            
            for batch_idx, (positions, moves, values) in enumerate(dataloader):
                positions = positions.to(self.device)
                moves = moves.to(self.device)
                values = values.to(self.device).unsqueeze(1)
                
                self.optimizer.zero_grad()
                policy_logits, value_pred = self.model(positions)
                
                policy_loss = F.cross_entropy(policy_logits, moves)
                value_loss = F.mse_loss(value_pred, values)
                loss = policy_loss + value_loss
                
                loss.backward()
                self.optimizer.step()
                
                total_loss += loss.item()
                policy_loss_total += policy_loss.item()
                value_loss_total += value_loss.item()
                
                if batch_idx % 10 == 0:
                    print(f"Epoch {epoch}, Batch {batch_idx}: "
                          f"Total Loss={loss.item():.4f}, "
                          f"Policy Loss={policy_loss.item():.4f}, "
                          f"Value Loss={value_loss.item():.4f}")
            
            self.scheduler.step()
            print(f"Epoch {epoch} complete. Avg Loss: {total_loss/len(dataloader):.4f}")
    
    def save_model(self, filepath):
        torch.save(self.model.state_dict(), filepath)
        print(f"Model saved to {filepath}")

def main():
    device = 'cuda' if torch.cuda.is_available() else 'cpu'
    print(f"Using device: {device}")
    
    # We use fewer blocks and channels here for faster local tests
    model = ChessNet(num_blocks=5, channels=128)
    trainer = ChessTrainer(model, device=device)
    
    # Load data from the root folder where C++ will generate JSONL files
    dataset = ChessGameDataset("games_iteration_*.jsonl")
    if len(dataset) == 0:
        print("No training data found! Run the C++ Phase 2 self-play generation first.")
        return
        
    dataloader = DataLoader(dataset, batch_size=64, shuffle=True)
    print(f"Dataset size: {len(dataset)} moves")
    
    trainer.train_epoch(dataloader, epochs=5)
    trainer.save_model("chess_model_v1.pt")
    
    print("Exporting newly trained model for the C++ engine...")
    export_to_onnx("chess_model_v1.pt", "chess_model.onnx")

if __name__ == "__main__":
    main()
