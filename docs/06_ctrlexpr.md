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
`string-literal` if (token.kind == PPToken::kStringLiteral)
```
Основное отличие подобных "правил" в том, что они являются мостами к окружению, а значит, не являются частью грамматики.
Согласно моему определению - это бойлерплейт, необходимый, чтобы вся грамматика заработала.

Поэтому я вынесу эти маппинги в отдельный файл. Если первый файл будет называться `ctrlexpr.grammar`, то второй -
`ctrlexpr.mappings`.

Очевидно, что подход, который я использовал для матчинга, не подойдёт здесь, потому что
1. У нас поток токенов, а не строки
2. Правила рекурсивные, чего не было в лексере

То есть, мне придётся генерировать полноценный парсер - схалявить с помощью регулярок не выйдет.

## Распаковка

Помимо того, что теперь надо матчить токены, а не символы, мне нужно ещё матчить что находится внутри
токенов. Например, если раскрыть грамматику `primary`, там будет
```
primary = pp_number | char_literal | identifier | NAN

pp_number = `pp_number` ~ i:integer us:unsigned_suffix? { us.empty() ? CtrlExpr { CtrlExpr::kSigned, i } : CtrlExpr { CtrlExpr::kUnsigned, i } }

char_literal = `char_literal` ~ L? ' c:[^'] ' { CtrlExpr { CtrlExpr::kSigned, code_point(c) } }

identifier = `identifier` ~ { CtrlExpr { CtrlExpr::kSigned, defined(it) } }

NAN = . { CtrlExpr { CtrlExpr::kError, 0 }; }
```
обратите внимание на правило `pp_number`. Это специальное правило распаковки - тильда (которую я украл
с перла) означает 'match' - после неё идет паттерн, которому должо соответствовать содержимое токена.
```
integer = hex_integer | oct_integer | dec_integer

hex_integer = 0 (x | X) hex_digit+
oct_integer = 0 oct_digit+
dec_integer = dec_digit+

unsigned_suffix = u l? l?
                | U l? l?
                | u L? L?
                | U L? L?
                | l l? u
                | l l? U
                | L L? u
                | L L? U

hex_prefix = 0 (x | X)

hex_digit = [0-9a-fA-F]
oct_digit = [0-7]
dec_digit = [0-9]
```
Это правила работают не на токенах, а символах. Соответственно, надо как-то сказать нашему генератору
парсеров, что входной тип для этих правила меняется. То есть, придётся граф правил красить.

Что я имею в виду - нам надо разметить, какие правила работают на каких типах. Самый простой вариант -
помечать типы рядом с правилами отпадает по очевидным причинам - эти типы бойлерплейт. Можно их пометить
в файле `mappings`, но тогда при добавлении нового правила в грамматику надо не забыть ещё расписать его
типы. Значит, надо менять в нескольких местах. Что тоже неоптимально.

Лучший вариант для меня - помечать типы только во время распаковки и красить граф программно, кидая ошибку,
если цвета смешиваются. Таким образом, правило для `pp-number` в маппингах становится
```
`pp-number` if (token.kind == PPToken::PPNumber) ~ Input { Input(token.value) }
```
И тогда все правила, на которые мы зависим после будут принимать `Input`.

В итоге, грамматика нового языка будет выглядеть как

grammar
```
grammar = rule*
rule = rule_start (mapping_name ~ expression? action?) | (expression action?)
rule_start = ^ name \=
name = identifier | mapping_name
expression = sequence (\| sequence)*
sequence = (!rule_start match)*
match = (identifier :)? primary primary_suffix
primary = name
        | \.
        | \( expression \)
primary_suffix = (\? | \* | \+)?
mapping_name = ` [^`]+ `
```
mapping
```
mappings = mapping*
mapping = ^ mapping_name "if" raw-expression (~ identifier action)?
```

## Унификация парсеров

Вы могли заметить, что я упустил правила `identifier` и `action` - я сделал это намеренно, ибо они такие,
как в 'lex'. Я просто сделал функции для парсинга этих правил видимыми из нового парсера, вынеся в новый
плюсовый файл. Как и класс `Input`. Кроме того, я переписал грамматику и парсер `lex`, чтобы они учитывали
начало строки, а не перенос строки. То есть, `^`, а не `\n`.

В будущем, скорее всего, я вообще эти парсеры буду генерировать, так как мне и так приходится генерировать
полноценный парсер. Почему бы теперь не генерировать парсеры моих DSL? Но это проблема будущего меня.
