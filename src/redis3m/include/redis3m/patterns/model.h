// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <string>
#include <boost/lexical_cast.hpp>
#include <map>
#include <redis3m/utils/exception.h>
#include <vector>

namespace redis3m
{
namespace patterns
{

REDIS3M_EXCEPTION(model_not_loaded)

#define REDIS3M_MODEL_RO_ATTRIBUTE(type, name) \
public:\
    inline const type& name() const { if (_loaded) return _##name; else throw redis3m::patterns::model_not_loaded(); }\
private:\
    type _##name;

/**
 * @brief Class useful to define models used by {@link orm} and {@link simple_obj_store}.
 * REDIS3M_MODEL_RO_ATTRIBUTE(type, name) macro defines automatically an attribute with
 * a public getter.
 * All fields need to be serialized to a std::map<std::string, std::string>. Model class
 * contains various helpers to to that easily.
 */
class model
{
public:
    /**
     * @brief Default constructor, use it to build a new object, still not saved to
     * database. If you need to access data with getters, change _loaded to true.
     * Otherwise is better to leave it as false
     */
    model():
        _loaded(false)
    {

    }

    /**
     * @brief Contructor called by orm or simple_obj_store pattern. Used to fill
     * object with data from database, call it from subclasses passing parameters
     * as-is. It will set _loaded to true, because data stored on this object is correct,
     * as it's just retrieved from database.
     * @param id identifier of object
     * @param map serialized fields
     */
    model(const std::string& id, const std::map<std::string, std::string>& map):
        _id(id),
        _loaded(true)
    {

    }

    /**
     * @brief Serialize all object data to a string:string map, it will be
     * saved on Redis on a Hash
     * @return
     */
    std::map<std::string, std::string> to_map()
    {
        return std::map<std::string, std::string>();
    }

    /**
     * @brief Object unique identifier
     * @return
     */
    inline const std::string& id() const { if (_loaded) return _id; else throw model_not_loaded(); }

    /**
     * @brief If true means that object data is valid, as just get from database.
     * @return
     */
    inline bool loaded() const { return _loaded; }

    /**
     * @brief A vector of fields to be indexed, used by Orm
     * @return
     */
    static inline std::vector< std::string > indices()
    {
      return std::vector<std::string>();
    }

    /**
     * @brief A vector of fields to be indexed in a unique fashion,
     * used by Orm
     * @return
     */
    static inline std::vector< std::string > uniques()
    {
      return std::vector<std::string>();
    }

    /**
     * @brief Keys associated with a Model, may be sets or lists usually.
     * They will be saved as <Modelname>:<modelid>:<keyname>
     * @return A vector of keys,
     */
    static inline std::vector<std::string> tracked()
    {
        return std::vector<std::string>();
    }

protected:
    /**
     * @brief Convenient method to get a field from a map, or returning a
     * default value if the key is not present. Useful on constructor
     * @param map
     * @param key
     * @param default_value
     * @return
     */
    inline static std::string read_str_from_map(const std::map<std::string, std::string>& map,
                                             const std::string& key,
                                             const std::string& default_value="")
    {
        if (map.find(key) != map.end())
        {
            return map.at(key);
        }
        else
        {
            return default_value;
        }
    }

    /**
     * @brief Write a string to a map, useful on to_map() method. It saves value on
     * the map only if it's different from default
     * @param map
     * @param key
     * @param value
     * @param default_value
     */
    inline static void write_str_to_map(std::map<std::string, std::string>& map,
                                            const std::string& key,
                                            const std::string& value,
                                            const std::string& default_value="")
    {
        if (value != default_value)
        {
            map[key] = value;
        }
    }

    template<typename IntegerType>
    /**
     * @brief Read an integer from map, otherwise use the specified default
     * value (0)
     * @param map
     * @param key
     * @param default_value
     * @return
     */
    inline static IntegerType read_int_from_map(const std::map<std::string, std::string>& map,
                                                    const std::string& key,
                                                    const IntegerType default_value=0)
    {
        if (map.find(key) != map.end())
        {
            return boost::lexical_cast<IntegerType>(map.at(key));
        }
        else
        {
            return default_value;
        }
    }

    template<typename IntegerType>
    /**
     * @brief Write an integer to a map, only if it's different from
     * default value
     * @param map
     * @param key
     * @param value
     * @param default_value
     */
    inline static void write_int_to_map(std::map<std::string, std::string>& map,
                                            const std::string& key,
                                            const IntegerType value,
                                            const IntegerType default_value=0)
    {
        if (value != default_value)
        {
            map[key] = boost::lexical_cast<std::string>(value);
        }
    }

    /**
     * @brief Read a boolean from a map.
     * @param map
     * @param key
     * @return
     */
    inline static bool read_bool_from_map(const std::map<std::string, std::string>& map,
                                              const std::string& key)
    {
        if (map.find(key) != map.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * @brief Write a boolean to a map, only if it's true
     * @param map
     * @param key
     * @param value
     */
    inline static void write_bool_to_map(std::map<std::string, std::string>& map,
                                             const std::string& key,
                                             const bool value)
    {
        if (value)
        {
            map[key] = "true";
        }
    }

    /**
     * @brief Unique identifier of the object, subclasses can set it to a custom
     * value.
     */
    std::string _id;

    /**
     * @brief If this flag is true, data contained by an instance are correct, means
     * that they reflect something stored on database. Can be set to true even for
     * new object, still not saved, if it's necessary (by subclasses).
     */
    bool _loaded;
};


}
}
