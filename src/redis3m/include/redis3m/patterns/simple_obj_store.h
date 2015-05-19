// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <string>
#include <redis3m/connection.h>
#include <redis3m/command.h>
#include <map>
#include <stdexcept>

namespace redis3m
{
namespace patterns
{

template<typename Model>
/**
 * @brief Simple object storage, ready to use save, find and remove
 * of {@link model} classes. id management is not provided.
 */
class simple_obj_store
{
public:

    /**
     * @brief As the name says, get an object from id specifying it's unique
     * identifier
     * @param conn
     * @param id
     * @return Use {@link model.loaded()} to check if it's valid or not
     */
    Model find(connection::ptr_t connection, const std::string& id)
    {
        redis3m::reply r = connection->run(redis3m::command("HGETALL")(Model::model_name() + ":" +  id));

        const std::vector<redis3m::reply>& key_values = r.elements();
        if (key_values.size() > 0)
        {
            std::map<std::string, std::string> serialized;

            for (int i=0; i < key_values.size(); i+=2)
            {
                serialized[key_values.at(i).str()] = key_values.at(i+1).str();
            }
            return Model(id, serialized);
        }
        else
        {
            return Model();
        }
    }

    /**
     * @brief Save an object on database, or update it if it's already present.
     * Append only method, for using inside a transaction for example. You should
     * take care of calling {@link connection::get_reply()} after it.
     * @param conn
     * @param model
     */
    void append_save(connection::ptr_t connection, const Model& m)
    {
        std::map<std::string, std::string> serialized = m.to_map();

        std::vector<std::string> hmset_command;
        hmset_command.push_back("HMSET");
        hmset_command.push_back(Model::model_name() + ":" + m.id());

        for(auto item : serialized)
        {
            hmset_command.push_back(item.first);
            hmset_command.push_back(item.second);
        }

        connection->append(hmset_command);
    }

    /**
     * @brief Save an object on database, or update it if it's already present
     * @param conn
     * @param model
     */
    void save(connection::ptr_t connection, const Model& m)
    {
        append_save(connection, m);
        connection->get_reply();
    }

    /**
     * @brief Remove object and all related data from database.
     * Append only method, for using inside a transaction for example. You should
     * take care of calling {@link connection::get_reply()} after it.
     * @param conn
     * @param model
     */
    void append_remove(connection::ptr_t connection, const Model& m)
    {
        connection->append(command("DEL")(m.model_name() + ":" + m.id()));
    }

    /**
     * @brief Remove object and all related data from database
     * @param conn
     * @param model
     */
    void remove(connection::ptr_t connection, const Model& m)
    {
        append_remove(connection, m);
        connection->get_reply();
    }

    /**
     * @brief Returns Redis Key used to save object data
     * @param id
     * @return
     */
    inline std::string model_key(const std::string& id)
    {
      return Model::model_name() + ":" + id;
    }
};
}
}
