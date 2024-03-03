#ifndef COMPILED_INSTRUCTION_H
#define COMPILED_INSTRUCTION_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include "public_key.h"
#include "account_meta.h"

class Instruction
{
public:
    // PublicKey of the program that executes this instruction
    PublicKey programId;

    // Metadata describing accounts that should be passed to the program
    std::vector<AccountMeta> accounts;

    // Opaque data passed to the program for its own interpretation
    std::vector<uint8_t> data;

    static Instruction newWithBytes(PublicKey programId, std::vector<uint8_t> &data, std::vector<AccountMeta> accounts);
    std::vector<uint8_t> serialize();
    Instruction deserialize(const std::vector<uint8_t> &input);
};

class CompiledInstruction
{
public:
    // Index into the transaction keys array indicating the program account that executes this instruction.
    uint8_t programIdIndex;

    // Ordered indices into the transaction keys array indicating which accounts to pass to the program.
    std::vector<uint8_t> accounts;

    // The program input data.
    std::vector<uint8_t> data;

    template <typename T>
    CompiledInstruction(uint8_t programIdsIndex, const T &data, std::vector<uint8_t> accounts);
    CompiledInstruction(uint8_t programIdIndex, std::vector<uint8_t> data, std::vector<uint8_t> accounts);
    PublicKey programId(const std::vector<PublicKey> &program_ids) const;
    void sanitize();
    std::vector<uint8_t> serialize();
    static CompiledInstruction deserialize(uint8_t programIdIndex, const std::vector<uint8_t> &accounts, const std::vector<uint8_t> &data, const std::vector<uint8_t> &input);
};

uint8_t position(const std::vector<PublicKey> &keys, const PublicKey &key);
CompiledInstruction compileIx(const Instruction &ix, const std::vector<PublicKey> &keys);
std::vector<CompiledInstruction> compileInstructions(const std::vector<Instruction> &ixs, const std::vector<PublicKey> &keys);

#endif // COMPILED_INSTRUCTION_H
