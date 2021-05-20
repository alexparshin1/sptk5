#include "sptk5/wsdl/WSType.h"

using namespace std;
using namespace sptk;

void WSType::owaspCheck(const String& value)
{
    if (value.find("<script") != string::npos || value.find("</script>") != string::npos)
        throw Exception("Invalid value: constains a script");
}
