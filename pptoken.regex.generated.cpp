R"((\n)|)" // NewLine
R"((L?'.*?(?:'|$))|)" // CharacterConstant
R"((L?".*?(?:"|$))|)" // StringLiteral
R"(([_a-zA-Z][_a-zA-Z0-9]*)|)" // Identifier
R"(((?://.*?(?=\n|$)|/\*[\s\S]*?(?:\*/|$)|[ \t\r\f\v]+)+)|)" // Whitespace
R"((\.?[0-9](?:[eE][+-]|\.|[0-9a-zA-Z_])*)|)" // PPNumber
R"((-|\+|\*=)|)" // Operator
R"((.))" // NonWhiteSpace
