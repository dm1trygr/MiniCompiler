#pragma once

#include <string>

enum class TypeKind { INT, VOID, CLASS };

struct Type {
  TypeKind kind;
  std::string class_name;

  Type();
  explicit Type(TypeKind k);
  Type(TypeKind k, const std::string& name);

  bool operator==(const Type& other) const;
  bool operator!=(const Type& other) const;

  std::string ToString() const;

  static Type Int();
  static Type Void();
  static Type Class(const std::string& name);
};
