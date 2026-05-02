# Компилятор

Проект с курса по компиляторам в МФТИ

## Грамматика

```
Program ::= Statement*;
Statement ::= ClassDeclaration | MethodDeclaration | VariableDeclaration | Assignment | FieldAssignment | IfStatement | WhileStatement | PrintStatement | ReturnStatement | ExpressionStatement;
ClassDeclaration ::= "class" Identifier "{" ClassMember* "}";
ClassMember ::= FieldDeclaration | MethodDeclaration;
FieldDeclaration ::= "declare" Identifier ":" Type ";";
MethodDeclaration ::= "method" Identifier "(" ParameterList? ")" ":" Type Block;
ParameterList ::= Parameter ("," Parameter)*;
Parameter ::= Identifier ":" Type;
VariableDeclaration ::= "declare" Identifier ":" Type ";";
Assignment ::= Identifier "=" Expression ";";
FieldAssignment ::= Identifier "." Identifier "=" Expression ";";
IfStatement ::= "if" "(" Expression ")" Block ("else" Block)?;
WhileStatement ::= "while" "(" Expression ")" Block;
PrintStatement ::= "print" "(" Expression ")" ";";
ReturnStatement ::= "return" Expression? ";";
ExpressionStatement ::= Expression ";";
Block ::= "{" Statement* "}";
Expression ::= Comparison;
Comparison ::= Additive ("==" Additive)?;
Additive ::= Multiplicative (("+" | "-") Multiplicative)*;
Multiplicative ::= Primary (("*" | "/") Primary)*;
Primary ::= IntLiteral | Identifier | FieldAccess | MethodCall | FunctionCall | "(" Expression ")";
FieldAccess ::= Identifier "." Identifier;
MethodCall ::= Identifier "." Identifier "(" ArgumentList? ")";
FunctionCall ::= Identifier "(" ArgumentList? ")";
ArgumentList ::= Expression ("," Expression)*;
Type ::= "int" | "void" | Identifier;
Identifier ::= [a-zA-Z_][a-zA-Z0-9_]*;
IntLiteral ::= [0-9]+;
```

## Что поддерживается

### Типы данных
- `int`
- `void` как тип возвращаемого значения метода/функции
- Свои типы-классы (определенные как `class Type {}`)

### Операторы
- Арифметические: `+`, `-`, `*`, `/`
- Сравнение: `==`

### Конструкции
- **Объявление переменных**: `declare x: int;`
- **Присваивание**: `x = 5;`
- **Условный оператор**: `if (condition) { ... } else { ... }`
- **Цикл**: `while (condition) { ... }`
- **Вывод**: `print(expression);`

### Классы и методы

Синтаксис объявления классов, полей и методов в них следующий:

```
class ClassName {
    declare field: int;

    method methodName(arg: int): int {
        return arg;
    }
}
```

В том числе поддержаны и функции в глобальном скоупе

### Визиторы

В проекте поддержаны следующие визиторы:

- `PrintVisitor` — сохраняет дерево в файл
- `Interpreter` — интерпретирует программу (**Важно**: этот визитор не поддерживает классы и методы)
- `SemanticAnalyzer` — семантический анализ
- `IrGenerator` - генератор промежуточного представления

## Как запустить

1. Сначала соберем проект (при этом если параллельно с директорией проекта нет директории с LLVM, выполнится скачивание и сборка):

```bash
./build.sh
```

2. Запуск:

```bash
./compiler <путь-к-файлу-с-кодом>
```

3. После запуска выполнятся следующие шаги:

- **Лексический анализ**: получаем массив токенов, выводим их количество
- **Построение AST**: парсим токены и строим абстрактное синтаксическое дерево
- **Семантический анализ**: проверяем корректность программы
- **Генерация промежуточного представления**: генерируем промежуточное представление и сохраняем в output.ll
