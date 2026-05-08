import torch
import torch.nn as nn
import torch.nn.functional as F

class ChessResidualBlock(nn.Module):
    def __init__(self, channels=256):
        super().__init__()
        self.conv1 = nn.Conv2d(channels, channels, kernel_size=3, padding=1)
        self.bn1 = nn.BatchNorm2d(channels)
        self.conv2 = nn.Conv2d(channels, channels, kernel_size=3, padding=1)
        self.bn2 = nn.BatchNorm2d(channels)
    
    def forward(self, x):
        residual = x
        out = F.relu(self.bn1(self.conv1(x)))
        out = self.bn2(self.conv2(out))
        out += residual
        return F.relu(out)

class ChessNet(nn.Module):
    """
    AlphaZero-style CNN for chess.
    Input: 119x8x8 planes representing the board state.
    Output: Policy (4672 possible moves) + Value ([-1, 1]).
    """
    def __init__(self, num_blocks=5, channels=256): 
        # Note: num_blocks defaults to 5 for faster local training. 
        # AlphaZero used 20 or 40.
        super().__init__()
        self.conv_initial = nn.Conv2d(119, channels, kernel_size=3, padding=1)
        self.bn_initial = nn.BatchNorm2d(channels)
        
        self.res_blocks = nn.ModuleList([
            ChessResidualBlock(channels) for _ in range(num_blocks)
        ])
        
        # Policy head
        self.policy_conv = nn.Conv2d(channels, 32, kernel_size=1)
        self.policy_bn = nn.BatchNorm2d(32)
        self.policy_fc = nn.Linear(32 * 8 * 8, 4672)
        
        # Value head
        self.value_conv = nn.Conv2d(channels, 1, kernel_size=1)
        self.value_bn = nn.BatchNorm2d(1)
        self.value_fc1 = nn.Linear(1 * 8 * 8, 128)
        self.value_fc2 = nn.Linear(128, 1)
    
    def forward(self, x):
        out = F.relu(self.bn_initial(self.conv_initial(x)))
        
        for block in self.res_blocks:
            out = block(out)
            
        # Policy output
        policy = F.relu(self.policy_bn(self.policy_conv(out)))
        policy = policy.view(policy.size(0), -1)
        policy = self.policy_fc(policy)
        
        # Value output
        value = F.relu(self.value_bn(self.value_conv(out)))
        value = value.view(value.size(0), -1)
        value = F.relu(self.value_fc1(value))
        value = torch.tanh(self.value_fc2(value))
        
        return policy, value
