/**
 *   Copyright 2016 Pivotal Software, Inc.
 *   Copyright 2016, Quickstep Research Group, Computer Sciences Department,
 *     University of Wisconsin—Madison.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 **/

#ifndef QUICKSTEP_TYPES_OPERATIONS_BINARY_OPERATIONS_MODULO_BINARY_OPERATION_HPP_
#define QUICKSTEP_TYPES_OPERATIONS_BINARY_OPERATIONS_MODULO_BINARY_OPERATION_HPP_

#include <utility>

#include "types/TypedValue.hpp"
#include "types/operations/binary_operations/ArithmeticBinaryOperation.hpp"
#include "types/operations/binary_operations/BinaryOperationID.hpp"
#include "utility/Macros.hpp"

namespace quickstep {

class Type;
class UncheckedBinaryOperator;

/** \addtogroup Types
 *  @{
 */

/**
 * @brief The BinaryOperation for modulo.
 *
 * @note ModuloBinaryOperation is not commutative: the left argument is the
 *       dividend and the right argument is the divisor.
 **/
class ModuloBinaryOperation : public ArithmeticBinaryOperation {
 public:
  /**
   * @brief Get a reference to the singleton instance of this Operation.
   *
   * @return A reference to the singleton instance of this Operation.
   **/
  static const ModuloBinaryOperation& Instance() {
    static ModuloBinaryOperation instance;
    return instance;
  }

  bool canApplyToTypes(const Type &left,
                       const Type &right) const override;

  const Type* resultTypeForArgumentTypes(const Type &left,
                                         const Type &right) const override;

  const Type* resultTypeForPartialArgumentTypes(const Type *left,
                                                const Type *right) const override;

  bool partialTypeSignatureIsPlausible(const Type *result_type,
                                       const Type *left_argument_type,
                                       const Type *right_argument_type) const override;

  std::pair<const Type*, const Type*> pushDownTypeHint(
      const Type *result_type_hint) const override;

  TypedValue applyToChecked(const TypedValue &left,
                            const Type &left_type,
                            const TypedValue &right,
                            const Type &right_type) const override;

  UncheckedBinaryOperator* makeUncheckedBinaryOperatorForTypes(const Type &left,
                                                               const Type &right) const override;

 private:
  ModuloBinaryOperation()
      : ArithmeticBinaryOperation(BinaryOperationID::kModulo) {
  }

  DISALLOW_COPY_AND_ASSIGN(ModuloBinaryOperation);
};

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_TYPES_OPERATIONS_BINARY_OPERATIONS_MODULO_BINARY_OPERATION_HPP_