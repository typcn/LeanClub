// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <redis3m/connection.h>
#include <redis3m/command.h>
#include <map>
#include <string>
#include <algorithm>
#include <redis3m/patterns/model.h>
#include <redis3m/patterns/script_exec.h>
#include <msgpack.hpp>
#include <redis3m/utils/file.h>

namespace redis3m
{
namespace patterns
{

template<typename Model>
/**
 * @brief Object-Redis-Mapper is a convenient pattern to store object on Redis.
 * Useful when you need classic CRUD operations. It's compatible and inspired by
 * http://github.com/soveran/ohm.
 * Data can be indexed and it supports also uniques.
 * To use it make a subclass of {@link model} to model your attribute and use it
 * to fill orm template parameter.
 */
class orm {
public:

    /**
     * @brief As the name says, get an object from id specifying it's unique
     * identifier
     * @param conn
     * @param id
     * @return Use {@link model.loaded()} to check if it's valid or not
     */
    Model find_by_id(connection::ptr_t conn, const std::string& id)
    {
        reply r = conn->run(command("HGETALL")(model_key(id)));
        if (r.elements().size() > 0 )
        {
            std::map<std::string, std::string> map;
            for (unsigned long i = 0; i < r.elements().size(); i+=2 )
            {
                map[r.elements()[i].str()] = r.elements()[i+1].str();
            }
            return Model(id, map);
        }
        else
        {
            return Model();
        }
    }

    /**
     * @brief Find an object by using a unique field
     * @param conn
     * @param field name of field, should be returned by {@link model::uniques}
     * @param value
     * @return Use {@link model.loaded()} to check if it's valid or not
     */
    Model find_by_unique_field(connection::ptr_t conn, const std::string& field, const std::string& value)
    {
        std::string id = conn->run(command("HGET")(unique_field_key(field))(value)).str();
        if (!id.empty())
        {
            return find_by_id(conn, id);
        }
        else
        {
            return Model();
        }
    }

    /**
     * @brief Check if an object exists or not.
     * @param conn
     * @param id Unique identifier
     * @return
     */
    bool exists_by_id(connection::ptr_t conn, const std::string& id)
    {
        return conn->run(command("SISMEMBER")(collection_key())(id)).integer() == 1;
    }

    /**
     * @brief Save an object on database, or update it if it's already present
     * @param conn
     * @param model
     * @return unique identifier of the object, a new one on creation.
     */
    std::string save(connection::ptr_t conn, const Model& model)
    {
        std::map<std::string, std::string> model_map;
        model_map["name"] = model.model_name();
        if (model.loaded() && !model.id().empty())
        {
            model_map["id"] = model.id();
        }
        std::vector<std::string> args;
        msgpack::sbuffer sbuf;  // simple buffer

        // Pack model data
        msgpack::pack(&sbuf, model_map);
        args.push_back(std::string(sbuf.data(), sbuf.size()));
        sbuf.clear();

        // pack model attributes
        std::map<std::string, std::string> attributes = model.to_map();
        std::vector<std::string> attributes_vector;
        typedef std::pair<std::string, std::string> strpair;
        for(const strpair& item : attributes)
        {
            attributes_vector.push_back(item.first);
            attributes_vector.push_back(item.second);
        }

        msgpack::pack(&sbuf, attributes_vector);
        args.push_back(std::string(sbuf.data(), sbuf.size()));
        sbuf.clear();

        // pack model indices

        std::map<std::string, std::vector<std::string> > indices;
        for(const std::string& index : Model::indices())
        {
            std::vector<std::string> values;
            values.push_back(attributes[index]);
            indices[index] = values;
        }
        msgpack::pack(&sbuf, indices);
        args.push_back(std::string(sbuf.data(), sbuf.size()));
        sbuf.clear();

        // pack model uniques
        std::map<std::string, std::string> uniques;
        for(const std::string& index : model.uniques())
        {
            uniques[index] = attributes[index];
        }
        msgpack::pack(&sbuf, uniques);
        args.push_back(std::string(sbuf.data(), sbuf.size()));

        reply r = save_script.exec(conn, std::vector<std::string>(), args);

        return r.str();
    }

