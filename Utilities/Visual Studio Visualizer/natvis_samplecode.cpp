#include <string>

#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <array>


#include <Foundation/Basics.h>

#include <Foundation/Strings/String.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/List.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Containers/StaticArray.h>

#include <Core/Application/Application.h>
#include <Foundation/Configuration/Startup.h>


class Application : public wdApplication
{
  void AfterEngineInit() WD_OVERRIDE
  {
    // string & string builder
    {
      wdString bla_wd("bla");
      wdStringBuilder bla_wd_builder("bla");
      std::string bla_std = "bla";
    }

    // dynamic array
    {
      wdDynamicArray<wdString> dynarray_wd;
      dynarray_wd.PushBack("asdf");
      dynarray_wd.PushBack("fads");

      std::vector<std::string> dynarray_std;
      dynarray_std.push_back("asdf");
      dynarray_std.push_back("fads");
    }

    // linked list
    {
      wdList<wdString> linkedlist_wd;
      linkedlist_wd.PushBack("asdf");
      linkedlist_wd.PushBack("fads");

      std::vector<std::string> linkedlist_std;
      linkedlist_std.push_back("asdf");
      linkedlist_std.push_back("fads");
    }

    // map
    {
      wdMap<wdString, wdString> map_wd;
      map_wd.Insert("asdf", "value");
      map_wd.Insert("fads", "value");

      std::map<std::string, std::string> map_std;
      map_std.insert(std::pair<std::string, std::string>("asdf", "value"));
      map_std.insert(std::pair<std::string, std::string>("fads", "value"));
    }

    // set
    {
      wdSet<wdString> set_wd;
      set_wd.Insert("asdf");
      set_wd.Insert("fads");

      std::set<std::string> set_std;
      set_std.insert("asdf");
      set_std.insert("fads");
    }

    // hashtable
    {
      wdHashTable<wdString, wdString> hashmap_wd;
      hashmap_wd.Insert("asdf", "value");
      hashmap_wd.Insert("fads", "value"); // currently some troubles here with reading the strings - even in raw view; usage works fine

      std::unordered_map<std::string, wdString> hashmap_std;
      hashmap_std.insert(std::pair<std::string, wdString>("asdf", "value"));
      hashmap_std.insert(std::pair<std::string, wdString>("fads", "value"));
    }

    // deque - doesn't work yet (chunked based design of wdDeque makes it difficult)
  /*{
      wdDeque<wdString> deque_wd;
      deque_wd.PushBack("asdf");
      deque_wd.PushBack("fads");

      std::deque<std::string> deque_std;
      deque_std.push_back("asdf");
      deque_std.push_back("fads");
    }*/

    // array
    {
      wdArrayPtr<int> pArray = WD_DEFAULT_NEW_ARRAY(int, 100);
      WD_DEFAULT_DELETE_ARRAY(pArray);
    }

    // static array
    {
      wdStaticArray<std::string, 2> array_wd;
      array_wd.PushBack("asdf");
      array_wd.PushBack("fads");

      std::array<std::string, 2> array_std;
      array_std[0] = "asdf";
      array_std[1] = "fads";
    }
  }

  virtual ApplicationExecution Run()
  {
    return wdApplication::Quit;
  }
};

WD_CONSOLEAPP_ENTRY_POINT(Application)