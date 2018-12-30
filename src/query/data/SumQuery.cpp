//
// Created by camelboat on 18-11-1.
//

#include "SumQuery.h"
#include "../../db/Database.h"
#include "../QueryResult.h"

#include <iostream>
#include <numeric>
#include <algorithm>

constexpr const char *SumQuery::qname;

QueryResult::Ptr SumQuery::execute() {
    using namespace std;
    if (this->operands.empty())
        return make_unique<ErrorMsgResult>(
                qname, this->targetTable.c_str(),
                "No operand (? operands)."_f % operands.size()
        );
    Database &db = Database::getInstance();
    try {
        auto &table = db[this->targetTable];
        this->field_sum.reserve(this->operands.size());
        for ( auto it = this->operands.begin();it!=this->operands.end();++it) {
            if (*it == "KEY") {
                throw invalid_argument(
                        R"(Can not input KEY for SUM.)"_f
                );
            } else {
                this->field_sum.emplace_back(std::make_pair(table.getFieldIndex(*it),0));
            }
        }
        auto result = initCondition(table);
        if (result.second) {
            for (auto it = table.begin(); it != table.end(); ++it) {
                if (this->evalCondition(*it)) {
                    for (auto fieldit = this->field_sum.begin(); fieldit!=this->field_sum.end();++fieldit) {
                        (*fieldit).second += (*it)[(*fieldit).first];
                    }
                }
            }
        }
        vector<Table::ValueType> sum_result;
        for (unsigned int i=0;i<this->field_sum.size();i++){
            sum_result.emplace_back(this->field_sum.at(i).second);
        }
        // lcx
//        int *sum = new int [this->operands.size()];
////        cout << "operands size is: " << this->operands.size() << endl;
//        for (unsigned int i = 0; i < this->operands.size(); i++) {
//            int sum_tmp = 0;
//            for (auto it = table.begin(); it != table.end(); it++) {
//                unsigned long field_tmp = table.getFieldIndex(operands[i]);
//                sum_tmp+=it->get(field_tmp);
//            }
//            sum[i] = sum_tmp;
////            int sum_tmp = accumulate(table.field().data()->begin(), table.field().data()->end(), 0);
////            cout << "sum is: " << sum_tmp << endl;
//        }
//        cout << "ANSWER = ( ";
//        for (unsigned int i = 0; i < this->operands.size() ; i++) {
//            cout << sum[i] << " ";
//        }
//        cout << ")" << '\n';
//        free(sum);
        return std::make_unique<AnswerMsgResult>(sum_result);
    }
    catch (const TableNameNotFound &e) {
        return make_unique<ErrorMsgResult>(qname, this->targetTable, "No such table."s);
    } catch (const IllFormedQueryCondition &e) {
        return make_unique<ErrorMsgResult>(qname, this->targetTable, e.what());
    } catch (const invalid_argument &e) {
        // Cannot convert operand to string
        return make_unique<ErrorMsgResult>(qname, this->targetTable, "Unknown error '?'"_f % e.what());
    } catch (const exception &e) {
        return make_unique<ErrorMsgResult>(qname, this->targetTable, "Unkonwn error '?'."_f % e.what());
    }

}

std::string SumQuery::toString() {
    return "QUERY = SUM " + this->targetTable + "\"";
}