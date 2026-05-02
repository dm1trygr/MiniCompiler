# Компилятор

Проект с курса по компиляторам в МФТИ

## Грамматика

```
Program ::= Statement*;
Statement ::= ClassDeclaration | VariableDeclaration | Assignment | IfStatement | WhileStatement | PrintStatement | ReturnStatement;
ClassDeclaration ::= "class" Identifier "{" ClassMember* "}";
ClassMember ::= FieldDeclaration | MethodDeclaration;
FieldDeclaration ::= "declare" Identifier ":" Type ";";
MethodDeclaration ::= "method" Identifier "(" ParameterList? ")" ":" Type Block;
ParameterList ::= Parameter ("," Parameter)*;
Parameter ::= Identifier ":" Type;
VariableDeclaration ::= "declare" Identifier ":" Type ";";
Assignment ::= Identifier "=" Expression ";";
IfStatement ::= "if" "(" Expression ")" Block ("else" Block)?;
WhileStatement ::= "while" "(" Expression ")" Block;
PrintStatement ::= "print" "(" Expression ")" ";";
ReturnStatement ::= "return" Expression? ";";
Block ::= "{" Statement* "}";
Expression ::= Comparison;
Comparison ::= Additive ("==" Additive)?;
Additive ::= Multiplicative (("+" | "-") Multiplicative)*;
Multiplicative ::= Primary (("*" | "/") Primary)*;
Primary ::= IntLiteral | Identifier | "(" Expression ")";
Type ::= "int" | "void";
Identifier ::= [a-zA-Z_][a-zA-Z0-9_]*;
IntLiteral ::= [0-9]+;
```

## Что поддерживается

Данная итерация по сути пока представляет собой базовый интерпретатор (в следующей планируется добавление поддержки IR-генератора). Поддержаны:

### Типы данных
- `int`
- `void` как тип возвращаемого значения метода/функции

### Операторы
- Арифметические: `+`, `-`, `*`, `/`
- Сравнение: `==`

### Конструкции
- **Объявление переменных**: `declare x: int;`
- **Присваивание**: `x = 5;`
- **Условный оператор**: `if (condition) { ... } else { ... }`
- **Цикл**: `while (condition) { ... }`
- **Вывод**: `print(expression);`

### Заготовка для классов и методов

**Важно**: В текущей итерации классы и методы распознаются парсером, в AST тоже попадают, но они **игнорируются** интерпретатором (ибо когда реализовывал визиторы классы и методы не существовали в коде впринципе). В следующей итерации планируется поддержка классов в IR-генераторе

Синтаксис объявления классов, полей и методов в них следующий:

```
class ClassName {
    declare field: int;

    method methodName(arg: int): int {
        return arg;
    }
}
```

### Визиторы

В проекте поддержаны следующие визиторы:

- `PrintVisitor` — сохраняет дерево в файл
- `Interpreter` — интерпретирует программу
- `SemanticAnalyzer` — семантический анализ

### Способ задания кода

В текущей версии исходный код программы задается как строка в `main.cpp`:

```cpp
std::string code = R"(
    declare x: int;
    x = 0;

    if (x == 0) {
        print(100);
    }
)";
```

В следующей итерации планируется переход на чтение кода из файла

## Как запустить

1. Сначала соберем проект:

```bash
./build.sh
```

2. Запуск:

```bash
./compiler
```

3. После запуска выполнятся следующие шаги:

- **Лексический анализ**: получаем массив токенов, выводим их количество
- **Построение AST**: парсим токены и строим абстрактное синтаксическое дерево
- **Вывод дерева в файл**: сохраняем дерево в файл `ast_tree.txt`
- **Интерпретация**: интерпретируем и выводим результаты `print()`
- **Семантический анализ**: проверяем корректность программы
