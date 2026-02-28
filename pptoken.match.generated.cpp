if(std::regex_search(iter, end, match, patterns[0])) { // NewLine
  kind = PPToken::kNewLine;
} else if(std::regex_search(iter, end, match, patterns[1])) { // singleLineComment
  kind = PPToken::kWhitespace;
} else if(std::regex_search(iter, end, match, patterns[2])) { // multiLineComment
  const auto &it = match.str(0);
  { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
  kind = PPToken::kWhitespace;
} else if(std::regex_search(iter, end, match, patterns[3])) { // whitespace
  kind = PPToken::kWhitespace;
} else if(std::regex_search(iter, end, match, patterns[4])) { // PPNumber
  kind = PPToken::kPPNumber;
} else if(std::regex_search(iter, end, match, patterns[5])) { // Auto
  kind = PPToken::kAuto;
} else if(std::regex_search(iter, end, match, patterns[6])) { // Break
  kind = PPToken::kBreak;
} else if(std::regex_search(iter, end, match, patterns[7])) { // Case
  kind = PPToken::kCase;
} else if(std::regex_search(iter, end, match, patterns[8])) { // Char
  kind = PPToken::kChar;
} else if(std::regex_search(iter, end, match, patterns[9])) { // Const
  kind = PPToken::kConst;
} else if(std::regex_search(iter, end, match, patterns[10])) { // Continue
  kind = PPToken::kContinue;
} else if(std::regex_search(iter, end, match, patterns[11])) { // Default
  kind = PPToken::kDefault;
} else if(std::regex_search(iter, end, match, patterns[12])) { // Do
  kind = PPToken::kDo;
} else if(std::regex_search(iter, end, match, patterns[13])) { // Double
  kind = PPToken::kDouble;
} else if(std::regex_search(iter, end, match, patterns[14])) { // Else
  kind = PPToken::kElse;
} else if(std::regex_search(iter, end, match, patterns[15])) { // Enum
  kind = PPToken::kEnum;
} else if(std::regex_search(iter, end, match, patterns[16])) { // Extern
  kind = PPToken::kExtern;
} else if(std::regex_search(iter, end, match, patterns[17])) { // Float
  kind = PPToken::kFloat;
} else if(std::regex_search(iter, end, match, patterns[18])) { // For
  kind = PPToken::kFor;
} else if(std::regex_search(iter, end, match, patterns[19])) { // Goto
  kind = PPToken::kGoto;
} else if(std::regex_search(iter, end, match, patterns[20])) { // If
  kind = PPToken::kIf;
} else if(std::regex_search(iter, end, match, patterns[21])) { // Int
  kind = PPToken::kInt;
} else if(std::regex_search(iter, end, match, patterns[22])) { // Long
  kind = PPToken::kLong;
} else if(std::regex_search(iter, end, match, patterns[23])) { // Register
  kind = PPToken::kRegister;
} else if(std::regex_search(iter, end, match, patterns[24])) { // Return
  kind = PPToken::kReturn;
} else if(std::regex_search(iter, end, match, patterns[25])) { // Short
  kind = PPToken::kShort;
} else if(std::regex_search(iter, end, match, patterns[26])) { // Signed
  kind = PPToken::kSigned;
} else if(std::regex_search(iter, end, match, patterns[27])) { // Sizeof
  kind = PPToken::kSizeof;
} else if(std::regex_search(iter, end, match, patterns[28])) { // Static
  kind = PPToken::kStatic;
} else if(std::regex_search(iter, end, match, patterns[29])) { // Struct
  kind = PPToken::kStruct;
} else if(std::regex_search(iter, end, match, patterns[30])) { // Switch
  kind = PPToken::kSwitch;
} else if(std::regex_search(iter, end, match, patterns[31])) { // Typedef
  kind = PPToken::kTypedef;
} else if(std::regex_search(iter, end, match, patterns[32])) { // Union
  kind = PPToken::kUnion;
} else if(std::regex_search(iter, end, match, patterns[33])) { // Unsigned
  kind = PPToken::kUnsigned;
} else if(std::regex_search(iter, end, match, patterns[34])) { // Void
  kind = PPToken::kVoid;
} else if(std::regex_search(iter, end, match, patterns[35])) { // Volatile
  kind = PPToken::kVolatile;
} else if(std::regex_search(iter, end, match, patterns[36])) { // While
  kind = PPToken::kWhile;
} else if(std::regex_search(iter, end, match, patterns[37])) { // DotDotDot
  kind = PPToken::kDotDotDot;
} else if(std::regex_search(iter, end, match, patterns[38])) { // Semicolon
  kind = PPToken::kSemicolon;
} else if(std::regex_search(iter, end, match, patterns[39])) { // OpenSquare
  kind = PPToken::kOpenSquare;
} else if(std::regex_search(iter, end, match, patterns[40])) { // CloseSquare
  kind = PPToken::kCloseSquare;
} else if(std::regex_search(iter, end, match, patterns[41])) { // OpenParen
  kind = PPToken::kOpenParen;
} else if(std::regex_search(iter, end, match, patterns[42])) { // CloseParen
  kind = PPToken::kCloseParen;
} else if(std::regex_search(iter, end, match, patterns[43])) { // Dot
  kind = PPToken::kDot;
} else if(std::regex_search(iter, end, match, patterns[44])) { // Arrow
  kind = PPToken::kArrow;
} else if(std::regex_search(iter, end, match, patterns[45])) { // Inv
  kind = PPToken::kInv;
} else if(std::regex_search(iter, end, match, patterns[46])) { // Inc
  kind = PPToken::kInc;
} else if(std::regex_search(iter, end, match, patterns[47])) { // Dec
  kind = PPToken::kDec;
} else if(std::regex_search(iter, end, match, patterns[48])) { // AndAnd
  kind = PPToken::kAndAnd;
} else if(std::regex_search(iter, end, match, patterns[49])) { // AndAssign
  kind = PPToken::kAndAssign;
} else if(std::regex_search(iter, end, match, patterns[50])) { // And
  kind = PPToken::kAnd;
} else if(std::regex_search(iter, end, match, patterns[51])) { // StarAssign
  kind = PPToken::kStarAssign;
} else if(std::regex_search(iter, end, match, patterns[52])) { // Star
  kind = PPToken::kStar;
} else if(std::regex_search(iter, end, match, patterns[53])) { // Minus
  kind = PPToken::kMinus;
} else if(std::regex_search(iter, end, match, patterns[54])) { // NotAssign
  kind = PPToken::kNotAssign;
} else if(std::regex_search(iter, end, match, patterns[55])) { // Not
  kind = PPToken::kNot;
} else if(std::regex_search(iter, end, match, patterns[56])) { // DivAssign
  kind = PPToken::kDivAssign;
} else if(std::regex_search(iter, end, match, patterns[57])) { // Div
  kind = PPToken::kDiv;
} else if(std::regex_search(iter, end, match, patterns[58])) { // ModAssign
  kind = PPToken::kModAssign;
} else if(std::regex_search(iter, end, match, patterns[59])) { // Mod
  kind = PPToken::kMod;
} else if(std::regex_search(iter, end, match, patterns[60])) { // PlusAssign
  kind = PPToken::kPlusAssign;
} else if(std::regex_search(iter, end, match, patterns[61])) { // Plus
  kind = PPToken::kPlus;
} else if(std::regex_search(iter, end, match, patterns[62])) { // ShiftLeftAssign
  kind = PPToken::kShiftLeftAssign;
} else if(std::regex_search(iter, end, match, patterns[63])) { // ShiftLeft
  kind = PPToken::kShiftLeft;
} else if(std::regex_search(iter, end, match, patterns[64])) { // ShiftRightAssign
  kind = PPToken::kShiftRightAssign;
} else if(std::regex_search(iter, end, match, patterns[65])) { // ShiftRight
  kind = PPToken::kShiftRight;
} else if(std::regex_search(iter, end, match, patterns[66])) { // LessEq
  kind = PPToken::kLessEq;
} else if(std::regex_search(iter, end, match, patterns[67])) { // Less
  kind = PPToken::kLess;
} else if(std::regex_search(iter, end, match, patterns[68])) { // GrEq
  kind = PPToken::kGrEq;
} else if(std::regex_search(iter, end, match, patterns[69])) { // Gr
  kind = PPToken::kGr;
} else if(std::regex_search(iter, end, match, patterns[70])) { // EqEq
  kind = PPToken::kEqEq;
} else if(std::regex_search(iter, end, match, patterns[71])) { // XorAssign
  kind = PPToken::kXorAssign;
} else if(std::regex_search(iter, end, match, patterns[72])) { // Xor
  kind = PPToken::kXor;
} else if(std::regex_search(iter, end, match, patterns[73])) { // OrOr
  kind = PPToken::kOrOr;
} else if(std::regex_search(iter, end, match, patterns[74])) { // OrAssign
  kind = PPToken::kOrAssign;
} else if(std::regex_search(iter, end, match, patterns[75])) { // Or
  kind = PPToken::kOr;
} else if(std::regex_search(iter, end, match, patterns[76])) { // Query
  kind = PPToken::kQuery;
} else if(std::regex_search(iter, end, match, patterns[77])) { // Colon
  kind = PPToken::kColon;
} else if(std::regex_search(iter, end, match, patterns[78])) { // Assign
  kind = PPToken::kAssign;
} else if(std::regex_search(iter, end, match, patterns[79])) { // Comma
  kind = PPToken::kComma;
} else if(std::regex_search(iter, end, match, patterns[80])) { // HashHash
  kind = PPToken::kHashHash;
} else if(std::regex_search(iter, end, match, patterns[81])) { // Hash
  kind = PPToken::kHash;
} else if(std::regex_search(iter, end, match, patterns[82])) { // CharacterConstant
  const auto &it = match.str(0);
  { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
  kind = PPToken::kCharacterConstant;
} else if(std::regex_search(iter, end, match, patterns[83])) { // StringLiteral
  const auto &it = match.str(0);
  { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
  kind = PPToken::kStringLiteral;
} else if(std::regex_search(iter, end, match, patterns[84])) { // Identifier
  kind = PPToken::kIdentifier;
} else if(std::regex_search(iter, end, match, patterns[85])) { // NonWhiteSpace
  kind = PPToken::kNonWhiteSpace;
}
