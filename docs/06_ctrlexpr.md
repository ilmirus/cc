# Control Expression

Следующее на очереди после лексера в препроцессоре, разумеется, директивы (раздел 6.8 Preprocessing directives).
Сегодня я бы хотел сосредоточиться на самом первом подразделе (6.8.1 Conditional inclusion), который описывает директивы
```
#if
#elif
#ifdef
#ifndef
```
и оператор `defined`. А конкретно, рассмотрю только условия после директивы `#if`. Остальное будет в секции про
макроподстановки.

Эта секция определяет, какие условия могут быть после директивы.
```
#if <constant-expression> new-line
```

Ограничения для константных выражений описаны в разделе 6.4. Если пройтись по ссылкам и раскрыть правила, то итоговая
грамматика будет выглядеть как
```
constant_expression = conditional_expression

conditional_expression = logical_or_expression (`?` constant_expression `:` conditional_expression)?

logical_or_expression = logical_and_expression (`||` logical_and_expression)*

logical_and_expression = inclusive_or_expression (`&&` inclusive_or_expression)*

inclusive_or_expression = exclusive_or_expression (`|` exclusive_or_expression)*

exclusive_or_expression = and_expression (`^` and_expression)*

and_expression = equality_expression (`&` equality_expression)*

equality_expression = relational_expression (equality_operator relational_expression)*
equality_operator = `==` | `!=`

relational_expression = shift_expression (relational_operator shift_expression)*
relational_operator = `>` | `<` | `>=` | `<=`

shift_expression = additive_expression (shift_operator additive_expression)*
shift_operator = `<<` | `>>`

additive_expression = multiplicative_expression (additive_operator multiplicative_expression)*
additive_operator = `+` | `-`

multiplicative_expression = unary_expression (multiplicative_operator unary_expression)*
multiplicative_operator = `*` | `/` | `%`

unary_expression = `defined` (identifier | `(` identifier `)`)
                 | unary_operator* primary_expression
unary_operator = `&` | `*` | `+` | `-` | `~` | `!`

primary_expression = constant | identifier | `string-literal` | `(` constant_expression `)`

constant = `pp-number` | `character-literal`

identifier = `identifier` | `keyword`
```

Я использовал бектики для обозначения токенов, чтобы парсеру было проще отделять их от правил.
Маппинги этих токенов в `PPToken` я пока опустил. Но они будут выглядеть примерно
```
`defined` if (token.kind == PPToken::kIdentifier && token.value == "defined")
`&` if (token.kind == PPToken::kAnd)
`string-literal` if (token.kind == PPToken::kStringLiteral) { CtrlExpr { kError, token.value } }
```
Основное отличие подобных "правил" в том, что они являются мостами к окружению, а значит, не являются частью грамматики.
Согласно моему определению - это бойлерплейт, необходимый, чтобы вся грамматика заработала.

Поэтому я вынесу эти маппинги в отдельный файл. Если первый файл будет называться `ctrlexpr.grammar`, то второй -
`ctrlexpr.mappings`.

Очевидно, что подход, который я использовал для матчинга, не подойдёт здесь, потому что
1. У нас поток токенов, а не строки
2. Правила рекурсивные, чего не было в лексере

То есть, мне придётся генерировать полноценный парсер - схалявить с помощью регулярок не выйдет.