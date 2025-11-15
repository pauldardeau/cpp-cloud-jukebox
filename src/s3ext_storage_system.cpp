#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "s3ext_storage_system.h"
#include "OSUtils.h"
#include "StrUtils.h"

using namespace std;
using namespace chaudiere;

// Helpful links:
// https://docs.ceph.com/en/latest/radosgw/s3/csharp/
// https://docs.ceph.com/en/latest/radosgw/s3/cpp/


//*****************************************************************************
//*****************************************************************************

S3ExtStorageSystem::S3ExtStorageSystem(const string& access_key,
                                       const string& secret_key,
                                       const string& protocol,
                                       const string& host,
                                       const string& container_prefix,
                                       bool debug) :
   StorageSystem("S3Ext", debug),
   m_debug_mode(debug),
   m_aws_access_key(access_key),
   m_aws_secret_key(secret_key),
   m_s3_host(host),
   m_connected(false)
{
   string protocolInUse(protocol);

   if (!protocol.empty()) {
      StrUtils::toLowerCase(protocolInUse);
      if (protocolInUse == "http") {
         //s3_protocol = S3ProtocolHTTP;
      } else if (protocolInUse == "https") {
         //s3_protocol = S3ProtocolHTTPS;
      } else {
         /*
         s3_protocol = S3ProtocolHTTPS;
         protocolInUse = "https";
         printf("S3ExtStorageSystem error: unrecognized protocol '%s'\n",
                protocol.c_str());
         printf("valid options are: 'http' and 'https'\n");
         printf("using default value of %s\n", protocolInUse.c_str());
         */
      }
   }

   if (debug_mode()) {
      printf("S3ExtStorageSystem parameters:\n");
      printf("access_key=%s\n", m_aws_access_key.c_str());
      printf("secret_key=%s\n", m_aws_secret_key.c_str());
      printf("protocol=%s\n", protocolInUse.c_str());
      printf("host=%s\n", m_s3_host.c_str());
      printf("container_prefix=%s\n", container_prefix.c_str());
   }
}

//*****************************************************************************

S3ExtStorageSystem::~S3ExtStorageSystem() {
   S3ExtStorageSystem::exit();
}

//*****************************************************************************

bool S3ExtStorageSystem::enter() {
   if (debug_mode()) {
      printf("S3ExtStorageSystem.enter\n");
   }

   set_list_containers(list_account_containers());

   return true;
}

//*****************************************************************************

void S3ExtStorageSystem::exit() {
   if (debug_mode()) {
      printf("S3ExtStorageSystem.exit\n");
   }

   m_connected = false;
   set_authenticated(false);
}

//*****************************************************************************

vector<string> S3ExtStorageSystem::list_account_containers() {
   if (debug_mode()) {
      printf("list_account_containers\n");
   }

   vector<string> list_containers;
   KeyValuePairs kvp;
   populate_common_variables(kvp);

   string script_template = "s3-list-containers.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (!run_program(run_script, list_containers)) {
         list_containers.clear();
         printf("S3ExtStorageSystem::list_account_containers - error: unable to run script\n");
      }
   } else {
      printf("S3ExtStorageSystem::list_account_containers - error: unable to prepare script\n");
   }

   Utils::file_delete(run_script);

   return list_containers;
}

//*****************************************************************************

bool S3ExtStorageSystem::create_container(const string& container_name) {
   if (debug_mode()) {
      printf("create_container: %s\n", container_name.c_str());
   }

   bool container_created = false;

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);

   string script_template = "s3-create-container.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (run_program(run_script)) {
         container_created = true;
      } else {
         printf("S3ExtStorageSystem::create_container - error: create container '%s' failed\n", container_name.c_str());
      }
   } else {
      printf("S3ExtStorageSystem::create_container - error: unable to prepare run script\n");
   }

   Utils::file_delete(run_script);

   return container_created;
}

//*****************************************************************************

bool S3ExtStorageSystem::delete_container(const string& container_name) {
   if (debug_mode()) {
      printf("delete_container: %s\n", container_name.c_str());
   }

   bool container_deleted = false;

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);

   string script_template = "s3-delete-container.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (run_program(run_script)) {
         container_deleted = true;
      }
   }

   Utils::file_delete(run_script);

   return container_deleted;
}

//*****************************************************************************

