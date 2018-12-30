//
// Created by liu on 18-10-21.
//

#include "query/QueryParser.h"
#include "query/QueryBuilders.h"
#include "lemondb.h"

#include <getopt.h>
#include <iostream>
#include <string>
#include <sstream>

struct {
    std::string listen;
    long threads = 0;
} parsedArgs;

std::string extractQueryString(std::istream &is) {
    std::string buf;
    do {
        int ch = is.get();
        if (ch == ';') return buf;
        if (ch == EOF) throw std::ios_base::failure("End of input");
        buf.push_back((char) ch);
    } while (true);
}

int execute_query(char const * table_src, char const * query, char const* table_name, char const* dump_path) {

    std::ostringstream os;
    os << "LOAD " << table_src << ";\n";
    os << query << "\n";
    os << "DUMP " << table_name << " " << dump_path << ";\n";

    std::istringstream is(os.str());

    QueryParser p;

    p.registerQueryBuilder(std::make_unique<QueryBuilder(Debug)>());
    p.registerQueryBuilder(std::make_unique<QueryBuilder(ManageTable)>());
    p.registerQueryBuilder(std::make_unique<QueryBuilder(Complex)>());

    size_t counter = 0;

    while (is) {
        try {
            // A very standard REPL
            // REPL: Read-Evaluate-Print-Loop
            std::string queryStr = extractQueryString(is);
            Query::Ptr query = p.parseQuery(queryStr);
            QueryResult::Ptr result = query->execute();
            std::cout << ++counter << "\n";
            if (result->success()) {
                if (result->display()) {
                    std::cout << *result;
                    //std::cout.flush();
                } else {
#ifndef NDEBUG
                    std::cout.flush();
                    std::cerr << *result;
#endif
                    //std::cout.flush();
                }
            } else {
                std::cout.flush();
                std::cerr << "QUERY FAILED:\n\t" << *result;
            }
        }  catch (const std::ios_base::failure& e) {
            // End of input
            break;
        } catch (const std::exception& e) {
            std::cout.flush();
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}
