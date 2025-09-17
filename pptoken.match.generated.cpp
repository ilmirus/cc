if(match[1].matched) { // NewLine
  token.kind = PPToken::kNewLine;
} else if(match[2].matched) { // KwAuto
  token.kind = PPToken::kKwAuto;
} else if(match[3].matched) { // KwBreak
  token.kind = PPToken::kKwBreak;
} else if(match[4].matched) { // KwCase
  token.kind = PPToken::kKwCase;
} else if(match[5].matched) { // KwChar
  token.kind = PPToken::kKwChar;
} else if(match[6].matched) { // KwConst
  token.kind = PPToken::kKwConst;
} else if(match[7].matched) { // KwContinue
  token.kind = PPToken::kKwContinue;
} else if(match[8].matched) { // KwDefault
  token.kind = PPToken::kKwDefault;
} else if(match[9].matched) { // KwDo
  token.kind = PPToken::kKwDo;
} else if(match[10].matched) { // KwDouble
  token.kind = PPToken::kKwDouble;
} else if(match[11].matched) { // KwElse
  token.kind = PPToken::kKwElse;
} else if(match[12].matched) { // KwEnum
  token.kind = PPToken::kKwEnum;
} else if(match[13].matched) { // KwExtern
  token.kind = PPToken::kKwExtern;
} else if(match[14].matched) { // KwFloat
  token.kind = PPToken::kKwFloat;
} else if(match[15].matched) { // KwFor
  token.kind = PPToken::kKwFor;
} else if(match[16].matched) { // KwGoto
  token.kind = PPToken::kKwGoto;
} else if(match[17].matched) { // KwIf
  token.kind = PPToken::kKwIf;
} else if(match[18].matched) { // KwInt
  token.kind = PPToken::kKwInt;
} else if(match[19].matched) { // KwLong
  token.kind = PPToken::kKwLong;
} else if(match[20].matched) { // KwRegister
  token.kind = PPToken::kKwRegister;
} else if(match[21].matched) { // KwReturn
  token.kind = PPToken::kKwReturn;
} else if(match[22].matched) { // KwShort
  token.kind = PPToken::kKwShort;
} else if(match[23].matched) { // KwSigned
  token.kind = PPToken::kKwSigned;
} else if(match[24].matched) { // KwSizeof
  token.kind = PPToken::kKwSizeof;
} else if(match[25].matched) { // KwStatic
  token.kind = PPToken::kKwStatic;
} else if(match[26].matched) { // KwStruct
  token.kind = PPToken::kKwStruct;
} else if(match[27].matched) { // KwSwitch
  token.kind = PPToken::kKwSwitch;
} else if(match[28].matched) { // KwTypedef
  token.kind = PPToken::kKwTypedef;
} else if(match[29].matched) { // KwUnion
  token.kind = PPToken::kKwUnion;
} else if(match[30].matched) { // KwUnsigned
  token.kind = PPToken::kKwUnsigned;
} else if(match[31].matched) { // KwVoid
  token.kind = PPToken::kKwVoid;
} else if(match[32].matched) { // KwVolatile
  token.kind = PPToken::kKwVolatile;
} else if(match[33].matched) { // KwWhile
  token.kind = PPToken::kKwWhile;
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
