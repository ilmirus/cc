NewLine = \n

KwAuto = auto(?![_0-9a-zA-Z])
KwBreak = break(?![_0-9a-zA-Z])
KwCase = case(?![_0-9a-zA-Z])
KwChar = char(?![_0-9a-zA-Z])
KwConst = const(?![_0-9a-zA-Z])
KwContinue = continue(?![_0-9a-zA-Z])
KwDefault = default(?![_0-9a-zA-Z])
KwDo = do(?![_0-9a-zA-Z])
KwDouble = double(?![_0-9a-zA-Z])
KwElse = else(?![_0-9a-zA-Z])
KwEnum = enum(?![_0-9a-zA-Z])
KwExtern = extern(?![_0-9a-zA-Z])
KwFloat = float(?![_0-9a-zA-Z])
KwFor = for(?![_0-9a-zA-Z])
KwGoto = goto(?![_0-9a-zA-Z])
KwIf = if(?![_0-9a-zA-Z])
KwInt = int(?![_0-9a-zA-Z])
KwLong = long(?![_0-9a-zA-Z])
KwRegister = register(?![_0-9a-zA-Z])
KwReturn = return(?![_0-9a-zA-Z])
KwShort = short(?![_0-9a-zA-Z])
KwSigned = signed(?![_0-9a-zA-Z])
KwSizeof = sizeof(?![_0-9a-zA-Z])
KwStatic = static(?![_0-9a-zA-Z])
KwStruct = struct(?![_0-9a-zA-Z])
KwSwitch = switch(?![_0-9a-zA-Z])
KwTypedef = typedef(?![_0-9a-zA-Z])
KwUnion = union(?![_0-9a-zA-Z])
KwUnsigned = unsigned(?![_0-9a-zA-Z])
KwVoid = void(?![_0-9a-zA-Z])
KwVolatile = volatile(?![_0-9a-zA-Z])
KwWhile = while(?![_0-9a-zA-Z])

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
