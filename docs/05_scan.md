# Инлайнинг

Вот и настал тот момент, когда мне пришлось вернуть инлайнинг. Я поддерживал ключевые слова и каждое правило завершалось
```
(?![_0-9a-zA-Z])
```
Что означает, что после матча не должно быть этих символов (negative lookahead). То есть, была вереница правил типа
```
Auto = auto(?![_0-9a-zA-Z])
Break = break(?![_0-9a-zA-Z])
Case = case(?![_0-9a-zA-Z])
Char = char(?![_0-9a-zA-Z])
```

Разумеется, эти суффиксы желательно вынести в отдельное правило и его же использовать в правиле для идентификаторов.
Например,
```
inline identifierRest = [_a-zA-Z0-9]
inline notId = (?!identifierRest)

Identifier = [_a-zA-Z] identifierRest*
```
Я добавил модификатор `inline`, чтобы отличать эти правила от правил для пробельных символов
```
Whitespace = singleLineComment | multiLineComment | whitespace
singleLineComment = //[^\n]*
multiLineComment = /\*[\s\S]*?(\*/|$) { if (!it.ends_with("*/")) throw std::runtime_error("Unclosed multiline comment"); }
whitespace = [ \t\r\f\v]+
```
Последние присутствуют в матче, тогда как новые правила должны встраиваться в места использования.

