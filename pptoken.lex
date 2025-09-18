NewLine = \n

Auto = auto(?![_0-9a-zA-Z])
Break = break(?![_0-9a-zA-Z])
Case = case(?![_0-9a-zA-Z])
Char = char(?![_0-9a-zA-Z])
Const = const(?![_0-9a-zA-Z])
Continue = continue(?![_0-9a-zA-Z])
Default = default(?![_0-9a-zA-Z])
Do = do(?![_0-9a-zA-Z])
Double = double(?![_0-9a-zA-Z])
Else = else(?![_0-9a-zA-Z])
Enum = enum(?![_0-9a-zA-Z])
Extern = extern(?![_0-9a-zA-Z])
Float = float(?![_0-9a-zA-Z])
For = for(?![_0-9a-zA-Z])
Goto = goto(?![_0-9a-zA-Z])
If = if(?![_0-9a-zA-Z])
Int = int(?![_0-9a-zA-Z])
Long = long(?![_0-9a-zA-Z])
Register = register(?![_0-9a-zA-Z])
Return = return(?![_0-9a-zA-Z])
Short = short(?![_0-9a-zA-Z])
Signed = signed(?![_0-9a-zA-Z])
Sizeof = sizeof(?![_0-9a-zA-Z])
Static = static(?![_0-9a-zA-Z])
Struct = struct(?![_0-9a-zA-Z])
Switch = switch(?![_0-9a-zA-Z])
Typedef = typedef(?![_0-9a-zA-Z])
Union = union(?![_0-9a-zA-Z])
Unsigned = unsigned(?![_0-9a-zA-Z])
Void = void(?![_0-9a-zA-Z])
Volatile = volatile(?![_0-9a-zA-Z])
While = while(?![_0-9a-zA-Z])

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
