#include "types.hpp"

Type::Type() : kind(TypeKind::INT), class_name("") {}

Type::Type(TypeKind k) : kind(k), class_name("") {}

Type::Type(TypeKind k, const std::string& name) : kind(k), class_name(name) {}

bool Type::operator==(const Type& other) const {
  if (kind != other.kind) return false;
  if (kind == TypeKind::CLASS) return class_name == other.class_name;
  return true;
}

bool Type::operator!=(const Type& other) const { return !(*this == other); }

std::string Type::ToString() const {
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

Type Type::Int() { return Type(TypeKind::INT); }

Type Type::Void() { return Type(TypeKind::VOID); }

Type Type::Class(const std::string& name) {
  return Type(TypeKind::CLASS, name);
}
