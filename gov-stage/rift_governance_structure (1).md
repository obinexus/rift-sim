# RIFT Governance Directory Structure
## OBINexus Framework - Stage-Bound Configuration System

```
project-root/
├── rift-gov/                          # Governance configuration root
│   ├── .riftrc.0                      # RIFT-0 Tokenizer stage configuration
│   ├── .riftrc.1                      # RIFT-1 Parser stage configuration  
│   ├── .riftrc.2                      # RIFT-2 AST Coordinator stage configuration
│   ├── .riftrc.3                      # RIFT-3 Output stage configuration
│   ├── global.rift                    # Global governance policies
│   ├── intention-maps/                # Intention mapping definitions
│   │   ├── tokenizer-intentions.rift  # RIFT-0 specific intentions
│   │   ├── parser-intentions.rift     # RIFT-1 specific intentions
│   │   ├── coordinator-intentions.rift # RIFT-2 specific intentions
│   │   └── output-intentions.rift     # RIFT-3 specific intentions
│   └── schemas/                       # Configuration validation schemas
│       ├── stage-0.schema             # RIFT-0 validation rules
│       ├── stage-1.schema             # RIFT-1 validation rules
│       ├── stage-2.schema             # RIFT-2 validation rules
│       └── stage-3.schema             # RIFT-3 validation rules
├── src/
│   └── rift_sim_staged.c              # Enhanced stage-bound simulation
└── poc/
    └── rift_sim_standalone.c          # Original monolithic version
```

## Configuration File Purposes

### .riftrc.0 - Tokenizer Configuration
- **Purpose**: Defines lexical patterns, token types, and DFA state definitions
- **Key Elements**: Regular expressions, priority rules, tokenization strategies
- **SP Alignment**: Controls how raw text transforms into classified token streams

### .riftrc.1 - Parser Configuration  
- **Purpose**: Specifies grammar rules, precedence tables, and AST construction policies
- **Key Elements**: BNF grammar fragments, operator precedence, error recovery strategies
- **SP Alignment**: Governs token-to-AST transformation according to language semantics

### .riftrc.2 - AST Coordinator Configuration
- **Purpose**: Defines optimization passes, semantic analysis rules, and tree transformation policies
- **Key Elements**: Optimization strategies, symbol table management, type checking rules
- **SP Alignment**: Controls semantic analysis and intermediate representation optimization

### .riftrc.3 - Output Configuration
- **Purpose**: Specifies output formats, code generation templates, and target language mappings
- **Key Elements**: Template definitions, formatting rules, target-specific generation strategies
- **SP Alignment**: Manages final transformation from internal representation to target output

## Integration with OBINexus Toolchain
The stage-bound configuration integrates seamlessly with the OBINexus toolchain progression:
- **riftlang.exe**: Reads and validates stage configurations
- **.so.a**: Compiles stage-specific governance into linkable modules
- **rift.exe**: Executes stage-bound pipeline with governance enforcement
- **gosilang**: Provides runtime support for governance-driven execution
