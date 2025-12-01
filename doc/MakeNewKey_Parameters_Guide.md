# MakeNewKey() Parameters and Signature Algorithm Mapping

## Overview

The `CKey::MakeNewKey(unsigned int config_value)` function generates new key pairs, where the `config_value` parameter determines which Post-Quantum Cryptography (PQC) signature algorithm to use.

This document provides detailed information about the signature algorithms corresponding to different parameter values and their characteristics.

---

## Configuration Value Calculation

```cpp
config_value = (type << 8) | subtype
```

Where:
- `type`: Algorithm type (0=Classic Rainbow, 1=RainbowPro series, 2=Hybrid algorithm)
- `subtype`: Subtype indicating specific algorithm variant

---

## Complete Configuration List

### Overview Table

| Config | Type | SubType | Algorithm Name | Private Key (B) | Public Key (B) | Hash Size (B) | Signature (B) | Notes |
|--------|------|---------|---------------|-----------------|----------------|---------------|---------------|-------|
| **0** | 0 | 0 | **PQC(Rainbow_SL0.8)** | 100,209 | 152,097 | 32 | 64 | Classic Rainbow |
| **262** | 1 | 6 | **PQC(UOV_SL1)** | 70 | 43,598 | 44 | 172 | UOV Security Level 1 |
| **265** | 1 | 9 | **PQC(UOV_SL3)** | 70 | 189,254 | 72 | 272 | UOV Security Level 3 |
| **268** | 1 | 12 | **PQC(UOV_SL5)** | 70 | 447,014 | 96 | 356 | UOV Security Level 5 |
| **274** | 1 | 18 | **PQC(Rainbow_SL1)** | 70 | 48,470 | 28 | 80 | Rainbow Compressed SL1 ⭐Default |
| **280** | 1 | 24 | **PQC(Rainbow_SL3)** | 70 | 207,430 | 72 | 192 | Rainbow Compressed SL3 |
| **286** | 1 | 30 | **PQC(Rainbow_SL5)** | 70 | 604,358 | 96 | 268 | Rainbow Compressed SL5 |
| **513** | 2 | 1 | **PQC(Mix:Rainbow_SL5\|UOV_SL5)** | 146 | 1,051,378 | 192 | 624 | Hybrid (Highest Security) |

> **Note**: Default configuration `DEFAULT_CONFIG = 4`, corresponding to `config_value = 274` (Rainbow SL1)

---

## Grouped by Algorithm Type

### 1. Classic Rainbow Algorithm

#### `config_value = 0`
- **Algorithm**: Rainbow Security Level 0.8 (RAINBOW16_32_32_32)
- **Full Name**: L1_RAINBOW_CLASSIC
- **Features**: 
  - Original Rainbow signature algorithm
  - Smallest signature (64 bytes)
  - Large private and public keys
- **Use Case**: Legacy compatibility

```cpp
CKey key;
key.MakeNewKey(0);  // Classic Rainbow SL0.8
```

---

### 2. UOV (Unbalanced Oil and Vinegar) Series

UOV is a post-quantum signature algorithm based on multivariate quadratic equations.

#### `config_value = 262` (Type=1, SubType=6)
- **Algorithm**: UOV Security Level 1
- **Full Name**: L1_UOV_COMPRESSED (RAINBOW256_112_44_00)
- **Features**:
  - Minimal private key (70 bytes)
  - Medium public key (43,598 bytes)
  - 172-byte signature
- **Security Level**: NIST Level 1

```cpp
CKey key;
key.MakeNewKey(262);  // UOV SL1
```

#### `config_value = 265` (Type=1, SubType=9)
- **Algorithm**: UOV Security Level 3
- **Full Name**: L3_UOV_COMPRESSED (RAINBOW256_184_72_00)
- **Features**:
  - Minimal private key (70 bytes)
  - Larger public key (189,254 bytes)
  - 272-byte signature
- **Security Level**: NIST Level 3

```cpp
CKey key;
key.MakeNewKey(265);  // UOV SL3
```

#### `config_value = 268` (Type=1, SubType=12)
- **Algorithm**: UOV Security Level 5
- **Full Name**: L5_UOV_COMPRESSED (RAINBOW256_244_96_00)
- **Features**:
  - Minimal private key (70 bytes)
  - Large public key (447,014 bytes)
  - 356-byte signature