vector<string> S3ExtStorageSystem::list_container_contents(const string& container_name) {
   if (debug_mode()) {
      printf("list_container_contents: %s\n", container_name.c_str());
   }

   vector<string> list_objects;

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);

   string script_template = "s3-list-container-contents.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (!run_program(run_script, list_objects)) {
         list_objects.clear();
         printf("S3ExtStorageSystem::list_container_contents - error: unable to run program\n");
      }
   } else {
      printf("S3ExtStorageSystem::list_container_contents - error: unable to prepare run script\n");
   }

   Utils::file_delete(run_script);

   return list_objects;
}

//*****************************************************************************

bool S3ExtStorageSystem::get_object_metadata(const string& container_name,
                                             const string& object_name,
                                             PropertySet& properties) {
   if (debug_mode()) {
      printf("get_object_metadata: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   bool success = false;

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);
   populate_object(kvp, object_name);

   string script_template = "s3-head-object.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      string std_out;
      if (run_program(run_script, std_out)) {
         printf("%s\n", std_out.c_str());
         success = true;
      }
   }

   Utils::file_delete(run_script);

   return success;
}

//*****************************************************************************

bool S3ExtStorageSystem::put_object(const string& container_name,
                                    const string& object_name,
                                    const vector<unsigned char>& file_contents,
                                    const PropertySet* headers) {

   if (debug_mode()) {
      printf("put_object: container=%s, object=%s, length=%ld\n",
             container_name.c_str(),
             object_name.c_str(),
             file_contents.size());
   }

   bool object_added = false;

   string tmp_file = "tmp_";
   tmp_file += container_name;
   tmp_file += "_";
   tmp_file += object_name;

   if (Utils::file_write_all_bytes(tmp_file, file_contents)) {
      Utils::file_set_permissions(tmp_file, 6, 0, 0);
      object_added = put_object_from_file(container_name,
                                          object_name,
                                          tmp_file,
                                          headers);
      Utils::file_delete(tmp_file);
   } else {
      printf("error: put_object not able to write to tmp file\n");
   }

   return object_added;
}

//*****************************************************************************

bool S3ExtStorageSystem::put_object_from_file(const string& container_name,
                                              const string& object_name,
                                              const string& file_path,
                                              const PropertySet* headers) {
   if (debug_mode()) {
      printf("put_object_from_file: container=%s, object=%s, file_path=%s\n",
             container_name.c_str(),
             object_name.c_str(),
             file_path.c_str());
   }

   bool object_added = false;
   string metadata_props;

   /*
   put                  : Puts an object
     <bucket>/<key>     : Bucket/key to put object to
     [filename]         : Filename to read source data from (default is stdin)
     [contentLength]    : How many bytes of source data to put (required if
                          source file is stdin)
     [cacheControl]     : Cache-Control HTTP header string to associate with
                          object
     [contentType]      : Content-Type HTTP header string to associate with
                          object
     [md5]              : MD5 for validating source data
     [contentDispositionFilename] : Content-Disposition filename string to
                          associate with object
     [contentEncoding]  : Content-Encoding HTTP header string to associate
                          with object
     [expires]          : Expiration date to associate with object
     [cannedAcl]        : Canned ACL for the object (see Canned ACLs)
     [x-amz-meta-...]]  : Metadata headers to associate with the object
   */

   // each metadata property (aside from predefined ones) gets "x-amz-meta-" prefix

   if (headers != nullptr) {
      if (headers->contains(PropertySet::PROP_CONTENT_LENGTH)) {
         unsigned long content_length =
            headers->get_ulong_value(PropertySet::PROP_CONTENT_LENGTH);
         metadata_props += "contentLength=";
         metadata_props += StrUtils::toString(content_length);
         metadata_props += " ";
      }

      if (headers->contains(PropertySet::PROP_CONTENT_TYPE)) {
         string content_type =
            headers->get_string_value(PropertySet::PROP_CONTENT_TYPE);
         // contentType
         if (!content_type.empty()) {
            metadata_props += "contentType=";
            metadata_props += content_type;
            metadata_props += " ";
         }
      }

      if (headers->contains(PropertySet::PROP_CONTENT_MD5)) {
         string content_md5 =
            headers->get_string_value(PropertySet::PROP_CONTENT_MD5);
         // md5
         if (!content_md5.empty()) {
            metadata_props += "md5=";
            metadata_props += content_md5;
            metadata_props += " ";
         }
      }

      if (headers->contains(PropertySet::PROP_CONTENT_ENCODING)) {
         string content_encoding =
            headers->get_string_value(PropertySet::PROP_CONTENT_ENCODING);
         // contentEncoding
         if (!content_encoding.empty()) {
            metadata_props += "contentEncoding=";
            metadata_props += content_encoding;
            metadata_props += " ";
         }
      }
   }

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);
   populate_object(kvp, object_name);

   string script_template;

   StrUtils::strip(metadata_props);

   if (!metadata_props.empty()) {
      script_template = "s3-put-object-props.sh";
      kvp.addPair("%%METADATA_PROPERTIES%%", metadata_props);
   } else {
      script_template = "s3-put-object.sh";
   }

   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (run_program(run_script)) {
         object_added = true;
      }
   }

   Utils::file_delete(run_script);

   return object_added;
}

