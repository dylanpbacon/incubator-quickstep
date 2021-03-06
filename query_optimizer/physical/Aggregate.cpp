/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 **/

#include "query_optimizer/physical/Aggregate.hpp"

#include <string>
#include <vector>

#include "query_optimizer/OptimizerTree.hpp"
#include "query_optimizer/expressions/Alias.hpp"
#include "query_optimizer/expressions/AttributeReference.hpp"
#include "query_optimizer/expressions/ExpressionUtil.hpp"
#include "query_optimizer/expressions/NamedExpression.hpp"
#include "query_optimizer/expressions/PatternMatcher.hpp"
#include "query_optimizer/expressions/Predicate.hpp"
#include "query_optimizer/physical/PartitionSchemeHeader.hpp"
#include "query_optimizer/physical/Physical.hpp"
#include "utility/Cast.hpp"

#include "glog/logging.h"

namespace quickstep {
namespace optimizer {
namespace physical {

namespace E = ::quickstep::optimizer::expressions;

std::vector<E::AttributeReferencePtr> Aggregate::getOutputAttributes() const {
  std::vector<E::AttributeReferencePtr> output_attributes(
      E::ToRefVector(grouping_expressions_));
  const std::vector<E::AttributeReferencePtr> aggregate_output_attributes =
      E::ToRefVector(aggregate_expressions_);
  output_attributes.insert(output_attributes.end(),
                           aggregate_output_attributes.begin(),
                           aggregate_output_attributes.end());
  return output_attributes;
}

std::vector<E::AttributeReferencePtr> Aggregate::getReferencedAttributes()
    const {
  std::vector<E::AttributeReferencePtr> referenced_attributes;
  for (const E::NamedExpressionPtr &grouping_expression :
       grouping_expressions_) {
    const std::vector<E::AttributeReferencePtr> referenced_attributes_in_expression =
        grouping_expression->getReferencedAttributes();
    referenced_attributes.insert(referenced_attributes.end(),
                                 referenced_attributes_in_expression.begin(),
                                 referenced_attributes_in_expression.end());
  }
  for (const E::AliasPtr &aggregate_expression :
       aggregate_expressions_) {
    const std::vector<E::AttributeReferencePtr> referenced_attributes_in_expression =
        aggregate_expression->getReferencedAttributes();
    referenced_attributes.insert(referenced_attributes.end(),
                                 referenced_attributes_in_expression.begin(),
                                 referenced_attributes_in_expression.end());
  }
  if (filter_predicate_ != nullptr) {
    const std::vector<E::AttributeReferencePtr> referenced_attributes_in_predicate =
        filter_predicate_->getReferencedAttributes();
    referenced_attributes.insert(referenced_attributes.end(),
                                 referenced_attributes_in_predicate.begin(),
                                 referenced_attributes_in_predicate.end());
  }

  // TODO(jianqiao): This is a quick fix to the COUNT(*) problem that we retain
  // at least one column from the child plan to survive the PruneColumns physical
  // optimization. The comprehensive approach should also try to optimize the
  // child plan with regard to the knowledge that the aggregation actually don't
  // care about the values of the child plan's output but needs only the number
  // of rows of the output.
  if (referenced_attributes.size() == 0) {
    DCHECK_GT(input_->getOutputAttributes().size(), static_cast<std::size_t>(0));
    referenced_attributes.emplace_back(input_->getOutputAttributes()[0]);
  }

  return referenced_attributes;
}

PhysicalPtr Aggregate::copyWithNewProjectExpressions(
    const std::vector<E::NamedExpressionPtr> &output_expressions) const {
  DCHECK_EQ(aggregate_expressions_.size(), output_expressions.size());

  std::vector<E::AliasPtr> new_aggregate_expressions;
  new_aggregate_expressions.reserve(aggregate_expressions_.size());
  for (const auto &output_expression : output_expressions) {
    DCHECK(E::SomeAlias::Matches(output_expression));

    new_aggregate_expressions.push_back(
        std::static_pointer_cast<const E::Alias>(output_expression));
  }

  return Create(input_, grouping_expressions_, new_aggregate_expressions, filter_predicate_);
}

void Aggregate::getFieldStringItems(
    std::vector<std::string> *inline_field_names,
    std::vector<std::string> *inline_field_values,
    std::vector<std::string> *non_container_child_field_names,
    std::vector<OptimizerTreeBaseNodePtr> *non_container_child_fields,
    std::vector<std::string> *container_child_field_names,
    std::vector<std::vector<OptimizerTreeBaseNodePtr>> *container_child_fields) const {
  inline_field_names->push_back("has_repartition");
  inline_field_values->push_back(has_repartition_ ? "true" : "false");

  if (partition_scheme_header_) {
    inline_field_names->push_back("output_partition_scheme_header");
    inline_field_values->push_back(partition_scheme_header_->toString());
  }

  non_container_child_field_names->push_back("input");
  non_container_child_fields->push_back(input_);

  container_child_field_names->push_back("grouping_expressions");
  container_child_fields->push_back(
      CastSharedPtrVector<OptimizerTreeBase>(grouping_expressions_));
  container_child_field_names->push_back("aggregate_expressions");
  container_child_fields->push_back(
      CastSharedPtrVector<OptimizerTreeBase>(aggregate_expressions_));

  if (filter_predicate_ != nullptr) {
    non_container_child_field_names->push_back("filter_predicate");
    non_container_child_fields->push_back(filter_predicate_);
  }
}

}  // namespace physical
}  // namespace optimizer
}  // namespace quickstep
