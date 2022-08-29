#ifndef ENUMS_H
#define ENUMS_H

enum SectionLabel
{
    OTHER = 0,
    TEXT,
    DATA,
    BSS,
    START,
    UND
};

enum SymbolType {
    NOTYP,
    SCTN
};

enum TokenType
{
    LABEL,
    SECTION,
    EXT,
    GLB,
    SKIP,
    INSTRUCTION,
    INCORECT,
    DIRECTIVE,
    SYMBOL,
    OP_DEC,
    OP_SYM_VALUE,
    EXP,
    END
};

enum Scope
{
    LOCAL,
    GLOBAL
};

enum AddressingType
{
    IMM,
    REG_DIR,
    REG_IND,
    REG_IND_POM8,
    REG_IND_POM16,
    MEM,
    PC_POM,
    
};

enum SymLit{
    SYM,
    LIT,
    LITHEX,
    REG
};

enum RelType{
    R_386_16,
    R_386_16I,
    R_386_PC16
};

#endif
