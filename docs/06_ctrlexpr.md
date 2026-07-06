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

Моя первая мысль была вынести эти маппинги в отдельный файл, однако это ведёт к необходимости менять два файла при
добавлении нового типа токенов, что гораздо большее зло, чем небольшое количество бойлерплейта.

Очевидно, что подход, который я использовал для матчинга, не подойдёт здесь, потому что
1. У нас поток токенов, а не строки
2. Правила рекурсивные, чего не было в лексере

То есть мне придётся генерировать полноценный парсер - схалявить с помощью регулярок не выйдет.

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
с перла) означает 'match' - после неё идет паттерн, которому должно соответствовать содержимое токена.
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
Эти правила работают не на токенах, а символах. Соответственно, надо как-то сказать нашему генератору
парсеров, что входной тип для этих правила меняется. То есть придётся граф правил красить.

Что я имею в виду - нам надо разметить, какие правила работают на каких типах. Самый простой вариант -
помечать типы рядом с правилами отпадает по очевидным причинам - эти типы бойлерплейт. Можно их пометить
в файле `mappings`, но тогда при добавлении нового правила в грамматику надо не забыть ещё расписать его
типы. Значит, надо менять в нескольких местах. Что тоже неоптимально.

Лучший вариант для меня - помечать типы только во время распаковки и красить граф программно, кидая ошибку,
если цвета смешиваются. Таким образом, правило для `pp-number` в маппингах становится
```
`pp-number` if (token.kind == PPToken::PPNumber) ~ Input { Input(token.value) }
```
И тогда все правила, на которые мы зависим после, будут принимать `Input`.

В итоге грамматика нового языка будет выглядеть как

```
grammar = (rule | mapping)*
rule = rule_start ((mapping_name ~ expression?) | expression) action?
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

mapping = ^ mapping_name if \( raw-expression \) (~ identifier action)?
```

## Унификация парсеров

Вы могли заметить, что я упустил правила `identifier` и `action` - я сделал это намеренно, ибо они такие,
как в 'lex'. Я просто сделал функции для парсинга этих правил видимыми из нового парсера, вынеся в новый
плюсовый файл. Как и класс `Input`. Кроме того, я переписал грамматику и парсер `lex`, чтобы они учитывали
начало строки, а не перенос строки. То есть, `^`, а не `\n`.

В будущем, скорее всего, я вообще эти парсеры буду генерировать, так как мне и так приходится генерировать
полноценный парсер. Почему бы теперь не генерировать парсеры моих DSL? Но это проблема будущего меня.

## Кодогенерация

Так как подход к DSL в этом случае противоположен подходу в лексере, то есть сначала я придумал DSL и только потом
генерирую код, в отличие от лексера, где я повышал абстракцию по ходу разработки, стоит рассказать, какой логики
сгенерированный парсер будет придерживаться.

### PEG

Если коротко - это PEG. Разберём на примере `primary = pp_number | char_literal | identifier | NAN`.
Мы пытаемся сматчить одну ветку (`pp_number`), если не вышло, откатываемся и пытаемся сматчить другую ветку
(`char_literal`). Значит, сгенерированный парсер должен поддерживать откат до предыдущего состояния и функции, которые
будут соответствовать правилам (для каждого правила будет сгенерирована функция, например, `parse_pp_number`) должны
сообщать о нематче.

Возьмём, к примеру, правило `integer = hex_integer | oct_integer | dec_integer`, сгенерированный код будет выглядеть
примерно как
```
auto parse_integer(PPInput &input) -> std::optional<...> {
  auto safepoint = input;
  auto result = parse_hex_integer(input);
  if (result.has_value()) return result;

  result = parse_dec_integer(input);
  if (result.has_value()) return result;

  result = parse_oct_integer(input);
  if (result.has_value()) return result;
  
  input = safepoint;
  return {};
}
```

Конкретный тип возвращаемого значения - `std::optional<std::invoke_result_t<decltype(parse_hex_integer), PPInput>>`.
Выглядит громозко, но компилятор должен суметь тип вывести без явного указания. Так как сырой код в действиях я не хочу
парсить, и не хочу явно задавать возвращаемый тип, то приходится полагаться на магию вывода типа в современных плюсах.
В будущем я ещё планирую по графу ходить и отсекать рекурсивные ветки, чтобы компилятор не ругался, но пока хватает и
этого - рекурсии пока что нет.

