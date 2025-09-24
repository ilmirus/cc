if(match[1].matched) { // NewLine
  token.kind = PPToken::kNewLine;
} else if(match[2].matched) { // singleLineComment
  token.kind = PPToken::kWhitespace;
} else if(match[3].matched) { // multiLineComment
  const auto &it = match.str(0);
  { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
  token.kind = PPToken::kWhitespace;
} else if(match[4].matched) { // whitespace
  token.kind = PPToken::kWhitespace;
} else if(match[5].matched) { // Auto
  token.kind = PPToken::kAuto;
} else if(match[6].matched) { // Break
  token.kind = PPToken::kBreak;
} else if(match[7].matched) { // Case
  token.kind = PPToken::kCase;
} else if(match[8].matched) { // Char
  token.kind = PPToken::kChar;
} else if(match[9].matched) { // Const
  token.kind = PPToken::kConst;
} else if(match[10].matched) { // Continue
  token.kind = PPToken::kContinue;
} else if(match[11].matched) { // Default
  token.kind = PPToken::kDefault;
} else if(match[12].matched) { // Do
  token.kind = PPToken::kDo;
} else if(match[13].matched) { // Double
  token.kind = PPToken::kDouble;
} else if(match[14].matched) { // Else
  token.kind = PPToken::kElse;
} else if(match[15].matched) { // Enum
  token.kind = PPToken::kEnum;
} else if(match[16].matched) { // Extern
  token.kind = PPToken::kExtern;
} else if(match[17].matched) { // Float
  token.kind = PPToken::kFloat;
} else if(match[18].matched) { // For
  token.kind = PPToken::kFor;
} else if(match[19].matched) { // Goto
  token.kind = PPToken::kGoto;
} else if(match[20].matched) { // If
  token.kind = PPToken::kIf;
} else if(match[21].matched) { // Int
  token.kind = PPToken::kInt;
} else if(match[22].matched) { // Long
  token.kind = PPToken::kLong;
} else if(match[23].matched) { // Register
  token.kind = PPToken::kRegister;
} else if(match[24].matched) { // Return
  token.kind = PPToken::kReturn;
} else if(match[25].matched) { // Short
  token.kind = PPToken::kShort;
} else if(match[26].matched) { // Signed
  token.kind = PPToken::kSigned;
} else if(match[27].matched) { // Sizeof
  token.kind = PPToken::kSizeof;
} else if(match[28].matched) { // Static
  token.kind = PPToken::kStatic;
} else if(match[29].matched) { // Struct
  token.kind = PPToken::kStruct;
} else if(match[30].matched) { // Switch
  token.kind = PPToken::kSwitch;
} else if(match[31].matched) { // Typedef
  token.kind = PPToken::kTypedef;
} else if(match[32].matched) { // Union
  token.kind = PPToken::kUnion;
} else if(match[33].matched) { // Unsigned
  token.kind = PPToken::kUnsigned;
} else if(match[34].matched) { // Void
  token.kind = PPToken::kVoid;
} else if(match[35].matched) { // Volatile
  token.kind = PPToken::kVolatile;
} else if(match[36].matched) { // While
  token.kind = PPToken::kWhile;
} else if(match[37].matched) { // DotDotDot
  token.kind = PPToken::kDotDotDot;
} else if(match[38].matched) { // Semicolon
  token.kind = PPToken::kSemicolon;
} else if(match[39].matched) { // OpenSquare
  token.kind = PPToken::kOpenSquare;
} else if(match[40].matched) { // CloseSquare
  token.kind = PPToken::kCloseSquare;
} else if(match[41].matched) { // OpenParen
  token.kind = PPToken::kOpenParen;
} else if(match[42].matched) { // CloseParen
  token.kind = PPToken::kCloseParen;
} else if(match[43].matched) { // Dot
  token.kind = PPToken::kDot;
} else if(match[44].matched) { // Arrow
  token.kind = PPToken::kArrow;
} else if(match[45].matched) { // Inv
  token.kind = PPToken::kInv;
} else if(match[46].matched) { // Inc
  token.kind = PPToken::kInc;
} else if(match[47].matched) { // Dec
  token.kind = PPToken::kDec;
} else if(match[48].matched) { // AndAnd
  token.kind = PPToken::kAndAnd;
} else if(match[49].matched) { // AndAssign
  token.kind = PPToken::kAndAssign;
} else if(match[50].matched) { // And
  token.kind = PPToken::kAnd;
} else if(match[51].matched) { // StarAssign
  token.kind = PPToken::kStarAssign;
} else if(match[52].matched) { // Star
  token.kind = PPToken::kStar;
} else if(match[53].matched) { // Minus
  token.kind = PPToken::kMinus;
} else if(match[54].matched) { // NotAssign
  token.kind = PPToken::kNotAssign;
} else if(match[55].matched) { // Not
  token.kind = PPToken::kNot;
} else if(match[56].matched) { // DivAssign
  token.kind = PPToken::kDivAssign;
} else if(match[57].matched) { // Div
  token.kind = PPToken::kDiv;
} else if(match[58].matched) { // ModAssign
  token.kind = PPToken::kModAssign;
} else if(match[59].matched) { // Mod
  token.kind = PPToken::kMod;
} else if(match[60].matched) { // PlusAssign
  token.kind = PPToken::kPlusAssign;
} else if(match[61].matched) { // Plus
  token.kind = PPToken::kPlus;
} else if(match[62].matched) { // ShiftLeftAssign
  token.kind = PPToken::kShiftLeftAssign;
} else if(match[63].matched) { // ShiftLeft
  token.kind = PPToken::kShiftLeft;
} else if(match[64].matched) { // ShiftRightAssign
  token.kind = PPToken::kShiftRightAssign;
} else if(match[65].matched) { // ShiftRight
  token.kind = PPToken::kShiftRight;
} else if(match[66].matched) { // LessEq
  token.kind = PPToken::kLessEq;
} else if(match[67].matched) { // Less
  token.kind = PPToken::kLess;
} else if(match[68].matched) { // GrEq
  token.kind = PPToken::kGrEq;
} else if(match[69].matched) { // Gr
  token.kind = PPToken::kGr;
} else if(match[70].matched) { // EqEq
  token.kind = PPToken::kEqEq;
} else if(match[71].matched) { // XorAssign
  token.kind = PPToken::kXorAssign;
} else if(match[72].matched) { // Xor
  token.kind = PPToken::kXor;
} else if(match[73].matched) { // OrOr
  token.kind = PPToken::kOrOr;
} else if(match[74].matched) { // OrAssign
  token.kind = PPToken::kOrAssign;
} else if(match[75].matched) { // Or
  token.kind = PPToken::kOr;
} else if(match[76].matched) { // Query
  token.kind = PPToken::kQuery;
} else if(match[77].matched) { // Colon
  token.kind = PPToken::kColon;
} else if(match[78].matched) { // Assign
  token.kind = PPToken::kAssign;
} else if(match[79].matched) { // Comma
  token.kind = PPToken::kComma;
} else if(match[80].matched) { // HashHash
  token.kind = PPToken::kHashHash;
} else if(match[81].matched) { // Hash
  token.kind = PPToken::kHash;
} else if(match[82].matched) { // CharacterConstant
  const auto &it = match.str(0);
  { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
  token.kind = PPToken::kCharacterConstant;
} else if(match[83].matched) { // StringLiteral
  const auto &it = match.str(0);
  { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
  token.kind = PPToken::kStringLiteral;
} else if(match[84].matched) { // Identifier
  token.kind = PPToken::kIdentifier;
} else if(match[85].matched) { // PPNumber
  token.kind = PPToken::kPPNumber;
} else if(match[86].matched) { // NonWhiteSpace
  token.kind = PPToken::kNonWhiteSpace;
}