//*****************************************************************************

bool S3ExtStorageSystem::delete_object(const string& container_name,
                                       const string& object_name) {
   if (debug_mode()) {
      printf("delete_object: container=%s, object=%s\n",
             container_name.c_str(), object_name.c_str());
   }

   bool object_deleted = false;

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);
   populate_object(kvp, object_name);

   string script_template = "s3-delete-object.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (run_program(run_script)) {
         object_deleted = true;
      }
   }

   Utils::file_delete(run_script);

   return object_deleted;
}

//*****************************************************************************

int64_t S3ExtStorageSystem::get_object(const string& container_name,
                                       const string& object_name,
                                       const string& local_file_path) {
   if (debug_mode()) {
      printf("get_object: container=%s, object=%s, local_file_path=%s\n",
             container_name.c_str(), object_name.c_str(),
             local_file_path.c_str());
   }

   if (local_file_path.length() == 0) {
      printf("error: local file path is empty\n");
      return 0;
   }

   bool success = false;

   KeyValuePairs kvp;
   populate_common_variables(kvp);
   populate_bucket(kvp, container_name);
   populate_object(kvp, object_name);
   kvp.addPair("%%OUTPUT_FILE%%", local_file_path);

   string script_template = "s3-get-object.sh";
   string run_script = run_script_name_for_template(script_template);

   if (prepare_run_script(script_template, kvp)) {
      if (run_program(run_script)) {
         success = true;
      }
   }

   Utils::file_delete(run_script);

   if (success) {
      return Utils::get_file_size(local_file_path);
   } else {
      return 0;
   }
}

//*****************************************************************************

void S3ExtStorageSystem::populate_common_variables(KeyValuePairs& kvp) {
   kvp.addPair("%%S3_ACCESS_KEY%%", m_aws_access_key);
   kvp.addPair("%%S3_SECRET_KEY%%", m_aws_secret_key);
   kvp.addPair("%%S3_HOST%%", m_s3_host);
}

//*****************************************************************************

void S3ExtStorageSystem::populate_bucket(KeyValuePairs& kvp,
                                         const string& bucket_name) {
   kvp.addPair("%%BUCKET_NAME%%", bucket_name);
}

//*****************************************************************************

void S3ExtStorageSystem::populate_object(KeyValuePairs& kvp,
                                         const string& object_name) {
   kvp.addPair("%%OBJECT_NAME%%", object_name);
}

//*****************************************************************************

bool S3ExtStorageSystem::run_program(const string& program_path,
                                     vector<string>& list_output_lines) {
   bool success = false;

   if (!Utils::file_exists(program_path)) {
      printf("run_program: error '%s' does not exist\n", program_path.c_str());
      return false;
   }

   bool is_shell_script = false;
   string executable_path = program_path;

   if (StrUtils::endsWith(program_path, ".sh")) {
      vector<string> file_lines = Utils::file_read_lines(program_path);
      if (file_lines.size() == 0) {
         printf("run_program: unable to read file '%s'\n", program_path.c_str());
         return false;
      }
      const string& first_line = file_lines[0];
      if (StrUtils::startsWith(first_line, "#!")) {
         size_t line_length = first_line.length();
         executable_path = first_line.substr(2, line_length-2);
      } else {
         executable_path = "/bin/sh";
      }
      is_shell_script = true;
   }

   vector<string> program_args;
   int exit_code = 0;
   string std_out;
   string std_err;

   if (is_shell_script) {
      program_args.push_back(program_path);
   }

   if (Utils::execute_program(executable_path,
                              program_args,
                              exit_code,
                              std_out,
                              std_err)) {
      //printf("exit_code = %d\n", exit_code);
      //printf("*********** START STDOUT **************\n");
      //printf("%s\n", std_out.c_str());
      //printf("*********** END STDOUT **************\n");

      if (exit_code == 0) {
         if (!std_out.empty()) {
            vector<string> output_lines = StrUtils::split(std_out, "\n");
            for (const auto& line : output_lines) {
               if (!line.empty()) {
                  list_output_lines.push_back(line);
               }
            }
         }
         success = true;
      }
   }

   return success;
}

