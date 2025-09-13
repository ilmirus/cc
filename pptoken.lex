NewLine = \n
CharacterConstant = L?'.*?('|$) { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
StringLiteral = L?".*?("|$) { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
Identifier = [_a-zA-Z][_a-zA-Z0-9]*

Whitespace = singleLineComment | multiLineComment | whitespace
singleLineComment = //[^\n]*
multiLineComment = /\*[\s\S]*?(\*/|$) { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
whitespace = [ \t\r\f\v]+

PPNumber = \.?[0-9]([eE][+-]|\.|[0-9a-zA-Z_])*
Operator = -|\+|\*=
NonWhiteSpace = .
