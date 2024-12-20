# PL/0 解释器控制语句实现设计文档

## 概述

本设计文档描述了如何在一个简化的 PL/0 解释器中实现以下控制语句：

- **else/elif 子句**
- **exit 语句**
- **return 语句及返回值的实现**
- **for 语句实现**

PL/0 是一种简单的编程语言，广泛用于教学与编译原理的研究。目标是设计一个支持控制流的 PL/0 解释器，扩展 PL/0 的语法，支持类似于 C 语言的控制结构。

## 1. else/elif 子句

### 1.1 语法

```text
if condition then statement [else statement]
```

- condition：布尔表达式，决定执行哪个语句。
- statement：执行的语句块。
- 可选的 else 子句，表示条件为 false 时执行的语句块。

### 1.2 C++ 实现

```C++
class IfStatement : public Statement {
    Expression* condition;
    Statement* thenStmt;
    Statement* elseStmt;

public:
    IfStatement(Expression* cond, Statement* thenStmt, Statement* elseStmt)
        : condition(cond), thenStmt(thenStmt), elseStmt(elseStmt) {}

    void execute() override {
        if (condition->evaluate()) {
            thenStmt->execute();
        } else if (elseStmt != nullptr) {
            elseStmt->execute();
        }
    }
};
```

### 1.3 说明

- `IfStatement` 类用来实现 `if` 语句，`condition` 为布尔表达式，`thenStmt` 和 `elseStmt` 分别表示 then 和 else 子句的语句。
- 使用 `evaluate()` 方法来计算 `condition` 的值，依据值决定是否执行 `thenStmt` 或 `elseStmt`。

## 2. exit 语句

### 2.1 语法设计

```text
exit
```

- `exit` 语句用于终止程序的执行。

### 2.2 C++ 实现

```C++
class ExitStatement : public Statement {
public:
    void execute() override {
        std::cout << "Exiting the program." << std::endl;
        exit(0);  // 退出程序
    }
};
```

### 2.3 说明

- `ExitStatement` 类直接调用 `exit(0)` 来终止程序执行，并打印退出信息。

## 3. return 语句及返回值的实现

### 3.1 C++ 实现

```C++
class ReturnStatement : public Statement {
    Expression* returnValue;

public:
    ReturnStatement(Expression* expr) : returnValue(expr) {}

    void execute() override {
        int value = returnValue->evaluate(); // 获取返回值
        std::cout << "Returning value: " << value << std::endl;
        throw ReturnException(value); // 使用异常机制返回值
    }
};

class ReturnException {
    int returnValue;

public:
    ReturnException(int value) : returnValue(value) {}
    int getValue() const { return returnValue; }
};
```

### 3.2 说明

- `ReturnStatement` 类通过 `evaluate()` 方法计算返回值，并使用自定义异常 `ReturnException` 模拟返回值的传递。
- 该异常在调用时会被捕获，用于返回值的传递。

## 4. for 循环语句

### 4.1 语法设计

```C++
for initialization; condition; increment do statement
initialization：初始化表达式，通常用于初始化循环变量。
condition：循环条件，布尔表达式。
increment：循环递增表达式。
statement：循环体，执行的语句。
4.2 C++ 实现
class ForStatement : public Statement {
    Statement* initialization;
    Expression* condition;
    Statement* increment;
    Statement* body;

public:
    ForStatement(Statement* init, Expression* cond, Statement* incr, Statement* stmt)
        : initialization(init), condition(cond), increment(incr), body(stmt) {}

    void execute() override {
        initialization->execute(); // 执行初始化语句
        while (condition->evaluate()) { // 判断条件是否成立
            body->execute(); // 执行循环体
            increment->execute(); // 执行递增语句
        }
    }
};
```

### 4.3 说明

- `ForStatement` 类表示 for 循环，包含初始化、条件判断、递增和循环体四个部分。
- initialization 是初始化语句，condition 是循环条件，increment 是递增语句，body 是循环体。

## 5. 自增和自减运算符

### 5.1 C++ 实现

```C++
class IncrementStatement : public Statement {
    Variable* var;

public:
    IncrementStatement(Variable* v) : var(v) {}

    void execute() override {
        var->setValue(var->getValue() + 1);
    }
};

class DecrementStatement : public Statement {
    Variable* var;

public:
    DecrementStatement(Variable* v) : var(v) {}

    void execute() override {
        var->setValue(var->getValue() - 1);
    }
};
```

### 5.2 说明

- `IncrementStatement` 和 `DecrementStatement` 类分别表示自增和自减操作，操作的目标是 `Variable` 类型的对象。