- **Security Level**: NIST Level 5 (Highest)

```cpp
CKey key;
key.MakeNewKey(268);  // UOV SL5
```

---

### 3. Rainbow Compressed Series

Rainbow compressed version reduces key sizes through optimization while maintaining security.

#### `config_value = 274` (Type=1, SubType=18) ⭐ **Recommended/Default**
- **Algorithm**: Rainbow Security Level 1
- **Full Name**: L1_RAINBOW_COMPRESSED (RAINBOW16_72_8_48)
- **Features**:
  - Minimal private key (70 bytes)
  - Moderate public key (48,470 bytes)
  - Small signature (80 bytes)
  - **System default configuration**
- **Security Level**: NIST Level 1

```cpp
CKey key;
key.MakeNewKey(274);  // Rainbow SL1 (Default recommended)
```

#### `config_value = 280` (Type=1, SubType=24)
- **Algorithm**: Rainbow Security Level 3
- **Full Name**: L3_RAINBOW_COMPRESSED (RAINBOW256_104_8_64)
- **Features**:
  - Minimal private key (70 bytes)
  - Larger public key (207,430 bytes)
  - 192-byte signature
- **Security Level**: NIST Level 3

```cpp
CKey key;
key.MakeNewKey(280);  // Rainbow SL3
```

#### `config_value = 286` (Type=1, SubType=30)
- **Algorithm**: Rainbow Security Level 5
- **Full Name**: L5_RAINBOW_COMPRESSED (RAINBOW256_148_8_96)
- **Features**:
  - Minimal private key (70 bytes)
  - Very large public key (604,358 bytes)
  - 268-byte signature
- **Security Level**: NIST Level 5 (Highest)

```cpp
CKey key;
key.MakeNewKey(286);  // Rainbow SL5
```

---

### 4. Hybrid Algorithm

#### `config_value = 513` (Type=2, SubType=1)
- **Algorithm**: Mix (Rainbow SL5 + UOV SL5)
- **Full Name**: MIX_SIGN
- **Features**:
  - Combines advantages of Rainbow SL5 and UOV SL5
  - Small private key (146 bytes)
  - Largest public key (1,051,378 bytes ≈ 1MB)
  - Largest signature (624 bytes)
  - **Highest security**
- **Security Level**: NIST Level 5 (Dual protection)

```cpp
CKey key;
key.MakeNewKey(513);  // Hybrid algorithm (Highest security)
```

---

## Grouped by Security Level

### Security Level 0.8
- **config_value = 0**: Rainbow Classic

### Security Level 1 (NIST Level 1)
- **config_value = 262**: UOV SL1
- **config_value = 274**: Rainbow SL1 ⭐ **Default Recommended**

### Security Level 3 (NIST Level 3)
- **config_value = 265**: UOV SL3
- **config_value = 280**: Rainbow SL3

### Security Level 5 (NIST Level 5) - Highest Security
- **config_value = 268**: UOV SL5
- **config_value = 286**: Rainbow SL5
- **config_value = 513**: Mix (Rainbow SL5 + UOV SL5) 🔒 **Strongest**

---

## Usage Examples

### Basic Usage

```cpp
#include "key.h"

int main() {
    CKey key;
    
    // Use default configuration (recommended)
    key.MakeNewKey(274);  // Rainbow SL1
    
    // Get key information
    CPubKey pubKey = key.GetPubKey();
    CPrivKey privKey = key.GetPrivKey();
    
    std::cout << "Private key size: " << privKey.size() << " bytes" << std::endl;
    std::cout << "Public key size: " << pubKey.vchPubKey.size() << " bytes" << std::endl;
    
    return 0;
}
```

### Choose Algorithm Based on Requirements

```cpp
// Minimize signature size
key.MakeNewKey(274);  // Rainbow SL1: 80-byte signature

// Minimize private key size
key.MakeNewKey(262);  // UOV SL1: 70-byte private key

// Maximize security
key.MakeNewKey(513);  // Mix: Dual protection

// Balance performance and security
key.MakeNewKey(280);  // Rainbow SL3
```

### Use in Transaction Testing

