#ifndef _LOGCHAN_UTILS
#define _LOGCHAN_UTILS

#include <string>
#include <vector>

class Utils
{
public:
	static std::vector<std::string> split(const std::string& input, char splitter, bool handle_double_quote = false);
};
#endif // _LOGCHAN_UTILS