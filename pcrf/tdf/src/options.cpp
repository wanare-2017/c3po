/*
* Copyright (c) 2017 Sprint
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <freeDiameter/freeDiameter-host.h>

#include "options.h"

#define RAPIDJSON_NAMESPACE scefrapidjson
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

Options *Options::m_singleton = NULL;

Options::Options()
{
}

Options::~Options()
{
}

void Options::help()
{
   std::cout << std::endl
             << "Usage:  c3poctf -t casshost [OPTIONS]..." << std::endl
             << "  -h, --help                   Print help and exit" << std::endl
             << "  -j, --jsoncfg filename       The JSON configuration file." << std::endl
             << "  -s, --originhost host        The diameter destination host." << std::endl
             << "  -r, --originrealm realm      The diameter destination realm." << std::endl
             << "  -a, --pcrfhost host          The diameter host for the PCRF." << std::endl
             << "  -b, --pcrfrealm realm        The diameter realm for the PCRF." << std::endl
             << "  -t, --casshost host          The Cassandra host IP address." << std::endl
             << "  -k, --casskeyspace keyspace  The Cassandra keyspace." << std::endl
             << "  -z, --logsize size           The maximum log size in MB." << std::endl
             << "  -n, --lognumber number       The number of log files to maintain." << std::endl
             << "  -l, --logname filename       The base filename for the log files." << std::endl
             << "  -q, --logqsize size          The log queue size in bytes, must be a power of 2." << std::endl
             << "  -c, --fdcfg filename         Read the freeDiameter configuration from this file" << std::endl
             << "                               instead of the default location (" DEFAULT_CONF_PATH "/" FD_DEFAULT_CONF_FILENAME ")." << std::endl
   ;
}

bool Options::parse( int argc, char **argv ){

   bool ret = true;

   ret = singleton().parseInputOptions( argc, argv );

   if(ret && !singleton().m_jsoncfg.empty()){
      ret &= singleton().parseJson();
   }

   ret &= singleton().validateOptions();

   return ret;
}


bool Options::parseInputOptions( int argc, char **argv )
{
   int c;
   int option_index = 0;
   bool result = true;
   bool ffound = false;

   struct option long_options[] = {
      { "help",         no_argument,        NULL, 'h' },
      { "jsoncfg",      required_argument,  NULL, 'j' },
      { "originhost",   required_argument,  NULL, 's' },
      { "originrealm",  required_argument,  NULL, 'r' },
      { "pcrfhost",     required_argument,  NULL, 'a' },
      { "pcrfrealm",    required_argument,  NULL, 'b' },
      { "casshost",     required_argument,  NULL, 't' },
      { "casskeyspace", required_argument,  NULL, 'k' },
      { "logsize",      required_argument,  NULL, 'z' },
      { "lognumber",    required_argument,  NULL, 'n' },
      { "logname",      required_argument,  NULL, 'l' },
      { "logqsize",     required_argument,  NULL, 'q' },
      { "fdcfg",        required_argument,  NULL, 'c' },
      { NULL,0,NULL,0 }
   };

   // Loop on arguments
   while (1)
   {
      c = getopt_long(argc, argv, "j:hs:r:t:k:a:b:c:z:n:d:l:", long_options, &option_index );
      if (c == -1)
         break;	// Exit from the loop.

      switch (c)
      {
         case 'h': { help(); exit(0); }
         case 'j': { m_jsoncfg = optarg;              m_options |= opt_jsoncfg; break; }
         case 's': { m_originhost = optarg;           m_options |= opt_originhost; break; }
         case 'r': { m_originrealm = optarg;          m_options |= opt_originrealm; break; }
         case 'a': { m_pcrfhost = optarg;             m_options |= opt_pcrfhost; break; }
         case 'b': { m_pcrfrealm = optarg;            m_options |= opt_pcrfrealm; break; }
         case 'c': { m_fdcfg = optarg;                m_options |= opt_fdcfg; break; }
         case 't': { m_cassHost = optarg;             m_options |= opt_casshost; break; }
         case 'k': { m_cassKeyspace = optarg;         m_options |= opt_cassks; break; }
         case 'z': { m_logmaxsize = atoi(optarg);     m_options |= opt_logmaxsize; break; }
         case 'n': { m_lognbrfiles = atoi(optarg);    m_options |= opt_lognbrfiles; break; }
         case 'l': { m_logfilename = optarg;          m_options |= opt_logfilename; break; }
         case '1': { m_logqueuesize = atoi(optarg);   m_options |= opt_logqueuesize; break; }
         case '?':
         {
            switch ( optopt )
            {
               case 'j': { std::cout << "Option -j (json config) requires an argument" << std::endl; break; }
               case 's': { std::cout << "Option -s (origin host) requires an argument" << std::endl; break; }
               case 'r': { std::cout << "Option -r (origin realm) requires an argument" << std::endl; break; }
               case 'a': { std::cout << "Option -a (PCRF host) requires an argument" << std::endl; break; }
               case 'b': { std::cout << "Option -b (PCRF realm) requires an argument" << std::endl; break; }
               case 't': { std::cout << "Option -t (Cassandra host) requires an argument" << std::endl; break; }
               case 'k': { std::cout << "Option -k (Cassandra keyspace) requires an argument" << std::endl; break; }
               case 'c': { std::cout << "Option -c (diameter configuration file) requires an argument" << std::endl; break; }
               case 'z': { std::cout << "Option -z (log max size) requires an argument" << std::endl; break; }
               case 'n': { std::cout << "Option -n (log number of files) requires an argument" << std::endl; break; }
               case 'l': { std::cout << "Option -l (log filename) requires an argument" << std::endl; break; }
               case 'q': { std::cout << "Option -q (log queue size) requires an argument" << std::endl; break; }
               default: { std::cout << "Unrecognized option [" << c << "]" << std::endl; break; }
            }
            result = false;
         }
         default:
         {
            std::cout << "Unrecognized option [" << c << "]" << std::endl;
            result = false;
         }
      }
   }

   return result;
}

bool Options::parseJson()
{
   char buf[2048];

   FILE *fp = fopen ( m_jsoncfg.c_str() , "r");
   RAPIDJSON_NAMESPACE::FileReadStream is( fp, buf, sizeof(buf) );
   RAPIDJSON_NAMESPACE::Document doc;
   doc.ParseStream<0>( is );
   fclose( fp );

   if(!doc.IsObject()) {
      std::cout << "Error parsing the json config file [" << m_jsoncfg << "]" << std::endl;
      return false;
   }

   if(doc.HasMember("common")){
      const RAPIDJSON_NAMESPACE::Value& commonSection = doc["common"];
      if(!(m_options & opt_fdcfg) && commonSection.HasMember("fdcfg")){
         if(!commonSection["fdcfg"].IsString()){ std::cout << "Error parsing json value: [fdcfg]" << std::endl; return false; }
         m_fdcfg = commonSection["fdcfg"].GetString();
         m_options |= opt_fdcfg;
      }
      if(!(m_options & opt_originhost) && commonSection.HasMember("originhost")){
         if(!commonSection["originhost"].IsString()) { std::cout << "Error parsing json value: [originhost]" << std::endl; return false; }
         m_originhost = commonSection["originhost"].GetString();
         m_options |= opt_originhost;
      }
      if(!(m_options & opt_originrealm) && commonSection.HasMember("originrealm")){
         if(!commonSection["originrealm"].IsString()) { std::cout << "Error parsing json value: [originrealm]" << std::endl; return false; }
         m_originrealm = commonSection["originrealm"].GetString();
         m_options |= opt_originrealm;
      }
   }
   if(doc.HasMember("tdf")){
      const RAPIDJSON_NAMESPACE::Value& tdfSection = doc["tdf"];
      if(!(m_options & opt_pcrfhost) && tdfSection.HasMember("pcrfhost")){
         if(!tdfSection["pcrfhost"].IsString()) { std::cout << "Error parsing json value: [pcrfhost]" << std::endl; return false; }
         m_pcrfhost = tdfSection["pcrfhost"].GetString();
         m_options |= opt_pcrfhost;
      }
      if(!(m_options & opt_pcrfrealm) && tdfSection.HasMember("pcrfrealm")){
         if(!tdfSection["pcrfrealm"].IsString()) { std::cout << "Error parsing json value: [pcrfrealm]" << std::endl; return false; }
         m_pcrfrealm = tdfSection["pcrfrealm"].GetString();
         m_options |= opt_pcrfrealm;
      }
      if(!(m_options & opt_casshost) && tdfSection.HasMember("casshost")){
         if(!tdfSection["casshost"].IsString()) { std::cout << "Error parsing json value: [casshost]" << std::endl; return false; }
         m_cassHost = tdfSection["casshost"].GetString();
         m_options |= opt_casshost;
      }
      if(!(m_options & opt_cassks) && tdfSection.HasMember("casskeyspace")){
         if(!tdfSection["casskeyspace"].IsString()) { std::cout << "Error parsing json value: [casskeyspace]" << std::endl; return false; }
         m_cassKeyspace = tdfSection["casskeyspace"].GetString();
         m_options |= opt_cassks;
      }
      if(!(m_options & opt_logmaxsize) && tdfSection.HasMember("logsize")){
         if(!tdfSection["logsize"].IsInt()) { std::cout << "Error parsing json value: [logsize]" << std::endl; return false; }
         m_logmaxsize = tdfSection["logsize"].GetInt();
         m_options |= opt_logmaxsize;
      }
      if(!(m_options & opt_lognbrfiles) && tdfSection.HasMember("lognumber")){
         if(!tdfSection["lognumber"].IsInt()) { std::cout << "Error parsing json value: [lognumber]" << std::endl; return false; }
         m_lognbrfiles = tdfSection["lognumber"].GetInt();
         m_options |= opt_lognbrfiles;
      }
      if(!(m_options & opt_logfilename) && tdfSection.HasMember("logname")){
         if(!tdfSection["logname"].IsString()) { std::cout << "Error parsing json value: [logname]" << std::endl; return false; }
         m_logfilename = tdfSection["logname"].GetString();
         m_options |= opt_logfilename;
      }
      if(!(m_options & opt_logqueuesize) && tdfSection.HasMember("logqsize")){
         if(!tdfSection["logqsize"].IsInt()) { std::cout << "Error parsing json value: [logqsize]" << std::endl; return false; }
         m_logqueuesize = tdfSection["logqsize"].GetInt();
         m_options |= opt_logqueuesize;
      }
   }

   return true;
}

bool Options::validateOptions(){
   return
         ( m_options & opt_jsoncfg )
      && ( m_options & opt_originhost )
      && ( m_options & opt_originrealm )
      && ( m_options & opt_fdcfg )
      && ( m_options & opt_casshost )
      && ( m_options & opt_cassks )
      && ( m_options & opt_pcrfhost )
      && ( m_options & opt_pcrfrealm )
      && ( m_options & opt_logmaxsize )
      && ( m_options & opt_lognbrfiles )
      && ( m_options & opt_logfilename )
      ;
}

