{% if mode != "delivery" %}
static const char * dal3ibisversion[] = {"ISDC component {{component_name}} {{component_version}}"};
{% else %}
#include "git_version.h"

#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
static const char * dal3ibisversion[] = {"ISDC component {{component_name}} {{component_version}}"};
{% endif %}
