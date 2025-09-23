NewLine = \n

Whitespace = singleLineComment | multiLineComment | whitespace
singleLineComment = //[^\n]*
multiLineComment = /\*[\s\S]*?(\*/|$) { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
whitespace = [ \t\r\f\v]+

Auto = auto notId
Break = break notId
Case = case notId
Char = char notId
Const = const notId
Continue = continue notId
Default = default notId
Do = do notId
Double = double notId
Else = else notId
Enum = enum notId
Extern = extern notId
Float = float notId
For = for notId
Goto = goto notId
If = if notId
Int = int notId
Long = long notId
Register = register notId
Return = return notId
Short = short notId
Signed = signed notId
Sizeof = sizeof notId
Static = static notId
Struct = struct notId
Switch = switch notId
Typedef = typedef notId
Union = union notId
Unsigned = unsigned notId
Void = void notId
Volatile = volatile notId
While = while notId

DotDotDot = \.\.\.
Semicolon = ;
OpenSquare = \[
CloseSquare = \]
OpenParen = \(
CloseParen = \)
Dot = \.
Arrow = ->
BitNot = ~
Inc = \+\+
Dec = --
AndAnd = &&
AndAssign = &=
And = &
StarAssign = \*=
Star = \*
Minus = -
NotAssign = !=
Not = !
DivAssign = /=
Div = /
ModAssign = %=
Mod = %
PlusAssign = \+=
Plus = \+
ShiftLeftAssign = <<=
ShiftLeft = <<
ShiftRightAssign = >>=
ShiftRight = >>
LessEq = <=
Less = <
GrEq = >=
Gr = >
EqEq = ==
XorAssign = \^=
Xor = \^
OrOr =\|\|
OrAssign = \|=
Or = \|
Query = \?
Colon = :
Assign = =
Comma = ,
HashHash = ##
Hash = #

inline identifierRest = [_a-zA-Z0-9]
inline notId = (?!identifierRest)

CharacterConstant = L?'.*?('|$) { if (!it.ends_with("'")) throw std::runtime_error("Unclosed character constant"); }
StringLiteral = L?".*?("|$) { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }

Identifier = [_a-zA-Z] identifierRest*

PPNumber = \.?[0-9]([eE][+-]|\.|[0-9a-zA-Z_])*
Operator = -|\+|\*=
NonWhiteSpace = .