```cpp
// In transactionTest.cpp
CBasicKeyStore keystore;
CKey key[4];

for (int i = 0; i < 4; i++) {
    // Can use different algorithms for each key
    key[i].MakeNewKey(274);  // Use Rainbow SL1
    keystore.AddKey(key[i]);
}
```

---

## Performance and Size Comparison

### Private Key Size Comparison
| Algorithm | Private Key Size | Rank |
|-----------|------------------|------|
| UOV Series | 70 bytes | 🥇 Smallest |
| Rainbow Compressed | 70 bytes | 🥇 Smallest |
| Mix Hybrid | 146 bytes | Small |
| Classic Rainbow | 100,209 bytes | ❌ Largest |

### Public Key Size Comparison
| Algorithm | Public Key Size | Rank |
|-----------|-----------------|------|
| UOV SL1 | 43,598 bytes | 🥇 Smallest |
| Rainbow SL1 | 48,470 bytes | 🥈 Smaller |
| Classic Rainbow | 152,097 bytes | Medium |
| Rainbow SL3 | 207,430 bytes | Large |
| UOV SL5 | 447,014 bytes | Larger |
| Rainbow SL5 | 604,358 bytes | Very Large |
| Mix | 1,051,378 bytes | ❌ Largest |

### Signature Size Comparison
| Algorithm | Signature Size | Rank |
|-----------|----------------|------|
| Classic Rainbow | 64 bytes | 🥇 Smallest |
| Rainbow SL1 | 80 bytes | 🥈 Very Small |
| UOV SL1 | 172 bytes | Small |
| Rainbow SL3 | 192 bytes | Medium |
| UOV SL3 | 272 bytes | Large |
| Rainbow SL5 | 268 bytes | Large |
| UOV SL5 | 356 bytes | Larger |
| Mix | 624 bytes | ❌ Largest |

---

## Selection Guidelines

### Recommended Configurations

| Use Case | Recommended | Config | Reason |
|----------|-------------|--------|--------|
| **General Use** | Rainbow SL1 | 274 | Default, well-balanced |
| **High Security** | Rainbow SL5 | 286 | NIST Level 5 |
| **Maximum Security** | Mix | 513 | Dual algorithm protection |
| **Small Signature** | Rainbow SL1 | 274 | Only 80 bytes |
| **Small Private Key** | UOV SL1 or Rainbow SL1 | 262 or 274 | Only 70 bytes |
| **Legacy Compatibility** | Classic Rainbow | 0 | Backward compatible |

### Not Recommended

- ❌ **config_value = 0** (Classic Rainbow): Private and public keys too large (100KB+), unless compatibility needed
- ⚠️ **config_value = 513** (Mix): Public key exceeds 1MB, high transmission cost

### Practical Application Recommendations

```cpp
// 🎯 Recommended: Suitable for most scenarios
key.MakeNewKey(274);  // Rainbow SL1

// 🔒 High-value transactions or long-term storage
key.MakeNewKey(286);  // Rainbow SL5

// ⚡ Scenarios requiring fast verification
key.MakeNewKey(274);  // Rainbow SL1 (small signature, fast verification)
```

---

## Technical Details

### config_value Structure

```
config_value (32-bit integer)
├─ High 8 bits: type (algorithm type)
│   ├─ 0: Classic Rainbow
│   ├─ 1: RainbowPro series (UOV/Rainbow compressed)
│   └─ 2: Hybrid algorithm
└─ Low 8 bits: subtype (specific variant)
    └─ See SIGN_CONFIG_RBOW array

Example: 274 = 0x0112 = (1 << 8) | 18
         ↑    ↑  ↑
         Decimal  type=1 (RainbowPro)
                  subtype=18 (Rainbow SL1 compressed)
```

### PUBLIC_CONFIG Array

Enabled configurations are defined in the `PUBLIC_CONFIG` array:

```c
unsigned int PUBLIC_CONFIG[PUBLIC_CONFIG_NUM+1][2] = {
    {0, 0},    // Index 0: Placeholder
    {1, 6},    // Index 1: UOV SL1
    {1, 9},    // Index 2: UOV SL3
    {1, 12},   // Index 3: UOV SL5
    {1, 18},   // Index 4: Rainbow SL1 (Default)
    {1, 24},   // Index 5: Rainbow SL3
    {1, 30},   // Index 6: Rainbow SL5
    {2, 1},    // Index 7: Mix
};
```

