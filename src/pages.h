//
//  pages.h
//  leanclub
//
//  Created by TYPCN on 2015/5/2.
//
//

#ifndef __leanclub__pages__
#define __leanclub__pages__

#include <stdio.h>
#include <string>
#include "json.h"

void BuildCategoryIndex();
crow::json::rvalue GetCategoryInfoByUrl(std::string url);
crow::json::wvalue render_topic_list_string(std::string list);
crow::json::wvalue render_topic_json(crow::json::rvalue &json);

#endif /* defined(__leanclub__pages__) */