Кстати, в kometa я использовал подход как раз с явным указанием возвращаемого типа. Причём этот тип был один и тот же
для всех правил. Мне это не понравилось, так как заставляло пользователя все классы в абстрактном синтаксическом дереве
наследовать от одного интерфейса. Чего я хочу избежать сейчас. Потому что наследовать всё дерево от одного интерфейса
имеет смысл для визиторов, а на них я достаточно насмотрелся на работе. К тому же, я всё равно код прохода по дереву
буду генерировать.

### Bindings

Кстати про kometa. Там я совершил ту же ошибку, что совершил в первой версии грамматики - всё что угодно может быть
именованным. Например
```
repeat = (a:subrule)+ -> { ... }
```
и тогда `a` будет перезаписываться в цикле, что приведёт к трудноотлаживаемым багам и усложнит кодогенерацию. Поэтому,
теперь я байндинги разрешаю только в топ-левеле и если нет ветвлений. Следовательно, грамматика немного усложнится.
```
rule = rule_start (bound_expression (mapping_name ~ bound_expression?) | expression) action?
bound_expression = (!(\{ | \| | \() binding)*
binding = (identifier :)? match
// ...
match = primary primary_suffix?
```

Как теперь будет выглядеть сгенерированный код? Вопрос не праздный, так как не имеет смысла заводить переменные, если
их не использовать в правилах. Давайте на минуту представим, что у нас есть подобное правило

```
rule = a:subrule1 b:subrule2 { call(a, b) }
```

у нас не получится использовать трюк с `std::optional<std::invoke_result_t<decltype(parse_subrule1), Input>>`, потому
что возвращаемый тип задаётся не подправилами, а самим правилом. Давайте попробуем решить задачу в лоб.
```
auto parse_rule(Input &input) {
  auto safepoint = input;
  auto a_opt = parse_subrule1(input);
  if (!a_opt.has_value()) {
    input = safepoint;
    return {};
  }
  
  auto b_opt = parse_subrule2(input);
  if (!b_opt.has_value()) {
    input = safepoint;
    return {};
  }
  
  auto a = a_opt.value();
  auto b = b_opt.value();
  return std::optional(call(a, b));
}
```

В языке с нормальными нулябельными типами (типа Котлина) это бы сработало, но не в плюсах. В плюсах огромная таблица
неявного приведения типов, которая, по идее, должна здесь сработать и вывести тип функции за нас, но нет. При попытке
скомпилировать компилятор ругнётся `error: returning initializer list` и откажется привести к единому
```
std::optional<
  std::invoke_result_t<decltype(call), 
    std::invoke_result_t<decltype(parse_subrule1), Input>,
    std::invoke_result_t<decltype(parse_subrule2), Input>>>
```

Да, я специально выписал весь тип. Но помните про ограничение - в `{}` у нас может быть любой плюсовый код, не только
простой вызов функции и писать парсер плюсов, только чтобы выводить типы, который компилятор, в теории, должен выводить
за нас, мне не хочется.

А так как компилятор должен выводить типы за нас, ему можно подсказать. Нам нужно выражение, в котором будет весь код
с точки зрения типов, но которое не будет выполняться. Его задача - всего лишь дать компилятору подсказку. То есть,
нам нужно лямбда выражение.
```
auto TYPE_HINTER = [](Input &input) -> decltype(auto) {
  auto a = parse_subrule1(input).value();
  auto b = parse_subrule2(input).value();
  return call(a, b);
}
using RETURN_TYPE = std::invoke_result_t<decltype(TYPE_HINTER), Input)>;
```

И код возврата при мисматче будет выглядеть как
```
  if (!a_opt.has_value()) {
    input = safepoint;
    return std::optional<RETURN_TYPE>{};
  }
```

А вся функция будет

```
auto parse_rule(Input &input) {
  auto TYPE_HINTER = [](Input &input) -> decltype(auto) {
    auto a = parse_subrule1(input).value();
    auto b = parse_subrule2(input).value();
    return call(a, b);
  }
  using RETURN_TYPE = std::invoke_result_t<decltype(TYPE_HINTER), Input)>;

  auto safepoint = input;
  auto a_opt = parse_subrule1(input);
  if (!a_opt.has_value()) {
    input = safepoint;
    return std::optional<RETURN_TYPE>{};
  }
  
  auto b_opt = parse_subrule2(input);
  if (!b_opt.has_value()) {
    input = safepoint;
    return std::optional<RETURN_TYPE>{};
  }
  
  auto a = a_opt.value();
  auto b = b_opt.value();
  return std::optional(call(a, b));
}
```

И теперь компилятор сможет правильно вывести возвращаемый тип функции.

### Mappings

