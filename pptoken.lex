NewLine = \n
CharacterConstant = L?'.*?('|$) { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
StringLiteral = L?".*?("|$) { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
Identifier = [_a-zA-Z][_a-zA-Z0-9]*
Whitespace = (singleLineComment | multiLineComment | whitespace)+
PPNumber = \.?[0-9]([eE][+-]|\.|[0-9a-zA-Z_])*
Operator = -|\+|\*=
NonWhiteSpace = .

singleLineComment = //.*?(?=\n|$)
multiLineComment = /\*[\s\S]*?(\*/|$)
whitespace = [ \t\r\f\v]+