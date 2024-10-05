#pragma once

#include <google/protobuf/compiler/code_generator.h>

#include <cstdint>
#include <string>

class Generator 
    : public google::protobuf::compiler::CodeGenerator 
{
public:
    ~Generator() 
    {}

    Generator() 
    {}

    bool Generate(
        const google::protobuf::FileDescriptor* file, 
        const std::string& parameter, 
        google::protobuf::compiler::GeneratorContext* context, 
        std::string* error
    ) const override;

    uint64_t GetSupportedFeatures() const override { return FEATURE_PROTO3_OPTIONAL; }
};
