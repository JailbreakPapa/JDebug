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


class Application : public nsApplication
{
  void AfterEngineInit() NS_OVERRIDE
  {
    // string & string builder
    {
      nsString bla_ns("bla");
      nsStringBuilder bla_ns_builder("bla");
      std::string bla_std = "bla";
    }

    // dynamic array
    {
      nsDynamicArray<nsString> dynarray_ns;
      dynarray_ns.PushBack("asdf");
      dynarray_ns.PushBack("fads");

      std::vector<std::string> dynarray_std;
      dynarray_std.push_back("asdf");
      dynarray_std.push_back("fads");
    }

    // linked list
    {
      nsList<nsString> linkedlist_ns;
      linkedlist_ns.PushBack("asdf");
      linkedlist_ns.PushBack("fads");

      std::vector<std::string> linkedlist_std;
      linkedlist_std.push_back("asdf");
      linkedlist_std.push_back("fads");
    }

    // map
    {
      nsMap<nsString, nsString> map_ns;
      map_ns.Insert("asdf", "value");
      map_ns.Insert("fads", "value");

      std::map<std::string, std::string> map_std;
      map_std.insert(std::pair<std::string, std::string>("asdf", "value"));
      map_std.insert(std::pair<std::string, std::string>("fads", "value"));
    }

    // set
    {
      nsSet<nsString> set_ns;
      set_ns.Insert("asdf");
      set_ns.Insert("fads");

      std::set<std::string> set_std;
      set_std.insert("asdf");
      set_std.insert("fads");
    }

    // hashtable
    {
      nsHashTable<nsString, nsString> hashmap_ns;
      hashmap_ns.Insert("asdf", "value");
      hashmap_ns.Insert("fads", "value"); // currently some troubles here with reading the strings - even in raw view; usage works fine

      std::unordered_map<std::string, nsString> hashmap_std;
      hashmap_std.insert(std::pair<std::string, nsString>("asdf", "value"));
      hashmap_std.insert(std::pair<std::string, nsString>("fads", "value"));
    }

    // deque - doesn't work yet (chunked based design of nsDeque makes it difficult)
  /*{
      nsDeque<nsString> deque_ns;
      deque_ns.PushBack("asdf");
      deque_ns.PushBack("fads");

      std::deque<std::string> deque_std;
      deque_std.push_back("asdf");
      deque_std.push_back("fads");
    }*/

    // array
    {
      nsArrayPtr<int> pArray = NS_DEFAULT_NEW_ARRAY(int, 100);
      NS_DEFAULT_DELETE_ARRAY(pArray);
    }

    // static array
    {
      nsStaticArray<std::string, 2> array_ns;
      array_ns.PushBack("asdf");
      array_ns.PushBack("fads");

      std::array<std::string, 2> array_std;
      array_std[0] = "asdf";
      array_std[1] = "fads";
    }
  }

  virtual ApplicationExecution Run()
  {
    return nsApplication::Quit;
  }
};

NS_CONSOLEAPP_ENTRY_POINT(Application)