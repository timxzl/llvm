//===- IRMover.h ------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LINKER_IRMOVER_H
#define LLVM_LINKER_IRMOVER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include <functional>

namespace llvm {
class GlobalValue;
class Module;
class StructType;
class Type;

class IRMover {
  struct StructTypeKeyInfo {
    struct KeyTy {
      ArrayRef<Type *> ETypes;
      bool IsPacked;
      KeyTy(ArrayRef<Type *> E, bool P);
      KeyTy(const StructType *ST);
      bool operator==(const KeyTy &that) const;
      bool operator!=(const KeyTy &that) const;
    };
    static StructType *getEmptyKey();
    static StructType *getTombstoneKey();
    static unsigned getHashValue(const KeyTy &Key);
    static unsigned getHashValue(const StructType *ST);
    static bool isEqual(const KeyTy &LHS, const StructType *RHS);
    static bool isEqual(const StructType *LHS, const StructType *RHS);
  };

public:
  class IdentifiedStructTypeSet {
    // The set of opaque types is the composite module.
    DenseSet<StructType *> OpaqueStructTypes;

    // The set of identified but non opaque structures in the composite module.
    DenseSet<StructType *, StructTypeKeyInfo> NonOpaqueStructTypes;

  public:
    void addNonOpaque(StructType *Ty);
    void switchToNonOpaque(StructType *Ty);
    void addOpaque(StructType *Ty);
    StructType *findNonOpaque(ArrayRef<Type *> ETypes, bool IsPacked);
    bool hasType(StructType *Ty);
  };

  IRMover(Module &M);

  typedef std::function<void(GlobalValue &)> ValueAdder;

  /// Move in the provide values in \p ValuesToLink from \p Src.
  ///
  /// - \p AddLazyFor is a call back that the IRMover will call when a global
  ///   value is referenced by one of the ValuesToLink (transitively) but was
  ///   not present in ValuesToLink. The GlobalValue and a ValueAdder callback
  ///   are passed as an argument, and the callback is expected to be called
  ///   if the GlobalValue needs to be added to the \p ValuesToLink and linked.
  ///
  /// Returns true on error.
  bool move(std::unique_ptr<Module> Src, ArrayRef<GlobalValue *> ValuesToLink,
            std::function<void(GlobalValue &GV, ValueAdder Add)> AddLazyFor);
  Module &getModule() { return Composite; }

private:
  Module &Composite;
  IdentifiedStructTypeSet IdentifiedStructTypes;
};

} // End llvm namespace

#endif
