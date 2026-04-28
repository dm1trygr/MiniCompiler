#pragma once

#include <string>

enum class TypeKind { INT, VOID, CLASS };

struct Type {
  TypeKind kind;
  std::string class_name;

  Type() : kind(TypeKind::INT), class_name("") {}
  explicit Type(TypeKind k) : kind(k), class_name("") {}
  Type(TypeKind k, const std::string& name) : kind(k), class_name(name) {}

  bool operator==(const Type& other) const {
    if (kind != other.kind) return false;
    if (kind == TypeKind::CLASS) return class_name == other.class_name;
    return true;
  }

  bool operator!=(const Type& other) const { return !(*this == other); }

  std::string ToString() const {
    switch (kind) {
      case TypeKind::INT:
        return "int";
      case TypeKind::VOID:
        return "void";
      case TypeKind::CLASS:
        return class_name;
    }
    return "unknown";
  }

  static Type Int() { return Type(TypeKind::INT); }
  static Type Void() { return Type(TypeKind::VOID); }
  static Type Class(const std::string& name) {
    return Type(TypeKind::CLASS, name);
  }
};
