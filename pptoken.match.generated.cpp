if(match[1].matched) { // NewLine
  token.kind = PPToken::kNewLine;
} else if(match[2].matched) { // CharacterConstant
  const auto &it = match.str(0);
  { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
  token.kind = PPToken::kCharacterConstant;
} else if(match[3].matched) { // StringLiteral
  const auto &it = match.str(0);
  { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
  token.kind = PPToken::kStringLiteral;
} else if(match[4].matched) { // Identifier
  token.kind = PPToken::kIdentifier;
} else if(match[5].matched) { // Whitespace
  token.kind = PPToken::kWhitespace;
} else if(match[6].matched) { // PPNumber
  token.kind = PPToken::kPPNumber;
} else if(match[7].matched) { // Operator
  token.kind = PPToken::kOperator;
} else if(match[8].matched) { // NonWhiteSpace
  token.kind = PPToken::kNonWhiteSpace;
}