    /**
     * @brief Remove object and all related data from database
     * @param conn
     * @param model
     */
    void remove(connection::ptr_t conn, const Model& model)
    {
        std::map<std::string, std::string> model_map;
        model_map["name"] = model.model_name();
        model_map["id"] = model.id();
        model_map["key"] = model_key(model.id());

        std::vector<std::string> args;
        msgpack::sbuffer sbuf;  // simple buffer

        // Pack model data
        msgpack::pack(&sbuf, model_map);
        args.push_back(std::string(sbuf.data(), sbuf.size()));
        sbuf.clear();

        // pack model uniques
        std::map<std::string, std::string> attributes = model.to_map();
        std::map<std::string, std::string> uniques;
        for(const std::string& index : model.uniques())
        {
            uniques[index] = attributes[index];
        }
        msgpack::pack(&sbuf, uniques);
        args.push_back(std::string(sbuf.data(), sbuf.size()));
        sbuf.clear();

        msgpack::pack(&sbuf, Model::tracked());
        args.push_back(std::string(sbuf.data(), sbuf.size()));
        sbuf.clear();

        remove_script.exec(conn, std::vector<std::string>(), args);
    }

    std::vector<std::string> list_members(connection::ptr_t conn, const Model& m, const std::string& list_name)
    {
        std::vector<std::string> ret;
        reply lrange = conn->run(command("LRANGE")
                            (tracked_key(m.id(), list_name))
                            ("0")("-1"));
        for(const reply& r : lrange.elements())
        {
            ret.push_back(r.str());
        }
        return ret;
    }

    void set_add(connection::ptr_t conn, const Model& m, const std::string& set_name, const std::string& entry)
    {
        conn->run(command("SADD")(tracked_key(m.id(), set_name))(entry));
    }

    void set_remove(connection::ptr_t conn, const Model& m, const std::string& set_name, const std::string& entry)
    {
        conn->run(command("SREM")(tracked_key(m.id(), set_name)(entry)));
    }

    std::set<std::string> set_members(connection::ptr_t conn, const Model& m, const std::string& set_name)
    {
        reply r = conn->run(command("SMEMBERS")(tracked_key(m.id(), set_name)));
        std::set<std::string> ret;
        for(const reply& i : r.elements())
        {
            ret.insert(i.str());
        }
        return ret;
    }

    /**
     * @brief Redis key of a SET containing all object ids. Useful for
     * custom operations. Pay attention to mantain compatibility with
     * all other functions.
     * @return
     */
    inline std::string collection_key()
    {
        return Model::model_name() + ":all";
    }

    /**
     * @brief Key containing an incremental counter, used to generate
     * an id for new objects. Useful for custom operations.
     * Pay attention to mantain compatibility with
     * all other functions.
     * @return
     */
    inline std::string collection_id_key()
    {
        return Model::model_name() + ":id";
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

    /**
     * @brief Returns full key, related to object.
     * @param id
     * @param key
     * @return
     */
    inline std::string tracked_key(const std::string& id, const std::string& key)
    {
        return model_key(id) + ":" + key;
    }

    inline std::string indexed_field_key(const std::string& field, const std::string& value)
    {
        return Model::model_name() + ":indices:" + field + ":" + value;
    }

    inline std::string unique_field_key(const std::string& field)
    {
        return Model::model_name() + ":uniques:" + field;
    }

private:
    static script_exec save_script;
    static script_exec remove_script;
};

template<typename Model>
script_exec orm<Model>::save_script(utils::datadir("/lua/save.lua"), true);

template<typename Model>
script_exec orm<Model>::remove_script(utils::datadir("/lua/delete.lua"), true);

}
}
