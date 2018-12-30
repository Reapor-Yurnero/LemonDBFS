//
// Created by wenda on 10/30/18.
//

#ifndef PROJECT_MAXQUERY_H
#define PROJECT_MAXQUERY_H

#include "../Query.h"

class MaxQuery : public ComplexQuery {
    static constexpr const char *qname = "MAX";
    std::vector<std::pair<Table::FieldIndex,Table::ValueType>> max;
public:
    using ComplexQuery::ComplexQuery;

    QueryResult::Ptr execute() override;

    std::string toString() override;
};



#endif //PROJECT_MAXQUERY_H
