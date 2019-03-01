// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_TORQUE_SOURCE_POSITIONS_H_
#define V8_TORQUE_SOURCE_POSITIONS_H_

#include <iostream>

#include "src/torque/contextual.h"

namespace v8 {
namespace internal {
namespace torque {

class SourceId {
 public:
  static SourceId Invalid() { return SourceId(-1); }
  bool IsValid() const { return id_ != -1; }
  int operator==(const SourceId& s) const { return id_ == s.id_; }
  bool operator<(const SourceId& s) const { return id_ < s.id_; }

 private:
  explicit SourceId(int id) : id_(id) {}
  int id_;
  friend struct SourcePosition;
  friend class SourceFileMap;
};

struct LineAndColumn {
  int line;
  int column;

  static LineAndColumn Invalid() { return {-1, -1}; }
};

struct SourcePosition {
  SourceId source;
  LineAndColumn start;
  LineAndColumn end;

  static SourcePosition Invalid() {
    SourcePosition pos{SourceId::Invalid(), LineAndColumn::Invalid(),
                       LineAndColumn::Invalid()};
    return pos;
  }

  bool CompareStartIgnoreColumn(const SourcePosition& pos) const {
    return start.line == pos.start.line && source == pos.source;
  }

  bool Contains(LineAndColumn pos) const {
    if (pos.line < start.line || pos.line > end.line) return false;

    if (pos.line == start.line && pos.column < start.column) return false;
    if (pos.line == end.line && pos.column >= end.column) return false;
    return true;
  }
};

DECLARE_CONTEXTUAL_VARIABLE(CurrentSourceFile, SourceId);
DECLARE_CONTEXTUAL_VARIABLE(CurrentSourcePosition, SourcePosition);

class SourceFileMap : public ContextualClass<SourceFileMap> {
 public:
  SourceFileMap() = default;
  static const std::string& GetSource(SourceId source) {
    return Get().sources_[source.id_];
  }

  static SourceId AddSource(std::string path) {
    Get().sources_.push_back(std::move(path));
    return SourceId(static_cast<int>(Get().sources_.size()) - 1);
  }

  static SourceId GetSourceId(const std::string& path) {
    for (size_t i = 0; i < Get().sources_.size(); ++i) {
      if (Get().sources_[i] == path) {
        return SourceId(static_cast<int>(i));
      }
    }
    return SourceId::Invalid();
  }

 private:
  std::vector<std::string> sources_;
};

inline std::string PositionAsString(SourcePosition pos) {
  return SourceFileMap::GetSource(pos.source) + ":" +
         std::to_string(pos.start.line + 1) + ":" +
         std::to_string(pos.start.column + 1);
}

inline std::ostream& operator<<(std::ostream& out, SourcePosition pos) {
  return out << PositionAsString(pos);
}

}  // namespace torque
}  // namespace internal
}  // namespace v8

#endif  // V8_TORQUE_SOURCE_POSITIONS_H_
