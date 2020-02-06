
#include <iostream>
#include <jsoncons/json.hpp>
namespace ns {
    typedef struct RoomData
    {
        unsigned int id;
        std::string name;
        std::string maxPlayers;
        unsigned int questionCount;
        unsigned int timePerQuestion;
        unsigned int isActive;
    } RoomData;
}

// std::vector<ns::RoomData> rooms;
// rooms.push_back(ns::RoomData{ 1, "Room 1", "Few", 2, 56, 1 });
// rooms.push_back(ns::RoomData{ 2, "Room 2", "Lots", 2, 56, 1 });


namespace jc = jsoncons;

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_TYPE_TRAITS_DECL(ns::RoomData, id, name, maxPlayers, 
                          questionCount, timePerQuestion, isActive);

int main()
{
    std::vector<ns::RoomData> rooms;
    rooms.push_back(ns::RoomData{ 1, "Room 1", "Few", 2, 56, 1 });
    rooms.push_back(ns::RoomData{ 2, "Room 2", "Lots", 2, 56, 1 });

    std::string s;
    jc::encode_json(rooms, s, jc::indenting::indent);
    std::cout << "(1)\n" << s << "\n\n";

    auto rooms2 = jc::decode_json<std::vector<ns::RoomData>>(s);

    std::cout << "(2)\n";
    for (auto item : rooms2)
    {
        std::cout << "id: " << item.id << ", name: " << item.name 
                  << ", maxPlayers: " << item.maxPlayers << "\n"; 
    }
}
