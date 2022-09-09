#include "property_set.h"

using namespace std;

static const string EMPTY = "";


PropertySet::PropertySet() {
}

PropertySet::~PropertySet() {
   //TODO: delete all PropertyValue instances
}

void PropertySet::add(const std::string& prop_name, PropertyValue* prop_value) {
   if (prop_value != NULL) {
      map_props[prop_name] = prop_value;
   }
}

bool PropertySet::contains(const std::string& prop_name) const {
   auto it = map_props.find(prop_name);
   return it != map_props.end();
}

const PropertyValue* PropertySet::get(const std::string& prop_name) const {
   auto it = map_props.find(prop_name);
   if (it != map_props.end()) {
      return it->second;
   } else {
      return NULL;
   }
}

int PropertySet::get_int_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_int()) {
      return pv->get_int_value();
   } else {
      return 0;
   }
}

long PropertySet::get_long_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_long()) {
      return pv->get_long_value();
   } else {
      return 0L;
   }
}

unsigned long PropertySet::get_ulong_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_ulong()) {
      return pv->get_ulong_value();
   } else {
      return 0L;
   }
}

bool PropertySet::get_bool_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_bool()) {
      return pv->get_bool_value();
   } else {
      return false;
   }
}

const std::string& PropertySet::get_string_value(const std::string& prop_name) const {
   const PropertyValue* pv = get(prop_name);
   if (pv->is_string()) {
      return pv->get_string_value();
   } else {
      return EMPTY;
   }
}

