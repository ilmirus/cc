# Интерлюдия

## Повышаем абстракцию

Регулярки зло. Не потому, что мало кто их понимает, не потому, что на их изучение требуется некоторое время. Нет,
конкретно в этом проекте это не важно - одно их предусловий, что над проектом я работаю один и мне не надо следовать
общепринятым правилам хорошего кода (спойлер - ещё одно правило я сегодня нарушу).
Важно другое, могу ли я вернуться к проекту через несколько месяцев инактивности и продолжить с того же места,
где остановился. В принципе, да. Проект не такой большой и сложный, и я избегал бойлерплейта,  который бы мешал мне
разобраться в коде. Основная логика сгруппирована в одном месте.

Но вот это вот
```
    const static std::regex patterns(
        R"((\n)|)" // new-line
        R"((L?'.*?(?:'|$))|)" // character-constant
        R"((L?".*?(?:"|$))|)" // string-literal
        R"(([_a-zA-Z][_a-zA-Z0-9]*)|)" // identifier
        "((?:"
            R"((?://.*?(?=\n|$))|)" // singleline-comment
            R"((?:/\*[\s\S]*?(?:\*/|\z))|)" // multiline-comment
            R"((?:[ \t\r\f\v]+))" // whitespace
        ")+)|"
        R"((\.?[0-9](?:[eE][+-]|\.|[0-9a-zA-Z_])*)|)" // pp-number
        R"((-|\+|\*=)|)" // operator
        "(.)" // anything else
    );
```

содержит слишком много скобок, и я уже начал путаться.

А всё потому, что видов этих скобок три
1. Как часть сырых строк `R"(...)"`
2. Как часть, которую надо сматчить
3. Positive lookahead `(?= ...)`
4. Незахватывающий матч, которую я использую для группировки `(?:...)`

Только последний (ну и третий, да) вид для меня имеет смысл - я группирую часть регулярки. Остальное - бойлерплейт, которые мешает
считывать суть.

Кроме того, у текущего подхода я вижу следующий минус - для добавления нового вида токенов мне надо менять три места
1. Энам, в котором они перечислены
2. Регулярки
3. Матчинг токенов регуляркой

В идеале, мне бы хотелось не разделять эти три места, а менять всё в одном.

По этим двум причинам я повышу абстракцию. Обычно под повышением абстракции понимают написание кучи бойлерплейта, который
ничего не делает, только рябит в глазах. Я же буду использовать определение от Аппеля - абстракция (в языках программирования) -
это то, что позволяет избавиться от именования промежуточных значений.

Как этого можно добиться? Не выходя за пределы плюсов - никак. Поэтому я напишу DSL, их которого буду генерировать
плюсы.

В качестве синтаксиса (да и что греха таить, и семантики тоже) я возьму AWK, как язык, в котором есть чёткий паттерн
"условие срабатывания { действие }". Я нахожу подобный паттерн естественным и буду использовать его. Но немного спишу из
OMeta и добавлю имя к каждому подобному паттерну.

В итоге я хочу, чтобы вот этот код
```
std::vector<PPToken> pp_scan(const std::string &file, std::string input) {
    const static std::regex patterns(
        R"((\n)|)" // new-line
        R"((L?'.*?(?:'|$))|)" // character-constant
        R"((L?".*?(?:"|$))|)" // string-literal
        R"(([_a-zA-Z][_a-zA-Z0-9]*)|)" // identifier
        "((?:"
            R"((?://.*?(?=\n|$))|)" // singleline-comment
            R"((?:/\*[\s\S]*?(?:\*/|\z))|)" // multiline-comment
            R"((?:[ \t\r\f\v]+))" // whitespace
        ")+)|"
        R"((\.?[0-9](?:[eE][+-]|\.|[0-9a-zA-Z_])*)|)" // pp-number
        R"((-|\+|\*=)|)" // operator
        "(.)" // anything else
    );

    std::match_results<PPInput::Iterator> match;
    std::vector<PPToken> result;
    PPInput pp_input { input };
    auto iter = pp_input.begin(), end = pp_input.end();

    auto locus = Locus { file, 1, 1 };
    while (std::regex_search(iter, end, match, patterns)) {
        PPToken token { locus, PPToken::kReserved, match.str(0) };
        if (match[PPToken::kNewLine].matched) {
            token.kind = PPToken::kNewLine;
        } else if (match[PPToken::kCharacterConstant].matched) {
            if (!match.str(0).ends_with("'")) throw std::runtime_error("Unclosed character constant");
            token.kind = PPToken::kCharacterConstant;
        } else if (match[PPToken::kStringLiteral].matched) {
            if (!match.str(0).ends_with("\"")) throw std::runtime_error("Unclosed string literal");
            token.kind = PPToken::kStringLiteral;
        } else if (match[PPToken::kIdentifier].matched) {
            token.kind = PPToken::kIdentifier;
        } else if (match[PPToken::kWhitespace].matched) {
            token.kind = PPToken::kWhitespace;
        } else if (match[PPToken::kPPNumber].matched) {
            token.kind = PPToken::kPPNumber;
        } else if (match[PPToken::kOperator].matched) {
            token.kind = PPToken::kOperator;
        } else if (match[PPToken::kNonWhiteSpace].matched) {
            token.kind = PPToken::kNonWhiteSpace;
        }

        result.push_back(token);

        iter += match.length();
        locus = calculate_locus(file, input, iter.ptr);
    }
    if (!result.empty() && result.back().kind != PPToken::kNewLine) {
        result.push_back(PPToken { calculate_locus(file, input, iter.ptr), PPToken::kNewLine, "" });
    }
    return result;
}
```

