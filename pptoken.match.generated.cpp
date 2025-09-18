if(match[1].matched) { // NewLine
  token.kind = PPToken::kNewLine;
} else if(match[2].matched) { // Auto
  token.kind = PPToken::kAuto;
} else if(match[3].matched) { // Break
  token.kind = PPToken::kBreak;
} else if(match[4].matched) { // Case
  token.kind = PPToken::kCase;
} else if(match[5].matched) { // Char
  token.kind = PPToken::kChar;
} else if(match[6].matched) { // Const
  token.kind = PPToken::kConst;
} else if(match[7].matched) { // Continue
  token.kind = PPToken::kContinue;
} else if(match[8].matched) { // Default
  token.kind = PPToken::kDefault;
} else if(match[9].matched) { // Do
  token.kind = PPToken::kDo;
} else if(match[10].matched) { // Double
  token.kind = PPToken::kDouble;
} else if(match[11].matched) { // Else
  token.kind = PPToken::kElse;
} else if(match[12].matched) { // Enum
  token.kind = PPToken::kEnum;
} else if(match[13].matched) { // Extern
  token.kind = PPToken::kExtern;
} else if(match[14].matched) { // Float
  token.kind = PPToken::kFloat;
} else if(match[15].matched) { // For
  token.kind = PPToken::kFor;
} else if(match[16].matched) { // Goto
  token.kind = PPToken::kGoto;
} else if(match[17].matched) { // If
  token.kind = PPToken::kIf;
} else if(match[18].matched) { // Int
  token.kind = PPToken::kInt;
} else if(match[19].matched) { // Long
  token.kind = PPToken::kLong;
} else if(match[20].matched) { // Register
  token.kind = PPToken::kRegister;
} else if(match[21].matched) { // Return
  token.kind = PPToken::kReturn;
} else if(match[22].matched) { // Short
  token.kind = PPToken::kShort;
} else if(match[23].matched) { // Signed
  token.kind = PPToken::kSigned;
} else if(match[24].matched) { // Sizeof
  token.kind = PPToken::kSizeof;
} else if(match[25].matched) { // Static
  token.kind = PPToken::kStatic;
} else if(match[26].matched) { // Struct
  token.kind = PPToken::kStruct;
} else if(match[27].matched) { // Switch
  token.kind = PPToken::kSwitch;
} else if(match[28].matched) { // Typedef
  token.kind = PPToken::kTypedef;
} else if(match[29].matched) { // Union
  token.kind = PPToken::kUnion;
} else if(match[30].matched) { // Unsigned
  token.kind = PPToken::kUnsigned;
} else if(match[31].matched) { // Void
  token.kind = PPToken::kVoid;
} else if(match[32].matched) { // Volatile
  token.kind = PPToken::kVolatile;
} else if(match[33].matched) { // While
  token.kind = PPToken::kWhile;
} else if(match[34].matched) { // CharacterConstant
  const auto &it = match.str(0);
  { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
  token.kind = PPToken::kCharacterConstant;
} else if(match[35].matched) { // StringLiteral
  const auto &it = match.str(0);
  { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
  token.kind = PPToken::kStringLiteral;
} else if(match[36].matched) { // Identifier
  token.kind = PPToken::kIdentifier;
} else if(match[37].matched) { // singleLineComment
  token.kind = PPToken::kWhitespace;
} else if(match[38].matched) { // multiLineComment
  const auto &it = match.str(0);
  { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
  token.kind = PPToken::kWhitespace;
} else if(match[39].matched) { // whitespace
  token.kind = PPToken::kWhitespace;
} else if(match[40].matched) { // PPNumber
  token.kind = PPToken::kPPNumber;
} else if(match[41].matched) { // Operator
  token.kind = PPToken::kOperator;
} else if(match[42].matched) { // NonWhiteSpace
  token.kind = PPToken::kNonWhiteSpace;
}