//*****************************************************************************

bool S3ExtStorageSystem::run_program(const string& program_path,
                                     string& std_out_text) {
   bool success = false;

   if (!Utils::file_exists(program_path)) {
      printf("run_program: error '%s' does not exist\n", program_path.c_str());
      return false;
   }

   bool is_shell_script = false;
   string executable_path = program_path;

   if (StrUtils::endsWith(program_path, ".sh")) {
      vector<string> file_lines = Utils::file_read_lines(program_path);
      if (file_lines.size() == 0) {
         printf("run_program: unable to read file '%s'\n", program_path.c_str());
         return false;
      }
      const string& first_line = file_lines[0];
      if (StrUtils::startsWith(first_line, "#!")) {
         size_t line_length = first_line.length();
         executable_path = first_line.substr(2, line_length-2);
      } else {
         executable_path = "/bin/sh";
      }
      is_shell_script = true;
   }

   vector<string> program_args;
   int exit_code = 0;
   string std_out;
   string std_err;

   if (is_shell_script) {
      program_args.push_back(program_path);
   }

   if (Utils::execute_program(executable_path,
                              program_args,
                              exit_code,
                              std_out,
                              std_err)) {
      if (exit_code == 0) {
         if (!std_out.empty()) {
            std_out_text = std_out;
         }
         success = true;
      }
   }

   return success;
}

//*****************************************************************************

bool S3ExtStorageSystem::run_program(const string& program_path) {
   bool success = false;

   if (!Utils::file_exists(program_path)) {
      printf("run_program: error '%s' does not exist\n", program_path.c_str());
      return false;
   }

   bool is_shell_script = false;
   string executable_path = program_path;

   if (StrUtils::endsWith(program_path, ".sh")) {
      vector<string> file_lines = Utils::file_read_lines(program_path);
      if (file_lines.size() == 0) {
         printf("run_program: unable to read file '%s'\n", program_path.c_str());
         return false;
      }
      const string& first_line = file_lines[0];
      if (StrUtils::startsWith(first_line, "#!")) {
         size_t line_length = first_line.length();
         executable_path = first_line.substr(2, line_length-2);
      } else {
         executable_path = "/bin/sh";
      }
      is_shell_script = true;
   }

   vector<string> program_args;
   int exit_code = 0;
   string std_out;
   string std_err;

   if (is_shell_script) {
      program_args.push_back(program_path);
   }

   if (Utils::execute_program(executable_path,
                              program_args,
                              exit_code,
                              std_out,
                              std_err)) {
      if (exit_code == 0) {
         success = true;
      }
   }

   return success;
}

//*****************************************************************************

bool S3ExtStorageSystem::prepare_run_script(const string& script_template,
                                            const KeyValuePairs& kvp) {
   string run_script = run_script_name_for_template(script_template);

   if (!Utils::file_copy(script_template, run_script)) {
      return false;
   }

   if (!Utils::file_set_permissions(run_script, 7, 0, 0)) {
      return false;
   }

   string file_text;
   if (!Utils::file_read_all_text(run_script, file_text)) {
      return false;
   }

   vector<string> kvp_keys;
   kvp.getKeys(kvp_keys);

   for (const auto& key : kvp_keys) {
      StrUtils::replaceAll(file_text, key, kvp.getValue(key));
   }

   if (!Utils::file_write_all_text(run_script, file_text)) {
      return false;
   }

   return true;
}

//*****************************************************************************

string S3ExtStorageSystem::run_script_name_for_template(const string& script_template) {
   string run_script = "exec-";
   run_script += script_template;
   return run_script;
}

//*****************************************************************************

