 bool reset_header_state = true; 
if(std::regex_search(iter, end, match, patterns[0])) {
  const auto &it = match.str(0);
  { header_state = kNewLine; reset_header_state = false; }
  kind = PPToken::kNewLine;
} else if(std::regex_search(iter, end, match, patterns[1])) {
  const auto &it = match.str(0);
  { reset_header_state = false; }
  kind = PPToken::kWhitespace;
} else if(std::regex_search(iter, end, match, patterns[2])) {
  const auto &it = match.str(0);
  { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
  { reset_header_state = false; }
  kind = PPToken::kWhitespace;
} else if(std::regex_search(iter, end, match, patterns[3])) {
  const auto &it = match.str(0);
  { reset_header_state = false; }
  kind = PPToken::kWhitespace;
} else if(std::regex_search(iter, end, match, patterns[4])) {
  kind = PPToken::kPPNumber;
} else if(std::regex_search(iter, end, match, patterns[5])) {
  kind = PPToken::kAuto;
} else if(std::regex_search(iter, end, match, patterns[6])) {
  kind = PPToken::kBreak;
} else if(std::regex_search(iter, end, match, patterns[7])) {
  kind = PPToken::kCase;
} else if(std::regex_search(iter, end, match, patterns[8])) {
  kind = PPToken::kChar;
} else if(std::regex_search(iter, end, match, patterns[9])) {
  kind = PPToken::kConst;
} else if(std::regex_search(iter, end, match, patterns[10])) {
  kind = PPToken::kContinue;
} else if(std::regex_search(iter, end, match, patterns[11])) {
  kind = PPToken::kDefault;
} else if(std::regex_search(iter, end, match, patterns[12])) {
  kind = PPToken::kDo;
} else if(std::regex_search(iter, end, match, patterns[13])) {
  kind = PPToken::kDouble;
} else if(std::regex_search(iter, end, match, patterns[14])) {
  kind = PPToken::kElse;
} else if(std::regex_search(iter, end, match, patterns[15])) {
  kind = PPToken::kEnum;
} else if(std::regex_search(iter, end, match, patterns[16])) {
  kind = PPToken::kExtern;
} else if(std::regex_search(iter, end, match, patterns[17])) {
  kind = PPToken::kFloat;
} else if(std::regex_search(iter, end, match, patterns[18])) {
  kind = PPToken::kFor;
} else if(std::regex_search(iter, end, match, patterns[19])) {
  kind = PPToken::kGoto;
} else if(std::regex_search(iter, end, match, patterns[20])) {
  kind = PPToken::kIf;
} else if(std::regex_search(iter, end, match, patterns[21])) {
  kind = PPToken::kInt;
} else if(std::regex_search(iter, end, match, patterns[22])) {
  kind = PPToken::kLong;
} else if(std::regex_search(iter, end, match, patterns[23])) {
  kind = PPToken::kRegister;
} else if(std::regex_search(iter, end, match, patterns[24])) {
  kind = PPToken::kReturn;
} else if(std::regex_search(iter, end, match, patterns[25])) {
  kind = PPToken::kShort;
} else if(std::regex_search(iter, end, match, patterns[26])) {
  kind = PPToken::kSigned;
} else if(std::regex_search(iter, end, match, patterns[27])) {
  kind = PPToken::kSizeof;
} else if(std::regex_search(iter, end, match, patterns[28])) {
  kind = PPToken::kStatic;
} else if(std::regex_search(iter, end, match, patterns[29])) {
  kind = PPToken::kStruct;
} else if(std::regex_search(iter, end, match, patterns[30])) {
  kind = PPToken::kSwitch;
} else if(std::regex_search(iter, end, match, patterns[31])) {
  kind = PPToken::kTypedef;
} else if(std::regex_search(iter, end, match, patterns[32])) {
  kind = PPToken::kUnion;
} else if(std::regex_search(iter, end, match, patterns[33])) {
  kind = PPToken::kUnsigned;
} else if(std::regex_search(iter, end, match, patterns[34])) {
  kind = PPToken::kVoid;
} else if(std::regex_search(iter, end, match, patterns[35])) {
  kind = PPToken::kVolatile;
} else if(std::regex_search(iter, end, match, patterns[36])) {
  kind = PPToken::kWhile;
} else if((header_state == kInclude) && std::regex_search(iter, end, match, patterns[37])) {
  kind = PPToken::kHeaderName;
} else if(std::regex_search(iter, end, match, patterns[38])) {
  kind = PPToken::kDotDotDot;
} else if(std::regex_search(iter, end, match, patterns[39])) {
  kind = PPToken::kSemicolon;
} else if(std::regex_search(iter, end, match, patterns[40])) {
  kind = PPToken::kOpenSquare;
} else if(std::regex_search(iter, end, match, patterns[41])) {
  kind = PPToken::kCloseSquare;
} else if(std::regex_search(iter, end, match, patterns[42])) {
  kind = PPToken::kOpenParen;
} else if(std::regex_search(iter, end, match, patterns[43])) {
  kind = PPToken::kCloseParen;
} else if(std::regex_search(iter, end, match, patterns[44])) {
  kind = PPToken::kDot;
} else if(std::regex_search(iter, end, match, patterns[45])) {
  kind = PPToken::kArrow;
} else if(std::regex_search(iter, end, match, patterns[46])) {
  kind = PPToken::kInv;
} else if(std::regex_search(iter, end, match, patterns[47])) {
  kind = PPToken::kInc;
} else if(std::regex_search(iter, end, match, patterns[48])) {
  kind = PPToken::kDec;
} else if(std::regex_search(iter, end, match, patterns[49])) {
  kind = PPToken::kAndAnd;
} else if(std::regex_search(iter, end, match, patterns[50])) {
  kind = PPToken::kAndAssign;
} else if(std::regex_search(iter, end, match, patterns[51])) {
  kind = PPToken::kAnd;
} else if(std::regex_search(iter, end, match, patterns[52])) {
  kind = PPToken::kStarAssign;
} else if(std::regex_search(iter, end, match, patterns[53])) {
  kind = PPToken::kStar;
} else if(std::regex_search(iter, end, match, patterns[54])) {
  kind = PPToken::kMinus;
} else if(std::regex_search(iter, end, match, patterns[55])) {
  kind = PPToken::kNotAssign;
} else if(std::regex_search(iter, end, match, patterns[56])) {
  kind = PPToken::kNot;
} else if(std::regex_search(iter, end, match, patterns[57])) {
  kind = PPToken::kDivAssign;
} else if(std::regex_search(iter, end, match, patterns[58])) {
  kind = PPToken::kDiv;
} else if(std::regex_search(iter, end, match, patterns[59])) {
  kind = PPToken::kModAssign;
} else if(std::regex_search(iter, end, match, patterns[60])) {
  kind = PPToken::kMod;
} else if(std::regex_search(iter, end, match, patterns[61])) {
  kind = PPToken::kPlusAssign;
} else if(std::regex_search(iter, end, match, patterns[62])) {
  kind = PPToken::kPlus;
} else if(std::regex_search(iter, end, match, patterns[63])) {
  kind = PPToken::kShiftLeftAssign;
} else if(std::regex_search(iter, end, match, patterns[64])) {
  kind = PPToken::kShiftLeft;
} else if(std::regex_search(iter, end, match, patterns[65])) {
  kind = PPToken::kShiftRightAssign;
} else if(std::regex_search(iter, end, match, patterns[66])) {
  kind = PPToken::kShiftRight;
} else if(std::regex_search(iter, end, match, patterns[67])) {
  kind = PPToken::kLessEq;
} else if(std::regex_search(iter, end, match, patterns[68])) {
  kind = PPToken::kLess;
} else if(std::regex_search(iter, end, match, patterns[69])) {
  kind = PPToken::kGrEq;
} else if(std::regex_search(iter, end, match, patterns[70])) {
  kind = PPToken::kGr;
} else if(std::regex_search(iter, end, match, patterns[71])) {
  kind = PPToken::kEqEq;
} else if(std::regex_search(iter, end, match, patterns[72])) {
  kind = PPToken::kXorAssign;
} else if(std::regex_search(iter, end, match, patterns[73])) {
  kind = PPToken::kXor;
} else if(std::regex_search(iter, end, match, patterns[74])) {
  kind = PPToken::kOrOr;
} else if(std::regex_search(iter, end, match, patterns[75])) {
  kind = PPToken::kOrAssign;
} else if(std::regex_search(iter, end, match, patterns[76])) {
  kind = PPToken::kOr;
} else if(std::regex_search(iter, end, match, patterns[77])) {
  kind = PPToken::kQuery;
} else if(std::regex_search(iter, end, match, patterns[78])) {
  kind = PPToken::kColon;
} else if(std::regex_search(iter, end, match, patterns[79])) {
  kind = PPToken::kAssign;
} else if(std::regex_search(iter, end, match, patterns[80])) {
  kind = PPToken::kComma;
} else if(std::regex_search(iter, end, match, patterns[81])) {
  kind = PPToken::kHashHash;
} else if(std::regex_search(iter, end, match, patterns[82])) {
  const auto &it = match.str(0);
  {
  if (header_state == kNewLine) {
  header_state = kHash;
  reset_header_state = false;
  }
  }
  kind = PPToken::kHash;
} else if(std::regex_search(iter, end, match, patterns[83])) {
  const auto &it = match.str(0);
  { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
  kind = PPToken::kCharacterConstant;
} else if(std::regex_search(iter, end, match, patterns[84])) {
  const auto &it = match.str(0);
  { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
  kind = PPToken::kStringLiteral;
} else if(std::regex_search(iter, end, match, patterns[85])) {
  const auto &it = match.str(0);
  {
  if (header_state == kHash && it == "include") {
  header_state = kInclude;
  reset_header_state = false;
  }
  }
  kind = PPToken::kIdentifier;
} else if(std::regex_search(iter, end, match, patterns[86])) {
  kind = PPToken::kNonWhiteSpace;
}
 if (reset_header_state) header_state = kNone; 
