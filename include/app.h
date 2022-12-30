#include <string>

namespace slf
{

std::string parse_argv(int argc, const char* argv[], const char* desc_str);

void initialize_logger(std::string log_path, std::string log_level);
}
