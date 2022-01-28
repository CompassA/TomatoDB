/*
 * @Author: Tomato
 * @Date: 2022-01-26 22:08:13
 * @LastEditTime: 2022-01-28 17:47:58
 */
#ifndef TOMATO_DB_DB_INCLUDE_TOMATO_DB_H
#define TOMATO_DB_DB_INCLUDE_TOMATO_DB_H

#include <vector>
#include <string>
#include <memory>

namespace tomato {


class DataBaseConfig {

};

class DataBase {
public:
    DataBase();
    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;
    
    virtual ~DataBase() {}

    virtual void put(const std::string& key, const std::string& value) = 0;
    virtual std::string get(const std::string& key) = 0;
    virtual void del(std::string& key) = 0;
    virtual std::vector<std::string> scan(const std::string& begin, const std::string& end);
};

std::shared_ptr<DataBase> createDataBaseInstance(DataBaseConfig config_);


}
#endif