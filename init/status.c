#include "status.h"

struct status_translation_t {
    status_t value;
    const char* name;
};

struct status_translation_t translations[] = {
    { S_OK, "Success" },
    { 0, 0 }
};

const char* status_translation(status_t status)
{
    for (int i = 0; translations[i].name; ++i) {
        if (translations[i].value == status) {
            return translations[i].name;
        }
    }
    return "unknown";
}
