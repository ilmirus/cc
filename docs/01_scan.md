Перед тем, как начнём, если вы хотите следовать моим шагам, рекомендую скачать сишный стандарт по ссылке https://web.archive.org/web/20200909074736if_/https://www.pdf-archive.com/2014/10/02/ansi-iso-9899-1990-1/ansi-iso-9899-1990-1.pdf

В разделе 5.1.1.2 расписаны шаги трансляции. Там всё расписано максимально непонятно для обычного человека, поэтому, если коротко, то трансляция состоит из следующих шагов

1. Сырцы токенизируются в токены препроцессора.
2. Директивы препроцессора исполняются, инклюды токенизируются, директивы в них исполняются и всё встраивается в текущую единицу трансляции.
3. Токены препроцессора транслируются в токены.
4. Поток токенов парсится, анализируется и транслируется в бинарные объекты.
5. Бинарные объекты линкуются линкером.

Я опустил некоторые шаги, например, замену комментариев на пробелы, но они будут покрыты, не беспокойтесь.
Триграммы (5.2.1.1) теперь не используются, поэтому я их буду игнорировать.
Раздел 5.2.1 описывает минимальный набор символов, которые нужно будет поддержать.
Юникод может быть поддержан, но не обязателен, поэтому я пока юникод буду игнорировать.

Можно переходить к реализации.
Начнём, разумеется, с первого шага.
Раздел 6.1 описывает, что представляют собой токены и токены препроцессора.
Последние вот они

```
preprocessing-token
	header-name
	identifier
	pp-number
	character-constant
	string-literal
	operator
	punctuator
	each non-white-space character that cannot be one of the above
```

В плюсах это будет выглядеть как
```
struct PPToken {
	Locus locus;

	enum {
		kWhitespace,
		kNewLine,
		kHash,
		kHashHash,
		kHeaderName,
		kIdentifier,
		kPPNumber,
		kCharacterConstant,
		kStringLiteral,
		kOperator,
		kPunctuator,
		kNonWhiteSpace
	} kind;

	std::string value;
};
```
в рамках препроцессора нам не важны точные значения констант, поэтому все токены это просто строки с тегом и их координатами в сырце.
Вообще, `Locus` объявлен как
```
struct Locus {
	std::string file;
	int row, column;
};
```
Конкретно сейчас он нам не особо нужен, но будет нужен для фазы анализа, чтобы кидать ошибки и варнинги.
Плюс, я добавил пробельные символы и новые строки, которые важны для препроцессора.

`#` и `##` используются только для препроцессора и поэтому проще для последующих шагов их обрабатывать явно.

Те символы, которые не определены в стандарте, например, `@`, необходимо обрабатывать, не кидая ошибки (пока).
Потому что они могут быть внутри директив `#if 0 ... #endif`.

Почему я использую тяжёлый `std::string` вместо легковесного `std::string_view`?
Потому что авторы LLVM - ленивые пидорасы, из-за которых конкатенация `std::string` и `std::string_view` будет доступна только через пару лет, как и конкатенация двух `std::string_view` - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2591r5.html.
Из комментариев к ответу https://stackoverflow.com/questions/44636549/why-is-there-no-support-for-concatenating-stdstring-and-stdstring-view -
"Wow. Just wow! This seems to be the worst possible reason for leaving something out.
The standard is supposed to be for the good of the language, not for any particular implementation."

## Первые шаги

После того, как я написал необходимую для тестирования обвязку (всё просто - читаем файл и сравниваем вывод с тем, что ожидается), я начал реализовывать в лоб.
И практически сразу же наткнулся на проблему. Не буду распинаться, просто приведу код основного цикла.
```
        if (input.empty()) break;

        auto c = peek(input);
        std::string value;
        auto locus = calculate_locus(file, initial, input);

        #define NEXT do { \
            value += c; \
            input = skip(input); \
            c = peek(input); \
        } while(0)

        // new-line
        if (c == '\n') {
            input = skip(input);
            result.push_back(PPToken { locus, PPToken::kNewLine, "" });
            continue;
        }
        // character
        if (c == '\'' || c == 'L' && peek(input, 1) == '\'') {
            if (c == 'L') NEXT;
            do NEXT; while (c != '\0' && c != '\'');
            if (c == '\0') throw std::logic_error("Unclosed character constant");
            NEXT;
            result.push_back(PPToken { locus, PPToken::kCharacterConstant, value });
            continue;
        }
        // identifier
        if (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
            while (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9')
                NEXT;
            result.push_back(PPToken { locus, PPToken::kIdentifier, value });
            continue;
        }
        assert(false && "unimplemented");

        #undef NEXT
```
Как видите, мне даже пришлось использовать дефайн для того, чтобы сократить код. TDD учит нас, что после того, как тесты прошли, приходит время рефакторинга.
Обычно под рефакторингом понимают вынесение отдельных функций, объединение функций в классы, и т.д.
Но моя проблема с данным кодом не в том, что код нечитаемый - он читаемый, а в том, что логика размазана и присутствует куча лишней логики, которая нужна только для того, чтобы основная логика сработала.
То есть, мне не нравится смешение так называемой бизнес логики с бойлерплейтом.