`DEFAULT_CONFIG = 4` corresponds to array index 4, i.e., Rainbow SL1.

---

## Code Reference

### MakeNewKey Function Implementation

```cpp
void CKey::MakeNewKey(unsigned int config_value)
{
    int status;
    if (0 == config_value) {
        // Use Classic Rainbow
        privKey.resize(RAINBOW_PRIVATE_KEY_SIZE);
        pubKey.vchPubKey.resize(RAINBOW_PUBLIC_KEY_SIZE);
        status = crypto_sign_keypair(pubKey.vchPubKey.data(), privKey.data());
        if (status != 0) {
            throw key_error("CKey::MakeNewKey, make key pair failure.");
        }
    } else {
        // Use RainbowPlus series
        if (1 != rainbowplus_if_the_choised_configvalue(config_value)) {
            throw key_error("CKey::MakeNewKey, config_value not supported.");
        }
        // ... Generate key pair
    }
    fSet = true;
}
```

### Signature Verification

```cpp
bool CPubKey::Verify(unsigned int config_value, 
                     unsigned char* hash_buf, unsigned int hash_size,
                     unsigned char* sign_buf, unsigned int sign_size, 
                     bool bMsg)
{
    if (0 == config_value) {
        // Verify Classic Rainbow signature
        int status = rainbow_verify(hash_buf, sign_buf, vchPubKey.data());
        return (0 == status);
    } else if (bMsg || nBestHeight >= RAINBOWFORkHEIGHT) {
        // Verify RainbowPlus signature
        // ...
    }
    return false;
}
```

---

## Frequently Asked Questions

### Q1: Why is there such a large difference in private key sizes between algorithms?

**A:** Classic Rainbow (config_value=0) uses the original Rainbow implementation, requiring storage of large matrix data (100KB+). The RainbowPlus series uses seed generation technology, only needing to store a seed (70 bytes), from which the key can be regenerated.

### Q2: Why is config_value = 274 the default configuration?

**A:** Rainbow SL1 (config_value=274) provides the best balance:
- Small private key (70 bytes)
- Small signature (80 bytes)
- Moderate public key (48KB)
- Meets NIST Level 1 security requirements
- Good performance, suitable for blockchain applications

### Q3: When should I use the Mix algorithm?

**A:** The Mix algorithm (config_value=513) is suitable for:
- Extremely high-value asset protection
- Need to defend against future quantum attacks
- Applications insensitive to public key size
- Scenarios requiring dual algorithm protection

However, note: Public key exceeds 1MB, high network transmission cost.

### Q4: How do I choose a security level?

**A:** Security level selection guide:
- **Level 1**: Regular transactions, daily use
- **Level 3**: Important transactions, enterprise applications
- **Level 5**: High-value assets, long-term storage

### Q5: Can different algorithms interoperate?

**A:** No. Each config_value corresponds to an independent cryptographic system, with different key and signature formats that cannot be mixed. Signature verification must use the same config_value.

---

## Change Log

- **2025-11-30**: Initial English version, documenting all available signature algorithm configurations

---

## References

- Source files: `src/key.cpp`, `src/key.h`
- Configuration definitions: `src/librainbowpro/src/rainbow/core/rb_core_rainbow-sign.c`
- Rainbow algorithm: NIST PQC standardization project
- UOV algorithm: Unbalanced Oil and Vinegar multivariate signature scheme

---

## Summary

| **Quick Reference** | **Config Value** |
|--------------------|------------------|
| Default Recommended ⭐ | 274 (Rainbow SL1) |
| Smallest Signature | 274 (Rainbow SL1, 80 bytes) |
| Smallest Private Key | 262/274/265/268/280/286 (70 bytes) |
| Smallest Public Key | 262 (UOV SL1, 43.6 KB) |
| Highest Security 🔒 | 513 (Mix, NIST Level 5×2) |
| Legacy Compatibility | 0 (Classic Rainbow) |

**Recommendation**: Unless there are special requirements, use the default configuration `MakeNewKey(274)`.
