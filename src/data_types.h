#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <string>
#include <set>


//*****************************************************************************

class PropertyValue {
private:
   std::string m_data_type;
   std::string m_empty;


public:
   PropertyValue(const std::string& data_type) :
      m_data_type(data_type) {
   }

   PropertyValue(const PropertyValue& other) :
      m_data_type(other.m_data_type) {
   }

   virtual ~PropertyValue() {}

   PropertyValue& operator=(const PropertyValue& other) {
      if (this == &other) {
         return *this;
      }

      m_data_type = other.m_data_type;

      return *this;
   }

   const std::string& get_data_type() const {
      return m_data_type;
   }

   bool operator==(const PropertyValue& other) const {
      return m_data_type == other.m_data_type;
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
      return m_empty;
   }

   bool is_int() const {
      return m_data_type == "Int";
   }

   bool is_long() const {
      return m_data_type == "Long";
   }

   bool is_ulong() const {
      return m_data_type == "ULong";
   }

   bool is_bool() const {
      return m_data_type == "Bool";
   }

   bool is_string() const {
      return m_data_type == "String";
   }

   virtual PropertyValue* clone() const = 0;
};

//*****************************************************************************

class IntPropertyValue : public PropertyValue {
private:
   int m_value;

public:
   IntPropertyValue(int value) :
      PropertyValue("Int"),
      m_value(value) {
   }

   IntPropertyValue(const IntPropertyValue& copy) :
      PropertyValue(copy),
      m_value(copy.m_value) {
   }

   virtual ~IntPropertyValue() {}

   IntPropertyValue& operator=(const IntPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      m_value = copy.m_value;

      return *this;
   }

   bool operator==(const IntPropertyValue& other) const {
      return m_value == other.m_value;
   }

   int get_int_value() const {
      return m_value;
   }

   virtual PropertyValue* clone() const {
      return new IntPropertyValue(m_value);
   }
};

//*****************************************************************************

class LongPropertyValue : public PropertyValue {
private:
   long m_value;

public:
   LongPropertyValue(long value) :
      PropertyValue("Long"),
      m_value(value) {
   }

   LongPropertyValue(const LongPropertyValue& copy) :
      PropertyValue(copy),
      m_value(copy.m_value) {
   }

   virtual ~LongPropertyValue() {}

   LongPropertyValue& operator=(const LongPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      m_value = copy.m_value;

      return *this;
   }

   bool operator==(const LongPropertyValue& other) const {
      return m_value == other.m_value;
   }

   long get_long_value() const {
      return m_value;
   }

   virtual PropertyValue* clone() const {
      return new LongPropertyValue(m_value);
   }
};

//*****************************************************************************

class ULongPropertyValue : public PropertyValue {
private:
   unsigned long m_value;

public:
   ULongPropertyValue(unsigned long value) :
      PropertyValue("ULong"),
      m_value(value) {
   }

   ULongPropertyValue(const ULongPropertyValue& copy) :
      PropertyValue(copy),
      m_value(copy.m_value) {
   }

   virtual ~ULongPropertyValue() {}

   ULongPropertyValue& operator=(const ULongPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      m_value = copy.m_value;

      return *this;
   }

   bool operator==(const ULongPropertyValue& other) const {
      return m_value == other.m_value;
   }

   unsigned long get_ulong_value() const {
      return m_value;
   }

   virtual PropertyValue* clone() const {
      return new ULongPropertyValue(m_value);
   }
};

//*****************************************************************************

class BoolPropertyValue : public PropertyValue {
private:
   bool m_value;

public:
   BoolPropertyValue(bool value) :
      PropertyValue("Bool"),
      m_value(value) {
   }

   BoolPropertyValue(const BoolPropertyValue& copy) :
      PropertyValue(copy),
      m_value(copy.m_value) {
   }

   virtual ~BoolPropertyValue() {}

   BoolPropertyValue& operator=(const BoolPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      m_value = copy.m_value;

      return *this;
   }

   bool operator==(const BoolPropertyValue& other) const {
      return m_value == other.m_value;
   }

   bool get_bool_value() const {
      return m_value;
   }

   virtual PropertyValue* clone() const {
      return new BoolPropertyValue(m_value);
   }
};

//*****************************************************************************

class StrPropertyValue : public PropertyValue {
private:
   std::string m_value;

public:
   StrPropertyValue(const std::string& value) :
      PropertyValue("String"),
      m_value(value) {
   }

   StrPropertyValue(const StrPropertyValue& copy) :
      PropertyValue(copy),
      m_value(copy.m_value) {
   }

   virtual ~StrPropertyValue() {}

   StrPropertyValue& operator=(const StrPropertyValue& copy) {
      if (this == &copy) {
         return *this;
      }

      PropertyValue::operator=(copy);
      m_value = copy.m_value;

      return *this;
   }

   bool operator==(const StrPropertyValue& other) const {
      return m_value == other.m_value;
   }

   const std::string& get_string_value() const {
      return m_value;
   }

   virtual PropertyValue* clone() const {
      return new StrPropertyValue(m_value);
   }
};

//*****************************************************************************

class StringSet {
private:
   std::set<std::string> m_set_of_strings;

public:
   StringSet() {
   }

   StringSet(const StringSet& copy) :
      m_set_of_strings(copy.m_set_of_strings) {
   }

   ~StringSet() {
   }

   StringSet& operator=(const StringSet& copy) {
      if (this == &copy) {
         return *this;
      }

      m_set_of_strings = copy.m_set_of_strings;

      return *this;
   }

   bool operator==(const StringSet& other) const {
      return m_set_of_strings == other.m_set_of_strings;
   }

   bool contains(const std::string& s) const {
      auto it = m_set_of_strings.find(s);
      return it != m_set_of_strings.end();
   }

   void add(const std::string& s) {
      m_set_of_strings.insert(s);
   }

   void remove(const std::string& s) {
      m_set_of_strings.erase(s);
   }

   void clear() {
      m_set_of_strings.clear();
   }

   void append(const StringSet& other) {
      for (const auto& s : other.m_set_of_strings) {
         add(s);
      }
   }

   std::string to_string() const {
      const std::string COMMA_SPACE = ", ";
      bool first_entry = true;
      std::string combined;

      for (const auto& s : m_set_of_strings) {
         if (!first_entry) {
            combined += COMMA_SPACE;
         }

         const std::string& current = s;
         combined += current;

         if (first_entry) {
            first_entry = false;
         }
      }

      return combined;
   }
};

//*****************************************************************************

#endif

