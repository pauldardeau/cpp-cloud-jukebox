#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <string>
#include <set>

class PropertyValue {
public:
   std::string data_type;
   std::string empty;

   PropertyValue(const std::string& the_data_type) :
      data_type(the_data_type) {
   }

   PropertyValue(const PropertyValue& other) :
      data_type(other.data_type) {
   }

   virtual ~PropertyValue() {}

   PropertyValue& operator=(const PropertyValue& other) {
      if (this == &other) {
         return *this;
      }

      data_type = other.data_type;

      return *this;
   }

   bool operator==(const PropertyValue& other) const {
      return data_type == other.data_type;
   }

   virtual int get_int_value() const {
      return 0;
   }

   virtual long get_long_value() const {
      return 0L;
   }

   virtual unsigned long get_ulong_value() const {
      return 0L;
   }

   virtual bool get_bool_value() const {
      return false;
   }

   virtual const std::string& get_string_value() const {
      return empty;
   }

   bool is_int() const {
      return data_type == "Int";
   }

   bool is_long() const {
      return data_type == "Long";
   }

   bool is_ulong() const {
      return data_type == "ULong";
   }

   bool is_bool() const {
      return data_type == "Bool";
   }

   bool is_string() const {
      return data_type == "String";
   }

   virtual PropertyValue* clone() const = 0;
};

class IntPropertyValue : public PropertyValue {
private:
   int value;
public:
   IntPropertyValue(int the_value) :
      PropertyValue("Int"),
      value(the_value) {
   }

   IntPropertyValue(const IntPropertyValue& copy) :
      PropertyValue(copy),
      value(copy.value) {
   }

   virtual ~IntPropertyValue() {}

   IntPropertyValue& operator=(const IntPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      value = copy.value;

      return *this;
   }

   bool operator==(const IntPropertyValue& other) const {
      return value == other.value;
   }

   int get_int_value() const {
      return value;
   }

   virtual PropertyValue* clone() const {
      return new IntPropertyValue(value);
   }
};

class LongPropertyValue : public PropertyValue {
private:
   long value;
public:
   LongPropertyValue(long the_value) :
      PropertyValue("Long"),
      value(the_value) {
   }

   LongPropertyValue(const LongPropertyValue& copy) :
      PropertyValue(copy),
      value(copy.value) {
   }

   virtual ~LongPropertyValue() {}

   LongPropertyValue& operator=(const LongPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      value = copy.value;

      return *this;
   }

   bool operator==(const LongPropertyValue& other) const {
      return value == other.value;
   }

   long get_long_value() const {
      return value;
   }

   virtual PropertyValue* clone() const {
      return new LongPropertyValue(value);
   }
};

class ULongPropertyValue : public PropertyValue {
private:
   unsigned long value;
public:
   ULongPropertyValue(unsigned long the_value) :
      PropertyValue("ULong"),
      value(the_value) {
   }

   ULongPropertyValue(const ULongPropertyValue& copy) :
      PropertyValue(copy),
      value(copy.value) {
   }

   virtual ~ULongPropertyValue() {}

   ULongPropertyValue& operator=(const ULongPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      value = copy.value;

      return *this;
   }
   
   bool operator==(const ULongPropertyValue& other) const {
      return value == other.value;
   }

   unsigned long get_ulong_value() const {
      return value;
   }

   virtual PropertyValue* clone() const {
      return new ULongPropertyValue(value);
   }
};

class BoolPropertyValue : public PropertyValue {
private:
   bool value;
public:
   BoolPropertyValue(bool the_value) :
      PropertyValue("Bool"),
      value(the_value) {
   }

   BoolPropertyValue(const BoolPropertyValue& copy) :
      PropertyValue(copy),
      value(copy.value) {
   }

   virtual ~BoolPropertyValue() {}

   BoolPropertyValue& operator=(const BoolPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      value = copy.value;

      return *this;
   }

   bool operator==(const BoolPropertyValue& other) const {
      return value == other.value;
   }

   bool get_bool_value() const {
      return value;
   }

   virtual PropertyValue* clone() const {
      return new BoolPropertyValue(value);
   }
};

class StrPropertyValue : public PropertyValue {
private:
   std::string value;
public:
   StrPropertyValue(const std::string& the_value) :
      PropertyValue("String"),
      value(the_value) {
   }

   StrPropertyValue(const StrPropertyValue& copy) :
      PropertyValue(copy),
      value(copy.value) {
   }

   virtual ~StrPropertyValue() {}

   StrPropertyValue& operator=(const StrPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      value = copy.value;

      return *this;
   }

   bool operator==(const StrPropertyValue& other) const {
      return value == other.value;
   }

   const std::string& get_string_value() const {
      return value;
   }

   virtual PropertyValue* clone() const {
      return new StrPropertyValue(value);
   }
};


class StringSet {
private:
   std::set<std::string> set_of_strings;

public:
   StringSet() {
   }

   StringSet(const StringSet& copy) :
      set_of_strings(copy.set_of_strings) {
   }

   ~StringSet() {
   }

   StringSet& operator=(const StringSet& copy) {
      if (this == &copy) {
         return *this;
      }

      set_of_strings = copy.set_of_strings;

      return *this;
   }

   bool operator==(const StringSet& other) const {
      return set_of_strings == other.set_of_strings;
   }
   
   bool contains(const std::string& s) const {
      auto it = set_of_strings.find(s);
      return it != set_of_strings.end();
   }

   void add(const std::string& s) {
      set_of_strings.insert(s);
   }

   void remove(const std::string& s) {
      set_of_strings.erase(s);
   }

   void clear() {
      set_of_strings.clear();
   }

   void append(const StringSet& other) {
      auto it = other.set_of_strings.begin();
      const auto it_end = other.set_of_strings.end();
      for (; it != it_end; it++) {
         add(*it);
      }
   }

   std::string to_string() const {
      const std::string COMMA_SPACE = ", ";
      bool first_entry = true;
      auto it = set_of_strings.begin();
      const auto it_end = set_of_strings.end();
      std::string combined;

      for (; it != it_end; it++) {
         if (!first_entry) {
            combined += COMMA_SPACE;
         }

         const std::string& current = *it;
         combined += current;

         if (first_entry) {
            first_entry = false;
         }
      }

      return combined;
   }
};

#endif