Что я понимаю под бизнес логикой в данном случае?
Меня не интересует, как происходит работа с отдельными символами.
Также не интересует, как токены добавляются в результирующий вектор.
В идеале, я бы хотелось видеть что-то типа
```
NewLine = \n
CharacterConstant = L? ' .* ($ | ') -> if (!matched.ends_with("'")) throw std::runtime_error("Unclosed character constant");
Identifier = [_a-zA-Z][_a-zA-Z0-9]*
```
Что мне нравится в этом представлении?
Сразу видно, чего не хватает в поддержке `CharacterConstant`.
По мере усложнения, например, если необходимо поддержать escape sequences, сразу будет видна логика, которая, опять-таки, иначе будет размазана, что усложнит отладку.
Именно эта причина - неочевидность логики, смешанной с бойлерплейтом, на мой взгляд, и приводит к тому, что отладка сильно сложнее написания кода.
У бизнес логики, разумеется, есть сложность, которую не уменьшить.
Поэтому единственная сложность, которую можно уменьшить - это сложность бойлерплейта.

Давайте, кстати, расширим `CharacterConstant`, чтобы включить escape sequences.
```
CharacterConstant = L? ' c-char* ($ | ') -> if (!matched.ends_with("'")) throw std::runtime_error("Unclosed character constant");
c-char = escape-sequence | [^\n\\']
escape-sequence = simple-escape-sequence
                | octal-escape-sequence
                | hexadecimal-escape-sequence
simple-escape-sequence = \\[\\'"?abfnrtv]
octal-escape-sequence = \\[0-7]{1-3}
hexadecimal-escape-sequence = \\x[0-9a-fA-F]+
```
Теперь становится очевидно, что после решения этой проблемы у нас будет две проблемы.
Потому что плюсы поддерживают регекспы.

Переписав код, получаем более прямой код, в котором вся логика матчинга отдельных символов запрятана за регекспами.
```
    const static std::regex patterns(
        R"((\n)|)" // new-line
        R"((L?'.*(?:'|$))|)" // character-constant
        R"(([_a-zA-Z][_a-zA-Z0-9]*)|)" // identifier
    );
```
И основной цикл
```
    while (!input.empty() && std::regex_search(input, match, patterns)) {
        PPToken token { locus, 0, match.str(0) };
        if (match[1].matched) {
            token.kind = PPToken::kNewLine;
        } else if (match[2].matched) {
            if (!match.str(0).ends_with("'")) throw std::runtime_error("Unclosed character constant");
            token.kind = PPToken::kCharacterConstant;
        } else if (match[3].matched) {
            token.kind = PPToken::kIdentifier;
        }

        result.push_back(token);

        input = input.substr(match.position() + match.length());
        locus = calculate_locus(file, initial, input);
    }
```

И теперь для расширения `CharacterConstant` нам не нужно будет менять основной цикл - только паттерны, в которых и заключена основная логика.

Кстати, если мы поменяем нашу структурку, чтобы номера паттернов совпадали со значения в энаме, примерно так
```
    enum {
        kReserved,
        kNewLine,
        kCharacterConstant,
        kIdentifier,
        ...
    } kind;
```
то в основном цикле можно будет использовать эти значения вместо волшебных констант.

```
    while (!input.empty() && std::regex_search(input, match, patterns)) {
        PPToken token { locus, 0, match.str(0) };
        if (match[PPToken::kNewLine].matched) {
            token.kind = PPToken::kNewLine;
        } else if (match[PPToken::kCharacterConstant].matched) {
            if (!match.str(0).ends_with("'")) throw std::runtime_error("Unclosed character constant");
            token.kind = PPToken::kCharacterConstant;
        } else if (match[PPToken::kIdentifier].matched) {
            token.kind = PPToken::kIdentifier;
        }

        result.push_back(token);

        input = input.substr(match.position() + match.length());
        locus = calculate_locus(file, initial, input);
    }
```

Теперь можно спокойно и без особой отладки расширять код, пока не покроем весь лексер. И можно будет потом подумать, как отрефачить этот код ещё лучше.
Потому что мне не нравится, что логика обработки ошибки затесалась в бойлерплейт, в который мы вынесли всё неинтересное.

