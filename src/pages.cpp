//
//  pages.cpp
//  leanclub
//
//  Created by TYPCN on 2015/5/2.
//
//

#include "pages.h"
#include "config.h"
#include "utils.h"
#include <map>

#include "restclient/restclient.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "sundown/html.h"

using namespace std;
using namespace rapidjson;

extern string category_nav;
crow::json::wvalue category_id;
crow::json::wvalue category_url;

void BuildCategoryNav(map<int,string> catemap){
    category_nav = "";
    typedef map<int,string>::iterator it_type;
    for(it_type iterator = catemap.begin(); iterator != catemap.end(); iterator++) {
        string id = iterator->second;
        string url = category_id[id]["url"].str();
        string name = category_id[id]["name"].str();
        if(iterator->first < 0){
            category_nav += "<a class=\"menu-item hidden\" href=\"#\">" + url + "</a>";
        }else{
            category_nav += "<a class=\"menu-item\" href=\"/node/" + url + "/1\">" + name + "</a>";
        }
    }
}

void BuildCategoryIndex(){
    RestClient::response r = RestClient::get(DATABASE_URL"category/_design/category/_view/order", true);
    if(r.code == 200){

        Document d;
        d.Parse(r.body.c_str());
        map<int,string> catemap;
        const Value& rows = d["rows"];
        
        for(int i=0; i < rows.Size(); i++) {
            string id = rows[i]["id"].GetString();
            int order = rows[i]["value"]["order"].GetInt();
            string url = rows[i]["value"]["url"].GetString();
            string name = rows[i]["value"]["name"].GetString();
            category_id[id]["url"] = url;
            category_id[id]["name"] = name;
            category_url[url]["id"] = id;
            category_url[url]["name"] = name;
            catemap.insert(make_pair(order, id));
        }
        BuildCategoryNav(catemap);
        
    }else{
        cout << "Fatal Error: Unable to get category,is database down?" << endl;
        exit(-1);
    }
}

crow::json::rvalue GetCategoryInfoByUrl(string url){
    crow::json::rvalue json = crow::json::load(crow::json::dump(category_url[url]));
    return json;
}

crow::json::wvalue render_topic_list_string(string list){
    crow::json::wvalue result;
    crow::json::rvalue json = crow::json::load(list);
    
    for(int i=0; i < json["rows"].size(); i++)
    {
        crow::json::wvalue wv = crow::json::wvalue(json["rows"][i]["value"]);
        string category = wv["category"].str();
        wv["categoryname"] = category_id[category]["name"].str();
        wv["categoryurl"] = category_id[category]["url"].str();
        result[i] = crow::json::load(crow::json::dump(wv));
    }
    return result;
}

string Markdown_to_HTML(string md){
    
    struct hoedown_buffer *ib, *ob;
    ib = hoedown_buffer_new(md.length());
    string result = "Empty Content";
    if (ib != NULL)
    {
        if(md.length() > 0){
            hoedown_buffer_grow(ib, md.length());
            hoedown_buffer_puts(ib,md.c_str());
            ob = hoedown_buffer_new(MARKDOWN_UNIT);
            
            hoedown_renderer *renderer = hoedown_html_renderer_new((hoedown_html_flags)HOEDOWN_HTML_ESCAPE,0);
            unsigned int extensions = HOEDOWN_EXT_TABLES | HOEDOWN_EXT_AUTOLINK | HOEDOWN_EXT_HIGHLIGHT | HOEDOWN_EXT_STRIKETHROUGH;
            hoedown_document *document = hoedown_document_new(renderer,(hoedown_extensions)extensions, 16);
            
            hoedown_document_render(document, ob , ib->data , ib->size);
            hoedown_buffer_free(ib);
            hoedown_document_free(document);
            hoedown_html_renderer_free(renderer);
            result = hoedown_buffer_cstr(ob);
            hoedown_buffer_free(ob);
        }
        
    }
    return result;
}

crow::json::wvalue render_topic_json(crow::json::rvalue &json){
    crow::json::wvalue topic = crow::json::wvalue(json);
    size_t content_num = json["content"].size();
    topic["edit_times"] = content_num - 1;
    for(int i=0; i < content_num; i++)
    {
        /* Convert every post content to HTML */
        topic["content"][i] = Markdown_to_HTML(json["content"][i].s());
        
    }
    if(json.has("replies")){
        size_t reply_num = json["replies"].size();
        topic["reply_num"] = reply_num;
        for(int i=0; i < reply_num; i++)
        {
            /* Convert replies content to HTML */
            topic["replies"][i]["content"] = Markdown_to_HTML(json["replies"][i]["content"].s());
            topic["replies"][i]["floor"] = i+2;
        }
    }else{
        topic["reply_num"] = 0;
    }
    string category = json["category"].s(); // Get category(node) name
    topic["categoryname"] = category_id[category]["name"].str(); // Write category name to result
    topic["categoryurl"] = category_id[category]["url"].str(); // Write category url to result
    return topic;
}