выглядел как

```
NewLine = \n
CharacterConstant = L?'.*?('|$) { if (!it.ends_with("'") throw std::runtime_error("Unclosed character constant"); }
StringLiteral = L?".*?("|$) { if (!it.ends_with("\"")) throw std::runtime_error("Unclosed string literal"); }
Identifier = [_a-zA-Z][_a-zA-Z0-9]*
Whitespace = (singleLineComment | multiLineComment | whitespace)+
PPNumber = \.?[0-9]([eE][+-]|\.|[0-9a-zA-Z_])*
Operator = -|\+|\*=

singleLineComment = //.*?(?=\n|$)
multiLineComment = /\*[\s\S]*?(\*/|\z)
whitespace = [ \t\r\f\v]+
```

Правил именования только два - ASCII и если не с заглавной, то не идёт в энам.

Грамматика такая - потому что, разумеется, перед тем, как что-то парсить, надо сформулировать грамматику.

```
program = (rule | new-line)*
rule = identifier \s* '=' \s* pattern (\s '{' action '}')? new-line
identifier = [_a-zA-Z][_a-zA-Z0-9]*
pattern = (grouping | [^\s\n])*
grouping = parens | square
parens = '(' (grouping | [^)])* ')'
square = '[' (grouping | [^\]])* ']'
action = [^}]*
new-line = \n
```

Теперь нам надо сгенерировать три файла, чтобы при добавлении нового типа токенов нам не пришлось трогать руками
плюсовый код
1. pptoken.enum.generated.cpp, в котором выписаны элементы энама
2. pptoken.regex.generated.cpp - регекс
3. pptoken.match.generated.cpp - всякие кейсы для свитча.

Структура будет выглядеть как
```
struct PPToken {
	Locus locus;

	enum {
#include "pptoken.enum.generated.cpp"
	} kind;

	std::string value;
};
```

А функция сократится до
```
std::vector<PPToken> pp_scan(const std::string &file, std::string input) {
    const static std::regex patterns(
#include "pptoken.regex.generated.cpp"
    );

    std::match_results<PPInput::Iterator> match;
    std::vector<PPToken> result;
    PPInput pp_input { input };
    auto iter = pp_input.begin(), end = pp_input.end();

    auto locus = Locus { file, 1, 1 };
    while (std::regex_search(iter, end, match, patterns)) {
        auto it = match.str(0);
        PPToken token { locus, PPToken::kReserved, it };
#include "pptoken.match.generated.cpp"

        result.push_back(token);

        iter += match.length();
        locus = calculate_locus(file, input, iter.ptr);
    }
    if (!result.empty() && result.back().kind != PPToken::kNewLine) {
        result.push_back(PPToken { calculate_locus(file, input, iter.ptr), PPToken::kNewLine, "" });
    }
    return result;
}
```

Да, получается, в итоге я всё равно напишу больше кода, чем если бы решал задачу в лоб. И сгенерирую ещё немного. Однако,
в отличие от общепринятого подхода к абстракциям, у этого подхода есть плюс - весь код парсера и кодогенератора будет
спрятан (абстрагирован) не только с глаз долой, но и от результирующего бинарника. Кроме того, в результирующем коде
будет минимум магии - если, конечно, сгенерированный код будет читабельным. А он будет читабельным.

Конечно, этот пример пока маловат для того, чтобы показать всю мощь абстрагирования через кодогенерацию, но в будущем я
ещё не раз буду к нему прибегать.

Причина этого проста. Я вынес всю самую важную логику (то есть саму суть программы) в отдельное место. А всё остальное
в данном случае служит только в качестве обвязки бизнес логики, чтобы она работала - то есть, это бойлерплейт. А то, что
бойлерплейта в итоге больше - ну, такова цена абстракции.

### lex

В качестве расширения исходных файлов буду использовать `lex` и точно так же назову язык. Так как я работаю над проектом
один, мне не нужно выдумывать уникальное имя. Самая первая и дубовая реализация будет расширяться, пока не покроет нужды
лексера.

Программа проста и состоит из трёх стадий - парсинга, встраивания, и кодогенерации.

Парсинг простой и скучный, и я не буду на нём останавливаться.

Встраивание, как следует из названия, встраивает вложенные правила. Например, превращает
```
Whitespace = (singleLineComment | multiLineComment | whitespace)+

singleLineComment = //.*?(?=\n|$)
multiLineComment = /\*[\s\S]*?(\*/|\z)
whitespace = [ \t\r\f\v]+
```

в
```
Whitespace = (//.*?(?=\n|$)|/\*[\s\S]*?(\*/|\z)|[ \t\r\f\v]+)+
```

На самом деле немного всё сложнее, потому что ещё добавляются комментарии и результат ближе к тому, что было в плюсовом
коде изначально, но суть такова - вложенные правила встраивают в родительские и становятся частью одной матч группы.

Очевидно, что в таком случае вложенные правила не могут содержать действий (пока).

Кодогенерация тоже скучна и в основном бойлерплейтна.

## Итоги

Таким образом, я выделил бизнес логику в отдельный файл и эта логика не разбросана россыпью среди бойлерплейта.
То, что ради этого мне пришлось написать ещё бойлерплейта - эту цену я готов заплатить - будущий я будет мне
благодарен. Так как все промежуточные файлы остаются на диске, то это упрощает отладку.

Использование мейка в качестве системы сборки также поможет разобраться в том, как концентрированная бизнес логика
в итоге превращается в бинарь.
