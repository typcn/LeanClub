//
//  base64.h
//  leanclub
//
//  Created by TYPCN on 2015/5/6.
//
//

#ifndef __leanclub__base64__
#define __leanclub__base64__

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif /* defined(__leanclub__base64__) */
